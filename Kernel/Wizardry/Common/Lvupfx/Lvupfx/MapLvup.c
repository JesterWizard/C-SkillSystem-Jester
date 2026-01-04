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

LYN_REPLACE_CHECK(PutManimLevelUpFrame);
void PutManimLevelUpFrame(int actor_id, int x, int y)
{
	int i;

	BG_Fill(gBG1TilemapBuffer, 0);

	/* Background level up box image */
	Decompress(Img_LevelUpBoxFrame, (void*)VRAM + GetBackgroundTileDataOffset(1) + 0x200 * CHR_SIZE);
	Decompress(Tsa_LevelUpBoxFrame, gGenericBuffer);
	PutTmLinear((void*)gGenericBuffer, (void*)gBG1TilemapBuffer, 0x20 * 0x1C, TILEREF(0x200, BGPAL_MANIM_INFO));
	ApplyPalette(Pal_LevelUpBoxFrame, BGPAL_MANIM_INFO);

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
		if (gpKernelDesigerConfig->promotion_on_max_level == true) {
			if (gManimSt.actor[proc->actor_id].unit->level == UNIT_LEVEL_MAX_RE && !(UNIT_CATTRIBUTES(gManimSt.actor[proc->actor_id].unit) & CA_PROMOTED))
			{
				gActionData.subjectIndex = gManimSt.actor[proc->actor_id].unit->index;
				StartBmPromotion(proc);
				GetUnit(gActionData.subjectIndex)->level = 1;
				GetUnit(gActionData.subjectIndex)->exp = 0;
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

LYN_REPLACE_CHECK(StartManimLevelUp);
void StartManimLevelUp(int actor_id, ProcPtr parent)
{
	struct ManimLevelUpProc* proc;

if (gpKernelDesigerConfig->talk_on_level_up == true) {
	proc = Proc_StartBlocking(ProcScr_ManimLevelUp_CUSTOM, parent);
}
else {
	proc = Proc_StartBlocking(ProcScr_ManimLevelUp, parent);
}

	proc->actor_id = actor_id;
}

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

static const EventScr EventScr_LevelUpSpeech[] = {
    EVBIT_MODIFY(0x4) // Can't skip scene
    TEXTSTART
    TEXTSHOW(0xFFFF)
    TEXTEND
    REMA
    NOFADE
    ENDA
};

void DisplayCharacterSpeech(struct ManimLevelUpProc* proc)
{
	/* JESTER - I'm doing this as a little patch for end of map BEXP. The procs intefere with each other and softlock the game so if we've won we return early */
	if (gEventSlots[EVT_SLOT_8] > 0)
		return;

	int message = -1;
	int unitID = gManimSt.actor[proc->actor_id].unit->pCharacterData->number;

	if (gEventSlots[EVT_SLOT_7] <= 2)
		message = 0;
	else if (gEventSlots[EVT_SLOT_7] <= 5)
		message = 1;
	else if (gEventSlots[EVT_SLOT_7] <= 8)
		message = 2;
	else 
		message = 0;

	const int unit_dialogue_label = character_level_up_strings[unitID].values[message];

	/* Switch out the face for a talking one when the dialogue is triggered */
	gEventSlots[EVT_SLOT_2] = unit_dialogue_label;
	ResetFaces();
	EndManimLevelUpStatGainLabels();
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG1_SYNC_BIT);
	// StartFace(0, gManimSt.actor[proc->actor_id].unit->pCharacterData->portraitId, 184, 32 - proc->y_scroll_offset, FACE_DISP_KIND(FACE_96x80) | FACE_DISP_TALK_1);
	KernelCallEvent(EventScr_LevelUpSpeech, EV_EXEC_CUTSCENE, proc);
	// DrawUiFrame(gBG1TilemapBuffer, 30 - (GetStringTextLen(unit_dialogue) / 8) - 4, 28, (GetStringTextLen(unit_dialogue) / 8) + 3, 4, 0, 1);
	// PutStringRightAligned(TILEMAP_LOCATED(gBG0TilemapBuffer, (30 - (GetStringTextLen(unit_dialogue) / 8)) - 3, 29), TEXT_COLOR_SYSTEM_WHITE, (GetStringTextLen(unit_dialogue) / 8) + 2, unit_dialogue);
};

const struct ProcCmd ProcScr_ManimLevelUp_CUSTOM[] = {
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
	PROC_CALL(DisplayCharacterSpeech), /* My character speech insertion */
	PROC_CALL(EndManimLevelUpStatGainLabels),
	PROC_SLEEP(1),
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

	// #ifdef CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS
	//    switch (proc->actor_id) {
	// 	case CHARACTER_SUMMON_EWAN:
	// 	   for (int i = 0; i < ARRAY_COUNT(gNewSummonConfig); i ++)
	// 	   {
	// 		 if (proc->actor_id == gNewSummonConfig[i][1])
	// 		 {
	// 		    proc->actor_id = gNewSummonConfig[i][0];
	// 			break;
	// 		 }
	// 	   }
	//    }

	//    gManimSt.actor[proc->actor_id].unit = GetUnit(CHARACTER_EIRIKA + 1);
	//    //InitBattleUnit(&gBattleActor, GetUnit(CHARACTER_EIRIKA + 1));
	//    gManimSt.actor[proc->actor_id].bu = &gBattleActor;
	//    gManimSt.actor[proc->actor_id].mu = StartMu(GetUnit(CHARACTER_EIRIKA + 1));
	// #endif

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

	StartFace(0, gManimSt.actor[proc->actor_id].unit->pCharacterData->portraitId, 184, 32 - proc->y_scroll_offset, 0x1042);

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