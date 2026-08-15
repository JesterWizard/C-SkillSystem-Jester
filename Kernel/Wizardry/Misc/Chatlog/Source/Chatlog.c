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

/*
 * VRAM budget, measured on a live map talk scene:
 *
 *   0x000-0x07F  dialogue box / frame graphics
 *   0x080-0x16F  talk font glyphs (3 lines * 30 cols + a 30 col nameplate)
 *   0x170-0x1FF  free
 *   0x200-0x3FF  unusable (BG3 image and the four tilemaps)
 *
 * The log needs 320 tiles, so it takes 0x0C0-0x1FF and stashes the talk
 * glyphs it overwrites (0x0C0-0x16F) in gGenericBuffer while it is open,
 * together with the BG0 tilemap it draws over.  The first 0x200 bytes are
 * left alone for graphics unpacking.
 */
enum {
	CHATLOG_CHIBI_CHR = 0x0C0,
	CHATLOG_CHIBI_CHR_STRIDE = 0x10,
	CHATLOG_TEXT_CHR = 0x100,
	/* Blank tile behind the panel, so the backdrop colour is ours alone. */
	CHATLOG_BLANK_CHR = 0x1F8,

	CHATLOG_VRAM_SAVE_CHR = CHATLOG_CHIBI_CHR,
	CHATLOG_VRAM_SAVE_SIZE = (0x170 - CHATLOG_CHIBI_CHR) * CHR_SIZE,
	CHATLOG_VRAM_SAVE_OFFSET = 0x200,
	CHATLOG_TM_SAVE_SIZE = 0x800,
	CHATLOG_TM_SAVE_OFFSET = CHATLOG_VRAM_SAVE_OFFSET + CHATLOG_VRAM_SAVE_SIZE,

	/* BG palettes. All of them are backed up, so any free slot works. */
	CHATLOG_TEXT_PAL = 8,
	CHATLOG_CHIBI_PAL = 4,
	CHATLOG_PAL_COUNT = 16,

	/* Tile coordinates inside the panel. */
	CHATLOG_CHIBI_X = 1,
	CHATLOG_TEXT_X = 6,
	CHATLOG_TOP_Y = 1,
	CHATLOG_NAME_W = 8,
	CHATLOG_BODY_W = 23,
	CHATLOG_BODY_PX = CHATLOG_BODY_W * 8,
	CHATLOG_NAME_PX = CHATLOG_NAME_W * 8,

	CHATLOG_NAME_BUF = 0x40,
	CHATLOG_BACKDROP = RGB(3, 4, 8),
};

#define sTalkState (*(struct TalkState **)0x0859133C)

struct ChatlogUiState {
	struct Font font;
	struct Text texts[CHATLOG_VISIBLE * 2];
	u16 palBackup[CHATLOG_PAL_COUNT * 16];
	u8 dispcntBackup[2];
	u8 bldcntBackup[2];
	u8 wincntBackup[4];
	u8 blendCoeffABackup;
	u8 blendCoeffBBackup;
	s16 bg0xBackup;
	s16 bg0yBackup;
	u8 capture;
	u8 hwSaved;
	char nameBuf[CHATLOG_NAME_BUF];
	char drawBuf[CHATLOG_TEXT_LEN];
};

extern struct ChatLogState sChatLogState;
extern struct ChatlogUiState sChatlogUiState;

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
static struct ChatLogEntry *Chatlog_LiveEntry(void);
static void Chatlog_EnsureLiveEntry(void);
static void Chatlog_FillSpeaker(struct ChatLogEntry *entry);

static const struct ProcCmd ProcScr_Chatlog[] = {
	PROC_NAME("Chatlog"),
	PROC_MARK(PROC_MARK_TALK),
	PROC_REPEAT(Chatlog_OnIdle),
	PROC_END,
};

_Static_assert(sizeof(struct ChatLogEntry) == 64, "ChatLogEntry size");
_Static_assert(sizeof(struct ChatLogState) == 0x444, "ChatLogState size");
_Static_assert(sizeof(struct ChatlogUiState) <= 0x600, "ChatlogUiState size");
_Static_assert(
	CHATLOG_TM_SAVE_OFFSET + CHATLOG_TM_SAVE_SIZE <= 0x2000,
	"chatlog VRAM stash overruns gGenericBuffer"
);
_Static_assert(
	CHATLOG_TEXT_CHR + CHATLOG_VISIBLE * 2 * (CHATLOG_NAME_W + CHATLOG_BODY_W)
		<= CHATLOG_BLANK_CHR,
	"chatlog glyph tiles overrun the blank tile"
);
_Static_assert(CHATLOG_BLANK_CHR < 0x200, "chatlog blank tile overruns BG3 graphics");

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

static int Chatlog_MaxViewTop(void)
{
	int total = Chatlog_TotalLines();

	if (total > CHATLOG_VISIBLE)
		return total - CHATLOG_VISIBLE;

	return 0;
}

static struct ChatLogEntry *Chatlog_LiveEntry(void)
{
	return &sChatLogState.entries[sChatLogState.head];
}

static struct ChatLogEntry *Chatlog_EntryAt(int index)
{
	int oldest;

	if (index < 0)
		return NULL;

	if (index < sChatLogState.count) {
		/* No modulo: this build has no division helper to link against. */
		oldest = sChatLogState.head - sChatLogState.count;
		if (oldest < 0)
			oldest += CHATLOG_CAP;

		oldest += index;
		if (oldest >= CHATLOG_CAP)
			oldest -= CHATLOG_CAP;

		return &sChatLogState.entries[oldest];
	}

	if ((sChatLogState.flags & CHATLOG_FLAG_LINEOPEN) && index == sChatLogState.count)
		return Chatlog_LiveEntry();

	return NULL;
}

static void Chatlog_ResetRing(void)
{
	CpuFill16(0, &sChatLogState, sizeof(sChatLogState));
}

void ChapterInit_ResetChatlog(void)
{
	Chatlog_ResetRing();
}

static void Chatlog_SanitizeState(void)
{
	if (sChatLogState.head >= CHATLOG_CAP)
		sChatLogState.head = 0;
	if (sChatLogState.count > CHATLOG_CAP)
		sChatLogState.count = CHATLOG_CAP;
	if (sChatLogState.viewTop > Chatlog_MaxViewTop())
		sChatLogState.viewTop = Chatlog_MaxViewTop();
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
	if (sChatLogState.count > CHATLOG_CAP || sChatLogState.head >= CHATLOG_CAP)
		Chatlog_ResetRing();
	else
		Chatlog_SanitizeState();
}

/* --- capture ------------------------------------------------------------- */

static int Chatlog_TextLen(const struct ChatLogEntry *entry)
{
	int len = 0;

	while (len < CHATLOG_TEXT_LEN - 1 && entry->text[len])
		len++;

	return len;
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

	entry->charId = Chatlog_CurrentSpeakerCharId();
	faceId = TextEngine_GetSpeakingFaceId();
	entry->portraitId = (faceId > 0) ? (u16)faceId : 0;
	entry->nameTextId = TextEngine_GetSpeakingNameTextId();

	if (!entry->charId)
		return;

	character = GetCharacterData(entry->charId);
	if (!character)
		return;

	if (!entry->portraitId && character->portraitId)
		entry->portraitId = character->portraitId;
	if (!entry->nameTextId)
		entry->nameTextId = character->nameTextId;
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

static void Chatlog_PushLiveEntry(void)
{
	sChatLogState.flags &= ~CHATLOG_FLAG_LINEOPEN;
	sChatLogState.head++;
	if (sChatLogState.head >= CHATLOG_CAP)
		sChatLogState.head = 0;
	if (sChatLogState.count < CHATLOG_CAP)
		sChatLogState.count++;

	sChatLogState.viewTop = Chatlog_MaxViewTop();
}

/*
 * The stored line is full: close it and reopen a continuation line for the
 * same speaker, moving any half-written word across so words stay intact.
 */
static void Chatlog_WrapLine(void)
{
	struct ChatLogEntry *entry = Chatlog_LiveEntry();
	char tail[CHATLOG_TEXT_LEN];
	u8 charId = entry->charId;
	u16 portraitId = entry->portraitId;
	u16 nameTextId = entry->nameTextId;
	int len = Chatlog_TextLen(entry);
	int cut = -1;
	int i;

	tail[0] = '\0';
	for (i = len - 1; i > 0; i--) {
		if (entry->text[i] == ' ') {
			cut = i;
			break;
		}
	}

	if (cut > 0 && len - cut - 1 > 0) {
		for (i = 0; i < len - cut - 1; i++)
			tail[i] = entry->text[cut + 1 + i];
		tail[i] = '\0';
		entry->text[cut] = '\0';
	}

	Chatlog_PushLiveEntry();

	entry = Chatlog_LiveEntry();
	CpuFill16(0, entry, sizeof(*entry));
	entry->charId = charId;
	entry->portraitId = portraitId;
	entry->nameTextId = nameTextId;
	entry->flags = CHATLOG_ENTRY_CONT;
	sChatLogState.flags |= CHATLOG_FLAG_LINEOPEN;

	for (i = 0; tail[i]; i++)
		entry->text[i] = tail[i];
	entry->text[i] = '\0';
	entry->width = GetStringTextLen(entry->text);
}

static void Chatlog_AppendGlyph(u32 unicod)
{
	struct ChatLogEntry *entry;
	char bytes[5];
	const char *next;
	u32 width;
	int len;
	int used;
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
	bytes[len] = '\0';

	width = 0;
	next = GetCharTextLen(bytes, &width);
	if (!next || width == 0 || width > 16)
		width = 8;

	/* A new speaker always starts its own entry, even mid-page. */
	if (sChatLogState.flags & CHATLOG_FLAG_LINEOPEN) {
		u8 speaker = Chatlog_CurrentSpeakerCharId();

		if (speaker && speaker != Chatlog_LiveEntry()->charId)
			Chatlog_CommitPage();
	}

	Chatlog_EnsureLiveEntry();
	entry = Chatlog_LiveEntry();
	used = Chatlog_TextLen(entry);

	/* Leading spaces on a wrapped line would look like a stray indent. */
	if (unicod == ' ' && used == 0 && (entry->flags & CHATLOG_ENTRY_CONT))
		return;

	if (used + len >= CHATLOG_TEXT_LEN - 1 || entry->width + width > CHATLOG_BODY_PX) {
		Chatlog_WrapLine();
		entry = Chatlog_LiveEntry();
		used = Chatlog_TextLen(entry);
		if (unicod == ' ' && used == 0)
			return;
		if (used + len >= CHATLOG_TEXT_LEN - 1)
			return;
	}

	for (i = 0; i < len; i++)
		entry->text[used + i] = bytes[i];
	entry->text[used + len] = '\0';
	entry->width += width;
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

/*
 * A line break inside a message is only a break in the dialogue box: the log
 * keeps one entry per message and re-wraps it to its own width, so the break
 * degrades to a word separator.
 */
void Chatlog_AppendSoftBreak(void)
{
	struct ChatLogEntry *entry;
	int used;

	if (!(sChatLogState.flags & CHATLOG_FLAG_LINEOPEN))
		return;

	entry = Chatlog_LiveEntry();
	used = Chatlog_TextLen(entry);

	if (used == 0 || entry->text[used - 1] == ' ')
		return;

	Chatlog_AppendGlyph(' ');
}

void Chatlog_CommitPage(void)
{
	if (!(sChatLogState.flags & CHATLOG_FLAG_LINEOPEN))
		return;

	if (Chatlog_LiveEntry()->text[0] == '\0') {
		sChatLogState.flags &= ~CHATLOG_FLAG_LINEOPEN;
		return;
	}

	Chatlog_PushLiveEntry();

	if (Chatlog_IsVisible())
		Chatlog_Draw();
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

/* --- hardware ------------------------------------------------------------ */

static void Chatlog_SaveTalkGfx(void)
{
	CpuFastCopy(
		BG_CHR_ADDR(CHATLOG_VRAM_SAVE_CHR),
		gGenericBuffer + CHATLOG_VRAM_SAVE_OFFSET,
		CHATLOG_VRAM_SAVE_SIZE
	);
	CpuFastCopy(
		gBG0TilemapBuffer,
		gGenericBuffer + CHATLOG_TM_SAVE_OFFSET,
		CHATLOG_TM_SAVE_SIZE
	);
}

static void Chatlog_RestoreTalkGfx(void)
{
	CpuFastCopy(
		gGenericBuffer + CHATLOG_VRAM_SAVE_OFFSET,
		BG_CHR_ADDR(CHATLOG_VRAM_SAVE_CHR),
		CHATLOG_VRAM_SAVE_SIZE
	);
	CpuFastCopy(
		gGenericBuffer + CHATLOG_TM_SAVE_OFFSET,
		gBG0TilemapBuffer,
		CHATLOG_TM_SAVE_SIZE
	);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void Chatlog_CopyBytes(u8 *dst, const u8 *src, int size)
{
	int i;

	for (i = 0; i < size; i++)
		dst[i] = src[i];
}

static void Chatlog_SaveHw(void)
{
	int i;

	Chatlog_CopyBytes(sChatlogUiState.dispcntBackup, (const u8 *)&gLCDControlBuffer.dispcnt, 2);
	Chatlog_CopyBytes(sChatlogUiState.bldcntBackup, (const u8 *)&gLCDControlBuffer.bldcnt, 2);
	Chatlog_CopyBytes(sChatlogUiState.wincntBackup, (const u8 *)&gLCDControlBuffer.wincnt, 4);
	sChatlogUiState.blendCoeffABackup = gLCDControlBuffer.blendCoeffA;
	sChatlogUiState.blendCoeffBBackup = gLCDControlBuffer.blendCoeffB;
	sChatlogUiState.bg0xBackup = gLCDControlBuffer.bgoffset[BG_0].x;
	sChatlogUiState.bg0yBackup = gLCDControlBuffer.bgoffset[BG_0].y;

	for (i = 0; i < CHATLOG_PAL_COUNT * 16; i++)
		sChatlogUiState.palBackup[i] = gPaletteBuffer[i];

	sChatlogUiState.hwSaved = 1;
}

static void Chatlog_RestoreHw(void)
{
	int i;

	if (!sChatlogUiState.hwSaved)
		return;

	Chatlog_CopyBytes((u8 *)&gLCDControlBuffer.dispcnt, sChatlogUiState.dispcntBackup, 2);
	Chatlog_CopyBytes((u8 *)&gLCDControlBuffer.bldcnt, sChatlogUiState.bldcntBackup, 2);
	Chatlog_CopyBytes((u8 *)&gLCDControlBuffer.wincnt, sChatlogUiState.wincntBackup, 4);
	gLCDControlBuffer.blendCoeffA = sChatlogUiState.blendCoeffABackup;
	gLCDControlBuffer.blendCoeffB = sChatlogUiState.blendCoeffBBackup;
	BG_SetPosition(BG_0, sChatlogUiState.bg0xBackup, sChatlogUiState.bg0yBackup);

	for (i = 0; i < CHATLOG_PAL_COUNT * 16; i++)
		gPaletteBuffer[i] = sChatlogUiState.palBackup[i];

	EnablePaletteSync();
	sChatlogUiState.hwSaved = 0;
}

/*
 * The panel borrows BG0, the layer the dialogue text itself uses, because the
 * map engine writes the other layers behind our back.  Every other layer is
 * switched off so nothing of the box, the map or the portraits shows through,
 * and the log needs no blending of its own.
 */
static void Chatlog_ApplyHw(void)
{
	SetDispEnable(1, 0, 0, 0, 0);
	SetWinEnable(0, 0, 0);
	SetBlendNone();
	BG_SetPosition(BG_0, 0, 0);

	/* The scene behind us keeps writing palette RAM, so restate ours. */
	ApplyPalette(Pal_Text, CHATLOG_TEXT_PAL);
	gPaletteBuffer[CHATLOG_TEXT_PAL * 16 + 15] = CHATLOG_BACKDROP;
	gPaletteBuffer[0] = CHATLOG_BACKDROP;
	EnablePaletteSync();
}

/* --- drawing ------------------------------------------------------------- */

/* Copy as much of str as fits in maxWidth pixels, so glyphs never spill out
 * of the tiles reserved for this line. */
static const char *Chatlog_ClipToWidth(const char *str, char *out, int outSize, int maxWidth)
{
	const char *cursor = str;
	int used = 0;
	int total = 0;

	out[0] = '\0';
	if (!str)
		return out;

	while (*cursor) {
		u32 width = 0;
		const char *next = GetCharTextLen(cursor, &width);
		int len;

		if (!next || next == cursor)
			break;

		len = next - cursor;
		if (width == 0 || width > 16)
			width = 8;
		if (total + (int)width > maxWidth)
			break;
		if (used + len >= outSize)
			break;

		while (cursor < next)
			out[used++] = *cursor++;

		total += width;
	}

	out[used] = '\0';
	return out;
}

static void Chatlog_SetupFont(void)
{
	struct Glyph **glyphs;
	void (*drawGlyph)(struct Text *, struct Glyph *);
	void *(*get_draw_dest)(struct Text *);
	u8 lang;

	/* Use the talk glyph set: it is the one that resolves this project's
	 * UTF-8 text, and it matches the widths measured while capturing. */
	SetInitTalkTextFont();
	glyphs = gActiveFont->glyphs;
	drawGlyph = gActiveFont->drawGlyph;
	get_draw_dest = gActiveFont->get_draw_dest;
	lang = gActiveFont->lang;

	InitTextFont(
		&sChatlogUiState.font,
		BG_CHR_ADDR(CHATLOG_TEXT_CHR),
		CHATLOG_TEXT_CHR,
		CHATLOG_TEXT_PAL
	);
	sChatlogUiState.font.glyphs = glyphs;
	sChatlogUiState.font.drawGlyph = drawGlyph;
	sChatlogUiState.font.get_draw_dest = get_draw_dest;
	sChatlogUiState.font.lang = lang;
	SetTextFont(&sChatlogUiState.font);
}

static const char *Chatlog_ResolveName(struct ChatLogEntry *entry)
{
	sChatlogUiState.nameBuf[0] = '\0';

	if (!entry->nameTextId)
		return sChatlogUiState.nameBuf;

	/*
	 * GetStringFromIndex decompresses into a shared buffer that the live
	 * dialogue string still points into, so always decode into our own.
	 */
	GetStringFromIndexInBuffer(entry->nameTextId, sChatlogUiState.nameBuf);
	sChatlogUiState.nameBuf[CHATLOG_NAME_BUF - 1] = '\0';
	return sChatlogUiState.nameBuf;
}

static void Chatlog_Draw(void)
{
	struct ChatLogEntry *entry;
	struct Text *text;
	int row;
	int y;
	u8 savedCapture = sChatlogUiState.capture;

	sChatlogUiState.capture = 0;

	/* Solid colour 15 of the text palette: the hardware backdrop colour is
	 * not ours to keep, the scene keeps rewriting it. */
	CpuFill16(0xFFFF, BG_CHR_ADDR(CHATLOG_BLANK_CHR), CHR_SIZE);
	BG_Fill(gBG0TilemapBuffer, TILEREF(CHATLOG_BLANK_CHR, CHATLOG_TEXT_PAL));
	Chatlog_ApplyHw();
	Chatlog_SetupFont();

	for (row = 0; row < CHATLOG_VISIBLE; row++) {
		entry = Chatlog_EntryAt(sChatLogState.viewTop + row);

		/* InitText only reserves tiles; without clearing them the talk
		 * glyphs that live in the same VRAM show through the gaps. */
		text = &sChatlogUiState.texts[row * 2];
		InitText(text, CHATLOG_NAME_W);
		ClearText(text);
		text = &sChatlogUiState.texts[row * 2 + 1];
		InitText(text, CHATLOG_BODY_W);
		ClearText(text);

		if (!entry)
			continue;

		y = CHATLOG_TOP_Y + row * CHATLOG_ENTRY_H;
		entry->text[CHATLOG_TEXT_LEN - 1] = '\0';

		if (!(entry->flags & CHATLOG_ENTRY_CONT)) {
			const char *name = Chatlog_ResolveName(entry);

			if (entry->portraitId) {
				PutFaceChibi(
					entry->portraitId,
					TILEMAP_LOCATED(gBG0TilemapBuffer, CHATLOG_CHIBI_X, y),
					CHATLOG_CHIBI_CHR + row * CHATLOG_CHIBI_CHR_STRIDE,
					CHATLOG_CHIBI_PAL + row,
					0
				);
			}

			if (name[0]) {
				text = &sChatlogUiState.texts[row * 2];
				Text_SetColor(text, TEXT_COLOR_SYSTEM_GOLD);
				Text_DrawString(
					text,
					Chatlog_ClipToWidth(
						name,
						sChatlogUiState.drawBuf,
						sizeof(sChatlogUiState.drawBuf),
						CHATLOG_NAME_PX
					)
				);
				PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, CHATLOG_TEXT_X, y));
			}
		}

		if (entry->text[0]) {
			/* A wrapped line sits directly under the line it continues. */
			int bodyY = (entry->flags & CHATLOG_ENTRY_CONT) ? y : y + 2;

			text = &sChatlogUiState.texts[row * 2 + 1];
			Text_SetColor(text, TEXT_COLOR_SYSTEM_WHITE);
			Text_DrawString(
				text,
				Chatlog_ClipToWidth(
					entry->text,
					sChatlogUiState.drawBuf,
					sizeof(sChatlogUiState.drawBuf),
					CHATLOG_BODY_PX
				)
			);
			PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, CHATLOG_TEXT_X, bodyY));
		}
	}

	EnablePaletteSync();
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	SetInitTalkTextFont();
	sChatlogUiState.capture = savedCapture;
}

static void Chatlog_Open(void)
{
	Chatlog_SanitizeState();
	Chatlog_SaveTalkGfx();
	Chatlog_SaveHw();
	Chatlog_ApplyHw();

	sChatLogState.flags |= CHATLOG_FLAG_VISIBLE;
	sChatLogState.viewTop = Chatlog_MaxViewTop();

	Chatlog_Draw();
}

static void Chatlog_Close(void)
{
	sChatLogState.flags &= ~CHATLOG_FLAG_VISIBLE;
	Chatlog_RestoreTalkGfx();
	Chatlog_RestoreHw();
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

	if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN) &&
		sChatLogState.viewTop < Chatlog_MaxViewTop()) {
		sChatLogState.viewTop++;
		Chatlog_Draw();
		return;
	}

	/* Other scenes keep poking the display registers, so keep claiming them. */
	Chatlog_ApplyHw();
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
