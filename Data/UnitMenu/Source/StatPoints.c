#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "strmag.h"
#include "lvup.h"
#include "bmmenu.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

enum {
	STAT_POINT_HP,
	STAT_POINT_POW,
	STAT_POINT_MAG,
	STAT_POINT_SKL,
	STAT_POINT_SPD,
	STAT_POINT_LCK,
	STAT_POINT_DEF,
	STAT_POINT_RES,
};

/* Tile column (relative to the item) where the right-aligned number ends */
#define STAT_POINT_NUMBER_X 7

static u8 *GetLvupStatPointsRef(u8 pid)
{
	if (pid == 0 || pid > LVUP_STAT_POINTS_AMT)
		return NULL;

	return &gLvupStatPoints[pid - 1];
}

int GetLvupStatPoints(u8 pid)
{
	u8 *ref = GetLvupStatPointsRef(pid);

	return ref ? *ref : 0;
}

void AddLvupStatPoints(u8 pid, int amt)
{
	u8 *ref = GetLvupStatPointsRef(pid);
	int val;

	if (!ref)
		return;

	val = *ref + amt;

	if (val < 0)
		val = 0;

	if (val > 0xFF)
		val = 0xFF;

	*ref = val;
}

/* NewGameSaveHook */
void ResetLvupStatPoints(void)
{
	memset(gLvupStatPoints, 0, sizeof(gLvupStatPoints));
}

/* SaveData */
void SaveLvupStatPoints(u8 *dst, const u32 size)
{
	Assert(size == sizeof(gLvupStatPoints));

	WriteAndVerifySramFast(gLvupStatPoints, dst, size);
}

void LoadLvupStatPoints(u8 *src, const u32 size)
{
	Assert(size == sizeof(gLvupStatPoints));

	ReadSramFast(src, gLvupStatPoints, size);
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

static void ApplyStatPoint(struct Unit *unit, int stat)
{
	switch (stat) {
	case STAT_POINT_HP:
		unit->maxHP++;
		SetUnitHp(unit, GetUnitCurrentHp(unit) + 1);
		break;

	case STAT_POINT_POW: unit->pow++;       break;
	case STAT_POINT_MAG: UNIT_MAG(unit)++;  break;
	case STAT_POINT_SKL: unit->skl++;       break;
	case STAT_POINT_SPD: unit->spd++;       break;
	case STAT_POINT_LCK: unit->lck++;       break;
	case STAT_POINT_DEF: unit->def++;       break;
	case STAT_POINT_RES: unit->res++;       break;
	}

	UnitCheckStatCaps(unit);
}

/**
 * Allocate sub-menu: row 0 shows the remaining points, rows 1..8 are stats.
 */
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
	DrawStatPointsRow(menu, item, TEXT_COLOR_SYSTEM_GOLD, TEXT_COLOR_SYSTEM_BLUE,
		GetLvupStatPoints(UNIT_CHAR_ID(gActiveUnit)));

	return 0;
}

static int StatPointsMenu_DrawStat(struct MenuProc *menu, struct MenuItemProc *item)
{
	int stat = item->itemNumber - 1;
	int value = GetStatPointValue(gActiveUnit, stat);
	bool capped = value >= GetStatPointCap(gActiveUnit, stat);

	DrawStatPointsRow(menu, item,
		capped ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
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
	struct Unit *unit = gActiveUnit;
	u8 pid = UNIT_CHAR_ID(unit);
	int stat = item->itemNumber - 1;

	if (GetLvupStatPoints(pid) == 0)
		return MENU_ACT_SND6B;

	if (GetStatPointValue(unit, stat) >= GetStatPointCap(unit, stat))
		return MENU_ACT_SND6B;

	ApplyStatPoint(unit, stat);
	AddLvupStatPoints(pid, -1);

	if (GetLvupStatPoints(pid) == 0)
		return (ItemMenu_ButtonBPressed(menu, item) & ~MENU_ACT_SND6B) | MENU_ACT_SND6A;

	RedrawMenu(menu);
	return MENU_ACT_SND6A;
}

#define STAT_POINT_ROW(label) \
	{label, 0, MSG_MenuCommand_Allocate_DESC, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, StatPointsMenu_DrawStat, StatPointsMenu_OnSelectStat, 0, 0, 0}

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

/* 9 rows fill the full 20-tile screen height, so the menu starts at y = 0 */
static const struct MenuDef sStatPointsMenuDef = {
	{1, 0, 10, 0},
	0,
	sStatPointsMenuItems,
	0, 0, 0,
	ItemMenu_ButtonBPressed,
	MenuAutoHelpBoxSelect,
	MenuStdHelpBox
};

u8 AllocateCommandUsability(const struct MenuItemDef *def, int number)
{
	if (!gpKernelDesignerConfig->lvup_stat_points)
		return MENU_NOTSHOWN;

	if (!UNIT_IS_VALID(gActiveUnit) || UNIT_FACTION(gActiveUnit) != FACTION_BLUE)
		return MENU_NOTSHOWN;

	if (GetLvupStatPoints(UNIT_CHAR_ID(gActiveUnit)) == 0)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 AllocateCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	struct MenuRect rect = sStatPointsMenuDef.rect;

	if (menuItem->xTile >= 12)
		rect.x = 22 + 7 - rect.w;

	StartMenuAt(&sStatPointsMenuDef, rect, NULL);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

LYN_REPLACE_CHECK(NewEkrLevelup);
void NewEkrLevelup(struct Anim *ais)
{
	struct ProcEkrLevelup *proc = Proc_Start(ProcScr_EkrLevelup, PROC_TREE_3);

	gpProcEkrLevelup = proc;
	proc->ais_main = ais;
	proc->ais_core = GetAnimAnotherSide(ais);
	proc->timer = 0;
	proc->finished = gpKernelDesignerConfig->lvup_stat_points ? true : false;
	proc->is_promotion = false;
}
