#include "global.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "bmlib.h"
#include "hardware.h"
#include "fontgrp.h"
#include "functions.h"
#include "statscreen.h"
#include "player_interface.h"
#include "ctc.h"
#include "proc.h"
#include "bm.h"
#include "bmmap.h"
#include "bmtrick.h"
#include "face.h"
#include "monstergen.h"
#include "constants/classes.h"
#include "constants/items.h"

#define EXPANDED_HP_CAP 254
#define MMB_WIDTH 16
#define MMB_HEIGHT 6
#define MMB_NUMBER_OBJ_PAL 11

struct ExpandedHpTestEntry {
	u8 charId;
	u8 hp;
};

extern struct ExpandedHpTestEntry ExpandedHpTestUnits[];
extern u16 ModularMinimugBox_TileMap[];

static const s8 sExpandedHpMmbSlideInLut[4] = { 8, 12, 14, 16 };
static const s8 sExpandedHpMmbSlideOutLut[3] = { 12, 8, 4 };

static int ExpandedHpCap(void)
{
	return EXPANDED_HP_CAP;
}

int GetUnitMaxHp_ExpandedHp(struct Unit *unit)
{
	return unit->maxHP + GetItemHpBonus(GetUnitEquippedWeapon(unit));
}

int GetUnitCurrentHp_ExpandedHp(struct Unit *unit)
{
	if (unit->curHP > GetUnitMaxHp(unit))
		unit->curHP = GetUnitMaxHp(unit);

	return unit->curHP;
}

void SetUnitHp_ExpandedHp(struct Unit *unit, int value)
{
	if (value < 0)
		value = 0;

	unit->curHP = value;

	if (unit->curHP > GetUnitMaxHp(unit))
		unit->curHP = GetUnitMaxHp(unit);
}

void AddUnitHp_ExpandedHp(struct Unit *unit, int amount)
{
	int hp = unit->curHP + amount;

	if (hp > GetUnitMaxHp(unit))
		hp = GetUnitMaxHp(unit);

	if (hp < 0)
		hp = 0;

	unit->curHP = hp;
}

void UnitCheckStatCaps_ExpandedHp(struct Unit *unit)
{
	if (unit->maxHP > ExpandedHpCap())
		unit->maxHP = ExpandedHpCap();

	if (unit->pow > UNIT_POW_MAX(unit))
		unit->pow = UNIT_POW_MAX(unit);

	if (unit->skl > UNIT_SKL_MAX(unit))
		unit->skl = UNIT_SKL_MAX(unit);

	if (unit->spd > UNIT_SPD_MAX(unit))
		unit->spd = UNIT_SPD_MAX(unit);

	if (unit->def > UNIT_DEF_MAX(unit))
		unit->def = UNIT_DEF_MAX(unit);

	if (unit->res > UNIT_RES_MAX(unit))
		unit->res = UNIT_RES_MAX(unit);

	if (unit->lck > UNIT_LCK_MAX(unit))
		unit->lck = UNIT_LCK_MAX(unit);

	if (unit->conBonus > (UNIT_CON_MAX(unit) - UNIT_CON_BASE(unit)))
		unit->conBonus = (UNIT_CON_MAX(unit) - UNIT_CON_BASE(unit));

	if (unit->movBonus > (UNIT_MOV_MAX(unit) - UNIT_MOV_BASE(unit)))
		unit->movBonus = (UNIT_MOV_MAX(unit) - UNIT_MOV_BASE(unit));
}

void CheckBattleUnitStatCaps_ExpandedHp(struct Unit *unit, struct BattleUnit *bu)
{
	if ((unit->maxHP + bu->changeHP) > ExpandedHpCap())
		bu->changeHP = ExpandedHpCap() - unit->maxHP;

	if ((unit->pow + bu->changePow) > UNIT_POW_MAX(unit))
		bu->changePow = UNIT_POW_MAX(unit) - unit->pow;

	if ((unit->skl + bu->changeSkl) > UNIT_SKL_MAX(unit))
		bu->changeSkl = UNIT_SKL_MAX(unit) - unit->skl;

	if ((unit->spd + bu->changeSpd) > UNIT_SPD_MAX(unit))
		bu->changeSpd = UNIT_SPD_MAX(unit) - unit->spd;

	if ((unit->def + bu->changeDef) > UNIT_DEF_MAX(unit))
		bu->changeDef = UNIT_DEF_MAX(unit) - unit->def;

	if ((unit->res + bu->changeRes) > UNIT_RES_MAX(unit))
		bu->changeRes = UNIT_RES_MAX(unit) - unit->res;

	if ((unit->lck + bu->changeLck) > UNIT_LCK_MAX(unit))
		bu->changeLck = UNIT_LCK_MAX(unit) - unit->lck;
}

void InitBattleUnit_ExpandedHp(struct BattleUnit *bu, struct Unit *unit)
{
	if (!unit)
		return;

	bu->unit = *unit;

	bu->unit.maxHP = GetUnitMaxHp(unit);
	bu->unit.pow = GetUnitPower(unit);
	bu->unit.skl = GetUnitSkill(unit);
	bu->unit.spd = GetUnitSpeed(unit);
	bu->unit.def = GetUnitDefense(unit);
	bu->unit.lck = GetUnitLuck(unit);
	bu->unit.res = GetUnitResistance(unit);
	bu->unit.conBonus = UNIT_CON(unit);
	bu->unit.movBonus = UNIT_MOV(unit);

	bu->levelPrevious = bu->unit.level;
	bu->expPrevious = bu->unit.exp;

	bu->hpInitial = bu->unit.curHP;
	bu->statusOut = -1;

	bu->changeHP = 0;
	bu->changePow = 0;
	bu->changeSkl = 0;
	bu->changeSpd = 0;
	bu->changeDef = 0;
	bu->changeRes = 0;
	bu->changeLck = 0;
	bu->changeCon = 0;

	gBattleActor.wexpMultiplier = 0;
	gBattleTarget.wexpMultiplier = 0;

	bu->wTriangleHitBonus = 0;
	bu->wTriangleDmgBonus = 0;

	bu->nonZeroDamage = FALSE;

	gBattleActor.weaponBroke = FALSE;
	gBattleTarget.weaponBroke = FALSE;

	gBattleActor.expGain = 0;
	gBattleTarget.expGain = 0;
}

void InitBattleUnitWithoutBonuses_ExpandedHp(struct BattleUnit *bu, struct Unit *unit)
{
	InitBattleUnit(bu, unit);

	bu->unit.maxHP = unit->maxHP;
	bu->unit.pow = unit->pow;
	bu->unit.skl = unit->skl;
	bu->unit.spd = unit->spd;
	bu->unit.def = unit->def;
	bu->unit.lck = unit->lck;
	bu->unit.res = unit->res;
	bu->unit.conBonus = UNIT_CON_BASE(unit);
}

void BattleGenerateHitEffects_ExpandedHp(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	attacker->wexpMultiplier++;

	if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)) {
		if (defender->unit.pClassData->number != CLASS_DEMON_KING) {
			switch (GetItemWeaponEffect(attacker->weapon)) {
			case WPN_EFFECT_POISON:
				defender->statusOut = UNIT_STATUS_POISON;
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_POISON;

				if (defender->unit.statusIndex == UNIT_STATUS_PETRIFY ||
					defender->unit.statusIndex == UNIT_STATUS_13)
					defender->unit.state = defender->unit.state & ~US_UNSELECTABLE;
				break;

			case WPN_EFFECT_HPHALVE:
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPHALVE;
				break;
			}
		}

		if ((GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_DEVIL) &&
			(BattleRoll1RN(31 - attacker->unit.lck, FALSE))) {
			int hp;

			gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_DEVIL;

			hp = attacker->unit.curHP - gBattleStats.damage;
			if (hp < 0)
				hp = 0;
			attacker->unit.curHP = hp;
		} else {
			int hp;

			if (gBattleStats.damage > defender->unit.curHP)
				gBattleStats.damage = defender->unit.curHP;

			hp = defender->unit.curHP - gBattleStats.damage;
			if (hp < 0)
				hp = 0;
			defender->unit.curHP = hp;
		}

		if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_HPDRAIN) {
			int hp = attacker->unit.curHP + gBattleStats.damage;

			if (hp > attacker->unit.maxHP)
				hp = attacker->unit.maxHP;
			attacker->unit.curHP = hp;

			gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPSTEAL;
		}

		if (defender->unit.pClassData->number != CLASS_DEMON_KING) {
			if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_PETRIFY) {
				switch (gPlaySt.faction) {
				case FACTION_BLUE:
					if (UNIT_FACTION(&defender->unit) == FACTION_BLUE)
						defender->statusOut = UNIT_STATUS_13;
					else
						defender->statusOut = UNIT_STATUS_PETRIFY;
					break;

				case FACTION_RED:
					if (UNIT_FACTION(&defender->unit) == FACTION_RED)
						defender->statusOut = UNIT_STATUS_13;
					else
						defender->statusOut = UNIT_STATUS_PETRIFY;
					break;

				case FACTION_GREEN:
					if (UNIT_FACTION(&defender->unit) == FACTION_GREEN)
						defender->statusOut = UNIT_STATUS_13;
					else
						defender->statusOut = UNIT_STATUS_PETRIFY;
					break;
				}

				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_PETRIFY;
			}
		}
	}

	gBattleHitIterator->hpChange = gBattleStats.damage;

	if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS) ||
		attacker->weaponAttributes & (IA_UNCOUNTERABLE | IA_MAGIC)) {
		attacker->weapon = GetItemAfterUse(attacker->weapon);

		if (!attacker->weapon)
			attacker->weaponBroke = TRUE;
	}
}

void StoreNumberStringOrDashesToSmallBuffer_ExpandedHp(int n)
{
	ClearSmallStringBuffer();
	StoreNumberStringToSmallBuffer(n);
}

static void DisplayStatScreenHp(void)
{
	struct Unit *unit = gStatScreen.unit;
	int hpcur = GetUnitCurrentHp(unit);
	int hpmax = GetUnitMaxHp(unit);
	int color = hpcur == hpmax ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE;

	PutTwoSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(1, 17), TEXT_COLOR_SYSTEM_GOLD,
		TEXT_SPECIAL_HP_A, TEXT_SPECIAL_HP_B);

	if (hpmax > 99) {
		PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(6, 17), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_SLASH);
		PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(5, 17), color, hpcur);
		PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(9, 17), color, hpmax);
	} else {
		PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(5, 17), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_SLASH);
		PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(4, 17), color, hpcur);
		PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(7, 17), color, hpmax);
	}
}

void DisplayLeftPanel_ExpandedHp(void)
{
	const char *namestr = GetStringFromIndex(UNIT_NAME_ID(gStatScreen.unit));
	unsigned namexoff = GetStringTextCenteredPos(0x30, namestr);

	BG_Fill(gBG0TilemapBuffer, 0);

	BattleGenerateUiStats(gStatScreen.unit, GetUnitEquippedWeaponSlot(gStatScreen.unit));

	PutDrawText(&gStatScreen.text[STATSCREEN_TEXT_CHARANAME],
		gBG0TilemapBuffer + TILEMAP_INDEX(3, 10),
		TEXT_COLOR_SYSTEM_WHITE, namexoff, 0, namestr);

	PutDrawText(&gStatScreen.text[STATSCREEN_TEXT_CLASSNAME],
		gBG0TilemapBuffer + TILEMAP_INDEX(1, 13),
		TEXT_COLOR_SYSTEM_WHITE, 0, 0,
		GetStringFromIndex(gStatScreen.unit->pClassData->nameTextId));

	PutTwoSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(1, 15), TEXT_COLOR_SYSTEM_GOLD,
		TEXT_SPECIAL_LV_A, TEXT_SPECIAL_LV_B);
	PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(5, 15), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_E);

	PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(4, 15), TEXT_COLOR_SYSTEM_BLUE,
		gStatScreen.unit->level);
	PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(7, 15), TEXT_COLOR_SYSTEM_BLUE,
		gStatScreen.unit->exp);

	DisplayStatScreenHp();
}

static void GetMinimugHpDigits(int val, u8 *d)
{
	int i;

	if (val < 0)
		val = 0;
	if (val > EXPANDED_HP_CAP)
		val = EXPANDED_HP_CAP;

	StoreNumberStringToSmallBuffer(val);
	for (i = 0; i < 3; i++)
		d[i] = gNumberStr[i + 5] - '0';

	if (val < 100)
		d[0] = 0;
	if (val < 10)
		d[1] = 0;
}

static void DrawMinimugHpDigits(int x, int y, u8 *d)
{
	int i;

	for (i = 0; i < 3; i++)
		CallARM_PushToSecondaryOAM(
			x + (i * 7),
			y,
			gObject_8x8,
			OAM2_CHR(d[i] + 0x2E0) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
}

static void ApplyMinimugNumberPalette(void)
{
	ApplyPalette(Pal_Text, 0x10 + MMB_NUMBER_OBJ_PAL);
}

static int GetMinimugBaseX(struct PlayerInterfaceProc *proc)
{
	return (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0) ? 0 : (30 - MMB_WIDTH);
}

static struct Unit *GetMinimugCursorUnit(void)
{
	int unitId = gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x];

	return unitId ? GetUnit(unitId) : NULL;
}

static void DrawLongHpBar(u16 *buffer, struct Unit *unit, int tileBase, int length)
{
	int i;
	int hpCurrent = GetUnitCurrentHp(unit);
	int hpMax = GetUnitMaxHp(unit);
	int hpPercent;
	int middleHp;
	int middleCount;

	if (hpCurrent < 0)
		hpCurrent = 0;

	if (hpMax <= 0)
		hpPercent = 0;
	else {
		if (hpCurrent > hpMax)
			hpCurrent = hpMax;

		hpPercent = 50 * hpCurrent / hpMax;
	}

	if (hpPercent > 50)
		hpPercent = 50;

	buffer[0] = tileBase + (hpPercent > 5 ? 5 : hpPercent);

	middleCount = 4 + length;
	middleHp = hpPercent - 5;
	if (middleHp < 0)
		middleHp = 0;
	if (middleHp > 40)
		middleHp = 40;

	for (i = 0; i < middleCount; i++) {
		int fill = ((middleHp * middleCount - 40 * i) * 8) / 40;

		if (fill <= 0)
			buffer[1 + i] = tileBase + 6;
		else if (fill >= 8)
			buffer[1 + i] = tileBase + 14;
		else
			buffer[1 + i] = tileBase + 6 + fill;
	}

	hpPercent -= 45;
	if (hpPercent < 0)
		hpPercent = 0;
	if (hpPercent > 5)
		hpPercent = 5;

	buffer[middleCount + 1] = tileBase + 15 + hpPercent;
}

void ClearUnitMapUiStatus_ExpandedHp(struct PlayerInterfaceProc *proc, u16 *buffer, struct Unit *unit)
{
	(void)proc;
	(void)unit;

	buffer[0] = TILEREF(0x120, 2);
	buffer[1] = TILEREF(0x121, 2);
	buffer[2] = 0;
	buffer[3] = 0;
	buffer[4] = 0;
	buffer[5] = TILEREF(0x13E, 2);
	buffer[6] = 0;
}

void UnitMapUiUpdate_ExpandedHp(struct PlayerInterfaceProc *proc, struct Unit *unit)
{
	u8 hp[6];
	int xb;
	int yb;

	if (!UNIT_IS_VALID(unit))
		return;

	if (unit->statusIndex == UNIT_STATUS_RECOVER)
		proc->unitClock = 0;

	if ((proc->unitClock & 63) == 0) {
		if ((proc->unitClock & 64) != 0)
			PutUnitMapUiStatus(proc->statusTm, unit);
		else
			ClearUnitMapUiStatus(proc, proc->statusTm, unit);

		BG_EnableSyncByMask(BG0_SYNC_BIT);
	}

	if (proc->hideContents || ((proc->unitClock & 64) && unit->statusIndex != UNIT_STATUS_NONE))
		return;

	xb = proc->xHp * 8;
	yb = proc->yHp * 8;

	ApplyMinimugNumberPalette();
	GetMinimugHpDigits(GetUnitCurrentHp(unit), hp);
	GetMinimugHpDigits(GetUnitMaxHp(unit), hp + 3);
	DrawMinimugHpDigits(xb + 11, yb, hp);
	DrawMinimugHpDigits(xb + 41, yb, hp + 3);
}

void DrawUnitMapUi_ExpandedHp(struct PlayerInterfaceProc *proc, struct Unit *unit)
{
	char *str;

	CpuFastFill(0, gUiTmScratchA, 6 * CHR_SIZE * sizeof(u16));

	str = GetStringFromIndex(unit->pCharacterData->nameTextId);

	ClearText(proc->texts);
	Text_SetParams(proc->texts, GetStringTextCenteredPos(56, str), TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(proc->texts, str);
	PutText(proc->texts, gUiTmScratchA + TILEMAP_INDEX(5, 1));
	PutFaceChibi(
		GetUnitMiniPortraitId(unit) + ((unit->state & US_BIT23) ? 1 : 0),
		gUiTmScratchA + TILEMAP_INDEX(1, 1),
		0xF0,
		4,
		0);

	gUiTmScratchA[TILEMAP_INDEX(5, 3)] = TILEREF(0x120, 2);
	gUiTmScratchA[TILEMAP_INDEX(6, 3)] = TILEREF(0x121, 2);
	gUiTmScratchA[TILEMAP_INDEX(10, 3)] = TILEREF(0x13E, 2);

	proc->statusTm = gUiTmScratchA + TILEMAP_INDEX(5, 3);
	proc->unitClock = 0;
	proc->xHp = (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0) ? 6 : 24;
	proc->yHp = (sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0) ? 3 : 17;

	UnitMapUiUpdate(proc, unit);
	DrawLongHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1), 3);
	CallARM_FillTileRect(gUiTmScratchB, ModularMinimugBox_TileMap, TILEREF(0x0, 3));
	ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);
}

static void MMB_Slide_Common(struct PlayerInterfaceProc *proc, int out)
{
	int y;
	int baseX;
	int left;
	int width;
	int destX;
	u16 *srcA;
	u16 *srcB;
	struct Unit *unit;

	y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;
	baseX = GetMinimugBaseX(proc);
	left = (baseX == 0);
	width = out ? sExpandedHpMmbSlideOutLut[proc->showHideClock] : sExpandedHpMmbSlideInLut[proc->showHideClock];
	unit = GetMinimugCursorUnit();

	if (out)
		proc->hideContents = true;
	else if (proc->showHideClock == 0)
		proc->hideContents = false;

	TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(baseX, y), MMB_WIDTH, MMB_HEIGHT, 0);
	TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(baseX, y), MMB_WIDTH, MMB_HEIGHT, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	srcA = gUiTmScratchA;
	srcB = gUiTmScratchB;
	destX = left ? 0 : (30 - width);
	if (left) {
		srcA += (MMB_WIDTH - width);
		srcB += (MMB_WIDTH - width);
	}

	TileMap_CopyRect(srcA, gBG0TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMB_HEIGHT);
	TileMap_CopyRect(srcB, gBG1TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMB_HEIGHT);

	proc->showHideClock++;
	if (proc->showHideClock == (out ? 3 : 4)) {
		proc->showHideClock = 0;
		if (out) {
			TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(baseX, y), MMB_WIDTH, MMB_HEIGHT, 0);
			TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(baseX, y), MMB_WIDTH, MMB_HEIGHT, 0);
			BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
			proc->isRetracting = false;
			proc->windowQuadrant = -1;
		} else {
			proc->hideContents = false;
			UnitMapUiUpdate(proc, unit);
		}
		Proc_Break((ProcPtr)proc);
	} else if (!out && !proc->hideContents) {
		UnitMapUiUpdate(proc, unit);
	}
}

void MMB_Loop_SlideIn_ExpandedHp(struct PlayerInterfaceProc *proc)
{
	MMB_Slide_Common(proc, false);
}

void MMB_Loop_SlideOut_ExpandedHp(struct PlayerInterfaceProc *proc)
{
	MMB_Slide_Common(proc, true);
}

void PutUnitMapUiWindow_ExpandedHp(struct PlayerInterfaceProc *proc)
{
	int y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;
	int x = GetMinimugBaseX(proc);

	TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(x, y), MMB_WIDTH, MMB_HEIGHT);
	TileMap_CopyRect(gUiTmScratchB, gBG1TilemapBuffer + TILEMAP_INDEX(x, y), MMB_WIDTH, MMB_HEIGHT);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void ApplyExpandedHpTestHp(struct Unit *unit)
{
	struct ExpandedHpTestEntry *it;
	int hp;

	if (!unit || !unit->pCharacterData)
		return;

	for (it = ExpandedHpTestUnits; it->charId != 0; it++) {
		if (it->charId != unit->pCharacterData->number)
			continue;

		hp = it->hp;
		if (hp < 1)
			hp = 1;
		if (hp > EXPANDED_HP_CAP)
			hp = EXPANDED_HP_CAP;

		unit->maxHP = hp;
		unit->curHP = hp;
		return;
	}
}

struct Unit *LoadUnit_ExpandedHp(const struct UnitDefinition *uDef)
{
	struct UnitDefinition buf;
	struct Unit *unit = NULL;

	if (uDef->genMonster) {
		u32 packedItems;
		u16 item1;
		u16 item2;
		u16 monsterClass = GenerateMonsterClass(uDef->classIndex);

		buf = *uDef;
		buf.autolevel = TRUE;
		buf.classIndex = monsterClass;
		buf.level = GenerateMonsterLevel(uDef->level);

		packedItems = GenerateMonsterItems(monsterClass);
		item1 = packedItems >> 16;
		item2 = packedItems & 0xFFFF;

		buf.items[0] = item1;
		buf.items[1] = item2;
		buf.items[2] = 0;
		buf.items[3] = 0;

		if ((GetItemWeaponEffect(item1) == 1) || !item2)
			buf.itemDrop = FALSE;
		else
			buf.itemDrop = TRUE;

		if (item1 == ITEM_MONSTER_SHADOW_SHOT || item1 == ITEM_MONSTER_STONE) {
			buf.items[2] = buf.items[1];

			switch (monsterClass) {
			case CLASS_MOGALL:
				buf.items[1] = ITEM_MONSTER_EVIL_EYE;
				break;

			case CLASS_ARCH_MOGALL:
				buf.items[1] = ITEM_MONSTER_CRIMSON_EYE;
				break;

			case CLASS_GORGON:
				buf.items[1] = ITEM_MONSTER_DEMON_SURGE;
				break;
			}
		}

		if (CanClassWieldWeaponType(monsterClass, ITYPE_BOW) == TRUE) {
			buf.ai[2] = buf.ai[2] & (1 | 2 | 4);
			buf.ai[2] = buf.ai[2] | (8 | 32);
		}

		uDef = &buf;
	}

	switch (uDef->allegiance) {
	case 0:
		unit = GetFreeBlueUnit(uDef);
		break;

	case 1:
		unit = GetFreeUnit(FACTION_GREEN);
		break;

	case 2:
		unit = GetFreeUnit(FACTION_RED);
		break;
	}

	if (!unit)
		return NULL;

	ClearUnit(unit);
	UnitInitFromDefinition(unit, uDef);
	((void (*)(struct Unit *, const struct CharacterData *))(0x08017E35))(unit, unit->pCharacterData);
	UnitHideIfUnderRoof(unit);

	if (UNIT_IS_GORGON_EGG(unit))
		SetUnitStatus(unit, UNIT_STATUS_RECOVER);

	if (uDef->autolevel) {
		if (UNIT_FACTION(unit) == FACTION_BLUE) {
			UnitAutolevelRealistic(unit);
			UnitAutolevelWExp(unit, uDef);
		} else {
			if ((UNIT_CATTRIBUTES(unit) & CA_BOSS) || (unit->pCharacterData->number < 0x40)) {
				struct Unit *unit2 = GetFreeUnit(0);

				CopyUnit(unit, unit2);
				unit2->exp = 0;
				UnitAutolevelRealistic(unit2);
				ClearUnit(unit);
				CopyUnit(unit2, unit);
				ClearUnit(unit2);
				unit->exp = UNIT_EXP_DISABLED;
				unit->level = uDef->level;
			} else {
				UnitAutolevel(unit);
			}

			UnitAutolevelWExp(unit, uDef);
			SetUnitLeaderCharId(unit, uDef->leaderCharIndex);
		}

		if (UNIT_IS_GORGON_EGG(unit))
			unit->maxHP = (unit->level + 1) * 5;
	}

	FixROMUnitStructPtr(unit);
	UnitLoadSupports(unit);

	if (uDef->itemDrop)
		unit->state |= US_DROP_ITEM;

	UnitCheckStatCaps(unit);
	unit->curHP = GetUnitMaxHp(unit);

	if (UNIT_IS_GORGON_EGG(unit))
		SetUnitHp(unit, 5);

	ApplyExpandedHpTestHp(unit);
	return unit;
}
