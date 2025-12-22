#pragma once

extern void IsTraineeLevelCappedOrPromoted(void);
extern int GetStatIncrease_NEW(int growth, int expGained);
extern void MakeHurtTargetList(int faction);

extern void TryAddUnitToAdjacentEnemyTargetList(struct Unit* unit);
extern void MakeTargetListForAdjacentEnemies(struct Unit* unit);

extern void TryAddUnitToAdjacentEnemyNonBossTargetList(struct Unit* unit);
extern void MakeTargetListForAdjacentNonBossEnemies(struct Unit* unit);

extern void TryAddUnitToAdjacentSameFactionTargetList(struct Unit* unit);
extern void MakeTargetListForAdjacentSameFaction(struct Unit* unit);

extern void TryAddUnitToAdjacentUnitsTargetList(struct Unit* unit);
extern void MakeTargetListForAdjacentUnits(struct Unit* unit);

extern void TryAddUnitToRangedStatusStavesTargetList(struct Unit* unit);
extern void MakeTargetListForRangedStatusStaves(struct Unit* unit);

extern void ExecCustomStaves(ProcPtr proc);
extern void TryAddUnitToSlowTargetList(struct Unit* unit);
extern void MakeTargetListForSlow(struct Unit *unit);
extern void TryAddUnitToForgeTargetList(struct Unit* unit);
extern void MakeTargetListForForge(struct Unit *unit);
extern void TryAddToRewarpTargetList(int x, int y);
extern void MakeTargetListForRewarp(struct Unit* unit) ;
extern void DoUseRewarpStaff(struct Unit * unit);
extern void ExecRewarpStaff(ProcPtr proc);
extern const struct SelectInfo gSelectInfo_RewarpTile;
extern void RewarpUnitMapSelect_Init(ProcPtr menu);
extern void ForEachPosInMagBy2Range(void(*func)(int x, int y));
extern const struct ProcCmd ProcScr_PostWarpStaffAction[];
extern u8 RewarpOnSelectTarget(ProcPtr proc, struct SelectTarget* target);
extern void RewarpMapSelect_Init(ProcPtr proc);      // Optional: setup help text/UI for tile selection
extern u8 RewarpMapSelect_SwitchIn(ProcPtr proc, struct SelectTarget* target); // Optional: called when cursor moves to a tile
extern void MakeRewarpRangeMap(struct Unit* unit);
extern const struct ProcCmd gProcScr_SquareSelectWarp[];
extern void TryAddUnitToPoisonTargetList(struct Unit* unit);
extern void MakeTargetListForPoison(struct Unit *unit);
extern void TryAddUnitToDelayTargetList(struct Unit* unit);
extern void MakeTargetListForDelay(struct Unit *unit);
extern void TryAddUnitToEntrapTargetList(struct Unit* unit);
extern void MakeTargetListForEntrap(struct Unit *unit);
extern void DoUseEntrapStaff(struct Unit* unit, void(*func)(struct Unit*));
extern void TryAddUnitToQuickenTargetList(struct Unit* unit);
extern void MakeTargetListForQuicken(struct Unit *unit);
extern void TryAddUnitToHideTargetList(struct Unit* unit);
extern void MakeTargetListForHide(struct Unit *unit);
extern void TryAddUnitToProvokeTargetList(struct Unit* unit);
extern void MakeTargetListForProvoke(struct Unit *unit);
extern void TryAddUnitToPetrifyTargetList(struct Unit* unit);
extern void MakeTargetListForPetrify(struct Unit *unit);
extern void TryAddUnitToSoothTargetList(struct Unit* unit);
extern void MakeTargetListForSooth(struct Unit *unit);
extern void TryAddUnitToEnfeebleTargetList(struct Unit* unit);
extern void MakeTargetListForEnfeeble(struct Unit *unit);
extern void TryAddUnitToInvestTargetList(struct Unit* unit);
extern void MakeTargetListForInvest(struct Unit *unit);

struct SecondaryGoalWindowProc {
    PROC_HEADER;
    struct Text text; // Enough for HP display
    int xCursor, yCursor;
    int xCursorPrev, yCursorPrev;
    int unitIdPrev;
};

extern void DrawSecondaryGoalWindow(struct SecondaryGoalWindowProc *proc);
extern void SecondaryGoalWindow_Init(struct SecondaryGoalWindowProc *proc);
extern void SecondaryGoalWindow_Loop_Display(struct SecondaryGoalWindowProc *proc);
extern void GetGoalWindowPosition(int* x, int* y);
extern const struct ProcCmd gProcScr_SecondaryGoalWindow[];

#ifdef CONFIG_FREE_MOVEMENT
    extern void EnableFreeMovementASMC();
    extern void DisableFreeMovementASMC();
#endif

#ifdef CONFIG_LIGHTS_OUT_GAME

    enum {
        DIMENSIONS_3x3 = 0,
        DIMENSIONS_3x4 = 1,
        DIMENSIONS_4x4 = 2,
        DIMENSIONS_4x5 = 3,

        ICON_COUNT_2 = 2,
        ICON_COUNT_3 = 3,
        
        CAN_SKIP = 0,
        CANT_SKIP = 1,

        REWARD_TIER_1 = 0,
        REWARD_TIER_2 = 1,
        REWARD_TIER_3 = 2,

        FLAG_0 = 0,
        FLAG_1 = 1,
        FLAG_2 = 2,
        FLAG_3 = 3,
        FLAG_4 = 4,
        FLAG_5 = 5,
        FLAG_6 = 6,
        FLAG_7 = 7,
        FLAG_8 = 8,
        FLAG_9 = 9,
        FLAG_10 = 10,
        FLAG_11 = 11,
        FLAG_12 = 12,
        FLAG_13 = 13,
        FLAG_14 = 14,
        FLAG_15 = 15
    };

    extern void PuzzleEvent();
    extern void PuzzleEvent2();
#endif

void PrepItemUseScroll_OnInit(struct ProcPrepItemUseJunaFruit * proc);
void PrepItemUseScroll_OnEnd(struct ProcPrepItemUseJunaFruit * proc);
void PrepItemUseScroll_OnDraw(struct ProcPrepItemUseJunaFruit * proc, int item, int x, int y);
extern const struct ProcCmd ProcScr_PrepItemUseScroll[];

extern const struct ProcCmd ProcScr_PrepItemUseArmsScroll[];
bool CanUnitUseArmsScroll(struct Unit * unit);
void ItemUseAction_ArmsScroll(struct Unit * unit);
void PrepItemUseArmsScroll_OnInit(struct ProcPrepItemUseJunaFruit * proc);
void PrepItemUseArmsScroll_OnEnd(struct ProcPrepItemUseJunaFruit * proc);
void PrepItemUseArmsScroll_OnDraw(struct ProcPrepItemUseJunaFruit * proc, int item, int x, int y);

int GetHighestWeaponRank(struct Unit * unit);

void BeginMapAnimForSimultaneousDamage(struct BattleUnit* actor, struct BattleUnit* target, int actorDamage, int targetDamage);

extern void BattleApplyMiscActionExpGains_Modular(int exp);
extern void AddExp_Event(int exp);

extern void TransferStatsandExperience(void);

extern const struct ProcCmd ProcScr_AddExp[];

bool isWeaponTriangleAdvantage(int attackerWeapon, int defenderWeapon); // weapon type
bool weaponHasSpecialEffect(int weaponAttributes);  // weaponID
int findMax(u8 *array, int size);  // find max value in array and return index

int RandSkill(int id, struct Unit * unit);
extern bool SkillTesterPlus(struct Unit *unit, u16 sid);
extern bool isMonsterClass(int classId);
extern int find_item_slot(struct Unit *unit, int item);
extern int GetUnitCurrentMP(struct Unit * unit);
extern int GetUnitMaxMP(struct Unit * unit);

extern int GetWEXPForNextLevel(int wexp);

extern u8 GetItemReward(struct Unit * winner, struct Unit * loser);

extern void Popup_GotItem(ProcPtr proc);
extern const struct ProcCmd ProcPopup_GotItem[];

/* CONFIG_INFUSE_MENU */
extern void displayScrollBackground_INFUSE(void);
extern void PrepItemList_InitGfx_INFUSE(struct PrepItemListProc * proc);
extern struct ProcCmd const ProcScr_PrepItemListScreen_INFUSE[];


/* Bonus EXP events */
struct ProcGrantBEXP {
    PROC_HEADER;
    int unitIndex;
};
extern void GrantBEXP_Loop(struct ProcGrantBEXP* proc);
extern const struct ProcCmd ProcScr_GrantBEXP[];
extern void GrantBEXP(ProcPtr parent);