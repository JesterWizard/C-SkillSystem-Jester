#include "common-chax.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "shield.h"
#include "constants/skills.h"
#include "jester_headers/custom-arrays.h"

/**
 * 0: generic use
 * 1: battle actor
 * 2: battle target
 */
extern struct SkillList sSkillList[3];
#define SkillListGeneric      (&sSkillList[0])
#define SkillListBattleActor  (&sSkillList[1])
#define SkillListBattleTarget (&sSkillList[2])

/**
 * Fast list
 */
extern u32 sSkillFastList[0x40];
#define SkillFastListActor  (&sSkillFastList[0])
#define SkillFastListTarget (&sSkillFastList[0x20])

extern void (*gpExternalSkillListGenerator)(struct Unit * unit, struct SkillList * list, u16 * ref);

// Lookup table: maps plus-skill ID → base-skill ID.
// 0 means “no mapping” (skill not a plus skill).
const u16 gSkillUpgradePlusLookup[MAX_SKILL_NUM + 1] = {
    // We’ll generate these entries at compile-time using designated initializers:
    #if defined(SID_NoGuard) && defined(SID_NoGuardPlus)
        [SID_NoGuardPlus] = SID_NoGuard,
    #endif
    #if defined(SID_Canto) && defined(SID_CantoPlus)
        [SID_CantoPlus] = SID_Canto,
    #endif
    #if defined(SID_Bladegift) && defined(SID_BladegiftPlus)
        [SID_BladegiftPlus] = SID_Bladegift,
    #endif
    #if defined(SID_Piercegift) && defined(SID_PiercegiftPlus)
        [SID_PiercegiftPlus] = SID_Piercegift,
    #endif
    #if defined(SID_Gracegift) && defined(SID_GracegiftPlus)
        [SID_GracegiftPlus] = SID_Gracegift,
    #endif
    #if defined(SID_Arcgift) && defined(SID_ArcgiftPlus)
        [SID_ArcgiftPlus] = SID_Arcgift,
    #endif
    #if defined(SID_Hackgift) && defined(SID_HackgiftPlus)
        [SID_HackgiftPlus] = SID_Hackgift,
    #endif
    #if defined(SID_Stormgift) && defined(SID_StormgiftPlus)
        [SID_StormgiftPlus] = SID_Stormgift,
    #endif
    #if defined(SID_ShadowGift) && defined(SID_ShadowGiftPlus)
        [SID_ShadowGiftPlus] = SID_ShadowGift,
    #endif
    #if defined(SID_LightGift) && defined(SID_LightGiftPlus)
        [SID_LightGiftPlus] = SID_LightGift,
    #endif
    #if defined(SID_Fury) && defined(SID_FuryPlus)
        [SID_FuryPlus] = SID_Fury,
    #endif
    #if defined(SID_Steal) && defined(SID_StealPlus)
        [SID_StealPlus] = SID_Steal,
    #endif
    #if defined(SID_Charge) && defined(SID_ChargePlus)
        [SID_ChargePlus] = SID_Charge,
    #endif
    #if defined(SID_Shade) && defined(SID_ShadePlus)
        [SID_ShadePlus] = SID_Shade,
    #endif
    #if defined(SID_Renewal) && defined(SID_RenewalPlus)
        [SID_RenewalPlus] = SID_Renewal,
    #endif
    #if defined(SID_Barricade) && defined(SID_BarricadePlus)
        [SID_BarricadePlus] = SID_Barricade,
    #endif
    #if defined(SID_Blacksmith) && defined(SID_BlacksmithPlus)
        [SID_BlacksmithPlus] = SID_Blacksmith,
    #endif
    #if defined(SID_Armsthrift) && defined(SID_ArmsthriftPlus)
        [SID_ArmsthriftPlus] = SID_Armsthrift,
    #endif
    #if defined(SID_AlertStance) && defined(SID_AlertStancePlus)
        [SID_AlertStancePlus] = SID_AlertStance,
    #endif
    #if defined(SID_LimitBreaker) && defined(SID_LimitBreakerPlus)
        [SID_LimitBreakerPlus] = SID_LimitBreaker,
    #endif
    #if defined(SID_WaterWalking) && defined(SID_WaterWalkingPlus)
        [SID_WaterWalkingPlus] = SID_WaterWalking,
    #endif
    #if defined(SID_MountainClimber) && defined(SID_MountainClimberPlus)
        [SID_MountainClimberPlus] = SID_MountainClimber,
    #endif
    #if defined(SID_TriangleAdept) && defined(SID_TriangleAdeptPlus)
        [SID_TriangleAdeptPlus] = SID_TriangleAdept,
    #endif
    #if defined(SID_Predation) && defined(SID_PredationPlus)
        [SID_PredationPlus] = SID_Predation,
    #endif
    #if defined(SID_Alacrity) && defined(SID_AlacrityPlus)
        [SID_AlacrityPlus] = SID_Alacrity,
    #endif
    #if defined(SID_Summon) && defined(SID_SummonPlus)
        [SID_SummonPlus] = SID_Summon,
    #endif
    #if defined(SID_HugePower) && defined(SID_HugePowerPlus)
        [SID_HugePowerPlus] = SID_HugePower,
    #endif
    #if defined(SID_DualWield) && defined(SID_DualWieldPlus)
        [SID_DualWieldPlus] = SID_DualWield,
    #endif
    #if defined(SID_Shuffle) && defined(SID_ShufflePlus)
        [SID_ShufflePlus] = SID_Shuffle,
    #endif
    #if defined(SID_Persuade) && defined(SID_PersuadePlus)
        [SID_PersuadePlus] = SID_Persuade,
    #endif
    #if defined(SID_ScrollScribe) && defined(SID_ScrollScribePlus)
        [SID_ScrollScribePlus] = SID_ScrollScribe,
    #endif
    #if defined(SID_Dance) && defined(SID_DancePlus)
        [SID_DancePlus] = SID_Dance,
    #endif
    #if defined(SID_TowerShield) && defined(SID_TowerShieldPlus)
        [SID_TowerShieldPlus] = SID_TowerShield,
    #endif
    #if defined(SID_SteadyRider) && defined(SID_SteadyRiderPlus)
        [SID_SteadyRiderPlus] = SID_SteadyRider,
    #endif
    #if defined(SID_Mercy) && defined(SID_MercyPlus)
        [SID_MercyPlus] = SID_Mercy,
    #endif
    #if defined(SID_Shotput) && defined(SID_ShotputPlus)
        [SID_ShotputPlus] = SID_Shotput,
    #endif
    #if defined(SID_MPBoost) && defined(SID_MPBoostPlus)
        [SID_MPBoostPlus] = SID_MPBoost,
    #endif
    #if defined(SID_AssignDecoy) && defined(SID_AssignDecoyPlus)
        [SID_AssignDecoyPlus] = SID_AssignDecoy,
    #endif
    #if defined(SID_Galeforce) && defined(SID_GaleforcePlus)
        [SID_GaleforcePlus] = SID_Galeforce,
    #endif
};

const u16 gSkillUpgradeBaseLookup[MAX_SKILL_NUM + 1] = {
    #if defined(SID_NoGuard) && defined(SID_NoGuardPlus)
        [SID_NoGuard] = SID_NoGuardPlus,
    #endif
    #if defined(SID_Canto) && defined(SID_CantoPlus)
        [SID_Canto] = SID_CantoPlus,
    #endif
    #if defined(SID_Bladegift) && defined(SID_BladegiftPlus)
        [SID_Bladegift] = SID_BladegiftPlus,
    #endif
    #if defined(SID_Piercegift) && defined(SID_PiercegiftPlus)
        [SID_Piercegift] = SID_PiercegiftPlus,
    #endif
    #if defined(SID_Gracegift) && defined(SID_GracegiftPlus)
        [SID_Gracegift] = SID_GracegiftPlus,
    #endif
    #if defined(SID_Arcgift) && defined(SID_ArcgiftPlus)
        [SID_Arcgift] = SID_ArcgiftPlus,
    #endif
    #if defined(SID_Hackgift) && defined(SID_HackgiftPlus)
        [SID_Hackgift] = SID_HackgiftPlus,
    #endif
    #if defined(SID_Stormgift) && defined(SID_StormgiftPlus)
        [SID_Stormgift] = SID_StormgiftPlus,
    #endif
    #if defined(SID_ShadowGift) && defined(SID_ShadowGiftPlus)
        [SID_ShadowGift] = SID_ShadowGiftPlus,
    #endif
    #if defined(SID_LightGift) && defined(SID_LightGiftPlus)
        [SID_LightGift] = SID_LightGiftPlus,
    #endif
    #if defined(SID_Fury) && defined(SID_FuryPlus)
        [SID_Fury] = SID_FuryPlus,
    #endif
    #if defined(SID_Steal) && defined(SID_StealPlus)
        [SID_Steal] = SID_StealPlus,
    #endif
    #if defined(SID_Charge) && defined(SID_ChargePlus)
        [SID_Charge] = SID_ChargePlus,
    #endif
    #if defined(SID_Shade) && defined(SID_ShadePlus)
        [SID_Shade] = SID_ShadePlus,
    #endif
    #if defined(SID_Renewal) && defined(SID_RenewalPlus)
        [SID_Renewal] = SID_RenewalPlus,
    #endif
    #if defined(SID_Barricade) && defined(SID_BarricadePlus)
        [SID_Barricade] = SID_BarricadePlus,
    #endif
    #if defined(SID_Blacksmith) && defined(SID_BlacksmithPlus)
        [SID_Blacksmith] = SID_BlacksmithPlus,
    #endif
    #if defined(SID_Armsthrift) && defined(SID_ArmsthriftPlus)
        [SID_Armsthrift] = SID_ArmsthriftPlus,
    #endif
    #if defined(SID_AlertStance) && defined(SID_AlertStancePlus)
        [SID_AlertStance] = SID_AlertStancePlus,
    #endif
    #if defined(SID_LimitBreaker) && defined(SID_LimitBreakerPlus)
        [SID_LimitBreaker] = SID_LimitBreakerPlus,
    #endif
    #if defined(SID_WaterWalking) && defined(SID_WaterWalkingPlus)
        [SID_WaterWalking] = SID_WaterWalkingPlus,
    #endif
    #if defined(SID_MountainClimber) && defined(SID_MountainClimberPlus)
        [SID_MountainClimber] = SID_MountainClimberPlus,
    #endif
    #if defined(SID_TriangleAdept) && defined(SID_TriangleAdeptPlus)
        [SID_TriangleAdept] = SID_TriangleAdeptPlus,
    #endif
    #if defined(SID_Predation) && defined(SID_PredationPlus)
        [SID_Predation] = SID_PredationPlus,
    #endif
    #if defined(SID_Alacrity) && defined(SID_AlacrityPlus)
        [SID_Alacrity] = SID_AlacrityPlus,
    #endif
    #if defined(SID_Summon) && defined(SID_SummonPlus)
        [SID_Summon] = SID_SummonPlus,
    #endif
    #if defined(SID_HugePower) && defined(SID_HugePowerPlus)
        [SID_HugePower] = SID_HugePowerPlus,
    #endif
    #if defined(SID_DualWield) && defined(SID_DualWieldPlus)
        [SID_DualWield] = SID_DualWieldPlus,
    #endif
    #if defined(SID_Shuffle) && defined(SID_ShufflePlus)
        [SID_Shuffle] = SID_ShufflePlus,
    #endif
    #if defined(SID_Persuade) && defined(SID_PersuadePlus)
        [SID_Persuade] = SID_PersuadePlus,
    #endif
    #if defined(SID_ScrollScribe) && defined(SID_ScrollScribePlus)
        [SID_ScrollScribe] = SID_ScrollScribePlus,
    #endif
    #if defined(SID_Dance) && defined(SID_DancePlus)
        [SID_Dance] = SID_DancePlus,
    #endif
    #if defined(SID_TowerShield) && defined(SID_TowerShieldPlus)
        [SID_TowerShield] = SID_TowerShieldPlus,
    #endif
    #if defined(SID_SteadyRider) && defined(SID_SteadyRiderPlus)
        [SID_SteadyRider] = SID_SteadyRiderPlus,
    #endif
    #if defined(SID_Mercy) && defined(SID_MercyPlus)
        [SID_Mercy] = SID_MercyPlus,
    #endif
    #if defined(SID_Shotput) && defined(SID_ShotputPlus)
        [SID_Shotput] = SID_ShotputPlus,
    #endif
    #if defined(SID_GridMaster) && defined(SID_GridMasterPlus)
        [SID_GridMaster] = SID_GridMasterPlus,
    #endif
    #if defined(SID_MPBoost) && defined(SID_MPBoostPlus)
        [SID_MPBoost] = SID_MPBoostPlus,
    #endif
    #if defined(SID_AssignDecoyPlus) && defined(SID_AssignDecoy)
        [SID_AssignDecoy] = SID_AssignDecoyPlus,
    #endif
    #if defined(SID_GaleforcePlus) && defined(SID_Galeforce)
        [SID_Galeforce] = SID_GaleforcePlus,
    #endif
};

// Function to find the "plus" version
int getPlusVersion(int sid)
{
    if (sid < 0 || sid > MAX_SKILL_NUM)
        return sid;

    const u16 plus = gSkillUpgradeBaseLookup[sid];
    return plus ? plus : sid;
}

void GenerateSkillListExt(struct Unit *unit, struct SkillList *list)
{
	#define ADD_LIST(skill_index) \
	do { \
		int __tmp_sid = skill_index; \
		if (upgrade) \
			__tmp_sid = getPlusVersion(__tmp_sid); \
		if (COMMON_SKILL_VALID(__tmp_sid) && !tmp_list[__tmp_sid]) { \
			tmp_list[__tmp_sid] = true; \
			list->sid[list->amt++] = __tmp_sid; \
		} \
	} while (0)

	int i, weapon;
	const struct ShieldInfo *shield;
	int pid = UNIT_CHAR_ID(unit);
	int jid = UNIT_CLASS_ID(unit);
	u16 *tmp_list = (u16 *)gGenericBuffer;
	bool upgrade = false;

#if defined(SID_Upgrade) && (COMMON_SKILL_VALID(SID_Upgrade))
	if (SkillTester(unit, SID_Upgrade))
		upgrade = true;
#endif

	memset(list, 0, sizeof(*list));

	if (!UNIT_IS_VALID(unit)) {
		WriteUnitList(unit, &list->header);
		return;
	}

	WARN_GENERIC_BUF_USED;
#ifdef CONFIG_TURN_ON_ALL_SKILLS
	memset(tmp_list, 0, (MAX_SKILL_NUM + 1) * sizeof(u16));
#else
    memset(tmp_list, 0, MAX_SKILL_NUM + 1);
#endif

	/* equippable */
	for (i = 0; i < UNIT_RAM_SKILLS_LEN; i++)
	{
#ifdef CONFIG_TURN_ON_ALL_SKILLS
		ADD_LIST(GET_UNIT_SKILL(unit, i));
#else
		ADD_LIST(UNIT_RAM_SKILLS(unit)[i]);
#endif
	}

	/* person */
	ADD_LIST(gpConstSkillTable_Person[pid * 2 + 0]);
	ADD_LIST(gpConstSkillTable_Person[pid * 2 + 1]);

	/* job */
	ADD_LIST(gpConstSkillTable_Job[jid * 2 + 0]);
	ADD_LIST(gpConstSkillTable_Job[jid * 2 + 1]);

	/* item */
	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u8 iid = ITEM_INDEX(unit->items[i]);

		if (iid == ITEM_NONE)
			break;

		ADD_LIST(gpConstSkillTable_Item[iid * 2 + 0]);
		ADD_LIST(gpConstSkillTable_Item[iid * 2 + 1]);
	}

	/* weapon & shield */
	if (unit == &gBattleActor.unit || unit == &gBattleTarget.unit) {
		struct BattleUnit *bu = (struct BattleUnit *)unit;

		weapon = ITEM_INDEX(bu->weapon);
		shield = GetBattleUnitShield(bu);
	} else {
		weapon = ITEM_INDEX(GetUnitEquippedWeapon(unit));
		shield = GetUnitShield(unit);
	}

	if (weapon != ITEM_NONE) {
		ADD_LIST(gpConstSkillTable_Weapon[weapon * 2 + 0]);
		ADD_LIST(gpConstSkillTable_Weapon[weapon * 2 + 1]);
	}

	if (shield) {
		ADD_LIST(shield->skills[0]);
		ADD_LIST(shield->skills[1]);
	}

	/* external */
	AppendSkillListViaDebugList(unit, list, tmp_list);

	if (gpExternalSkillListGenerator)
		gpExternalSkillListGenerator(unit, list, tmp_list);

	WARN_GENERIC_BUF_RELEASED;
	WriteUnitList(unit, &list->header);

	#undef ADD_LIST
}

void ForceUpdateUnitSkillList(struct Unit *unit)
{
	struct SkillList *list = SkillListGeneric;

	if (unit == &gBattleActor.unit)
		list = SkillListBattleActor;
	else if (unit == &gBattleTarget.unit)
		list = SkillListBattleTarget;

	GenerateSkillListExt(unit, list);
}

struct SkillList *GetUnitSkillList(struct Unit *unit)
{
	struct SkillList *list = SkillListGeneric;

	if (unit == &gBattleActor.unit)
		list = SkillListBattleActor;
	else if (unit == &gBattleTarget.unit)
		list = SkillListBattleTarget;

	if (!JudgeUnitList(unit, &list->header)) {
		Errorf("Ops! regenerate skilllist: uid=%02X, pid=%02X", unit->index & 0xFF, UNIT_CHAR_ID(unit));

		GenerateSkillListExt(unit, list);
	}

	return list;
}

bool SkillListTester(struct Unit *unit, const u16 sid)
{
	int i;
	struct SkillList *list = GetUnitSkillList(unit);

	for (i = 0; i < list->amt; i++)
		if (list->sid[i] == sid)
			return true;

	return false;
}

void SetupBattleSkillList(void)
{
	GenerateSkillListExt(&gBattleActor.unit,  SkillListBattleActor);
	GenerateSkillListExt(&gBattleTarget.unit, SkillListBattleTarget);
}

void DisableUnitSkilLList(struct Unit *unit)
{
	struct SkillList *list = GetUnitSkillList(unit);

	list->amt = 0;
}

void ResetSkillLists(void)
{
	memset(&sSkillList, 0, sizeof(sSkillList));
}

STATIC_DECLAR void SetupBattleSkillFastList(void)
{
	int i;
	struct SkillList *list;
	u32 *fast_list;

	Assert(MAX_SKILL_NUM <= (sizeof(sSkillFastList) * 8 / 2));

	CpuFastFill(0, sSkillFastList, sizeof(sSkillFastList));

	/**
	 * Actor
	 */
	list = GetUnitSkillList(&gBattleActor.unit);
	fast_list = SkillFastListActor;

	for (i = 0; i < list->amt; i++)
		_BIT_SET(fast_list, list->sid[i]);

	/**
	 * Target
	 */
	list = GetUnitSkillList(&gBattleTarget.unit);
	fast_list = SkillFastListTarget;

	for (i = 0; i < list->amt; i++)
		_BIT_SET(fast_list, list->sid[i]);
}

#if 0
bool _BattleFastSkillTester(struct BattleUnit *bu, const u16 sid)
{
	u32 *fast_list;

	if (bu == &gBattleActor)
		fast_list = SkillFastListActor;
	else if (bu == &gBattleTarget)
		fast_list = SkillFastListTarget;
	else
		return false;

	return _BIT_CHK(fast_list, sid);
}
#endif

void AppendBattleUnitSkillList(struct BattleUnit *bu, u16 skill)
{
	u32 i;
	struct SkillList *list;
	u32 *fast_list;

	list = GetUnitSkillList(&bu->unit);

	/**
	 * Fulfilled
	 */
	if (list->amt >= ARRAY_COUNT(list->sid))
		return;

	/**
	 * Already in force
	 */
	for (i = 0; i < list->amt; i++) {
		if (skill == list->sid[i])
			return;
	}

	/**
	 * Add to list
	 */
	list->sid[list->amt++] = skill;


	if (bu == &gBattleActor)
		fast_list = SkillFastListActor;
	else if (bu == &gBattleTarget)
		fast_list = SkillFastListTarget;
	else
		return;

	/**
	 * Add to fast-list
	 */
	_BIT_SET(fast_list, skill);
}

void UnitToBattle_SetupSkillList(struct Unit *unit, struct BattleUnit *bu)
{
	FORCE_DECLARE bool nihil_on_actor, nihil_on_target;

	/**
	 * Here we hold 3 assumption:
	 *
	 * 1. UnitToBattle routine stands at the very beginning of battle-generate
	 * 2. Battle target initialization is behind actor.
	 * 3. No skill activcated before during function: InitBattleUnit()
	 */
	if (bu == &gBattleTarget) {
		SetupBattleSkillList();

#if (defined(SID_Nihil) && COMMON_SKILL_VALID(SID_Nihil))
		nihil_on_actor  = SkillListTester(&gBattleTarget.unit,  SID_Nihil);
		nihil_on_target = SkillListTester(&gBattleActor.unit, SID_Nihil);

		if (nihil_on_actor)
			DisableUnitSkilLList(&gBattleActor.unit);

		if (nihil_on_target)
			DisableUnitSkilLList(&gBattleTarget.unit);
#endif

		SetupBattleSkillFastList();
	}
}