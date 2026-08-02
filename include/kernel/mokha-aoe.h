#pragma once

#include "common-chax.h"

enum MokhaAoeAttackIndex {
	MOKHA_AOE_ATK_DEFAULT = 0,
	MOKHA_AOE_ATK_BLAZE = 1,
	MOKHA_AOE_ATK_ABSORPTION = 2,
	MOKHA_AOE_ATK_FUSILLADE = 3,
	MOKHA_AOE_ATK_FIRE_ARROWS = 4,
	MOKHA_AOE_ATK_GROUP_LANCE = 5,
	MOKHA_AOE_ATK_COUNT = 6,
};

struct MokhaAoeAttackInfo {
	u16 nameMsg;
	u16 descMsg;
	u8 range;
	u8 damage;
	u8 mapRoutine;
};

typedef void (*GambitAoeMapFunc)(u8 x, u8 y, u8 direction);

extern const struct MokhaAoeAttackInfo gMokhaAoeAttackTable[MOKHA_AOE_ATK_COUNT];
extern const u8 gMokhaAoeEligibleByPid[0x100];
extern GambitAoeMapFunc const GambitEffectMap_DrawMapRoutineTable[];

extern u8 sGambitTargetSaveBuf[0x42];
extern u16 sGambitExpAccum;
extern u8 sGambitSelectedAttack;

extern u8 Gambit_ActionIndex;
extern const struct ProcCmd ProcScr_GambitAction[];

extern const struct SelectInfo gSelectInfo_Gambit;

bool IsMokhaAoeEnabled(void);
bool IsUnitMokhaAoeEligible(struct Unit *unit);
const struct MokhaAoeAttackInfo *GetMokhaAoeAttackInfo(int index);
int GetMokhaAoeDamage(struct Unit *target, int attackIndex);

void GambitResetMaps(void);
void FillRangeMapForHover(struct Unit *unit, u8 range);
void FillAOEEffectMap_OnChangeTarget(u8 x, u8 y, u8 gambitIndex);

void MakeTargetListFor_SubGambitMenu(struct Unit *unit, u8 range);
void MakeTargetListFor_AfterSelectAPressed(u8 x, u8 y, u8 gambitIndex);
void SaveTarget_PostGambitTargetSelection(void);
void ClearTarget_CommonFlagSaveSu(void);

u8 Gambit_UpperMenu_Usability(const struct MenuItemDef *def, int number);
u8 Gambit_UpperMenu_Effect(struct MenuProc *menu, struct MenuItemProc *item);
int Gambit_UpperMenu_Hover(struct MenuProc *menu, struct MenuItemProc *item);
int Gambit_UpperMenu_Unhover(struct MenuProc *menu, struct MenuItemProc *item);

u8 GambitSelectMenu_Usability(const struct MenuItemDef *def, int number);
u8 GambitSelectMenu_Effect(struct MenuProc *menu, struct MenuItemProc *item);
int GambitSelectMenu_Hover(struct MenuProc *menu, struct MenuItemProc *item);
int GambitSelectMenu_Unhover(struct MenuProc *menu, struct MenuItemProc *item);

bool GambitAction(ProcPtr proc);

int GetBattleUnitExpGainRework(struct BattleUnit *actor, struct BattleUnit *target);
