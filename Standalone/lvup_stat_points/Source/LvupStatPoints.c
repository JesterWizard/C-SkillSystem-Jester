#include "global.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "uimenu.h"
#include "fontgrp.h"
#include "uiutils.h"
#include "hardware.h"
#include "proc.h"
#include "ekrbattle.h"
#include "ekrlevelup.h"
#include "efxbattle.h"
#include "anime.h"
#include "mapanim.h"
#include "variables.h"
#include "bm.h"

/*
 * Level-up stat points / timing minigame (standalone port of the
 * C Skill System lvup_stat_points / lvup_stat_timing flags).
 *
 * Player (blue) units do not roll growths: when the level-up screen would
 * show, the rolled gains are discarded and a menu appears instead:
 *  - mode 0 (points): spend LvupStatPointsConfig.points per level, at most
 *    one point per stat per round.
 *  - mode 1 (timing): each stat has a moving cursor over a bar; green bars
 *    come from the growth rate (10% = 1 bar). Press A while the cursor is on
 *    green to gain +1. Growths of 100% or more get another pass.
 * The gains are written to the BattleUnit's change* fields, so vanilla
 * applies them to the unit after the battle, like normal growths.
 * Other units level up normally.
 */

struct LvupStatPointsConfig {
	u8 mode;   /* 0 = spend points, 1 = timing minigame */
	u8 points; /* points per level (mode 0) */
};

extern const struct LvupStatPointsConfig gLvupStatPointsConfig;

struct ProcLvupStatPoints {
	PROC_HEADER;
	/* 29 */ u8 finished; /* same offset as ProcEkrLevelup::finished */
	/* 2A */ u8 ekr_mode; /* deferred until the battle animation ends */
	/* 2B */ u8 points;
	/* 2C */ u8 allocated;
	/* 2D */ u8 timing;
	/* 2E */ u8 delay;
	/* 2F */ u8 phase[8];
	/* 38 */ struct Unit *unit;
	/* 3C */ struct Unit *saved_active_unit;
	/* 40 */ struct BattleUnit *bu;
	/* 44 */ u8 remaining[8];
};

enum {
	STAT_POINT_HP,
	STAT_POINT_POW,
	STAT_POINT_SKL,
	STAT_POINT_SPD,
	STAT_POINT_LCK,
	STAT_POINT_DEF,
	STAT_POINT_RES,
	STAT_POINT_COUNT,
};

#define STAT_POINT_NUMBER_X 7
#define TIMING_FRAME_X 12
#define TIMING_FRAME_W 12
#define TIMING_COL_X 13
#define TIMING_BAR_W 10
#define TIMING_PERIOD 100 /* one left-to-right pass, then wrap */
#define TIMING_SPEED 3
#define TIMING_END_DELAY 60

static struct ProcLvupStatPoints *GetStatPointsProc(struct MenuProc *menu)
{
	return menu->proc_parent;
}

static int GetStatPointValue(struct Unit *unit, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:  return unit->maxHP;
	case STAT_POINT_POW: return unit->pow;
	case STAT_POINT_SKL: return unit->skl;
	case STAT_POINT_SPD: return unit->spd;
	case STAT_POINT_LCK: return unit->lck;
	case STAT_POINT_DEF: return unit->def;
	case STAT_POINT_RES: return unit->res;
	default:             return 0;
	}
}

/* Vanilla growth source: character growths, +5 with the Metis' Tome boost */
static int GetStatPointGrowth(struct Unit *unit, int stat)
{
	const struct CharacterData *ch = unit->pCharacterData;
	int growth;

	switch (stat) {
	case STAT_POINT_HP:  growth = ch->growthHP;  break;
	case STAT_POINT_POW: growth = ch->growthPow; break;
	case STAT_POINT_SKL: growth = ch->growthSkl; break;
	case STAT_POINT_SPD: growth = ch->growthSpd; break;
	case STAT_POINT_LCK: growth = ch->growthLck; break;
	case STAT_POINT_DEF: growth = ch->growthDef; break;
	case STAT_POINT_RES: growth = ch->growthRes; break;
	default:             growth = 0;             break;
	}

	if (unit->state & US_GROWTH_BOOST)
		growth += 5;

	if (growth < 0)
		return 0;
	if (growth > 255)
		return 255;

	return growth;
}

static int GetStatPointChange(struct BattleUnit *bu, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:  return bu->changeHP;
	case STAT_POINT_POW: return bu->changePow;
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

/* Same caps as vanilla CheckBattleUnitStatCaps */
static int GetStatPointCap(struct Unit *unit, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:  return UNIT_FACTION(unit) == FACTION_RED ? 120 : 60;
	case STAT_POINT_POW: return UNIT_POW_MAX(unit);
	case STAT_POINT_SKL: return UNIT_SKL_MAX(unit);
	case STAT_POINT_SPD: return UNIT_SPD_MAX(unit);
	case STAT_POINT_LCK: return UNIT_LCK_MAX(unit);
	case STAT_POINT_DEF: return UNIT_DEF_MAX(unit);
	case STAT_POINT_RES: return UNIT_RES_MAX(unit);
	default:             return 0;
	}
}

static bool StatPointCapped(struct ProcLvupStatPoints *proc, int stat)
{
	return GetStatPointDisplay(proc, stat) >= GetStatPointCap(proc->unit, stat);
}

static void ApplyStatPoint(struct BattleUnit *bu, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:
		bu->changeHP++;
		bu->unit.curHP++;
		break;
	case STAT_POINT_POW: bu->changePow++; break;
	case STAT_POINT_SKL: bu->changeSkl++; break;
	case STAT_POINT_SPD: bu->changeSpd++; break;
	case STAT_POINT_LCK: bu->changeLck++; break;
	case STAT_POINT_DEF: bu->changeDef++; break;
	case STAT_POINT_RES: bu->changeRes++; break;
	}
}

static void ClearStatChanges(struct BattleUnit *bu)
{
	bu->changeHP = 0;
	bu->changePow = 0;
	bu->changeSkl = 0;
	bu->changeSpd = 0;
	bu->changeDef = 0;
	bu->changeRes = 0;
	bu->changeLck = 0;
}

static bool StatPointIsAllocated(struct ProcLvupStatPoints *proc, int stat)
{
	return proc->allocated & (1 << stat);
}

static bool StatPointHasSpendableStat(struct ProcLvupStatPoints *proc)
{
	int stat;

	for (stat = 0; stat < STAT_POINT_COUNT; stat++) {
		if (StatPointIsAllocated(proc, stat))
			continue;

		if (proc->timing && proc->remaining[stat] == 0)
			continue;

		if (StatPointCapped(proc, stat))
			continue;

		return true;
	}

	return false;
}

static void MarkStatPointAllocated(struct ProcLvupStatPoints *proc, int stat)
{
	proc->allocated |= (1 << stat);

	/*
	 * Points mode: one point per stat per round. Start a new round once
	 * every stat that can still grow has had its point.
	 */
	if (!proc->timing && (proc->points == 0 || !StatPointHasSpendableStat(proc)))
		proc->allocated = 0;
}

static int GetPendingLevels(struct BattleUnit *bu)
{
	int levels = bu->unit.level - bu->levelPrevious;

	return levels < 1 ? 0 : levels;
}

static int GetPendingStatPoints(struct BattleUnit *bu)
{
	return GetPendingLevels(bu) * gLvupStatPointsConfig.points;
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

/* Points mode */

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
	bool capped = StatPointCapped(proc, stat);
	bool locked = capped || StatPointIsAllocated(proc, stat);

	DrawStatPointsRow(menu, item,
		locked ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
		capped ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE,
		GetStatPointDisplay(proc, stat));

	return 0;
}

static u8 StatPointsMenu_OnSelectPoints(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;
	(void)item;
	return MENU_ACT_SND6B;
}

static u8 StatPointsMenu_OnSelectStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber - 1;

	if (proc->points == 0)
		return MENU_ACT_SND6B;

	if (StatPointCapped(proc, stat))
		return MENU_ACT_SND6B;

	if (StatPointIsAllocated(proc, stat))
		return MENU_ACT_SND6B;

	ApplyStatPoint(proc->bu, stat);
	proc->points--;
	MarkStatPointAllocated(proc, stat);

	if (proc->points == 0 || !StatPointHasSpendableStat(proc))
		return (MenuCancelSelect(menu, item) & ~MENU_ACT_SND6B) | MENU_ACT_SND6A;

	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

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

#define STAT_POINT_ROW(label) \
	{label, 0, 0, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, StatPointsMenu_DrawStat, StatPointsMenu_OnSelectStat, 0, 0, 0}

static const struct MenuItemDef sStatPointsMenuItems[] = {
	{" Points", 0, 0, TEXT_COLOR_SYSTEM_GOLD, 0, MenuAlwaysEnabled, StatPointsMenu_DrawPoints, StatPointsMenu_OnSelectPoints, 0, 0, 0},
	STAT_POINT_ROW(" HP"),
	STAT_POINT_ROW(" Str"),
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
	0,
	0
};

/* Timing mode */

static int GetTimingBarPos(struct ProcLvupStatPoints *proc, int stat)
{
	return proc->phase[stat] * TIMING_BAR_W / TIMING_PERIOD;
}

static int GetTimingGreenBars(int remaining)
{
	int chunk;

	if (remaining <= 0)
		return 0;

	chunk = remaining > 100 ? 100 : remaining;
	return (chunk + 9) / 10;
}

static bool TimingOnGreenBar(struct ProcLvupStatPoints *proc, int stat)
{
	int bars = GetTimingGreenBars(proc->remaining[stat]);

	if (bars <= 0)
		return false;

	return GetTimingBarPos(proc, stat) < bars;
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
	int bars = GetTimingGreenBars(proc->remaining[stat]);
	bool locked = StatPointIsAllocated(proc, stat);
	bool capped = StatPointCapped(proc, stat);
	bool hit = GetStatPointChange(proc->bu, stat) > 0;

	for (i = 0; i < TIMING_BAR_W; i++) {
		color = (!locked && !capped && i < bars) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY;
		PutSpecialChar(tm + i, color, TEXT_SPECIAL_DASH);
	}

	if (capped && !locked)
		return;

	pos = GetTimingBarPos(proc, stat);

	if (locked)
		color = hit ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GOLD;
	else
		color = TimingOnGreenBar(proc, stat) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GOLD;

	PutSpecialChar(tm + pos, color, TEXT_SPECIAL_ARROW);
}

static int StatTimingMenu_DrawStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber;
	bool capped = StatPointCapped(proc, stat);
	bool locked = StatPointIsAllocated(proc, stat);
	bool hit = GetStatPointChange(proc->bu, stat) > 0;

	DrawStatPointsRow(menu, item,
		locked && !hit ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
		(capped || hit) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE,
		GetStatPointDisplay(proc, stat));

	DrawTimingBar(menu, item);
	return 0;
}

static void TimingClose_Now(ProcPtr proc)
{
	struct MenuProc *menu = ((struct Proc *)proc)->proc_parent;

	/* EndMenu alone leaves the menu text on BG0 (MENU_ACT_CLEAR does this) */
	ClearMenuBgs(menu);
	EndMenu(menu);
}

static const struct ProcCmd ProcScr_LvupStatTimingClose[] = {
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

		if (proc->remaining[stat] == 0)
			continue;

		if (StatPointCapped(proc, stat))
			continue;

		menu->itemPrevious = menu->itemCurrent;
		menu->itemCurrent = idx;
		return;
	}
}

static void StatTimingStartClose(struct MenuProc *menu)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);

	if (proc->delay == 0) {
		proc->delay = 1;
		Proc_Start(ProcScr_LvupStatTimingClose, menu);
	}
}

static u8 StatTimingMenu_OnSelectStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = item->itemNumber;

	if (proc->delay)
		return 0;

	if (StatPointIsAllocated(proc, stat))
		return MENU_ACT_SND6B;

	if (StatPointCapped(proc, stat))
		return MENU_ACT_SND6B;

	if (proc->remaining[stat] == 0)
		return MENU_ACT_SND6B;

	if (TimingOnGreenBar(proc, stat))
		ApplyStatPoint(proc->bu, stat);

	if (proc->remaining[stat] >= 100) {
		proc->remaining[stat] -= 100;

		if (proc->remaining[stat] > 0 && !StatPointCapped(proc, stat)) {
			proc->phase[stat] = 0;
			RedrawMenu(menu);
			return MENU_ACT_SND6A;
		}
	}

	proc->remaining[stat] = 0;
	MarkStatPointAllocated(proc, stat);

	if (!StatPointHasSpendableStat(proc)) {
		StatTimingStartClose(menu);
		RedrawMenu(menu);
		return MENU_ACT_SND6A;
	}

	StatTimingSelectNextRow(menu);
	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

static void StatTimingMenu_OnInit(struct MenuProc *menu)
{
	struct ProcLvupStatPoints *proc = GetStatPointsProc(menu);
	int stat = menu->menuItems[0]->itemNumber;

	menu->itemCurrent = 0;

	if (StatPointIsAllocated(proc, stat) ||
		proc->remaining[stat] == 0 ||
		StatPointCapped(proc, stat))
		StatTimingSelectNextRow(menu);
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
	{label, 0, 0, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, StatTimingMenu_DrawStat, StatTimingMenu_OnSelectStat, StatTimingMenu_OnIdle, 0, 0}

static const struct MenuItemDef sStatTimingMenuItems[] = {
	STAT_TIMING_ROW(" HP"),
	STAT_TIMING_ROW(" Str"),
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
	0,
	0
};

/* Driver proc */

static void LvupStatPoints_WaitBattleAnim(struct ProcLvupStatPoints *proc)
{
	/* Deferred mode: wait until the battle animation has returned to the map */
	if (proc->ekr_mode && (IsBattleDeamonActive() || Proc_Find(ProcScr_ekrBattleEnding)))
		return;

	Proc_Break(proc);
}

static void LvupStatPoints_Init(struct ProcLvupStatPoints *proc)
{
	gActiveUnit = proc->unit;

	if (proc->ekr_mode) {
		/*
		 * Back from the battle animation: start from clean UI layers and a
		 * fresh text allocator (the animation used the same VRAM and left
		 * stale cached number glyphs).
		 */
		ClearBg0Bg1();
		SetTextFont(NULL);
		ResetTextFont();
	}

	InitSystemTextFont();
	LoadUiFrameGraphics();

	if (proc->timing) {
		struct MenuProc *menu = StartMenuAt(&sStatTimingMenuDef, sStatTimingMenuDef.rect, proc);

		DrawTimingColumnFrame(menu);

		if (!StatPointHasSpendableStat(proc))
			StatTimingStartClose(menu);
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

	/* Deferred mode: let the battle result code apply the gains now */
	if (proc->ekr_mode)
		UnlockGame();
}

static const struct ProcCmd ProcScr_LvupStatPoints[] = {
	PROC_NAME("LvupStatPoints"),
	PROC_YIELD,
	PROC_REPEAT(LvupStatPoints_WaitBattleAnim),
	PROC_CALL(LvupStatPoints_Init),
	PROC_REPEAT(LvupStatPoints_WaitMenu),
	PROC_CALL(LvupStatPoints_OnMenuDone),
	PROC_END,
};

/*
 * Stands in for the battle-animation level-up screen: already finished, so
 * the animation goes straight on (ekrBattle_WaitEkrLvupIdle ends it).
 */
static const struct ProcCmd ProcScr_LvupStatPointsEkrSkip[] = {
	PROC_NAME("LvupStatPointsEkrSkip"),
	PROC_BLOCK,
};

static bool CanOpenStatPointsMenu(struct Unit *unit, struct BattleUnit *bu)
{
	if (!unit || !unit->pCharacterData || !bu)
		return false;

	if (UNIT_FACTION(unit) != FACTION_BLUE)
		return false;

	return GetPendingLevels(bu) > 0;
}

static void StartLvupStatPointsMenu(struct Unit *unit, struct BattleUnit *bu, ProcPtr parent, bool deferred)
{
	struct ProcLvupStatPoints *proc;
	bool timing = gLvupStatPointsConfig.mode != 0;
	int points = timing ? 0 : GetPendingStatPoints(bu);
	int i;

	/* Discard the rolled growths: the menu decides the gains */
	ClearStatChanges(bu);

	if (deferred) {
		/*
		 * Battle animations: the menu cannot be drawn over the animation, so
		 * it opens on the map once the animation ends. The game lock keeps
		 * the battle result code (which copies the BattleUnit gains to the
		 * unit) waiting until the menu is closed.
		 */
		proc = Proc_Start(ProcScr_LvupStatPoints, PROC_TREE_3);
		LockGame();
	} else {
		proc = Proc_StartBlocking(ProcScr_LvupStatPoints, parent);
	}

	proc->finished = false;
	proc->ekr_mode = deferred;
	proc->points = points > 0xFF ? 0xFF : points;
	proc->allocated = 0;
	proc->timing = timing;
	proc->delay = 0;
	proc->unit = unit;
	proc->saved_active_unit = gActiveUnit;
	proc->bu = bu;

	for (i = 0; i < STAT_POINT_COUNT; i++) {
		int growth;

		proc->phase[i] = i * (TIMING_PERIOD / STAT_POINT_COUNT);
		proc->remaining[i] = 0;

		if (!timing)
			continue;

		growth = GetStatPointGrowth(unit, i);
		proc->remaining[i] = growth;
		if (growth == 0)
			proc->allocated |= (1 << i);
	}
}

/* Replaces vanilla NewEkrLevelup (0x08073988) */
void NewEkrLevelup_StatPoints(struct Anim *ais)
{
	struct ProcEkrLevelup *proc;
	struct BattleUnit *bu;
	struct Unit *unit = NULL;

	/* gEkrDistanceType 4 is the promotion screen: leave it vanilla */
	if (gEkrDistanceType != 4) {
		bu = (GetAnimPosition(ais) == EKR_POS_L) ? gpEkrBattleUnitLeft : gpEkrBattleUnitRight;

		if (bu)
			unit = GetUnit(bu->unit.index);

		if (CanOpenStatPointsMenu(unit, bu)) {
			StartLvupStatPointsMenu(unit, bu, NULL, true);

			proc = Proc_Start(ProcScr_LvupStatPointsEkrSkip, PROC_TREE_3);
			proc->finished = true;
			gpProcEkrLevelup = proc;
			return;
		}
	}

	/* Vanilla */
	proc = Proc_Start(ProcScr_EkrLevelup, PROC_TREE_3);
	gpProcEkrLevelup = proc;
	proc->ais_main = ais;
	proc->ais_core = GetAnimAnotherSide(ais);
	proc->is_promotion = (gEkrDistanceType == 4);
	proc->timer = 0;
	proc->finished = false;
}

/* Replaces vanilla StartManimLevelUp (0x0807F10C) */
void StartManimLevelUp_StatPoints(int actor_id, ProcPtr parent)
{
	struct ManimLevelUpProc *proc;
	struct Unit *unit = gManimSt.actor[actor_id].unit;
	struct BattleUnit *bu = gManimSt.actor[actor_id].bu;

	if (CanOpenStatPointsMenu(unit, bu)) {
		StartLvupStatPointsMenu(unit, bu, parent, false);
		return;
	}

	/* Vanilla */
	proc = Proc_StartBlocking(ProcScr_ManimLevelUp, parent);
	proc->actor_id = actor_id;
}
