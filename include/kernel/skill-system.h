#pragma once

#include "common-chax.h"
#include "efx-anim.h"
#include "efx-skill.h"
#include "list-verify.h"
#include "kernel-lib.h"
#include "constants/event-cmds.h"

#ifndef MAX_SKILL_NUM
#define MAX_SKILL_NUM 0x3FF
#endif

#ifdef CONFIG_TURN_ON_ALL_SKILLS
#define MAX_EQUIP_SKILL_NUM 0x3FF
#else
#define MAX_EQUIP_SKILL_NUM 0xFF
#endif

#ifdef CONFIG_TURN_ON_ALL_SKILLS
#define UNIT_RAM_SKILLS_LEN 5
#else
#define UNIT_RAM_SKILLS_LEN 7
#endif

#ifdef CONFIG_TURN_ON_ALL_SKILLS

void StartSkillSwapTradeMenu(struct Unit* leftUnit, struct Unit* rightUnit);

static inline u16 GetUnitSkillIndex(const struct Unit* unit, int index) {
	if (index < 0 || index >= UNIT_RAM_SKILLS_LEN) return 0xFFFF;

	u64 buffer = 0;
	for (int i = 0; i < UNIT_SUPPORT_MAX_COUNT; ++i)
		buffer |= ((u64)unit->supports[i]) << (8 * i);

	return (buffer >> (index * 10)) & 0x3FF;
}

static inline void SetUnitSkillIndex(struct Unit* unit, int index, u16 sid) {
	if (index < 0 || index >= UNIT_RAM_SKILLS_LEN) return;

	u64 buffer = 0;
	for (int i = 0; i < UNIT_SUPPORT_MAX_COUNT; ++i)
		buffer |= ((u64)unit->supports[i]) << (8 * i);

	buffer &= ~(((u64)0x3FF) << (index * 10));
	buffer |= ((u64)(sid & 0x3FF)) << (index * 10);

	for (int i = 0; i < UNIT_SUPPORT_MAX_COUNT; ++i)
		unit->supports[i] = (buffer >> (8 * i)) & 0xFF;
}

#define GET_SKILL(unit, i) GetUnitSkillIndex(unit, i)
#define SET_SKILL(unit, i, sid) SetUnitSkillIndex(unit, i, sid)

#else

#define GET_SKILL(unit, i) (UNIT_RAM_SKILLS(unit)[i])
#define SET_SKILL(unit, i, sid) (UNIT_RAM_SKILLS(unit)[i] = (sid))

#endif

enum SkillInfoListss {
	SKILL_INFO_EQUIP,
	SKILL_INFO_CHARACTER,
	SKILL_INFO_CLASS,
	SKILL_INFO_ITEM,

	Skill_INFO_MAX
};

#ifdef CONFIG_TURN_ON_ALL_SKILLS
#define MAX_EQUIP_SKILL_NUM 0x3FF
// For extended equippable skills, always use generic tester.
#define SKILL_INDEX_LIST(sid) (0)
#define SKILL_INDEX_REAL(sid) ((sid) & 0x3FF)
#else
#define MAX_EQUIP_SKILL_NUM 0xFF
#define SKILL_INDEX_LIST(sid) (((sid) >> 8) & 0xFF)
#define SKILL_INDEX_REAL(sid) ((sid) & 0xFF)
#endif

#ifdef CONFIG_TURN_ON_ALL_SKILLS
#define EQUIP_SKILL_VALID(sid) (sid > 0x000 && sid < 0x400)
#else
#define EQUIP_SKILL_VALID(sid) (sid > 0x000 && sid < 0x0FF)
#endif

#define COMMON_SKILL_VALID(sid) (sid > 0x000 && sid < 0x400)

// Apply-cost macro (ensures we never set a *worse* cost)
#define APPLY_COST_IF_LOWER(terr, newCost) \
	do { \
		if ((terr) < TERRAIN_COUNT && gWorkingTerrainMoveCosts[(terr)] > (newCost)) \
			gWorkingTerrainMoveCosts[(terr)] = (newCost); \
	} while (0)

/**
 * Equippable skills
 */

#define GET_UNIT_SKILL(unit, i) (                  \
    (((((u64)(unit)->supports[0]))     |           \
    (((u64)(unit)->supports[1]) << 8)  |           \
    (((u64)(unit)->supports[2]) << 16) |           \
    (((u64)(unit)->supports[3]) << 24) |           \
    (((u64)(unit)->supports[4]) << 32) |           \
    (((u64)(unit)->supports[5]) << 40) |           \
    (((u64)(unit)->supports[6]) << 48)) >> ((i) * 10)) & 0x3FF)

#define UNIT_RAM_SKILLS(unit) ((u16 *)((unit)->supports))

#define RAM_SKILL_LEN_EXT ( \
	gpKernelDesignerConfig->max_equipable_skill < UNIT_RAM_SKILLS_LEN \
		? gpKernelDesignerConfig->max_equipable_skill \
		: UNIT_RAM_SKILLS_LEN)

extern u16 const* const gpConstSkillTable_Person;
extern u16 const* const gpConstSkillTable_Job;
extern u16 const* const gpConstSkillTable_Item;
extern u16 const* const gpConstSkillTable_Weapon;

struct SkillInfo {
	const u8* icon;
	u16 name, desc, price, capacity;
	u8 iconPal;
};

extern struct SkillInfo const* const gpSkillInfos;

const u8* GetSkillIcon1(const u8 sid);
const u8* GetSkillIcon2(const u8 sid);
const u8* GetSkillIcon3(const u8 sid);
const u8* GetSkillIcon4(const u8 sid);
u8 GetSkillIconPal(const u16 sid);

u16 GetSkillDescMsg(const u16 sid);
u16 GetSkillNameMsg(const u16 sid);
char* GetSkillDescStr(const u16 sid);
char* SkillDescToName(char* str);
char* GetSkillNameStrFormDesc(const u16 sid);
char* GetSkillNameStr(const u16 sid);

/* An interesting quirk of the narrow font transformer in GetSkillNameStr is that the output is impossible
 to perform string comparisons on. Hence, a secondary function is need that just gives the raw output */
char* GetSkillNameStr_NormalFont(const u16 sid);

u16 GetSkillPrice(const u16 sid);
u16 GetSkillCapacity(const u16 sid);

/**
 * 7 generic skill
 * 3 person skill
 * 3 job skill
 * 10 item skill
 *
 * (maybe todo) 2 weapon skill
 */
struct SkillList {
	struct UnitListHeader header;
	u8 amt;
	u8 _pad_;
	u16 sid[31];
};
struct SkillList* GetUnitSkillList(struct Unit* unit);

void SetupBattleSkillList(void);
void GenerateSkillListExt(struct Unit* unit, struct SkillList* list);
void ForceUpdateUnitSkillList(struct Unit* unit);
void DisableUnitSkilLList(struct Unit* unit);
void ResetSkillLists(void);
void AppendBattleUnitSkillList(struct BattleUnit* bu, u16 skill);

static inline bool SkillListOverflow(struct SkillList* list)
{
	return (list->amt >= ARRAY_COUNT(list->sid));
}

/* Skill tetsers */
// see: ../docs/SkillSys.md
bool SkillTester(struct Unit* unit, const u16 sid);
bool SkillListTester(struct Unit* unit, const u16 sid);
bool BattleFastSkillTester(struct BattleUnit* bu, const u16 sid);

bool CheckSkillActivate(struct Unit* unit, int sid, int rate);
bool CheckActiveUnitSkillActivate(int sid, int rate);
bool CheckBattleSkillActivate(struct BattleUnit* actor, struct BattleUnit* target, int sid, int rate);

/**
 * FEB list
 *
 * This is aligned to old asm skillsystem config, see:
 * https://feuniverse.us/t/the-skill-system-and-you-maximizing-your-usage-of-fe8s-most-prolific-bundle-of-wizardry/8232/5
 */
struct FebListEnt {
	u8 level;
	u8 sid;
};

#define JOB_SKILL_LEVEL_PROMOTE 0xFF
#define JOB_SKILL_LEVEL_CONFIG(__level, __config) (((__level) & 0x1F) | (__config))
#define LOAD_JOB_SKILL_CONFIG(__level_config)  ((__level_config) & 0xE0)
#define LOAD_JOB_SKILL_LEVEL(__level_config)   ((__level_config) & 0x1F)
#define LOAD_JOB_SKILL_CONFIG_NONE           (0 << 5)
#define LOAD_JOB_SKILL_CONFIG_PLAYER_ONLY    (1 << 5)
#define LOAD_JOB_SKILL_CONFIG_ENEMY_ONLY     (2 << 5)
#define LOAD_JOB_SKILL_CONFIG_WO_TUTORIAL    (3 << 5)
#define LOAD_JOB_SKILL_CONFIG_HARD_MODE_ONLY (4 << 5)

// extern struct FebListEnt const *const CharLevelUpSkillTable[0x100];
// extern struct FebListEnt const *const ClassLevelUpSkillTable[0x100];

extern struct FebListEnt const* const* const gpCharLevelUpSkillTable;
extern struct FebListEnt const* const* const gpClassLevelUpSkillTable;

#define DEFAULT_LEVEL_SKILLS_BUF_MAX_LEN 16

int GetUnitLevelSkills(struct Unit* unit, int level_from, int level_to, u8* out_buffer, int max_len);
int GetInitialSkillList(struct Unit* unit, u8* out_buffer, int max_len);
int GetLevelUpSkillList(struct Unit* unit, int level, u8* out_buffer, int max_len);
int GetPromotionSkillList(struct Unit* unit, u8* out_buffer, int max_len);

void FebList_LoadUnitSkill(struct Unit* unit);
void FebList_LvupAddSkill(struct Unit* unit, int level);
void FebList_PromotionAddSkill(struct Unit* unit);

/* Prep equip skill list */
struct PrepEquipSkillList {
	struct UnitListHeader header;
	u8 amt;
	u8 sid[0x3FF];
};

void ResetPrepEquipSkillList(void);
struct PrepEquipSkillList* GetPrepEquipSkillList(struct Unit* unit);

/* Game data */
#ifdef CONFIG_TURN_ON_ALL_SKILLS
#define SKILL_ROM_DATA_AMT 6
#else
#define SKILL_ROM_DATA_AMT 7 /* Unit can learn 7 skills on lv0/5/10/15/20 */
#endif

struct SkillPreloadJConf { u16 skills[SKILL_ROM_DATA_AMT * (UNIT_LEVEL_MAX_RE / 5 + 1)]; };
struct SkillPreloadPConf { u16 skills[SKILL_ROM_DATA_AMT * (UNIT_RECORDED_LEVEL_MAX / 5 + 1)]; };

extern const struct SkillPreloadJConf gSkillPreloadJData[0x100];
extern const struct SkillPreloadPConf gSkillPreloadPData[0x100];

extern struct SkillPreloadJConf const* const gpSkillPreloadJData;
extern struct SkillPreloadPConf const* const gpSkillPreloadPData;

enum SkillAnimPriorityConfig {
	EFX_PRIORITY_LOW = 0x1,
	EFX_PRIORITY_NORMAL,
	EFX_PRIORITY_HIGH,
	EFX_PRIORITY_HIGHHIGH,
};

extern u8 const* const gpEfxSkillAnimPriority;
extern struct EfxAnimConf const* const* const gpEfxSkillAnims;

static inline int GetEfxSkillPriority(const u16 sid)
{
	return gpEfxSkillAnimPriority[sid];
}

/**
 * Skill mapanim
 */
enum mapanimskillfx_idx {
	MAPANIMFX_CHR_L = 0x19C,
	MAPANIMFX_CHR_R = 0x19E,

	MAPANIMFX_PAL = 4,
};

struct ProcMapAnimSkillfx {
	PROC_HEADER;

	u8 pos;
	u32 icon_idx;
	int timer;
	int x, y;
};

extern u8 const* const gpImg_MapAnimSKILL;
extern const struct ProcCmd ProcScr_MapAnimSkillfx[];

void NewSkillMapAnimMini(int x, int y, u16 sid, ProcPtr parent);
bool SkillMapAnimMiniExists(void);

void AnimOnActiveUnit(u16 sid, void (*callback1)(ProcPtr proc), void (*callback2)(ProcPtr proc));
void NewMuSkillAnimOnActiveUnit(u16 sid, void (*callback1)(ProcPtr proc), void (*callback2)(ProcPtr proc));
bool MuSkillAnimExists(void);
void NewMuSkillAnimOnActiveUnitWithDeamon(ProcPtr parent, u16 sid, void (*callback1)(ProcPtr proc), void (*callback2)(ProcPtr proc));
void NewMuAnimOnActiveUnitWithDeamon(ProcPtr parent, void (*callback1)(ProcPtr proc), void (*callback2)(ProcPtr proc));
void RemoveMuForActiveUnit(int delay);

extern const EventScr EventScr_MuSkillAnim[];

/**
 * Skill menu
 */
#define UNIT_MENU_SKILL_AMOUNT 4
extern u16 UnitMenuSkills[UNIT_MENU_SKILL_AMOUNT];
extern struct MenuItemDef const* const gSkillMenuInfos[MAX_SKILL_NUM + 1];
extern struct MenuItemDef const* const* const gpSkillMenuInfos;

u8 UpperMenuSkill_Usability(const struct MenuItemDef* self, int number);
u8 UpperMenuSkill_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);

u8 MenuSkills_OnHelpBox(struct MenuProc* menu, struct MenuItemProc* item);
u8 MenuSkills_Usability(const struct MenuItemDef* self, int number);
int MenuSkills_OnDraw(struct MenuProc* menu, struct MenuItemProc* item);
u8 MenuSkills_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 MenuSkills_Idle(struct MenuProc* menu, struct MenuItemProc* item);
int MenuSkills_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int MenuSkills_Unhover(struct MenuProc* menu, struct MenuItemProc* item);

#define MENU_SKILL_INDEX(menu_item) ((menu_item)->helpMsgId) /* We use rtext msg as index */
#define IS_SKILL_MENU_ITEM(menu_item) ((menu_item)->isAvailable == MenuSkills_Usability)

/**
 * Skill action
 */
typedef bool (*SkillActionFunc_t)(ProcPtr proc);
extern SkillActionFunc_t const* const gpSkillActionFuncTable;

/**
 * Skill scroll
 */
bool IsSkillScrollItem(int item);
char* GetSkillScrollItemName(int item);
int GetSkillScrollItemDescId(int item);
int GetSkillScrollItemUseDescId(int item);
int GetSkillScrollItemIconId(int item);

extern const struct MenuDef RemoveSkillMenuDef;

extern bool TryGetSkillScrollSid(int item, int *outSid);

/**
 * Skill debug list
 */
struct SkillDbgListEnt {
	u8 pid, _pad_;
	u16 sid;
};

enum { SKILL_DBG_LIST_LEN = 20 };
extern EWRAM_DATA struct SkillDbgListEnt gSkillDbgList[SKILL_DBG_LIST_LEN];

void ResetSkillDbgList(void);
void AddSkillDbgList(struct Unit* unit, int sid);
void AddSkillDbgListByPid(int pid, int sid);
void RemoveSkillDbgList(struct Unit* unit, int sid);
void FlushUnitSkillDbgList(struct Unit* unit);

void EMS_SaveSkillDbgList(u8* dst, const u32 size);
void EMS_LoadSkillDbgList(u8* src, const u32 size);
void AppendSkillListViaDebugList(struct Unit* unit, struct SkillList* list, u16* ref);

extern const struct MenuDef RemoveSkillMenuDef;
extern const struct MenuDef PredationSkillMenuDef;
extern const struct MenuDef ScrollScribeMenuDef;
extern const struct ProcCmd ProcScr_SkillScrollUseSoftLock[];
extern const struct ProcCmd ProcScr_PredationSoftLock[];
extern const struct ProcCmd ProcScr_PredationPlusSoftLock[];

/**
 * Miscs
 */

struct SkillExtraInfo {
	s8 priv[10];
};
extern struct SkillExtraInfo const* const gpSkillExtraInfo;
#define SKILL_EFF0(sid) (gpSkillExtraInfo[sid].priv[0])
#define SKILL_EFF1(sid) (gpSkillExtraInfo[sid].priv[1])
#define SKILL_EFF2(sid) (gpSkillExtraInfo[sid].priv[2])
#define SKILL_EFF3(sid) (gpSkillExtraInfo[sid].priv[3])
#define SKILL_EFF4(sid) (gpSkillExtraInfo[sid].priv[4])
#define SKILL_EFF5(sid) (gpSkillExtraInfo[sid].priv[5])
#define SKILL_EFF6(sid) (gpSkillExtraInfo[sid].priv[6])
#define SKILL_EFF7(sid) (gpSkillExtraInfo[sid].priv[7])
#define SKILL_EFF8(sid) (gpSkillExtraInfo[sid].priv[8])
#define SKILL_EFF9(sid) (gpSkillExtraInfo[sid].priv[9])

bool IsSkillLearned(struct Unit* unit, const u16 sid);
void LearnSkill(struct Unit* unit, const u16 sid);
void ForgetSkill(struct Unit* unit, const u16 sid);
void ResetUnitLearnedSkillLists(void);					 /* GameInitHook */
void SaveUnitLearnedSkillLists(u8* dst, const u32 size);   /* SaveData */
void LoadUnitLearnedSkillLists(u8* src, const u32 size);   /* LoadData */

void UnitAutoLoadSkills(struct Unit* unit);
int GetSkillSlot(struct Unit* unit, int sid);
int GetFreeSkillSlot(struct Unit* unit);
bool CanRemoveSkill(struct Unit* unit, const u16 sid);
int RemoveSkill(struct Unit* unit, const u16 sid);
int AddSkill(struct Unit* unit, const u16 sid);
void TryAddSkillLvup(struct Unit* unit, int level);
void TryAddSkillPromotion(struct Unit* unit, int jid);

/**
 * Popups
 */
void ResetPopupSkillStack(void);
void PushSkillListStack(u16 sid);
int PopSkillListStack(void);
bool SkillPopupHasPendingSkills(void);

int PoprGetLen_SkillIcon(struct PopupProc* proc, const struct PopupInstruction* inst);
void PoprDisp_SkillIcon(struct Text* text, const struct PopupInstruction* inst);
int PoprGetLen_SkillName(struct PopupProc* proc, const struct PopupInstruction* inst);
void PoprDisp_SkillName(struct Text* text, const struct PopupInstruction* inst);
bool PopR_SetupLearnSkill(void);
extern const struct PopupInstruction PopupScr_LearnSkill[];
extern const struct PopupInstruction PopupScr_ObtainedSkill[];

/**
 * External MiscSkillEffects
 */
bool GetTeleportationRandomPosition(struct Unit* unit, struct Vec2* out);

/* lucky 7 */
enum skill_lucky_seven_idx {
	LUCKY7_POW = 0,
	LUCKY7_MAG,
	LUCKY7_SKL,
	LUCKY7_SPD,
	LUCKY7_LCK,
	LUCKY7_DEF,
	LUCKY7_RES,
	LUCKY7_MOV,
};

extern const struct ProcCmd ProcScr_SkillScrollUseSoftLock[];

/* Menu skills */
u8 HealingFocus_Usability(const struct MenuItemDef* def, int number);
u8 HealingFocus_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Teleportation_Usability(const struct MenuItemDef* def, int number);
u8 Teleportation_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Portal_Usability(const struct MenuItemDef* def, int number);
u8 Portal_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 LightRune_Usability(const struct MenuItemDef* def, int number);
u8 LightRune_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Mine_Usability(const struct MenuItemDef* def, int number);
u8 Mine_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
int Rally_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int Rally_Unhover(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 Rally_Usability(const struct MenuItemDef* def, int number);
u8 Rally_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
int GoddessDance_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int GoddessDance_Unhover(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 GoddessDance_Usability(const struct MenuItemDef* def, int number);
u8 GoddessDance_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
int Stride_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int Stride_Unhover(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 Stride_Usability(const struct MenuItemDef* def, int number);
u8 Stride_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Swarp_Usability(const struct MenuItemDef* def, int number);
u8 Swarp_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Blacksmith_Usability(const struct MenuItemDef* def, int number);
u8 Blacksmith_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 DivineReprieve_Usability(const struct MenuItemDef* def, int number);
u8 DivineReprieve_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 AssumingCommand_Usability(const struct MenuItemDef* def, int number);
u8 AssumingCommand_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Refuge_Usability(const struct MenuItemDef* def, int number);
u8 Refuge_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Replicate_Usability(const struct MenuItemDef* def, int number);
u8 Replicate_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
struct Unit *GetReplicateLinkedUnit(struct Unit *unit);
void LinkReplicateUnits(struct Unit *unit_a, struct Unit *unit_b);
void ClearReplicateUnitLink(struct Unit *unit);
void SyncReplicateLinkedHp(struct Unit *unit);
int Replicate_OnBattleToUnit(struct BattleUnit *bu, struct Unit *unit);
void Replicate_OnClearUnit(struct Unit *unit);
void Replicate_OnUnitKill(struct Unit *unit);
u8 MasterForge_Usability(const struct MenuItemDef* def, int number);
u8 MasterForge_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 BravelyDefault_Usability(const struct MenuItemDef* def, int number);
u8 BravelyDefault_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Fogger_Usability(const struct MenuItemDef* def, int number);
u8 Fogger_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 StealPlus_Usability(const struct MenuItemDef* def, int number);
u8 StealPlus_Effect(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 Duplicate_Usability(const struct MenuItemDef* def, int number);
u8 Duplicate_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 Reroll_Usability(const struct MenuItemDef* def, int number);
u8 Reroll_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 SpellBlade_Usability(const struct MenuItemDef* def, int number);
u8 SpellBlade_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
int SpellBlade_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int SpellBlade_Unhover(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 FocusEnergy_Usability(const struct MenuItemDef* def, int number);
u8 FocusEnergy_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
int FocusEnergy_Hover(struct MenuProc* menu, struct MenuItemProc* item);
int FocusEnergy_Unhover(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 GorillaTactics_Usability(const struct MenuItemDef* def, int number);
u8 GorillaTactics_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 GraceOfFire_Usability(const struct MenuItemDef* def, int number);
u8 GraceOfFire_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 GraceOfWater_Usability(const struct MenuItemDef* def, int number);
u8 GraceOfWater_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Swap_Usability(const struct MenuItemDef * def, int number);
u8 Swap_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Transform_Usability(const struct MenuItemDef * def, int number);
u8 Transform_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 DrawBack_Usability(const struct MenuItemDef * def, int number);
u8 DrawBack_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Pivot_Usability(const struct MenuItemDef * def, int number);
u8 Pivot_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Reposition_Usability(const struct MenuItemDef * def, int number);
u8 Reposition_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Shove_Usability(const struct MenuItemDef * def, int number);
u8 Shove_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Smite_Usability(const struct MenuItemDef * def, int number);
u8 Smite_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ArdentSacrifice_Usability(const struct MenuItemDef * def, int number);
u8 ArdentSacrifice_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ReciprocalAid_Usability(const struct MenuItemDef * def, int number);
u8 ReciprocalAid_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Sacrifice_Usability(const struct MenuItemDef * def, int number);
u8 Sacrifice_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Capture_Usability(const struct MenuItemDef * def, int number);
u8 Capture_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Doppleganger_Usability(const struct MenuItemDef * def, int number);
u8 Doppleganger_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Dismount_Usability(const struct MenuItemDef * def, int number);
u8 Dismount_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Entrepreneur_Usability(const struct MenuItemDef * def, int number);
u8 Entrepreneur_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Arise_Usability(const struct MenuItemDef * def, int number);
u8 Arise_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 AssignDecoy_Usability(const struct MenuItemDef * def, int number);
u8 AssignDecoy_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 AssignDecoyPlus_Usability(const struct MenuItemDef * def, int number);
u8 AssignDecoyPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ShadowFlash_Usability(const struct MenuItemDef * def, int number);
u8 ShadowFlash_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
int ShadowFlash_Hover(struct MenuProc * menu, struct MenuItemProc * item);
int ShadowFlash_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem);
u8 DeathBlight_Usability(const struct MenuItemDef * def, int number);
u8 DeathBlight_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 LoadstarRush_Usability(const struct MenuItemDef * def, int number);
u8 LoadstarRush_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 WyvernCrash_Usability(const struct MenuItemDef * def, int number);
u8 WyvernCrash_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Sellsword_Usability(const struct MenuItemDef * def, int number);
u8 Sellsword_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Switcher_Usability(const struct MenuItemDef * def, int number);
u8 Switcher_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 LethalityPlus_Usability(const struct MenuItemDef * def, int number);
u8 LethalityPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 GrimReaper_Usability(const struct MenuItemDef * def, int number);
u8 GrimReaper_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Hide_Usability(const struct MenuItemDef * def, int number);
u8 Hide_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Salve_Usability(const struct MenuItemDef * def, int number);
u8 Salve_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 SkillSwap_Usability(const struct MenuItemDef * def, int number);
u8 SkillSwap_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 SkillSwapPlus_Usability(const struct MenuItemDef * def, int number);
u8 SkillSwapPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Shuffle_Usability(const struct MenuItemDef * def, int number);
u8 Shuffle_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ShufflePlus_Usability(const struct MenuItemDef * def, int number);
u8 ShufflePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Persuade_Usability(const struct MenuItemDef * def, int number);
u8 Persuade_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 PersuadePlus_Usability(const struct MenuItemDef * def, int number);
u8 PersuadePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
int Kamikaze_Hover(struct MenuProc * menu, struct MenuItemProc * item);
int Kamikaze_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem);
u8 Kamikaze_Usability(const struct MenuItemDef * def, int number);
u8 Kamikaze_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
int Reinforcements_Hover(struct MenuProc * menu, struct MenuItemProc * item);
int Reinforcements_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem);
u8 Reinforcements_Usability(const struct MenuItemDef * def, int number);
u8 Reinforcements_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ResolvedHeart_Usability(const struct MenuItemDef * def, int number);
u8 ResolvedHeart_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Ballistary_Usability(const struct MenuItemDef * def, int number);
u8 Ballistary_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Bide_Usability(const struct MenuItemDef * def, int number);
u8 Bide_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 CoinFlip_Usability(const struct MenuItemDef * def, int number);
u8 CoinFlip_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Geomancy_Usability(const struct MenuItemDef * def, int number);
u8 Geomancy_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 ScrollScribePlus_Usability(const struct MenuItemDef * def, int number);
u8 ScrollScribePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
int DancePlus_Hover(struct MenuProc * menu, struct MenuItemProc * item);
int DancePlus_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem);
u8 DancePlus_Usability(const struct MenuItemDef * def, int number);
u8 DancePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Acidic_Usability(const struct MenuItemDef * def, int number);
u8 Acidic_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 AcidicPlus_Usability(const struct MenuItemDef * def, int number);
u8 AcidicPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Reinforce_Usability(const struct MenuItemDef * def, int number);
u8 Reinforce_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 SealedZone_Usability(const struct MenuItemDef * def, int number);
u8 SealedZone_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Thrust_Usability(const struct MenuItemDef * def, int number);
u8 Thrust_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 DoubleTime_Usability(const struct MenuItemDef * def, int number);
u8 DoubleTime_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 BloodyAlchemy_Usability(const struct MenuItemDef* def, int number);
u8 BloodyAlchemy_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 EmergencyExit_Usability(const struct MenuItemDef* def, int number);
u8 EmergencyExit_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 EmergencyExitPlus_Usability(const struct MenuItemDef* def, int number);
u8 EmergencyExitPlus_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
void EmergencyExitPlus_ResetMenuState(void);
u8 EmergencyExitPlus_GetCharIdForVisibleIndex(int itemNumber);
bool EmergencyExitPlus_HandleMenuScroll(struct MenuProc * menu);
u8 Songstress_Usability(const struct MenuItemDef* def, int number);
u8 Songstress_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Gungnir_Usability(const struct MenuItemDef * def, int number);
u8 Gungnir_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Transcendence_Usability(const struct MenuItemDef * def, int number);
u8 Transcendence_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Necromancy_Usability(const struct MenuItemDef* def, int number);
u8 Necromancy_OnSelected(struct MenuProc* menu, struct MenuItemProc* menuItem);
u8 SpatialShift_Usability(const struct MenuItemDef * def, int number);
u8 SpatialShift_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 SpatialShiftPlus_Usability(const struct MenuItemDef * def, int number);
u8 SpatialShiftPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);
u8 Hellscape_Usability(const struct MenuItemDef * def, int number);
u8 Hellscape_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);

u8 Transform_Laguz_Usability(const struct MenuItemDef* def, int number);
u8 Transform_Laguz_OnSelected(struct MenuProc* menu, struct MenuItemProc* item);
u8 Transform_Laguz_Effect(struct MenuProc* menu, struct MenuItemProc* item);

/* Skill actions */
bool Action_HealingFocus(ProcPtr proc);
bool Action_Teleportation(ProcPtr parent);
bool Action_Portal(ProcPtr parent);
bool Action_LightRune(ProcPtr parent);
bool Action_Mine(ProcPtr parent);
bool Action_Rally(ProcPtr parent);
bool Action_GoddessDance(ProcPtr parent);
bool Action_Stride(ProcPtr parent);
bool Action_Swarp(ProcPtr parent);
bool Action_Blacksmith(ProcPtr parent);
bool Action_DivineReprieve(ProcPtr parent);
bool Action_Refuge(ProcPtr parent);
bool Action_Replicate(ProcPtr parent);
bool Action_MasterForge(ProcPtr parent);
bool Action_BravelyDefault(ProcPtr parent);
bool Action_Fogger(ProcPtr parent);
bool Action_Duplicate(ProcPtr parent);
bool Action_Reroll(ProcPtr parent);
bool Action_SpellBlade(ProcPtr parent);
bool Action_FocusEnergy(ProcPtr parent);
bool Action_GorillaTactics(ProcPtr parent);
bool Action_GraceOfFire(ProcPtr parent);
bool Action_GraceOfWater(ProcPtr parent);
bool Action_Swap(ProcPtr parent);
bool Action_Transform(ProcPtr parent);
bool Action_DrawBack(ProcPtr parent);
bool Action_Pivot(ProcPtr parent);
bool Action_Reposition(ProcPtr parent);
bool Action_Shove(ProcPtr parent);
bool Action_Smite(ProcPtr parent);
bool Action_StealPlus(ProcPtr parent);
bool Action_ArdentSacrifice(ProcPtr parent);
bool Action_ReciprocalAid(ProcPtr parent);
bool Action_Sacrifice(ProcPtr parent);
bool Action_Capture(ProcPtr parent);
bool Action_Doppleganger(ProcPtr parent);
bool Action_Dismount(ProcPtr parent);
bool Action_Entrepreneur(ProcPtr parent);
bool Action_Arise(ProcPtr parent);
bool Action_AssignDecoy(ProcPtr parent);
bool Action_AssignDecoyPlus(ProcPtr parent);
bool Action_ShadowFlash(ProcPtr parent);
bool Action_DeathBlight(ProcPtr parent);
bool Action_LoadstarRush(ProcPtr parent);
bool Action_WyvernCrash(ProcPtr parent);
bool Action_Sellsword(ProcPtr parent);
bool Action_Switcher(ProcPtr parent);
bool Action_LethalityPlus(ProcPtr parent);
bool Action_GrimReaper(ProcPtr parent);
bool Action_Hide(ProcPtr parent);
bool Action_Salve(ProcPtr parent);
bool Action_SkillSwap(ProcPtr parent);
bool Action_SkillSwapPlus(ProcPtr parent);
bool Action_Shuffle(ProcPtr parent);
bool Action_ShufflePlus(ProcPtr parent);
bool Action_Persuade(ProcPtr parent);
bool Action_PersuadePlus(ProcPtr parent);
bool Action_Kamikaze(ProcPtr parent);
bool Action_Reinforcements(ProcPtr parent);
bool Action_ResolvedHeart(ProcPtr parent);
bool Action_Ballistary(ProcPtr parent);
bool Action_Bide(ProcPtr parent);
bool Action_CoinFlip(ProcPtr parent);
bool Action_Geomancy(ProcPtr parent);
bool Action_ScrollScribePlus(ProcPtr parent);
bool Action_Dance(ProcPtr parent);
bool Action_DancePlus(ProcPtr parent);
bool Action_Acidic(ProcPtr parent);
bool Action_AcidicPlus(ProcPtr parent);
bool Action_Reinforce(ProcPtr parent);
bool Action_SealedZone(ProcPtr parent);
bool Action_Thrust(ProcPtr parent);
bool Action_DoubleTime(ProcPtr parent);
bool Action_BloodyAlchemy(ProcPtr parent);
bool Action_EmergencyExit(ProcPtr parent);
bool Action_EmergencyExitPlus(ProcPtr parent);
bool Action_Songstress(ProcPtr parent);
bool Action_Gungnir(ProcPtr parent);
bool Action_Transcendence(ProcPtr parent);
bool Action_Necromancy(ProcPtr parent);
bool Action_SpatialShift(ProcPtr parent);
bool Action_SpatialShiftPlus(ProcPtr parent);
bool Action_AssumingCommand(ProcPtr parent);
void PostAction_HellscapeVoidTile(ProcPtr proc);
bool Action_Hellscape(ProcPtr parent);
bool PrePhase_HellscapeTrapDecay(ProcPtr parent);

u8 Override_Usability(const struct MenuItemDef *def, int number);
u8 Override_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
bool Action_Override(ProcPtr parent);

bool Action_Transform_Laguz(ProcPtr parent);

u8 SummonPlusCommandEffect(struct MenuProc * menu, struct MenuItemProc * menuItem);