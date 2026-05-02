#pragma once

extern void IsTraineeLevelCappedOrPromoted(void);
extern int GetStatIncrease_NEW(int growth, int expGained);
extern void MakeHurtTargetList(int faction);

extern void TryAddUnitToAdjacentEnemyTargetList(struct Unit *unit);
extern void MakeTargetListForAdjacentEnemies(struct Unit *unit);

extern void TryAddUnitToAdjacentEnemyNonBossTargetList(struct Unit *unit);
extern void MakeTargetListForAdjacentNonBossEnemies(struct Unit *unit);

extern void TryAddUnitToAdjacentSameFactionTargetList(struct Unit *unit);
extern void MakeTargetListForAdjacentSameFaction(struct Unit *unit);

extern void TryAddUnitToAdjacentUnitsTargetList(struct Unit *unit);
extern void MakeTargetListForAdjacentUnits(struct Unit *unit);

extern void TryAddUnitToRangedStatusStavesTargetList(struct Unit *unit);
extern void MakeTargetListForRangedStatusStaves(struct Unit *unit);

bool PhoenixStaff_HandleMenuScroll(struct MenuProc *menu);

extern void ExecCustomStaves(ProcPtr proc);
extern void TryAddUnitToSlowTargetList(struct Unit *unit);
extern void MakeTargetListForSlow(struct Unit *unit);
extern void TryAddUnitToForgeTargetList(struct Unit *unit);
extern void MakeTargetListForForge(struct Unit *unit);
extern void TryAddToRewarpTargetList(int x, int y);
extern void MakeTargetListForRewarp(struct Unit *unit);
extern void DoUseRewarpStaff(struct Unit *unit);
extern void ExecRewarpStaff(ProcPtr proc);
extern const struct SelectInfo gSelectInfo_RewarpTile;
extern void RewarpUnitMapSelect_Init(ProcPtr menu);
extern void ForEachPosInMagBy2Range(void(*func)(int x, int y));
extern const struct ProcCmd ProcScr_PostWarpStaffAction[];
extern u8 RewarpOnSelectTarget(ProcPtr proc, struct SelectTarget *target);
extern void RewarpMapSelect_Init(ProcPtr proc);      // Optional: setup help text/UI for tile selection
extern u8 RewarpMapSelect_SwitchIn(ProcPtr proc, struct SelectTarget *target); // Optional: called when cursor moves to a tile
extern void MakeRewarpRangeMap(struct Unit *unit);
extern const struct ProcCmd gProcScr_SquareSelectWarp[];
extern void TryAddUnitToPoisonTargetList(struct Unit *unit);
extern void MakeTargetListForPoison(struct Unit *unit);
extern void TryAddUnitToDelayTargetList(struct Unit *unit);
extern void MakeTargetListForDelay(struct Unit *unit);
extern void TryAddUnitToEntrapTargetList(struct Unit *unit);
extern void MakeTargetListForEntrap(struct Unit *unit);
extern void DoUseEntrapStaff(struct Unit *unit, void(*func)(struct Unit *));
extern void TryAddUnitToQuickenTargetList(struct Unit *unit);
extern void MakeTargetListForQuicken(struct Unit *unit);
extern void TryAddUnitToHideTargetList(struct Unit *unit);
extern void MakeTargetListForHide(struct Unit *unit);
extern void TryAddUnitToProvokeTargetList(struct Unit *unit);
extern void MakeTargetListForProvoke(struct Unit *unit);
extern void TryAddUnitToPetrifyTargetList(struct Unit *unit);
extern void MakeTargetListForPetrify(struct Unit *unit);
extern void TryAddUnitToSoothTargetList(struct Unit *unit);
extern void MakeTargetListForSooth(struct Unit *unit);
extern void TryAddUnitToEnfeebleTargetList(struct Unit *unit);
extern void MakeTargetListForEnfeeble(struct Unit *unit);
extern void TryAddUnitToInvestTargetList(struct Unit *unit);
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
extern void GetGoalWindowPosition(int *x, int *y);
extern const struct ProcCmd gProcScr_SecondaryGoalWindow[];

extern void EnableFreeMovementASMC(void);
extern void DisableFreeMovementASMC(void);

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

    extern void PuzzleEvent(void);
    extern void PuzzleEvent2(void);
#endif

void StartDebuggerProc(ProcPtr playerPhaseProc);

void PrepItemUseScroll_OnInit(struct ProcPrepItemUseJunaFruit *proc);
void PrepItemUseScroll_OnEnd(struct ProcPrepItemUseJunaFruit *proc);
void PrepItemUseScroll_OnDraw(struct ProcPrepItemUseJunaFruit *proc, int item, int x, int y);
extern const struct ProcCmd ProcScr_PrepItemUseScroll[];

extern const struct ProcCmd ProcScr_PrepItemUseArmsScroll[];
bool CanUnitUseArmsScroll(struct Unit *unit);
void ItemUseAction_ArmsScroll(struct Unit *unit);
void PrepItemUseArmsScroll_OnInit(struct ProcPrepItemUseJunaFruit *proc);
void PrepItemUseArmsScroll_OnEnd(struct ProcPrepItemUseJunaFruit *proc);
void PrepItemUseArmsScroll_OnDraw(struct ProcPrepItemUseJunaFruit *proc, int item, int x, int y);

int GetHighestWeaponRank(struct Unit *unit);

void BeginMapAnimForSimultaneousDamage(struct BattleUnit *actor, struct BattleUnit *target, int actorDamage, int targetDamage);

extern void BattleApplyMiscActionExpGains_Modular(int exp);
extern void AddExp_Event(int exp);

extern void TransferStatsandExperience(void);

extern const struct ProcCmd ProcScr_AddExp[];

bool isWeaponTriangleAdvantage(int attackerWeapon, int defenderWeapon); // weapon type
bool weaponHasSpecialEffect(int weaponAttributes);  // weaponID
int findMax(u8 *array, int size);  // find max value in array and return index

int RandSkill(int id, struct Unit *unit);
extern bool SkillTesterPlus(struct Unit *unit, u16 sid);
extern bool isMonsterClass(int classId);
extern int find_item_slot(struct Unit *unit, int item);
extern int GetUnitCurrentMP(struct Unit *unit);
extern int GetUnitMaxMP(struct Unit *unit);

extern int GetWEXPForNextLevel(int wexp);

extern u8 GetItemReward(struct Unit *winner, struct Unit *loser);

extern void Popup_GotItem(ProcPtr proc);
extern const struct ProcCmd ProcPopup_GotItem[];

/* gpKernelDesignerConfig->prep_menu_infuse */
extern struct ProcCmd const ProcScr_PrepItemListScreen_INFUSE[];


/* Bonus EXP events */
struct ProcGrantBEXP {
    PROC_HEADER;
    int unitIndex;
};
extern void GrantBEXP_Loop(struct ProcGrantBEXP *proc);
extern const struct ProcCmd ProcScr_GrantBEXP[];
extern void GrantBEXP(ProcPtr parent);

extern u8 Gfx_Skill_Capacity_Circle_0_8[];
extern u8 Gfx_Skill_Capacity_Circle_1_8[];
extern u8 Gfx_Skill_Capacity_Circle_2_8[];
extern u8 Gfx_Skill_Capacity_Circle_3_8[];
extern u8 Gfx_Skill_Capacity_Circle_4_8[];
extern u8 Gfx_Skill_Capacity_Circle_5_8[];
extern u8 Gfx_Skill_Capacity_Circle_6_8[];
extern u8 Gfx_Skill_Capacity_Circle_7_8[];
extern u8 Gfx_Skill_Capacity_Circle_8_8[];

extern u8 Gfx_Down_Arrow[];
extern u8 Gfx_UI_Frame_One_Line_1[];
extern u8 Gfx_UI_Frame_One_Line_2[];
extern u8 Gfx_UI_Frame_One_Line_3[];
extern u8 Gfx_UI_Frame_One_Line_4[];

extern u8 Gfx_UI_Number_0[];
extern u8 Gfx_UI_Number_1[];
extern u8 Gfx_UI_Number_2[];
extern u8 Gfx_UI_Number_3[];
extern u8 Gfx_UI_Number_4[];
extern u8 Gfx_UI_Number_5[];
extern u8 Gfx_UI_Number_6[];
extern u8 Gfx_UI_Number_7[];
extern u8 Gfx_UI_Number_8[];
extern u8 Gfx_UI_Number_9[];

extern void ShootArrow_ASMC(void);
void StartInfuseScreen_FromPrep(struct ProcAtMenu *parent);

extern void List_PutHighlightedCategorySprites_INFUSE(struct PrepItemListProc *proc);
extern void drawItems_INFUSE(struct Text *textBase, u16 *tm, int yLines, struct Unit *unit);

extern void StartCreditsProc_ASMC(ProcPtr parent);
extern int StartAvatarEdits_ASMC(ProcPtr proc);

void StartBEXPScreen_FromPrep(struct ProcAtMenu *parent);
void StartBaseScreen_FromPrep(struct ProcAtMenu *parent);
void StartAuguryScreen_FromPrep(struct ProcAtMenu *parent);

u8 MapMenu_IsBiographyCommandAvailable(const struct MenuItemDef *def, int number);
int MapMenu_BiographyCommandDraw(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 MapMenu_BiographyCommand(struct MenuProc *menu, struct MenuItemProc *menuItem);

struct BiographyEntry {
    int textId;
    u8 backgroundId;
};

struct CharacterBiography {
    u8 characterId;
    char *subtitle;
    u8 songId;
    struct BiographyEntry entries[4];
};

extern bool Generic_CanUnitBeOnPos(struct Unit *unit, s8 x, s8 y, int x2, int y2);

extern int StaffEXP(int weapon);
extern void ExecBallista(ProcPtr proc);

extern int HPTonic(struct Unit *unit);
extern int PowTonic(struct Unit *unit);
extern int MagTonic(struct Unit *unit);
extern int SklTonic(struct Unit *unit);
extern int SpdTonic(struct Unit *unit);
extern int LckTonic(struct Unit *unit);
extern int DefTonic(struct Unit *unit);
extern int ResTonic(struct Unit *unit);
extern int OmniTonic(struct Unit *unit);

extern void WorldMap_CenterCamera(ProcPtr proc, int nodeid);

extern void StartUiGoldBox_New(int x, int y, int palNum, int oam_tile, ProcPtr parent);
extern void returnToWorldMap_External(void);

extern void TalkEXPGain(ProcPtr * proc);
