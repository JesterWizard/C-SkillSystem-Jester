#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "strmag.h"
#include "lvup.h"
#include "bmmenu.h"
#include "ekrbattle.h"
#include "ekrlevelup.h"
#include "anime.h"
#include "fontgrp.h"
#include "uiutils.h"
#include "mapanim.h"
#include "jester_headers/custom-arrays.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

struct ProcLvupStatPoints {
	PROC_HEADER;
	/* 29 */ u8 finished;
	/* 2A */ u8 ekr_mode;
	/* 2B */ u8 points;
	/* 2C */ u8 allocated;
	/* 2D */ u8 timing;
	/* 2E */ u8 delay;
	/* 2F */ u8 phase[8];
	/* 38 */ struct Unit *unit;
	/* 3C */ struct Unit *saved_active_unit;
	/* 40 */ struct BattleUnit *bu;
};

enum {
	STAT_POINT_HP,
	STAT_POINT_POW,
	STAT_POINT_MAG,
	STAT_POINT_SKL,
	STAT_POINT_SPD,
	STAT_POINT_LCK,
	STAT_POINT_DEF,
	STAT_POINT_RES,
	STAT_POINT_COUNT,
};

#define STAT_POINT_NUMBER_X 7
#define STAT_POINT_ALL_MASK ((1 << STAT_POINT_COUNT) - 1)
#define TIMING_FRAME_X 12
#define TIMING_FRAME_Y 0
#define TIMING_FRAME_W 12
#define TIMING_FRAME_H (STAT_POINT_COUNT + 2)
#define TIMING_COL_X 13
#define TIMING_BAR_W 10
#define TIMING_PERIOD 200
#define TIMING_SPEED 3
#define TIMING_END_DELAY 60

static struct ProcLvupStatPoints *GetStatPointsProc(struct MenuProc *menu)
{
	return menu->proc_parent;
}

static int GetStatPointLimitBreaker(struct Unit *unit)
{
	int limitBreaker = 0;

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
	if (SkillTester(unit, SID_LimitBreaker))
		limitBreaker = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
	if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
		limitBreaker = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

	return limitBreaker;
}

static int GetStatPointValue(struct Unit *unit, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:  return unit->maxHP;
	case STAT_POINT_POW: return unit->pow;
	case STAT_POINT_MAG: return UNIT_MAG(unit);
	case STAT_POINT_SKL: return unit->skl;
	case STAT_POINT_SPD: return unit->spd;
	case STAT_POINT_LCK: return unit->lck;
	case STAT_POINT_DEF: return unit->def;
	case STAT_POINT_RES: return unit->res;
	default:             return 0;
	}
}

static int GetStatPointChange(struct BattleUnit *bu, int stat)
{
	if (!bu)
		return 0;

	switch (stat) {
	case STAT_POINT_HP:  return bu->changeHP;
	case STAT_POINT_POW: return bu->changePow;
	case STAT_POINT_MAG: return BU_CHG_MAG(bu);
	case STAT_POINT_SKL: return bu->changeSkl;
	case STAT_POINT_SPD: return bu->changeSpd;
	case STAT_POINT_LCK: return bu->changeLck;
	case STAT_POINT_DEF: return bu->changeDef;
	case STAT_POINT_RES: return bu->changeRes;
	default:             return 0;
	}
}

static int GetStatPointDisplay(struct ProcLvupStatPoints *proc, int stat)
{
	return GetStatPointValue(proc->unit, stat) + GetStatPointChange(proc->bu, stat);
}

static int GetStatPointCap(struct Unit *unit, int stat)
{
	int limitBreaker = GetStatPointLimitBreaker(unit);

	switch (stat) {
	case STAT_POINT_HP:  return unit->pClassData->maxHP + limitBreaker;
	case STAT_POINT_POW: return UNIT_POW_MAX(unit) + limitBreaker;
	case STAT_POINT_MAG: return GetUnitMaxMagic(unit) + limitBreaker;
	case STAT_POINT_SKL: return UNIT_SKL_MAX(unit) + limitBreaker;
	case STAT_POINT_SPD: return UNIT_SPD_MAX(unit) + limitBreaker;
	case STAT_POINT_RES: return UNIT_RES_MAX(unit) + limitBreaker;
	case STAT_POINT_LCK: return UNIT_LCK_MAX(unit) + limitBreaker;
	case STAT_POINT_DEF: return UNIT_DEF_MAX(unit) + limitBreaker;
	default:             return 0;
	}
}

static void ApplyStatPoint(struct BattleUnit *bu, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:
		bu->changeHP++;
		bu->unit.curHP++;
		break;
	case STAT_POINT_POW: bu->changePow++;      break;
	case STAT_POINT_MAG: BU_CHG_MAG(bu)++;     break;
	case STAT_POINT_SKL: bu->changeSkl++;      break;
	case STAT_POINT_SPD: bu->changeSpd++;      break;
	case STAT_POINT_LCK: bu->changeLck++;      break;
	case STAT_POINT_DEF: bu->changeDef++;      break;
	case STAT_POINT_RES: bu->changeRes++;      break;
	}
}

static bool StatPointIsAllocated(struct ProcLvupStatPoints *proc, int stat)
{
	return proc->allocated & (1 << stat);
}

static void MarkStatPointAllocated(struct ProcLvupStatPoints *proc, int stat)
{
	proc->allocated |= (1 << stat);

	if (!proc->timing && (proc->points == 0 || (proc->allocated & STAT_POINT_ALL_MASK) == STAT_POINT_ALL_MASK))
		proc->allocated = 0;
}

static bool StatPointHasSpendableStat(struct ProcLvupStatPoints *proc)
{
	int stat;

	for (stat = 0; stat < STAT_POINT_COUNT; stat++) {
		if (StatPointIsAllocated(proc, stat))
			continue;

		if (GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat))
			continue;

		return true;
	}

	return false;
}

static int GetPendingLevels(struct BattleUnit *bu)
{
	int levels;

	if (!bu)
		return 0;

	levels = bu->unit.level - bu->levelPrevious;
	if (levels < 1)
		return 0;

	return levels;
}

static int GetPendingStatPoints(struct BattleUnit *bu)
{
	return GetPendingLevels(bu) * gpKernelDesignerConfig->lvup_stat_points;
}

static void DrawStatPointsRow(struct MenuProc *menu, struct MenuItemProc *item, int labelColor, int numColor, int number)
{
	u16 *tm = TILEMAP_LOCATED(BG_GetMapBuffer(menu->frontBg), item->xTile, item->yTile);

	ClearText(&item->text);
	Text_SetColor(&item->text, labelColor);
	Text_DrawString(&item->text, item->def->name);
	PutText(&item->text, tm);

	tm[STAT_POINT_NUMBER_X - 1] = 0;
	tm[STAT_POINT_NUMBER_X - 2] = 0;
	PutNumber(tm + STAT_POINT_NUMBER_X, numColor, number);
}

static int StatPointsMenu_DrawPoints(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);

	DrawStatPointsRow(menu, item, TEXT_COLOR_SYSTEM_GOLD, TEXT_COLOR_SYSTEM_BLUE, proc->points);
	return 0;
}

static int StatPointsMenu_DrawStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber - 1;
	int value = GetStatPointDisplay(proc, stat);
	bool capped = value >= GetStatPointCap(proc->unit, stat);
	bool locked = capped || StatPointIsAllocated(proc, stat);

	DrawStatPointsRow(menu, item,
		locked ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
		capped ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE,
		value);

	return 0;
}

static u8 StatPointsMenu_OnSelectPoints(struct MenuProc *menu, struct MenuItemProc *item)
{
	return MENU_ACT_SND6B;
}

static u8 StatPointsMenu_OnSelectStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber - 1;

	if (proc->points == 0)
		return MENU_ACT_SND6B;

	if (GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat))
		return MENU_ACT_SND6B;

	if (StatPointIsAllocated(proc, stat))
		return MENU_ACT_SND6B;

	ApplyStatPoint(proc->bu, stat);
	proc->points--;
	MarkStatPointAllocated(proc, stat);

	if (proc->points == 0)
		return (MenuCancelSelect(menu, item) & ~MENU_ACT_SND6B) | MENU_ACT_SND6A;

	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

#define STAT_POINT_ROW(label) \
	{label, 0, MSG_MenuCommand_Allocate_DESC, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, StatPointsMenu_DrawStat, StatPointsMenu_OnSelectStat, 0, 0, 0}

static void StatPointsMenu_OnInit(struct MenuProc *menu)
{
	menu->itemCurrent = 1;
}

static u8 StatPointsMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);

	if (proc->points == 0 || !StatPointHasSpendableStat(proc))
		return MenuCancelSelect(menu, item);

	return MENU_ACT_SND6B;
}

static const struct MenuItemDef sStatPointsMenuItems[] = {
	{" Points", 0, MSG_MenuCommand_Allocate_DESC, TEXT_COLOR_SYSTEM_GOLD, 0, MenuAlwaysEnabled, StatPointsMenu_DrawPoints, StatPointsMenu_OnSelectPoints, 0, 0, 0},
	STAT_POINT_ROW(" HP"),
	STAT_POINT_ROW(" Str"),
	STAT_POINT_ROW(" Mag"),
	STAT_POINT_ROW(" Skl"),
	STAT_POINT_ROW(" Spd"),
	STAT_POINT_ROW(" Lck"),
	STAT_POINT_ROW(" Def"),
	STAT_POINT_ROW(" Res"),
	MenuItemsEnd
};

static const struct MenuDef sStatPointsMenuDef = {
	{1, 0, 10, 0},
	0,
	sStatPointsMenuItems,
	StatPointsMenu_OnInit, 0, 0,
	StatPointsMenu_OnCancel,
	MenuAutoHelpBoxSelect,
	MenuStdHelpBox
};

static int GetTimingMeter(struct ProcLvupStatPoints *proc, int stat)
{
	int p = proc->phase[stat];

	if (p < 100)
		return p;

	return TIMING_PERIOD - p;
}

static int GetTimingWindow(void)
{
	int window = gpKernelDesignerConfig->lvup_stat_timing;

	if (window < 1)
		window = 1;
	if (window > 99)
		window = 99;

	return window;
}

static bool TimingInWindow(struct ProcLvupStatPoints *proc, int stat)
{
	return GetTimingMeter(proc, stat) >= (100 - GetTimingWindow());
}

static void AdvanceTimingPhases(struct ProcLvupStatPoints *proc)
{
	int stat;

	for (stat = 0; stat < STAT_POINT_COUNT; stat++) {
		if (StatPointIsAllocated(proc, stat))
			continue;

		proc->phase[stat] += TIMING_SPEED;
		if (proc->phase[stat] >= TIMING_PERIOD)
			proc->phase[stat] -= TIMING_PERIOD;
	}
}

static void DrawTimingColumnFrame(struct MenuProc *menu)
{
	int y;
	int h;

	if (menu->itemCount < 1)
		return;

	y = menu->menuItems[0]->yTile - 1;
	h = menu->menuItems[menu->itemCount - 1]->yTile - y + 3;

	DrawUiFrame(
		BG_GetMapBuffer(menu->backBg),
		TIMING_FRAME_X, y, TIMING_FRAME_W, h,
		menu->tileref, 0);
}

static void DrawTimingBar(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	u16 *tm = TILEMAP_LOCATED(BG_GetMapBuffer(menu->frontBg), TIMING_COL_X, item->yTile);
	int stat = item->itemNumber;
	int i;
	int color;
	int pos;
	bool locked = StatPointIsAllocated(proc, stat);
	bool capped = GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat);
	bool hit = GetStatPointChange(proc->bu, stat) > 0;

	for (i = 0; i < TIMING_BAR_W; i++)
		PutSpecialChar(tm + i, TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_DASH);

	if (capped && !locked)
		return;

	pos = GetTimingMeter(proc, stat) * (TIMING_BAR_W - 1) / 99;

	if (locked)
		color = hit ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GOLD;
	else
		color = TimingInWindow(proc, stat) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GOLD;

	PutSpecialChar(tm + pos, color, TEXT_SPECIAL_ARROW);
}

static int StatTimingMenu_DrawStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber;
	int value = GetStatPointDisplay(proc, stat);
	bool capped = value >= GetStatPointCap(proc->unit, stat);
	bool locked = StatPointIsAllocated(proc, stat);
	bool hit = GetStatPointChange(proc->bu, stat) > 0;

	DrawStatPointsRow(menu, item,
		locked && !hit ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
		(capped || hit) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE,
		value);

	DrawTimingBar(menu, item);
	return 0;
}

static void TimingClose_Now(ProcPtr proc)
{
	EndMenu(((struct Proc *)proc)->proc_parent);
}

static const struct ProcCmd ProcScr_TimingClose[] = {
	PROC_SLEEP(TIMING_END_DELAY),
	PROC_CALL(TimingClose_Now),
	PROC_END,
};

static u8 StatTimingMenu_OnIdle(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int i;

	(void)item;
	DrawTimingColumnFrame(menu);
	AdvanceTimingPhases(proc);

	for (i = 0; i < menu->itemCount; i++)
		DrawTimingBar(menu, menu->menuItems[i]);

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	return 0;
}

static void StatTimingSelectNextRow(struct MenuProc *menu)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int start = menu->itemCurrent;
	int i;

	for (i = 1; i < menu->itemCount; i++) {
		int idx = start + i;
		int stat;

		if (idx >= menu->itemCount)
			idx -= menu->itemCount;

		stat = menu->menuItems[idx]->itemNumber;
		if (StatPointIsAllocated(proc, stat))
			continue;

		if (GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat))
			continue;

		menu->itemPrevious = menu->itemCurrent;
		menu->itemCurrent = idx;
		return;
	}
}

static u8 StatTimingMenu_OnSelectStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber;

	if (StatPointIsAllocated(proc, stat))
		return MENU_ACT_SND6B;

	if (GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat))
		return MENU_ACT_SND6B;

	if (TimingInWindow(proc, stat))
		ApplyStatPoint(proc->bu, stat);

	MarkStatPointAllocated(proc, stat);

	if (!StatPointHasSpendableStat(proc)) {
		if (proc->delay == 0) {
			proc->delay = 1;
			Proc_Start(ProcScr_TimingClose, menu);
		}

		RedrawMenu(menu);
		return MENU_ACT_SND6A;
	}

	StatTimingSelectNextRow(menu);
	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

static void StatTimingMenu_OnInit(struct MenuProc *menu)
{
	menu->itemCurrent = 0;
}

static void StatTimingMenu_OnEnd(struct MenuProc *menu)
{
	int y;
	int h;

	if (menu->itemCount < 1)
		return;

	y = menu->menuItems[0]->yTile - 1;
	h = menu->menuItems[menu->itemCount - 1]->yTile - y + 3;
	ClearUiFrame(BG_GetMapBuffer(menu->backBg), TIMING_FRAME_X, y, TIMING_FRAME_W, h);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static u8 StatTimingMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;
	(void)item;
	return MENU_ACT_SND6B;
}

#define STAT_TIMING_ROW(label) \
	{label, 0, MSG_MenuCommand_Timing_DESC, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, StatTimingMenu_DrawStat, StatTimingMenu_OnSelectStat, StatTimingMenu_OnIdle, 0, 0}

static const struct MenuItemDef sStatTimingMenuItems[] = {
	STAT_TIMING_ROW(" HP"),
	STAT_TIMING_ROW(" Str"),
	STAT_TIMING_ROW(" Mag"),
	STAT_TIMING_ROW(" Skl"),
	STAT_TIMING_ROW(" Spd"),
	STAT_TIMING_ROW(" Lck"),
	STAT_TIMING_ROW(" Def"),
	STAT_TIMING_ROW(" Res"),
	MenuItemsEnd
};

static const struct MenuDef sStatTimingMenuDef = {
	{1, 0, 10, STAT_POINT_COUNT + 2},
	0,
	sStatTimingMenuItems,
	StatTimingMenu_OnInit, StatTimingMenu_OnEnd, 0,
	StatTimingMenu_OnCancel,
	MenuAutoHelpBoxSelect,
	MenuStdHelpBox
};

static void LvupStatPoints_Init(struct ProcLvupStatPoints *proc)
{
	gActiveUnit = proc->unit;

	InitSystemTextFont();
	LoadUiFrameGraphics();

	if (proc->timing) {
		struct MenuProc *menu = StartMenuAt(&sStatTimingMenuDef, sStatTimingMenuDef.rect, proc);

		DrawTimingColumnFrame(menu);
	} else {
		StartMenuAt(&sStatPointsMenuDef, sStatPointsMenuDef.rect, proc);
	}
}

static void LvupStatPoints_WaitMenu(struct ProcLvupStatPoints *proc)
{
	if (proc->proc_child == NULL)
		Proc_Break(proc);
}

static void LvupStatPoints_OnMenuDone(struct ProcLvupStatPoints *proc)
{
	gActiveUnit = proc->saved_active_unit;

	if (proc->ekr_mode) {
		proc->finished = true;
		Proc_Goto(proc, 1);
	}
}

static const struct ProcCmd ProcScr_LvupStatPoints[] = {
	PROC_NAME("LvupStatPoints"),
	PROC_YIELD,
	PROC_CALL(LvupStatPoints_Init),
	PROC_REPEAT(LvupStatPoints_WaitMenu),
	PROC_CALL(LvupStatPoints_OnMenuDone),
	PROC_END,

	PROC_LABEL(1),
	PROC_BLOCK,
};

static void StartLvupStatPointsMenuExt(struct Unit *unit, struct BattleUnit *bu, ProcPtr parent, bool ekr_mode)
{
	struct ProcLvupStatPoints *proc;
	bool timing = gpKernelDesignerConfig->lvup_stat_timing != 0;
	int i;
	int points = 0;

	if (!KernelLvupReplacesGrowths())
		return;

	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE || !bu)
		return;

	if (timing) {
		if (GetPendingLevels(bu) <= 0)
			return;
	} else {
		points = GetPendingStatPoints(bu);
		if (points <= 0)
			return;
	}

	if (ekr_mode)
		proc = Proc_Start(ProcScr_LvupStatPoints, PROC_TREE_3);
	else
		proc = Proc_StartBlocking(ProcScr_LvupStatPoints, parent);

	proc->finished = false;
	proc->ekr_mode = ekr_mode;
	proc->points = points > 0xFF ? 0xFF : points;
	proc->allocated = 0;
	proc->timing = timing;
	proc->delay = 0;
	proc->unit = unit;
	proc->saved_active_unit = gActiveUnit;
	proc->bu = bu;

	for (i = 0; i < STAT_POINT_COUNT; i++)
		proc->phase[i] = i * 25;

	if (ekr_mode)
		gpProcEkrLevelup = (void *)proc;
}

void StartLvupStatPointsMenu(struct Unit *unit, struct BattleUnit *bu, ProcPtr parent)
{
	StartLvupStatPointsMenuExt(unit, bu, parent, false);
}

static void StartSkippedEkrLevelup(struct Anim *ais)
{
	struct ProcEkrLevelup *proc = Proc_Start(ProcScr_EkrLevelup, PROC_TREE_3);

	gpProcEkrLevelup = proc;
	proc->ais_main = ais;
	proc->ais_core = GetAnimAnotherSide(ais);
	proc->timer = 0;
	proc->finished = true;
	proc->is_promotion = false;
}

static bool BattleUnitCanOpenLvupMenu(struct BattleUnit *bu, struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return false;

	if (gpKernelDesignerConfig->lvup_stat_timing)
		return GetPendingLevels(bu) > 0;

	return GetPendingStatPoints(bu) > 0;
}

LYN_REPLACE_CHECK(NewEkrLevelup);
void NewEkrLevelup(struct Anim *ais)
{
	struct BattleUnit *bu;
	struct Unit *unit;

	if (!KernelLvupReplacesGrowths()) {
		struct ProcEkrLevelup *proc = Proc_Start(ProcScr_EkrLevelup, PROC_TREE_3);

		gpProcEkrLevelup = proc;
		proc->ais_main = ais;
		proc->ais_core = GetAnimAnotherSide(ais);
		proc->timer = 0;
		proc->finished = false;
		proc->is_promotion = false;
		return;
	}

	bu = (GetAnimPosition(ais) == EKR_POS_L) ? gpEkrBattleUnitLeft : gpEkrBattleUnitRight;
	unit = bu ? GetUnit(bu->unit.index) : NULL;

	if (BattleUnitCanOpenLvupMenu(bu, unit)) {
		StartLvupStatPointsMenuExt(unit, bu, NULL, true);
		return;
	}

	StartSkippedEkrLevelup(ais);
}

LYN_REPLACE_CHECK(StartManimLevelUp);
void StartManimLevelUp(int actor_id, ProcPtr parent)
{
	struct ManimLevelUpProc *proc;

	if (KernelLvupReplacesGrowths()) {
		StartLvupStatPointsMenu(gManimSt.actor[actor_id].unit, gManimSt.actor[actor_id].bu, parent);
		return;
	}

	if (gpKernelDesignerConfig->talk_on_level_up == true)
		proc = Proc_StartBlocking(ProcScr_ManimLevelUp_UnitComment, parent);
	else
		proc = Proc_StartBlocking(ProcScr_ManimLevelUp, parent);

	proc->actor_id = actor_id;
}
