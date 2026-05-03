#include "common-chax.h"
#include "strmag.h"
#include "lvup.h"

struct EfxLvupInfo {
	u8 x, y;
	u16 msg;
};

enum {
	EFXLVUP_DISP_HP = 0,
	EFXLVUP_DISP_POW,
	EFXLVUP_DISP_MAG,
	EFXLVUP_DISP_SKL,
	EFXLVUP_DISP_SPD,
	EFXLVUP_DISP_LCK,
	EFXLVUP_DISP_DEF,
	EFXLVUP_DISP_RES,
};

STATIC_DECLAR const struct EfxLvupInfo NewEfxLvupInfos[] = {
	{0x02, 0x0B, 0x4E9}, /* HP */
	{0x02, 0x0D, 0x4FE}, /* Str */
	{0x02, 0x0F, 0x4FF}, /* Mag */
	{0x02, 0x011, 0x4EC}, /* Skl */
	{0x0A, 0x0B, 0x4ED}, /* Spd */
	{0x0A, 0x0D, 0x4EE}, /* Lck */
	{0x0A, 0x0F, 0x4EF}, /* Def */
	{0x0A, 0x11, 0x4F0}, /* Res */

	{-1, -1, 0}
};

STATIC_DECLAR void EkrLvup_InitStatusTextVanilla(struct ProcEkrLevelup *proc)
{
	int i;
	struct Text *th, *th_base = gBanimText + EKRLVUP_STAT_MAX;

	(void)proc;

	for (i = 0; i < EKRLVUP_STAT_MAX; i++) {
		InitText(&th_base[i], 2);
		Text_SetCursor(&th_base[i], 8);
		Text_SetColor(&th_base[i], TEXT_COLOR_SYSTEM_BLUE);
		Text_DrawNumber(&th_base[i], gEkrLvupBaseStatus[i]);
		PutText(&th_base[i], gBG2TilemapBuffer + 3 + sEfxLvupPartsPos[i]);
	}

	/* class */
	th = th_base + EKRLVUP_STAT_CLASS;
	InitText(th, 8);
	Text_DrawString(th, GetStringFromIndex(gpEkrLvupUnit->pClassData->nameTextId));
	PutText(th, TILEMAP_LOCATED(gBG2TilemapBuffer, 3, 7));

	/* level msg */
	th = th_base + EKRLVUP_STAT_LV_MSG;
	InitText(th, 3);
	Text_SetColor(th, TEXT_COLOR_SYSTEM_GOLD);
	Text_DrawString(th, GetStringFromIndex(0x4E7));
	PutText(th, TILEMAP_LOCATED(gBG2TilemapBuffer, 10, 7));

	/* level value */
	th = th_base + EKRLVUP_STAT_LV_VAL;
	InitText(th, 2);
	Text_SetCursor(th, 8);
	Text_SetColor(th, TEXT_COLOR_SYSTEM_BLUE);
	Text_DrawNumber(th, gEkrLvupPreLevel);
	PutText(th, TILEMAP_LOCATED(gBG2TilemapBuffer, 13, 7));
}

LYN_REPLACE_CHECK(EkrLvup_InitStatusText);
void EkrLvup_InitStatusText(struct ProcEkrLevelup *proc)
{
	int i;
	struct BattleUnit *bunit, *bunit2;
	struct Unit *unit;
	const u16 *stat_label_pos;

	if (proc->ais_main == NULL) {
		bunit2 = gpEkrBattleUnitLeft;
		gpEkrLvupUnit = unit = &bunit2->unit;
		gpEkrLvupBattleUnit = bunit = gpEkrBattleUnitRight;
	} else {
		bunit2 = gpEkrBattleUnitRight;
		gpEkrLvupUnit = unit = &bunit2->unit;
		gpEkrLvupBattleUnit = bunit = gpEkrBattleUnitLeft;
	}

	if (proc->is_promotion == false) {
		unit = GetUnit(unit->index);

		gEkrLvupPreLevel = bunit2->levelPrevious;
		gEkrLvupBaseStatus[EFXLVUP_DISP_HP]  = unit->maxHP;
		gEkrLvupBaseStatus[EFXLVUP_DISP_POW] = unit->pow;
		gEkrLvupBaseStatus[EFXLVUP_DISP_MAG] = UNIT_MAG(unit);
		gEkrLvupBaseStatus[EFXLVUP_DISP_SKL] = unit->skl;
		gEkrLvupBaseStatus[EFXLVUP_DISP_SPD] = unit->spd;
		gEkrLvupBaseStatus[EFXLVUP_DISP_LCK] = unit->lck;
		gEkrLvupBaseStatus[EFXLVUP_DISP_DEF] = unit->def;
		gEkrLvupBaseStatus[EFXLVUP_DISP_RES] = unit->res;
		gEkrLvupPostLevel = bunit2->levelPrevious + 1;

		gEkrLvupPostStatus[EFXLVUP_DISP_HP]  = unit->maxHP + bunit2->changeHP;
		gEkrLvupPostStatus[EFXLVUP_DISP_POW] = unit->pow + bunit2->changePow;
		gEkrLvupPostStatus[EFXLVUP_DISP_MAG] = UNIT_MAG(unit) + BU_CHG_MAG(bunit2);
		gEkrLvupPostStatus[EFXLVUP_DISP_SKL] = unit->skl + bunit2->changeSkl;
		gEkrLvupPostStatus[EFXLVUP_DISP_SPD] = unit->spd + bunit2->changeSpd;
		gEkrLvupPostStatus[EFXLVUP_DISP_LCK] = unit->lck + bunit2->changeLck;
		gEkrLvupPostStatus[EFXLVUP_DISP_DEF] = unit->def + bunit2->changeDef;
		gEkrLvupPostStatus[EFXLVUP_DISP_RES] = unit->res + bunit2->changeRes;
	} else {
		gEkrLvupPreLevel = unit->level;
		gEkrLvupBaseStatus[EFXLVUP_DISP_HP]  = unit->maxHP;
		gEkrLvupBaseStatus[EFXLVUP_DISP_POW] = unit->pow;
		gEkrLvupBaseStatus[EFXLVUP_DISP_MAG] = UNIT_MAG(unit);
		gEkrLvupBaseStatus[EFXLVUP_DISP_SKL] = unit->skl;
		gEkrLvupBaseStatus[EFXLVUP_DISP_SPD] = unit->spd;
		gEkrLvupBaseStatus[EFXLVUP_DISP_LCK] = unit->lck;
		gEkrLvupBaseStatus[EFXLVUP_DISP_DEF] = unit->def;
		gEkrLvupBaseStatus[EFXLVUP_DISP_RES] = unit->res;
		gEkrLvupPostLevel = 1;

		gEkrLvupPostStatus[EFXLVUP_DISP_HP]  = bunit->unit.maxHP;
		gEkrLvupPostStatus[EFXLVUP_DISP_POW] = bunit->unit.pow;
		gEkrLvupPostStatus[EFXLVUP_DISP_MAG] = UNIT_MAG(&bunit->unit);
		gEkrLvupPostStatus[EFXLVUP_DISP_SKL] = bunit->unit.skl;
		gEkrLvupPostStatus[EFXLVUP_DISP_SPD] = bunit->unit.spd;
		gEkrLvupPostStatus[EFXLVUP_DISP_LCK] = bunit->unit.lck;
		gEkrLvupPostStatus[EFXLVUP_DISP_DEF] = bunit->unit.def;
		gEkrLvupPostStatus[EFXLVUP_DISP_RES] = bunit->unit.res;
	}

	InitTextFont(&gBanimFont, BG_CHR_ADDR(0x146), 0x146, 0);
	stat_label_pos = sEfxLvupPartsPos;

	for (i = 0; i < EKRLVUP_STAT_MAX; i++) {
		const char *str = GetStringFromIndex(NewEfxLvupInfos[i].msg);

		InitText(&gBanimText[i], 3);
		Text_SetCursor(&gBanimText[i], 0);
		Text_SetColor(&gBanimText[i], TEXT_COLOR_SYSTEM_GOLD);
		Text_DrawString(&gBanimText[i], str);
		PutText(&gBanimText[i], gBG2TilemapBuffer + stat_label_pos[i]);
	}

	EkrLvup_InitStatusTextVanilla(proc);
}
