#pragma once

#include "common-chax.h"
#include "kernel-lib.h"

struct WrankBonusConfEnt {
	u8 wtype, wrank;
	u8 _pad_[2];
	s8 bonus[BATTLE_STATUS_MAX];
};

extern struct WrankBonusConfEnt const gWrankBonusConf[];
extern struct WrankBonusConfEnt const *const gpWrankBonusConf;

struct WrankRtextConfEnt {
	u16 msg;
	u16 wtype;
};

extern struct WrankRtextConfEnt const gWrankRtextConf[];
extern struct WrankRtextConfEnt const *const gpWrankRtextConf;

const struct WrankBonusConfEnt *GetWrankBonusConf(struct Unit *unit, int wtype, int wrank);
int GetWtypeMasteryMsg(int wtype);
int GetWtypeFromRTextMsg(u16 msg);
int GetDisplayedWeaponTypeAt(struct Unit *unit, int index);

void PreBattleCalc_WrankBonus(struct BattleUnit *attacker, struct BattleUnit *defender);
void HbPopuplate_WrankBonus(struct HelpBoxProc *proc);
void HbRedirect_WrankPage(struct HelpBoxProc *proc);
void DrawHelpBoxLabels_WrankBonus(void);
void DrawHelpBoxStats_WrankBonus(struct ProcHelpBoxIntro *proc);
