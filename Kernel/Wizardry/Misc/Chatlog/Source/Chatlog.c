#include "common-chax.h"
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
#include "constants/faces.h"
#include "constants/characters.h"

enum {
	/* Keep all overlay tiles below 0x200: 224-color Talk BG3 gfx starts at VRAM 0x4000. */
	CHATLOG_TEXT_CHR = 0x40,
	CHATLOG_TEXT_PAL = 2,
	CHATLOG_CHIBI_CHR = 0x1C0,
	CHATLOG_CHIBI_PAL = 12,
	CHATLOG_DIRTY_X = 1,
	CHATLOG_DIRTY_Y = 1,
	CHATLOG_DIRTY_W = 21,
	CHATLOG_DIRTY_H = 4,
	CHATLOG_PAL_COUNT = 32,
};

#define sTalkState (*(struct TalkState **)0x0859133C)

struct ChatlogUiState {
	struct Font font;
	struct Text texts[2];
	u16 palBackup[CHATLOG_PAL_COUNT * 16];
	u16 bg0Backup[CHATLOG_DIRTY_W * CHATLOG_DIRTY_H];
	u8 talkWin0Left;
	u8 talkWin0Top;
	u8 talkWin0Right;
	u8 talkWin0Bottom;
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
static void Chatlog_GetSpeaker(u16 *faceIdOut, u16 *nameIdOut);
static struct ChatLogEntry *Chatlog_LiveEntry(void);

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

static const struct CharacterData *Chatlog_FindCharacterByPortrait(int portraitId)
{
	const struct CharacterData *character;
	int characterId;

	for (characterId = 1; characterId < 0x100; characterId++) {
		character = GetCharacterData(characterId);
		if (!character || !character->nameTextId)
			continue;
		if (character->portraitId == portraitId)
			return character;
	}

	return NULL;
}

static void Chatlog_GetSpeaker(u16 *faceIdOut, u16 *nameIdOut)
{
	struct TalkState *state = sTalkState;
	struct FaceProc *face;
	const struct CharacterData *character;
	int slot;
	u16 faceId = 0;
	u16 nameId = 0;

	if (!state) {
		*faceIdOut = 0;
		*nameIdOut = 0;
		return;
	}

	slot = state->speakingFaceSlot;
	if (slot < 0)
		slot = state->activeFaceSlot;

	if (slot >= 0 && slot < (int)ARRAY_COUNT(state->faces)) {
		face = state->faces[slot];
		if (face) {
			faceId = face->faceId;
			character = Chatlog_FindCharacterByPortrait(faceId);
			if (character)
				nameId = character->nameTextId;
		}
	}

	if (faceId == 0 && UNIT_IS_VALID(gActiveUnit) && gActiveUnit->pCharacterData) {
		faceId = GetUnitPortraitId(gActiveUnit);
		nameId = gActiveUnit->pCharacterData->nameTextId;
	}

	*faceIdOut = faceId;
	*nameIdOut = nameId;
}

static void Chatlog_EnsureLiveEntry(void)
{
	struct ChatLogEntry *entry;

	if (sChatLogState.flags & CHATLOG_FLAG_LINEOPEN)
		return;

	entry = Chatlog_LiveEntry();
	CpuFill16(0, entry, sizeof(*entry));
	Chatlog_GetSpeaker(&entry->faceId, &entry->nameTextId);
	sChatLogState.flags |= CHATLOG_FLAG_LINEOPEN;
}

static void Chatlog_EnsureDrawnLine(void)
{
	struct ChatLogEntry *entry;
	const char *fallback = "...";
	int i;

	if (sChatLogState.count > 0)
		return;

	Chatlog_EnsureLiveEntry();
	entry = Chatlog_LiveEntry();
	if (entry->text[0] != '\0')
		return;

	for (i = 0; fallback[i] && i < CHATLOG_TEXT_LEN - 1; i++)
		entry->text[i] = fallback[i];
	entry->text[i] = '\0';
}

void Chatlog_AppendPrinted(const char *str)
{
	struct ChatLogEntry *entry;
	int len;
	int copy;
	int used;

	if (!str || !*str)
		return;

	if ((u8)*str < 0x20 || (u8)*str == 0x80)
		return;

	len = GetChLenUtf8(str);
	if (len <= 0)
		len = 1;

	Chatlog_EnsureLiveEntry();
	entry = Chatlog_LiveEntry();
	used = 0;
	while (entry->text[used])
		used++;

	if (used + len >= CHATLOG_TEXT_LEN - 1)
		return;

	for (copy = 0; copy < len; copy++)
		entry->text[used + copy] = str[copy];

	entry->text[used + len] = '\0';
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

static void Chatlog_BackupPanel(void)
{
	int y;
	int x;

	for (y = 0; y < CHATLOG_DIRTY_H; y++) {
		for (x = 0; x < CHATLOG_DIRTY_W; x++) {
			sChatlogUiState.bg0Backup[y * CHATLOG_DIRTY_W + x] =
				gBG0TilemapBuffer[TILEMAP_INDEX(CHATLOG_DIRTY_X + x, CHATLOG_DIRTY_Y + y)];
		}
	}
}

static void Chatlog_RestorePanel(void)
{
	int y;
	int x;

	for (y = 0; y < CHATLOG_DIRTY_H; y++) {
		for (x = 0; x < CHATLOG_DIRTY_W; x++) {
			gBG0TilemapBuffer[TILEMAP_INDEX(CHATLOG_DIRTY_X + x, CHATLOG_DIRTY_Y + y)] =
				sChatlogUiState.bg0Backup[y * CHATLOG_DIRTY_W + x];
		}
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT);
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
	/* Pal 2/3: Talk text + BG1 box. Pal 12-15: free BG banks for the minimug. */
	if (pal == 2 || pal == 3)
		return 0;

	if (pal >= 12 && pal <= 15)
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

static bool Chatlog_ChibiIsEmbedded(const struct FaceData *info)
{
	const u8 *img;
	const u8 *chibi;

	if (!info || !info->img || !info->imgChibi)
		return false;

	img = info->img;
	chibi = (const u8 *)info->imgChibi;
	return chibi == img + 0x2648 || chibi == img + 0x2644;
}

static void Chatlog_PutChibiTm(u16 *tm, int chr, int pal)
{
	int i;
	int j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			tm[TILEMAP_INDEX(j, i)] = TILEREF(chr + i * 4 + j, pal);
	}
}

static void Chatlog_PutSethMinimug(void)
{
	const struct CharacterData *cd;
	const struct FaceData *info;
	u16 *tm = TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 1);
	int fid;

	cd = GetCharacterData(CHARACTER_SETH);
	info = (cd && cd->portraitId) ? GetPortraitData(cd->portraitId) : NULL;

	if (info && info->pal && Chatlog_ChibiIsEmbedded(info)) {
		ApplyPalette(info->pal, CHATLOG_CHIBI_PAL);
		Copy2dChr(
			(const void *)info->imgChibi,
			(void *)(VRAM + CHATLOG_CHIBI_CHR * CHR_SIZE),
			4,
			4
		);
		Chatlog_PutChibiTm(tm, CHATLOG_CHIBI_CHR, CHATLOG_CHIBI_PAL);
		EnablePaletteSync();
		return;
	}

	fid = (cd && cd->portraitId) ? cd->portraitId : FID_FACTION_CHIBI;
	PutFaceChibi(fid, tm, CHATLOG_CHIBI_CHR, CHATLOG_CHIBI_PAL, 0);
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
	SetBlendTargetB(0, 0, 0, 1, 1);
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

	Chatlog_ApplyOverlayHw();
	Chatlog_ClearDirtyBg0();
	Chatlog_PutSethMinimug();

	vram = (void *)(
		VRAM
		+ GetBackgroundTileDataOffset(BG_0)
		+ CHATLOG_TEXT_CHR * CHR_SIZE
	);
	InitTextFont(&sChatlogUiState.font, vram, CHATLOG_TEXT_CHR, CHATLOG_TEXT_PAL);
	SetTextFont(&sChatlogUiState.font);
	SetTextFontGlyphs(TEXT_GLYPHS_SYSTEM);

	InitText(nameText, 8);
	Text_SetColor(nameText, TEXT_COLOR_SYSTEM_GOLD);
	Text_DrawString(nameText, "Seth");
	PutText(nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 1));

	InitText(bodyText, 16);
	Text_SetColor(bodyText, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(bodyText, "Princess Eirika");
	PutText(bodyText, TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3));

	BG_EnableSyncByMask(BG0_SYNC_BIT);
	SetTextFont(NULL);
}

static void Chatlog_Open(void)
{
	int total;

	Chatlog_SanitizeState();
	Chatlog_EnsureDrawnLine();
	Chatlog_BackupPanel();
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
	Chatlog_RestorePanel();
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
