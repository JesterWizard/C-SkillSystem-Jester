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
	/* 30 */ struct Unit *unit;
	/* 34 */ struct Unit *saved_active_unit;
	/* 38 */ struct BattleUnit *bu;
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
	case STAT_POINT_LCK: return UNIT_LCK_MAX(unit) + limitBreaker;
	case STAT_POINT_DEF: return UNIT_DEF_MAX(unit) + limitBreaker;
	case STAT_POINT_RES: return UNIT_RES_MAX(unit) + limitBreaker;
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

	if (proc->points == 0 || (proc->allocated & STAT_POINT_ALL_MASK) == STAT_POINT_ALL_MASK)
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

static int GetPendingStatPoints(struct BattleUnit *bu)
{
	int levels;

	if (!bu)
		return 0;

	levels = bu->unit.level - bu->levelPrevious;
	if (levels < 1)
		return 0;

	return levels * gpKernelDesignerConfig->lvup_stat_points;
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

static void LvupStatPoints_Init(struct ProcLvupStatPoints *proc)
{
	gActiveUnit = proc->unit;

	InitSystemTextFont();
	LoadUiFrameGraphics();
	StartMenuAt(&sStatPointsMenuDef, sStatPointsMenuDef.rect, proc);
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
	int points;

	if (!gpKernelDesignerConfig->lvup_stat_points)
		return;

	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE || !bu)
		return;

	points = GetPendingStatPoints(bu);
	if (points <= 0)
		return;

	if (ekr_mode)
		proc = Proc_Start(ProcScr_LvupStatPoints, PROC_TREE_3);
	else
		proc = Proc_StartBlocking(ProcScr_LvupStatPoints, parent);

	proc->finished = false;
	proc->ekr_mode = ekr_mode;
	proc->points = points > 0xFF ? 0xFF : points;
	proc->allocated = 0;
	proc->unit = unit;
	proc->saved_active_unit = gActiveUnit;
	proc->bu = bu;

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

LYN_REPLACE_CHECK(NewEkrLevelup);
void NewEkrLevelup(struct Anim *ais)
{
	struct BattleUnit *bu;
	struct Unit *unit;

	if (!gpKernelDesignerConfig->lvup_stat_points) {
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

	if (UNIT_IS_VALID(unit) && UNIT_FACTION(unit) == FACTION_BLUE && GetPendingStatPoints(bu) > 0) {
		StartLvupStatPointsMenuExt(unit, bu, NULL, true);
		return;
	}

	StartSkippedEkrLevelup(ais);
}

LYN_REPLACE_CHECK(StartManimLevelUp);
void StartManimLevelUp(int actor_id, ProcPtr parent)
{
	struct ManimLevelUpProc *proc;

	if (gpKernelDesignerConfig->lvup_stat_points) {
		StartLvupStatPointsMenu(gManimSt.actor[actor_id].unit, gManimSt.actor[actor_id].bu, parent);
		return;
	}

	if (gpKernelDesignerConfig->talk_on_level_up == true)
		proc = Proc_StartBlocking(ProcScr_ManimLevelUp_UnitComment, parent);
	else
		proc = Proc_StartBlocking(ProcScr_ManimLevelUp, parent);

	proc->actor_id = actor_id;
}
