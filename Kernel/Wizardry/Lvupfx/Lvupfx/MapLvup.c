#include "common-chax.h"
#include "strmag.h"
#include "lvup.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/class-pairs.h"
#include "constants/texts.h"

LYN_REPLACE_CHECK(GetManimLevelUpStatGain);
int GetManimLevelUpStatGain(int actor_id, int stat_num)
{
	switch (stat_num) {
	case 0:
		return (gManimSt.actor[actor_id].bu->expPrevious + gManimSt.actor[actor_id].bu->expGain) / 100;

	case 1:
		return gManimSt.actor[actor_id].bu->changeHP;

	case 2:
		return gManimSt.actor[actor_id].bu->changePow;

	case 3:
		return BU_CHG_MAG(gManimSt.actor[actor_id].bu);

	case 4:
		return gManimSt.actor[actor_id].bu->changeSkl;

	case 5:
		return gManimSt.actor[actor_id].bu->changeSpd;

	case 6:
		return gManimSt.actor[actor_id].bu->changeLck;

	case 7:
		return gManimSt.actor[actor_id].bu->changeDef;

	case 8:
		return gManimSt.actor[actor_id].bu->changeRes;

	case 9:
		return gManimSt.actor[actor_id].bu->changeCon;

	default:
		return 0;
	}
}

LYN_REPLACE_CHECK(GetManimLevelUpBaseStat);
int GetManimLevelUpBaseStat(int actor_id, int stat_num)
{
	struct Unit* unit = GetUnit(gManimSt.actor[actor_id].unit->index);

	switch (stat_num) {
	case 0:
		return gManimSt.actor[actor_id].bu->levelPrevious;

	case 1:
		return unit->maxHP;

	case 2:
		return unit->pow;

	case 3:
		return UNIT_MAG(unit);

	case 4:
		return unit->skl;

	case 5:
		return unit->spd;

	case 6:
		return unit->lck;

	case 7:
		return unit->def;

	case 8:
		return unit->res;

	default:
		return 0;
	}
}

struct MapLvupInfo {
	u8 x, y;
	u16 msg;
};

STATIC_DECLAR const struct MapLvupInfo NewMapLvupInfos[] = {
	{0x09, 0x00, 0x4E7}, /* Lv */

	{0x01, 0x04, 0x4E9}, /* HP */
	{0x01, 0x06, 0x4FE}, /* Str */
	{0x01, 0x08, 0x4FF}, /* Mag */
	{0x01, 0x0A, 0x4EC}, /* Skl */

	{0x09, 0x04, 0x4ED}, /* Spd */
	{0x09, 0x06, 0x4EE}, /* Lck */
	{0x09, 0x08, 0x4EF}, /* Def */
	{0x09, 0x0A, 0x4F0}, /* Res */

	{-1, -1, 0}
};

enum {
	MANIM_LVUP_FRAME_X = 1,
	MANIM_LVUP_FRAME_W = 18,
	MANIM_LVUP_HEADER_Y = 0,
	MANIM_LVUP_HEADER_H = 4,
	MANIM_LVUP_STATS_Y = 4,
	MANIM_LVUP_STATS_H = 10,
};

LYN_REPLACE_CHECK(PutManimLevelUpFrame);
void PutManimLevelUpFrame(int actor_id, int x, int y)
{
	int i;

	BG_Fill(gBG1TilemapBuffer, 0);

	LoadUiFrameGraphics();
	DrawUiFrame(gBG1TilemapBuffer, MANIM_LVUP_FRAME_X, MANIM_LVUP_HEADER_Y,
			MANIM_LVUP_FRAME_W, MANIM_LVUP_HEADER_H, 0, 0);
	DrawUiFrame(gBG1TilemapBuffer, MANIM_LVUP_FRAME_X, MANIM_LVUP_STATS_Y,
			MANIM_LVUP_FRAME_W, MANIM_LVUP_STATS_H, 0, 0);

	PutString(
		TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y),
		TEXT_COLOR_SYSTEM_WHITE,
		GetStringFromIndex(gManimSt.actor[actor_id].unit->pClassData->nameTextId));

	for (i = 0; NewMapLvupInfos[i].x != 0xFF; i++) {
		PutStringCentered(
			TILEMAP_LOCATED(gBG0TilemapBuffer, x + NewMapLvupInfos[i].x, y + NewMapLvupInfos[i].y),
			TEXT_COLOR_SYSTEM_GOLD, 3,
			GetStringFromIndex(NewMapLvupInfos[i].msg));
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT + BG1_SYNC_BIT);
}

LYN_REPLACE_CHECK(PutManimLevelUpStat);
void PutManimLevelUpStat(int actor_id, int x, int y, int stat_num, bool after_gain)
{
	PutNumberOrBlank(
		TILEMAP_LOCATED(gBG0TilemapBuffer, x + NewMapLvupInfos[stat_num].x + 4, y + NewMapLvupInfos[stat_num].y),
		TEXT_COLOR_SYSTEM_BLUE,
		GetManimLevelUpBaseStat(actor_id, stat_num) + (after_gain ? GetManimLevelUpStatGain(actor_id, stat_num) : 0));
}

LYN_REPLACE_CHECK(ManimLevelUp_ScrollOut);
void ManimLevelUp_ScrollOut(struct ManimLevelUpProc* proc)
{
	proc->y_scroll_offset -= 8;

	BG_SetPosition(BG_0, 0, proc->y_scroll_offset);
	BG_SetPosition(BG_1, 0, proc->y_scroll_offset);

	// NOTE: this is inconsistent with math in ManimLevelUp_InitMainScreen
	gFaces[0]->yPos = 32 - proc->y_scroll_offset;

	if (proc->y_scroll_offset <= -144)
	{
		if (gpKernelDesignerConfig->promotion_on_max_level == true) {
			if (gManimSt.actor[proc->actor_id].unit->level == UNIT_LEVEL_MAX_RE && !(UNIT_CATTRIBUTES(gManimSt.actor[proc->actor_id].unit) & CA_PROMOTED))
			{
				gActionData.subjectIndex = gManimSt.actor[proc->actor_id].unit->index;
				StartBmPromotion(proc);
				GetUnit(gActionData.subjectIndex)->level = 1;
				GetUnit(gActionData.subjectIndex)->exp = 0;
				gActionData.subjectIndex = 0; // Since this might be enemy phase, we don't want the unit to enter the position of the enemy unit
			}
		}
		if (gpKernelDesignerConfig->promote_trainees_in_chapter == true) {
			if ((UNIT_CATTRIBUTES(gManimSt.actor[proc->actor_id].unit) & CA_MAXLEVEL10) &&
				gManimSt.actor[proc->actor_id].unit->level >= 10)
			{
				gActionData.subjectIndex = gManimSt.actor[proc->actor_id].unit->index;
				StartBmPromotion(proc);
				gActionData.subjectIndex = 0; // Since this might be enemy phase, we don't want the unit to enter the position of the enemy unit
			}
		}
		Proc_Break(proc);
	}
}

typedef struct
{
	const int key;
	const int values[3];
} LevelUpStrings;

static const LevelUpStrings character_level_up_strings[] =
{
	{0},
	{ CHARACTER_EIRIKA,    {MSG_Eirika_Poor_Level_Up,    MSG_Eirika_Good_Level_Up,    MSG_Eirika_Great_Level_Up} },
	{ CHARACTER_SETH,      {MSG_Seth_Poor_Level_Up,      MSG_Seth_Good_Level_Up,      MSG_Seth_Great_Level_Up} },
	{ CHARACTER_GILLIAM,   {MSG_Gilliam_Poor_Level_Up,   MSG_Gilliam_Good_Level_Up,   MSG_Gilliam_Great_Level_Up} },
	{ CHARACTER_FRANZ,     {MSG_Franz_Poor_Level_Up,     MSG_Franz_Good_Level_Up,     MSG_Franz_Great_Level_Up} },
	{ CHARACTER_MOULDER,   {MSG_Moulder_Poor_Level_Up,   MSG_Moulder_Good_Level_Up,   MSG_Moulder_Great_Level_Up} },
	{ CHARACTER_VANESSA,   {MSG_Vanessa_Poor_Level_Up,   MSG_Vanessa_Good_Level_Up,   MSG_Vanessa_Great_Level_Up} },
	{ CHARACTER_ROSS,      {MSG_Ross_Poor_Level_Up,      MSG_Ross_Good_Level_Up,      MSG_Ross_Great_Level_Up} },
	{ CHARACTER_NEIMI,     {MSG_Neimi_Poor_Level_Up,     MSG_Neimi_Good_Level_Up,     MSG_Neimi_Great_Level_Up} },
	{ CHARACTER_COLM,      {MSG_Colm_Poor_Level_Up,      MSG_Colm_Good_Level_Up,      MSG_Colm_Great_Level_Up} },
	{ CHARACTER_GARCIA,    {MSG_Garcia_Poor_Level_Up,    MSG_Garcia_Good_Level_Up,    MSG_Garcia_Great_Level_Up} },
	{ CHARACTER_INNES,     {MSG_Innes_Poor_Level_Up,     MSG_Innes_Good_Level_Up,     MSG_Innes_Great_Level_Up} },
	{ CHARACTER_LUTE,      {MSG_Lute_Poor_Level_Up,      MSG_Lute_Good_Level_Up,      MSG_Lute_Great_Level_Up} },
	{ CHARACTER_NATASHA,   {MSG_Natasha_Poor_Level_Up,   MSG_Natasha_Good_Level_Up,   MSG_Natasha_Great_Level_Up} },
	{ CHARACTER_CORMAG,    {MSG_Cormag_Poor_Level_Up,    MSG_Cormag_Good_Level_Up,    MSG_Cormag_Great_Level_Up} },
	{ CHARACTER_EPHRAIM,   {MSG_Ephraim_Poor_Level_Up,   MSG_Ephraim_Good_Level_Up,   MSG_Ephraim_Great_Level_Up} },
	{ CHARACTER_FORDE,     {MSG_Forde_Poor_Level_Up,     MSG_Forde_Good_Level_Up,     MSG_Forde_Great_Level_Up} },
	{ CHARACTER_KYLE,      {MSG_Kyle_Poor_Level_Up,      MSG_Kyle_Good_Level_Up,      MSG_Kyle_Great_Level_Up} },
	{ CHARACTER_AMELIA,    {MSG_Amelia_Poor_Level_Up,    MSG_Amelia_Good_Level_Up,    MSG_Amelia_Great_Level_Up} },
	{ CHARACTER_ARTUR,     {MSG_Artur_Poor_Level_Up,     MSG_Artur_Good_Level_Up,     MSG_Artur_Great_Level_Up} },
	{ CHARACTER_GERIK,     {MSG_Gerik_Poor_Level_Up,     MSG_Gerik_Good_Level_Up,     MSG_Gerik_Great_Level_Up} },
	{ CHARACTER_TETHYS,    {MSG_Tethys_Poor_Level_Up,    MSG_Tethys_Good_Level_Up,    MSG_Tethys_Great_Level_Up} },
	{ CHARACTER_MARISA,    {MSG_Marisa_Poor_Level_Up,    MSG_Marisa_Good_Level_Up,    MSG_Marisa_Great_Level_Up} },
	{ CHARACTER_SALEH,     {MSG_Saleh_Poor_Level_Up,     MSG_Saleh_Good_Level_Up,     MSG_Saleh_Great_Level_Up} },
	{ CHARACTER_EWAN,      {MSG_Ewan_Poor_Level_Up,      MSG_Ewan_Good_Level_Up,      MSG_Ewan_Great_Level_Up} },
	{ CHARACTER_LARACHEL,  {MSG_Larachel_Poor_Level_Up,  MSG_Larachel_Good_Level_Up,  MSG_Larachel_Great_Level_Up} },
	{ CHARACTER_DOZLA,     {MSG_Dozla_Poor_Level_Up,     MSG_Dozla_Good_Level_Up,     MSG_Dozla_Great_Level_Up} },
	{ CHARACTER_ENEMY_1B,  {MSG_Enemy_1B_Poor_Level_Up,  MSG_Enemy_1B_Good_Level_Up,  MSG_Enemy_1B_Great_Level_Up} },
	{ CHARACTER_RENNAC,    {MSG_Rennac_Poor_Level_Up,    MSG_Rennac_Good_Level_Up,    MSG_Rennac_Great_Level_Up} },
	{ CHARACTER_DUESSEL,   {MSG_Duessel_Poor_Level_Up,   MSG_Duessel_Good_Level_Up,   MSG_Duessel_Great_Level_Up} },
	{ CHARACTER_MYRRH,     {MSG_Myrrh_Poor_Level_Up,     MSG_Myrrh_Good_Level_Up,     MSG_Myrrh_Great_Level_Up} },
	{ CHARACTER_KNOLL,     {MSG_Knoll_Poor_Level_Up,     MSG_Knoll_Good_Level_Up,     MSG_Knoll_Great_Level_Up} },
	{ CHARACTER_JOSHUA,    {MSG_Joshua_Poor_Level_Up,    MSG_Joshua_Good_Level_Up,    MSG_Joshua_Great_Level_Up} },
	{ CHARACTER_SYRENE,    {MSG_Syrene_Poor_Level_Up,    MSG_Syrene_Good_Level_Up,    MSG_Syrene_Great_Level_Up} },
	{ CHARACTER_TANA,      {MSG_Tana_Poor_Level_Up,      MSG_Tana_Good_Level_Up,      MSG_Tana_Great_Level_Up} },
	{ CHARACTER_LYON,      {MSG_Lyon_Poor_Level_Up,      MSG_Lyon_Good_Level_Up,      MSG_Lyon_Great_Level_Up} },
	{ CHARACTER_ORSON,     {MSG_Orson_Poor_Level_Up,     MSG_Orson_Good_Level_Up,     MSG_Orson_Great_Level_Up} },
	{ CHARACTER_ORSON_CH5X,{MSG_Orson_CH5X_Poor_Level_Up,MSG_Orson_CH5X_Good_Level_Up,MSG_Orson_CH5X_Great_Level_Up} },
	{ CHARACTER_GLEN,      {MSG_Glen_Poor_Level_Up,      MSG_Glen_Good_Level_Up,      MSG_Glen_Great_Level_Up} },
	{ CHARACTER_SELENA,    {MSG_Selena_Poor_Level_Up,    MSG_Selena_Good_Level_Up,    MSG_Selena_Great_Level_Up} },
	{ CHARACTER_VALTER,    {MSG_Valter_Poor_Level_Up,    MSG_Valter_Good_Level_Up,    MSG_Valter_Great_Level_Up} },
	{ CHARACTER_RIEV,      {MSG_Riev_Poor_Level_Up,      MSG_Riev_Good_Level_Up,      MSG_Riev_Great_Level_Up} },
	{ CHARACTER_CAELLACH,  {MSG_Caellach_Poor_Level_Up,  MSG_Caellach_Good_Level_Up,  MSG_Caellach_Great_Level_Up} },
	{ CHARACTER_FADO,      {MSG_Fado_Poor_Level_Up,      MSG_Fado_Good_Level_Up,      MSG_Fado_Great_Level_Up} },
	{ CHARACTER_ISMAIRE,   {MSG_Ismaire_Poor_Level_Up,   MSG_Ismaire_Good_Level_Up,   MSG_Ismaire_Great_Level_Up} },
	{ CHARACTER_HAYDEN,    {MSG_Hayden_Poor_Level_Up,    MSG_Hayden_Good_Level_Up,    MSG_Hayden_Great_Level_Up} },

};

void PutStringRightAligned(u16* tilemap, int color, int width, const char* str)
{
	struct Text tmp_text;
	struct Text* const text = &tmp_text;

	InitText(text, width);

	// Calculate the starting position for right alignment
	Text_SetCursor(text, 0);
	Text_SetColor(text, color);
	Text_DrawString(text, str);

	PutText(text, tilemap);

	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* Vanilla talk state pointer (static in the decomp). */
#define sTalkState (*(struct TalkState **)0x0859133C)

/*
 * Talk slot the level-up portrait is handed to. The quotes open with
 * [OpenMidRight] (slot 4), the closest slot to the portrait at x = 184, so
 * their [LoadFace] reuses this face instead of loading a second one.
 */
#define LVUP_SPEECH_TALK_FACE 4

/* Frames the finished quote stays up before closing (A/B closes it early). */
#define LVUP_SPEECH_HOLD_FRAMES 90

/* Level-up window offset once ManimLevelUp_ScrollIn has finished. */
#define LVUP_WINDOW_SHOWN_OFFSET (-48)

/*
 * The talk engine keeps BG0/BG1 at offset 0, so while the quote is up the
 * window is moved down this many tilemap rows and shown at offset 0 instead.
 * It lands on the same screen rows and frees rows 0-5 above it for the box.
 */
#define LVUP_WINDOW_SHIFT_ROWS (-LVUP_WINDOW_SHOWN_OFFSET / 8)

/*
 * One-line box drawn in the free rows above the window: box rows 1-4 and the
 * tail on rows 4-5. The quotes all use [BoxHeight_1].
 */
#define LVUP_SPEECH_LINES     1
#define LVUP_SPEECH_BOX_H     (LVUP_SPEECH_LINES * 2 + 2)
#define LVUP_SPEECH_TAIL_Y    4
#define LVUP_SPEECH_BOX_W_MAX 28

/* Normal text box graphics and palette, the same ones InitTalk loads. */
#define Img_LvupSpeechBox ((const void *)0x089E8238)
#define Pal_LvupSpeechBox ((const void *)0x089E84D4)
#define LVUP_SPEECH_BOX_PAL 3

enum {
	LVUP_LABEL_SCROLL_OUT = 1,
};

static int GetLevelUpSpeechMsg(struct ManimLevelUpProc *proc)
{
	int i, tier;
	int unitID = gManimSt.actor[proc->actor_id].unit->pCharacterData->number;

	/* JESTER - I'm doing this as a little patch for end of map BEXP. The procs intefere with each other and softlock the game so if we've beaten the map we return early */
	if (gEventSlots[EVT_SLOT_8] > 0)
		return 0;

	if (gEventSlots[EVT_SLOT_9] <= 2)
		tier = 0;
	else if (gEventSlots[EVT_SLOT_9] <= 5)
		tier = 1;
	else if (gEventSlots[EVT_SLOT_9] <= 8)
		tier = 2;
	else
		tier = 0;

	for (i = 1; i < (int)ARRAY_COUNT(character_level_up_strings); i++) {
		if (character_level_up_strings[i].key == unitID)
			return character_level_up_strings[i].values[tier];
	}

	return 0;
}

/* Skip the speech steps when this unit has nothing to say. */
static void LevelUpSpeech_Init(struct ManimLevelUpProc *proc)
{
	if (GetLevelUpSpeechMsg(proc) == 0)
		Proc_Goto(proc, LVUP_LABEL_SCROLL_OUT);
}

/* Move the level-up window down (rows > 0) or back up (rows < 0) in BG0/BG1. */
static void LevelUpSpeech_ShiftWindow(int rows)
{
	u16 *tilemaps[] = { gBG0TilemapBuffer, gBG1TilemapBuffer };
	int i, x, y, src;

	for (i = 0; i < (int)ARRAY_COUNT(tilemaps); i++) {
		u16 *tm = tilemaps[i];

		for (y = 0; y < 32; y++) {
			/* Walk against the shift so rows are read before they are overwritten. */
			int row = rows > 0 ? 31 - y : y;

			src = row - rows;

			for (x = 0; x < 32; x++)
				tm[TILEMAP_INDEX(x, row)] = (src >= 0 && src < 32) ? tm[TILEMAP_INDEX(x, src)] : 0;
		}
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void LevelUpSpeech_SetWindowOffset(struct ManimLevelUpProc *proc, int offset)
{
	proc->y_scroll_offset = offset;

	BG_SetPosition(BG_0, 0, offset);
	BG_SetPosition(BG_1, 0, offset);
}

/*
 * Same layout as vanilla PutTalkBubbleTm plus a right-side tail, but from a
 * chosen tile so the box graphics do not overwrite the UI frame tiles the
 * level-up window is drawn with.
 */
static void LevelUpSpeech_PutBox(int chr, int x, int y, int width, int height, int xTail, int yTail)
{
	u16 *tm = gBG1TilemapBuffer;
	int tile = TILEREF(chr, LVUP_SPEECH_BOX_PAL);
	int xMax = x + width - 1;
	int yMax = y + height - 1;
	int ix, iy;

	for (ix = x; ix < xMax; ix++) {
		tm[TILEMAP_INDEX(ix, y)] = tile + 1;
		tm[TILEMAP_INDEX(ix, yMax)] = tile + 1 + TILE_VFLIP;
	}

	for (iy = y; iy < yMax; iy++) {
		tm[TILEMAP_INDEX(x, iy)] = tile + 2;
		tm[TILEMAP_INDEX(xMax, iy)] = tile + 2 + TILE_HFLIP;
	}

	for (ix = x + 1; ix < xMax; ix++)
		for (iy = y + 1; iy < yMax; iy++)
			tm[TILEMAP_INDEX(ix, iy)] = tile + 3;

	tm[TILEMAP_INDEX(x, y)] = tile;
	tm[TILEMAP_INDEX(xMax, y)] = tile + TILE_HFLIP;
	tm[TILEMAP_INDEX(x, yMax)] = tile + TILE_VFLIP;
	tm[TILEMAP_INDEX(xMax, yMax)] = tile + TILE_HFLIP + TILE_VFLIP;

	tm[TILEMAP_INDEX(xTail, yTail)] = tile + 4;
	tm[TILEMAP_INDEX(xTail + 1, yTail)] = tile + 4 + TILE_HFLIP;
	tm[TILEMAP_INDEX(xTail, yTail + 1)] = tile + 5;
	tm[TILEMAP_INDEX(xTail + 1, yTail + 1)] = tile + 6;

	BG_EnableSyncByMask(BG1_SYNC_BIT);
}

/*
 * Speak the quote through the level-up portrait while the level-up window
 * stays on screen. The talk runs with TALK_FLAG_NOBUBBLE so the engine never
 * clears or scrolls BG0/BG1; the box is drawn here in the rows above the
 * window instead.
 */
static void LevelUpSpeech_StartTalk(struct ManimLevelUpProc *proc)
{
	struct TalkState *state = sTalkState;
	int fontChr, boxChr;
	int width, x, xTail;

	LevelUpSpeech_ShiftWindow(LVUP_WINDOW_SHIFT_ROWS);
	LevelUpSpeech_SetWindowOffset(proc, 0);

	/* Put the talk glyphs after the level-up text instead of over it. */
	fontChr = 0x80 + gActiveFont->chr_counter * 2;
	InitTalk(fontChr, LVUP_SPEECH_LINES, FALSE);

	/* gActiveFont is the talk font now; the box tiles go after its glyphs. */
	boxChr = fontChr + gActiveFont->chr_counter * 2;
	Decompress(Img_LvupSpeechBox, (void *)(VRAM + GetBackgroundTileDataOffset(BG_1) + boxChr * CHR_SIZE));
	CopyToPaletteBuffer(Pal_LvupSpeechBox, 0x20 * LVUP_SPEECH_BOX_PAL, 0x20);

	/* InitTalk clears the face refs, so hand the portrait over afterwards. */
	state->faces[LVUP_SPEECH_TALK_FACE] = gFaces[0];

	StartTalkMsgExt(0, 0, GetLevelUpSpeechMsg(proc), proc);
	SetTalkFlag(TALK_FLAG_NOSKIP);
	SetTalkFlag(TALK_FLAG_NOBUBBLE);

	/* StartTalkMsgExt sized the box for the whole quote; place it like vanilla's right-side bubble. */
	width = state->activeWidth;
	if (width > LVUP_SPEECH_BOX_W_MAX)
		width = LVUP_SPEECH_BOX_W_MAX;

	xTail = gFaces[0]->xPos / 8 - 5;

	if ((width + 1) / 2 + xTail >= 30)
		x = 29 - width;
	else
		x = xTail - width / 2;

	if (x < 0)
		x = 0;

	LevelUpSpeech_PutBox(boxChr, x, LVUP_SPEECH_TAIL_Y - LVUP_SPEECH_BOX_H + 1,
		width, LVUP_SPEECH_BOX_H, xTail, LVUP_SPEECH_TAIL_Y);

	state->xText = x + 1;
	state->yText = LVUP_SPEECH_TAIL_Y - LVUP_SPEECH_BOX_H + 2;

	proc->clock = 0;
}

static void LevelUpSpeech_Hold(struct ManimLevelUpProc *proc)
{
	proc->clock++;

	if (proc->clock >= LVUP_SPEECH_HOLD_FRAMES || (gKeyStatusPtr->newKeys & (A_BUTTON | B_BUTTON)))
		Proc_Break(proc);
}

static void LevelUpSpeech_EndTalk(struct ManimLevelUpProc *proc)
{
	ClearTalkFaceRefs();

	SetTextFont(NULL);
	InitSystemTextFont();

	/*
	 * Moving the window back up drops the box and text rows above it, and
	 * puts it back where ManimLevelUp_ScrollOut expects it.
	 */
	LevelUpSpeech_ShiftWindow(-LVUP_WINDOW_SHIFT_ROWS);
	LevelUpSpeech_SetWindowOffset(proc, LVUP_WINDOW_SHOWN_OFFSET);
}

const struct ProcCmd ProcScr_ManimLevelUp_UnitComment[] = {
	PROC_SET_END_CB(ManimLevelUp_Clear),
	PROC_SLEEP(1),
	PROC_CALL(InitManimLevelUpWindow),
	PROC_CALL(ManimLevelUp_DimBgm),
	PROC_YIELD,
	PROC_CALL(ManimLevelUp_StartLevelUpText),
	PROC_SLEEP(70),
	PROC_CALL(ManimLevelUp_EndLevelUpText),
	PROC_SLEEP(1),
	PROC_CALL(ManimLevelUp_RestoreBgm),
	PROC_YIELD,
	PROC_CALL(ManimLevelUp_InitMainScreen),
	PROC_YIELD,
	PROC_REPEAT(ManimLevelUp_ScrollIn),
	PROC_SLEEP(15),
	PROC_REPEAT(ManimLevelUp_PutStatGainLabels),
	PROC_SLEEP(45),
	PROC_CALL(EndManimLevelUpStatGainLabels),
	PROC_SLEEP(1),
	PROC_CALL(LevelUpSpeech_Init),
	PROC_CALL(LevelUpSpeech_StartTalk),
	PROC_REPEAT(LevelUpSpeech_Hold),
	PROC_CALL(LevelUpSpeech_EndTalk),
PROC_LABEL(LVUP_LABEL_SCROLL_OUT),
	PROC_REPEAT(ManimLevelUp_ScrollOut),
	PROC_CALL(ClearManimLevelUpWindow),
	PROC_CALL(ResetDialogueScreen),
	PROC_SLEEP(4),
	PROC_END,
};

LYN_REPLACE_CHECK(ManimLevelUp_InitMainScreen);
void ManimLevelUp_InitMainScreen(struct ManimLevelUpProc* proc)
{
	int i;

	if (gpKernelDesignerConfig->summons_gain_exp == true)
    {
		switch (gManimSt.actor[proc->actor_id].unit->pCharacterData->number) 
		{
			case CHARACTER_SUMMON_EWAN:
			case CHARACTER_SUMMON_LYON:
			case CHARACTER_SUMMON_KNOLL:
				gManimSt.actor[proc->actor_id].unit = GetUnit(gManimSt.actor[proc->actor_id].unit->ranks[ITYPE_STAFF]);
				break;
		}

		// gManimSt.actor[proc->actor_id].unit = GetUnit(CHARACTER_EIRIKA + 1);
		// gManimSt.actor[proc->actor_id].bu = &gBattleActor;
		// gManimSt.actor[proc->actor_id].mu = StartMu(GetUnit(CHARACTER_EIRIKA + 1));
	}

	ResetTextFont();
	BG_Fill(gBG0TilemapBuffer, 0);
	PutManimLevelUpFrame(proc->actor_id, 1, 1);

	for (i = 0; i < 9; i++)
		PutManimLevelUpStat(proc->actor_id, 1, 1, i, false);

	BG_EnableSyncByMask(BG0_SYNC_BIT);

	proc->next_stat_num = 0;
	proc->clock = 0;
	proc->y_scroll_offset = -144;

	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 1;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 2;

	SetDefaultColorEffects();
	SetWinEnable(0, 0, 0);

	/* Level up screen board */
	BG_SetPosition(BG_0, 0, proc->y_scroll_offset);
	/* Level up screen stats */
	BG_SetPosition(BG_1, 0, proc->y_scroll_offset);

	StartFace(
		0,
		gManimSt.actor[proc->actor_id].unit->pCharacterData->portraitId,
		184,
		32 - proc->y_scroll_offset,
		/*
		 * No FACE_DISP_BIT_12: that skips the blink/mouth procs, and the
		 * portrait needs them to talk during the level-up quote. Half-body
		 * mugs still get none (StartFace leaves them out).
		 */
		gpKernelDesignerConfig->half_body_portraits ? 0x0044 : 0x0042
	);

	gFaces[0]->yPos = 32 - proc->y_scroll_offset;

	// TODO: constants
	StartManimLevelUpStatGainLabels(0x200, 3, 1, proc);
}

LYN_REPLACE_CHECK(ResetDialogueScreen);
void ResetDialogueScreen(void) // function: MapLevelUp_EndFace
{
	ClearTalkBubble();
	Proc_EndEach(gProcScr_E_FACE);
	ResetFaces();
	ClearTalkFaceRefs();
}

LYN_REPLACE_CHECK(ManimLevelUp_PutStatGainLabels);
void ManimLevelUp_PutStatGainLabels(struct ManimLevelUpProc * proc)
{
    int stat_num;

    if (proc->clock != 0) {
        proc->clock--;
        return;
    }

    for (stat_num = proc->next_stat_num; stat_num < 9; stat_num++) {
        if (GetManimLevelUpStatGain(proc->actor_id, stat_num) != 0)
            break;
    }

    if (stat_num >= 9) {
        Proc_Break(proc);
        return;
    }

    PutManimLevelUpStat(proc->actor_id, 1, 1, stat_num, true);
    BG_EnableSyncByMask(BG0_SYNC_BIT);

    StartManimLevelUpStatGainLabelAnim(
        gManimLevelUpLabelInfoList[stat_num].x * 8 + 62,
        gManimLevelUpLabelInfoList[stat_num].y * 8 + 23 - proc->y_scroll_offset,
        stat_num, GetManimLevelUpStatGain(proc->actor_id, stat_num));

    if (stat_num == 0) {
        PlaySoundEffect(SONG_2CD);
    } else {
        PlaySoundEffect(SONG_76);
    }

    proc->next_stat_num = stat_num + 1;

	proc->clock = gpKernelDesignerConfig->stat_gain_frame_speed;
	// proc->clock = 20; <- old value
}