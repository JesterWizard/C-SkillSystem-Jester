#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/chatlog.h"
#include "kernel/utf8.h"
#include "scene.h"
#include "face.h"
#include "fontgrp.h"
#include "hardware.h"
#include "bmlib.h"
#include "proc.h"
#include "ctc.h"
#include "agb_sram.h"
#include "bmunit.h"

enum {
	/*
	 * Talk glyphs live at 0x80..~0x16F. 224-color BG3 starts at tile 0x200.
	 * Overlay text fits in the gap: 4 rows * (8+8) columns * 2 tiles = 128.
	 */
	CHATLOG_TEXT_CHR = 0x180,
	CHATLOG_TEXT_PAL = 2,
	/*
	 * This build's gDefaultFaceConfig parks faces at OBJ 0x4000/0x5000/0x6000/0x7000
	 * (chr 0x200/0x280/0x300/0x380). 0x280 is slot 1 and the overflow of slot 2 (Seth).
	 * Low OBJ 0x000-0x1FF is free during Talk; pack four 32x32 chibis at 0x1C0.
	 */
	CHATLOG_CHIBI_CHR = 0x1C0,
	CHATLOG_CHIBI_PAL = 0,
	CHATLOG_CHIBI_CHR_STRIDE = 8,
	CHATLOG_DIRTY_X = 1,
	CHATLOG_DIRTY_Y = 1,
	CHATLOG_DIRTY_W = 21,
	CHATLOG_DIRTY_H = CHATLOG_VISIBLE * CHATLOG_ENTRY_H,
	CHATLOG_NAME_W = 8,
	CHATLOG_BODY_W = 8,
	CHATLOG_PAL_COUNT = 32,
};

#define sTalkState (*(struct TalkState **)0x0859133C)

struct ChatlogUiState {
	struct Font font;
	struct Text texts[2];
	u16 palBackup[CHATLOG_PAL_COUNT * 16];
	u8 talkWin0Left;
	u8 talkWin0Top;
	u8 talkWin0Right;
	u8 talkWin0Bottom;
	u8 capture;
	u8 chibiMask;
};

extern struct ChatLogState sChatLogState;
extern struct ChatlogUiState sChatlogUiState;

static const u16 Sprite_ChatlogChibi[] = {
	2,
	OAM0_SHAPE_32x16, OAM1_SIZE_32x16, OAM2_CHR(0),
	OAM0_SHAPE_32x16 + OAM0_Y(16), OAM1_SIZE_32x16, OAM2_CHR(4),
};

extern const struct ProcCmd gProcScr_Talk[];
extern const struct ProcCmd gProcScr_TalkWaitForInput[];
extern const struct ProcCmd gProcScr_TalkSkipListener[];
extern const struct ProcCmd gProcScr_TalkShiftClearAll[];
extern const struct ProcCmd gProcScr_TalkShiftClear[];
extern u16 const *gPressKeyArrowSpriteLut[];

static void Chatlog_OnIdle(ProcPtr proc);
static void Chatlog_Open(void);
static void Chatlog_Close(void);
static void Chatlog_Draw(void);
static void Chatlog_StoreName(struct ChatLogEntry *entry, u16 nameTextId);
static struct ChatLogEntry *Chatlog_LiveEntry(void);
static void Chatlog_EnsureLiveEntry(void);

static const struct ProcCmd ProcScr_Chatlog[] = {
	PROC_NAME("Chatlog"),
	PROC_MARK(PROC_MARK_TALK),
	PROC_REPEAT(Chatlog_OnIdle),
	PROC_END,
};

_Static_assert(sizeof(struct ChatLogEntry) == 76, "ChatLogEntry size");
_Static_assert(sizeof(struct ChatLogState) == 0x478, "ChatLogState size");
_Static_assert(sizeof(struct ChatlogUiState) <= 0x600, "ChatlogUiState size");

bool Chatlog_IsVisible(void)
{
	return (sChatLogState.flags & CHATLOG_FLAG_VISIBLE) != 0;
}

static int Chatlog_TotalLines(void)
{
	int total = sChatLogState.count;

	if (sChatLogState.flags & CHATLOG_FLAG_LINEOPEN)
		total++;

	return total;
}

static struct ChatLogEntry *Chatlog_LiveEntry(void)
{
	return &sChatLogState.entries[sChatLogState.head];
}

static void Chatlog_ResetRing(void)
{
	CpuFill16(0, &sChatLogState, sizeof(sChatLogState));
}

void ChapterInit_ResetChatlog(void)
{
	Chatlog_ResetRing();
}

void SaveChatLogSuspendState(u8 *dst, const u32 size)
{
	struct ChatLogState tmp;

	Assert(size == sizeof(sChatLogState));
	tmp = sChatLogState;
	tmp.flags = 0;
	WriteAndVerifySramFast(&tmp, dst, size);
}

void LoadChatLogSuspendState(u8 *src, const u32 size)
{
	Assert(size == sizeof(sChatLogState));
	ReadSramFast(src, &sChatLogState, size);
	sChatLogState.flags = 0;
	if (sChatLogState.count > CHATLOG_CAP ||
		sChatLogState.head >= CHATLOG_CAP ||
		sChatLogState.viewTop >= CHATLOG_CAP)
		Chatlog_ResetRing();
}

static u8 Chatlog_CurrentSpeakerCharId(void)
{
	u8 charId = TextEngine_GetSpeakingCharacterId();
	int faceId;

	if (charId)
		return charId;

	faceId = TextEngine_GetSpeakingFaceId();
	if (faceId > 0 && faceId <= 0xFF)
		return (u8)faceId;

	if (UNIT_IS_VALID(gActiveUnit) && gActiveUnit->pCharacterData)
		return UNIT_CHAR_ID(gActiveUnit);

	return 0;
}

static void Chatlog_FillSpeaker(struct ChatLogEntry *entry)
{
	const struct CharacterData *character;
	int faceId;
	u16 nameTextId;

	entry->charId = Chatlog_CurrentSpeakerCharId();
	faceId = TextEngine_GetSpeakingFaceId();
	entry->portraitId = (faceId > 0) ? (u16)faceId : 0;
	entry->name[0] = '\0';

	nameTextId = TextEngine_GetSpeakingNameTextId();
	if (entry->charId) {
		character = GetCharacterData(entry->charId);
		if (character) {
			if (!entry->portraitId && character->portraitId)
				entry->portraitId = character->portraitId;
			if (!nameTextId)
				nameTextId = character->nameTextId;
		}
	}

	if (nameTextId)
		Chatlog_StoreName(entry, nameTextId);
}

static void Chatlog_StoreName(struct ChatLogEntry *entry, u16 nameTextId)
{
	char buf[0x40];
	int i;

	entry->name[0] = '\0';
	if (!nameTextId)
		return;

	GetStringFromIndexInBuffer(nameTextId, buf);
	for (i = 0; i < CHATLOG_NAME_LEN - 1 && buf[i]; i++) {
		if ((u8)buf[i] < 0x20)
			break;
		entry->name[i] = buf[i];
	}
	entry->name[i] = '\0';
}

static void Chatlog_AppendGlyph(u32 unicod)
{
	struct ChatLogEntry *entry;
	char bytes[4];
	int used;
	int len;
	int i;

	if (unicod < 0x20 || unicod == 0x7F)
		return;

	if (unicod < 0x80) {
		bytes[0] = (char)unicod;
		len = 1;
	} else if (unicod < 0x800) {
		bytes[0] = (char)(0xC0 | (unicod >> 6));
		bytes[1] = (char)(0x80 | (unicod & 0x3F));
		len = 2;
	} else if (unicod < 0x10000) {
		bytes[0] = (char)(0xE0 | (unicod >> 12));
		bytes[1] = (char)(0x80 | ((unicod >> 6) & 0x3F));
		bytes[2] = (char)(0x80 | (unicod & 0x3F));
		len = 3;
	} else {
		return;
	}

	Chatlog_EnsureLiveEntry();
	entry = Chatlog_LiveEntry();
	if (entry->charId == 0)
		Chatlog_FillSpeaker(entry);
	used = 0;
	while (entry->text[used])
		used++;

	if (used + len >= CHATLOG_TEXT_LEN - 1)
		return;

	for (i = 0; i < len; i++)
		entry->text[used + i] = bytes[i];
	entry->text[used + len] = '\0';
}

static void Chatlog_EnsureLiveEntry(void)
{
	struct ChatLogEntry *entry;

	if (sChatLogState.flags & CHATLOG_FLAG_LINEOPEN)
		return;

	entry = Chatlog_LiveEntry();
	CpuFill16(0, entry, sizeof(*entry));
	Chatlog_FillSpeaker(entry);
	sChatLogState.flags |= CHATLOG_FLAG_LINEOPEN;
}

static struct ChatLogEntry *Chatlog_EntryAt(int index)
{
	int oldest;

	if (index < 0)
		return NULL;

	if (index < sChatLogState.count) {
		oldest = (sChatLogState.head - sChatLogState.count + CHATLOG_CAP) % CHATLOG_CAP;
		return &sChatLogState.entries[(oldest + index) % CHATLOG_CAP];
	}

	if ((sChatLogState.flags & CHATLOG_FLAG_LINEOPEN) && index == sChatLogState.count)
		return Chatlog_LiveEntry();

	return NULL;
}

void Chatlog_BeginGlyphCapture(void)
{
	sChatlogUiState.capture = 1;
}

void Chatlog_EndGlyphCapture(void)
{
	sChatlogUiState.capture = 0;
}

void Chatlog_AppendUnicode(u32 unicod)
{
	if (!sChatlogUiState.capture)
		return;

	Chatlog_AppendGlyph(unicod);
}

void Chatlog_AppendPrintedSpan(const char *start, const char *end)
{
	u32 unicod;
	int decode_len;

	(void)end;
	if (!start || !*start)
		return;

	if (DecodeUtf8(start, &unicod, &decode_len) != 0)
		return;

	Chatlog_AppendGlyph(unicod);
}

void Chatlog_AppendPrinted(const char *str)
{
	int len;

	if (!str || !*str)
		return;

	len = GetChLenUtf8(str);
	if (len <= 0)
		len = 1;

	Chatlog_AppendPrintedSpan(str, str + len);
}

void Chatlog_AppendNewline(void)
{
	Chatlog_CommitPage();
}

void Chatlog_CommitPage(void)
{
	if (!(sChatLogState.flags & CHATLOG_FLAG_LINEOPEN))
		return;

	if (Chatlog_LiveEntry()->text[0] == '\0') {
		sChatLogState.flags &= ~CHATLOG_FLAG_LINEOPEN;
		return;
	}

	sChatLogState.flags &= ~CHATLOG_FLAG_LINEOPEN;
	sChatLogState.head = (sChatLogState.head + 1) % CHATLOG_CAP;
	if (sChatLogState.count < CHATLOG_CAP)
		sChatLogState.count++;

	if (sChatLogState.count > CHATLOG_VISIBLE)
		sChatLogState.viewTop = sChatLogState.count - CHATLOG_VISIBLE;
	else
		sChatLogState.viewTop = 0;

	if (Chatlog_IsVisible())
		Chatlog_Draw();
}

static void Chatlog_SanitizeState(void)
{
	if (sChatLogState.head >= CHATLOG_CAP)
		sChatLogState.head = 0;
	if (sChatLogState.count > CHATLOG_CAP)
		sChatLogState.count = CHATLOG_CAP;
	if (sChatLogState.viewTop >= CHATLOG_CAP)
		sChatLogState.viewTop = 0;
}

void Chatlog_StartSession(void)
{
	Chatlog_SanitizeState();
	sChatLogState.flags &= ~(CHATLOG_FLAG_VISIBLE | CHATLOG_FLAG_LINEOPEN);
	Proc_EndEach(ProcScr_Chatlog);
	Proc_Start(ProcScr_Chatlog, PROC_TREE_3);
}

void Chatlog_EndSession(void)
{
	Chatlog_CommitPage();

	if (Chatlog_IsVisible())
		Chatlog_Close();

	Proc_EndEach(ProcScr_Chatlog);
}

static void Chatlog_MoveTalkToBg1Bg2(void)
{
	/* Box was BG1, text was BG0. Move them so BG0 is free for the log. */
	CpuCopy16(gBG1TilemapBuffer, gBG2TilemapBuffer, 0x800);
	CpuCopy16(gBG0TilemapBuffer, gBG1TilemapBuffer, 0x800);
	BG_Fill(gBG0TilemapBuffer, 0);
	gLCDControlBuffer.bg2cnt.charBaseBlock = gLCDControlBuffer.bg1cnt.charBaseBlock;
	gLCDControlBuffer.bg2cnt.colorMode = 0;
	gLCDControlBuffer.bg2cnt.priority = 2;
	gLCDControlBuffer.dispcnt.bg2_on = 1;
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

static void Chatlog_RestoreTalkLayers(void)
{
	CpuCopy16(gBG1TilemapBuffer, gBG0TilemapBuffer, 0x800);
	CpuCopy16(gBG2TilemapBuffer, gBG1TilemapBuffer, 0x800);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

static u16 Chatlog_DimColor(u16 color)
{
	int r = (color >> 0) & 0x1F;
	int g = (color >> 5) & 0x1F;
	int b = (color >> 10) & 0x1F;

	return RGB(r >> 1, g >> 1, b >> 1);
}

static int Chatlog_ShouldDimPalette(int pal)
{
	/* Pal 2/3: Talk text + box. Pal 16-19: four OBJ minimug pals. */
	if (pal == 2 || pal == 3)
		return 0;

	if (pal >= 16 && pal <= 19)
		return 0;

	return 1;
}

static void Chatlog_BackupAndDimPalettes(void)
{
	int pal;
	int color;

	for (pal = 0; pal < CHATLOG_PAL_COUNT; pal++) {
		for (color = 0; color < 16; color++) {
			u16 value = gPaletteBuffer[pal * 16 + color];

			sChatlogUiState.palBackup[pal * 16 + color] = value;
			if (Chatlog_ShouldDimPalette(pal))
				gPaletteBuffer[pal * 16 + color] = Chatlog_DimColor(value);
		}
	}

	EnablePaletteSync();
}

static void Chatlog_RestorePalettes(void)
{
	int pal;
	int color;

	for (pal = 0; pal < CHATLOG_PAL_COUNT; pal++) {
		for (color = 0; color < 16; color++)
			gPaletteBuffer[pal * 16 + color] = sChatlogUiState.palBackup[pal * 16 + color];
	}

	EnablePaletteSync();
}

static void Chatlog_ClearDirtyBg0(void)
{
	int y;
	int x;

	for (y = 0; y < CHATLOG_DIRTY_H; y++) {
		for (x = 0; x < CHATLOG_DIRTY_W; x++)
			gBG0TilemapBuffer[TILEMAP_INDEX(CHATLOG_DIRTY_X + x, CHATLOG_DIRTY_Y + y)] = 0;
	}
}

static bool Chatlog_ChibiIsLz(const u8 *chibi)
{
	u32 size;

	if (!chibi || chibi[0] != 0x10)
		return false;

	size = chibi[1] | (chibi[2] << 8) | (chibi[3] << 16);
	return size > 0 && size <= 0x200;
}

static int Chatlog_EmbeddedChibiOffset(const struct FaceData *info)
{
	const u8 *img;
	const u8 *chibi;

	if (!info || !info->img || !info->imgChibi)
		return 0;

	img = info->img;
	chibi = info->imgChibi;
	if (chibi == img + 0x1624)
		return 0x1624;
	if (chibi == img + 0x2648)
		return 0x2648;
	if (chibi == img + 0x2644)
		return 0x2644;
	return 0;
}

static int Chatlog_PortraitIdForEntry(struct ChatLogEntry *entry, int charId)
{
	const struct CharacterData *character;
	int faceId;

	if (entry && entry->portraitId)
		return entry->portraitId;

	faceId = TextEngine_GetSpeakingFaceId();
	if (faceId > 0)
		return faceId;

	if (charId > 0) {
		character = GetCharacterData(charId);
		if (character && character->portraitId)
			return character->portraitId;
	}

	return charId;
}

static bool Chatlog_LoadLinearChibi(const struct FaceData *info)
{
	const u8 *chibi;

	if (!info)
		return false;

	chibi = info->imgChibi;
	if (!chibi)
		return false;

	/*
	 * Formatter blobs keep a raw 4×4 chibi at img+0x1624 / +0x2648.
	 * Vanilla mugs (Eirika/Seth) use a separate LZ chibi (header 10 00 02 00).
	 * Never decompress into a stack buffer; LZ needs 4-byte-aligned WRAM.
	 */
	if (Chatlog_EmbeddedChibiOffset(info)) {
		CpuCopy16(chibi, gGenericBuffer, 0x200);
		return true;
	}

	if (Chatlog_ChibiIsLz(chibi)) {
		Decompress(chibi, gGenericBuffer);
		return true;
	}

	if (chibi[0] == 0x10)
		return false;

	CpuCopy16(chibi, gGenericBuffer, 0x200);
	return true;
}

static void Chatlog_UploadChibiObj(const u8 *linear, int chr)
{
	CpuCopy16(linear + 0x00, OBJ_CHR_ADDR(chr + 0x00), 0x80);
	CpuCopy16(linear + 0x80, OBJ_CHR_ADDR(chr + 0x20), 0x80);
	CpuCopy16(linear + 0x100, OBJ_CHR_ADDR(chr + 0x04), 0x80);
	CpuCopy16(linear + 0x180, OBJ_CHR_ADDR(chr + 0x24), 0x80);
}

static void Chatlog_PutMinimug(int portraitId, int row)
{
	const struct FaceData *info;
	int chr;
	int pal;

	if (portraitId <= 0)
		return;

	info = GetPortraitData(portraitId);
	if (!info || !Chatlog_LoadLinearChibi(info))
		return;

	chr = CHATLOG_CHIBI_CHR + row * CHATLOG_CHIBI_CHR_STRIDE;
	pal = CHATLOG_CHIBI_PAL + row;
	if (info->pal)
		ApplyPalette(info->pal, 0x10 + pal);

	Chatlog_UploadChibiObj(gGenericBuffer, chr);
	sChatlogUiState.chibiMask |= (u8)(1 << row);
}

static void Chatlog_PutMinimugSprites(void)
{
	struct ChatLogEntry *entry;
	int row;
	int charId;
	int portraitId;
	int x;
	int y;
	int chr;
	int pal;

	x = CHATLOG_DIRTY_X * 8;
	for (row = 0; row < CHATLOG_VISIBLE; row++) {
		entry = Chatlog_EntryAt(sChatLogState.viewTop + row);
		if (!entry)
			break;

		charId = entry->charId;
		if (!charId &&
			(sChatLogState.flags & CHATLOG_FLAG_LINEOPEN) &&
			entry == Chatlog_LiveEntry())
			charId = Chatlog_CurrentSpeakerCharId();

		if (!(sChatlogUiState.chibiMask & (1 << row)))
			continue;

		portraitId = Chatlog_PortraitIdForEntry(entry, charId);
		if (portraitId <= 0)
			continue;

		y = (CHATLOG_DIRTY_Y + row * CHATLOG_ENTRY_H) * 8;
		chr = CHATLOG_CHIBI_CHR + row * CHATLOG_CHIBI_CHR_STRIDE;
		pal = CHATLOG_CHIBI_PAL + row;
		PutSprite(6, x, y, Sprite_ChatlogChibi, OAM2_CHR(chr) + OAM2_PAL(pal));
	}
}

static void Chatlog_BackupTalkWin0(void)
{
	sChatlogUiState.talkWin0Left = gLCDControlBuffer.win0_left;
	sChatlogUiState.talkWin0Top = gLCDControlBuffer.win0_top;
	sChatlogUiState.talkWin0Right = gLCDControlBuffer.win0_right;
	sChatlogUiState.talkWin0Bottom = gLCDControlBuffer.win0_bottom;
}

static void Chatlog_ApplyOverlayHw(void)
{
	/*
	 * WIN0 beats WIN1, so the log owns this rect and BG0 is not clipped.
	 * Keep BG1/BG2 on (and blended) so the Talk box and line show through
	 * empty BG0 pixels. BG0 stays out of blend A and stays opaque.
	 */
	SetWinEnable(1, 1, 0);
	SetWin0Box(
		CHATLOG_DIRTY_X * 8,
		CHATLOG_DIRTY_Y * 8,
		(CHATLOG_DIRTY_X + CHATLOG_DIRTY_W) * 8,
		(CHATLOG_DIRTY_Y + CHATLOG_DIRTY_H) * 8
	);
	SetWin1Box(
		sChatlogUiState.talkWin0Left,
		sChatlogUiState.talkWin0Top,
		sChatlogUiState.talkWin0Right,
		sChatlogUiState.talkWin0Bottom
	);
	SetWin0Layers(1, 1, 1, 1, 1);
	SetWin1Layers(0, 1, 1, 1, 1);
	SetWOutLayers(0, 0, 1, 1, 1);
	gLCDControlBuffer.dispcnt.bg2_on = 1;
	gLCDControlBuffer.wincnt.win0_enableBlend = 1;
	gLCDControlBuffer.wincnt.win1_enableBlend = 1;
	gLCDControlBuffer.wincnt.wout_enableBlend = 1;
	SetBlendTargetA(0, 1, 1, 0, 0);
	/* OBJ stays out of blend so the four minimugs keep their own palettes. */
	SetBlendTargetB(0, 0, 0, 1, 0);
	SetBlendBackdropB(1);
	SetBlendAlpha(10, 6);
}

static void Chatlog_RestoreOverlayHw(void)
{
	SetWinEnable(1, 0, 0);
	SetWin0Box(
		sChatlogUiState.talkWin0Left,
		sChatlogUiState.talkWin0Top,
		sChatlogUiState.talkWin0Right,
		sChatlogUiState.talkWin0Bottom
	);
	SetWin0Layers(1, 1, 1, 1, 1);
	SetWOutLayers(0, 1, 1, 1, 1);
	SetBlendTargetA(0, 1, 0, 0, 0);
	SetBlendTargetB(0, 0, 1, 1, 1);
	SetBlendBackdropB(1);
	gLCDControlBuffer.wincnt.win0_enableBlend = 1;
	gLCDControlBuffer.wincnt.wout_enableBlend = 1;
	SetBlendAlpha(16, 1);
}

static void Chatlog_Draw(void)
{
	void *vram;
	struct Text *nameText = &sChatlogUiState.texts[0];
	struct Text *bodyText = &sChatlogUiState.texts[1];
	struct ChatLogEntry *entry;
	int row;
	int y;
	int charId;
	u8 savedCapture = sChatlogUiState.capture;

	sChatlogUiState.capture = 0;
	sChatlogUiState.chibiMask = 0;
	gLCDControlBuffer.bg0cnt.colorMode = 0;
	Chatlog_ApplyOverlayHw();
	Chatlog_ClearDirtyBg0();

	vram = (void *)(
		VRAM
		+ GetBackgroundTileDataOffset(BG_0)
		+ CHATLOG_TEXT_CHR * CHR_SIZE
	);
	SetInitTalkTextFont();
	{
		struct Glyph **glyphs = gActiveFont->glyphs;
		void (*drawGlyph)(struct Text *, struct Glyph *) = gActiveFont->drawGlyph;
		void *(*get_draw_dest)(struct Text *) = gActiveFont->get_draw_dest;
		u8 lang = gActiveFont->lang;

		InitTextFont(&sChatlogUiState.font, vram, CHATLOG_TEXT_CHR, CHATLOG_TEXT_PAL);
		sChatlogUiState.font.glyphs = glyphs;
		sChatlogUiState.font.drawGlyph = drawGlyph;
		sChatlogUiState.font.get_draw_dest = get_draw_dest;
		sChatlogUiState.font.lang = lang;
		SetTextFont(&sChatlogUiState.font);
	}

	for (row = 0; row < CHATLOG_VISIBLE; row++) {
		entry = Chatlog_EntryAt(sChatLogState.viewTop + row);
		if (!entry)
			break;

		y = CHATLOG_DIRTY_Y + row * CHATLOG_ENTRY_H;

		entry->name[CHATLOG_NAME_LEN - 1] = '\0';
		entry->text[CHATLOG_TEXT_LEN - 1] = '\0';

		if (entry->name[0]) {
			InitText(nameText, CHATLOG_NAME_W);
			Text_SetColor(nameText, TEXT_COLOR_SYSTEM_GOLD);
			Text_DrawString(nameText, entry->name);
			PutText(nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, 6, y));
		}

		if (entry->text[0]) {
			InitText(bodyText, CHATLOG_BODY_W);
			Text_SetColor(bodyText, TEXT_COLOR_SYSTEM_WHITE);
			Text_DrawString(bodyText, entry->text);
			PutText(bodyText, TILEMAP_LOCATED(gBG0TilemapBuffer, 6, y + 2));
		}
	}

	for (row = 0; row < CHATLOG_VISIBLE; row++) {
		entry = Chatlog_EntryAt(sChatLogState.viewTop + row);
		if (!entry)
			break;

		charId = entry->charId;
		if (!charId &&
			(sChatLogState.flags & CHATLOG_FLAG_LINEOPEN) &&
			entry == Chatlog_LiveEntry())
			charId = Chatlog_CurrentSpeakerCharId();
		Chatlog_PutMinimug(Chatlog_PortraitIdForEntry(entry, charId), row);
	}

	EnablePaletteSync();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	Chatlog_PutMinimugSprites();
	sChatlogUiState.capture = savedCapture;
	SetInitTalkTextFont();
}

static void Chatlog_Open(void)
{
	int total;

	Chatlog_SanitizeState();
	Chatlog_BackupTalkWin0();
	Chatlog_MoveTalkToBg1Bg2();
	Chatlog_BackupAndDimPalettes();
	sChatLogState.flags |= CHATLOG_FLAG_VISIBLE;

	total = Chatlog_TotalLines();
	if (total > CHATLOG_VISIBLE)
		sChatLogState.viewTop = total - CHATLOG_VISIBLE;
	else
		sChatLogState.viewTop = 0;

	Chatlog_Draw();
}

static void Chatlog_Close(void)
{
	sChatLogState.flags &= ~CHATLOG_FLAG_VISIBLE;
	Chatlog_RestoreOverlayHw();
	Chatlog_RestorePalettes();
	Chatlog_RestoreTalkLayers();
	SetInitTalkTextFont();
}

static void Chatlog_OnIdle(ProcPtr proc)
{
	(void)proc;

	if (!Proc_Find(gProcScr_Talk)) {
		if (Chatlog_IsVisible())
			Chatlog_Close();
		return;
	}

	if (gKeyStatusPtr->newKeys & SELECT_BUTTON) {
		if (Chatlog_IsVisible())
			Chatlog_Close();
		else
			Chatlog_Open();
		return;
	}

	if (!Chatlog_IsVisible())
		return;

	if ((gKeyStatusPtr->repeatedKeys & DPAD_UP) && sChatLogState.viewTop > 0) {
		sChatLogState.viewTop--;
		Chatlog_Draw();
		return;
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		int maxTop = 0;
		int total = Chatlog_TotalLines();

		if (total > CHATLOG_VISIBLE)
			maxTop = total - CHATLOG_VISIBLE;

		if (sChatLogState.viewTop < maxTop) {
			sChatLogState.viewTop++;
			Chatlog_Draw();
			return;
		}
	}

	Chatlog_ApplyOverlayHw();
	Chatlog_PutMinimugSprites();
}

LYN_REPLACE_CHECK(TalkWaitForInput_OnIdle);
void TalkWaitForInput_OnIdle(struct Proc *proc)
{
	int frame = (GetGameClock() / 2) & 0xf;

	if (Chatlog_IsVisible())
		return;

	if (!CheckTalkFlag(TALK_FLAG_7))
		PutSprite(2, proc->unk64, proc->unk66, gPressKeyArrowSpriteLut[frame], 4);
	else
		PutSprite(0, proc->unk64, proc->unk66, gPressKeyArrowSpriteLut[frame], 0x0000B2BF);

	if (gKeyStatusPtr->newKeys & (DPAD_ANY | A_BUTTON | B_BUTTON))
		Proc_Break(proc);
}

LYN_REPLACE_CHECK(TalkSkipListener_OnIdle);
void TalkSkipListener_OnIdle(ProcPtr proc)
{
	struct TalkState *state = sTalkState;

	if (Chatlog_IsVisible())
		return;

	if (Proc_Find(gProcScr_TalkShiftClearAll) != NULL)
		return;

	if (Proc_Find(gProcScr_TalkShiftClear) != NULL)
		return;

	if (!CheckTalkFlag(TALK_FLAG_NOSKIP) && (gKeyStatusPtr->newKeys & (B_BUTTON | START_BUTTON))) {
		SetDialogueSkipEvBit();
		SetTalkFaceNoMouthMove(state->activeFaceSlot);

		Proc_End(proc);
		EndTalk();

		BG_Fill(gBG0TilemapBuffer, 0);
		BG_Fill(gBG1TilemapBuffer, 0);
		BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
		return;
	}

	if (Proc_Find(gProcScr_TalkWaitForInput) != NULL)
		return;

	if (!CheckTalkFlag(TALK_FLAG_NOFAST) && (gKeyStatusPtr->newKeys & (DPAD_ANY | A_BUTTON | B_BUTTON)))
		state->instantScroll = 1;
}
