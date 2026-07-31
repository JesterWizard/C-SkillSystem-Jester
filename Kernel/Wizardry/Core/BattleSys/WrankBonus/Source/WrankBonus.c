#include "common-chax.h"
#include "help-box.h"
#include "wrank-bonus.h"
#include "weapon-slots.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

#define LOCAL_TRACE 0

const struct WrankBonusConfEnt *GetWrankBonusConf(struct Unit *unit, int wtype, int wrank)
{
	const struct WrankBonusConfEnt *it;

	for (it = gpWrankBonusConf; it->wrank != WPN_LEVEL_0; it++)
		if (it->wtype == wtype && it->wrank == wrank)
			return it;

	return NULL;
}

int GetWtypeFromRTextMsg(u16 msg)
{
	const struct WrankRtextConfEnt *it;

	for (it = gpWrankRtextConf; it->msg != 0; it++) {
		if (it->msg == msg)
			return it->wtype;
	}
	return -1;
}

int GetWtypeMasteryMsg(int wtype)
{
	const struct WrankRtextConfEnt *it;

	for (it = gpWrankRtextConf; it->msg != 0; it++) {
		if (it->wtype == wtype)
			return it->msg;
	}

	return MSG_0561; /* fallback */
}

int GetDisplayedWeaponTypeAt(struct Unit *unit, int index)
{
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	int wcount;

	if (!unit || index < 0)
		return -1;

	wcount = ListUnitMappedWeaponTypes(unit, wtypes, UNIT_WEAPON_SLOT_COUNT);
	if (index >= wcount)
		return -1;

	return wtypes[index];
}

/**
 * pre-battle
 */
void PreBattleCalc_WrankBonus(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	int wtype = attacker->weaponType;
	int wrank = GetWeaponLevelFromExp(UNIT_WRANK(&attacker->unit, wtype));
	const struct WrankBonusConfEnt *conf = GetWrankBonusConf(&attacker->unit, wtype, wrank);

	if (conf) {
		attacker->battleAttack       += conf->bonus[BATTLE_STATUS_ATK];
		attacker->battleDefense      += conf->bonus[BATTLE_STATUS_DEF];
		attacker->battleSpeed        += conf->bonus[BATTLE_STATUS_AS];
		attacker->battleHitRate      += conf->bonus[BATTLE_STATUS_HIT];
		attacker->battleAvoidRate    += conf->bonus[BATTLE_STATUS_AVO];
		attacker->battleCritRate     += conf->bonus[BATTLE_STATUS_CRIT];
		attacker->battleDodgeRate    += conf->bonus[BATTLE_STATUS_DODGE];
		attacker->battleSilencerRate += conf->bonus[BATTLE_STATUS_SILENCER];
	}
}

/**
 * help-box
 */
void HbRedirect_WrankPage(struct HelpBoxProc *proc)
{
	u8 wtypes[UNIT_WEAPON_SLOT_COUNT];
	int wcount = ListUnitMappedWeaponTypes(gStatScreen.unit, wtypes, UNIT_WEAPON_SLOT_COUNT);

	if (proc->info->mid < wcount)
		return;

	switch (proc->moveKey) {
	case DPAD_DOWN:
		TryRelocateHbDown(proc);
		break;

	case DPAD_UP:
		TryRelocateHbUp(proc);
		break;

	case DPAD_LEFT:
		TryRelocateHbLeft(proc);
		break;

	case DPAD_RIGHT:
	default:
		TryRelocateHbRight(proc);
		break;
	}
}

void HbPopuplate_WrankBonus(struct HelpBoxProc *proc)
{
	int wtype = GetDisplayedWeaponTypeAt(gStatScreen.unit, proc->info->mid);

	sHelpBoxType = NEW_HB_DEFAULT;

	if (wtype < 0)
		return;

	proc->item = 0;
	proc->mid = GetWtypeMasteryMsg(wtype);

	if (!gpKernelDesignerConfig->wrank_bonux_rtext_auto_gen)
		return;

	sHelpBoxType = NEW_HB_WRANK_STATSCREEN;
}

void DrawHelpBoxLabels_WrankBonus(void)
{
	if (gpKernelDesignerConfig->quality_of_life_fixes == true) {
		Text_InsertDrawString(&gHelpBoxSt.text[0], 0x00, TEXT_COLOR_47CF, "WEXP");
		Text_InsertDrawString(&gHelpBoxSt.text[0], 0x40, TEXT_COLOR_47CF, "Rank Up");

		Text_InsertDrawString(&gHelpBoxSt.text[1], 0x00, TEXT_COLOR_47CF, GetStringFromIndex(MSG_04F4)); /* atk */
		Text_InsertDrawString(&gHelpBoxSt.text[1], 0x40, TEXT_COLOR_47CF, GetStringFromIndex(MSG_0501)); /* hit */
	}
}

void DrawHelpBoxStats_WrankBonus(struct ProcHelpBoxIntro *proc)
{
	int wtype = GetWtypeFromRTextMsg(proc->msg);
	int wrank;
	const struct WrankBonusConfEnt _conf = { 0 };
	const struct WrankBonusConfEnt *conf;

	if (wtype < 0)
		return;

	wrank = GetWeaponLevelFromExp(UNIT_WRANK(gStatScreen.unit, wtype));
	conf = GetWrankBonusConf(gStatScreen.unit, wtype, wrank);
	if (!conf)
		conf = &_conf;

	if (gpKernelDesignerConfig->quality_of_life_fixes == true) {
		Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 0x30, TEXT_COLOR_456F, UNIT_WRANK(gStatScreen.unit, wtype));
		Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 0x80, TEXT_COLOR_456F, GetWEXPForNextLevel(UNIT_WRANK(gStatScreen.unit, wtype)));

		Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[1], 0x30, TEXT_COLOR_456F, conf->bonus[BATTLE_STATUS_HIT]);
		Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[1], 0x80, TEXT_COLOR_456F, conf->bonus[BATTLE_STATUS_CRIT]);
	}
}

int GetWEXPForNextLevel(int wexp)
{
	if (wexp < WPN_EXP_D)
		return WPN_EXP_D - wexp;
	else if (wexp < WPN_EXP_C)
		return WPN_EXP_C - wexp;
	else if (wexp < WPN_EXP_B)
		return WPN_EXP_B - wexp;
	else if (wexp < WPN_EXP_A)
		return WPN_EXP_A - wexp;
	else
		return WPN_EXP_S - wexp;
}
