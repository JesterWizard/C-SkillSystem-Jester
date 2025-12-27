#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/gfx.h"
#include "Skills_Capacity.h"
#include "Skills_Price.h"

const struct SkillInfo gSkillInfos[MAX_SKILL_NUM + 1] = {

#if (defined(SID_HpBonus) && COMMON_SKILL_VALID(SID_HpBonus))
    [SID_HpBonus] = {
        .name = MSG_SKILL_HpBonus_NAME,
        .desc = MSG_SKILL_HpBonus_DESC,
        .capacity = MSG_SKILL_HpBonus_CAPACITY,
        .price = MSG_SKILL_HpBonus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_StrBonus) && COMMON_SKILL_VALID(SID_StrBonus))
    [SID_StrBonus] = {
        .name = MSG_SKILL_StrBonus_NAME,
        .desc = MSG_SKILL_StrBonus_DESC,
        .capacity = MSG_SKILL_StrBonus_CAPACITY,
        .price = MSG_SKILL_StrBonus_PRICE,
        .icon = GFX_SkillIcon_StrBonus,
    },
#endif

#if (defined(SID_MagBonus) && COMMON_SKILL_VALID(SID_MagBonus))
    [SID_MagBonus] = {
        .name = MSG_SKILL_MagBonus_NAME,
        .desc = MSG_SKILL_MagBonus_DESC,
        .capacity = MSG_SKILL_MagBonus_CAPACITY,
        .price = MSG_SKILL_MagBonus_PRICE,
        .icon = GFX_SkillIcon_MagBonus,
    },
#endif

#if (defined(SID_SklBonus) && COMMON_SKILL_VALID(SID_SklBonus))
    [SID_SklBonus] = {
        .name = MSG_SKILL_SklBonus_NAME,
        .desc = MSG_SKILL_SklBonus_DESC,
        .capacity = MSG_SKILL_SklBonus_CAPACITY,
        .price = MSG_SKILL_SklBonus_PRICE,
        .icon = GFX_SkillIcon_SklBonus,
    },
#endif

#if (defined(SID_SpdBonus) && COMMON_SKILL_VALID(SID_SpdBonus))
    [SID_SpdBonus] = {
        .name = MSG_SKILL_SpdBonus_NAME,
        .desc = MSG_SKILL_SpdBonus_DESC,
        .capacity = MSG_SKILL_SpdBonus_CAPACITY,
        .price = MSG_SKILL_SpdBonus_PRICE,
        .icon = GFX_SkillIcon_SpdBonus,
    },
#endif

#if (defined(SID_LckBonus) && COMMON_SKILL_VALID(SID_LckBonus))
    [SID_LckBonus] = {
        .name = MSG_SKILL_LukBonus_NAME,
        .desc = MSG_SKILL_LukBonus_DESC,
        .capacity = MSG_SKILL_LukBonus_CAPACITY,
        .price = MSG_SKILL_LukBonus_PRICE,
        .icon = GFX_SkillIcon_LukBonus,
    },
#endif

#if (defined(SID_DefBonus) && COMMON_SKILL_VALID(SID_DefBonus))
    [SID_DefBonus] = {
        .name = MSG_SKILL_DefBonus_NAME,
        .desc = MSG_SKILL_DefBonus_DESC,
        .capacity = MSG_SKILL_DefBonus_CAPACITY,
        .price = MSG_SKILL_DefBonus_PRICE,
        .icon = GFX_SkillIcon_DefBonus,
    },
#endif

#if (defined(SID_ResBonus) && COMMON_SKILL_VALID(SID_ResBonus))
    [SID_ResBonus] = {
        .name = MSG_SKILL_ResBonus_NAME,
        .desc = MSG_SKILL_ResBonus_DESC,
        .capacity = MSG_SKILL_ResBonus_CAPACITY,
        .price = MSG_SKILL_ResBonus_PRICE,
        .icon = GFX_SkillIcon_ResBonus,
    },
#endif

#if (defined(SID_MovBonus) && COMMON_SKILL_VALID(SID_MovBonus))
    [SID_MovBonus] = {
        .name = MSG_SKILL_MovBonus_NAME,
        .desc = MSG_SKILL_MovBonus_DESC,
        .capacity = MSG_SKILL_MovBonus_CAPACITY,
        .price = MSG_SKILL_MovBonus_PRICE,
        .icon = GFX_SkillIcon_MovBonus,
    },
#endif

#if (defined(SID_DefiantStr) && COMMON_SKILL_VALID(SID_DefiantStr))
    [SID_DefiantStr] = {
        .name = MSG_SKILL_DefiantStr_NAME,
        .desc = MSG_SKILL_DefiantStr_DESC,
        .capacity = MSG_SKILL_DefiantStr_CAPACITY,
        .price = MSG_SKILL_DefiantStr_PRICE,
        .icon = GFX_SkillIcon_DefiantStr,
    },
#endif

#if (defined(SID_DefiantMag) && COMMON_SKILL_VALID(SID_DefiantMag))
    [SID_DefiantMag] = {
        .name = MSG_SKILL_DefiantMag_NAME,
        .desc = MSG_SKILL_DefiantMag_DESC,
        .capacity = MSG_SKILL_DefiantMag_CAPACITY,
        .price = MSG_SKILL_DefiantMag_PRICE,
        .icon = GFX_SkillIcon_DefiantMag,
    },
#endif

#if (defined(SID_DefiantSkl) && COMMON_SKILL_VALID(SID_DefiantSkl))
    [SID_DefiantSkl] = {
        .name = MSG_SKILL_DefiantSkl_NAME,
        .desc = MSG_SKILL_DefiantSkl_DESC,
        .capacity = MSG_SKILL_DefiantSkl_CAPACITY,
        .price = MSG_SKILL_DefiantSkl_PRICE,
        .icon = GFX_SkillIcon_DefiantSkl,
    },
#endif

#if (defined(SID_DefiantSpd) && COMMON_SKILL_VALID(SID_DefiantSpd))
    [SID_DefiantSpd] = {
        .name = MSG_SKILL_DefiantSpd_NAME,
        .desc = MSG_SKILL_DefiantSpd_DESC,
        .capacity = MSG_SKILL_DefiantSpd_CAPACITY,
        .price = MSG_SKILL_DefiantSpd_PRICE,
        .icon = GFX_SkillIcon_DefiantSpd,
    },
#endif

#if (defined(SID_DefiantLck) && COMMON_SKILL_VALID(SID_DefiantLck))
    [SID_DefiantLck] = {
        .name = MSG_SKILL_DefiantLck_NAME,
        .desc = MSG_SKILL_DefiantLck_DESC,
        .capacity = MSG_SKILL_DefiantLck_CAPACITY,
        .price = MSG_SKILL_DefiantLck_PRICE,
        .icon = GFX_SkillIcon_DefiantLck,
    },
#endif

#if (defined(SID_DefiantDef) && COMMON_SKILL_VALID(SID_DefiantDef))
    [SID_DefiantDef] = {
        .name = MSG_SKILL_DefiantDef_NAME,
        .desc = MSG_SKILL_DefiantDef_DESC,
        .capacity = MSG_SKILL_DefiantDef_CAPACITY,
        .price = MSG_SKILL_DefiantDef_PRICE,
        .icon = GFX_SkillIcon_DefiantDef,
    },
#endif

#if (defined(SID_DefiantRes) && COMMON_SKILL_VALID(SID_DefiantRes))
    [SID_DefiantRes] = {
        .name = MSG_SKILL_DefiantRes_NAME,
        .desc = MSG_SKILL_DefiantRes_DESC,
        .capacity = MSG_SKILL_DefiantRes_CAPACITY,
        .price = MSG_SKILL_DefiantRes_PRICE,
        .icon = GFX_SkillIcon_DefiantRes,
    },
#endif

#if (defined(SID_DefiantCrit) && COMMON_SKILL_VALID(SID_DefiantCrit))
    [SID_DefiantCrit] = {
        .name = MSG_SKILL_DefiantCrit_NAME,
        .desc = MSG_SKILL_DefiantCrit_DESC,
        .capacity = MSG_SKILL_DefiantCrit_CAPACITY,
        .price = MSG_SKILL_DefiantCrit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DefiantAvoid) && COMMON_SKILL_VALID(SID_DefiantAvoid))
    [SID_DefiantAvoid] = {
        .name = MSG_SKILL_DefiantAvoid_NAME,
        .desc = MSG_SKILL_DefiantAvoid_DESC,
        .capacity = MSG_SKILL_DefiantAvoid_CAPACITY,
        .price = MSG_SKILL_DefiantAvoid_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Fury) && COMMON_SKILL_VALID(SID_Fury))
    [SID_Fury] = {
        .name = MSG_SKILL_Fury_NAME,
        .desc = MSG_SKILL_Fury_DESC,
        .capacity = MSG_SKILL_Fury_CAPACITY,
        .price = MSG_SKILL_Fury_PRICE,
        .icon = GFX_SkillIcon_Fury,
    },
#endif

#if (defined(SID_FuryPlus) && COMMON_SKILL_VALID(SID_FuryPlus))
    [SID_FuryPlus] = {
        .name = MSG_SKILL_FuryPlus_NAME,
        .desc = MSG_SKILL_FuryPlus_DESC,
        .capacity = MSG_SKILL_FuryPlus_CAPACITY,
        .price = MSG_SKILL_FuryPlus_PRICE,
        .icon = GFX_SkillIcon_FuryPlus,
    },
#endif

#if (defined(SID_FortressDef) && COMMON_SKILL_VALID(SID_FortressDef))
    [SID_FortressDef] = {
        .name = MSG_SKILL_FortressDef_NAME,
        .desc = MSG_SKILL_FortressDef_DESC,
        .capacity = MSG_SKILL_FortressDef_CAPACITY,
        .price = MSG_SKILL_FortressDef_PRICE,
        .icon = GFX_SkillIcon_FortressDef,
    },
#endif

#if (defined(SID_FortressRes) && COMMON_SKILL_VALID(SID_FortressRes))
    [SID_FortressRes] = {
        .name = MSG_SKILL_FortressRes_NAME,
        .desc = MSG_SKILL_FortressRes_DESC,
        .capacity = MSG_SKILL_FortressRes_CAPACITY,
        .price = MSG_SKILL_FortressRes_PRICE,
        .icon = GFX_SkillIcon_FortressRes,
    },
#endif

#if (defined(SID_LifeAndDeath) && COMMON_SKILL_VALID(SID_LifeAndDeath))
    [SID_LifeAndDeath] = {
        .name = MSG_SKILL_LifeAndDeath_NAME,
        .desc = MSG_SKILL_LifeAndDeath_DESC,
        .capacity = MSG_SKILL_LifeAndDeath_CAPACITY,
        .price = MSG_SKILL_LifeAndDeath_PRICE,
        .icon = GFX_SkillIcon_LifeAndDeath,
    },
#endif

#if (defined(SID_LuckySeven) && COMMON_SKILL_VALID(SID_LuckySeven))
    [SID_LuckySeven] = {
        .name = MSG_SKILL_LuckySeven_NAME,
        .desc = MSG_SKILL_LuckySeven_DESC,
        .capacity = MSG_SKILL_LuckySeven_CAPACITY,
        .price = MSG_SKILL_LuckySeven_PRICE,
        .icon = GFX_SkillIcon_LuckySeven,
    },
#endif

#if (defined(SID_Nullify) && COMMON_SKILL_VALID(SID_Nullify))
    [SID_Nullify] = {
        .name = MSG_SKILL_Nullify_NAME,
        .desc = MSG_SKILL_Nullify_DESC,
        .capacity = MSG_SKILL_Nullify_CAPACITY,
        .price = MSG_SKILL_Nullify_PRICE,
        .icon = GFX_SkillIcon_Nullify,
    },
#endif

#if (defined(SID_Slayer) && COMMON_SKILL_VALID(SID_Slayer))
    [SID_Slayer] = {
        .name = MSG_SKILL_Slayer_NAME,
        .desc = MSG_SKILL_Slayer_DESC,
        .capacity = MSG_SKILL_Slayer_CAPACITY,
        .price = MSG_SKILL_Slayer_PRICE,
        .icon = GFX_SkillIcon_Slayer,
    },
#endif

#if (defined(SID_Skybreaker) && COMMON_SKILL_VALID(SID_Skybreaker))
    [SID_Skybreaker] = {
        .name = MSG_SKILL_Skybreaker_NAME,
        .desc = MSG_SKILL_Skybreaker_DESC,
        .capacity = MSG_SKILL_Skybreaker_CAPACITY,
        .price = MSG_SKILL_Skybreaker_PRICE,
        .icon = GFX_SkillIcon_Skybreaker,
    },
#endif

#if (defined(SID_Resourceful) && COMMON_SKILL_VALID(SID_Resourceful))
    [SID_Resourceful] = {
        .name = MSG_SKILL_Resourceful_NAME,
        .desc = MSG_SKILL_Resourceful_DESC,
        .capacity = MSG_SKILL_Resourceful_CAPACITY,
        .price = MSG_SKILL_Resourceful_PRICE,
        .icon = GFX_SkillIcon_Resourceful,
    },
#endif

#if (defined(SID_Nihil) && COMMON_SKILL_VALID(SID_Nihil))
    [SID_Nihil] = {
        .name = MSG_SKILL_Nihil_NAME,
        .desc = MSG_SKILL_Nihil_DESC,
        .capacity = MSG_SKILL_Nihil_CAPACITY,
        .price = MSG_SKILL_Nihil_PRICE,
        .icon = GFX_SkillIcon_Nihil,
    },
#endif

#if (defined(SID_CritSword) && COMMON_SKILL_VALID(SID_CritSword))
    [SID_CritSword] = {
        .name = MSG_SKILL_CritSword_NAME,
        .desc = MSG_SKILL_CritSword_DESC,
        .capacity = MSG_SKILL_CritSword_CAPACITY,
        .price = MSG_SKILL_CritSword_PRICE,
        .icon = GFX_SkillIcon_CritSword,
    },
#endif

#if (defined(SID_CritAxe) && COMMON_SKILL_VALID(SID_CritAxe))
    [SID_CritAxe] = {
        .name = MSG_SKILL_CritAxe_NAME,
        .desc = MSG_SKILL_CritAxe_DESC,
        .capacity = MSG_SKILL_CritAxe_CAPACITY,
        .price = MSG_SKILL_CritAxe_PRICE,
        .icon = GFX_SkillIcon_CritAxe,
    },
#endif

#if (defined(SID_CritLance) && COMMON_SKILL_VALID(SID_CritLance))
    [SID_CritLance] = {
        .name = MSG_SKILL_CritLance_NAME,
        .desc = MSG_SKILL_CritLance_DESC,
        .capacity = MSG_SKILL_CritLance_CAPACITY,
        .price = MSG_SKILL_CritLance_PRICE,
        .icon = GFX_SkillIcon_CritLance,
    },
#endif

#if (defined(SID_CritBow) && COMMON_SKILL_VALID(SID_CritBow))
    [SID_CritBow] = {
        .name = MSG_SKILL_CritBow_NAME,
        .desc = MSG_SKILL_CritBow_DESC,
        .capacity = MSG_SKILL_CritBow_CAPACITY,
        .price = MSG_SKILL_CritBow_PRICE,
        .icon = GFX_SkillIcon_CritBow,
    },
#endif

#if (defined(SID_FaireSword) && COMMON_SKILL_VALID(SID_FaireSword))
    [SID_FaireSword] = {
        .name = MSG_SKILL_FaireSword_NAME,
        .desc = MSG_SKILL_FaireSword_DESC,
        .capacity = MSG_SKILL_FaireSword_CAPACITY,
        .price = MSG_SKILL_FaireSword_PRICE,
        .icon = GFX_SkillIcon_FaireSword,
    },
#endif

#if (defined(SID_FaireLance) && COMMON_SKILL_VALID(SID_FaireLance))
    [SID_FaireLance] = {
        .name = MSG_SKILL_FaireLance_NAME,
        .desc = MSG_SKILL_FaireLance_DESC,
        .capacity = MSG_SKILL_FaireLance_CAPACITY,
        .price = MSG_SKILL_FaireLance_PRICE,
        .icon = GFX_SkillIcon_FaireLance,
    },
#endif

#if (defined(SID_FaireAxe) && COMMON_SKILL_VALID(SID_FaireAxe))
    [SID_FaireAxe] = {
        .name = MSG_SKILL_FaireAxe_NAME,
        .desc = MSG_SKILL_FaireAxe_DESC,
        .capacity = MSG_SKILL_FaireAxe_CAPACITY,
        .price = MSG_SKILL_FaireAxe_PRICE,
        .icon = GFX_SkillIcon_FaireAxe,
    },
#endif

#if (defined(SID_FaireBow) && COMMON_SKILL_VALID(SID_FaireBow))
    [SID_FaireBow] = {
        .name = MSG_SKILL_FaireBow_NAME,
        .desc = MSG_SKILL_FaireBow_DESC,
        .capacity = MSG_SKILL_FaireBow_CAPACITY,
        .price = MSG_SKILL_FaireBow_PRICE,
        .icon = GFX_SkillIcon_FaireBow,
    },
#endif

#if (defined(SID_FaireBMag) && COMMON_SKILL_VALID(SID_FaireBMag))
    [SID_FaireBMag] = {
        .name = MSG_SKILL_FaireBMag_NAME,
        .desc = MSG_SKILL_FaireBMag_DESC,
        .capacity = MSG_SKILL_FaireBMag_CAPACITY,
        .price = MSG_SKILL_FaireBMag_PRICE,
        .icon = GFX_SkillIcon_FaireBMag,
    },
#endif

#if (defined(SID_Avoid) && COMMON_SKILL_VALID(SID_Avoid))
    [SID_Avoid] = {
        .name = MSG_SKILL_Avoid_NAME,
        .desc = MSG_SKILL_Avoid_DESC,
        .capacity = MSG_SKILL_Avoid_CAPACITY,
        .price = MSG_SKILL_Avoid_PRICE,
        .icon = GFX_SkillIcon_Avoid,
    },
#endif

#if (defined(SID_AvoidSword) && COMMON_SKILL_VALID(SID_AvoidSword))
    [SID_AvoidSword] = {
        .name = MSG_SKILL_AvoidSword_NAME,
        .desc = MSG_SKILL_AvoidSword_DESC,
        .capacity = MSG_SKILL_AvoidSword_CAPACITY,
        .price = MSG_SKILL_AvoidSword_PRICE,
        .icon = GFX_SkillIcon_AvoidSword,
    },
#endif

#if (defined(SID_RuinedBlade) && COMMON_SKILL_VALID(SID_RuinedBlade))
    [SID_RuinedBlade] = {
        .name = MSG_SKILL_RuinedBlade_NAME,
        .desc = MSG_SKILL_RuinedBlade_DESC,
        .capacity = MSG_SKILL_RuinedBlade_CAPACITY,
        .price = MSG_SKILL_RuinedBlade_PRICE,
        .icon = GFX_SkillIcon_RuinedBlade,
    },
#endif

#if (defined(SID_HeavyBlade) && COMMON_SKILL_VALID(SID_HeavyBlade))
    [SID_HeavyBlade] = {
        .name = MSG_SKILL_HeavyBlade_NAME,
        .desc = MSG_SKILL_HeavyBlade_DESC,
        .capacity = MSG_SKILL_HeavyBlade_CAPACITY,
        .price = MSG_SKILL_HeavyBlade_PRICE,
        .icon = GFX_SkillIcon_HeavyBlade,
    },
#endif

#if (defined(SID_FlashingBlade) && COMMON_SKILL_VALID(SID_FlashingBlade))
    [SID_FlashingBlade] = {
        .name = MSG_SKILL_FlashingBlade_NAME,
        .desc = MSG_SKILL_FlashingBlade_DESC,
        .capacity = MSG_SKILL_FlashingBlade_CAPACITY,
        .price = MSG_SKILL_FlashingBlade_PRICE,
        .icon = GFX_SkillIcon_FlashingBlade,
    },
#endif

#if (defined(SID_HeavyBladePlus) && COMMON_SKILL_VALID(SID_HeavyBladePlus))
    [SID_HeavyBladePlus] = {
        .name = MSG_SKILL_HeavyBladePlus_NAME,
        .desc = MSG_SKILL_HeavyBladePlus_DESC,
        .capacity = MSG_SKILL_HeavyBladePlus_CAPACITY,
        .price = MSG_SKILL_HeavyBladePlus_PRICE,
        .icon = GFX_SkillIcon_HeavyBladePlus,
    },
#endif

#if (defined(SID_FlashingBladePlus) && COMMON_SKILL_VALID(SID_FlashingBladePlus))
    [SID_FlashingBladePlus] = {
        .name = MSG_SKILL_FlashingBladePlus_NAME,
        .desc = MSG_SKILL_FlashingBladePlus_DESC,
        .capacity = MSG_SKILL_FlashingBladePlus_CAPACITY,
        .price = MSG_SKILL_FlashingBladePlus_PRICE,
        .icon = GFX_SkillIcon_FlashingBladePlus,
    },
#endif

#if (defined(SID_LunaAttack) && COMMON_SKILL_VALID(SID_LunaAttack))
    [SID_LunaAttack] = {
        .name = MSG_SKILL_LunaAttack_NAME,
        .desc = MSG_SKILL_LunaAttack_DESC,
        .capacity = MSG_SKILL_LunaAttack_CAPACITY,
        .price = MSG_SKILL_LunaAttack_PRICE,
        .icon = GFX_SkillIcon_LunaAttack,
    },
#endif

#if (defined(SID_SorceryBlade) && COMMON_SKILL_VALID(SID_SorceryBlade))
    [SID_SorceryBlade] = {
        .name = MSG_SKILL_SorceryBlade_NAME,
        .desc = MSG_SKILL_SorceryBlade_DESC,
        .capacity = MSG_SKILL_SorceryBlade_CAPACITY,
        .price = MSG_SKILL_SorceryBlade_PRICE,
        .icon = GFX_SkillIcon_SorceryBlade,
    },
#endif

#if (defined(SID_Fortune) && COMMON_SKILL_VALID(SID_Fortune))
    [SID_Fortune] = {
        .name = MSG_SKILL_Fortune_NAME,
        .desc = MSG_SKILL_Fortune_DESC,
        .capacity = MSG_SKILL_Fortune_CAPACITY,
        .price = MSG_SKILL_Fortune_PRICE,
        .icon = GFX_SkillIcon_Fortune,
    },
#endif

#if (defined(SID_BlueFlame) && COMMON_SKILL_VALID(SID_BlueFlame))
    [SID_BlueFlame] = {
        .name = MSG_SKILL_BlueFlame_NAME,
        .desc = MSG_SKILL_BlueFlame_DESC,
        .capacity = MSG_SKILL_BlueFlame_CAPACITY,
        .price = MSG_SKILL_BlueFlame_PRICE,
        .icon = GFX_SkillIcon_BlueFlame,
    },
#endif

#if (defined(SID_SwordBreaker) && COMMON_SKILL_VALID(SID_SwordBreaker))
    [SID_SwordBreaker] = {
        .name = MSG_SKILL_SwordBreaker_NAME,
        .desc = MSG_SKILL_SwordBreaker_DESC,
        .capacity = MSG_SKILL_SwordBreaker_CAPACITY,
        .price = MSG_SKILL_SwordBreaker_PRICE,
        .icon = GFX_SkillIcon_SwordBreaker,
    },
#endif

#if (defined(SID_AxeBreaker) && COMMON_SKILL_VALID(SID_AxeBreaker))
    [SID_AxeBreaker] = {
        .name = MSG_SKILL_AxeBreaker_NAME,
        .desc = MSG_SKILL_AxeBreaker_DESC,
        .capacity = MSG_SKILL_AxeBreaker_CAPACITY,
        .price = MSG_SKILL_AxeBreaker_PRICE,
        .icon = GFX_SkillIcon_AxeBreaker,
    },
#endif

#if (defined(SID_LanceBreaker) && COMMON_SKILL_VALID(SID_LanceBreaker))
    [SID_LanceBreaker] = {
        .name = MSG_SKILL_LanceBreaker_NAME,
        .desc = MSG_SKILL_LanceBreaker_DESC,
        .capacity = MSG_SKILL_LanceBreaker_CAPACITY,
        .price = MSG_SKILL_LanceBreaker_PRICE,
        .icon = GFX_SkillIcon_LanceBreaker,
    },
#endif

#if (defined(SID_BowBreaker) && COMMON_SKILL_VALID(SID_BowBreaker))
    [SID_BowBreaker] = {
        .name = MSG_SKILL_BowBreaker_NAME,
        .desc = MSG_SKILL_BowBreaker_DESC,
        .capacity = MSG_SKILL_BowBreaker_CAPACITY,
        .price = MSG_SKILL_BowBreaker_PRICE,
        .icon = GFX_SkillIcon_BowBreaker,
    },
#endif

#if (defined(SID_BMagBreaker) && COMMON_SKILL_VALID(SID_BMagBreaker))
    [SID_BMagBreaker] = {
        .name = MSG_SKILL_BMagBreaker_NAME,
        .desc = MSG_SKILL_BMagBreaker_DESC,
        .capacity = MSG_SKILL_BMagBreaker_CAPACITY,
        .price = MSG_SKILL_BMagBreaker_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Vantage) && COMMON_SKILL_VALID(SID_Vantage))
    [SID_Vantage] = {
        .name = MSG_SKILL_Vantage_NAME,
        .desc = MSG_SKILL_Vantage_DESC,
        .capacity = MSG_SKILL_Vantage_CAPACITY,
        .price = MSG_SKILL_Vantage_PRICE,
        .icon = GFX_SkillIcon_Vantage,
    },
#endif

#if (defined(SID_Desperation) && COMMON_SKILL_VALID(SID_Desperation))
    [SID_Desperation] = {
        .name = MSG_SKILL_Desperation_NAME,
        .desc = MSG_SKILL_Desperation_DESC,
        .capacity = MSG_SKILL_Desperation_CAPACITY,
        .price = MSG_SKILL_Desperation_PRICE,
        .icon = GFX_SkillIcon_Desperation,
    },
#endif

#if (defined(SID_QuickRiposte) && COMMON_SKILL_VALID(SID_QuickRiposte))
    [SID_QuickRiposte] = {
        .name = MSG_SKILL_QuickRiposte_NAME,
        .desc = MSG_SKILL_QuickRiposte_DESC,
        .capacity = MSG_SKILL_QuickRiposte_CAPACITY,
        .price = MSG_SKILL_QuickRiposte_PRICE,
        .icon = GFX_SkillIcon_QuickRiposte,
    },
#endif

#if (defined(SID_WaryFighter) && COMMON_SKILL_VALID(SID_WaryFighter))
    [SID_WaryFighter] = {
        .name = MSG_SKILL_WaryFighter_NAME,
        .desc = MSG_SKILL_WaryFighter_DESC,
        .capacity = MSG_SKILL_WaryFighter_CAPACITY,
        .price = MSG_SKILL_WaryFighter_PRICE,
        .icon = GFX_SkillIcon_WaryFighter,
    },
#endif

#if (defined(SID_DoubleLion) && COMMON_SKILL_VALID(SID_DoubleLion))
    [SID_DoubleLion] = {
        .name = MSG_SKILL_DoubleLion_NAME,
        .desc = MSG_SKILL_DoubleLion_DESC,
        .capacity = MSG_SKILL_DoubleLion_CAPACITY,
        .price = MSG_SKILL_DoubleLion_PRICE,
        .icon = GFX_SkillIcon_DoubleLion,
    },
#endif

#if (defined(SID_Adept) && COMMON_SKILL_VALID(SID_Adept))
    [SID_Adept] = {
        .name = MSG_SKILL_Adept_NAME,
        .desc = MSG_SKILL_Adept_DESC,
        .capacity = MSG_SKILL_Adept_CAPACITY,
        .price = MSG_SKILL_Adept_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Astra) && COMMON_SKILL_VALID(SID_Astra))
    [SID_Astra] = {
        .name = MSG_SKILL_Astra_NAME,
        .desc = MSG_SKILL_Astra_DESC,
        .capacity = MSG_SKILL_Astra_CAPACITY,
        .price = MSG_SKILL_Astra_PRICE,
        .icon = GFX_SkillIcon_Astra,
    },
#endif

#if (defined(SID_LoadstarRush) && COMMON_SKILL_VALID(SID_LoadstarRush))
    [SID_LoadstarRush] = {
        .name = MSG_SKILL_LoadstarRush_NAME,
        .desc = MSG_SKILL_LoadstarRush_DESC,
        .capacity = MSG_SKILL_LoadstarRush_CAPACITY,
        .price = MSG_SKILL_LoadstarRush_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif
#if (defined(SID_Aegis) && COMMON_SKILL_VALID(SID_Aegis))
    [SID_Aegis] = {
        .name = MSG_SKILL_Aegis_NAME,
        .desc = MSG_SKILL_Aegis_DESC,
        .capacity = MSG_SKILL_Aegis_CAPACITY,
        .price = MSG_SKILL_Aegis_PRICE,
        .icon = GFX_SkillIcon_Aegis,
    },
#endif

#if (defined(SID_Pavise) && COMMON_SKILL_VALID(SID_Pavise))
    [SID_Pavise] = {
        .name = MSG_SKILL_Pavise_NAME,
        .desc = MSG_SKILL_Pavise_DESC,
        .capacity = MSG_SKILL_Pavise_CAPACITY,
        .price = MSG_SKILL_Pavise_PRICE,
        .icon = GFX_SkillIcon_Pavise,
    },
#endif

#if (defined(SID_Bane) && COMMON_SKILL_VALID(SID_Bane))
    [SID_Bane] = {
        .name = MSG_SKILL_Bane_NAME,
        .desc = MSG_SKILL_Bane_DESC,
        .capacity = MSG_SKILL_Bane_CAPACITY,
        .price = MSG_SKILL_Bane_PRICE,
        .icon = GFX_SkillIcon_Bane,
    },
#endif

#if (defined(SID_Mercy) && COMMON_SKILL_VALID(SID_Mercy))
    [SID_Mercy] = {
        .name = MSG_SKILL_Mercy_NAME,
        .desc = MSG_SKILL_Mercy_DESC,
        .capacity = MSG_SKILL_Mercy_CAPACITY,
        .price = MSG_SKILL_Mercy_PRICE,
        .icon = GFX_SkillIcon_Mercy,
    },
#endif

#if (defined(SID_Aether) && COMMON_SKILL_VALID(SID_Aether))
    [SID_Aether] = {
        .name = MSG_SKILL_Aether_NAME,
        .desc = MSG_SKILL_Aether_DESC,
        .capacity = MSG_SKILL_Aether_CAPACITY,
        .price = MSG_SKILL_Aether_PRICE,
        .icon = GFX_SkillIcon_Aether,
    },
#endif

#if (defined(SID_Corona) && COMMON_SKILL_VALID(SID_Corona))
    [SID_Corona] = {
        .name = MSG_SKILL_Corona_NAME,
        .desc = MSG_SKILL_Corona_DESC,
        .capacity = MSG_SKILL_Corona_CAPACITY,
        .price = MSG_SKILL_Corona_PRICE,
        .icon = GFX_SkillIcon_Corona,
    },
#endif

#if (defined(SID_Luna) && COMMON_SKILL_VALID(SID_Luna))
    [SID_Luna] = {
        .name = MSG_SKILL_Luna_NAME,
        .desc = MSG_SKILL_Luna_DESC,
        .capacity = MSG_SKILL_Luna_CAPACITY,
        .price = MSG_SKILL_Luna_PRICE,
        .icon = GFX_SkillIcon_Luna,
    },
#endif

#if (defined(SID_Flare) && COMMON_SKILL_VALID(SID_Flare))
    [SID_Flare] = {
        .name = MSG_SKILL_Flare_NAME,
        .desc = MSG_SKILL_Flare_DESC,
        .capacity = MSG_SKILL_Flare_CAPACITY,
        .price = MSG_SKILL_Flare_PRICE,
        .icon = GFX_SkillIcon_Flare,
    },
#endif

#if (defined(SID_Sol) && COMMON_SKILL_VALID(SID_Sol))
    [SID_Sol] = {
        .name = MSG_SKILL_Sol_NAME,
        .desc = MSG_SKILL_Sol_DESC,
        .capacity = MSG_SKILL_Sol_CAPACITY,
        .price = MSG_SKILL_Sol_PRICE,
        .icon = GFX_SkillIcon_Sol,
    },
#endif

#if (defined(SID_DragonFang) && COMMON_SKILL_VALID(SID_DragonFang))
    [SID_DragonFang] = {
        .name = MSG_SKILL_DragonFang_NAME,
        .desc = MSG_SKILL_DragonFang_DESC,
        .capacity = MSG_SKILL_DragonFang_CAPACITY,
        .price = MSG_SKILL_DragonFang_PRICE,
        .icon = GFX_SkillIcon_DragonFang,
    },
#endif

#if (defined(SID_Foresight) && COMMON_SKILL_VALID(SID_Foresight))
    [SID_Foresight] = {
        .name = MSG_SKILL_Foresight_NAME,
        .desc = MSG_SKILL_Foresight_DESC,
        .capacity = MSG_SKILL_Foresight_CAPACITY,
        .price = MSG_SKILL_Foresight_PRICE,
        .icon = GFX_SkillIcon_Foresight,
    },
#endif

#if (defined(SID_BlowDarting) && COMMON_SKILL_VALID(SID_BlowDarting))
    [SID_BlowDarting] = {
        .name = MSG_SKILL_BlowDarting_NAME,
        .desc = MSG_SKILL_BlowDarting_DESC,
        .capacity = MSG_SKILL_BlowDarting_CAPACITY,
        .price = MSG_SKILL_BlowDarting_PRICE,
        .icon = GFX_SkillIcon_BlowDarting,
    },
#endif

#if (defined(SID_BlowDeath) && COMMON_SKILL_VALID(SID_BlowDeath))
    [SID_BlowDeath] = {
        .name = MSG_SKILL_BlowDeath_NAME,
        .desc = MSG_SKILL_BlowDeath_DESC,
        .capacity = MSG_SKILL_BlowDeath_CAPACITY,
        .price = MSG_SKILL_BlowDeath_PRICE,
        .icon = GFX_SkillIcon_BlowDeath,
    },
#endif

#if (defined(SID_BlowArmored) && COMMON_SKILL_VALID(SID_BlowArmored))
    [SID_BlowArmored] = {
        .name = MSG_SKILL_BlowArmored_NAME,
        .desc = MSG_SKILL_BlowArmored_DESC,
        .capacity = MSG_SKILL_BlowArmored_CAPACITY,
        .price = MSG_SKILL_BlowArmored_PRICE,
        .icon = GFX_SkillIcon_BlowArmored,
    },
#endif

#if (defined(SID_BlowFiendish) && COMMON_SKILL_VALID(SID_BlowFiendish))
    [SID_BlowFiendish] = {
        .name = MSG_SKILL_BlowFiendish_NAME,
        .desc = MSG_SKILL_BlowFiendish_DESC,
        .capacity = MSG_SKILL_BlowFiendish_CAPACITY,
        .price = MSG_SKILL_BlowFiendish_PRICE,
        .icon = GFX_SkillIcon_BlowFiendish,
    },
#endif

#if (defined(SID_BlowWarding) && COMMON_SKILL_VALID(SID_BlowWarding))
    [SID_BlowWarding] = {
        .name = MSG_SKILL_BlowWarding_NAME,
        .desc = MSG_SKILL_BlowWarding_DESC,
        .capacity = MSG_SKILL_BlowWarding_CAPACITY,
        .price = MSG_SKILL_BlowWarding_PRICE,
        .icon = GFX_SkillIcon_BlowWarding,
    },
#endif

#if (defined(SID_BlowDuelist) && COMMON_SKILL_VALID(SID_BlowDuelist))
    [SID_BlowDuelist] = {
        .name = MSG_SKILL_BlowDuelist_NAME,
        .desc = MSG_SKILL_BlowDuelist_DESC,
        .capacity = MSG_SKILL_BlowDuelist_CAPACITY,
        .price = MSG_SKILL_BlowDuelist_PRICE,
        .icon = GFX_SkillIcon_BlowDuelist,
    },
#endif

#if (defined(SID_BlowUncanny) && COMMON_SKILL_VALID(SID_BlowUncanny))
    [SID_BlowUncanny] = {
        .name = MSG_SKILL_BlowUncanny_NAME,
        .desc = MSG_SKILL_BlowUncanny_DESC,
        .capacity = MSG_SKILL_BlowUncanny_CAPACITY,
        .price = MSG_SKILL_BlowUncanny_PRICE,
        .icon = GFX_SkillIcon_BlowUncanny,
    },
#endif

#if (defined(SID_BlowKilling) && COMMON_SKILL_VALID(SID_BlowKilling))
    [SID_BlowKilling] = {
        .name = MSG_SKILL_BlowKilling_NAME,
        .desc = MSG_SKILL_BlowKilling_DESC,
        .capacity = MSG_SKILL_BlowKilling_CAPACITY,
        .price = MSG_SKILL_BlowKilling_PRICE,
        .icon = GFX_SkillIcon_BlowKilling,
    },
#endif

#if (defined(SID_StanceBracing) && COMMON_SKILL_VALID(SID_StanceBracing))
    [SID_StanceBracing] = {
        .name = MSG_SKILL_StanceBracing_NAME,
        .desc = MSG_SKILL_StanceBracing_DESC,
        .capacity = MSG_SKILL_StanceBracing_CAPACITY,
        .price = MSG_SKILL_StanceBracing_PRICE,
        .icon = GFX_SkillIcon_StanceBracing,
    },
#endif

#if (defined(SID_StanceDarting) && COMMON_SKILL_VALID(SID_StanceDarting))
    [SID_StanceDarting] = {
        .name = MSG_SKILL_StanceDarting_NAME,
        .desc = MSG_SKILL_StanceDarting_DESC,
        .capacity = MSG_SKILL_StanceDarting_CAPACITY,
        .price = MSG_SKILL_StanceDarting_PRICE,
        .icon = GFX_SkillIcon_StanceDarting,
    },
#endif

#if (defined(SID_StanceFierce) && COMMON_SKILL_VALID(SID_StanceFierce))
    [SID_StanceFierce] = {
        .name = MSG_SKILL_StanceFierce_NAME,
        .desc = MSG_SKILL_StanceFierce_DESC,
        .capacity = MSG_SKILL_StanceFierce_CAPACITY,
        .price = MSG_SKILL_StanceFierce_PRICE,
        .icon = GFX_SkillIcon_StanceFierce,
    },
#endif

#if (defined(SID_StanceKestrel) && COMMON_SKILL_VALID(SID_StanceKestrel))
    [SID_StanceKestrel] = {
        .name = MSG_SKILL_StanceKestrel_NAME,
        .desc = MSG_SKILL_StanceKestrel_DESC,
        .capacity = MSG_SKILL_StanceKestrel_CAPACITY,
        .price = MSG_SKILL_StanceKestrel_PRICE,
        .icon = GFX_SkillIcon_StanceKestrel,
    },
#endif

#if (defined(SID_StanceMirror) && COMMON_SKILL_VALID(SID_StanceMirror))
    [SID_StanceMirror] = {
        .name = MSG_SKILL_StanceMirror_NAME,
        .desc = MSG_SKILL_StanceMirror_DESC,
        .capacity = MSG_SKILL_StanceMirror_CAPACITY,
        .price = MSG_SKILL_StanceMirror_PRICE,
        .icon = GFX_SkillIcon_StanceMirror,
    },
#endif

#if (defined(SID_StanceReady) && COMMON_SKILL_VALID(SID_StanceReady))
    [SID_StanceReady] = {
        .name = MSG_SKILL_StanceReady_NAME,
        .desc = MSG_SKILL_StanceReady_DESC,
        .capacity = MSG_SKILL_StanceReady_CAPACITY,
        .price = MSG_SKILL_StanceReady_PRICE,
        .icon = GFX_SkillIcon_StanceReady,
    },
#endif

#if (defined(SID_StanceSteady) && COMMON_SKILL_VALID(SID_StanceSteady))
    [SID_StanceSteady] = {
        .name = MSG_SKILL_StanceSteady_NAME,
        .desc = MSG_SKILL_StanceSteady_DESC,
        .capacity = MSG_SKILL_StanceSteady_CAPACITY,
        .price = MSG_SKILL_StanceSteady_PRICE,
        .icon = GFX_SkillIcon_StanceSteady,
    },
#endif

#if (defined(SID_StanceSturdy) && COMMON_SKILL_VALID(SID_StanceSturdy))
    [SID_StanceSturdy] = {
        .name = MSG_SKILL_StanceSturdy_NAME,
        .desc = MSG_SKILL_StanceSturdy_DESC,
        .capacity = MSG_SKILL_StanceSturdy_CAPACITY,
        .price = MSG_SKILL_StanceSturdy_PRICE,
        .icon = GFX_SkillIcon_StanceSturdy,
    },
#endif

#if (defined(SID_StanceSwift) && COMMON_SKILL_VALID(SID_StanceSwift))
    [SID_StanceSwift] = {
        .name = MSG_SKILL_StanceSwift_NAME,
        .desc = MSG_SKILL_StanceSwift_DESC,
        .capacity = MSG_SKILL_StanceSwift_CAPACITY,
        .price = MSG_SKILL_StanceSwift_PRICE,
        .icon = GFX_SkillIcon_StanceSwift,
    },
#endif

#if (defined(SID_StanceWarding) && COMMON_SKILL_VALID(SID_StanceWarding))
    [SID_StanceWarding] = {
        .name = MSG_SKILL_StanceWarding_NAME,
        .desc = MSG_SKILL_StanceWarding_DESC,
        .capacity = MSG_SKILL_StanceWarding_CAPACITY,
        .price = MSG_SKILL_StanceWarding_PRICE,
        .icon = GFX_SkillIcon_StanceWarding,
    },
#endif

#if (defined(SID_MagicRangePlus1) && COMMON_SKILL_VALID(SID_MagicRangePlus1))
    [SID_MagicRangePlus1] = {
        .name = MSG_SKILL_MagicRangePlus1_NAME,
        .desc = MSG_SKILL_MagicRangePlus1_DESC,
        .capacity = MSG_SKILL_MagicRangePlus1_CAPACITY,
        .price = MSG_SKILL_MagicRangePlus1_PRICE,
        .icon = GFX_SkillIcon_MagicRangePlus1,
    },
#endif

#if (defined(SID_MagicRangePlus2) && COMMON_SKILL_VALID(SID_MagicRangePlus2))
    [SID_MagicRangePlus2] = {
        .name = MSG_SKILL_MagicRangePlus2_NAME,
        .desc = MSG_SKILL_MagicRangePlus2_DESC,
        .capacity = MSG_SKILL_MagicRangePlus2_CAPACITY,
        .price = MSG_SKILL_MagicRangePlus2_PRICE,
        .icon = GFX_SkillIcon_MagicRangePlus2,
    },
#endif

#if (defined(SID_BowRangePlus1) && COMMON_SKILL_VALID(SID_BowRangePlus1))
    [SID_BowRangePlus1] = {
        .name = MSG_SKILL_BowRangePlus1_NAME,
        .desc = MSG_SKILL_BowRangePlus1_DESC,
        .capacity = MSG_SKILL_BowRangePlus1_CAPACITY,
        .price = MSG_SKILL_BowRangePlus1_PRICE,
        .icon = GFX_SkillIcon_BowRangePlus1,
    },
#endif

#if (defined(SID_BowRangePlus2) && COMMON_SKILL_VALID(SID_BowRangePlus2))
    [SID_BowRangePlus2] = {
        .name = MSG_SKILL_BowRangePlus2_NAME,
        .desc = MSG_SKILL_BowRangePlus2_DESC,
        .capacity = MSG_SKILL_BowRangePlus2_CAPACITY,
        .price = MSG_SKILL_BowRangePlus2_PRICE,
        .icon = GFX_SkillIcon_BowRangePlus2,
    },
#endif

#if (defined(SID_RunningStart) && COMMON_SKILL_VALID(SID_RunningStart))
    [SID_RunningStart] = {
        .name = MSG_SKILL_RunningStart_NAME,
        .desc = MSG_SKILL_RunningStart_DESC,
        .capacity = MSG_SKILL_RunningStart_CAPACITY,
        .price = MSG_SKILL_RunningStart_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Canto) && COMMON_SKILL_VALID(SID_Canto))
    [SID_Canto] = {
        .name = MSG_SKILL_Canto_NAME,
        .desc = MSG_SKILL_Canto_DESC,
        .capacity = MSG_SKILL_Canto_CAPACITY,
        .price = MSG_SKILL_Canto_PRICE,
        .icon = GFX_SkillIcon_Canto,
    },
#endif

#if (defined(SID_CantoPlus) && COMMON_SKILL_VALID(SID_CantoPlus))
    [SID_CantoPlus] = {
        .name = MSG_SKILL_CantoPlus_NAME,
        .desc = MSG_SKILL_CantoPlus_DESC,
        .capacity = MSG_SKILL_CantoPlus_CAPACITY,
        .price = MSG_SKILL_CantoPlus_PRICE,
        .icon = GFX_SkillIcon_CantoPlus,
    },
#endif

#if (defined(SID_AlertStance) && COMMON_SKILL_VALID(SID_AlertStance))
    [SID_AlertStance] = {
        .name = MSG_SKILL_AlertStance_NAME,
        .desc = MSG_SKILL_AlertStance_DESC,
        .capacity = MSG_SKILL_AlertStance_CAPACITY,
        .price = MSG_SKILL_AlertStance_PRICE,
        .icon = GFX_SkillIcon_AlertStance,
    },
#endif

#if (defined(SID_AlertStancePlus) && COMMON_SKILL_VALID(SID_AlertStancePlus))
    [SID_AlertStancePlus] = {
        .name = MSG_SKILL_AlertStancePlus_NAME,
        .desc = MSG_SKILL_AlertStancePlus_DESC,
        .capacity = MSG_SKILL_AlertStancePlus_CAPACITY,
        .price = MSG_SKILL_AlertStancePlus_PRICE,
        .icon = GFX_SkillIcon_AlertStancePlus,
    },
#endif

#if (defined(SID_Galeforce) && COMMON_SKILL_VALID(SID_Galeforce))
    [SID_Galeforce] = {
        .name = MSG_SKILL_Galeforce_NAME,
        .desc = MSG_SKILL_Galeforce_DESC,
        .capacity = MSG_SKILL_Galeforce_CAPACITY,
        .price = MSG_SKILL_Galeforce_PRICE,
        .icon = GFX_SkillIcon_Galeforce,
    },
#endif

#if (defined(SID_GaleforcePlus) && COMMON_SKILL_VALID(SID_GaleforcePlus))
    [SID_GaleforcePlus] = {
        .name = MSG_SKILL_GaleforcePlus_NAME,
        .desc = MSG_SKILL_GaleforcePlus_DESC,
        .capacity = MSG_SKILL_GaleforcePlus_CAPACITY,
        .price = MSG_SKILL_GaleforcePlus_PRICE,
        .icon = GFX_SkillIcon_GaleforcePlus,
    },
#endif

#if (defined(SID_COMBAT_Galeforce) && COMMON_SKILL_VALID(SID_COMBAT_Galeforce))
    [SID_COMBAT_Galeforce] = {
        .name = MSG_SKILL_COMBAT_Galeforce_NAME,
        .desc = MSG_SKILL_COMBAT_Galeforce_DESC,
        .capacity = MSG_SKILL_COMBAT_Galeforce_CAPACITY,
        .price = MSG_SKILL_COMBAT_Galeforce_PRICE,
        .icon = GFX_SkillIcon_Galeforce,
    },
#endif

#if (defined(SID_SavageBlow) && COMMON_SKILL_VALID(SID_SavageBlow))
    [SID_SavageBlow] = {
        .name = MSG_SKILL_SavageBlow_NAME,
        .desc = MSG_SKILL_SavageBlow_DESC,
        .capacity = MSG_SKILL_SavageBlow_CAPACITY,
        .price = MSG_SKILL_SavageBlow_PRICE,
        .icon = GFX_SkillIcon_SavageBlow,
    },
#endif

#if (defined(SID_BreathOfLife) && COMMON_SKILL_VALID(SID_BreathOfLife))
    [SID_BreathOfLife] = {
        .name = MSG_SKILL_BreathOfLife_NAME,
        .desc = MSG_SKILL_BreathOfLife_DESC,
        .capacity = MSG_SKILL_BreathOfLife_CAPACITY,
        .price = MSG_SKILL_BreathOfLife_PRICE,
        .icon = GFX_SkillIcon_BreathOfLife,
    },
#endif

#if (defined(SID_Thunderstorm) && COMMON_SKILL_VALID(SID_Thunderstorm))
    [SID_Thunderstorm] = {
        .name = MSG_SKILL_Thunderstorm_NAME,
        .desc = MSG_SKILL_Thunderstorm_DESC,
        .capacity = MSG_SKILL_Thunderstorm_CAPACITY,
        .price = MSG_SKILL_Thunderstorm_PRICE,
        .icon = GFX_SkillIcon_Thunderstorm,
    },
#endif

#if (defined(SID_PosReturn) && COMMON_SKILL_VALID(SID_PosReturn))
    [SID_PosReturn] = {
        .name = MSG_SKILL_PosReturn_NAME,
        .desc = MSG_SKILL_PosReturn_DESC,
        .capacity = MSG_SKILL_PosReturn_CAPACITY,
        .price = MSG_SKILL_PosReturn_PRICE,
        .icon = GFX_SkillIcon_PosReturn,
    },
#endif

#if (defined(SID_Lunge) && COMMON_SKILL_VALID(SID_Lunge))
    [SID_Lunge] = {
        .name = MSG_SKILL_Lunge_NAME,
        .desc = MSG_SKILL_Lunge_DESC,
        .capacity = MSG_SKILL_Lunge_CAPACITY,
        .price = MSG_SKILL_Lunge_PRICE,
        .icon = GFX_SkillIcon_Lunge,
    },
#endif

#if (defined(SID_Renewal) && COMMON_SKILL_VALID(SID_Renewal))
    [SID_Renewal] = {
        .name = MSG_SKILL_Renewal_NAME,
        .desc = MSG_SKILL_Renewal_DESC,
        .capacity = MSG_SKILL_Renewal_CAPACITY,
        .price = MSG_SKILL_Renewal_PRICE,
        .icon = GFX_SkillIcon_Renewal,
    },
#endif

#if (defined(SID_RenewalPlus) && COMMON_SKILL_VALID(SID_RenewalPlus))
    [SID_RenewalPlus] = {
        .name = MSG_SKILL_RenewalPlus_NAME,
        .desc = MSG_SKILL_RenewalPlus_DESC,
        .capacity = MSG_SKILL_RenewalPlus_CAPACITY,
        .price = MSG_SKILL_RenewalPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Imbue) && COMMON_SKILL_VALID(SID_Imbue))
    [SID_Imbue] = {
        .name = MSG_SKILL_Imbue_NAME,
        .desc = MSG_SKILL_Imbue_DESC,
        .capacity = MSG_SKILL_Imbue_CAPACITY,
        .price = MSG_SKILL_Imbue_PRICE,
        .icon = GFX_SkillIcon_Imbue,
    },
#endif

#if (defined(SID_Forager) && COMMON_SKILL_VALID(SID_Forager))
    [SID_Forager] = {
        .name = MSG_SKILL_Forager_NAME,
        .desc = MSG_SKILL_Forager_DESC,
        .capacity = MSG_SKILL_Forager_CAPACITY,
        .price = MSG_SKILL_Forager_PRICE,
        .icon = GFX_SkillIcon_Forager,
    },
#endif

#if (defined(SID_Anathema) && COMMON_SKILL_VALID(SID_Anathema))
    [SID_Anathema] = {
        .name = MSG_SKILL_Anathema_NAME,
        .desc = MSG_SKILL_Anathema_DESC,
        .capacity = MSG_SKILL_Anathema_CAPACITY,
        .price = MSG_SKILL_Anathema_PRICE,
        .icon = GFX_SkillIcon_Anathema,
    },
#endif

#if (defined(SID_Bond) && COMMON_SKILL_VALID(SID_Bond))
    [SID_Bond] = {
        .name = MSG_SKILL_Bond_NAME,
        .desc = MSG_SKILL_Bond_DESC,
        .capacity = MSG_SKILL_Bond_CAPACITY,
        .price = MSG_SKILL_Bond_PRICE,
        .icon = GFX_SkillIcon_Bond,
    },
#endif

#if (defined(SID_Charm) && COMMON_SKILL_VALID(SID_Charm))
    [SID_Charm] = {
        .name = MSG_SKILL_Charm_NAME,
        .desc = MSG_SKILL_Charm_DESC,
        .capacity = MSG_SKILL_Charm_CAPACITY,
        .price = MSG_SKILL_Charm_PRICE,
        .icon = GFX_SkillIcon_Charm,
    },
#endif

#if (defined(SID_Intimidate) && COMMON_SKILL_VALID(SID_Intimidate))
    [SID_Intimidate] = {
        .name = MSG_SKILL_Intimidate_NAME,
        .desc = MSG_SKILL_Intimidate_DESC,
        .capacity = MSG_SKILL_Intimidate_CAPACITY,
        .price = MSG_SKILL_Intimidate_PRICE,
        .icon = GFX_SkillIcon_Intimidate,
    },
#endif

#if (defined(SID_Tantivy) && COMMON_SKILL_VALID(SID_Tantivy))
    [SID_Tantivy] = {
        .name = MSG_SKILL_Tantivy_NAME,
        .desc = MSG_SKILL_Tantivy_DESC,
        .capacity = MSG_SKILL_Tantivy_CAPACITY,
        .price = MSG_SKILL_Tantivy_PRICE,
        .icon = GFX_SkillIcon_Tantivy,
    },
#endif

#if (defined(SID_Focus) && COMMON_SKILL_VALID(SID_Focus))
    [SID_Focus] = {
        .name = MSG_SKILL_Focus_NAME,
        .desc = MSG_SKILL_Focus_DESC,
        .capacity = MSG_SKILL_Focus_CAPACITY,
        .price = MSG_SKILL_Focus_PRICE,
        .icon = GFX_SkillIcon_Focus,
    },
#endif

#if (defined(SID_Hex) && COMMON_SKILL_VALID(SID_Hex))
    [SID_Hex] = {
        .name = MSG_SKILL_Hex_NAME,
        .desc = MSG_SKILL_Hex_DESC,
        .capacity = MSG_SKILL_Hex_CAPACITY,
        .price = MSG_SKILL_Hex_PRICE,
        .icon = GFX_SkillIcon_Hex,
    },
#endif

#if (defined(SID_Infiltrator) && COMMON_SKILL_VALID(SID_Infiltrator))
    [SID_Infiltrator] = {
        .name = MSG_SKILL_Infiltrator_NAME,
        .desc = MSG_SKILL_Infiltrator_DESC,
        .capacity = MSG_SKILL_Infiltrator_CAPACITY,
        .price = MSG_SKILL_Infiltrator_PRICE,
        .icon = GFX_SkillIcon_Infiltrator,
    },
#endif

#if (defined(SID_Inspiration) && COMMON_SKILL_VALID(SID_Inspiration))
    [SID_Inspiration] = {
        .name = MSG_SKILL_Inspiration_NAME,
        .desc = MSG_SKILL_Inspiration_DESC,
        .capacity = MSG_SKILL_Inspiration_CAPACITY,
        .price = MSG_SKILL_Inspiration_PRICE,
        .icon = GFX_SkillIcon_Inspiration,
    },
#endif

#if (defined(SID_DivinelyInspiring) && COMMON_SKILL_VALID(SID_DivinelyInspiring))
    [SID_DivinelyInspiring] = {
        .name = MSG_SKILL_DivinelyInspiring_NAME,
        .desc = MSG_SKILL_DivinelyInspiring_DESC,
        .capacity = MSG_SKILL_DivinelyInspiring_CAPACITY,
        .price = MSG_SKILL_DivinelyInspiring_PRICE,
        .icon = GFX_SkillIcon_DivinelyInspiring,
    },
#endif

#if (defined(SID_VoiceOfPeace) && COMMON_SKILL_VALID(SID_VoiceOfPeace))
    [SID_VoiceOfPeace] = {
        .name = MSG_SKILL_VoiceOfPeace_NAME,
        .desc = MSG_SKILL_VoiceOfPeace_DESC,
        .capacity = MSG_SKILL_VoiceOfPeace_CAPACITY,
        .price = MSG_SKILL_VoiceOfPeace_PRICE,
        .icon = GFX_SkillIcon_VoiceOfPeace,
    },
#endif

#if (defined(SID_PowHone) && COMMON_SKILL_VALID(SID_PowHone))
    [SID_PowHone] = {
        .name = MSG_SKILL_PowHone_NAME,
        .desc = MSG_SKILL_PowHone_DESC,
        .capacity = MSG_SKILL_PowHone_CAPACITY,
        .price = MSG_SKILL_PowHone_PRICE,
        .icon = GFX_SkillIcon_HonePow
    },
#endif

#if (defined(SID_MagHone) && COMMON_SKILL_VALID(SID_MagHone))
    [SID_MagHone] = {
        .name = MSG_SKILL_MagHone_NAME,
        .desc = MSG_SKILL_MagHone_DESC,
        .capacity = MSG_SKILL_MagHone_CAPACITY,
        .price = MSG_SKILL_MagHone_PRICE,
        .icon = GFX_SkillIcon_HoneMag
    },
#endif

#if (defined(SID_SklHone) && COMMON_SKILL_VALID(SID_SklHone))
    [SID_SklHone] = {
        .name = MSG_SKILL_SklHone_NAME,
        .desc = MSG_SKILL_SklHone_DESC,
        .capacity = MSG_SKILL_SklHone_CAPACITY,
        .price = MSG_SKILL_SklHone_PRICE,
        .icon = GFX_SkillIcon_HoneSkl
    },
#endif

#if (defined(SID_SpdHone) && COMMON_SKILL_VALID(SID_SpdHone))
    [SID_SpdHone] = {
        .name = MSG_SKILL_SpdHone_NAME,
        .desc = MSG_SKILL_SpdHone_DESC,
        .capacity = MSG_SKILL_SpdHone_CAPACITY,
        .price = MSG_SKILL_SpdHone_PRICE,
        .icon = GFX_SkillIcon_HoneSpd
    },
#endif

#if (defined(SID_LckHone) && COMMON_SKILL_VALID(SID_LckHone))
    [SID_LckHone] = {
        .name = MSG_SKILL_LckHone_NAME,
        .desc = MSG_SKILL_LckHone_DESC,
        .capacity = MSG_SKILL_LckHone_CAPACITY,
        .price = MSG_SKILL_LckHone_PRICE,
        .icon = GFX_SkillIcon_HoneLuk
    },
#endif

#if (defined(SID_DefHone) && COMMON_SKILL_VALID(SID_DefHone))
    [SID_DefHone] = {
        .name = MSG_SKILL_DefHone_NAME,
        .desc = MSG_SKILL_DefHone_DESC,
        .capacity = MSG_SKILL_DefHone_CAPACITY,
        .price = MSG_SKILL_DefHone_PRICE,
        .icon = GFX_SkillIcon_HoneDef
    },
#endif

#if (defined(SID_ResHone) && COMMON_SKILL_VALID(SID_ResHone))
    [SID_ResHone] = {
        .name = MSG_SKILL_ResHone_NAME,
        .desc = MSG_SKILL_ResHone_DESC,
        .capacity = MSG_SKILL_ResHone_CAPACITY,
        .price = MSG_SKILL_ResHone_PRICE,
        .icon = GFX_SkillIcon_HoneRes
    },
#endif

#if (defined(SID_MovHone) && COMMON_SKILL_VALID(SID_MovHone))
    [SID_MovHone] = {
        .name = MSG_SKILL_MovHone_NAME,
        .desc = MSG_SKILL_MovHone_DESC,
        .capacity = MSG_SKILL_MovHone_CAPACITY,
        .price = MSG_SKILL_MovHone_PRICE,
        .icon = GFX_SkillIcon_HoneMov
    },
#endif

#if (defined(SID_HoneChaos) && COMMON_SKILL_VALID(SID_HoneChaos))
    [SID_HoneChaos] = {
        .name = MSG_SKILL_HoneChaos_NAME,
        .desc = MSG_SKILL_HoneChaos_DESC,
        .capacity = MSG_SKILL_HoneChaos_CAPACITY,
        .price = MSG_SKILL_HoneChaos_PRICE,
        .icon = GFX_SkillIcon_HoneChaos
    },
#endif

#if (defined(SID_Steal) && COMMON_SKILL_VALID(SID_Steal))
    [SID_Steal] = {
        .name = MSG_SKILL_Steal_NAME,
        .desc = MSG_SKILL_Steal_DESC,
        .capacity = MSG_SKILL_Steal_CAPACITY,
        .price = MSG_SKILL_Steal_PRICE,
        .icon = GFX_SkillIcon_Steal,
    },
#endif

#if (defined(SID_HealingFocus) && COMMON_SKILL_VALID(SID_HealingFocus))
    [SID_HealingFocus] = {
        .name = MSG_SKILL_HealingFocus_NAME,
        .desc = MSG_SKILL_HealingFocus_DESC,
        .capacity = MSG_SKILL_HealingFocus_CAPACITY,
        .price = MSG_SKILL_HealingFocus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Blossom) && COMMON_SKILL_VALID(SID_Blossom))
    [SID_Blossom] = {
        .name = MSG_SKILL_Blossom_NAME,
        .desc = MSG_SKILL_Blossom_DESC,
        .capacity = MSG_SKILL_Blossom_CAPACITY,
        .price = MSG_SKILL_Blossom_PRICE,
        .icon = GFX_SkillIcon_Blossom,
    },
#endif

#if (defined(SID_Paragon) && COMMON_SKILL_VALID(SID_Paragon))
    [SID_Paragon] = {
        .name = MSG_SKILL_Paragon_NAME,
        .desc = MSG_SKILL_Paragon_DESC,
        .capacity = MSG_SKILL_Paragon_CAPACITY,
        .price = MSG_SKILL_Paragon_PRICE,
        .icon = GFX_SkillIcon_Paragon,
    },
#endif

#if (defined(SID_VoidCurse) && COMMON_SKILL_VALID(SID_VoidCurse))
    [SID_VoidCurse] = {
        .name = MSG_SKILL_VoidCurse_NAME,
        .desc = MSG_SKILL_VoidCurse_DESC,
        .capacity = MSG_SKILL_VoidCurse_CAPACITY,
        .price = MSG_SKILL_VoidCurse_PRICE,
        .icon = GFX_SkillIcon_VoidCurse,
    },
#endif

#if (defined(SID_Aptitude) && COMMON_SKILL_VALID(SID_Aptitude))
    [SID_Aptitude] = {
        .name = MSG_SKILL_Aptitude_NAME,
        .desc = MSG_SKILL_Aptitude_DESC,
        .capacity = MSG_SKILL_Aptitude_CAPACITY,
        .price = MSG_SKILL_Aptitude_PRICE,
        .icon = GFX_SkillIcon_Aptitude,
    },
#endif

#if (defined(SID_Peacebringer) && COMMON_SKILL_VALID(SID_Peacebringer))
    [SID_Peacebringer] = {
        .name = MSG_SKILL_Peacebringer_NAME,
        .desc = MSG_SKILL_Peacebringer_DESC,
        .capacity = MSG_SKILL_Peacebringer_CAPACITY,
        .price = MSG_SKILL_Peacebringer_PRICE,
        .icon = GFX_SkillIcon_Peacebringer,
    },
#endif

#if (defined(SID_Frenzy) && COMMON_SKILL_VALID(SID_Frenzy))
    [SID_Frenzy] = {
        .name = MSG_SKILL_Frenzy_NAME,
        .desc = MSG_SKILL_Frenzy_DESC,
        .capacity = MSG_SKILL_Frenzy_CAPACITY,
        .price = MSG_SKILL_Frenzy_PRICE,
        .icon = GFX_SkillIcon_Frenzy,
    },
#endif

#if (defined(SID_WonderGuard) && COMMON_SKILL_VALID(SID_WonderGuard))
    [SID_WonderGuard] = {
        .name = MSG_SKILL_WonderGuard_NAME,
        .desc = MSG_SKILL_WonderGuard_DESC,
        .capacity = MSG_SKILL_WonderGuard_CAPACITY,
        .price = MSG_SKILL_WonderGuard_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PoisonPoint) && COMMON_SKILL_VALID(SID_PoisonPoint))
    [SID_PoisonPoint] = {
        .name = MSG_SKILL_PoisonPoint_NAME,
        .desc = MSG_SKILL_PoisonPoint_DESC,
        .capacity = MSG_SKILL_PoisonPoint_CAPACITY,
        .price = MSG_SKILL_PoisonPoint_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Merciless) && COMMON_SKILL_VALID(SID_Merciless))
    [SID_Merciless] = {
        .name = MSG_SKILL_Merciless_NAME,
        .desc = MSG_SKILL_Merciless_DESC,
        .capacity = MSG_SKILL_Merciless_CAPACITY,
        .price = MSG_SKILL_Merciless_PRICE,
        .icon = GFX_SkillIcon_Merciless,
    },
#endif

#if (defined(SID_CriticalPierce) && COMMON_SKILL_VALID(SID_CriticalPierce))
    [SID_CriticalPierce] = {
        .name = MSG_SKILL_CriticalPierce_NAME,
        .desc = MSG_SKILL_CriticalPierce_DESC,
        .capacity = MSG_SKILL_CriticalPierce_CAPACITY,
        .price = MSG_SKILL_CriticalPierce_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KillingMachine) && COMMON_SKILL_VALID(SID_KillingMachine))
    [SID_KillingMachine] = {
        .name = MSG_SKILL_KillingMachine_NAME,
        .desc = MSG_SKILL_KillingMachine_DESC,
        .capacity = MSG_SKILL_KillingMachine_CAPACITY,
        .price = MSG_SKILL_KillingMachine_PRICE,
        .icon = GFX_SkillIcon_KillingMachine,
    },
#endif

#if (defined(SID_HeavyStrikes) && COMMON_SKILL_VALID(SID_HeavyStrikes))
    [SID_HeavyStrikes] = {
        .name = MSG_SKILL_HeavyStrikes_NAME,
        .desc = MSG_SKILL_HeavyStrikes_DESC,
        .capacity = MSG_SKILL_HeavyStrikes_CAPACITY,
        .price = MSG_SKILL_HeavyStrikes_PRICE,
        .icon = GFX_SkillIcon_HeavyStrikes,
    },
#endif

#if (defined(SID_QuickBurn) && COMMON_SKILL_VALID(SID_QuickBurn))
    [SID_QuickBurn] = {
        .name = MSG_SKILL_QuickBurn_NAME,
        .desc = MSG_SKILL_QuickBurn_DESC,
        .capacity = MSG_SKILL_QuickBurn_CAPACITY,
        .price = MSG_SKILL_QuickBurn_PRICE,
        .icon = GFX_SkillIcon_QuickBurn,
    },
#endif

#if (defined(SID_SlowBurn) && COMMON_SKILL_VALID(SID_SlowBurn))
    [SID_SlowBurn] = {
        .name = MSG_SKILL_SlowBurn_NAME,
        .desc = MSG_SKILL_SlowBurn_DESC,
        .capacity = MSG_SKILL_SlowBurn_CAPACITY,
        .price = MSG_SKILL_SlowBurn_PRICE,
        .icon = GFX_SkillIcon_SlowBurn,
    },
#endif

#if (defined(SID_StanceSpectrum) && COMMON_SKILL_VALID(SID_StanceSpectrum))
    [SID_StanceSpectrum] = {
        .name = MSG_SKILL_StanceSpectrum_NAME,
        .desc = MSG_SKILL_StanceSpectrum_DESC,
        .capacity = MSG_SKILL_StanceSpectrum_CAPACITY,
        .price = MSG_SKILL_StanceSpectrum_PRICE,
        .icon = GFX_SkillIcon_StanceSpectrum,
    },
#endif

#if (defined(SID_Technician) && COMMON_SKILL_VALID(SID_Technician))
    [SID_Technician] = {
        .name = MSG_SKILL_Technician_NAME,
        .desc = MSG_SKILL_Technician_DESC,
        .capacity = MSG_SKILL_Technician_CAPACITY,
        .price = MSG_SKILL_Technician_PRICE,
        .icon = GFX_SkillIcon_Technician,
    },
#endif

#if (defined(SID_CatchingUp) && COMMON_SKILL_VALID(SID_CatchingUp))
    [SID_CatchingUp] = {
        .name = MSG_SKILL_CatchingUp_NAME,
        .desc = MSG_SKILL_CatchingUp_DESC,
        .capacity = MSG_SKILL_CatchingUp_CAPACITY,
        .price = MSG_SKILL_CatchingUp_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BloodTide) && COMMON_SKILL_VALID(SID_BloodTide))
    [SID_BloodTide] = {
        .name = MSG_SKILL_BloodTide_NAME,
        .desc = MSG_SKILL_BloodTide_DESC,
        .capacity = MSG_SKILL_BloodTide_CAPACITY,
        .price = MSG_SKILL_BloodTide_PRICE,
        .icon = GFX_SkillIcon_BloodTide,
    },
#endif

#if (defined(SID_WhitePool) && COMMON_SKILL_VALID(SID_WhitePool))
    [SID_WhitePool] = {
        .name = MSG_SKILL_WhitePool_NAME,
        .desc = MSG_SKILL_WhitePool_DESC,
        .capacity = MSG_SKILL_WhitePool_CAPACITY,
        .price = MSG_SKILL_WhitePool_PRICE,
        .icon = GFX_SkillIcon_WhitePool,
    },
#endif

#if (defined(SID_NightTide) && COMMON_SKILL_VALID(SID_NightTide))
    [SID_NightTide] = {
        .name = MSG_SKILL_NightTide_NAME,
        .desc = MSG_SKILL_NightTide_DESC,
        .capacity = MSG_SKILL_NightTide_CAPACITY,
        .price = MSG_SKILL_NightTide_PRICE,
        .icon = GFX_SkillIcon_NightTide,
    },
#endif

#if (defined(SID_DriveStr) && COMMON_SKILL_VALID(SID_DriveStr))
    [SID_DriveStr] = {
        .name = MSG_SKILL_DriveStr_NAME,
        .desc = MSG_SKILL_DriveStr_DESC,
        .capacity = MSG_SKILL_DriveStr_CAPACITY,
        .price = MSG_SKILL_DriveStr_PRICE,
        .icon = GFX_SkillIcon_DriveStr,
    },
#endif

#if (defined(SID_DriveMag) && COMMON_SKILL_VALID(SID_DriveMag))
    [SID_DriveMag] = {
        .name = MSG_SKILL_DriveMag_NAME,
        .desc = MSG_SKILL_DriveMag_DESC,
        .capacity = MSG_SKILL_DriveMag_CAPACITY,
        .price = MSG_SKILL_DriveMag_PRICE,
        .icon = GFX_SkillIcon_DriveMag,
    },
#endif

#if (defined(SID_DriveSpd) && COMMON_SKILL_VALID(SID_DriveSpd))
    [SID_DriveSpd] = {
        .name = MSG_SKILL_DriveSpd_NAME,
        .desc = MSG_SKILL_DriveSpd_DESC,
        .capacity = MSG_SKILL_DriveSpd_CAPACITY,
        .price = MSG_SKILL_DriveSpd_PRICE,
        .icon = GFX_SkillIcon_DriveSpd,
    },
#endif

#if (defined(SID_DriveDef) && COMMON_SKILL_VALID(SID_DriveDef))
    [SID_DriveDef] = {
        .name = MSG_SKILL_DriveDef_NAME,
        .desc = MSG_SKILL_DriveDef_DESC,
        .capacity = MSG_SKILL_DriveDef_CAPACITY,
        .price = MSG_SKILL_DriveDef_PRICE,
        .icon = GFX_SkillIcon_DriveDef,
    },
#endif

#if (defined(SID_DriveRes) && COMMON_SKILL_VALID(SID_DriveRes))
    [SID_DriveRes] = {
        .name = MSG_SKILL_DriveRes_NAME,
        .desc = MSG_SKILL_DriveRes_DESC,
        .capacity = MSG_SKILL_DriveRes_CAPACITY,
        .price = MSG_SKILL_DriveRes_PRICE,
        .icon = GFX_SkillIcon_DriveRes,
    },
#endif

#if (defined(SID_Charisma) && COMMON_SKILL_VALID(SID_Charisma))
    [SID_Charisma] = {
        .name = MSG_SKILL_Charisma_NAME,
        .desc = MSG_SKILL_Charisma_DESC,
        .capacity = MSG_SKILL_Charisma_CAPACITY,
        .price = MSG_SKILL_Charisma_PRICE,
        .icon = GFX_SkillIcon_Charisma,
    },
#endif

#if (defined(SID_LilysPoise) && COMMON_SKILL_VALID(SID_LilysPoise))
    [SID_LilysPoise] = {
        .name = MSG_SKILL_LilysPoise_NAME,
        .desc = MSG_SKILL_LilysPoise_DESC,
        .capacity = MSG_SKILL_LilysPoise_CAPACITY,
        .price = MSG_SKILL_LilysPoise_PRICE,
        .icon = GFX_SkillIcon_LilysPoise,
    },
#endif

#if (defined(SID_Daunt) && COMMON_SKILL_VALID(SID_Daunt))
    [SID_Daunt] = {
        .name = MSG_SKILL_Daunt_NAME,
        .desc = MSG_SKILL_Daunt_DESC,
        .capacity = MSG_SKILL_Daunt_CAPACITY,
        .price = MSG_SKILL_Daunt_PRICE,
        .icon = GFX_SkillIcon_Daunt,
    },
#endif

#if (defined(SID_SpurStr) && COMMON_SKILL_VALID(SID_SpurStr))
    [SID_SpurStr] = {
        .name = MSG_SKILL_SpurStr_NAME,
        .desc = MSG_SKILL_SpurStr_DESC,
        .capacity = MSG_SKILL_SpurStr_CAPACITY,
        .price = MSG_SKILL_SpurStr_PRICE,
        .icon = GFX_SkillIcon_SpurStr,
    },
#endif

#if (defined(SID_SpurMag) && COMMON_SKILL_VALID(SID_SpurMag))
    [SID_SpurMag] = {
        .name = MSG_SKILL_SpurMag_NAME,
        .desc = MSG_SKILL_SpurMag_DESC,
        .capacity = MSG_SKILL_SpurMag_CAPACITY,
        .price = MSG_SKILL_SpurMag_PRICE,
        .icon = GFX_SkillIcon_SpurMag,
    },
#endif

#if (defined(SID_SpurDef) && COMMON_SKILL_VALID(SID_SpurDef))
    [SID_SpurDef] = {
        .name = MSG_SKILL_SpurDef_NAME,
        .desc = MSG_SKILL_SpurDef_DESC,
        .capacity = MSG_SKILL_SpurDef_CAPACITY,
        .price = MSG_SKILL_SpurDef_PRICE,
        .icon = GFX_SkillIcon_SpurDef,
    },
#endif

#if (defined(SID_SpurRes) && COMMON_SKILL_VALID(SID_SpurRes))
    [SID_SpurRes] = {
        .name = MSG_SKILL_SpurRes_NAME,
        .desc = MSG_SKILL_SpurRes_DESC,
        .capacity = MSG_SKILL_SpurRes_CAPACITY,
        .price = MSG_SKILL_SpurRes_PRICE,
        .icon = GFX_SkillIcon_SpurRes,
    },
#endif

#if (defined(SID_SpurSpd) && COMMON_SKILL_VALID(SID_SpurSpd))
    [SID_SpurSpd] = {
        .name = MSG_SKILL_SpurSpd_NAME,
        .desc = MSG_SKILL_SpurSpd_DESC,
        .capacity = MSG_SKILL_SpurSpd_CAPACITY,
        .price = MSG_SKILL_SpurSpd_PRICE,
        .icon = GFX_SkillIcon_SpurSpd,
    },
#endif

#if (defined(SID_BattleVeteran) && COMMON_SKILL_VALID(SID_BattleVeteran))
    [SID_BattleVeteran] = {
        .name = MSG_SKILL_BattleVeteran_NAME,
        .desc = MSG_SKILL_BattleVeteran_DESC,
        .capacity = MSG_SKILL_BattleVeteran_CAPACITY,
        .price = MSG_SKILL_BattleVeteran_PRICE,
        .icon = GFX_SkillIcon_BattleVeteran,
    },
#endif

#if (defined(SID_Gentilhomme) && COMMON_SKILL_VALID(SID_Gentilhomme))
    [SID_Gentilhomme] = {
        .name = MSG_SKILL_Gentilhomme_NAME,
        .desc = MSG_SKILL_Gentilhomme_DESC,
        .capacity = MSG_SKILL_Gentilhomme_CAPACITY,
        .price = MSG_SKILL_Gentilhomme_PRICE,
        .icon = GFX_SkillIcon_Gentilhomme,
    },
#endif

#if (defined(SID_Demoiselle) && COMMON_SKILL_VALID(SID_Demoiselle))
    [SID_Demoiselle] = {
        .name = MSG_SKILL_Demoiselle_NAME,
        .desc = MSG_SKILL_Demoiselle_DESC,
        .capacity = MSG_SKILL_Demoiselle_CAPACITY,
        .price = MSG_SKILL_Demoiselle_PRICE,
        .icon = GFX_SkillIcon_Demoiselle,
    },
#endif

#if (defined(SID_MaleficAura) && (SID_MaleficAura < MAX_SKILL_NUM)) 
    [SID_MaleficAura] = {
        .name = MSG_SKILL_MaleficAura_NAME,
        .desc = MSG_SKILL_MaleficAura_DESC,
        .capacity = MSG_SKILL_MaleficAura_CAPACITY,
        .price = MSG_SKILL_MaleficAura_PRICE,
        .icon = GFX_SkillIcon_MaleficAura,
      },
#endif

#if (defined(SID_Solidarity) && COMMON_SKILL_VALID(SID_Solidarity))
    [SID_Solidarity] = {
        .name = MSG_SKILL_Solidarity_NAME,
        .desc = MSG_SKILL_Solidarity_DESC,
        .capacity = MSG_SKILL_Solidarity_CAPACITY,
        .price = MSG_SKILL_Solidarity_PRICE,
        .icon = GFX_SkillIcon_Solidarity,
    },
#endif

#if (defined(SID_StoneBody) && COMMON_SKILL_VALID(SID_StoneBody))
    [SID_StoneBody] = {
        .name = MSG_SKILL_StoneBody_NAME,
        .desc = MSG_SKILL_StoneBody_DESC,
        .capacity = MSG_SKILL_StoneBody_CAPACITY,
        .price = MSG_SKILL_StoneBody_PRICE,
        .icon = GFX_SkillIcon_StoneBody,
    },
#endif

#if (defined(SID_Chivalry) && COMMON_SKILL_VALID(SID_Chivalry))
    [SID_Chivalry] = {
        .name = MSG_SKILL_Chivalry_NAME,
        .desc = MSG_SKILL_Chivalry_DESC,
        .capacity = MSG_SKILL_Chivalry_CAPACITY,
        .price = MSG_SKILL_Chivalry_PRICE,
        .icon = GFX_SkillIcon_Chivalry,
    },
#endif

#if (defined(SID_Pragmatic) && COMMON_SKILL_VALID(SID_Pragmatic))
    [SID_Pragmatic] = {
        .name = MSG_SKILL_Pragmatic_NAME,
        .desc = MSG_SKILL_Pragmatic_DESC,
        .capacity = MSG_SKILL_Pragmatic_CAPACITY,
        .price = MSG_SKILL_Pragmatic_PRICE,
        .icon = GFX_SkillIcon_Pragmatic,
    },
#endif

#if (defined(SID_WindDisciple) && COMMON_SKILL_VALID(SID_WindDisciple))
    [SID_WindDisciple] = {
        .name = MSG_SKILL_WindDisciple_NAME,
        .desc = MSG_SKILL_WindDisciple_DESC,
        .capacity = MSG_SKILL_WindDisciple_CAPACITY,
        .price = MSG_SKILL_WindDisciple_PRICE,
        .icon = GFX_SkillIcon_WindDisciple,
    },
#endif

#if (defined(SID_Perfectionist) && COMMON_SKILL_VALID(SID_Perfectionist))
    [SID_Perfectionist] = {
        .name = MSG_SKILL_Perfectionist_NAME,
        .desc = MSG_SKILL_Perfectionist_DESC,
        .capacity = MSG_SKILL_Perfectionist_CAPACITY,
        .price = MSG_SKILL_Perfectionist_PRICE,
        .icon = GFX_SkillIcon_Perfectionist,
    },
#endif

#if (defined(SID_CriticalForce) && COMMON_SKILL_VALID(SID_CriticalForce))
    [SID_CriticalForce] = {
        .name = MSG_SKILL_CriticalForce_NAME,
        .desc = MSG_SKILL_CriticalForce_DESC,
        .capacity = MSG_SKILL_CriticalForce_CAPACITY,
        .price = MSG_SKILL_CriticalForce_PRICE,
        .icon = GFX_SkillIcon_CriticalForce,
    },
#endif

#if (defined(SID_StrongRiposte) && COMMON_SKILL_VALID(SID_StrongRiposte))
    [SID_StrongRiposte] = {
        .name = MSG_SKILL_StrongRiposte_NAME,
        .desc = MSG_SKILL_StrongRiposte_DESC,
        .capacity = MSG_SKILL_StrongRiposte_CAPACITY,
        .price = MSG_SKILL_StrongRiposte_PRICE,
        .icon = GFX_SkillIcon_StrongRiposte,
    },
#endif

#if (defined(SID_Patience) && COMMON_SKILL_VALID(SID_Patience))
    [SID_Patience] = {
        .name = MSG_SKILL_Patience_NAME,
        .desc = MSG_SKILL_Patience_DESC,
        .capacity = MSG_SKILL_Patience_CAPACITY,
        .price = MSG_SKILL_Patience_PRICE,
        .icon = GFX_SkillIcon_Patience,
    },
#endif

#if (defined(SID_Pursuit) && COMMON_SKILL_VALID(SID_Pursuit))
    [SID_Pursuit] = {
        .name = MSG_SKILL_Pursuit_NAME,
        .desc = MSG_SKILL_Pursuit_DESC,
        .capacity = MSG_SKILL_Pursuit_CAPACITY,
        .price = MSG_SKILL_Pursuit_PRICE,
        .icon = GFX_SkillIcon_Pursuit,
    },
#endif

#if (defined(SID_DragonSkin) && COMMON_SKILL_VALID(SID_DragonSkin))
    [SID_DragonSkin] = {
        .name = MSG_SKILL_DragonSkin_NAME,
        .desc = MSG_SKILL_DragonSkin_DESC,
        .capacity = MSG_SKILL_DragonSkin_CAPACITY,
        .price = MSG_SKILL_DragonSkin_PRICE,
        .icon = GFX_SkillIcon_DragonSkin,
    },
#endif

#if (defined(SID_Hawkeye) && COMMON_SKILL_VALID(SID_Hawkeye))
    [SID_Hawkeye] = {
        .name = MSG_SKILL_Hawkeye_NAME,
        .desc = MSG_SKILL_Hawkeye_DESC,
        .capacity = MSG_SKILL_Hawkeye_CAPACITY,
        .price = MSG_SKILL_Hawkeye_PRICE,
        .icon = GFX_SkillIcon_Hawkeye,
    },
#endif

#if (defined(SID_MagicEye) && COMMON_SKILL_VALID(SID_MagicEye))
    [SID_MagicEye] = {
        .name = MSG_SKILL_MagicEye_NAME,
        .desc = MSG_SKILL_MagicEye_DESC,
        .capacity = MSG_SKILL_MagicEye_CAPACITY,
        .price = MSG_SKILL_MagicEye_PRICE,
        .icon = GFX_SkillIcon_MagicEye,
    },
#endif

#if (defined(SID_KeenFighter) && COMMON_SKILL_VALID(SID_KeenFighter))
    [SID_KeenFighter] = {
        .name = MSG_SKILL_KeenFighter_NAME,
        .desc = MSG_SKILL_KeenFighter_DESC,
        .capacity = MSG_SKILL_KeenFighter_CAPACITY,
        .price = MSG_SKILL_KeenFighter_PRICE,
        .icon = GFX_SkillIcon_KeenFighter,
    },
#endif

#if (defined(SID_FireBoost) && COMMON_SKILL_VALID(SID_FireBoost))
    [SID_FireBoost] = {
        .name = MSG_SKILL_FireBoost_NAME,
        .desc = MSG_SKILL_FireBoost_DESC,
        .capacity = MSG_SKILL_FireBoost_CAPACITY,
        .price = MSG_SKILL_FireBoost_PRICE,
        .icon = GFX_SkillIcon_FireBoost,
    },
#endif

#if (defined(SID_WindBoost) && COMMON_SKILL_VALID(SID_WindBoost))
    [SID_WindBoost] = {
        .name = MSG_SKILL_WindBoost_NAME,
        .desc = MSG_SKILL_WindBoost_DESC,
        .capacity = MSG_SKILL_WindBoost_CAPACITY,
        .price = MSG_SKILL_WindBoost_PRICE,
        .icon = GFX_SkillIcon_WindBoost,
    },
#endif

#if (defined(SID_EarthBoost) && COMMON_SKILL_VALID(SID_EarthBoost))
    [SID_EarthBoost] = {
        .name = MSG_SKILL_EarthBoost_NAME,
        .desc = MSG_SKILL_EarthBoost_DESC,
        .capacity = MSG_SKILL_EarthBoost_CAPACITY,
        .price = MSG_SKILL_EarthBoost_PRICE,
        .icon = GFX_SkillIcon_EarthBoost,
    },
#endif

#if (defined(SID_WaterBoost) && COMMON_SKILL_VALID(SID_WaterBoost))
    [SID_WaterBoost] = {
        .name = MSG_SKILL_WaterBoost_NAME,
        .desc = MSG_SKILL_WaterBoost_DESC,
        .capacity = MSG_SKILL_WaterBoost_CAPACITY,
        .price = MSG_SKILL_WaterBoost_PRICE,
        .icon = GFX_SkillIcon_WaterBoost,
    },
#endif

#if (defined(SID_FieryBlood) && COMMON_SKILL_VALID(SID_FieryBlood))
    [SID_FieryBlood] = {
        .name = MSG_SKILL_FieryBlood_NAME,
        .desc = MSG_SKILL_FieryBlood_DESC,
        .capacity = MSG_SKILL_FieryBlood_CAPACITY,
        .price = MSG_SKILL_FieryBlood_PRICE,
        .icon = GFX_SkillIcon_FieryBlood,
    },
#endif

#if (defined(SID_Colossus) && COMMON_SKILL_VALID(SID_Colossus))
    [SID_Colossus] = {
        .name = MSG_SKILL_Colossus_NAME,
        .desc = MSG_SKILL_Colossus_DESC,
        .capacity = MSG_SKILL_Colossus_CAPACITY,
        .price = MSG_SKILL_Colossus_PRICE,
        .icon = GFX_SkillIcon_Colossus,
    },
#endif

#if (defined(SID_LightWeight) && COMMON_SKILL_VALID(SID_LightWeight))
    [SID_LightWeight] = {
        .name = MSG_SKILL_LightWeight_NAME,
        .desc = MSG_SKILL_LightWeight_DESC,
        .capacity = MSG_SKILL_LightWeight_CAPACITY,
        .price = MSG_SKILL_LightWeight_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EvenFooted) && COMMON_SKILL_VALID(SID_EvenFooted))
    [SID_EvenFooted] = {
        .name = MSG_SKILL_EvenFooted_NAME,
        .desc = MSG_SKILL_EvenFooted_DESC,
        .capacity = MSG_SKILL_EvenFooted_CAPACITY,
        .price = MSG_SKILL_EvenFooted_PRICE,
        .icon = GFX_SkillIcon_EvenFooted,
    },
#endif

#if (defined(SID_QuickDraw) && COMMON_SKILL_VALID(SID_QuickDraw))
    [SID_QuickDraw] = {
        .name = MSG_SKILL_QuickDraw_NAME,
        .desc = MSG_SKILL_QuickDraw_DESC,
        .capacity = MSG_SKILL_QuickDraw_CAPACITY,
        .price = MSG_SKILL_QuickDraw_PRICE,
        .icon = GFX_SkillIcon_QuickDraw,
    },
#endif

#if (defined(SID_ArcaneBlade) && COMMON_SKILL_VALID(SID_ArcaneBlade))
    [SID_ArcaneBlade] = {
        .name = MSG_SKILL_ArcaneBlade_NAME,
        .desc = MSG_SKILL_ArcaneBlade_DESC,
        .capacity = MSG_SKILL_ArcaneBlade_CAPACITY,
        .price = MSG_SKILL_ArcaneBlade_PRICE,
        .icon = GFX_SkillIcon_ArcaneBlade,
    },
#endif

#if (defined(SID_ElbowRoom) && COMMON_SKILL_VALID(SID_ElbowRoom))
    [SID_ElbowRoom] = {
        .name = MSG_SKILL_ElbowRoom_NAME,
        .desc = MSG_SKILL_ElbowRoom_DESC,
        .capacity = MSG_SKILL_ElbowRoom_CAPACITY,
        .price = MSG_SKILL_ElbowRoom_PRICE,
        .icon = GFX_SkillIcon_ElbowRoom,
    },
#endif

#if (defined(SID_NaturalCover) && COMMON_SKILL_VALID(SID_NaturalCover))
    [SID_NaturalCover] = {
        .name = MSG_SKILL_NaturalCover_NAME,
        .desc = MSG_SKILL_NaturalCover_DESC,
        .capacity = MSG_SKILL_NaturalCover_CAPACITY,
        .price = MSG_SKILL_NaturalCover_PRICE,
        .icon = GFX_SkillIcon_NaturalCover,
    },
#endif

#if (defined(SID_ChaosStyle) && COMMON_SKILL_VALID(SID_ChaosStyle))
    [SID_ChaosStyle] = {
        .name = MSG_SKILL_ChaosStyle_NAME,
        .desc = MSG_SKILL_ChaosStyle_DESC,
        .capacity = MSG_SKILL_ChaosStyle_CAPACITY,
        .price = MSG_SKILL_ChaosStyle_PRICE,
        .icon = GFX_SkillIcon_ChaosStyle,
    },
#endif

#if (defined(SID_PushDefense) && COMMON_SKILL_VALID(SID_PushDefense))
    [SID_PushDefense] = {
        .name = MSG_SKILL_PushDefense_NAME,
        .desc = MSG_SKILL_PushDefense_DESC,
        .capacity = MSG_SKILL_PushDefense_CAPACITY,
        .price = MSG_SKILL_PushDefense_PRICE,
        .icon = GFX_SkillIcon_PushDefense,
    },
#endif

#if (defined(SID_PushMagic) && COMMON_SKILL_VALID(SID_PushMagic))
    [SID_PushMagic] = {
        .name = MSG_SKILL_PushMagic_NAME,
        .desc = MSG_SKILL_PushMagic_DESC,
        .capacity = MSG_SKILL_PushMagic_CAPACITY,
        .price = MSG_SKILL_PushMagic_PRICE,
        .icon = GFX_SkillIcon_PushMagic,
    },
#endif

#if (defined(SID_PushResistance) && COMMON_SKILL_VALID(SID_PushResistance))
    [SID_PushResistance] = {
        .name = MSG_SKILL_PushResistance_NAME,
        .desc = MSG_SKILL_PushResistance_DESC,
        .capacity = MSG_SKILL_PushResistance_CAPACITY,
        .price = MSG_SKILL_PushResistance_PRICE,
        .icon = GFX_SkillIcon_PushResistance,
    },
#endif

#if (defined(SID_PushSkill) && COMMON_SKILL_VALID(SID_PushSkill))
    [SID_PushSkill] = {
        .name = MSG_SKILL_PushSkill_NAME,
        .desc = MSG_SKILL_PushSkill_DESC,
        .capacity = MSG_SKILL_PushSkill_CAPACITY,
        .price = MSG_SKILL_PushSkill_PRICE,
        .icon = GFX_SkillIcon_PushSkill,
    },
#endif

#if (defined(SID_PushSpeed) && COMMON_SKILL_VALID(SID_PushSpeed))
    [SID_PushSpeed] = {
        .name = MSG_SKILL_PushSpeed_NAME,
        .desc = MSG_SKILL_PushSpeed_DESC,
        .capacity = MSG_SKILL_PushSpeed_CAPACITY,
        .price = MSG_SKILL_PushSpeed_PRICE,
        .icon = GFX_SkillIcon_PushSpeed,
    },
#endif

#if (defined(SID_PushStrength) && COMMON_SKILL_VALID(SID_PushStrength))
    [SID_PushStrength] = {
        .name = MSG_SKILL_PushStrength_NAME,
        .desc = MSG_SKILL_PushStrength_DESC,
        .capacity = MSG_SKILL_PushStrength_CAPACITY,
        .price = MSG_SKILL_PushStrength_PRICE,
        .icon = GFX_SkillIcon_PushStrength,
    },
#endif

#if (defined(SID_PushLuck) && COMMON_SKILL_VALID(SID_PushLuck))
    [SID_PushLuck] = {
        .name = MSG_SKILL_PushLuck_NAME,
        .desc = MSG_SKILL_PushLuck_DESC,
        .capacity = MSG_SKILL_PushLuck_CAPACITY,
        .price = MSG_SKILL_PushLuck_PRICE,
        .icon = GFX_SkillIcon_PushLuck,
    },
#endif

#if (defined(SID_PushMovement) && COMMON_SKILL_VALID(SID_PushMovement))
    [SID_PushMovement] = {
        .name = MSG_SKILL_PushMovement_NAME,
        .desc = MSG_SKILL_PushMovement_DESC,
        .capacity = MSG_SKILL_PushMovement_CAPACITY,
        .price = MSG_SKILL_PushMovement_PRICE,
        .icon = GFX_SkillIcon_PushMovement,
    },
#endif

#if (defined(SID_PushSpectrum) && COMMON_SKILL_VALID(SID_PushSpectrum))
    [SID_PushSpectrum] = {
        .name = MSG_SKILL_PushSpectrum_NAME,
        .desc = MSG_SKILL_PushSpectrum_DESC,
        .capacity = MSG_SKILL_PushSpectrum_CAPACITY,
        .price = MSG_SKILL_PushSpectrum_PRICE,
        .icon = GFX_SkillIcon_PushSpectrum,
    },
#endif

#if (defined(SID_Charge) && COMMON_SKILL_VALID(SID_Charge))
    [SID_Charge] = {
        .name = MSG_SKILL_Charge_NAME,
        .desc = MSG_SKILL_Charge_DESC,
        .capacity = MSG_SKILL_Charge_CAPACITY,
        .price = MSG_SKILL_Charge_PRICE,
        .icon = GFX_SkillIcon_Charge,
    },
#endif

#if (defined(SID_ChargePlus) && COMMON_SKILL_VALID(SID_ChargePlus))
    [SID_ChargePlus] = {
        .name = MSG_SKILL_ChargePlus_NAME,
        .desc = MSG_SKILL_ChargePlus_DESC,
        .capacity = MSG_SKILL_ChargePlus_CAPACITY,
        .price = MSG_SKILL_ChargePlus_PRICE,
        .icon = GFX_SkillIcon_ChargePlus,
    },
#endif

#if (defined(SID_Wrath) && COMMON_SKILL_VALID(SID_Wrath))
    [SID_Wrath] = {
        .name = MSG_SKILL_Wrath_NAME,
        .desc = MSG_SKILL_Wrath_DESC,
        .capacity = MSG_SKILL_Wrath_CAPACITY,
        .price = MSG_SKILL_Wrath_PRICE,
        .icon = GFX_SkillIcon_Wrath,
    },
#endif

#if (defined(SID_WrathPlus) && COMMON_SKILL_VALID(SID_WrathPlus))
    [SID_WrathPlus] = {
        .name = MSG_SKILL_WrathPlus_NAME,
        .desc = MSG_SKILL_WrathPlus_DESC,
        .capacity = MSG_SKILL_WrathPlus_CAPACITY,
        .price = MSG_SKILL_WrathPlus_PRICE,
        .icon = GFX_SkillIcon_WrathPlus,
    },
#endif

#if (defined(SID_Vigilance) && COMMON_SKILL_VALID(SID_Vigilance))
    [SID_Vigilance] = {
        .name = MSG_SKILL_Vigilance_NAME,
        .desc = MSG_SKILL_Vigilance_DESC,
        .capacity = MSG_SKILL_Vigilance_CAPACITY,
        .price = MSG_SKILL_Vigilance_PRICE,
        .icon = GFX_SkillIcon_Vigilance,
    },
#endif

#if (defined(SID_OutdoorFighter) && COMMON_SKILL_VALID(SID_OutdoorFighter))
    [SID_OutdoorFighter] = {
        .name = MSG_SKILL_OutdoorFighter_NAME,
        .desc = MSG_SKILL_OutdoorFighter_DESC,
        .capacity = MSG_SKILL_OutdoorFighter_CAPACITY,
        .price = MSG_SKILL_OutdoorFighter_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DancingBlade) && COMMON_SKILL_VALID(SID_DancingBlade))
    [SID_DancingBlade] = {
        .name = MSG_SKILL_DancingBlade_NAME,
        .desc = MSG_SKILL_DancingBlade_DESC,
        .capacity = MSG_SKILL_DancingBlade_CAPACITY,
        .price = MSG_SKILL_DancingBlade_PRICE,
        .icon = GFX_SkillIcon_DancingBlade,
    },
#endif

#if (defined(SID_KnightAspirant) && COMMON_SKILL_VALID(SID_KnightAspirant))
    [SID_KnightAspirant] = {
        .name = MSG_SKILL_KnightAspirant_NAME,
        .desc = MSG_SKILL_KnightAspirant_DESC,
        .capacity = MSG_SKILL_KnightAspirant_CAPACITY,
        .price = MSG_SKILL_KnightAspirant_PRICE,
        .icon = GFX_SkillIcon_KnightAspirant,
    },
#endif

#if (defined(SID_Outrider) && COMMON_SKILL_VALID(SID_Outrider))
    [SID_Outrider] = {
        .name = MSG_SKILL_Outrider_NAME,
        .desc = MSG_SKILL_Outrider_DESC,
        .capacity = MSG_SKILL_Outrider_CAPACITY,
        .price = MSG_SKILL_Outrider_PRICE,
        .icon = GFX_SkillIcon_Outrider
    },
#endif

#if (defined(SID_EvenRhythm) && COMMON_SKILL_VALID(SID_EvenRhythm))
    [SID_EvenRhythm] = {
        .name = MSG_SKILL_EvenRhythm_NAME,
        .desc = MSG_SKILL_EvenRhythm_DESC,
        .capacity = MSG_SKILL_EvenRhythm_CAPACITY,
        .price = MSG_SKILL_EvenRhythm_PRICE,
        .icon = GFX_SkillIcon_EvenRhythm
    },
#endif

#if (defined(SID_OddRhythm) && COMMON_SKILL_VALID(SID_OddRhythm))
    [SID_OddRhythm] = {
        .name = MSG_SKILL_OddRhythm_NAME,
        .desc = MSG_SKILL_OddRhythm_DESC,
        .capacity = MSG_SKILL_OddRhythm_CAPACITY,
        .price = MSG_SKILL_OddRhythm_PRICE,
        .icon = GFX_SkillIcon_OddRhythm
    },
#endif

#if (defined(SID_OddFooted) && COMMON_SKILL_VALID(SID_OddFooted))
    [SID_OddFooted] = {
        .name = MSG_SKILL_OddFooted_NAME,
        .desc = MSG_SKILL_OddFooted_DESC,
        .capacity = MSG_SKILL_OddFooted_CAPACITY,
        .price = MSG_SKILL_OddFooted_PRICE,
        .icon = GFX_SkillIcon_OddFooted,
    },
#endif

#if (defined(SID_Corrosion) && COMMON_SKILL_VALID(SID_Corrosion))
    [SID_Corrosion] = {
        .name = MSG_SKILL_Corrosion_NAME,
        .desc = MSG_SKILL_Corrosion_DESC,
        .capacity = MSG_SKILL_Corrosion_CAPACITY,
        .price = MSG_SKILL_Corrosion_PRICE,
        .icon = GFX_SkillIcon_Corrosion,
    },
#endif

#if (defined(SID_NoGuard) && COMMON_SKILL_VALID(SID_NoGuard))
    [SID_NoGuard] = {
        .name = MSG_SKILL_NoGuard_NAME,
        .desc = MSG_SKILL_NoGuard_DESC,
        .capacity = MSG_SKILL_NoGuard_CAPACITY,
        .price = MSG_SKILL_NoGuard_PRICE,
        .icon = GFX_SkillIcon_NoGuard,
    },
#endif

#if (defined(SID_NoGuardPlus) && COMMON_SKILL_VALID(SID_NoGuardPlus))
    [SID_NoGuardPlus] = {
        .name = MSG_SKILL_NoGuardPlus_NAME,
        .desc = MSG_SKILL_NoGuardPlus_DESC,
        .capacity = MSG_SKILL_NoGuardPlus_CAPACITY,
        .price = MSG_SKILL_NoGuardPlus_PRICE,
        .icon = GFX_SkillIcon_NoGuardPlus,
    },
#endif

#if (defined(SID_Puissance) && COMMON_SKILL_VALID(SID_Puissance))
    [SID_Puissance] = {
        .name = MSG_SKILL_Puissance_NAME,
        .desc = MSG_SKILL_Puissance_DESC,
        .capacity = MSG_SKILL_Puissance_CAPACITY,
        .price = MSG_SKILL_Puissance_PRICE,
        .icon = GFX_SkillIcon_Puissance,
    },
#endif

#if (defined(SID_Prescience) && COMMON_SKILL_VALID(SID_Prescience))
    [SID_Prescience] = {
        .name = MSG_SKILL_Prescience_NAME,
        .desc = MSG_SKILL_Prescience_DESC,
        .capacity = MSG_SKILL_Prescience_CAPACITY,
        .price = MSG_SKILL_Prescience_PRICE,
        .icon = GFX_SkillIcon_Prescience,
    },
#endif

#if (defined(SID_Resolve) && COMMON_SKILL_VALID(SID_Resolve))
    [SID_Resolve] = {
        .name = MSG_SKILL_Resolve_NAME,
        .desc = MSG_SKILL_Resolve_DESC,
        .capacity = MSG_SKILL_Resolve_CAPACITY,
        .price = MSG_SKILL_Resolve_PRICE,
        .icon = GFX_SkillIcon_Resolve,
    },
#endif

#if (defined(SID_SilentPride) && COMMON_SKILL_VALID(SID_SilentPride))
    [SID_SilentPride] = {
        .name = MSG_SKILL_SilentPride_NAME,
        .desc = MSG_SKILL_SilentPride_DESC,
        .capacity = MSG_SKILL_SilentPride_CAPACITY,
        .price = MSG_SKILL_SilentPride_PRICE,
        .icon = GFX_SkillIcon_SilentPride,
    },
#endif

#if (defined(SID_RightfulKing) && COMMON_SKILL_VALID(SID_RightfulKing))
    [SID_RightfulKing] = {
        .name = MSG_SKILL_RightfulKing_NAME,
        .desc = MSG_SKILL_RightfulKing_DESC,
        .capacity = MSG_SKILL_RightfulKing_CAPACITY,
        .price = MSG_SKILL_RightfulKing_PRICE,
        .icon = GFX_SkillIcon_RightfulKing,
    },
#endif

#if (defined(SID_RightfulGod) && COMMON_SKILL_VALID(SID_RightfulGod))
    [SID_RightfulGod] = {
        .name = MSG_SKILL_RightfulGod_NAME,
        .desc = MSG_SKILL_RightfulGod_DESC,
        .capacity = MSG_SKILL_RightfulGod_CAPACITY,
        .price = MSG_SKILL_RightfulGod_PRICE,
        .icon = GFX_SkillIcon_RightfulGod,
    },
#endif

#if (defined(SID_RightfulArch) && COMMON_SKILL_VALID(SID_RightfulArch))
    [SID_RightfulArch] = {
        .name = MSG_SKILL_RightfulArch_NAME,
        .desc = MSG_SKILL_RightfulArch_DESC,
        .capacity = MSG_SKILL_RightfulArch_CAPACITY,
        .price = MSG_SKILL_RightfulArch_PRICE,
        .icon = GFX_SkillIcon_RightfulArch,
    },
#endif

#if (defined(SID_Guts) && COMMON_SKILL_VALID(SID_Guts))
    [SID_Guts] = {
        .name = MSG_SKILL_Guts_NAME,
        .desc = MSG_SKILL_Guts_DESC,
        .capacity = MSG_SKILL_Guts_CAPACITY,
        .price = MSG_SKILL_Guts_PRICE,
        .icon = GFX_SkillIcon_Guts,
    },
#endif

#if (defined(SID_StrongCon) && COMMON_SKILL_VALID(SID_StrongCon))
    [SID_StrongCon] = {
        .name = MSG_SKILL_StrongCon_NAME,
        .desc = MSG_SKILL_StrongCon_DESC,
        .capacity = MSG_SKILL_StrongCon_CAPACITY,
        .price = MSG_SKILL_StrongCon_PRICE,
        .icon = GFX_SkillIcon_StrongCon,
    },
#endif

#if (defined(SID_Impale) && COMMON_SKILL_VALID(SID_Impale))
    [SID_Impale] = {
        .name = MSG_SKILL_Impale_NAME,
        .desc = MSG_SKILL_Impale_DESC,
        .capacity = MSG_SKILL_Impale_CAPACITY,
        .price = MSG_SKILL_Impale_PRICE,
        .icon = GFX_SkillIcon_Impale,
    },
#endif

#if (defined(SID_Hero) && COMMON_SKILL_VALID(SID_Hero))
    [SID_Hero] = {
        .name = MSG_SKILL_Hero_NAME,
        .desc = MSG_SKILL_Hero_DESC,
        .capacity = MSG_SKILL_Hero_CAPACITY,
        .price = MSG_SKILL_Hero_PRICE,
        .icon = GFX_SkillIcon_Hero,
    },
#endif

#if (defined(SID_Ignis) && COMMON_SKILL_VALID(SID_Ignis))
    [SID_Ignis] = {
        .name = MSG_SKILL_Ignis_NAME,
        .desc = MSG_SKILL_Ignis_DESC,
        .capacity = MSG_SKILL_Ignis_CAPACITY,
        .price = MSG_SKILL_Ignis_PRICE,
        .icon = GFX_SkillIcon_Ignis,
    },
#endif

#if (defined(SID_HolyAura) && COMMON_SKILL_VALID(SID_HolyAura))
    [SID_HolyAura] = {
        .name = MSG_SKILL_HolyAura_NAME,
        .desc = MSG_SKILL_HolyAura_DESC,
        .capacity = MSG_SKILL_HolyAura_CAPACITY,
        .price = MSG_SKILL_HolyAura_PRICE,
        .icon = GFX_SkillIcon_HolyAura,
    },
#endif

#if (defined(SID_Loyalty) && COMMON_SKILL_VALID(SID_Loyalty))
    [SID_Loyalty] = {
        .name = MSG_SKILL_Loyalty_NAME,
        .desc = MSG_SKILL_Loyalty_DESC,
        .capacity = MSG_SKILL_Loyalty_CAPACITY,
        .price = MSG_SKILL_Loyalty_PRICE,
        .icon = GFX_SkillIcon_Loyalty,
    },
#endif

#if (defined(SID_TowerShield) && COMMON_SKILL_VALID(SID_TowerShield))
    [SID_TowerShield] = {
        .name = MSG_SKILL_TowerShield_NAME,
        .desc = MSG_SKILL_TowerShield_DESC,
        .capacity = MSG_SKILL_TowerShield_CAPACITY,
        .price = MSG_SKILL_TowerShield_PRICE,
        .icon = GFX_SkillIcon_TowerShield,
    },
#endif

#if (defined(SID_StunningSmile) && COMMON_SKILL_VALID(SID_StunningSmile))
    [SID_StunningSmile] = {
        .name = MSG_SKILL_StunningSmile_NAME,
        .desc = MSG_SKILL_StunningSmile_DESC,
        .capacity = MSG_SKILL_StunningSmile_CAPACITY,
        .price = MSG_SKILL_StunningSmile_PRICE,
        .icon = GFX_SkillIcon_StunningSmile,
    },
#endif

#if (defined(SID_Trample) && COMMON_SKILL_VALID(SID_Trample))
    [SID_Trample] = {
        .name = MSG_SKILL_Trample_NAME,
        .desc = MSG_SKILL_Trample_DESC,
        .capacity = MSG_SKILL_Trample_CAPACITY,
        .price = MSG_SKILL_Trample_PRICE,
        .icon = GFX_SkillIcon_Trample,
    },
#endif

#if (defined(SID_Opportunist) && COMMON_SKILL_VALID(SID_Opportunist))
    [SID_Opportunist] = {
        .name = MSG_SKILL_Opportunist_NAME,
        .desc = MSG_SKILL_Opportunist_DESC,
        .capacity = MSG_SKILL_Opportunist_CAPACITY,
        .price = MSG_SKILL_Opportunist_PRICE,
        .icon = GFX_SkillIcon_Opportunist,
    },
#endif

#if (defined(SID_SuperLuck) && COMMON_SKILL_VALID(SID_SuperLuck))
    [SID_SuperLuck] = {
        .name = MSG_SKILL_SuperLuck_NAME,
        .desc = MSG_SKILL_SuperLuck_DESC,
        .capacity = MSG_SKILL_SuperLuck_CAPACITY,
        .price = MSG_SKILL_SuperLuck_PRICE,
        .icon = GFX_SkillIcon_SuperLuck,
    },
#endif

#if (defined(SID_ShortShield) && COMMON_SKILL_VALID(SID_ShortShield))
    [SID_ShortShield] = {
        .name = MSG_SKILL_ShortShield_NAME,
        .desc = MSG_SKILL_ShortShield_DESC,
        .capacity = MSG_SKILL_ShortShield_CAPACITY,
        .price = MSG_SKILL_ShortShield_PRICE,
        .icon = GFX_SkillIcon_ShortShield,
    },
#endif

#if (defined(SID_Vanity) && COMMON_SKILL_VALID(SID_Vanity))
    [SID_Vanity] = {
        .name = MSG_SKILL_Vanity_NAME,
        .desc = MSG_SKILL_Vanity_DESC,
        .capacity = MSG_SKILL_Vanity_CAPACITY,
        .price = MSG_SKILL_Vanity_PRICE,
        .icon = GFX_SkillIcon_Vanity,
    },
#endif

#if (defined(SID_DefenseBoost) && COMMON_SKILL_VALID(SID_DefenseBoost))
    [SID_DefenseBoost] = {
        .name = MSG_SKILL_DefenseBoost_NAME,
        .desc = MSG_SKILL_DefenseBoost_DESC,
        .capacity = MSG_SKILL_DefenseBoost_CAPACITY,
        .price = MSG_SKILL_DefenseBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LuckBoost) && COMMON_SKILL_VALID(SID_LuckBoost))
    [SID_LuckBoost] = {
        .name = MSG_SKILL_LuckBoost_NAME,
        .desc = MSG_SKILL_LuckBoost_DESC,
        .capacity = MSG_SKILL_LuckBoost_CAPACITY,
        .price = MSG_SKILL_LuckBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MagicBoost) && COMMON_SKILL_VALID(SID_MagicBoost))
    [SID_MagicBoost] = {
        .name = MSG_SKILL_MagicBoost_NAME,
        .desc = MSG_SKILL_MagicBoost_DESC,
        .capacity = MSG_SKILL_MagicBoost_CAPACITY,
        .price = MSG_SKILL_MagicBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MoveBoost) && COMMON_SKILL_VALID(SID_MoveBoost))
    [SID_MoveBoost] = {
        .name = MSG_SKILL_MoveBoost_NAME,
        .desc = MSG_SKILL_MoveBoost_DESC,
        .capacity = MSG_SKILL_MoveBoost_CAPACITY,
        .price = MSG_SKILL_MoveBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ResistanceBoost) && COMMON_SKILL_VALID(SID_ResistanceBoost))
    [SID_ResistanceBoost] = {
        .name = MSG_SKILL_ResistanceBoost_NAME,
        .desc = MSG_SKILL_ResistanceBoost_DESC,
        .capacity = MSG_SKILL_ResistanceBoost_CAPACITY,
        .price = MSG_SKILL_ResistanceBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SkillBoost) && COMMON_SKILL_VALID(SID_SkillBoost))
    [SID_SkillBoost] = {
        .name = MSG_SKILL_SkillBoost_NAME,
        .desc = MSG_SKILL_SkillBoost_DESC,
        .capacity = MSG_SKILL_SkillBoost_CAPACITY,
        .price = MSG_SKILL_SkillBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SpeedBoost) && COMMON_SKILL_VALID(SID_SpeedBoost))
    [SID_SpeedBoost] = {
        .name = MSG_SKILL_SpeedBoost_NAME,
        .desc = MSG_SKILL_SpeedBoost_DESC,
        .capacity = MSG_SKILL_SpeedBoost_CAPACITY,
        .price = MSG_SKILL_SpeedBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_StrengthBoost) && COMMON_SKILL_VALID(SID_StrengthBoost))
    [SID_StrengthBoost] = {
        .name = MSG_SKILL_StrengthBoost_NAME,
        .desc = MSG_SKILL_StrengthBoost_DESC,
        .capacity = MSG_SKILL_StrengthBoost_CAPACITY,
        .price = MSG_SKILL_StrengthBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SpectrumBoost) && COMMON_SKILL_VALID(SID_SpectrumBoost))
    [SID_SpectrumBoost] = {
        .name = MSG_SKILL_SpectrumBoost_NAME,
        .desc = MSG_SKILL_SpectrumBoost_DESC,
        .capacity = MSG_SKILL_SpectrumBoost_CAPACITY,
        .price = MSG_SKILL_SpectrumBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Glacies) && COMMON_SKILL_VALID(SID_Glacies))
    [SID_Glacies] = {
        .name = MSG_SKILL_Glacies_NAME,
        .desc = MSG_SKILL_Glacies_DESC,
        .capacity = MSG_SKILL_Glacies_CAPACITY,
        .price = MSG_SKILL_Glacies_PRICE,
        .icon = GFX_SkillIcon_Glacies,
    },
#endif

#if (defined(SID_Vengeance) && COMMON_SKILL_VALID(SID_Vengeance))
    [SID_Vengeance] = {
        .name = MSG_SKILL_Vengeance_NAME,
        .desc = MSG_SKILL_Vengeance_DESC,
        .capacity = MSG_SKILL_Vengeance_CAPACITY,
        .price = MSG_SKILL_Vengeance_PRICE,
        .icon = GFX_SkillIcon_Vengeance,
    },
#endif

#if (defined(SID_Deadeye) && COMMON_SKILL_VALID(SID_Deadeye))
    [SID_Deadeye] = {
        .name = MSG_SKILL_Deadeye_NAME,
        .desc = MSG_SKILL_Deadeye_DESC,
        .capacity = MSG_SKILL_Deadeye_CAPACITY,
        .price = MSG_SKILL_Deadeye_PRICE,
        .icon = GFX_SkillIcon_Deadeye,
    },
#endif

#if (defined(SID_AxeFaith) && COMMON_SKILL_VALID(SID_AxeFaith))
    [SID_AxeFaith] = {
        .name = MSG_SKILL_AxeFaith_NAME,
        .desc = MSG_SKILL_AxeFaith_DESC,
        .capacity = MSG_SKILL_AxeFaith_CAPACITY,
        .price = MSG_SKILL_AxeFaith_PRICE,
        .icon = GFX_SkillIcon_AxeFaith,
    },
#endif

#if (defined(SID_RuinedBladePlus) && COMMON_SKILL_VALID(SID_RuinedBladePlus))
    [SID_RuinedBladePlus] = {
        .name = MSG_SKILL_RuinedBladePlus_NAME,
        .desc = MSG_SKILL_RuinedBladePlus_DESC,
        .capacity = MSG_SKILL_RuinedBladePlus_CAPACITY,
        .price = MSG_SKILL_RuinedBladePlus_PRICE,
        .icon = GFX_SkillIcon_RuinedBlade,
    },
#endif

#if (defined(SID_COMBAT_Grounder) && COMMON_SKILL_VALID(SID_COMBAT_Grounder))
    [SID_COMBAT_Grounder] = {
        .name = MSG_SKILL_COMBAT_Grounder_NAME,
        .desc = MSG_SKILL_COMBAT_Grounder_DESC,
        .capacity = MSG_SKILL_COMBAT_Grounder_CAPACITY,
        .price = MSG_SKILL_COMBAT_Grounder_PRICE,
        .icon = GFX_CombatArtIcon_SwordArtAtk,
    },
#endif

#if (defined(SID_COMBAT_BaneOfMonsters) && COMMON_SKILL_VALID(SID_COMBAT_BaneOfMonsters))
    [SID_COMBAT_BaneOfMonsters] = {
        .name = MSG_SKILL_COMBAT_BaneOfMonsters_NAME,
        .desc = MSG_SKILL_COMBAT_BaneOfMonsters_DESC,
        .capacity = MSG_SKILL_COMBAT_BaneOfMonsters_CAPACITY,
        .price = MSG_SKILL_COMBAT_BaneOfMonsters_PRICE,
        .icon = GFX_CombatArtIcon_SwordArtAtk,
    },
#endif

#if (defined(SID_COMBAT_Hexblade) && COMMON_SKILL_VALID(SID_COMBAT_Hexblade))
    [SID_COMBAT_Hexblade] = {
        .name = MSG_SKILL_COMBAT_Hexblade_NAME,
        .desc = MSG_SKILL_COMBAT_Hexblade_DESC,
        .capacity = MSG_SKILL_COMBAT_Hexblade_CAPACITY,
        .price = MSG_SKILL_COMBAT_Hexblade_PRICE,
        .icon = GFX_CombatArtIcon_SwordArtMag,
    },
#endif

#if (defined(SID_COMBAT_FoudroyantStrike) && COMMON_SKILL_VALID(SID_COMBAT_FoudroyantStrike))
    [SID_COMBAT_FoudroyantStrike] = {
        .name = MSG_SKILL_COMBAT_FoudroyantStrike_NAME,
        .desc = MSG_SKILL_COMBAT_FoudroyantStrike_DESC,
        .capacity = MSG_SKILL_COMBAT_FoudroyantStrike_CAPACITY,
        .price = MSG_SKILL_COMBAT_FoudroyantStrike_PRICE,
        .icon = GFX_CombatArtIcon_SwordArtCrit,
    },
#endif

#if (defined(SID_COMBAT_TempestLance) && COMMON_SKILL_VALID(SID_COMBAT_TempestLance))
    [SID_COMBAT_TempestLance] = {
        .name = MSG_SKILL_COMBAT_TempestLance_NAME,
        .desc = MSG_SKILL_COMBAT_TempestLance_DESC,
        .capacity = MSG_SKILL_COMBAT_TempestLance_CAPACITY,
        .price = MSG_SKILL_COMBAT_TempestLance_PRICE,
        .icon = GFX_CombatArtIcon_LanceArtAtk,
    },
#endif

#if (defined(SID_COMBAT_SwiftStrikes) && COMMON_SKILL_VALID(SID_COMBAT_SwiftStrikes))
    [SID_COMBAT_SwiftStrikes] = {
        .name = MSG_SKILL_COMBAT_SwiftStrikes_NAME,
        .desc = MSG_SKILL_COMBAT_SwiftStrikes_DESC,
        .capacity = MSG_SKILL_COMBAT_SwiftStrikes_CAPACITY,
        .price = MSG_SKILL_COMBAT_SwiftStrikes_PRICE,
        .icon = GFX_CombatArtIcon_LanceArtAtk,
    },
#endif

#if (defined(SID_COMBAT_FrozenLance) && COMMON_SKILL_VALID(SID_COMBAT_FrozenLance))
    [SID_COMBAT_FrozenLance] = {
        .name = MSG_SKILL_COMBAT_FrozenLance_NAME,
        .desc = MSG_SKILL_COMBAT_FrozenLance_DESC,
        .capacity = MSG_SKILL_COMBAT_FrozenLance_CAPACITY,
        .price = MSG_SKILL_COMBAT_FrozenLance_PRICE,
        .icon = GFX_CombatArtIcon_LanceArtMag,
    },
#endif

#if (defined(SID_COMBAT_Vengeance) && COMMON_SKILL_VALID(SID_COMBAT_Vengeance))
    [SID_COMBAT_Vengeance] = {
        .name = MSG_SKILL_COMBAT_Vengeance_NAME,
        .desc = MSG_SKILL_COMBAT_Vengeance_DESC,
        .capacity = MSG_SKILL_COMBAT_Vengeance_CAPACITY,
        .price = MSG_SKILL_COMBAT_Vengeance_PRICE,
        .icon = GFX_CombatArtIcon_LanceArtCrit,
    },
#endif

#if (defined(SID_COMBAT_HelmSplitter) && COMMON_SKILL_VALID(SID_COMBAT_HelmSplitter))
    [SID_COMBAT_HelmSplitter] = {
        .name = MSG_SKILL_COMBAT_HelmSplitter_NAME,
        .desc = MSG_SKILL_COMBAT_HelmSplitter_DESC,
        .capacity = MSG_SKILL_COMBAT_HelmSplitter_CAPACITY,
        .price = MSG_SKILL_COMBAT_HelmSplitter_PRICE,
        .icon = GFX_CombatArtIcon_AxeArtAtk,
    },
#endif

#if (defined(SID_COMBAT_LightningAxe) && COMMON_SKILL_VALID(SID_COMBAT_LightningAxe))
    [SID_COMBAT_LightningAxe] = {
        .name = MSG_SKILL_COMBAT_LightningAxe_NAME,
        .desc = MSG_SKILL_COMBAT_LightningAxe_DESC,
        .capacity = MSG_SKILL_COMBAT_LightningAxe_CAPACITY,
        .price = MSG_SKILL_COMBAT_LightningAxe_PRICE,
        .icon = GFX_CombatArtIcon_AxeArtMag,
    },
#endif

#if (defined(SID_COMBAT_WarMasterStrike) && COMMON_SKILL_VALID(SID_COMBAT_WarMasterStrike))
    [SID_COMBAT_WarMasterStrike] = {
        .name = MSG_SKILL_COMBAT_WarMasterStrike_NAME,
        .desc = MSG_SKILL_COMBAT_WarMasterStrike_DESC,
        .capacity = MSG_SKILL_COMBAT_WarMasterStrike_CAPACITY,
        .price = MSG_SKILL_COMBAT_WarMasterStrike_PRICE,
        .icon = GFX_CombatArtIcon_AxeArtCrit,
    },
#endif

#if (defined(SID_COMBAT_CurvedShot) && COMMON_SKILL_VALID(SID_COMBAT_CurvedShot))
    [SID_COMBAT_CurvedShot] = {
        .name = MSG_SKILL_COMBAT_CurvedShot_NAME,
        .desc = MSG_SKILL_COMBAT_CurvedShot_DESC,
        .capacity = MSG_SKILL_COMBAT_CurvedShot_CAPACITY,
        .price = MSG_SKILL_COMBAT_CurvedShot_PRICE,
        .icon = GFX_CombatArtIcon_BowArtAtk,
    },
#endif

#if (defined(SID_COMBAT_Deadeye) && COMMON_SKILL_VALID(SID_COMBAT_Deadeye))
    [SID_COMBAT_Deadeye] = {
        .name = MSG_SKILL_COMBAT_Deadeye_NAME,
        .desc = MSG_SKILL_COMBAT_Deadeye_DESC,
        .capacity = MSG_SKILL_COMBAT_Deadeye_CAPACITY,
        .price = MSG_SKILL_COMBAT_Deadeye_PRICE,
        .icon = GFX_CombatArtIcon_BowArtAtk,
    },
#endif

#if (defined(SID_COMBAT_Encloser) && COMMON_SKILL_VALID(SID_COMBAT_Encloser))
    [SID_COMBAT_Encloser] = {
        .name = MSG_SKILL_COMBAT_Encloser_NAME,
        .desc = MSG_SKILL_COMBAT_Encloser_DESC,
        .capacity = MSG_SKILL_COMBAT_Encloser_CAPACITY,
        .price = MSG_SKILL_COMBAT_Encloser_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_EncloserPlus) && COMMON_SKILL_VALID(SID_COMBAT_EncloserPlus))
    [SID_COMBAT_EncloserPlus] = {
        .name = MSG_SKILL_COMBAT_EncloserPlus_NAME,
        .desc = MSG_SKILL_COMBAT_EncloserPlus_DESC,
        .capacity = MSG_SKILL_COMBAT_EncloserPlus_CAPACITY,
        .price = MSG_SKILL_COMBAT_EncloserPlus_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_MonsterBlast) && COMMON_SKILL_VALID(SID_COMBAT_MonsterBlast))
    [SID_COMBAT_MonsterBlast] = {
        .name = MSG_SKILL_COMBAT_MonsterBlast_NAME,
        .desc = MSG_SKILL_COMBAT_MonsterBlast_DESC,
        .capacity = MSG_SKILL_COMBAT_MonsterBlast_CAPACITY,
        .price = MSG_SKILL_COMBAT_MonsterBlast_PRICE,
        .icon = GFX_CombatArtIcon_BowArtCrit,
    },
#endif

#if (defined(SID_COMBAT_SchismShot) && COMMON_SKILL_VALID(SID_COMBAT_SchismShot))
    [SID_COMBAT_SchismShot] = {
        .name = MSG_SKILL_COMBAT_SchismShot_NAME,
        .desc = MSG_SKILL_COMBAT_SchismShot_DESC,
        .capacity = MSG_SKILL_COMBAT_SchismShot_CAPACITY,
        .price = MSG_SKILL_COMBAT_SchismShot_PRICE,
        .icon = GFX_CombatArtIcon_BowArtCrit,
    },
#endif

#if (defined(SID_COMBAT_BreakShot) && COMMON_SKILL_VALID(SID_COMBAT_BreakShot))
    [SID_COMBAT_BreakShot] = {
        .name = MSG_SKILL_COMBAT_BreakShot_NAME,
        .desc = MSG_SKILL_COMBAT_BreakShot_DESC,
        .capacity = MSG_SKILL_COMBAT_BreakShot_CAPACITY,
        .price = MSG_SKILL_COMBAT_BreakShot_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_BreakShotPlus) && COMMON_SKILL_VALID(SID_COMBAT_BreakShotPlus))
    [SID_COMBAT_BreakShotPlus] = {
        .name = MSG_SKILL_COMBAT_BreakShotPlus_NAME,
        .desc = MSG_SKILL_COMBAT_BreakShotPlus_DESC,
        .capacity = MSG_SKILL_COMBAT_BreakShotPlus_CAPACITY,
        .price = MSG_SKILL_COMBAT_BreakShotPlus_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_WaningShot) && COMMON_SKILL_VALID(SID_COMBAT_WaningShot))
    [SID_COMBAT_WaningShot] = {
        .name = MSG_SKILL_COMBAT_WaningShot_NAME,
        .desc = MSG_SKILL_COMBAT_WaningShot_DESC,
        .capacity = MSG_SKILL_COMBAT_WaningShot_CAPACITY,
        .price = MSG_SKILL_COMBAT_WaningShot_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_WaningShotPlus) && COMMON_SKILL_VALID(SID_COMBAT_WaningShotPlus))
    [SID_COMBAT_WaningShotPlus] = {
        .name = MSG_SKILL_COMBAT_WaningShotPlus_NAME,
        .desc = MSG_SKILL_COMBAT_WaningShotPlus_DESC,
        .capacity = MSG_SKILL_COMBAT_WaningShotPlus_CAPACITY,
        .price = MSG_SKILL_COMBAT_WaningShotPlus_PRICE,
        .icon = GFX_CombatArtIcon_BowArtDebuff,
    },
#endif

#if (defined(SID_COMBAT_HuntersVolley) && COMMON_SKILL_VALID(SID_COMBAT_HuntersVolley))
    [SID_COMBAT_HuntersVolley] = {
        .name = MSG_SKILL_COMBAT_HuntersVolley_NAME,
        .desc = MSG_SKILL_COMBAT_HuntersVolley_DESC,
        .capacity = MSG_SKILL_COMBAT_HuntersVolley_CAPACITY,
        .price = MSG_SKILL_COMBAT_HuntersVolley_PRICE,
        .icon = GFX_CombatArtIcon_BowArtAtk,
    },
#endif

#if (defined(SID_COMBAT_AstraStorm) && COMMON_SKILL_VALID(SID_COMBAT_AstraStorm))
    [SID_COMBAT_AstraStorm] = {
        .name = MSG_SKILL_COMBAT_AstraStorm_NAME,
        .desc = MSG_SKILL_COMBAT_AstraStorm_DESC,
        .capacity = MSG_SKILL_COMBAT_AstraStorm_CAPACITY,
        .price = MSG_SKILL_COMBAT_AstraStorm_PRICE,
        .icon = GFX_CombatArtIcon_BowArtAtk,
    },
#endif

#if (defined(SID_GreatShield) && COMMON_SKILL_VALID(SID_GreatShield))
    [SID_GreatShield] = {
        .name = MSG_SKILL_GreatShield_NAME,
        .desc = MSG_SKILL_GreatShield_DESC,
        .capacity = MSG_SKILL_GreatShield_CAPACITY,
        .price = MSG_SKILL_GreatShield_PRICE,
        .icon = GFX_SkillIcon_GreatShield,
    },
#endif

#if (defined(SID_HoneCavalry) && COMMON_SKILL_VALID(SID_HoneCavalry))
    [SID_HoneCavalry] = {
        .name = MSG_SKILL_HoneCavalry_NAME,
        .desc = MSG_SKILL_HoneCavalry_DESC,
        .capacity = MSG_SKILL_HoneCavalry_CAPACITY,
        .price = MSG_SKILL_HoneCavalry_PRICE,
        .icon = GFX_SkillIcon_HoneCavalry
    },
#endif

#if (defined(SID_HoneFlier) && COMMON_SKILL_VALID(SID_HoneFlier))
    [SID_HoneFlier] = {
        .name = MSG_SKILL_HoneFlier_NAME,
        .desc = MSG_SKILL_HoneFlier_DESC,
        .capacity = MSG_SKILL_HoneFlier_CAPACITY,
        .price = MSG_SKILL_HoneFlier_PRICE,
        .icon = GFX_SkillIcon_HoneFlier
    },
#endif

#if (defined(SID_HoneArmor) && COMMON_SKILL_VALID(SID_HoneArmor))
    [SID_HoneArmor] = {
        .name = MSG_SKILL_HoneArmor_NAME,
        .desc = MSG_SKILL_HoneArmor_DESC,
        .capacity = MSG_SKILL_HoneArmor_CAPACITY,
        .price = MSG_SKILL_HoneArmor_PRICE,
        .icon = GFX_SkillIcon_HoneArmor
    },
#endif

#if (defined(SID_FortifyArmor) && COMMON_SKILL_VALID(SID_FortifyArmor))
    [SID_FortifyArmor] = {
        .name = MSG_SKILL_FortifyArmor_NAME,
        .desc = MSG_SKILL_FortifyArmor_DESC,
        .capacity = MSG_SKILL_FortifyArmor_CAPACITY,
        .price = MSG_SKILL_FortifyArmor_PRICE,
        .icon = GFX_SkillIcon_WIP
    },
#endif

#if (defined(SID_Shade) && COMMON_SKILL_VALID(SID_Shade))
    [SID_Shade] = {
        .name = MSG_SKILL_Shade_NAME,
        .desc = MSG_SKILL_Shade_DESC,
        .capacity = MSG_SKILL_Shade_CAPACITY,
        .price = MSG_SKILL_Shade_PRICE,
        .icon = GFX_SkillIcon_Shade,
    },
#endif

#if (defined(SID_ShadePlus) && COMMON_SKILL_VALID(SID_ShadePlus))
    [SID_ShadePlus] = {
        .name = MSG_SKILL_ShadePlus_NAME,
        .desc = MSG_SKILL_ShadePlus_DESC,
        .capacity = MSG_SKILL_ShadePlus_CAPACITY,
        .price = MSG_SKILL_ShadePlus_PRICE,
        .icon = GFX_SkillIcon_ShadePlus,
    },
#endif

#if (defined(SID_Provoke) && COMMON_SKILL_VALID(SID_Provoke))
    [SID_Provoke] = {
        .name = MSG_SKILL_Provoke_NAME,
        .desc = MSG_SKILL_Provoke_DESC,
        .capacity = MSG_SKILL_Provoke_CAPACITY,
        .price = MSG_SKILL_Provoke_PRICE,
        .icon = GFX_SkillIcon_Provoke,
    },
#endif

#if (defined(SID_Teleportation) && COMMON_SKILL_VALID(SID_Teleportation))
    [SID_Teleportation] = {
        .name = MSG_SKILL_Teleportation_NAME,
        .desc = MSG_SKILL_Teleportation_DESC,
        .capacity = MSG_SKILL_Teleportation_CAPACITY,
        .price = MSG_SKILL_Teleportation_PRICE,
        .icon = GFX_SkillIcon_Teleportation,
    },
#endif

#if (defined(SID_Dance) && COMMON_SKILL_VALID(SID_Dance))
    [SID_Dance] = {
        .name = MSG_SKILL_Dance_NAME,
        .desc = MSG_SKILL_Dance_DESC,
        .capacity = MSG_SKILL_Dance_CAPACITY,
        .price = MSG_SKILL_Dance_PRICE,
        .icon = GFX_SkillIcon_Dance,
    },
#endif

#if (defined(SID_DancePlus) && COMMON_SKILL_VALID(SID_DancePlus))
    [SID_DancePlus] = {
        .name = MSG_SKILL_DancePlus_NAME,
        .desc = MSG_SKILL_DancePlus_DESC,
        .capacity = MSG_SKILL_DancePlus_CAPACITY,
        .price = MSG_SKILL_DancePlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LockTouch) && COMMON_SKILL_VALID(SID_LockTouch))
    [SID_LockTouch] = {
        .name = MSG_SKILL_LockTouch_NAME,
        .desc = MSG_SKILL_LockTouch_DESC,
        .capacity = MSG_SKILL_LockTouch_CAPACITY,
        .price = MSG_SKILL_LockTouch_PRICE,
        .icon = GFX_SkillIcon_LockTouch,
    },
#endif

#if (defined(SID_Summon) && COMMON_SKILL_VALID(SID_Summon))
    [SID_Summon] = {
        .name = MSG_SKILL_Summon_NAME,
        .desc = MSG_SKILL_Summon_DESC,
        .capacity = MSG_SKILL_Summon_CAPACITY,
        .price = MSG_SKILL_Summon_PRICE,
        .icon = GFX_SkillIcon_Summon,
    },
#endif

#if (defined(SID_SureShot) && COMMON_SKILL_VALID(SID_SureShot))
    [SID_SureShot] = {
        .name = MSG_SKILL_SureShot_NAME,
        .desc = MSG_SKILL_SureShot_DESC,
        .capacity = MSG_SKILL_SureShot_CAPACITY,
        .price = MSG_SKILL_SureShot_PRICE,
        .icon = GFX_SkillIcon_SureShot,
    },
#endif

#if (defined(SID_Crit) && COMMON_SKILL_VALID(SID_Crit))
    [SID_Crit] = {
        .name = MSG_SKILL_Crit_NAME,
        .desc = MSG_SKILL_Crit_DESC,
        .capacity = MSG_SKILL_Crit_CAPACITY,
        .price = MSG_SKILL_Crit_PRICE,
        .icon = GFX_SkillIcon_Crit,
    },
#endif

#if (defined(SID_Lethality) && COMMON_SKILL_VALID(SID_Lethality))
    [SID_Lethality] = {
        .name = MSG_SKILL_Lethality_NAME,
        .desc = MSG_SKILL_Lethality_DESC,
        .capacity = MSG_SKILL_Lethality_CAPACITY,
        .price = MSG_SKILL_Lethality_PRICE,
        .icon = GFX_SkillIcon_Lethality,
    },
#endif

#if (defined(SID_WatchfulEye) && COMMON_SKILL_VALID(SID_WatchfulEye))
    [SID_WatchfulEye] = {
        .name = MSG_SKILL_WatchfulEye_NAME,
        .desc = MSG_SKILL_WatchfulEye_DESC,
        .capacity = MSG_SKILL_WatchfulEye_CAPACITY,
        .price = MSG_SKILL_WatchfulEye_PRICE,
        .icon = GFX_SkillIcon_WatchfulEye,
    },
#endif

#if (defined(SID_InfinityEdge) && COMMON_SKILL_VALID(SID_InfinityEdge))
    [SID_InfinityEdge] = {
        .name = MSG_SKILL_InfinityEdge_NAME,
        .desc = MSG_SKILL_InfinityEdge_DESC,
        .capacity = MSG_SKILL_InfinityEdge_CAPACITY,
        .price = MSG_SKILL_InfinityEdge_PRICE,
        .icon = GFX_SkillIcon_InfinityEdge,
    },
#endif

#if (defined(SID_AversaNight) && COMMON_SKILL_VALID(SID_AversaNight))
    [SID_AversaNight] = {
        .name = MSG_SKILL_Aversa_NAME,
        .desc = MSG_SKILL_Aversa_DESC,
        .capacity = MSG_SKILL_Aversa_CAPACITY,
        .price = MSG_SKILL_Aversa_PRICE,
        .icon = GFX_SkillIcon_AversaNight,
    },
#endif

#if (defined(SID_Supply) && COMMON_SKILL_VALID(SID_Supply))
    [SID_Supply] = {
        .name = MSG_SKILL_Supply_NAME,
        .desc = MSG_SKILL_Supply_DESC,
        .capacity = MSG_SKILL_Supply_CAPACITY,
        .price = MSG_SKILL_Supply_PRICE,
        .icon = GFX_SkillIcon_Supply,
    },
#endif

#if (defined(SID_LightRune) && COMMON_SKILL_VALID(SID_LightRune))
    [SID_LightRune] = {
        .name = MSG_SKILL_LightRune_NAME,
        .desc = MSG_SKILL_LightRune_DESC,
        .capacity = MSG_SKILL_LightRune_CAPACITY,
        .price = MSG_SKILL_LightRune_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Mine) && COMMON_SKILL_VALID(SID_Mine))
    [SID_Mine] = {
        .name = MSG_SKILL_Mine_NAME,
        .desc = MSG_SKILL_Mine_DESC,
        .capacity = MSG_SKILL_Mine_CAPACITY,
        .price = MSG_SKILL_Mine_PRICE,
        .icon = GFX_SkillIcon_Mine,
    },
#endif

#if (defined(SID_Miracle) && COMMON_SKILL_VALID(SID_Miracle))
    [SID_Miracle] = {
        .name = MSG_SKILL_Miracle_NAME,
        .desc = MSG_SKILL_Miracle_DESC,
        .capacity = MSG_SKILL_Miracle_CAPACITY,
        .price = MSG_SKILL_Miracle_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Armsthrift) && COMMON_SKILL_VALID(SID_Armsthrift))
    [SID_Armsthrift] = {
        .name = MSG_SKILL_Armsthrift_NAME,
        .desc = MSG_SKILL_Armsthrift_DESC,
        .capacity = MSG_SKILL_Armsthrift_CAPACITY,
        .price = MSG_SKILL_Armsthrift_PRICE,
        .icon = GFX_SkillIcon_Armsthrift,
    },
#endif

#if (defined(SID_HighGround) && COMMON_SKILL_VALID(SID_HighGround))
    [SID_HighGround] = {
        .name = MSG_SKILL_HighGround_NAME,
        .desc = MSG_SKILL_HighGround_DESC,
        .capacity = MSG_SKILL_HighGround_CAPACITY,
        .price = MSG_SKILL_HighGround_PRICE,
        .icon = GFX_SkillIcon_HighGround,
    },
#endif

#if (defined(SID_Airlock) && COMMON_SKILL_VALID(SID_Airlock))
    [SID_Airlock] = {
        .name = MSG_SKILL_Airlock_NAME,
        .desc = MSG_SKILL_Airlock_DESC,
        .capacity = MSG_SKILL_Airlock_CAPACITY,
        .price = MSG_SKILL_Airlock_PRICE,
        .icon = GFX_SkillIcon_Airlock,
    },
#endif

#if (defined(SID_Drought) && COMMON_SKILL_VALID(SID_Drought))
    [SID_Drought] = {
        .name = MSG_SKILL_Drought_NAME,
        .desc = MSG_SKILL_Drought_DESC,
        .capacity = MSG_SKILL_Drought_CAPACITY,
        .price = MSG_SKILL_Drought_PRICE,
        .icon = GFX_SkillIcon_Drought,
    },
#endif

#if (defined(SID_Drizzle) && COMMON_SKILL_VALID(SID_Drizzle))
    [SID_Drizzle] = {
        .name = MSG_SKILL_Drizzle_NAME,
        .desc = MSG_SKILL_Drizzle_DESC,
        .capacity = MSG_SKILL_Drizzle_CAPACITY,
        .price = MSG_SKILL_Drizzle_PRICE,
        .icon = GFX_SkillIcon_Drizzle,
    },
#endif

#if (defined(SID_SnowWarning) && COMMON_SKILL_VALID(SID_SnowWarning))
    [SID_SnowWarning] = {
        .name = MSG_SKILL_SnowWarning_NAME,
        .desc = MSG_SKILL_SnowWarning_DESC,
        .capacity = MSG_SKILL_SnowWarning_CAPACITY,
        .price = MSG_SKILL_SnowWarning_PRICE,
        .icon = GFX_SkillIcon_SnowWarning,
    },
#endif

#if (defined(SID_SandStream) && COMMON_SKILL_VALID(SID_SandStream))
    [SID_SandStream] = {
        .name = MSG_SKILL_SandStream_NAME,
        .desc = MSG_SKILL_SandStream_DESC,
        .capacity = MSG_SKILL_SandStream_CAPACITY,
        .price = MSG_SKILL_SandStream_PRICE,
        .icon = GFX_SkillIcon_SandStream,
    },
#endif

#if (defined(SID_PointBlank) && COMMON_SKILL_VALID(SID_PointBlank))
    [SID_PointBlank] = {
        .name = MSG_SKILL_PointBlank_NAME,
        .desc = MSG_SKILL_PointBlank_DESC,
        .capacity = MSG_SKILL_PointBlank_CAPACITY,
        .price = MSG_SKILL_PointBlank_PRICE,
        .icon = GFX_SkillIcon_PointBlank,
    },
#endif

#if (defined(SID_Synchronize) && COMMON_SKILL_VALID(SID_Synchronize))
    [SID_Synchronize] = {
        .name = MSG_SKILL_Synchronize_NAME,
        .desc = MSG_SKILL_Synchronize_DESC,
        .capacity = MSG_SKILL_Synchronize_CAPACITY,
        .price = MSG_SKILL_Synchronize_PRICE,
        .icon = GFX_SkillIcon_Synchronize,
    },
#endif

#if (defined(SID_StaffSavant) && COMMON_SKILL_VALID(SID_StaffSavant))
    [SID_StaffSavant] = {
        .name = MSG_SKILL_StaffSavant_NAME,
        .desc = MSG_SKILL_StaffSavant_DESC,
        .capacity = MSG_SKILL_StaffSavant_CAPACITY,
        .price = MSG_SKILL_StaffSavant_PRICE,
        .icon = GFX_SkillIcon_StaffSavant,
    },
#endif

#if (defined(SID_Skyguard) && COMMON_SKILL_VALID(SID_Skyguard))
    [SID_Skyguard] = {
        .name = MSG_SKILL_Skyguard_NAME,
        .desc = MSG_SKILL_Skyguard_DESC,
        .capacity = MSG_SKILL_Skyguard_CAPACITY,
        .price = MSG_SKILL_Skyguard_PRICE,
        .icon = GFX_SkillIcon_Skyguard,
    },
#endif

#if (defined(SID_Horseguard) && COMMON_SKILL_VALID(SID_Horseguard))
    [SID_Horseguard] = {
        .name = MSG_SKILL_Horseguard_NAME,
        .desc = MSG_SKILL_Horseguard_DESC,
        .capacity = MSG_SKILL_Horseguard_CAPACITY,
        .price = MSG_SKILL_Horseguard_PRICE,
        .icon = GFX_SkillIcon_Horseguard,
    },
#endif

#if (defined(SID_Armorboost) && COMMON_SKILL_VALID(SID_Armorboost))
    [SID_Armorboost] = {
        .name = MSG_SKILL_Armorboost_NAME,
        .desc = MSG_SKILL_Armorboost_DESC,
        .capacity = MSG_SKILL_Armorboost_CAPACITY,
        .price = MSG_SKILL_Armorboost_PRICE,
        .icon = GFX_SkillIcon_Armorboost,
    },
#endif

#if (defined(SID_GentleFlower) && COMMON_SKILL_VALID(SID_GentleFlower))
    [SID_GentleFlower] = {
        .name = MSG_SKILL_GentleFlower_NAME,
        .desc = MSG_SKILL_GentleFlower_DESC,
        .capacity = MSG_SKILL_GentleFlower_CAPACITY,
        .price = MSG_SKILL_GentleFlower_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Admiration) && COMMON_SKILL_VALID(SID_Admiration))
    [SID_Admiration] = {
        .name = MSG_SKILL_Admiration_NAME,
        .desc = MSG_SKILL_Admiration_DESC,
        .capacity = MSG_SKILL_Admiration_CAPACITY,
        .price = MSG_SKILL_Admiration_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FairyTaleFolk) && COMMON_SKILL_VALID(SID_FairyTaleFolk))
    [SID_FairyTaleFolk] = {
        .name = MSG_SKILL_FairyTaleFolk_NAME,
        .desc = MSG_SKILL_FairyTaleFolk_DESC,
        .capacity = MSG_SKILL_FairyTaleFolk_CAPACITY,
        .price = MSG_SKILL_FairyTaleFolk_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_InitCalm) && COMMON_SKILL_VALID(SID_InitCalm))
    [SID_InitCalm] = {
        .name = MSG_SKILL_InitCalm_NAME,
        .desc = MSG_SKILL_InitCalm_DESC,
        .capacity = MSG_SKILL_InitCalm_CAPACITY,
        .price = MSG_SKILL_InitCalm_PRICE,
        .icon = GFX_SkillIcon_InitCalm,
    },
#endif

#if (defined(SID_InitClever) && COMMON_SKILL_VALID(SID_InitClever))
    [SID_InitClever] = {
        .name = MSG_SKILL_InitClever_NAME,
        .desc = MSG_SKILL_InitClever_DESC,
        .capacity = MSG_SKILL_InitClever_CAPACITY,
        .price = MSG_SKILL_InitClever_PRICE,
        .icon = GFX_SkillIcon_InitClever,
    },
#endif

#if (defined(SID_InitDeft) && COMMON_SKILL_VALID(SID_InitDeft))
    [SID_InitDeft] = {
        .name = MSG_SKILL_InitDeft_NAME,
        .desc = MSG_SKILL_InitDeft_DESC,
        .capacity = MSG_SKILL_InitDeft_CAPACITY,
        .price = MSG_SKILL_InitDeft_PRICE,
        .icon = GFX_SkillIcon_InitDeft,
    },
#endif

#if (defined(SID_InitLucky) && COMMON_SKILL_VALID(SID_InitLucky))
    [SID_InitLucky] = {
        .name = MSG_SKILL_InitLucky_NAME,
        .desc = MSG_SKILL_InitLucky_DESC,
        .capacity = MSG_SKILL_InitLucky_CAPACITY,
        .price = MSG_SKILL_InitLucky_PRICE,
        .icon = GFX_SkillIcon_InitLucky,
    },
#endif

#if (defined(SID_InitNimble) && COMMON_SKILL_VALID(SID_InitNimble))
    [SID_InitNimble] = {
        .name = MSG_SKILL_InitNimble_NAME,
        .desc = MSG_SKILL_InitNimble_DESC,
        .capacity = MSG_SKILL_InitNimble_CAPACITY,
        .price = MSG_SKILL_InitNimble_PRICE,
        .icon = GFX_SkillIcon_InitNimble,
    },
#endif

#if (defined(SID_InitQuick) && COMMON_SKILL_VALID(SID_InitQuick))
    [SID_InitQuick] = {
        .name = MSG_SKILL_InitQuick_NAME,
        .desc = MSG_SKILL_InitQuick_DESC,
        .capacity = MSG_SKILL_InitQuick_CAPACITY,
        .price = MSG_SKILL_InitQuick_PRICE,
        .icon = GFX_SkillIcon_InitQuick,
    },
#endif

#if (defined(SID_InitSpectrum) && COMMON_SKILL_VALID(SID_InitSpectrum))
    [SID_InitSpectrum] = {
        .name = MSG_SKILL_InitSpectrum_NAME,
        .desc = MSG_SKILL_InitSpectrum_DESC,
        .capacity = MSG_SKILL_InitSpectrum_CAPACITY,
        .price = MSG_SKILL_InitSpectrum_PRICE,
        .icon = GFX_SkillIcon_InitSpectrum,
    },
#endif

#if (defined(SID_InitStrong) && COMMON_SKILL_VALID(SID_InitStrong))
    [SID_InitStrong] = {
        .name = MSG_SKILL_InitStrong_NAME,
        .desc = MSG_SKILL_InitStrong_DESC,
        .capacity = MSG_SKILL_InitStrong_CAPACITY,
        .price = MSG_SKILL_InitStrong_PRICE,
        .icon = GFX_SkillIcon_InitStrong,
    },
#endif

#if (defined(SID_InitSturdy) && COMMON_SKILL_VALID(SID_InitSturdy))
    [SID_InitSturdy] = {
        .name = MSG_SKILL_InitSturdy_NAME,
        .desc = MSG_SKILL_InitSturdy_DESC,
        .capacity = MSG_SKILL_InitSturdy_CAPACITY,
        .price = MSG_SKILL_InitSturdy_PRICE,
        .icon = GFX_SkillIcon_InitSturdy,
    },
#endif

#if (defined(SID_Lifetaker) && COMMON_SKILL_VALID(SID_Lifetaker))
    [SID_Lifetaker] = {
        .name = MSG_SKILL_Lifetaker_NAME,
        .desc = MSG_SKILL_Lifetaker_DESC,
        .capacity = MSG_SKILL_Lifetaker_CAPACITY,
        .price = MSG_SKILL_Lifetaker_PRICE,
        .icon = GFX_SkillIcon_Lifetaker,
    },
#endif

#if (defined(SID_Analytic) && COMMON_SKILL_VALID(SID_Analytic))
    [SID_Analytic] = {
        .name = MSG_SKILL_Analytic_NAME,
        .desc = MSG_SKILL_Analytic_DESC,
        .capacity = MSG_SKILL_Analytic_CAPACITY,
        .price = MSG_SKILL_Analytic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_OathDefense) && COMMON_SKILL_VALID(SID_OathDefense))
    [SID_OathDefense] = {
        .name = MSG_SKILL_OathDefense_NAME,
        .desc = MSG_SKILL_OathDefense_DESC,
        .capacity = MSG_SKILL_OathDefense_CAPACITY,
        .price = MSG_SKILL_OathDefense_PRICE,
        .icon = GFX_SkillIcon_OathDefense,
    },
#endif

#if (defined(SID_OathLuck) && COMMON_SKILL_VALID(SID_OathLuck))
    [SID_OathLuck] = {
        .name = MSG_SKILL_OathLuck_NAME,
        .desc = MSG_SKILL_OathLuck_DESC,
        .capacity = MSG_SKILL_OathLuck_CAPACITY,
        .price = MSG_SKILL_OathLuck_PRICE,
        .icon = GFX_SkillIcon_OathLuck,
    },
#endif

#if (defined(SID_OathMagic) && COMMON_SKILL_VALID(SID_OathMagic))
    [SID_OathMagic] = {
        .name = MSG_SKILL_OathMagic_NAME,
        .desc = MSG_SKILL_OathMagic_DESC,
        .capacity = MSG_SKILL_OathMagic_CAPACITY,
        .price = MSG_SKILL_OathMagic_PRICE,
        .icon = GFX_SkillIcon_OathMagic,
    },
#endif

#if (defined(SID_CounterRoar) && COMMON_SKILL_VALID(SID_CounterRoar))
    [SID_CounterRoar] = {
        .name = MSG_SKILL_CounterRoar_NAME,
        .desc = MSG_SKILL_CounterRoar_DESC,
        .capacity = MSG_SKILL_CounterRoar_CAPACITY,
        .price = MSG_SKILL_CounterRoar_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Gambit) && COMMON_SKILL_VALID(SID_Gambit))
    [SID_Gambit] = {
        .name = MSG_SKILL_Gambit_NAME,
        .desc = MSG_SKILL_Gambit_DESC,
        .capacity = MSG_SKILL_Gambit_CAPACITY,
        .price = MSG_SKILL_Gambit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MagicGambit) && COMMON_SKILL_VALID(SID_MagicGambit))
    [SID_MagicGambit] = {
        .name = MSG_SKILL_MagicGambit_NAME,
        .desc = MSG_SKILL_MagicGambit_DESC,
        .capacity = MSG_SKILL_MagicGambit_CAPACITY,
        .price = MSG_SKILL_MagicGambit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BeastAssault) && COMMON_SKILL_VALID(SID_BeastAssault))
    [SID_BeastAssault] = {
        .name = MSG_SKILL_BeastAssault_NAME,
        .desc = MSG_SKILL_BeastAssault_DESC,
        .capacity = MSG_SKILL_BeastAssault_CAPACITY,
        .price = MSG_SKILL_BeastAssault_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LightAndDark) && COMMON_SKILL_VALID(SID_LightAndDark))
    [SID_LightAndDark] = {
        .name = MSG_SKILL_LightAndDark_NAME,
        .desc = MSG_SKILL_LightAndDark_DESC,
        .capacity = MSG_SKILL_LightAndDark_CAPACITY,
        .price = MSG_SKILL_LightAndDark_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_OathMovement) && COMMON_SKILL_VALID(SID_OathMovement))
    [SID_OathMovement] = {
        .name = MSG_SKILL_OathMovement_NAME,
        .desc = MSG_SKILL_OathMovement_DESC,
        .capacity = MSG_SKILL_OathMovement_CAPACITY,
        .price = MSG_SKILL_OathMovement_PRICE,
        .icon = GFX_SkillIcon_OathMovement,
    },
#endif

#if (defined(SID_OathResistance) && COMMON_SKILL_VALID(SID_OathResistance))
    [SID_OathResistance] = {
        .name = MSG_SKILL_OathResistance_NAME,
        .desc = MSG_SKILL_OathResistance_DESC,
        .capacity = MSG_SKILL_OathResistance_CAPACITY,
        .price = MSG_SKILL_OathResistance_PRICE,
        .icon = GFX_SkillIcon_OathResistance,
    },
#endif

#if (defined(SID_OathSkill) && COMMON_SKILL_VALID(SID_OathSkill))
    [SID_OathSkill] = {
        .name = MSG_SKILL_OathSkill_NAME,
        .desc = MSG_SKILL_OathSkill_DESC,
        .capacity = MSG_SKILL_OathSkill_CAPACITY,
        .price = MSG_SKILL_OathSkill_PRICE,
        .icon = GFX_SkillIcon_OathSkill,
    },
#endif

#if (defined(SID_OathSpeed) && COMMON_SKILL_VALID(SID_OathSpeed))
    [SID_OathSpeed] = {
        .name = MSG_SKILL_OathSpeed_NAME,
        .desc = MSG_SKILL_OathSpeed_DESC,
        .capacity = MSG_SKILL_OathSpeed_CAPACITY,
        .price = MSG_SKILL_OathSpeed_PRICE,
        .icon = GFX_SkillIcon_OathSpeed,
    },
#endif

#if (defined(SID_OathStrength) && COMMON_SKILL_VALID(SID_OathStrength))
    [SID_OathStrength] = {
        .name = MSG_SKILL_OathStrength_NAME,
        .desc = MSG_SKILL_OathStrength_DESC,
        .capacity = MSG_SKILL_OathStrength_CAPACITY,
        .price = MSG_SKILL_OathStrength_PRICE,
        .icon = GFX_SkillIcon_OathStrength,
    },
#endif

#if (defined(SID_DivinePulse) && COMMON_SKILL_VALID(SID_DivinePulse))
    [SID_DivinePulse] = {
        .name = MSG_SKILL_DivinePulse_NAME,
        .desc = MSG_SKILL_DivinePulse_DESC,
        .capacity = MSG_SKILL_DivinePulse_CAPACITY,
        .price = MSG_SKILL_DivinePulse_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MysticBoost) && COMMON_SKILL_VALID(SID_MysticBoost))
    [SID_MysticBoost] = {
        .name = MSG_SKILL_MysticBoost_NAME,
        .desc = MSG_SKILL_MysticBoost_DESC,
        .capacity = MSG_SKILL_MysticBoost_CAPACITY,
        .price = MSG_SKILL_MysticBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GuardBearing) && COMMON_SKILL_VALID(SID_GuardBearing))
    [SID_GuardBearing] = {
        .name = MSG_SKILL_GuardBearing_NAME,
        .desc = MSG_SKILL_GuardBearing_DESC,
        .capacity = MSG_SKILL_GuardBearing_CAPACITY,
        .price = MSG_SKILL_GuardBearing_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Spurn) && COMMON_SKILL_VALID(SID_Spurn))
    [SID_Spurn] = {
        .name = MSG_SKILL_Spurn_NAME,
        .desc = MSG_SKILL_Spurn_DESC,
        .capacity = MSG_SKILL_Spurn_CAPACITY,
        .price = MSG_SKILL_Spurn_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Bushido) && COMMON_SKILL_VALID(SID_Bushido))
    [SID_Bushido] = {
        .name = MSG_SKILL_Bushido_NAME,
        .desc = MSG_SKILL_Bushido_DESC,
        .capacity = MSG_SKILL_Bushido_CAPACITY,
        .price = MSG_SKILL_Bushido_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DragonWall) && COMMON_SKILL_VALID(SID_DragonWall))
    [SID_DragonWall] = {
        .name = MSG_SKILL_DragonWall_NAME,
        .desc = MSG_SKILL_DragonWall_DESC,
        .capacity = MSG_SKILL_DragonWall_CAPACITY,
        .price = MSG_SKILL_DragonWall_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DragonsWrath) && COMMON_SKILL_VALID(SID_DragonsWrath))
    [SID_DragonsWrath] = {
        .name = MSG_SKILL_DragonsWrath_NAME,
        .desc = MSG_SKILL_DragonsWrath_DESC,
        .capacity = MSG_SKILL_DragonsWrath_CAPACITY,
        .price = MSG_SKILL_DragonsWrath_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BlueLionRule) && COMMON_SKILL_VALID(SID_BlueLionRule))
    [SID_BlueLionRule] = {
        .name = MSG_SKILL_BlueLionRule_NAME,
        .desc = MSG_SKILL_BlueLionRule_DESC,
        .capacity = MSG_SKILL_BlueLionRule_CAPACITY,
        .price = MSG_SKILL_BlueLionRule_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FaerghusAncestry) && COMMON_SKILL_VALID(SID_FaerghusAncestry))
    [SID_FaerghusAncestry] = {
        .name = MSG_SKILL_FaerghusAncestry_NAME,
        .desc = MSG_SKILL_FaerghusAncestry_DESC,
        .capacity = MSG_SKILL_FaerghusAncestry_CAPACITY,
        .price = MSG_SKILL_FaerghusAncestry_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CrusaderWard) && COMMON_SKILL_VALID(SID_CrusaderWard))
    [SID_CrusaderWard] = {
        .name = MSG_SKILL_CrusaderWard_NAME,
        .desc = MSG_SKILL_CrusaderWard_DESC,
        .capacity = MSG_SKILL_CrusaderWard_CAPACITY,
        .price = MSG_SKILL_CrusaderWard_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_YuneWhispers) && COMMON_SKILL_VALID(SID_YuneWhispers))
    [SID_YuneWhispers] = {
        .name = MSG_SKILL_YuneWhispers_NAME,
        .desc = MSG_SKILL_YuneWhispers_DESC,
        .capacity = MSG_SKILL_YuneWhispers_CAPACITY,
        .price = MSG_SKILL_YuneWhispers_PRICE,
        .icon = GFX_SkillIcon_YuneWhispers,
    },
#endif

#if (defined(SID_Plus) && COMMON_SKILL_VALID(SID_Plus))
    [SID_Plus] = {
        .name = MSG_SKILL_Plus_NAME,
        .desc = MSG_SKILL_Plus_DESC,
        .capacity = MSG_SKILL_Plus_CAPACITY,
        .price = MSG_SKILL_Plus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

  #if (defined(SID_Minus) && COMMON_SKILL_VALID(SID_Minus))
    [SID_Minus] = {
        .name = MSG_SKILL_Minus_NAME,
        .desc = MSG_SKILL_Minus_DESC,
        .capacity = MSG_SKILL_Minus_CAPACITY,
        .price = MSG_SKILL_Minus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Chlorophyll) && COMMON_SKILL_VALID(SID_Chlorophyll))
    [SID_Chlorophyll] = {
        .name = MSG_SKILL_Chlorophyll_NAME,
        .desc = MSG_SKILL_Chlorophyll_DESC,
        .capacity = MSG_SKILL_Chlorophyll_CAPACITY,
        .price = MSG_SKILL_Chlorophyll_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SlushRush) && COMMON_SKILL_VALID(SID_SlushRush))
    [SID_SlushRush] = {
        .name = MSG_SKILL_SlushRush_NAME,
        .desc = MSG_SKILL_SlushRush_DESC,
        .capacity = MSG_SKILL_SlushRush_CAPACITY,
        .price = MSG_SKILL_SlushRush_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SandRush) && COMMON_SKILL_VALID(SID_SandRush))
    [SID_SandRush] = {
        .name = MSG_SKILL_SandRush_NAME,
        .desc = MSG_SKILL_SandRush_DESC,
        .capacity = MSG_SKILL_SandRush_CAPACITY,
        .price = MSG_SKILL_SandRush_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SwiftSwim) && COMMON_SKILL_VALID(SID_SwiftSwim))
    [SID_SwiftSwim] = {
        .name = MSG_SKILL_SwiftSwim_NAME,
        .desc = MSG_SKILL_SwiftSwim_DESC,
        .capacity = MSG_SKILL_SwiftSwim_CAPACITY,
        .price = MSG_SKILL_SwiftSwim_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RainDish) && COMMON_SKILL_VALID(SID_RainDish))
    [SID_RainDish] = {
        .name = MSG_SKILL_RainDish_NAME,
        .desc = MSG_SKILL_RainDish_DESC,
        .capacity = MSG_SKILL_RainDish_CAPACITY,
        .price = MSG_SKILL_RainDish_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Assist) && COMMON_SKILL_VALID(SID_Assist))
    [SID_Assist] = {
        .name = MSG_SKILL_Assist_NAME,
        .desc = MSG_SKILL_Assist_DESC,
        .capacity = MSG_SKILL_Assist_CAPACITY,
        .price = MSG_SKILL_Assist_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Synergism) && COMMON_SKILL_VALID(SID_Synergism))
    [SID_Synergism] = {
        .name = MSG_SKILL_Synergism_NAME,
        .desc = MSG_SKILL_Synergism_DESC,
        .capacity = MSG_SKILL_Synergism_CAPACITY,
        .price = MSG_SKILL_Synergism_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Pass) && COMMON_SKILL_VALID(SID_Pass))
    [SID_Pass] = {
        .name = MSG_SKILL_Pass_NAME,
        .desc = MSG_SKILL_Pass_DESC,
        .capacity = MSG_SKILL_Pass_CAPACITY,
        .price = MSG_SKILL_Pass_PRICE,
        .icon = GFX_SkillIcon_Pass,
    },
#endif

#if (defined(SID_Obstruct) && COMMON_SKILL_VALID(SID_Obstruct))
    [SID_Obstruct] = {
        .name = MSG_SKILL_Obstruct_NAME,
        .desc = MSG_SKILL_Obstruct_DESC,
        .capacity = MSG_SKILL_Obstruct_CAPACITY,
        .price = MSG_SKILL_Obstruct_PRICE,
        .icon = GFX_SkillIcon_Obstruct,
    },
#endif

#if (defined(SID_Guidance) && COMMON_SKILL_VALID(SID_Guidance))
    [SID_Guidance] = {
        .name = MSG_SKILL_Guidance_NAME,
        .desc = MSG_SKILL_Guidance_DESC,
        .capacity = MSG_SKILL_Guidance_CAPACITY,
        .price = MSG_SKILL_Guidance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FlierGuidance) && COMMON_SKILL_VALID(SID_FlierGuidance))
    [SID_FlierGuidance] = {
        .name = MSG_SKILL_FlierGuidance_NAME,
        .desc = MSG_SKILL_FlierGuidance_DESC,
        .capacity = MSG_SKILL_FlierGuidance_CAPACITY,
        .price = MSG_SKILL_FlierGuidance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SoaringGuidance) && COMMON_SKILL_VALID(SID_SoaringGuidance))
    [SID_SoaringGuidance] = {
        .name = MSG_SKILL_SoaringGuidance_NAME,
        .desc = MSG_SKILL_SoaringGuidance_DESC,
        .capacity = MSG_SKILL_SoaringGuidance_CAPACITY,
        .price = MSG_SKILL_SoaringGuidance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Aerobatics) && COMMON_SKILL_VALID(SID_Aerobatics))
    [SID_Aerobatics] = {
        .name = MSG_SKILL_Aerobatics_NAME,
        .desc = MSG_SKILL_Aerobatics_DESC,
        .capacity = MSG_SKILL_Aerobatics_CAPACITY,
        .price = MSG_SKILL_Aerobatics_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FlierFormation) && COMMON_SKILL_VALID(SID_FlierFormation))
    [SID_FlierFormation] = {
        .name = MSG_SKILL_FlierFormation_NAME,
        .desc = MSG_SKILL_FlierFormation_DESC,
        .capacity = MSG_SKILL_FlierFormation_CAPACITY,
        .price = MSG_SKILL_FlierFormation_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SoaringWings) && COMMON_SKILL_VALID(SID_SoaringWings))
    [SID_SoaringWings] = {
        .name = MSG_SKILL_SoaringWings_NAME,
        .desc = MSG_SKILL_SoaringWings_DESC,
        .capacity = MSG_SKILL_SoaringWings_CAPACITY,
        .price = MSG_SKILL_SoaringWings_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DetailedReport) && COMMON_SKILL_VALID(SID_DetailedReport))
    [SID_DetailedReport] = {
        .name = MSG_SKILL_DetailedReport_NAME,
        .desc = MSG_SKILL_DetailedReport_DESC,
        .capacity = MSG_SKILL_DetailedReport_CAPACITY,
        .price = MSG_SKILL_DetailedReport_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_VengefulFighter) && COMMON_SKILL_VALID(SID_VengefulFighter))
    [SID_VengefulFighter] = {
        .name = MSG_SKILL_VengefulFighter_NAME,
         .desc = MSG_SKILL_VengefulFighter_DESC,
         .capacity = MSG_SKILL_VengefulFighter_CAPACITY,
        .price = MSG_SKILL_VengefulFighter_PRICE,
        .icon = GFX_SkillIcon_VengefulFighter,
    },
#endif

#if (defined(SID_RapidFighter) && COMMON_SKILL_VALID(SID_RapidFighter))
    [SID_RapidFighter] = {
        .name = MSG_SKILL_RapidFighter_NAME,
         .desc = MSG_SKILL_RapidFighter_DESC,
         .capacity = MSG_SKILL_RapidFighter_CAPACITY,
        .price = MSG_SKILL_RapidFighter_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RecklessFighter) && COMMON_SKILL_VALID(SID_RecklessFighter))
    [SID_RecklessFighter] = {
        .name = MSG_SKILL_RecklessFighter_NAME,
         .desc = MSG_SKILL_RecklessFighter_DESC,
         .capacity = MSG_SKILL_RecklessFighter_CAPACITY,
        .price = MSG_SKILL_RecklessFighter_PRICE,
        .icon = GFX_SkillIcon_RecklessFighter,
    },
#endif

#if (defined(SID_BoldFighter) && COMMON_SKILL_VALID(SID_BoldFighter))
    [SID_BoldFighter] = {
        .name = MSG_SKILL_BoldFighter_NAME,
         .desc = MSG_SKILL_BoldFighter_DESC,
         .capacity = MSG_SKILL_BoldFighter_CAPACITY,
        .price = MSG_SKILL_BoldFighter_PRICE,
        .icon = GFX_SkillIcon_BoldFighter,
    },
#endif

#if (defined(SID_Dazzle) && COMMON_SKILL_VALID(SID_Dazzle))
    [SID_Dazzle] = {
        .name = MSG_SKILL_Dazzle_NAME,
         .desc = MSG_SKILL_Dazzle_DESC,
         .capacity = MSG_SKILL_Dazzle_CAPACITY,
        .price = MSG_SKILL_Dazzle_PRICE,
        .icon = GFX_SkillIcon_Dazzle,
    },
#endif

#if (defined(SID_Boon) && COMMON_SKILL_VALID(SID_Boon))
    [SID_Boon] = {
        .name = MSG_SKILL_Boon_NAME,
         .desc = MSG_SKILL_Boon_DESC,
         .capacity = MSG_SKILL_Boon_CAPACITY,
        .price = MSG_SKILL_Boon_PRICE,
        .icon = GFX_SkillIcon_Boon,
    },
#endif

#if (defined(SID_BidingBlow) && COMMON_SKILL_VALID(SID_BidingBlow))
    [SID_BidingBlow] = {
        .name = MSG_SKILL_BidingBlow_NAME,
         .desc = MSG_SKILL_BidingBlow_DESC,
         .capacity = MSG_SKILL_BidingBlow_CAPACITY,
        .price = MSG_SKILL_BidingBlow_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Discipline) && COMMON_SKILL_VALID(SID_Discipline))
    [SID_Discipline] = {
        .name = MSG_SKILL_Discipline_NAME,
         .desc = MSG_SKILL_Discipline_DESC,
         .capacity = MSG_SKILL_Discipline_CAPACITY,
        .price = MSG_SKILL_Discipline_PRICE,
        .icon = GFX_SkillIcon_Discipline,
    },
#endif

#if (defined(SID_AdvantageChaser) && COMMON_SKILL_VALID(SID_AdvantageChaser))
    [SID_AdvantageChaser] = {
        .name = MSG_SKILL_AdvantageChaser_NAME,
         .desc = MSG_SKILL_AdvantageChaser_DESC,
         .capacity = MSG_SKILL_AdvantageChaser_CAPACITY,
        .price = MSG_SKILL_AdvantageChaser_PRICE,
        .icon = GFX_SkillIcon_AdvantageChaser,
    },
#endif

#if (defined(SID_Nonconforming) && COMMON_SKILL_VALID(SID_Nonconforming))
    [SID_Nonconforming] = {
        .name = MSG_SKILL_Nonconforming_NAME,
        .desc = MSG_SKILL_Nonconforming_DESC,
        .capacity = MSG_SKILL_Nonconforming_CAPACITY,
        .price = MSG_SKILL_Nonconforming_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Moonlight) && COMMON_SKILL_VALID(SID_Moonlight))
    [SID_Moonlight] = {
        .name = MSG_SKILL_Moonlight_NAME,
        .desc = MSG_SKILL_Moonlight_DESC,
        .capacity = MSG_SKILL_Moonlight_CAPACITY,
        .price = MSG_SKILL_Moonlight_PRICE,
        .icon = GFX_SkillIcon_Moonlight,
    },
#endif

#if (defined(SID_RouseDefense) && COMMON_SKILL_VALID(SID_RouseDefense))
    [SID_RouseDefense] = {
        .name = MSG_SKILL_RouseDefense_NAME,
        .desc = MSG_SKILL_RouseDefense_DESC,
        .capacity = MSG_SKILL_RouseDefense_CAPACITY,
        .price = MSG_SKILL_RouseDefense_PRICE,
        .icon = GFX_SkillIcon_RouseDefense,
    },
#endif

#if (defined(SID_RouseLuck) && COMMON_SKILL_VALID(SID_RouseLuck))
    [SID_RouseLuck] = {
        .name = MSG_SKILL_RouseLuck_NAME,
        .desc = MSG_SKILL_RouseLuck_DESC,
        .capacity = MSG_SKILL_RouseLuck_CAPACITY,
        .price = MSG_SKILL_RouseLuck_PRICE,
        .icon = GFX_SkillIcon_RouseLuck,
    },
#endif

#if (defined(SID_RouseMagic) && COMMON_SKILL_VALID(SID_RouseMagic))
    [SID_RouseMagic] = {
        .name = MSG_SKILL_RouseMagic_NAME,
        .desc = MSG_SKILL_RouseMagic_DESC,
        .capacity = MSG_SKILL_RouseMagic_CAPACITY,
        .price = MSG_SKILL_RouseMagic_PRICE,
        .icon = GFX_SkillIcon_RouseMagic,
    },
#endif

#if (defined(SID_RouseMovement) && COMMON_SKILL_VALID(SID_RouseMovement))
    [SID_RouseMovement] = {
        .name = MSG_SKILL_RouseMovement_NAME,
        .desc = MSG_SKILL_RouseMovement_DESC,
        .capacity = MSG_SKILL_RouseMovement_CAPACITY,
        .price = MSG_SKILL_RouseMovement_PRICE,
        .icon = GFX_SkillIcon_RouseMovement,
    },
#endif

#if (defined(SID_RouseResistance) && COMMON_SKILL_VALID(SID_RouseResistance))
    [SID_RouseResistance] = {
        .name = MSG_SKILL_RouseResistance_NAME,
        .desc = MSG_SKILL_RouseResistance_DESC,
        .capacity = MSG_SKILL_RouseResistance_CAPACITY,
        .price = MSG_SKILL_RouseResistance_PRICE,
        .icon = GFX_SkillIcon_RouseResistance,
    },
#endif

#if (defined(SID_RouseSkill) && COMMON_SKILL_VALID(SID_RouseSkill))
    [SID_RouseSkill] = {
        .name = MSG_SKILL_RouseSkill_NAME,
        .desc = MSG_SKILL_RouseSkill_DESC,
        .capacity = MSG_SKILL_RouseSkill_CAPACITY,
        .price = MSG_SKILL_RouseSkill_PRICE,
        .icon = GFX_SkillIcon_RouseSkill,
    },
#endif

#if (defined(SID_RouseSpeed) && COMMON_SKILL_VALID(SID_RouseSpeed))
    [SID_RouseSpeed] = {
        .name = MSG_SKILL_RouseSpeed_NAME,
        .desc = MSG_SKILL_RouseSpeed_DESC,
        .capacity = MSG_SKILL_RouseSpeed_CAPACITY,
        .price = MSG_SKILL_RouseSpeed_PRICE,
        .icon = GFX_SkillIcon_RouseSpeed,
    },
#endif

#if (defined(SID_RouseStrength) && COMMON_SKILL_VALID(SID_RouseStrength))
    [SID_RouseStrength] = {
        .name = MSG_SKILL_RouseStrength_NAME,
        .desc = MSG_SKILL_RouseStrength_DESC,
        .capacity = MSG_SKILL_RouseStrength_CAPACITY,
        .price = MSG_SKILL_RouseStrength_PRICE,
        .icon = GFX_SkillIcon_RouseStrength,
    },
#endif

#if (defined(SID_BlackMagic) && COMMON_SKILL_VALID(SID_BlackMagic))
    [SID_BlackMagic] = {
        .name = MSG_SKILL_BlackMagic_NAME,
        .desc = MSG_SKILL_BlackMagic_DESC,
        .capacity = MSG_SKILL_BlackMagic_CAPACITY,
        .price = MSG_SKILL_BlackMagic_PRICE,
        .icon = GFX_SkillIcon_BlackMagic,
    },
#endif

#if (defined(SID_Relief) && COMMON_SKILL_VALID(SID_Relief))
    [SID_Relief] = {
        .name = MSG_SKILL_Relief_NAME,
        .desc = MSG_SKILL_Relief_DESC,
        .capacity = MSG_SKILL_Relief_CAPACITY,
        .price = MSG_SKILL_Relief_PRICE,
        .icon = GFX_SkillIcon_Relief,
    },
#endif

#if (defined(SID_Camaraderie) && COMMON_SKILL_VALID(SID_Camaraderie))
    [SID_Camaraderie] = {
        .name = MSG_SKILL_Camaraderie_NAME,
        .desc = MSG_SKILL_Camaraderie_DESC,
        .capacity = MSG_SKILL_Camaraderie_CAPACITY,
        .price = MSG_SKILL_Camaraderie_PRICE,
        .icon = GFX_SkillIcon_Camaraderie,
    },
#endif

#if (defined(SID_Amaterasu) && (SID_Amaterasu < MAX_SKILL_NUM)) 
    [SID_Amaterasu] = {
        .name = MSG_SKILL_Amaterasu_NAME,
        .desc = MSG_SKILL_Amaterasu_DESC,
        .capacity = MSG_SKILL_Amaterasu_CAPACITY,
        .price = MSG_SKILL_Amaterasu_PRICE,
        .icon = GFX_SkillIcon_Amaterasu,
      },
#endif

#if (defined(SID_Counter) && COMMON_SKILL_VALID(SID_Counter))
    [SID_Counter] = {
        .name = MSG_SKILL_Counter_NAME,
        .desc = MSG_SKILL_Counter_DESC,
        .capacity = MSG_SKILL_Counter_CAPACITY,
        .price = MSG_SKILL_Counter_PRICE,
        .icon = GFX_SkillIcon_Counter,
    },
#endif

#if (defined(SID_LastWord) && COMMON_SKILL_VALID(SID_LastWord))
    [SID_LastWord] = {
        .name = MSG_SKILL_LastWord_NAME,
         .desc = MSG_SKILL_LastWord_DESC,
         .capacity = MSG_SKILL_LastWord_CAPACITY,
        .price = MSG_SKILL_LastWord_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PowerStaff) && COMMON_SKILL_VALID(SID_PowerStaff))
    [SID_PowerStaff] = {
        .name = MSG_SKILL_PowerStaff_NAME,
        .desc = MSG_SKILL_PowerStaff_DESC,
        .capacity = MSG_SKILL_PowerStaff_CAPACITY,
        .price = MSG_SKILL_PowerStaff_PRICE,
        .icon = GFX_SkillIcon_PowerStaff,
    },
#endif

#if (defined(SID_Multiscale) && COMMON_SKILL_VALID(SID_Multiscale))
    [SID_Multiscale] = {
        .name = MSG_SKILL_Multiscale_NAME,
        .desc = MSG_SKILL_Multiscale_DESC,
        .capacity = MSG_SKILL_Multiscale_CAPACITY,
        .price = MSG_SKILL_Multiscale_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PridefulWarrior) && COMMON_SKILL_VALID(SID_PridefulWarrior))
    [SID_PridefulWarrior] = {
        .name = MSG_SKILL_PridefulWarrior_NAME,
         .desc = MSG_SKILL_PridefulWarrior_DESC,
         .capacity = MSG_SKILL_PridefulWarrior_CAPACITY,
        .price = MSG_SKILL_PridefulWarrior_PRICE,
        .icon = GFX_SkillIcon_PridefulWarrior,
    },
#endif

#if (defined(SID_LiveToServe) && COMMON_SKILL_VALID(SID_LiveToServe))
    [SID_LiveToServe] = {
        .name = MSG_SKILL_LiveToServe_NAME,
        .desc = MSG_SKILL_LiveToServe_DESC,
        .capacity = MSG_SKILL_LiveToServe_CAPACITY,
        .price = MSG_SKILL_LiveToServe_PRICE,
        .icon = GFX_SkillIcon_LiveToServe,
    },
#endif

#if (defined(SID_MageSlayer) && COMMON_SKILL_VALID(SID_MageSlayer))
    [SID_MageSlayer] = {
        .name = MSG_SKILL_MageSlayer_NAME,
        .desc = MSG_SKILL_MageSlayer_DESC,
        .capacity = MSG_SKILL_MageSlayer_CAPACITY,
        .price = MSG_SKILL_MageSlayer_PRICE,
        .icon = GFX_SkillIcon_MageSlayer,
    },
#endif

#if (defined(SID_FailGale) && COMMON_SKILL_VALID(SID_FailGale))
    [SID_FailGale] = {
        .name = MSG_SKILL_FailGale_NAME,
        .desc = MSG_SKILL_FailGale_DESC,
        .capacity = MSG_SKILL_FailGale_CAPACITY,
        .price = MSG_SKILL_FailGale_PRICE,
        .icon = GFX_SkillIcon_FailGale,
    },
#endif

#if (defined(SID_Clearance) && COMMON_SKILL_VALID(SID_Clearance))
    [SID_Clearance] = {
        .name = MSG_SKILL_Clearance_NAME,
        .desc = MSG_SKILL_Clearance_DESC,
        .capacity = MSG_SKILL_Clearance_CAPACITY,
        .price = MSG_SKILL_Clearance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TriangleAdept) && COMMON_SKILL_VALID(SID_TriangleAdept))
    [SID_TriangleAdept] = {
        .name = MSG_SKILL_TriangleAdept_NAME,
        .desc = MSG_SKILL_TriangleAdept_DESC,
        .capacity = MSG_SKILL_TriangleAdept_CAPACITY,
        .price = MSG_SKILL_TriangleAdept_PRICE,
        .icon = GFX_SkillIcon_TriangleAdept,
    },
#endif

#if (defined(SID_TriangleAdeptPlus) && COMMON_SKILL_VALID(SID_TriangleAdeptPlus))
    [SID_TriangleAdeptPlus] = {
        .name = MSG_SKILL_TriangleAdeptPlus_NAME,
        .desc = MSG_SKILL_TriangleAdeptPlus_DESC,
        .capacity = MSG_SKILL_TriangleAdeptPlus_CAPACITY,
        .price = MSG_SKILL_TriangleAdeptPlus_PRICE,
        .icon = GFX_SkillIcon_TriangleAdeptPlus,
    },
#endif

#if (defined(SID_TowerShieldPlus) && COMMON_SKILL_VALID(SID_TowerShieldPlus))
    [SID_TowerShieldPlus] = {
        .name = MSG_SKILL_TowerShieldPlus_NAME,
        .desc = MSG_SKILL_TowerShieldPlus_DESC,
        .capacity = MSG_SKILL_TowerShieldPlus_CAPACITY,
        .price = MSG_SKILL_TowerShieldPlus_PRICE,
        .icon = GFX_SkillIcon_TowerShieldPlus,
    },
#endif

#if (defined(SID_SolarPower) && COMMON_SKILL_VALID(SID_SolarPower))
    [SID_SolarPower] = {
        .name = MSG_SKILL_SolarPower_NAME,
        .desc = MSG_SKILL_SolarPower_DESC,
        .capacity = MSG_SKILL_SolarPower_CAPACITY,
        .price = MSG_SKILL_SolarPower_PRICE,
        .icon = GFX_SkillIcon_SolarPower,
    },
#endif

#if (defined(SID_COMBAT_Gamble) && COMMON_SKILL_VALID(SID_COMBAT_Gamble))
    [SID_COMBAT_Gamble] = {
        .name = MSG_SKILL_COMBAT_Gamble_NAME,
        .desc = MSG_SKILL_COMBAT_Gamble_DESC,
        .capacity = MSG_SKILL_COMBAT_Gamble_CAPACITY,
        .price = MSG_SKILL_COMBAT_Gamble_PRICE,
        .icon = GFX_SkillIcon_Gamble,
    },
#endif

#if (defined(SID_PoisonStrike) && COMMON_SKILL_VALID(SID_PoisonStrike))
    [SID_PoisonStrike] = {
        .name = MSG_SKILL_PoisonStrike_NAME,
        .desc = MSG_SKILL_PoisonStrike_DESC,
        .capacity = MSG_SKILL_PoisonStrike_CAPACITY,
        .price = MSG_SKILL_PoisonStrike_PRICE,
        .icon = GFX_SkillIcon_PoisonStrike,
    },
#endif

#if (defined(SID_GrislyWound) && COMMON_SKILL_VALID(SID_GrislyWound))
    [SID_GrislyWound] = {
        .name = MSG_SKILL_GrislyWound_NAME,
        .desc = MSG_SKILL_GrislyWound_DESC,
        .capacity = MSG_SKILL_GrislyWound_CAPACITY,
        .price = MSG_SKILL_GrislyWound_PRICE,
        .icon = GFX_SkillIcon_GrislyWound,
    },
#endif

#if (defined(SID_Poise) && COMMON_SKILL_VALID(SID_Poise))
    [SID_Poise] = {
        .name = MSG_SKILL_Poise_NAME,
        .desc = MSG_SKILL_Poise_DESC,
        .capacity = MSG_SKILL_Poise_CAPACITY,
        .price = MSG_SKILL_Poise_PRICE,
        .icon = GFX_SkillIcon_Poise,
    },
#endif

#if (defined(SID_COMBAT_Detonate) && COMMON_SKILL_VALID(SID_COMBAT_Detonate))
    [SID_COMBAT_Detonate] = {
        .name = MSG_SKILL_COMBAT_Detonate_NAME,
        .desc = MSG_SKILL_COMBAT_Detonate_DESC,
        .capacity = MSG_SKILL_COMBAT_Detonate_CAPACITY,
        .price = MSG_SKILL_COMBAT_Detonate_PRICE,
        .icon = GFX_SkillIcon_Detonate,
    },
#endif

#if (defined(SID_Enrage) && COMMON_SKILL_VALID(SID_Enrage))
    [SID_Enrage] = {
        .name = MSG_SKILL_Enrage_NAME,
        .desc = MSG_SKILL_Enrage_DESC,
        .capacity = MSG_SKILL_Enrage_CAPACITY,
        .price = MSG_SKILL_Enrage_PRICE,
        .icon = GFX_SkillIcon_Enrage,
    },
#endif

#if (defined(SID_Petrify) && COMMON_SKILL_VALID(SID_Petrify))
    [SID_Petrify] = {
        .name = MSG_SKILL_Petrify_NAME,
        .desc = MSG_SKILL_Petrify_DESC,
        .capacity = MSG_SKILL_Petrify_CAPACITY,
        .price = MSG_SKILL_Petrify_PRICE,
        .icon = GFX_SkillIcon_Petrify,
    },
#endif

#if (defined(SID_AirRaidAttack) && COMMON_SKILL_VALID(SID_AirRaidAttack))
    [SID_AirRaidAttack] = {
        .name = MSG_SKILL_AirRaidAttack_NAME,
        .desc = MSG_SKILL_AirRaidAttack_DESC,
        .capacity = MSG_SKILL_AirRaidAttack_CAPACITY,
        .price = MSG_SKILL_AirRaidAttack_PRICE,
        .icon = GFX_SkillIcon_AirRaidAttack,
    },
#endif

#if (defined(SID_AirRaidAvoid) && COMMON_SKILL_VALID(SID_AirRaidAvoid))
    [SID_AirRaidAvoid] = {
        .name = MSG_SKILL_AirRaidAvoid_NAME,
        .desc = MSG_SKILL_AirRaidAvoid_DESC,
        .capacity = MSG_SKILL_AirRaidAvoid_CAPACITY,
        .price = MSG_SKILL_AirRaidAvoid_PRICE,
        .icon = GFX_SkillIcon_AirRaidAvoid,
    },
#endif

#if (defined(SID_AirRaidCrit) && COMMON_SKILL_VALID(SID_AirRaidCrit))
    [SID_AirRaidCrit] = {
        .name = MSG_SKILL_AirRaidCrit_NAME,
        .desc = MSG_SKILL_AirRaidCrit_DESC,
        .capacity = MSG_SKILL_AirRaidCrit_CAPACITY,
        .price = MSG_SKILL_AirRaidCrit_PRICE,
        .icon = GFX_SkillIcon_AirRaidCrit,
    },
#endif

#if (defined(SID_AirRaidDefense) && COMMON_SKILL_VALID(SID_AirRaidDefense))
    [SID_AirRaidDefense] = {
        .name = MSG_SKILL_AirRaidDefense_NAME,
        .desc = MSG_SKILL_AirRaidDefense_DESC,
        .capacity = MSG_SKILL_AirRaidDefense_CAPACITY,
        .price = MSG_SKILL_AirRaidDefense_PRICE,
        .icon = GFX_SkillIcon_AirRaidDefense,
    },
#endif

#if (defined(SID_AirRaidHit) && COMMON_SKILL_VALID(SID_AirRaidHit))
    [SID_AirRaidHit] = {
        .name = MSG_SKILL_AirRaidHit_NAME,
        .desc = MSG_SKILL_AirRaidHit_DESC,
        .capacity = MSG_SKILL_AirRaidHit_CAPACITY,
        .price = MSG_SKILL_AirRaidHit_PRICE,
        .icon = GFX_SkillIcon_AirRaidHit,
    },
#endif

#if (defined(SID_AirRaidResistance) && COMMON_SKILL_VALID(SID_AirRaidResistance))
    [SID_AirRaidResistance] = {
        .name = MSG_SKILL_AirRaidResistance_NAME,
        .desc = MSG_SKILL_AirRaidResistance_DESC,
        .capacity = MSG_SKILL_AirRaidResistance_CAPACITY,
        .price = MSG_SKILL_AirRaidResistance_PRICE,
        .icon = GFX_SkillIcon_AirRaidResistance,
    },
#endif

#if (defined(SID_AirRaidSpeed) && COMMON_SKILL_VALID(SID_AirRaidSpeed))
    [SID_AirRaidSpeed] = {
        .name = MSG_SKILL_AirRaidSpeed_NAME,
        .desc = MSG_SKILL_AirRaidSpeed_DESC,
        .capacity = MSG_SKILL_AirRaidSpeed_CAPACITY,
        .price = MSG_SKILL_AirRaidSpeed_PRICE,
        .icon = GFX_SkillIcon_AirRaidSpeed,
    },
#endif

#if (defined(SID_FaireAnima) && COMMON_SKILL_VALID(SID_FaireAnima))
    [SID_FaireAnima] = {
        .name = MSG_SKILL_FaireAnima_NAME,
        .desc = MSG_SKILL_FaireAnima_DESC,
        .capacity = MSG_SKILL_FaireAnima_CAPACITY,
        .price = MSG_SKILL_FaireAnima_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FaireLight) && COMMON_SKILL_VALID(SID_FaireLight))
    [SID_FaireLight] = {
        .name = MSG_SKILL_FaireLight_NAME,
        .desc = MSG_SKILL_FaireLight_DESC,
        .capacity = MSG_SKILL_FaireLight_CAPACITY,
        .price = MSG_SKILL_FaireLight_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FaireDark) && COMMON_SKILL_VALID(SID_FaireDark))
    [SID_FaireDark] = {
        .name = MSG_SKILL_FaireDark_NAME,
        .desc = MSG_SKILL_FaireDark_DESC,
        .capacity = MSG_SKILL_FaireDark_CAPACITY,
        .price = MSG_SKILL_FaireDark_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SealDefense) && COMMON_SKILL_VALID(SID_SealDefense))
    [SID_SealDefense] = {
        .name = MSG_SKILL_SealDefense_NAME,
        .desc = MSG_SKILL_SealDefense_DESC,
        .capacity = MSG_SKILL_SealDefense_CAPACITY,
        .price = MSG_SKILL_SealDefense_PRICE,
        .icon = GFX_SkillIcon_SealDefense
    },
#endif

#if (defined(SID_SealLuck) && COMMON_SKILL_VALID(SID_SealLuck))
    [SID_SealLuck] = {
        .name = MSG_SKILL_SealLuck_NAME,
        .desc = MSG_SKILL_SealLuck_DESC,
        .capacity = MSG_SKILL_SealLuck_CAPACITY,
        .price = MSG_SKILL_SealLuck_PRICE,
        .icon = GFX_SkillIcon_SealLuck
    },
#endif

#if (defined(SID_SealMagic) && COMMON_SKILL_VALID(SID_SealMagic))
    [SID_SealMagic] = {
        .name = MSG_SKILL_SealMagic_NAME,
        .desc = MSG_SKILL_SealMagic_DESC,
        .capacity = MSG_SKILL_SealMagic_CAPACITY,
        .price = MSG_SKILL_SealMagic_PRICE,
        .icon = GFX_SkillIcon_SealMagic
    },
#endif

#if (defined(SID_SealResistance) && COMMON_SKILL_VALID(SID_SealResistance))
    [SID_SealResistance] = {
        .name = MSG_SKILL_SealResistance_NAME,
        .desc = MSG_SKILL_SealResistance_DESC,
        .capacity = MSG_SKILL_SealResistance_CAPACITY,
        .price = MSG_SKILL_SealResistance_PRICE,
        .icon = GFX_SkillIcon_SealResistance
    },
#endif

#if (defined(SID_SealSkill) && COMMON_SKILL_VALID(SID_SealSkill))
    [SID_SealSkill] = {
        .name = MSG_SKILL_SealSkill_NAME,
        .desc = MSG_SKILL_SealSkill_DESC,
        .capacity = MSG_SKILL_SealSkill_CAPACITY,
        .price = MSG_SKILL_SealSkill_PRICE,
        .icon = GFX_SkillIcon_SealSkill
    },
#endif

#if (defined(SID_SealSpeed) && COMMON_SKILL_VALID(SID_SealSpeed))
    [SID_SealSpeed] = {
        .name = MSG_SKILL_SealSpeed_NAME,
        .desc = MSG_SKILL_SealSpeed_DESC,
        .capacity = MSG_SKILL_SealSpeed_CAPACITY,
        .price = MSG_SKILL_SealSpeed_PRICE,
        .icon = GFX_SkillIcon_SealSpeed
    },
#endif

#if (defined(SID_SealStrength) && COMMON_SKILL_VALID(SID_SealStrength))
    [SID_SealStrength] = {
        .name = MSG_SKILL_SealStrength_NAME,
        .desc = MSG_SKILL_SealStrength_DESC,
        .capacity = MSG_SKILL_SealStrength_CAPACITY,
        .price = MSG_SKILL_SealStrength_PRICE,
        .icon = GFX_SkillIcon_SealStrength
    },
#endif

#if (defined(SID_RallyDefense) && COMMON_SKILL_VALID(SID_RallyDefense))
    [SID_RallyDefense] = {
        .name = MSG_SKILL_RallyDefense_NAME,
        .desc = MSG_SKILL_RallyDefense_DESC,
        .capacity = MSG_SKILL_RallyDefense_CAPACITY,
        .price = MSG_SKILL_RallyDefense_PRICE,
        .icon = GFX_SkillIcon_RallyDefense,
    },
#endif

#if (defined(SID_RallyLuck) && COMMON_SKILL_VALID(SID_RallyLuck))
    [SID_RallyLuck] = {
        .name = MSG_SKILL_RallyLuck_NAME,
        .desc = MSG_SKILL_RallyLuck_DESC,
        .capacity = MSG_SKILL_RallyLuck_CAPACITY,
        .price = MSG_SKILL_RallyLuck_PRICE,
        .icon = GFX_SkillIcon_RallyLuck,
    },
#endif

#if (defined(SID_RallyMagic) && COMMON_SKILL_VALID(SID_RallyMagic))
    [SID_RallyMagic] = {
        .name = MSG_SKILL_RallyMagic_NAME,
        .desc = MSG_SKILL_RallyMagic_DESC,
        .capacity = MSG_SKILL_RallyMagic_CAPACITY,
        .price = MSG_SKILL_RallyMagic_PRICE,
        .icon = GFX_SkillIcon_RallyMagic,
    },
#endif

#if (defined(SID_RallyMovement) && COMMON_SKILL_VALID(SID_RallyMovement))
    [SID_RallyMovement] = {
        .name = MSG_SKILL_RallyMovement_NAME,
        .desc = MSG_SKILL_RallyMovement_DESC,
        .capacity = MSG_SKILL_RallyMovement_CAPACITY,
        .price = MSG_SKILL_RallyMovement_PRICE,
        .icon = GFX_SkillIcon_RallyMovement,
    },
#endif

#if (defined(SID_RallyResistance) && COMMON_SKILL_VALID(SID_RallyResistance))
    [SID_RallyResistance] = {
        .name = MSG_SKILL_RallyResistance_NAME,
        .desc = MSG_SKILL_RallyResistance_DESC,
        .capacity = MSG_SKILL_RallyResistance_CAPACITY,
        .price = MSG_SKILL_RallyResistance_PRICE,
        .icon = GFX_SkillIcon_RallyResistance,
    },
#endif

#if (defined(SID_RallySkill) && COMMON_SKILL_VALID(SID_RallySkill))
    [SID_RallySkill] = {
        .name = MSG_SKILL_RallySkill_NAME,
        .desc = MSG_SKILL_RallySkill_DESC,
        .capacity = MSG_SKILL_RallySkill_CAPACITY,
        .price = MSG_SKILL_RallySkill_PRICE,
        .icon = GFX_SkillIcon_RallySkill,
    },
#endif

#if (defined(SID_RallySpeed) && COMMON_SKILL_VALID(SID_RallySpeed))
    [SID_RallySpeed] = {
        .name = MSG_SKILL_RallySpeed_NAME,
        .desc = MSG_SKILL_RallySpeed_DESC,
        .capacity = MSG_SKILL_RallySpeed_CAPACITY,
        .price = MSG_SKILL_RallySpeed_PRICE,
        .icon = GFX_SkillIcon_RallySpeed,
    },
#endif

#if (defined(SID_RallyStrength) && COMMON_SKILL_VALID(SID_RallyStrength))
    [SID_RallyStrength] = {
        .name = MSG_SKILL_RallyStrength_NAME,
        .desc = MSG_SKILL_RallyStrength_DESC,
        .capacity = MSG_SKILL_RallyStrength_CAPACITY,
        .price = MSG_SKILL_RallyStrength_PRICE,
        .icon = GFX_SkillIcon_RallyStrength,
    },
#endif

#if (defined(SID_RallySpectrum) && COMMON_SKILL_VALID(SID_RallySpectrum))
    [SID_RallySpectrum] = {
        .name = MSG_SKILL_RallySpectrum_NAME,
        .desc = MSG_SKILL_RallySpectrum_DESC,
        .capacity = MSG_SKILL_RallySpectrum_CAPACITY,
        .price = MSG_SKILL_RallySpectrum_PRICE,
        .icon = GFX_SkillIcon_RallySpectrum,
    },
#endif

#if (defined(SID_Despoil) && COMMON_SKILL_VALID(SID_Despoil))
    [SID_Despoil] = {
        .name = MSG_SKILL_Despoil_NAME,
        .desc = MSG_SKILL_Despoil_DESC,
        .capacity = MSG_SKILL_Despoil_CAPACITY,
        .price = MSG_SKILL_Despoil_PRICE,
        .icon = GFX_SkillIcon_Despoil,
    },
#endif

#if (defined(SID_Pickup) && COMMON_SKILL_VALID(SID_Pickup))
    [SID_Pickup] = {
        .name = MSG_SKILL_Pickup_NAME,
        .desc = MSG_SKILL_Pickup_DESC,
        .capacity = MSG_SKILL_Pickup_CAPACITY,
        .price = MSG_SKILL_Pickup_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_IndoorMarch) && COMMON_SKILL_VALID(SID_IndoorMarch))
    [SID_IndoorMarch] = {
        .name = MSG_SKILL_IndoorMarch_NAME,
        .desc = MSG_SKILL_IndoorMarch_DESC,
        .capacity = MSG_SKILL_IndoorMarch_CAPACITY,
        .price = MSG_SKILL_IndoorMarch_PRICE,
        .icon = GFX_SkillIcon_IndoorMarch,
    },
#endif

#if (defined(SID_DevilsPact) && COMMON_SKILL_VALID(SID_DevilsPact))
    [SID_DevilsPact] = {
        .name = MSG_SKILL_DevilsPact_NAME,
        .desc = MSG_SKILL_DevilsPact_DESC,
        .capacity = MSG_SKILL_DevilsPact_CAPACITY,
        .price = MSG_SKILL_DevilsPact_PRICE,
        .icon = GFX_SkillIcon_DevilsPact,
    },
#endif

#if (defined(SID_DevilsWhim) && COMMON_SKILL_VALID(SID_DevilsWhim))
    [SID_DevilsWhim] = {
        .name = MSG_SKILL_DevilsWhim_NAME,
        .desc = MSG_SKILL_DevilsWhim_DESC,
        .capacity = MSG_SKILL_DevilsWhim_CAPACITY,
        .price = MSG_SKILL_DevilsWhim_PRICE,
        .icon = GFX_SkillIcon_DevilsWhim,
    },
#endif

#if (defined(SID_DevilsLuck) && COMMON_SKILL_VALID(SID_DevilsLuck))
    [SID_DevilsLuck] = {
        .name = MSG_SKILL_DevilsLuck_NAME,
        .desc = MSG_SKILL_DevilsLuck_DESC,
        .capacity = MSG_SKILL_DevilsLuck_CAPACITY,
        .price = MSG_SKILL_DevilsLuck_PRICE,
        .icon = GFX_SkillIcon_DevilsLuck,
    },
#endif

#if (defined(SID_Expertise) && COMMON_SKILL_VALID(SID_Expertise))
    [SID_Expertise] = {
        .name = MSG_SKILL_Expertise_NAME,
        .desc = MSG_SKILL_Expertise_DESC,
        .capacity = MSG_SKILL_Expertise_CAPACITY,
        .price = MSG_SKILL_Expertise_PRICE,
        .icon = GFX_SkillIcon_Expertise,
    },
#endif

#if (defined(SID_PassionsFlow) && COMMON_SKILL_VALID(SID_PassionsFlow))
    [SID_PassionsFlow] = {
        .name = MSG_SKILL_PassionsFlow_NAME,
         .desc = MSG_SKILL_PassionsFlow_DESC,
         .capacity = MSG_SKILL_PassionsFlow_CAPACITY,
        .price = MSG_SKILL_PassionsFlow_PRICE,
        .icon = GFX_SkillIcon_PassionsFlow,
    },
#endif

#if (defined(SID_NatureRush) && COMMON_SKILL_VALID(SID_NatureRush))
    [SID_NatureRush] = {
        .name = MSG_SKILL_NatureRush_NAME,
        .desc = MSG_SKILL_NatureRush_DESC,
        .capacity = MSG_SKILL_NatureRush_CAPACITY,
        .price = MSG_SKILL_NatureRush_PRICE,
        .icon = GFX_SkillIcon_NatureRush,
    },
#endif

#if (defined(SID_CounterMagic) && COMMON_SKILL_VALID(SID_CounterMagic))
    [SID_CounterMagic] = {
        .name = MSG_SKILL_CounterMagic_NAME,
        .desc = MSG_SKILL_CounterMagic_DESC,
        .capacity = MSG_SKILL_CounterMagic_CAPACITY,
        .price = MSG_SKILL_CounterMagic_PRICE,
        .icon = GFX_SkillIcon_CounterMagic,
    },
#endif

#if (defined(SID_LadyBlade) && COMMON_SKILL_VALID(SID_LadyBlade))
    [SID_LadyBlade] = {
        .name = MSG_SKILL_LadyBlade_NAME,
        .desc = MSG_SKILL_LadyBlade_DESC,
        .capacity = MSG_SKILL_LadyBlade_CAPACITY,
        .price = MSG_SKILL_LadyBlade_PRICE,
        .icon = GFX_SkillIcon_LadyBlade,
    },
#endif

#if (defined(SID_LullDefense) && COMMON_SKILL_VALID(SID_LullDefense))
    [SID_LullDefense] = {
        .name = MSG_SKILL_LullDefense_NAME,
        .desc = MSG_SKILL_LullDefense_DESC,
        .capacity = MSG_SKILL_LullDefense_CAPACITY,
        .price = MSG_SKILL_LullDefense_PRICE,
        .icon = GFX_SkillIcon_LullDefense,
    },
#endif

#if (defined(SID_LullLuck) && COMMON_SKILL_VALID(SID_LullLuck))
    [SID_LullLuck] = {
        .name = MSG_SKILL_LullLuck_NAME,
        .desc = MSG_SKILL_LullLuck_DESC,
        .capacity = MSG_SKILL_LullLuck_CAPACITY,
        .price = MSG_SKILL_LullLuck_PRICE,
        .icon = GFX_SkillIcon_LullLuck,
    },
#endif

#if (defined(SID_LullMagic) && COMMON_SKILL_VALID(SID_LullMagic))
    [SID_LullMagic] = {
        .name = MSG_SKILL_LullMagic_NAME,
        .desc = MSG_SKILL_LullMagic_DESC,
        .capacity = MSG_SKILL_LullMagic_CAPACITY,
        .price = MSG_SKILL_LullMagic_PRICE,
        .icon = GFX_SkillIcon_LullMagic,
    },
#endif

#if (defined(SID_LullMovement) && COMMON_SKILL_VALID(SID_LullMovement))
    [SID_LullMovement] = {
        .name = MSG_SKILL_LullMovement_NAME,
        .desc = MSG_SKILL_LullMovement_DESC,
        .capacity = MSG_SKILL_LullMovement_CAPACITY,
        .price = MSG_SKILL_LullMovement_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LullResistance) && COMMON_SKILL_VALID(SID_LullResistance))
    [SID_LullResistance] = {
        .name = MSG_SKILL_LullResistance_NAME,
        .desc = MSG_SKILL_LullResistance_DESC,
        .capacity = MSG_SKILL_LullResistance_CAPACITY,
        .price = MSG_SKILL_LullResistance_PRICE,
        .icon = GFX_SkillIcon_LullResistance,
    },
#endif

#if (defined(SID_LullSkill) && COMMON_SKILL_VALID(SID_LullSkill))
    [SID_LullSkill] = {
        .name = MSG_SKILL_LullSkill_NAME,
        .desc = MSG_SKILL_LullSkill_DESC,
        .capacity = MSG_SKILL_LullSkill_CAPACITY,
        .price = MSG_SKILL_LullSkill_PRICE,
        .icon = GFX_SkillIcon_LullSkill,
    },
#endif

#if (defined(SID_LullSpectrum) && COMMON_SKILL_VALID(SID_LullSpectrum))
    [SID_LullSpectrum] = {
        .name = MSG_SKILL_LullSpectrum_NAME,
        .desc = MSG_SKILL_LullSpectrum_DESC,
        .capacity = MSG_SKILL_LullSpectrum_CAPACITY,
        .price = MSG_SKILL_LullSpectrum_PRICE,
        .icon = GFX_SkillIcon_LullSpectrum,
    },
#endif

#if (defined(SID_LullSpeed) && COMMON_SKILL_VALID(SID_LullSpeed))
    [SID_LullSpeed] = {
        .name = MSG_SKILL_LullSpeed_NAME,
        .desc = MSG_SKILL_LullSpeed_DESC,
        .capacity = MSG_SKILL_LullSpeed_CAPACITY,
        .price = MSG_SKILL_LullSpeed_PRICE,
        .icon = GFX_SkillIcon_LullSpeed,
    },
#endif

#if (defined(SID_LullStrength) && COMMON_SKILL_VALID(SID_LullStrength))
    [SID_LullStrength] = {
        .name = MSG_SKILL_LullStrength_NAME,
        .desc = MSG_SKILL_LullStrength_DESC,
        .capacity = MSG_SKILL_LullStrength_CAPACITY,
        .price = MSG_SKILL_LullStrength_PRICE,
        .icon = GFX_SkillIcon_LullStrength,
    },
#endif

#if (defined(SID_ShrewdPotential) && COMMON_SKILL_VALID(SID_ShrewdPotential))
    [SID_ShrewdPotential] = {
        .name = MSG_SKILL_ShrewdPotential_NAME,
        .desc = MSG_SKILL_ShrewdPotential_DESC,
        .capacity = MSG_SKILL_ShrewdPotential_CAPACITY,
        .price = MSG_SKILL_ShrewdPotential_PRICE,
        .icon = GFX_SkillIcon_ShrewdPotential,
    },
#endif

#if (defined(SID_GoddessDance) && COMMON_SKILL_VALID(SID_GoddessDance))
    [SID_GoddessDance] = {
        .name = MSG_SKILL_GoddessDance_NAME,
        .desc = MSG_SKILL_GoddessDance_DESC,
        .capacity = MSG_SKILL_GoddessDance_CAPACITY,
        .price = MSG_SKILL_GoddessDance_PRICE,
        .icon = GFX_SkillIcon_GoddessDance,
    },
#endif

#if (defined(SID_SteadyBrawler) && COMMON_SKILL_VALID(SID_SteadyBrawler))
    [SID_SteadyBrawler] = {
        .name = MSG_SKILL_SteadyBrawler_NAME,
        .desc = MSG_SKILL_SteadyBrawler_DESC,
        .capacity = MSG_SKILL_SteadyBrawler_CAPACITY,
        .price = MSG_SKILL_SteadyBrawler_PRICE,
        .icon = GFX_SkillIcon_SteadyBrawler,
    },
#endif

#if (defined(SID_KeepUp) && COMMON_SKILL_VALID(SID_KeepUp))
    [SID_KeepUp] = {
        .name = MSG_SKILL_KeepUp_NAME,
        .desc = MSG_SKILL_KeepUp_DESC,
        .capacity = MSG_SKILL_KeepUp_CAPACITY,
        .price = MSG_SKILL_KeepUp_PRICE,
        .icon = GFX_SkillIcon_KeepUp,
    },
#endif

#if (defined(SID_LiquidOoze) && COMMON_SKILL_VALID(SID_LiquidOoze))
    [SID_LiquidOoze] = {
        .name = MSG_SKILL_LiquidOoze_NAME,
        .desc = MSG_SKILL_LiquidOoze_DESC,
        .capacity = MSG_SKILL_LiquidOoze_CAPACITY,
        .price = MSG_SKILL_LiquidOoze_PRICE,
        .icon = GFX_SkillIcon_LiquidOoze,
    },
#endif

#if (defined(SID_CloseCombat) && COMMON_SKILL_VALID(SID_CloseCombat))
    [SID_CloseCombat] = {
        .name = MSG_SKILL_CloseCombat_NAME,
        .desc = MSG_SKILL_CloseCombat_DESC,
        .capacity = MSG_SKILL_CloseCombat_CAPACITY,
        .price = MSG_SKILL_CloseCombat_PRICE,
        .icon = GFX_SkillIcon_CloseCombat,
    },
#endif

#if (defined(SID_ArmorMarch) && COMMON_SKILL_VALID(SID_ArmorMarch))
    [SID_ArmorMarch] = {
        .name = MSG_SKILL_ArmorMarch_NAME,
        .desc = MSG_SKILL_ArmorMarch_DESC,
        .capacity = MSG_SKILL_ArmorMarch_CAPACITY,
        .price = MSG_SKILL_ArmorMarch_PRICE,
        .icon = GFX_SkillIcon_ArmorMarch,
    },
#endif

#if (defined(SID_Eclipse) && COMMON_SKILL_VALID(SID_Eclipse))
    [SID_Eclipse] = {
        .name = MSG_SKILL_Eclipse_NAME,
        .desc = MSG_SKILL_Eclipse_DESC,
        .capacity = MSG_SKILL_Eclipse_CAPACITY,
        .price = MSG_SKILL_Eclipse_PRICE,
        .icon = GFX_SkillIcon_Eclipse,
    },
#endif

#if (defined(SID_Barricade) && COMMON_SKILL_VALID(SID_Barricade))
    [SID_Barricade] = {
        .name = MSG_SKILL_Barricade_NAME,
        .desc = MSG_SKILL_Barricade_DESC,
        .capacity = MSG_SKILL_Barricade_CAPACITY,
        .price = MSG_SKILL_Barricade_PRICE,
        .icon = GFX_SkillIcon_Barricade,
    },
#endif

#if (defined(SID_BarricadePlus) && COMMON_SKILL_VALID(SID_BarricadePlus))
    [SID_BarricadePlus] = {
        .name = MSG_SKILL_BarricadePlus_NAME,
        .desc = MSG_SKILL_BarricadePlus_DESC,
        .capacity = MSG_SKILL_BarricadePlus_CAPACITY,
        .price = MSG_SKILL_BarricadePlus_PRICE,
        .icon = GFX_SkillIcon_BarricadePlus,
    },
#endif

#if (defined(SID_LunarBrace) && COMMON_SKILL_VALID(SID_LunarBrace))
    [SID_LunarBrace] = {
        .name = MSG_SKILL_LunarBrace_NAME,
        .desc = MSG_SKILL_LunarBrace_DESC,
        .capacity = MSG_SKILL_LunarBrace_CAPACITY,
        .price = MSG_SKILL_LunarBrace_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SoulSap) && COMMON_SKILL_VALID(SID_SoulSap))
    [SID_SoulSap] = {
        .name = MSG_SKILL_SoulSap_NAME,
        .desc = MSG_SKILL_SoulSap_DESC,
        .capacity = MSG_SKILL_SoulSap_CAPACITY,
        .price = MSG_SKILL_SoulSap_PRICE,
        .icon = GFX_SkillIcon_SoulSap,
    },
#endif

#if (defined(SID_Pursuer) && COMMON_SKILL_VALID(SID_Pursuer))
    [SID_Pursuer] = {
        .name = MSG_SKILL_Pursuer_NAME,
        .desc = MSG_SKILL_Pursuer_DESC,
        .capacity = MSG_SKILL_Pursuer_CAPACITY,
        .price = MSG_SKILL_Pursuer_PRICE,
        .icon = GFX_SkillIcon_Pursuer,
    },
#endif

#if (defined(SID_Stride) && COMMON_SKILL_VALID(SID_Stride))
    [SID_Stride] = {
        .name = MSG_SKILL_Stride_NAME,
        .desc = MSG_SKILL_Stride_DESC,
        .capacity = MSG_SKILL_Stride_CAPACITY,
        .price = MSG_SKILL_Stride_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_QuickLearner) && COMMON_SKILL_VALID(SID_QuickLearner))
    [SID_QuickLearner] = {
        .name = MSG_SKILL_QuickLearner_NAME,
         .desc = MSG_SKILL_QuickLearner_DESC,
         .capacity = MSG_SKILL_QuickLearner_CAPACITY,
        .price = MSG_SKILL_QuickLearner_PRICE,
        .icon = GFX_SkillIcon_QuickLearner,
    },
#endif

#if (defined(SID_GoldDigger) && COMMON_SKILL_VALID(SID_GoldDigger))
    [SID_GoldDigger] = {
        .name = MSG_SKILL_GoldDigger_NAME,
        .desc = MSG_SKILL_GoldDigger_DESC,
        .capacity = MSG_SKILL_GoldDigger_CAPACITY,
        .price = MSG_SKILL_GoldDigger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EffectSpore) && COMMON_SKILL_VALID(SID_EffectSpore))
    [SID_EffectSpore] = {
        .name = MSG_SKILL_EffectSpore_NAME,
        .desc = MSG_SKILL_EffectSpore_DESC,
        .capacity = MSG_SKILL_EffectSpore_CAPACITY,
        .price = MSG_SKILL_EffectSpore_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DominantArm) && COMMON_SKILL_VALID(SID_DominantArm))
    [SID_DominantArm] = {
        .name = MSG_SKILL_DominantArm_NAME,
        .desc = MSG_SKILL_DominantArm_DESC,
        .capacity = MSG_SKILL_DominantArm_CAPACITY,
        .price = MSG_SKILL_DominantArm_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GaleWings) && COMMON_SKILL_VALID(SID_GaleWings))
    [SID_GaleWings] = {
        .name = MSG_SKILL_GaleWings_NAME,
        .desc = MSG_SKILL_GaleWings_DESC,
        .capacity = MSG_SKILL_GaleWings_CAPACITY,
        .price = MSG_SKILL_GaleWings_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SolidRock) && COMMON_SKILL_VALID(SID_SolidRock))
    [SID_SolidRock] = {
        .name = MSG_SKILL_SolidRock_NAME,
        .desc = MSG_SKILL_SolidRock_DESC,
        .capacity = MSG_SKILL_SolidRock_CAPACITY,
        .price = MSG_SKILL_SolidRock_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Sturdy) && COMMON_SKILL_VALID(SID_Sturdy))
    [SID_Sturdy] = {
        .name = MSG_SKILL_Sturdy_NAME,
        .desc = MSG_SKILL_Sturdy_DESC,
        .capacity = MSG_SKILL_Sturdy_CAPACITY,
        .price = MSG_SKILL_Sturdy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HoldOut) && COMMON_SKILL_VALID(SID_HoldOut))
    [SID_HoldOut] = {
        .name = MSG_SKILL_HoldOut_NAME,
        .desc = MSG_SKILL_HoldOut_DESC,
        .capacity = MSG_SKILL_HoldOut_CAPACITY,
        .price = MSG_SKILL_HoldOut_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FranticSwing) && COMMON_SKILL_VALID(SID_FranticSwing))
    [SID_FranticSwing] = {
        .name = MSG_SKILL_FranticSwing_NAME,
        .desc = MSG_SKILL_FranticSwing_DESC,
        .capacity = MSG_SKILL_FranticSwing_CAPACITY,
        .price = MSG_SKILL_FranticSwing_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SocialButterfly) && COMMON_SKILL_VALID(SID_SocialButterfly))
    [SID_SocialButterfly] = {
        .name = MSG_SKILL_SocialButterfly_NAME,
        .desc = MSG_SKILL_SocialButterfly_DESC,
        .capacity = MSG_SKILL_SocialButterfly_CAPACITY,
        .price = MSG_SKILL_SocialButterfly_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MeleeManiac) && COMMON_SKILL_VALID(SID_MeleeManiac))
    [SID_MeleeManiac] = {
        .name = MSG_SKILL_MeleeManiac_NAME,
        .desc = MSG_SKILL_MeleeManiac_DESC,
        .capacity = MSG_SKILL_MeleeManiac_CAPACITY,
        .price = MSG_SKILL_MeleeManiac_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CriticalOverload) && COMMON_SKILL_VALID(SID_CriticalOverload))
    [SID_CriticalOverload] = {
        .name = MSG_SKILL_CriticalOverload_NAME,
        .desc = MSG_SKILL_CriticalOverload_DESC,
        .capacity = MSG_SKILL_CriticalOverload_CAPACITY,
        .price = MSG_SKILL_CriticalOverload_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Swarp) && COMMON_SKILL_VALID(SID_Swarp))
    [SID_Swarp] = {
        .name = MSG_SKILL_Swarp_NAME,
        .desc = MSG_SKILL_Swarp_DESC,
        .capacity = MSG_SKILL_Swarp_CAPACITY,
        .price = MSG_SKILL_Swap_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Blacksmith) && COMMON_SKILL_VALID(SID_Blacksmith))
    [SID_Blacksmith] = {
        .name = MSG_SKILL_Blacksmith_NAME,
        .desc = MSG_SKILL_Blacksmith_DESC,
        .capacity = MSG_SKILL_Blacksmith_CAPACITY,
        .price = MSG_SKILL_Blacksmith_PRICE,
        .icon = GFX_SkillIcon_Blacksmith,
    },
#endif

#if (defined(SID_Deal) && COMMON_SKILL_VALID(SID_Deal))
    [SID_Deal] = {
        .name = MSG_SKILL_Deal_NAME,
        .desc = MSG_SKILL_Deal_DESC,
        .capacity = MSG_SKILL_Deal_CAPACITY,
        .price = MSG_SKILL_Deal_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ConcealedWeapon) && COMMON_SKILL_VALID(SID_ConcealedWeapon))
    [SID_ConcealedWeapon] = {
        .name = MSG_SKILL_ConcealedWeapon_NAME,
        .desc = MSG_SKILL_ConcealedWeapon_DESC,
        .capacity = MSG_SKILL_ConcealedWeapon_CAPACITY,
        .price = MSG_SKILL_ConcealedWeapon_PRICE,
        .icon = GFX_SkillIcon_ConcealedWeapon,
    },
#endif

#if (defined(SID_Bargain) && COMMON_SKILL_VALID(SID_Bargain))
    [SID_Bargain] = {
        .name = MSG_SKILL_Bargain_NAME,
        .desc = MSG_SKILL_Bargain_DESC,
        .capacity = MSG_SKILL_Bargain_CAPACITY,
        .price = MSG_SKILL_Bargain_PRICE,
        .icon = GFX_SkillIcon_Bargain,
    },
#endif

#if (defined(SID_WyvernFlight) && COMMON_SKILL_VALID(SID_WyvernFlight))
    [SID_WyvernFlight] = {
        .name = MSG_SKILL_WyvernFlight_NAME,
        .desc = MSG_SKILL_WyvernFlight_DESC,
        .capacity = MSG_SKILL_WyvernFlight_CAPACITY,
        .price = MSG_SKILL_WyvernFlight_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_YngviAscendant) && COMMON_SKILL_VALID(SID_YngviAscendant))
    [SID_YngviAscendant] = {
        .name = MSG_SKILL_YngviAscendant_NAME,
        .desc = MSG_SKILL_YngviAscendant_DESC,
        .capacity = MSG_SKILL_YngviAscendant_CAPACITY,
        .price = MSG_SKILL_YngviAscendant_PRICE,
        .icon = GFX_SkillIcon_YngviAscendant,
    },
#endif

#if (defined(SID_ChillPow) && COMMON_SKILL_VALID(SID_ChillPow))
    [SID_ChillPow] = {
        .name = MSG_SKILL_ChillPow_NAME,
        .desc = MSG_SKILL_ChillPow_DESC,
        .capacity = MSG_SKILL_ChillPow_CAPACITY,
        .price = MSG_SKILL_ChillPow_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChillMag) && COMMON_SKILL_VALID(SID_ChillMag))
    [SID_ChillMag] = {
        .name = MSG_SKILL_ChillMag_NAME,
        .desc = MSG_SKILL_ChillMag_DESC,
        .capacity = MSG_SKILL_ChillMag_CAPACITY,
        .price = MSG_SKILL_ChillMag_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChillSkl) && COMMON_SKILL_VALID(SID_ChillSkl))
    [SID_ChillSkl] = {
        .name = MSG_SKILL_ChillSkl_NAME,
        .desc = MSG_SKILL_ChillSkl_DESC,
        .capacity = MSG_SKILL_ChillSkl_CAPACITY,
        .price = MSG_SKILL_ChillSkl_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChillSpd) && COMMON_SKILL_VALID(SID_ChillSpd))
    [SID_ChillSpd] = {
        .name = MSG_SKILL_ChillSpd_NAME,
        .desc = MSG_SKILL_ChillSpd_DESC,
        .capacity = MSG_SKILL_ChillSpd_CAPACITY,
        .price = MSG_SKILL_ChillSpd_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChillLck) && COMMON_SKILL_VALID(SID_ChillLck))
    [SID_ChillLck] = {
        .name = MSG_SKILL_ChillLck_NAME,
        .desc = MSG_SKILL_ChillLck_DESC,
        .capacity = MSG_SKILL_ChillLck_CAPACITY,
        .price = MSG_SKILL_ChillLck_PRICE,
        .icon = GFX_SkillIcon_WIP
    },
#endif

#if (defined(SID_ChillDef) && COMMON_SKILL_VALID(SID_ChillDef))
    [SID_ChillDef] = {
        .name = MSG_SKILL_ChillDef_NAME,
        .desc = MSG_SKILL_ChillDef_DESC,
        .capacity = MSG_SKILL_ChillDef_CAPACITY,
        .price = MSG_SKILL_ChillDef_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChillRes) && COMMON_SKILL_VALID(SID_ChillRes))
    [SID_ChillRes] = {
        .name = MSG_SKILL_ChillRes_NAME,
        .desc = MSG_SKILL_ChillRes_DESC,
        .capacity = MSG_SKILL_ChillRes_CAPACITY,
        .price = MSG_SKILL_ChillRes_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DragonWrath) && COMMON_SKILL_VALID(SID_DragonWrath))
    [SID_DragonWrath] = {
        .name = MSG_SKILL_DragonWrath_NAME,
        .desc = MSG_SKILL_DragonWrath_DESC,
        .capacity = MSG_SKILL_DragonWrath_CAPACITY,
        .price = MSG_SKILL_DragonWrath_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BrashAssault) && COMMON_SKILL_VALID(SID_BrashAssault))
    [SID_BrashAssault] = {
        .name = MSG_SKILL_BrashAssault_NAME,
        .desc = MSG_SKILL_BrashAssault_DESC,
        .capacity = MSG_SKILL_BrashAssault_CAPACITY,
        .price = MSG_SKILL_BrashAssault_PRICE,
        .icon = GFX_SkillIcon_BrashAssault,
    },
#endif

#if (defined(SID_Scendscale) && COMMON_SKILL_VALID(SID_Scendscale))
    [SID_Scendscale] = {
        .name = MSG_SKILL_Scendscale_NAME,
        .desc = MSG_SKILL_Scendscale_DESC,
        .capacity = MSG_SKILL_Scendscale_CAPACITY,
        .price = MSG_SKILL_Scendscale_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LawsOfSacae) && COMMON_SKILL_VALID(SID_LawsOfSacae))
    [SID_LawsOfSacae] = {
        .name = MSG_SKILL_LawsOfSacae_NAME,
        .desc = MSG_SKILL_LawsOfSacae_DESC,
        .capacity = MSG_SKILL_LawsOfSacae_CAPACITY,
        .price = MSG_SKILL_LawsOfSacae_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BonusDoubler) && COMMON_SKILL_VALID(SID_BonusDoubler))
    [SID_BonusDoubler] = {
        .name = MSG_SKILL_BonusDoubler_NAME,
        .desc = MSG_SKILL_BonusDoubler_DESC,
        .capacity = MSG_SKILL_BonusDoubler_CAPACITY,
        .price = MSG_SKILL_BonusDoubler_PRICE,
        .icon = GFX_SkillIcon_BonusDoubler,
    },
#endif

#if (defined(SID_ShieldPulse) && COMMON_SKILL_VALID(SID_ShieldPulse))
    [SID_ShieldPulse] = {
        .name = MSG_SKILL_ShieldPulse_NAME,
        .desc = MSG_SKILL_ShieldPulse_DESC,
        .capacity = MSG_SKILL_ShieldPulse_CAPACITY,
        .price = MSG_SKILL_ShieldPulse_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BestDefense) && COMMON_SKILL_VALID(SID_BestDefense))
    [SID_BestDefense] = {
        .name = MSG_SKILL_BestDefense_NAME,
        .desc = MSG_SKILL_BestDefense_DESC,
        .capacity = MSG_SKILL_BestDefense_CAPACITY,
        .price = MSG_SKILL_BestDefense_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DoOrDie) && COMMON_SKILL_VALID(SID_DoOrDie))
    [SID_DoOrDie] = {
        .name = MSG_SKILL_DoOrDie_NAME,
        .desc = MSG_SKILL_DoOrDie_DESC,
        .capacity = MSG_SKILL_DoOrDie_CAPACITY,
        .price = MSG_SKILL_DoOrDie_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Glorifier) && COMMON_SKILL_VALID(SID_Glorifier))
    [SID_Glorifier] = {
        .name = MSG_SKILL_Glorifier_NAME,
        .desc = MSG_SKILL_Glorifier_DESC,
        .capacity = MSG_SKILL_Glorifier_CAPACITY,
        .price = MSG_SKILL_Glorifier_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Leader) && COMMON_SKILL_VALID(SID_Leader))
    [SID_Leader] = {
        .name = MSG_SKILL_Leader_NAME,
        .desc = MSG_SKILL_Leader_DESC,
        .capacity = MSG_SKILL_Leader_CAPACITY,
        .price = MSG_SKILL_Leader_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_OverKill) && COMMON_SKILL_VALID(SID_OverKill))
    [SID_OverKill] = {
        .name = MSG_SKILL_OverKill_NAME,
        .desc = MSG_SKILL_OverKill_DESC,
        .capacity = MSG_SKILL_OverKill_CAPACITY,
        .price = MSG_SKILL_OverKill_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Momentum) && COMMON_SKILL_VALID(SID_Momentum))
    [SID_Momentum] = {
        .name = MSG_SKILL_Momentum_NAME,
        .desc = MSG_SKILL_Momentum_DESC,
        .capacity = MSG_SKILL_Momentum_CAPACITY,
        .price = MSG_SKILL_Momentum_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FatalTen) && COMMON_SKILL_VALID(SID_FatalTen))
    [SID_FatalTen] = {
        .name = MSG_SKILL_FatalTen_NAME,
        .desc = MSG_SKILL_FatalTen_DESC,
        .capacity = MSG_SKILL_FatalTen_CAPACITY,
        .price = MSG_SKILL_FatalTen_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LionFlame) && COMMON_SKILL_VALID(SID_LionFlame))
    [SID_LionFlame] = {
        .name = MSG_SKILL_LionFlame_NAME,
        .desc = MSG_SKILL_LionFlame_DESC,
        .capacity = MSG_SKILL_LionFlame_CAPACITY,
        .price = MSG_SKILL_LionFlame_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LimitBreak) && COMMON_SKILL_VALID(SID_LimitBreak))
    [SID_LimitBreak] = {
        .name = MSG_SKILL_LimitBreak_NAME,
        .desc = MSG_SKILL_LimitBreak_DESC,
        .capacity = MSG_SKILL_LimitBreak_CAPACITY,
        .price = MSG_SKILL_LimitBreak_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ImmovableObject) && COMMON_SKILL_VALID(SID_ImmovableObject))
    [SID_ImmovableObject] = {
        .name = MSG_SKILL_ImmovableObject_NAME,
        .desc = MSG_SKILL_ImmovableObject_DESC,
        .capacity = MSG_SKILL_ImmovableObject_CAPACITY,
        .price = MSG_SKILL_ImmovableObject_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_UnstoppableForce) && COMMON_SKILL_VALID(SID_UnstoppableForce))
    [SID_UnstoppableForce] = {
        .name = MSG_SKILL_UnstoppableForce_NAME,
        .desc = MSG_SKILL_UnstoppableForce_DESC,
        .capacity = MSG_SKILL_UnstoppableForce_CAPACITY,
        .price = MSG_SKILL_UnstoppableForce_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_WeaponHeal) && COMMON_SKILL_VALID(SID_WeaponHeal))
    [SID_WeaponHeal] = {
        .name = MSG_SKILL_WeaponHeal_NAME,
        .desc = MSG_SKILL_WeaponHeal_DESC,
        .capacity = MSG_SKILL_WeaponHeal_CAPACITY,
        .price = MSG_SKILL_WeaponHeal_PRICE,
        .icon = GFX_SkillIcon_WeaponHeal,
    },
#endif

#if (defined(SID_BloodSurge) && COMMON_SKILL_VALID(SID_BloodSurge))
    [SID_BloodSurge] = {
        .name = MSG_SKILL_BloodSurge_NAME,
        .desc = MSG_SKILL_BloodSurge_DESC,
        .capacity = MSG_SKILL_BloodSurge_CAPACITY,
        .price = MSG_SKILL_BloodSurge_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BloodReaver) && COMMON_SKILL_VALID(SID_BloodReaver))
    [SID_BloodReaver] = {
        .name = MSG_SKILL_BloodReaver_NAME,
        .desc = MSG_SKILL_BloodReaver_DESC,
        .capacity = MSG_SKILL_BloodReaver_CAPACITY,
        .price = MSG_SKILL_BloodReaver_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Latona) && COMMON_SKILL_VALID(SID_Latona))
    [SID_Latona] = {
        .name = MSG_SKILL_Latona_NAME,
        .desc = MSG_SKILL_Latona_DESC,
        .capacity = MSG_SKILL_Latona_CAPACITY,
        .price = MSG_SKILL_Latona_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GrandMagician) && COMMON_SKILL_VALID(SID_GrandMagician))
    [SID_GrandMagician] = {
        .name = MSG_SKILL_GrandMagician_NAME,
        .desc = MSG_SKILL_GrandMagician_DESC,
        .capacity = MSG_SKILL_GrandMagician_CAPACITY,
        .price = MSG_SKILL_GrandMagician_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Witch) && COMMON_SKILL_VALID(SID_Witch))
    [SID_Witch] = {
        .name = MSG_SKILL_Witch_NAME,
        .desc = MSG_SKILL_Witch_DESC,
        .capacity = MSG_SKILL_Witch_CAPACITY,
        .price = MSG_SKILL_Witch_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_COMBAT_BloodTribute) && COMMON_SKILL_VALID(SID_COMBAT_BloodTribute))
    [SID_COMBAT_BloodTribute] = {
        .name = MSG_SKILL_COMBAT_BloodTribute_NAME,
        .desc = MSG_SKILL_COMBAT_BloodTribute_DESC,
        .capacity = MSG_SKILL_COMBAT_BloodTribute_CAPACITY,
        .price = MSG_SKILL_COMBAT_BloodTribute_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_COMBAT_CrimsonStrike) && COMMON_SKILL_VALID(SID_COMBAT_CrimsonStrike))
    [SID_COMBAT_CrimsonStrike] = {
        .name = MSG_SKILL_COMBAT_CrimsonStrike_NAME,
        .desc = MSG_SKILL_COMBAT_CrimsonStrike_DESC,
        .capacity = MSG_SKILL_COMBAT_CrimsonStrike_CAPACITY,
        .price = MSG_SKILL_COMBAT_CrimsonStrike_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_COMBAT_VitalReckoning) && COMMON_SKILL_VALID(SID_COMBAT_VitalReckoning))
    [SID_COMBAT_VitalReckoning] = {
        .name = MSG_SKILL_COMBAT_VitalReckoning_NAME,
        .desc = MSG_SKILL_COMBAT_VitalReckoning_DESC,
        .capacity = MSG_SKILL_COMBAT_VitalReckoning_CAPACITY,
        .price = MSG_SKILL_COMBAT_VitalReckoning_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_WingedShield) && COMMON_SKILL_VALID(SID_WingedShield))
    [SID_WingedShield] = {
        .name = MSG_SKILL_WingedShield_NAME,
        .desc = MSG_SKILL_WingedShield_DESC,
        .capacity = MSG_SKILL_WingedShield_CAPACITY,
        .price = MSG_SKILL_WingedShield_PRICE,
        .icon = GFX_SkillIcon_WingedShield,
    },
#endif

#if (defined(SID_BeastShield) && COMMON_SKILL_VALID(SID_BeastShield))
    [SID_BeastShield] = {
        .name = MSG_SKILL_BeastShield_NAME,
        .desc = MSG_SKILL_BeastShield_DESC,
        .capacity = MSG_SKILL_BeastShield_CAPACITY,
        .price = MSG_SKILL_BeastShield_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DishonorBreaker) && COMMON_SKILL_VALID(SID_DishonorBreaker))
    [SID_DishonorBreaker] = {
        .name = MSG_SKILL_DishonorBreaker_NAME,
        .desc = MSG_SKILL_DishonorBreaker_DESC,
        .capacity = MSG_SKILL_DishonorBreaker_CAPACITY,
        .price = MSG_SKILL_DishonorBreaker_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Stubborn) && COMMON_SKILL_VALID(SID_Stubborn))
    [SID_Stubborn] = {
        .name = MSG_SKILL_Stubborn_NAME,
        .desc = MSG_SKILL_Stubborn_DESC,
        .capacity = MSG_SKILL_Stubborn_CAPACITY,
        .price = MSG_SKILL_Stubborn_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Shakedown) && COMMON_SKILL_VALID(SID_Shakedown))
    [SID_Shakedown] = {
        .name = MSG_SKILL_Shakedown_NAME,
        .desc = MSG_SKILL_Shakedown_DESC,
        .capacity = MSG_SKILL_Shakedown_CAPACITY,
        .price = MSG_SKILL_Shakedown_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FreeSpirit) && COMMON_SKILL_VALID(SID_FreeSpirit))
    [SID_FreeSpirit] = {
        .name = MSG_SKILL_FreeSpirit_NAME,
        .desc = MSG_SKILL_FreeSpirit_DESC,
        .capacity = MSG_SKILL_FreeSpirit_CAPACITY,
        .price = MSG_SKILL_FreeSpirit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ArmsthriftPlus) && COMMON_SKILL_VALID(SID_ArmsthriftPlus))
    [SID_ArmsthriftPlus] = {
        .name = MSG_SKILL_ArmsthriftPlus_NAME,
        .desc = MSG_SKILL_ArmsthriftPlus_DESC,
        .capacity = MSG_SKILL_ArmsthriftPlus_CAPACITY,
        .price = MSG_SKILL_ArmsthriftPlus_PRICE,
        .icon = GFX_SkillIcon_ArmsthriftPlus,
    },
#endif

#if (defined(SID_GridMaster) && COMMON_SKILL_VALID(SID_GridMaster))
    [SID_GridMaster] = {
        .name = MSG_SKILL_GridMaster_NAME,
        .desc = MSG_SKILL_GridMaster_DESC,
        .capacity = MSG_SKILL_GridMaster_CAPACITY,
        .price = MSG_SKILL_GridMaster_PRICE,
        .icon = GFX_SkillIcon_GridMaster,
    },
#endif

#if (defined(SID_GridMasterPlus) && COMMON_SKILL_VALID(SID_GridMasterPlus))
    [SID_GridMasterPlus] = {
        .name = MSG_SKILL_GridMasterPlus_NAME,
        .desc = MSG_SKILL_GridMasterPlus_DESC,
        .capacity = MSG_SKILL_GridMasterPlus_CAPACITY,
        .price = MSG_SKILL_GridMasterPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Swap) && COMMON_SKILL_VALID(SID_Swap))
    [SID_Swap] = {
        .name = MSG_SKILL_Swap_NAME,
        .desc = MSG_SKILL_Swap_DESC,
        .capacity = MSG_SKILL_Swap_CAPACITY,
        .price = MSG_SKILL_Swap_PRICE,
        .icon = GFX_SkillIcon_Swap,
    },
#endif

#if (defined(SID_VigorDance) && COMMON_SKILL_VALID(SID_VigorDance))
    [SID_VigorDance] = {
        .name = MSG_SKILL_VigorDance_NAME,
        .desc = MSG_SKILL_VigorDance_DESC,
        .capacity = MSG_SKILL_VigorDance_CAPACITY,
        .price = MSG_SKILL_VigorDance_PRICE,
        .icon = GFX_SkillIcon_VigorDance,
    },
#endif

#if (defined(SID_MagicSeal) && COMMON_SKILL_VALID(SID_MagicSeal))
    [SID_MagicSeal] = {
        .name = MSG_SKILL_MagicSeal_NAME,
        .desc = MSG_SKILL_MagicSeal_DESC,
        .capacity = MSG_SKILL_MagicSeal_CAPACITY,
        .price = MSG_SKILL_MagicSeal_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Mimic) && COMMON_SKILL_VALID(SID_Mimic))
    [SID_Mimic] = {
        .name = MSG_SKILL_Mimic_NAME,
        .desc = MSG_SKILL_Mimic_DESC,
        .capacity = MSG_SKILL_Mimic_CAPACITY,
        .price = MSG_SKILL_Mimic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Insomnia) && COMMON_SKILL_VALID(SID_Insomnia))
    [SID_Insomnia] = {
        .name = MSG_SKILL_Insomnia_NAME,
        .desc = MSG_SKILL_Insomnia_DESC,
        .capacity = MSG_SKILL_Insomnia_CAPACITY,
        .price = MSG_SKILL_Insomnia_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Comatose) && COMMON_SKILL_VALID(SID_Comatose))
    [SID_Comatose] = {
        .name = MSG_SKILL_Comatose_NAME,
        .desc = MSG_SKILL_Comatose_DESC,
        .capacity = MSG_SKILL_Comatose_CAPACITY,
        .price = MSG_SKILL_Comatose_PRICE,
        .icon = GFX_SkillIcon_Comatose,
    },
#endif

#if (defined(SID_MagicBounce) && COMMON_SKILL_VALID(SID_MagicBounce))
    [SID_MagicBounce] = {
        .name = MSG_SKILL_MagicBounce_NAME,
        .desc = MSG_SKILL_MagicBounce_DESC,
        .capacity = MSG_SKILL_MagicBounce_CAPACITY,
        .price = MSG_SKILL_MagicBounce_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Accost) && COMMON_SKILL_VALID(SID_Accost))
    [SID_Accost] = {
        .name = MSG_SKILL_Accost_NAME,
        .desc = MSG_SKILL_Accost_DESC,
        .capacity = MSG_SKILL_Accost_CAPACITY,
        .price = MSG_SKILL_Accost_PRICE,
        .icon = GFX_SkillIcon_Accost,
    },
#endif

#if (defined(SID_HazeHunter) && COMMON_SKILL_VALID(SID_HazeHunter))
    [SID_HazeHunter] = {
        .name = MSG_SKILL_HazeHunter_NAME,
        .desc = MSG_SKILL_HazeHunter_DESC,
        .capacity = MSG_SKILL_HazeHunter_CAPACITY,
        .price = MSG_SKILL_HazeHunter_PRICE,
        .icon = GFX_SkillIcon_HazeHunter,
    },
#endif

#if (defined(SID_Rampage) && COMMON_SKILL_VALID(SID_Rampage))
    [SID_Rampage] = {
        .name = MSG_SKILL_Rampage_NAME,
        .desc = MSG_SKILL_Rampage_DESC,
        .capacity = MSG_SKILL_Rampage_CAPACITY,
        .price = MSG_SKILL_Rampage_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ArenaTrap) && COMMON_SKILL_VALID(SID_ArenaTrap))
    [SID_ArenaTrap] = {
        .name = MSG_SKILL_ArenaTrap_NAME,
        .desc = MSG_SKILL_ArenaTrap_DESC,
        .capacity = MSG_SKILL_ArenaTrap_CAPACITY,
        .price = MSG_SKILL_ArenaTrap_PRICE,
        .icon = GFX_SkillIcon_ArenaTrap,
    },
#endif

#if (defined(SID_Mentorship) && COMMON_SKILL_VALID(SID_Mentorship))
    [SID_Mentorship] = {
        .name = MSG_SKILL_Mentorship_NAME,
        .desc = MSG_SKILL_Mentorship_DESC,
        .capacity = MSG_SKILL_Mentorship_CAPACITY,
        .price = MSG_SKILL_Mentorship_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HyperCutter) && COMMON_SKILL_VALID(SID_HyperCutter))
    [SID_HyperCutter] = {
        .name = MSG_SKILL_HyperCutter_NAME,
        .desc = MSG_SKILL_HyperCutter_DESC,
        .capacity = MSG_SKILL_HyperCutter_CAPACITY,
        .price = MSG_SKILL_HyperCutter_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ClearBody) && COMMON_SKILL_VALID(SID_ClearBody))
    [SID_ClearBody] = {
        .name = MSG_SKILL_ClearBody_NAME,
        .desc = MSG_SKILL_ClearBody_DESC,
        .capacity = MSG_SKILL_ClearBody_CAPACITY,
        .price = MSG_SKILL_ClearBody_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LeadByExample) && COMMON_SKILL_VALID(SID_LeadByExample))
    [SID_LeadByExample] = {
        .name = MSG_SKILL_LeadByExample_NAME,
        .desc = MSG_SKILL_LeadByExample_DESC,
        .capacity = MSG_SKILL_LeadByExample_CAPACITY,
        .price = MSG_SKILL_LeadByExample_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GoodAsGold) && COMMON_SKILL_VALID(SID_GoodAsGold))
    [SID_GoodAsGold] = {
        .name = MSG_SKILL_GoodAsGold_NAME,
        .desc = MSG_SKILL_GoodAsGold_DESC,
        .capacity = MSG_SKILL_GoodAsGold_CAPACITY,
        .price = MSG_SKILL_GoodAsGold_PRICE,
        .icon = GFX_SkillIcon_GoodAsGold,
    },
#endif

#if (defined(SID_PsychUp) && COMMON_SKILL_VALID(SID_PsychUp))
    [SID_PsychUp] = {
        .name = MSG_SKILL_PsychUp_NAME,
        .desc = MSG_SKILL_PsychUp_DESC,
        .capacity = MSG_SKILL_PsychUp_CAPACITY,
        .price = MSG_SKILL_PsychUp_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Moody) && COMMON_SKILL_VALID(SID_Moody))
    [SID_Moody] = {
        .name = MSG_SKILL_Moody_NAME,
        .desc = MSG_SKILL_Moody_DESC,
        .capacity = MSG_SKILL_Moody_CAPACITY,
        .price = MSG_SKILL_Moody_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Amische) && COMMON_SKILL_VALID(SID_Amische))
    [SID_Amische] = {
        .name = MSG_SKILL_Amische_NAME,
        .desc = MSG_SKILL_Amische_DESC,
        .capacity = MSG_SKILL_Amische_CAPACITY,
        .price = MSG_SKILL_Amische_PRICE,
        .icon = GFX_SkillIcon_Amische,
    },
#endif

#if (defined(SID_DownWithArch) && COMMON_SKILL_VALID(SID_DownWithArch))
    [SID_DownWithArch] = {
        .name = MSG_SKILL_DownWithArch_NAME,
        .desc = MSG_SKILL_DownWithArch_DESC,
        .capacity = MSG_SKILL_DownWithArch_CAPACITY,
        .price = MSG_SKILL_DownWithArch_PRICE,
        .icon = GFX_SkillIcon_DownWithArch,
    },
#endif

#if (defined(SID_HeavyMetal) && COMMON_SKILL_VALID(SID_HeavyMetal))
    [SID_HeavyMetal] = {
        .name = MSG_SKILL_HeavyMetal_NAME,
        .desc = MSG_SKILL_HeavyMetal_DESC,
        .capacity = MSG_SKILL_HeavyMetal_CAPACITY,
        .price = MSG_SKILL_HeavyMetal_PRICE,
        .icon = GFX_SkillIcon_HeavyMetal,
    },
#endif

#if (defined(SID_Acrobat) && COMMON_SKILL_VALID(SID_Acrobat))
    [SID_Acrobat] = {
        .name = MSG_SKILL_Acrobat_NAME,
        .desc = MSG_SKILL_Acrobat_DESC,
        .capacity = MSG_SKILL_Acrobat_CAPACITY,
        .price = MSG_SKILL_Acrobat_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Contrary) && COMMON_SKILL_VALID(SID_Contrary))
    [SID_Contrary] = {
        .name = MSG_SKILL_Contrary_NAME,
        .desc = MSG_SKILL_Contrary_DESC,
        .capacity = MSG_SKILL_Contrary_CAPACITY,
        .price = MSG_SKILL_Contrary_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PairUp) && COMMON_SKILL_VALID(SID_PairUp))
    [SID_PairUp] = {
        .name = MSG_SKILL_PairUp_NAME,
        .desc = MSG_SKILL_PairUp_DESC,
        .capacity = MSG_SKILL_PairUp_CAPACITY,
        .price = MSG_SKILL_PairUp_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Reflex) && COMMON_SKILL_VALID(SID_Reflex))
    [SID_Reflex] = {
        .name = MSG_SKILL_Reflex_NAME,
        .desc = MSG_SKILL_Reflex_DESC,
        .capacity = MSG_SKILL_Reflex_CAPACITY,
        .price = MSG_SKILL_Reflex_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RampUp) && COMMON_SKILL_VALID(SID_RampUp))
    [SID_RampUp] = {
        .name = MSG_SKILL_RampUp_NAME,
        .desc = MSG_SKILL_RampUp_DESC,
        .capacity = MSG_SKILL_RampUp_CAPACITY,
        .price = MSG_SKILL_RampUp_PRICE,
        .icon = GFX_SkillIcon_RampUp,
    },
#endif

#if (defined(SID_Transform) && COMMON_SKILL_VALID(SID_Transform))
    [SID_Transform] = {
        .name = MSG_SKILL_Transform_NAME,
        .desc = MSG_SKILL_Transform_DESC,
        .capacity = MSG_SKILL_Transform_CAPACITY,
        .price = MSG_SKILL_Transform_PRICE,
        .icon = GFX_SkillIcon_Transform,
    },
#endif

#if (defined(SID_DarkBargain) && COMMON_SKILL_VALID(SID_DarkBargain))
    [SID_DarkBargain] = {
        .name = MSG_SKILL_DarkBargain_NAME,
        .desc = MSG_SKILL_DarkBargain_DESC,
        .capacity = MSG_SKILL_DarkBargain_CAPACITY,
        .price = MSG_SKILL_DarkBargain_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DrawBack) && COMMON_SKILL_VALID(SID_DrawBack))
    [SID_DrawBack] = {
        .name = MSG_SKILL_DrawBack_NAME,
        .desc = MSG_SKILL_DrawBack_DESC,
        .capacity = MSG_SKILL_DrawBack_CAPACITY,
        .price = MSG_SKILL_DrawBack_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Pivot) && COMMON_SKILL_VALID(SID_Pivot))
    [SID_Pivot] = {
        .name = MSG_SKILL_Pivot_NAME,
        .desc = MSG_SKILL_Pivot_DESC,
        .capacity = MSG_SKILL_Pivot_CAPACITY,
        .price = MSG_SKILL_Pivot_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Reposition) && COMMON_SKILL_VALID(SID_Reposition))
    [SID_Reposition] = {
        .name = MSG_SKILL_Reposition_NAME,
        .desc = MSG_SKILL_Reposition_DESC,
        .capacity = MSG_SKILL_Reposition_CAPACITY,
        .price = MSG_SKILL_Reposition_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Shove) && COMMON_SKILL_VALID(SID_Shove))
    [SID_Shove] = {
        .name = MSG_SKILL_Shove_NAME,
        .desc = MSG_SKILL_Shove_DESC,
        .capacity = MSG_SKILL_Shove_CAPACITY,
        .price = MSG_SKILL_Shove_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Smite) && COMMON_SKILL_VALID(SID_Smite))
    [SID_Smite] = {
        .name = MSG_SKILL_Smite_NAME,
        .desc = MSG_SKILL_Smite_DESC,
        .capacity = MSG_SKILL_Smite_CAPACITY,
        .price = MSG_SKILL_Smite_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ShadowGift) && COMMON_SKILL_VALID(SID_ShadowGift))
    [SID_ShadowGift] = {
        .name = MSG_SKILL_ShadowGift_NAME,
        .desc = MSG_SKILL_ShadowGift_DESC,
        .capacity = MSG_SKILL_ShadowGift_CAPACITY,
        .price = MSG_SKILL_ShadowGift_PRICE,
        .icon = GFX_SkillIcon_ShadowGift,
    },
#endif

#if (defined(SID_ShadowGiftPlus) && COMMON_SKILL_VALID(SID_ShadowGiftPlus))
    [SID_ShadowGiftPlus] = {
        .name = MSG_SKILL_ShadowGiftPlus_NAME,
        .desc = MSG_SKILL_ShadowGiftPlus_DESC,
        .capacity = MSG_SKILL_ShadowGiftPlus_CAPACITY,
        .price = MSG_SKILL_ShadowGiftPlus_PRICE,
        .icon = GFX_SkillIcon_ShadowGiftPlus,
    },
#endif

#if (defined(SID_LightGift) && COMMON_SKILL_VALID(SID_LightGift))
    [SID_LightGift] = {
        .name = MSG_SKILL_LightGift_NAME,
        .desc = MSG_SKILL_LightGift_DESC,
        .capacity = MSG_SKILL_LightGift_CAPACITY,
        .price = MSG_SKILL_LightGift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LightGiftPlus) && COMMON_SKILL_VALID(SID_LightGiftPlus))
    [SID_LightGiftPlus] = {
        .name = MSG_SKILL_LightGiftPlus_NAME,
        .desc = MSG_SKILL_LightGiftPlus_DESC,
        .capacity = MSG_SKILL_LightGiftPlus_CAPACITY,
        .price = MSG_SKILL_LightGiftPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Stormgift) && COMMON_SKILL_VALID(SID_Stormgift))
    [SID_Stormgift] = {
        .name = MSG_SKILL_Stormgift_NAME,
        .desc = MSG_SKILL_Stormgift_DESC,
        .capacity = MSG_SKILL_Stormgift_CAPACITY,
        .price = MSG_SKILL_Stormgift_PRICE,
        .icon = GFX_SkillIcon_Stormgift,
    },
#endif

#if (defined(SID_StormgiftPlus) && COMMON_SKILL_VALID(SID_StormgiftPlus))
    [SID_StormgiftPlus] = {
        .name = MSG_SKILL_StormgiftPlus_NAME,
        .desc = MSG_SKILL_StormgiftPlus_DESC,
        .capacity = MSG_SKILL_StormgiftPlus_CAPACITY,
        .price = MSG_SKILL_StormgiftPlus_PRICE,
        .icon = GFX_SkillIcon_StormgiftPlus,
    },
#endif

#if (defined(SID_Gracegift) && COMMON_SKILL_VALID(SID_Gracegift))
    [SID_Gracegift] = {
        .name = MSG_SKILL_Gracegift_NAME,
        .desc = MSG_SKILL_Gracegift_DESC,
        .capacity = MSG_SKILL_Gracegift_CAPACITY,
        .price = MSG_SKILL_Gracegift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GracegiftPlus) && COMMON_SKILL_VALID(SID_GracegiftPlus))
    [SID_GracegiftPlus] = {
        .name = MSG_SKILL_GracegiftPlus_NAME,
        .desc = MSG_SKILL_GracegiftPlus_DESC,
        .capacity = MSG_SKILL_GracegiftPlus_CAPACITY,
        .price = MSG_SKILL_GracegiftPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Bladegift) && COMMON_SKILL_VALID(SID_Bladegift))
    [SID_Bladegift] = {
        .name = MSG_SKILL_Bladegift_NAME,
        .desc = MSG_SKILL_Bladegift_DESC,
        .capacity = MSG_SKILL_Bladegift_CAPACITY,
        .price = MSG_SKILL_Bladegift_PRICE,
        .icon = GFX_SkillIcon_Bladegift,
    },
#endif

#if (defined(SID_BladegiftPlus) && COMMON_SKILL_VALID(SID_BladegiftPlus))
    [SID_BladegiftPlus] = {
        .name = MSG_SKILL_BladegiftPlus_NAME,
        .desc = MSG_SKILL_BladegiftPlus_DESC,
        .capacity = MSG_SKILL_BladegiftPlus_CAPACITY,
        .price = MSG_SKILL_BladegiftPlus_PRICE,
        .icon = GFX_SkillIcon_BladegiftPlus,
    },
#endif

#if (defined(SID_Piercegift) && COMMON_SKILL_VALID(SID_Piercegift))
    [SID_Piercegift] = {
        .name = MSG_SKILL_Piercegift_NAME,
        .desc = MSG_SKILL_Piercegift_DESC,
        .capacity = MSG_SKILL_Piercegift_CAPACITY,
        .price = MSG_SKILL_Piercegift_PRICE,
        .icon = GFX_SkillIcon_Piercegift,
    },
#endif

#if (defined(SID_PiercegiftPlus) && COMMON_SKILL_VALID(SID_PiercegiftPlus))
    [SID_PiercegiftPlus] = {
        .name = MSG_SKILL_PiercegiftPlus_NAME,
        .desc = MSG_SKILL_PiercegiftPlus_DESC,
        .capacity = MSG_SKILL_PiercegiftPlus_CAPACITY,
        .price = MSG_SKILL_PiercegiftPlus_PRICE,
        .icon = GFX_SkillIcon_PiercegiftPlus,
    },
#endif

#if (defined(SID_Hackgift) && COMMON_SKILL_VALID(SID_Hackgift))
    [SID_Hackgift] = {
        .name = MSG_SKILL_Hackgift_NAME,
        .desc = MSG_SKILL_Hackgift_DESC,
        .capacity = MSG_SKILL_Hackgift_CAPACITY,
        .price = MSG_SKILL_Hackgift_PRICE,
        .icon = GFX_SkillIcon_Hackgift,
    },
#endif

#if (defined(SID_HackgiftPlus) && COMMON_SKILL_VALID(SID_HackgiftPlus))
    [SID_HackgiftPlus] = {
        .name = MSG_SKILL_HackgiftPlus_NAME,
        .desc = MSG_SKILL_HackgiftPlus_DESC,
        .capacity = MSG_SKILL_HackgiftPlus_CAPACITY,
        .price = MSG_SKILL_HackgiftPlus_PRICE,
        .icon = GFX_SkillIcon_HackgiftPlus,
    },
#endif

#if (defined(SID_Arcgift) && COMMON_SKILL_VALID(SID_Arcgift))
    [SID_Arcgift] = {
        .name = MSG_SKILL_Arcgift_NAME,
        .desc = MSG_SKILL_Arcgift_DESC,
        .capacity = MSG_SKILL_Arcgift_CAPACITY,
        .price = MSG_SKILL_Arcgift_PRICE,
        .icon = GFX_SkillIcon_Arcgift,
    },
#endif

#if (defined(SID_ArcgiftPlus) && COMMON_SKILL_VALID(SID_ArcgiftPlus))
    [SID_ArcgiftPlus] = {
        .name = MSG_SKILL_ArcgiftPlus_NAME,
        .desc = MSG_SKILL_ArcgiftPlus_DESC,
        .capacity = MSG_SKILL_ArcgiftPlus_CAPACITY,
        .price = MSG_SKILL_ArcgiftPlus_PRICE,
        .icon = GFX_SkillIcon_ArcgiftPlus,
    },
#endif

#if (defined(SID_StealPlus) && COMMON_SKILL_VALID(SID_StealPlus))
    [SID_StealPlus] = {
        .name = MSG_SKILL_StealPlus_NAME,
        .desc = MSG_SKILL_StealPlus_DESC,
        .capacity = MSG_SKILL_StealPlus_CAPACITY,
        .price = MSG_SKILL_StealPlus_PRICE,
        .icon = GFX_SkillIcon_StealPlus,
    },
#endif

#if (defined(SID_UpWithArch) && COMMON_SKILL_VALID(SID_UpWithArch))
    [SID_UpWithArch] = {
        .name = MSG_SKILL_UpWithArch_NAME,
        .desc = MSG_SKILL_UpWithArch_DESC,
        .capacity = MSG_SKILL_UpWithArch_CAPACITY,
        .price = MSG_SKILL_UpWithArch_PRICE,
        .icon = GFX_SkillIcon_UpWithArch,
    },
#endif

#if (defined(SID_ArdentSacrifice) && COMMON_SKILL_VALID(SID_ArdentSacrifice))
    [SID_ArdentSacrifice] = {
        .name = MSG_SKILL_ArdentSacrifice_NAME,
        .desc = MSG_SKILL_ArdentSacrifice_DESC,
        .capacity = MSG_SKILL_ArdentSacrifice_CAPACITY,
        .price = MSG_SKILL_ArdentSacrifice_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Cultured) && COMMON_SKILL_VALID(SID_Cultured))
    [SID_Cultured] = {
        .name = MSG_SKILL_Cultured_NAME,
        .desc = MSG_SKILL_Cultured_DESC,
        .capacity = MSG_SKILL_Cultured_CAPACITY,
        .price = MSG_SKILL_Cultured_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_NiceThighs) && COMMON_SKILL_VALID(SID_NiceThighs))
    [SID_NiceThighs] = {
        .name = MSG_SKILL_NiceThighs_NAME,
        .desc = MSG_SKILL_NiceThighs_DESC,
        .capacity = MSG_SKILL_NiceThighs_CAPACITY,
        .price = MSG_SKILL_NiceThighs_PRICE,
        .icon = GFX_SkillIcon_NiceThighs,
    },
#endif

#if (defined(SID_Flank) && COMMON_SKILL_VALID(SID_Flank))
    [SID_Flank] = {
        .name = MSG_SKILL_Flank_NAME,
        .desc = MSG_SKILL_Flank_DESC,
        .capacity = MSG_SKILL_Flank_CAPACITY,
        .price = MSG_SKILL_Flank_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ForeignPrincess) && COMMON_SKILL_VALID(SID_ForeignPrincess))
    [SID_ForeignPrincess] = {
        .name = MSG_SKILL_ForeignPrincess_NAME,
        .desc = MSG_SKILL_ForeignPrincess_DESC,
        .capacity = MSG_SKILL_ForeignPrincess_CAPACITY,
        .price = MSG_SKILL_ForeignPrincess_PRICE,
        .icon = GFX_SkillIcon_ForeignPrincess,
    },
#endif

#if (defined(SID_FullMetalBody) && COMMON_SKILL_VALID(SID_FullMetalBody))
    [SID_FullMetalBody] = {
        .name = MSG_SKILL_FullMetalBody_NAME,
        .desc = MSG_SKILL_FullMetalBody_DESC,
        .capacity = MSG_SKILL_FullMetalBody_CAPACITY,
        .price = MSG_SKILL_FullMetalBody_PRICE,
        .icon = GFX_SkillIcon_FullMetalBody,
    },
#endif

#if (defined(SID_HoardersBane) && COMMON_SKILL_VALID(SID_HoardersBane))
    [SID_HoardersBane] = {
        .name = MSG_SKILL_HoardersBane_NAME,
        .desc = MSG_SKILL_HoardersBane_DESC,
        .capacity = MSG_SKILL_HoardersBane_CAPACITY,
        .price = MSG_SKILL_HoardersBane_PRICE,
        .icon = GFX_SkillIcon_HoardersBane,
    },
#endif

#if (defined(SID_IdentityProblems) && COMMON_SKILL_VALID(SID_IdentityProblems))
    [SID_IdentityProblems] = {
        .name = MSG_SKILL_IdentityProblems_NAME,
        .desc = MSG_SKILL_IdentityProblems_DESC,
        .capacity = MSG_SKILL_IdentityProblems_CAPACITY,
        .price = MSG_SKILL_IdentityProblems_PRICE,
        .icon = GFX_SkillIcon_IdentityProblems,
    },
#endif

#if (defined(SID_Pathfinder) && COMMON_SKILL_VALID(SID_Pathfinder))
    [SID_Pathfinder] = {
        .name = MSG_SKILL_Pathfinder_NAME,
        .desc = MSG_SKILL_Pathfinder_DESC,
        .capacity = MSG_SKILL_Pathfinder_CAPACITY,
        .price = MSG_SKILL_Pathfinder_PRICE,
        .icon = GFX_SkillIcon_Pathfinder,
    },
#endif

#if (defined(SID_ReciprocalAid) && COMMON_SKILL_VALID(SID_ReciprocalAid))
    [SID_ReciprocalAid] = {
        .name = MSG_SKILL_ReciprocalAid_NAME,
        .desc = MSG_SKILL_ReciprocalAid_DESC,
        .capacity = MSG_SKILL_ReciprocalAid_CAPACITY,
        .price = MSG_SKILL_ReciprocalAid_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Roll12) && COMMON_SKILL_VALID(SID_Roll12))
    [SID_Roll12] = {
        .name = MSG_SKILL_Roll12_NAME,
        .desc = MSG_SKILL_Roll12_DESC,
        .capacity = MSG_SKILL_Roll12_CAPACITY,
        .price = MSG_SKILL_Roll12_PRICE,
        .icon = GFX_SkillIcon_Roll12,
    },
#endif

#if (defined(SID_Sacrifice) && COMMON_SKILL_VALID(SID_Sacrifice))
    [SID_Sacrifice] = {
        .name = MSG_SKILL_Sacrifice_NAME,
        .desc = MSG_SKILL_Sacrifice_DESC,
        .capacity = MSG_SKILL_Sacrifice_CAPACITY,
        .price = MSG_SKILL_Sacrifice_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Thighdeology) && COMMON_SKILL_VALID(SID_Thighdeology))
    [SID_Thighdeology] = {
        .name = MSG_SKILL_Thighdeology_NAME,
        .desc = MSG_SKILL_Thighdeology_DESC,
        .capacity = MSG_SKILL_Thighdeology_CAPACITY,
        .price = MSG_SKILL_Thighdeology_PRICE,
        .icon = GFX_SkillIcon_Thighdeology,
    },
#endif

#if (defined(SID_ThotSlayer) && COMMON_SKILL_VALID(SID_ThotSlayer))
    [SID_ThotSlayer] = {
        .name = MSG_SKILL_ThotSlayer_NAME,
        .desc = MSG_SKILL_ThotSlayer_DESC,
        .capacity = MSG_SKILL_ThotSlayer_CAPACITY,
        .price = MSG_SKILL_ThotSlayer_PRICE,
        .icon = GFX_SkillIcon_ThotSlayer,
    },
#endif

#if (defined(SID_TriangleAttack) && COMMON_SKILL_VALID(SID_TriangleAttack))
    [SID_TriangleAttack] = {
        .name = MSG_SKILL_TriangleAttack_NAME,
        .desc = MSG_SKILL_TriangleAttack_DESC,
        .capacity = MSG_SKILL_TriangleAttack_CAPACITY,
        .price = MSG_SKILL_TriangleAttack_PRICE,
        .icon = GFX_SkillIcon_TriangleAttack,
    },
#endif

#if (defined(SID_Swarm) && COMMON_SKILL_VALID(SID_Swarm))
    [SID_Swarm] = {
        .name = MSG_SKILL_Swarm_NAME,
        .desc = MSG_SKILL_Swarm_DESC,
        .capacity = MSG_SKILL_Swarm_CAPACITY,
        .price = MSG_SKILL_Swarm_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Capture) && COMMON_SKILL_VALID(SID_Capture))
    [SID_Capture] = {
        .name = MSG_SKILL_Capture_NAME,
        .desc = MSG_SKILL_Capture_DESC,
        .capacity = MSG_SKILL_Capture_CAPACITY,
        .price = MSG_SKILL_Capture_PRICE,
        .icon = GFX_SkillIcon_Capture,
    },
#endif

#if (defined(SID_MoonBow) && COMMON_SKILL_VALID(SID_MoonBow))
    [SID_MoonBow] = {
        .name = MSG_SKILL_MoonBow_NAME,
        .desc = MSG_SKILL_MoonBow_DESC,
        .capacity = MSG_SKILL_MoonBow_CAPACITY,
        .price = MSG_SKILL_MoonBow_PRICE,
        .icon = GFX_SkillIcon_MoonBow,
    },
#endif

#if (defined(SID_PastelVeil) && COMMON_SKILL_VALID(SID_PastelVeil))
    [SID_PastelVeil] = {
        .name = MSG_SKILL_PastelVeil_NAME,
        .desc = MSG_SKILL_PastelVeil_DESC,
        .capacity = MSG_SKILL_PastelVeil_CAPACITY,
        .price = MSG_SKILL_PastelVeil_PRICE,
        .icon = GFX_SkillIcon_PastelVeil,
    },
#endif

#if (defined(SID_CatchEmAll) && COMMON_SKILL_VALID(SID_CatchEmAll))
    [SID_CatchEmAll] = {
        .name = MSG_SKILL_CatchEmAll_NAME,
        .desc = MSG_SKILL_CatchEmAll_DESC,
        .capacity = MSG_SKILL_CatchEmAll_CAPACITY,
        .price = MSG_SKILL_CatchEmAll_PRICE,
        .icon = GFX_SkillIcon_CatchEmAll,
    },
#endif

#if (defined(SID_DarkHorse) && COMMON_SKILL_VALID(SID_DarkHorse))
    [SID_DarkHorse] = {
        .name = MSG_SKILL_DarkHorse_NAME,
        .desc = MSG_SKILL_DarkHorse_DESC,
        .capacity = MSG_SKILL_DarkHorse_CAPACITY,
        .price = MSG_SKILL_DarkHorse_PRICE,
        .icon = GFX_SkillIcon_DarkHorse,
    },
#endif

#if (defined(SID_AdaptiveStance) && COMMON_SKILL_VALID(SID_AdaptiveStance))
    [SID_AdaptiveStance] = {
        .name = MSG_SKILL_AdaptiveStance_NAME,
        .desc = MSG_SKILL_AdaptiveStance_DESC,
        .capacity = MSG_SKILL_AdaptiveStance_CAPACITY,
        .price = MSG_SKILL_AdaptiveStance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AdrenalineRush) && COMMON_SKILL_VALID(SID_AdrenalineRush))
    [SID_AdrenalineRush] = {
        .name = MSG_SKILL_AdrenalineRush_NAME,
        .desc = MSG_SKILL_AdrenalineRush_DESC,
        .capacity = MSG_SKILL_AdrenalineRush_CAPACITY,
        .price = MSG_SKILL_AdrenalineRush_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TripUp) && COMMON_SKILL_VALID(SID_TripUp))
    [SID_TripUp] = {
        .name = MSG_SKILL_TripUp_NAME,
        .desc = MSG_SKILL_TripUp_DESC,
        .capacity = MSG_SKILL_TripUp_CAPACITY,
        .price = MSG_SKILL_TripUp_PRICE,
        .icon = GFX_SkillIcon_TripUp,
    },
#endif

#if (defined(SID_DemolitionExpert) && COMMON_SKILL_VALID(SID_DemolitionExpert))
    [SID_DemolitionExpert] = {
        .name = MSG_SKILL_DemolitionExpert_NAME,
        .desc = MSG_SKILL_DemolitionExpert_DESC,
        .capacity = MSG_SKILL_DemolitionExpert_CAPACITY,
        .price = MSG_SKILL_DemolitionExpert_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif


#if (defined(SID_Onimaru) && COMMON_SKILL_VALID(SID_Onimaru))
    [SID_Onimaru] = {
        .name = MSG_SKILL_Onimaru_NAME,
        .desc = MSG_SKILL_Onimaru_DESC,
        .capacity = MSG_SKILL_Onimaru_CAPACITY,
        .price = MSG_SKILL_Onimaru_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AdaptiveLunge) && COMMON_SKILL_VALID(SID_AdaptiveLunge))
    [SID_AdaptiveLunge] = {
        .name = MSG_SKILL_AdaptiveLunge_NAME,
        .desc = MSG_SKILL_AdaptiveLunge_DESC,
        .capacity = MSG_SKILL_AdaptiveLunge_CAPACITY,
        .price = MSG_SKILL_AdaptiveLunge_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DoubleUp) && COMMON_SKILL_VALID(SID_DoubleUp))
    [SID_DoubleUp] = {
        .name = MSG_SKILL_DoubleUp_NAME,
        .desc = MSG_SKILL_DoubleUp_DESC,
        .capacity = MSG_SKILL_DoubleUp_CAPACITY,
        .price = MSG_SKILL_DoubleUp_PRICE,
        .icon = GFX_SkillIcon_DoubleUp,
    },
#endif

#if (defined(SID_TripleUp) && COMMON_SKILL_VALID(SID_TripleUp))
    [SID_TripleUp] = {
        .name = MSG_SKILL_TripleUp_NAME,
        .desc = MSG_SKILL_TripleUp_DESC,
        .capacity = MSG_SKILL_TripleUp_CAPACITY,
        .price = MSG_SKILL_TripleUp_PRICE,
        .icon = GFX_SkillIcon_TripleUp,
    },
#endif

#if (defined(SID_Casual) && COMMON_SKILL_VALID(SID_Casual))
    [SID_Casual] = {
        .name = MSG_SKILL_Casual_NAME,
        .desc = MSG_SKILL_Casual_DESC,
        .capacity = MSG_SKILL_Casual_CAPACITY,
        .price = MSG_SKILL_Casual_PRICE,
        .icon = GFX_SkillIcon_Casual,
    },
#endif

#if (defined(SID_Doppleganger) && COMMON_SKILL_VALID(SID_Doppleganger))
    [SID_Doppleganger] = {
        .name = MSG_SKILL_Doppleganger_NAME,
        .desc = MSG_SKILL_Doppleganger_DESC,
        .capacity = MSG_SKILL_Doppleganger_CAPACITY,
        .price = MSG_SKILL_Doppleganger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Turncoat) && COMMON_SKILL_VALID(SID_Turncoat))
    [SID_Turncoat] = {
        .name = MSG_SKILL_Turncoat_NAME,
        .desc = MSG_SKILL_Turncoat_DESC,
        .capacity = MSG_SKILL_Turncoat_CAPACITY,
        .price = MSG_SKILL_Turncoat_PRICE,
        .icon = GFX_SkillIcon_Turncoat,
    },
#endif

#if (defined(SID_LimitBreaker) && COMMON_SKILL_VALID(SID_LimitBreaker))
    [SID_LimitBreaker] = {
        .name = MSG_SKILL_LimitBreaker_NAME,
        .desc = MSG_SKILL_LimitBreaker_DESC,
        .capacity = MSG_SKILL_LimitBreaker_CAPACITY,
        .price = MSG_SKILL_LimitBreaker_PRICE,
        .icon = GFX_SkillIcon_LimitBreaker,
    },
#endif

#if (defined(SID_LimitBreakerPlus) && COMMON_SKILL_VALID(SID_LimitBreakerPlus))
    [SID_LimitBreakerPlus] = {
        .name = MSG_SKILL_LimitBreakerPlus_NAME,
        .desc = MSG_SKILL_LimitBreakerPlus_DESC,
        .capacity = MSG_SKILL_LimitBreakerPlus_CAPACITY,
        .price = MSG_SKILL_LimitBreakerPlus_PRICE,
        .icon = GFX_SkillIcon_LimitBreakerPlus,
    },
#endif

#if (defined(SID_Dismount) && COMMON_SKILL_VALID(SID_Dismount))
    [SID_Dismount] = {
        .name = MSG_SKILL_Dismount_NAME,
        .desc = MSG_SKILL_Dismount_DESC,
        .capacity = MSG_SKILL_Dismount_CAPACITY,
        .price = MSG_SKILL_Dismount_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RiskItAll) && COMMON_SKILL_VALID(SID_RiskItAll))
    [SID_RiskItAll] = {
        .name = MSG_SKILL_RiskItAll_NAME,
        .desc = MSG_SKILL_RiskItAll_DESC,
        .capacity = MSG_SKILL_RiskItAll_CAPACITY,
        .price = MSG_SKILL_RiskItAll_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DestinyBond) && COMMON_SKILL_VALID(SID_DestinyBond))
    [SID_DestinyBond] = {
        .name = MSG_SKILL_DestinyBond_NAME,
        .desc = MSG_SKILL_DestinyBond_DESC,
        .capacity = MSG_SKILL_DestinyBond_CAPACITY,
        .price = MSG_SKILL_DestinyBond_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Secret) && COMMON_SKILL_VALID(SID_Secret))
    [SID_Secret] = {
        .name = MSG_SKILL_Secret_NAME,
        .desc = MSG_SKILL_Secret_DESC,
        .capacity = MSG_SKILL_Secret_CAPACITY,
        .price = MSG_SKILL_Secret_PRICE,
        .icon = GFX_SkillIcon_Secret,
    },
#endif

#if (defined(SID_Ludopathy) && COMMON_SKILL_VALID(SID_Ludopathy))
    [SID_Ludopathy] = {
        .name = MSG_SKILL_Ludopathy_NAME,
        .desc = MSG_SKILL_Ludopathy_DESC,
        .capacity = MSG_SKILL_Ludopathy_CAPACITY,
        .price = MSG_SKILL_Ludopathy_PRICE,
        .icon = GFX_SkillIcon_Ludopathy,
    },
#endif

#if (defined(SID_WaterWalking) && COMMON_SKILL_VALID(SID_WaterWalking))
    [SID_WaterWalking] = {
        .name = MSG_SKILL_WaterWalking_NAME,
        .desc = MSG_SKILL_WaterWalking_DESC,
        .capacity = MSG_SKILL_WaterWalking_CAPACITY,
        .price = MSG_SKILL_WaterWalking_PRICE,
        .icon = GFX_SkillIcon_WaterWalking,
    },
#endif

#if (defined(SID_WaterWalkingPlus) && COMMON_SKILL_VALID(SID_WaterWalkingPlus))
    [SID_WaterWalkingPlus] = {
        .name = MSG_SKILL_WaterWalkingPlus_NAME,
        .desc = MSG_SKILL_WaterWalkingPlus_DESC,
        .capacity = MSG_SKILL_WaterWalkingPlus_CAPACITY,
        .price = MSG_SKILL_WaterWalkingPlus_PRICE,
        .icon = GFX_SkillIcon_WaterWalkingPlus,
    },
#endif

#if (defined(SID_PoisonHeal) && COMMON_SKILL_VALID(SID_PoisonHeal))
    [SID_PoisonHeal] = {
        .name = MSG_SKILL_PoisonHeal_NAME,
        .desc = MSG_SKILL_PoisonHeal_DESC,
        .capacity = MSG_SKILL_PoisonHeal_CAPACITY,
        .price = MSG_SKILL_PoisonHeal_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BoundlessVitality) && COMMON_SKILL_VALID(SID_BoundlessVitality))
    [SID_BoundlessVitality] = {
        .name = MSG_SKILL_BoundlessVitality_NAME,
        .desc = MSG_SKILL_BoundlessVitality_DESC,
        .capacity = MSG_SKILL_BoundlessVitality_CAPACITY,
        .price = MSG_SKILL_BoundlessVitality_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Fluffy) && COMMON_SKILL_VALID(SID_Fluffy))
    [SID_Fluffy] = {
        .name = MSG_SKILL_Fluffy_NAME,
        .desc = MSG_SKILL_Fluffy_DESC,
        .capacity = MSG_SKILL_Fluffy_CAPACITY,
        .price = MSG_SKILL_Fluffy_PRICE,
        .icon = GFX_SkillIcon_Fluffy,
    },
#endif

#if (defined(SID_MountainClimber) && COMMON_SKILL_VALID(SID_MountainClimber))
    [SID_MountainClimber] = {
        .name = MSG_SKILL_MountainClimber_NAME,
        .desc = MSG_SKILL_MountainClimber_DESC,
        .capacity = MSG_SKILL_MountainClimber_CAPACITY,
        .price = MSG_SKILL_MountainClimber_PRICE,
        .icon = GFX_SkillIcon_MountainClimber,
    },
#endif

#if (defined(SID_MountainClimberPlus) && COMMON_SKILL_VALID(SID_MountainClimberPlus))
    [SID_MountainClimberPlus] = {
        .name = MSG_SKILL_MountainClimberPlus_NAME,
        .desc = MSG_SKILL_MountainClimberPlus_DESC,
        .capacity = MSG_SKILL_MountainClimberPlus_CAPACITY,
        .price = MSG_SKILL_MountainClimberPlus_PRICE,
        .icon = GFX_SkillIcon_MountainClimberPlus,
    },
#endif

#if (defined(SID_Survivor) && COMMON_SKILL_VALID(SID_Survivor))
    [SID_Survivor] = {
        .name = MSG_SKILL_Survivor_NAME,
        .desc = MSG_SKILL_Survivor_DESC,
        .capacity = MSG_SKILL_Survivor_CAPACITY,
        .price = MSG_SKILL_Survivor_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Anchor) && COMMON_SKILL_VALID(SID_Anchor))
    [SID_Anchor] = {
        .name = MSG_SKILL_Anchor_NAME,
        .desc = MSG_SKILL_Anchor_DESC,
        .capacity = MSG_SKILL_Anchor_CAPACITY,
        .price = MSG_SKILL_Anchor_PRICE,
        .icon = GFX_SkillIcon_Anchor,
    },
#endif

#if (defined(SID_Mercurious) && COMMON_SKILL_VALID(SID_Mercurious))
    [SID_Mercurious] = {
        .name = MSG_SKILL_Mercurious_NAME,
        .desc = MSG_SKILL_Mercurious_DESC,
        .capacity = MSG_SKILL_Mercurious_CAPACITY,
        .price = MSG_SKILL_Mercurious_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Absolve) && COMMON_SKILL_VALID(SID_Absolve))
    [SID_Absolve] = {
        .name = MSG_SKILL_Absolve_NAME,
        .desc = MSG_SKILL_Absolve_DESC,
        .capacity = MSG_SKILL_Absolve_CAPACITY,
        .price = MSG_SKILL_Absolve_PRICE,
        .icon = GFX_SkillIcon_Absolve,
    },
#endif

#if (defined(SID_Entrepreneur) && COMMON_SKILL_VALID(SID_Entrepreneur))
    [SID_Entrepreneur] = {
        .name = MSG_SKILL_Entrepreneur_NAME,
        .desc = MSG_SKILL_Entrepreneur_DESC,
        .capacity = MSG_SKILL_Entrepreneur_CAPACITY,
        .price = MSG_SKILL_Entrepreneur_PRICE,
        .icon = GFX_SkillIcon_Entrepreneur,
    },
#endif

#if (defined(SID_Velocity) && COMMON_SKILL_VALID(SID_Velocity))
    [SID_Velocity] = {
        .name = MSG_SKILL_Velocity_NAME,
        .desc = MSG_SKILL_Velocity_DESC,
        .capacity = MSG_SKILL_Velocity_CAPACITY,
        .price = MSG_SKILL_Velocity_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Unaware) && COMMON_SKILL_VALID(SID_Unaware))
    [SID_Unaware] = {
        .name = MSG_SKILL_Unaware_NAME,
        .desc = MSG_SKILL_Unaware_DESC,
        .capacity = MSG_SKILL_Unaware_CAPACITY,
        .price = MSG_SKILL_Unaware_PRICE,
        .icon = GFX_SkillIcon_Unaware,
    },
#endif

#if (defined(SID_AngerPoint) && COMMON_SKILL_VALID(SID_AngerPoint))
    [SID_AngerPoint] = {
        .name = MSG_SKILL_AngerPoint_NAME,
        .desc = MSG_SKILL_AngerPoint_DESC,
        .capacity = MSG_SKILL_AngerPoint_CAPACITY,
        .price = MSG_SKILL_AngerPoint_PRICE,
        .icon = GFX_SkillIcon_AngerPoint,
    },
#endif

#if (defined(SID_Adaptable) && COMMON_SKILL_VALID(SID_Adaptable))
    [SID_Adaptable] = {
        .name = MSG_SKILL_Adaptable_NAME,
        .desc = MSG_SKILL_Adaptable_DESC,
        .capacity = MSG_SKILL_Adaptable_CAPACITY,
        .price = MSG_SKILL_Adaptable_PRICE,
        .icon = GFX_SkillIcon_Adaptable,
    },
#endif

#if (defined(SID_Debilitator) && COMMON_SKILL_VALID(SID_Debilitator))
    [SID_Debilitator] = {
        .name = MSG_SKILL_Debilitator_NAME,
        .desc = MSG_SKILL_Debilitator_DESC,
        .capacity = MSG_SKILL_Debilitator_CAPACITY,
        .price = MSG_SKILL_Debilitator_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Mantle) && COMMON_SKILL_VALID(SID_Mantle))
    [SID_Mantle] = {
        .name = MSG_SKILL_Mantle_NAME,
        .desc = MSG_SKILL_Mantle_DESC,
        .capacity = MSG_SKILL_Mantle_CAPACITY,
        .price = MSG_SKILL_Mantle_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_QuickHands) && COMMON_SKILL_VALID(SID_QuickHands))
    [SID_QuickHands] = {
        .name = MSG_SKILL_QuickHands_NAME,
        .desc = MSG_SKILL_QuickHands_DESC,
        .capacity = MSG_SKILL_QuickHands_CAPACITY,
        .price = MSG_SKILL_QuickHands_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_StaffParagon) && COMMON_SKILL_VALID(SID_StaffParagon))
    [SID_StaffParagon] = {
        .name = MSG_SKILL_StaffParagon_NAME,
        .desc = MSG_SKILL_StaffParagon_DESC,
        .capacity = MSG_SKILL_StaffParagon_CAPACITY,
        .price = MSG_SKILL_StaffParagon_PRICE,
        .icon = GFX_SkillIcon_StaffParagon,
    },
#endif

#if (defined(SID_OgreBody) && COMMON_SKILL_VALID(SID_OgreBody))
    [SID_OgreBody] = {
        .name = MSG_SKILL_OgreBody_NAME,
        .desc = MSG_SKILL_OgreBody_DESC,
        .capacity = MSG_SKILL_OgreBody_CAPACITY,
        .price = MSG_SKILL_OgreBody_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BeginnersLuck) && COMMON_SKILL_VALID(SID_BeginnersLuck))
    [SID_BeginnersLuck] = {
        .name = MSG_SKILL_BeginnersLuck_NAME,
        .desc = MSG_SKILL_BeginnersLuck_DESC,
        .capacity = MSG_SKILL_BeginnersLuck_CAPACITY,
        .price = MSG_SKILL_BeginnersLuck_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ExpertsExperience) && COMMON_SKILL_VALID(SID_ExpertsExperience))
    [SID_ExpertsExperience] = {
        .name = MSG_SKILL_ExpertsExperience_NAME,
        .desc = MSG_SKILL_ExpertsExperience_DESC,
        .capacity = MSG_SKILL_ExpertsExperience_CAPACITY,
        .price = MSG_SKILL_ExpertsExperience_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HedgeFund) && COMMON_SKILL_VALID(SID_HedgeFund))
    [SID_HedgeFund] = {
        .name = MSG_SKILL_HedgeFund_NAME,
        .desc = MSG_SKILL_HedgeFund_DESC,
        .capacity = MSG_SKILL_HedgeFund_CAPACITY,
        .price = MSG_SKILL_HedgeFund_PRICE,
        .icon = GFX_SkillIcon_HedgeFund,
    },
#endif

#if (defined(SID_Domain) && COMMON_SKILL_VALID(SID_Domain))
    [SID_Domain] = {
        .name = MSG_SKILL_Domain_NAME,
        .desc = MSG_SKILL_Domain_DESC,
        .capacity = MSG_SKILL_Domain_CAPACITY,
        .price = MSG_SKILL_Domain_PRICE,
        .icon = GFX_SkillIcon_Domain,
    },
#endif

#if (defined(SID_FortifyingStaff) && COMMON_SKILL_VALID(SID_FortifyingStaff))
    [SID_FortifyingStaff] = {
        .name = MSG_SKILL_FortifyingStaff_NAME,
        .desc = MSG_SKILL_FortifyingStaff_DESC,
        .capacity = MSG_SKILL_FortifyingStaff_CAPACITY,
        .price = MSG_SKILL_FortifyingStaff_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_StaffGuard) && COMMON_SKILL_VALID(SID_StaffGuard))
    [SID_StaffGuard] = {
        .name = MSG_SKILL_StaffGuard_NAME,
        .desc = MSG_SKILL_StaffGuard_DESC,
        .capacity = MSG_SKILL_StaffGuard_CAPACITY,
        .price = MSG_SKILL_StaffGuard_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GoodyBasket) && COMMON_SKILL_VALID(SID_GoodyBasket))
    [SID_GoodyBasket] = {
        .name = MSG_SKILL_GoodyBasket_NAME,
        .desc = MSG_SKILL_GoodyBasket_DESC,
        .capacity = MSG_SKILL_GoodyBasket_CAPACITY,
        .price = MSG_SKILL_GoodyBasket_PRICE,
        .icon = GFX_SkillIcon_GoodyBasket,
    },
#endif

#if (defined(SID_MakeAKilling) && COMMON_SKILL_VALID(SID_MakeAKilling))
    [SID_MakeAKilling] = {
        .name = MSG_SKILL_MakeAKilling_NAME,
        .desc = MSG_SKILL_MakeAKilling_DESC,
        .capacity = MSG_SKILL_MakeAKilling_CAPACITY,
        .price = MSG_SKILL_MakeAKilling_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_NecroCopy) && COMMON_SKILL_VALID(SID_NecroCopy))
    [SID_NecroCopy] = {
        .name = MSG_SKILL_NecroCopy_NAME,
        .desc = MSG_SKILL_NecroCopy_DESC,
        .capacity = MSG_SKILL_NecroCopy_CAPACITY,
        .price = MSG_SKILL_NecroCopy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SwordProwess) && COMMON_SKILL_VALID(SID_SwordProwess))
    [SID_SwordProwess] = {
        .name = MSG_SKILL_SwordProwess_NAME,
        .desc = MSG_SKILL_SwordProwess_DESC,
        .capacity = MSG_SKILL_SwordProwess_CAPACITY,
        .price = MSG_SKILL_SwordProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LanceProwess) && COMMON_SKILL_VALID(SID_LanceProwess))
    [SID_LanceProwess] = {
        .name = MSG_SKILL_LanceProwess_NAME,
        .desc = MSG_SKILL_LanceProwess_DESC,
        .capacity = MSG_SKILL_LanceProwess_CAPACITY,
        .price = MSG_SKILL_LanceProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AxeProwess) && COMMON_SKILL_VALID(SID_AxeProwess))
    [SID_AxeProwess] = {
        .name = MSG_SKILL_AxeProwess_NAME,
        .desc = MSG_SKILL_AxeProwess_DESC,
        .capacity = MSG_SKILL_AxeProwess_CAPACITY,
        .price = MSG_SKILL_AxeProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BowProwess) && COMMON_SKILL_VALID(SID_BowProwess))
    [SID_BowProwess] = {
        .name = MSG_SKILL_BowProwess_NAME,
        .desc = MSG_SKILL_BowProwess_DESC,
        .capacity = MSG_SKILL_BowProwess_CAPACITY,
        .price = MSG_SKILL_BowProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AnimaProwess) && COMMON_SKILL_VALID(SID_AnimaProwess))
    [SID_AnimaProwess] = {
        .name = MSG_SKILL_AnimaProwess_NAME,
        .desc = MSG_SKILL_AnimaProwess_DESC,
        .capacity = MSG_SKILL_AnimaProwess_CAPACITY,
        .price = MSG_SKILL_AnimaProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DarkProwess) && COMMON_SKILL_VALID(SID_DarkProwess))
    [SID_DarkProwess] = {
        .name = MSG_SKILL_DarkProwess_NAME,
        .desc = MSG_SKILL_DarkProwess_DESC,
        .capacity = MSG_SKILL_DarkProwess_CAPACITY,
        .price = MSG_SKILL_DarkProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LightProwess) && COMMON_SKILL_VALID(SID_LightProwess))
    [SID_LightProwess] = {
        .name = MSG_SKILL_LightProwess_NAME,
        .desc = MSG_SKILL_LightProwess_DESC,
        .capacity = MSG_SKILL_LightProwess_CAPACITY,
        .price = MSG_SKILL_LightProwess_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Break) && COMMON_SKILL_VALID(SID_Break))
    [SID_Break] = {
        .name = MSG_SKILL_Break_NAME,
        .desc = MSG_SKILL_Break_DESC,
        .capacity = MSG_SKILL_Break_CAPACITY,
        .price = MSG_SKILL_Break_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Resurrection) && COMMON_SKILL_VALID(SID_Resurrection))
    [SID_Resurrection] = {
        .name = MSG_SKILL_Resurrection_NAME,
        .desc = MSG_SKILL_Resurrection_DESC,
        .capacity = MSG_SKILL_Resurrection_CAPACITY,
        .price = MSG_SKILL_Resurrection_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Seer) && COMMON_SKILL_VALID(SID_Seer))
    [SID_Seer] = {
        .name = MSG_SKILL_Seer_NAME,
        .desc = MSG_SKILL_Seer_DESC,
        .capacity = MSG_SKILL_Seer_CAPACITY,
        .price = MSG_SKILL_Seer_PRICE,
        .icon = GFX_SkillIcon_Seer,
    },
#endif

#if (defined(SID_Chipper) && COMMON_SKILL_VALID(SID_Chipper))
    [SID_Chipper] = {
        .name = MSG_SKILL_Chipper_NAME,
        .desc = MSG_SKILL_Chipper_DESC,
        .capacity = MSG_SKILL_Chipper_CAPACITY,
        .price = MSG_SKILL_Chipper_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Arise) && COMMON_SKILL_VALID(SID_Arise))
    [SID_Arise] = {
        .name = MSG_SKILL_Arise_NAME,
        .desc = MSG_SKILL_Arise_DESC,
        .capacity = MSG_SKILL_Arise_CAPACITY,
        .price = MSG_SKILL_Arise_PRICE,
        .icon = GFX_SkillIcon_Arise,
    },
#endif

#if (defined(SID_CapacityHalf) && COMMON_SKILL_VALID(SID_CapacityHalf))
    [SID_CapacityHalf] = {
        .name = MSG_SKILL_CapacityHalf_NAME,
        .desc = MSG_SKILL_CapacityHalf_DESC,
        .capacity = MSG_SKILL_CapacityHalf_CAPACITY,
        .price = MSG_SKILL_CapacityHalf_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CapacityOne) && COMMON_SKILL_VALID(SID_CapacityOne))
    [SID_CapacityOne] = {
        .name = MSG_SKILL_CapacityOne_NAME,
        .desc = MSG_SKILL_CapacityOne_DESC,
        .capacity = MSG_SKILL_CapacityOne_CAPACITY,
        .price = MSG_SKILL_CapacityOne_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MercyPlus) && COMMON_SKILL_VALID(SID_MercyPlus))
    [SID_MercyPlus] = {
        .name = MSG_SKILL_MercyPlus_NAME,
        .desc = MSG_SKILL_MercyPlus_DESC,
        .capacity = MSG_SKILL_MercyPlus_CAPACITY,
        .price = MSG_SKILL_MercyPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Prodigy) && COMMON_SKILL_VALID(SID_Prodigy))
    [SID_Prodigy] = {
        .name = MSG_SKILL_Prodigy_NAME,
        .desc = MSG_SKILL_Prodigy_DESC,
        .capacity = MSG_SKILL_Prodigy_CAPACITY,
        .price = MSG_SKILL_Prodigy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DangerRanger) && COMMON_SKILL_VALID(SID_DangerRanger))
    [SID_DangerRanger] = {
        .name = MSG_SKILL_DangerRanger_NAME,
        .desc = MSG_SKILL_DangerRanger_DESC,
        .capacity = MSG_SKILL_DangerRanger_CAPACITY,
        .price = MSG_SKILL_DangerRanger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Savior) && COMMON_SKILL_VALID(SID_Savior))
    [SID_Savior] = {
        .name = MSG_SKILL_Savior_NAME,
        .desc = MSG_SKILL_Savior_DESC,
        .capacity = MSG_SKILL_Savior_CAPACITY,
        .price = MSG_SKILL_Savior_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Toxic) && COMMON_SKILL_VALID(SID_Toxic))
    [SID_Toxic] = {
        .name = MSG_SKILL_Toxic_NAME,
        .desc = MSG_SKILL_Toxic_DESC,
        .capacity = MSG_SKILL_Toxic_CAPACITY,
        .price = MSG_SKILL_Toxic_PRICE,
        .icon = GFX_SkillIcon_Toxic,
    },
#endif

#if (defined(SID_MendArms) && COMMON_SKILL_VALID(SID_MendArms))
    [SID_MendArms] = {
        .name = MSG_SKILL_MendArms_NAME,
        .desc = MSG_SKILL_MendArms_DESC,
        .capacity = MSG_SKILL_MendArms_CAPACITY,
        .price = MSG_SKILL_MendArms_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KillStreak) && COMMON_SKILL_VALID(SID_KillStreak))
    [SID_KillStreak] = {
        .name = MSG_SKILL_KillStreak_NAME,
        .desc = MSG_SKILL_KillStreak_DESC,
        .capacity = MSG_SKILL_KillStreak_CAPACITY,
        .price = MSG_SKILL_KillStreak_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HyperFocus) && COMMON_SKILL_VALID(SID_HyperFocus))
    [SID_HyperFocus] = {
        .name = MSG_SKILL_HyperFocus_NAME,
        .desc = MSG_SKILL_HyperFocus_DESC,
        .capacity = MSG_SKILL_HyperFocus_CAPACITY,
        .price = MSG_SKILL_HyperFocus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerDefense) && COMMON_SKILL_VALID(SID_TakerDefense))
    [SID_TakerDefense] = {
        .name = MSG_SKILL_TakerDefense_NAME,
        .desc = MSG_SKILL_TakerDefense_DESC,
        .capacity = MSG_SKILL_TakerDefense_CAPACITY,
        .price = MSG_SKILL_TakerDefense_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerLuck) && COMMON_SKILL_VALID(SID_TakerLuck))
    [SID_TakerLuck] = {
        .name = MSG_SKILL_TakerLuck_NAME,
        .desc = MSG_SKILL_TakerLuck_DESC,
        .capacity = MSG_SKILL_TakerLuck_CAPACITY,
        .price = MSG_SKILL_TakerLuck_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerMagic) && COMMON_SKILL_VALID(SID_TakerMagic))
    [SID_TakerMagic] = {
        .name = MSG_SKILL_TakerMagic_NAME,
        .desc = MSG_SKILL_TakerMagic_DESC,
        .capacity = MSG_SKILL_TakerMagic_CAPACITY,
        .price = MSG_SKILL_TakerMagic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerMovement) && COMMON_SKILL_VALID(SID_TakerMovement))
    [SID_TakerMovement] = {
        .name = MSG_SKILL_TakerMovement_NAME,
        .desc = MSG_SKILL_TakerMovement_DESC,
        .capacity = MSG_SKILL_TakerMovement_CAPACITY,
        .price = MSG_SKILL_TakerMovement_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerResistance) && COMMON_SKILL_VALID(SID_TakerResistance))
    [SID_TakerResistance] = {
        .name = MSG_SKILL_TakerResistance_NAME,
        .desc = MSG_SKILL_TakerResistance_DESC,
        .capacity = MSG_SKILL_TakerResistance_CAPACITY,
        .price = MSG_SKILL_TakerResistance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerSkill) && COMMON_SKILL_VALID(SID_TakerSkill))
    [SID_TakerSkill] = {
        .name = MSG_SKILL_TakerSkill_NAME,
        .desc = MSG_SKILL_TakerSkill_DESC,
        .capacity = MSG_SKILL_TakerSkill_CAPACITY,
        .price = MSG_SKILL_TakerSkill_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerSpectrum) && COMMON_SKILL_VALID(SID_TakerSpectrum))
    [SID_TakerSpectrum] = {
        .name = MSG_SKILL_TakerSpectrum_NAME,
        .desc = MSG_SKILL_TakerSpectrum_DESC,
        .capacity = MSG_SKILL_TakerSpectrum_CAPACITY,
        .price = MSG_SKILL_TakerSpectrum_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerSpeed) && COMMON_SKILL_VALID(SID_TakerSpeed))
    [SID_TakerSpeed] = {
        .name = MSG_SKILL_TakerSpeed_NAME,
        .desc = MSG_SKILL_TakerSpeed_DESC,
        .capacity = MSG_SKILL_TakerSpeed_CAPACITY,
        .price = MSG_SKILL_TakerSpeed_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TakerStrength) && COMMON_SKILL_VALID(SID_TakerStrength))
    [SID_TakerStrength] = {
        .name = MSG_SKILL_TakerStrength_NAME,
        .desc = MSG_SKILL_TakerStrength_DESC,
        .capacity = MSG_SKILL_TakerStrength_CAPACITY,
        .price = MSG_SKILL_TakerStrength_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SelfHealing) && COMMON_SKILL_VALID(SID_SelfHealing))
    [SID_SelfHealing] = {
        .name = MSG_SKILL_SelfHealing_NAME,
        .desc = MSG_SKILL_SelfHealing_DESC,
        .capacity = MSG_SKILL_SelfHealing_CAPACITY,
        .price = MSG_SKILL_SelfHealing_PRICE,
        .icon = GFX_SkillIcon_SelfHealing,
    },
#endif

#if (defined(SID_IronDome) && COMMON_SKILL_VALID(SID_IronDome))
    [SID_IronDome] = {
        .name = MSG_SKILL_IronDome_NAME,
        .desc = MSG_SKILL_IronDome_DESC,
        .capacity = MSG_SKILL_IronDome_CAPACITY,
        .price = MSG_SKILL_IronDome_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Alacrity) && COMMON_SKILL_VALID(SID_Alacrity))
    [SID_Alacrity] = {
        .name = MSG_SKILL_Alacrity_NAME,
        .desc = MSG_SKILL_Alacrity_DESC,
        .capacity = MSG_SKILL_Alacrity_CAPACITY,
        .price = MSG_SKILL_Alacrity_PRICE,
        .icon = GFX_SkillIcon_Alacrity,
    },
#endif

#if (defined(SID_AlacrityPlus) && COMMON_SKILL_VALID(SID_AlacrityPlus))
    [SID_AlacrityPlus] = {
        .name = MSG_SKILL_AlacrityPlus_NAME,
        .desc = MSG_SKILL_AlacrityPlus_DESC,
        .capacity = MSG_SKILL_AlacrityPlus_CAPACITY,
        .price = MSG_SKILL_AlacrityPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AssignDecoy) && COMMON_SKILL_VALID(SID_AssignDecoy))
    [SID_AssignDecoy] = {
        .name = MSG_SKILL_AssignDecoy_NAME,
        .desc = MSG_SKILL_AssignDecoy_DESC,
        .capacity = MSG_SKILL_AssignDecoy_CAPACITY,
        .price = MSG_SKILL_AssignDecoy_PRICE,
        .icon = GFX_SkillIcon_AssignDecoy,
    },
#endif

#if (defined(SID_AssignDecoyPlus) && COMMON_SKILL_VALID(SID_AssignDecoyPlus))
    [SID_AssignDecoyPlus] = {
        .name = MSG_SKILL_AssignDecoyPlus_NAME,
        .desc = MSG_SKILL_AssignDecoyPlus_DESC,
        .capacity = MSG_SKILL_AssignDecoyPlus_CAPACITY,
        .price = MSG_SKILL_AssignDecoyPlus_PRICE,
        .icon = GFX_SkillIcon_AssignDecoyPlus,
    },
#endif

#if (defined(SID_Canter) && COMMON_SKILL_VALID(SID_Canter))
    [SID_Canter] = {
        .name = MSG_SKILL_Canter_NAME,
        .desc = MSG_SKILL_Canter_DESC,
        .capacity = MSG_SKILL_Canter_CAPACITY,
        .price = MSG_SKILL_Canter_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Echo) && COMMON_SKILL_VALID(SID_Echo))
    [SID_Echo] = {
        .name = MSG_SKILL_Echo_NAME,
        .desc = MSG_SKILL_Echo_DESC,
        .capacity = MSG_SKILL_Echo_CAPACITY,
        .price = MSG_SKILL_Echo_PRICE,
        .icon = GFX_SkillIcon_Echo,
    },
#endif

#if (defined(SID_Guardian) && COMMON_SKILL_VALID(SID_Guardian))
    [SID_Guardian] = {
        .name = MSG_SKILL_Guardian_NAME,
        .desc = MSG_SKILL_Guardian_DESC,
        .capacity = MSG_SKILL_Guardian_CAPACITY,
        .price = MSG_SKILL_Guardian_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ShadowFlash) && COMMON_SKILL_VALID(SID_ShadowFlash))
    [SID_ShadowFlash] = {
        .name = MSG_SKILL_ShadowFlash_NAME,
        .desc = MSG_SKILL_ShadowFlash_DESC,
        .capacity = MSG_SKILL_ShadowFlash_CAPACITY,
        .price = MSG_SKILL_ShadowFlash_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GrazingBlow) && COMMON_SKILL_VALID(SID_GrazingBlow))
    [SID_GrazingBlow] = {
        .name = MSG_SKILL_GrazingBlow_NAME,
        .desc = MSG_SKILL_GrazingBlow_DESC,
        .capacity = MSG_SKILL_GrazingBlow_CAPACITY,
        .price = MSG_SKILL_GrazingBlow_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Counterattack) && COMMON_SKILL_VALID(SID_Counterattack))
    [SID_Counterattack] = {
        .name = MSG_SKILL_Counterattack_NAME,
        .desc = MSG_SKILL_Counterattack_DESC,
        .capacity = MSG_SKILL_Counterattack_CAPACITY,
        .price = MSG_SKILL_Counterattack_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Wrestler) && COMMON_SKILL_VALID(SID_Wrestler))
    [SID_Wrestler] = {
        .name = MSG_SKILL_Wrestler_NAME,
        .desc = MSG_SKILL_Wrestler_DESC,
        .capacity = MSG_SKILL_Wrestler_CAPACITY,
        .price = MSG_SKILL_Wrestler_PRICE,
        .icon = GFX_SkillIcon_Wrestler,
    },
#endif

#if (defined(SID_AbsorbMelee) && COMMON_SKILL_VALID(SID_AbsorbMelee))
    [SID_AbsorbMelee] = {
        .name = MSG_SKILL_AbsorbMelee_NAME,
        .desc = MSG_SKILL_AbsorbMelee_DESC,
        .capacity = MSG_SKILL_AbsorbMelee_CAPACITY,
        .price = MSG_SKILL_AbsorbMelee_PRICE,
        .icon = GFX_SkillIcon_AbsorbMelee,
    },
#endif

#if (defined(SID_AbsorbMagic) && COMMON_SKILL_VALID(SID_AbsorbMagic))
    [SID_AbsorbMagic] = {
        .name = MSG_SKILL_AbsorbMagic_NAME,
        .desc = MSG_SKILL_AbsorbMagic_DESC,
        .capacity = MSG_SKILL_AbsorbMagic_CAPACITY,
        .price = MSG_SKILL_AbsorbMagic_PRICE,
        .icon = GFX_SkillIcon_AbsorbMagic,
    },
#endif

#if (defined(SID_Duel) && COMMON_SKILL_VALID(SID_Duel))
    [SID_Duel] = {
        .name = MSG_SKILL_Duel_NAME,
        .desc = MSG_SKILL_Duel_DESC,
        .capacity = MSG_SKILL_Duel_CAPACITY,
        .price = MSG_SKILL_Duel_PRICE,
        .icon = GFX_SkillIcon_Duel,
    },
#endif

#if (defined(SID_Trader) && COMMON_SKILL_VALID(SID_Trader))
    [SID_Trader] = {
        .name = MSG_SKILL_Trader_NAME,
        .desc = MSG_SKILL_Trader_DESC,
        .capacity = MSG_SKILL_Trader_CAPACITY,
        .price = MSG_SKILL_Trader_PRICE,
        .icon = GFX_SkillIcon_Trader,
    },
#endif

#if (defined(SID_Offhand) && COMMON_SKILL_VALID(SID_Offhand))
    [SID_Offhand] = {
        .name = MSG_SKILL_Offhand_NAME,
        .desc = MSG_SKILL_Offhand_DESC,
        .capacity = MSG_SKILL_Offhand_CAPACITY,
        .price = MSG_SKILL_Offhand_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TwinCrests) && COMMON_SKILL_VALID(SID_TwinCrests))
    [SID_TwinCrests] = {
        .name = MSG_SKILL_TwinCrests_NAME,
        .desc = MSG_SKILL_TwinCrests_DESC,
        .capacity = MSG_SKILL_TwinCrests_CAPACITY,
        .price = MSG_SKILL_TwinCrests_PRICE,
        .icon = GFX_SkillIcon_TwinCrests,
    },
#endif

#if (defined(SID_Flurry) && COMMON_SKILL_VALID(SID_Flurry))
    [SID_Flurry] = {
        .name = MSG_SKILL_Flurry_NAME,
        .desc = MSG_SKILL_Flurry_DESC,
        .capacity = MSG_SKILL_Flurry_CAPACITY,
        .price = MSG_SKILL_Flurry_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Antihex) && COMMON_SKILL_VALID(SID_Antihex))
    [SID_Antihex] = {
        .name = MSG_SKILL_Antihex_NAME,
        .desc = MSG_SKILL_Antihex_DESC,
        .capacity = MSG_SKILL_Antihex_CAPACITY,
        .price = MSG_SKILL_Antihex_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Equalizer) && COMMON_SKILL_VALID(SID_Equalizer))
    [SID_Equalizer] = {
        .name = MSG_SKILL_Equalizer_NAME,
        .desc = MSG_SKILL_Equalizer_DESC,
        .capacity = MSG_SKILL_Equalizer_CAPACITY,
        .price = MSG_SKILL_Equalizer_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Perch) && COMMON_SKILL_VALID(SID_Perch))
    [SID_Perch] = {
        .name = MSG_SKILL_Perch_NAME,
        .desc = MSG_SKILL_Perch_DESC,
        .capacity = MSG_SKILL_Perch_CAPACITY,
        .price = MSG_SKILL_Perch_PRICE,
        .icon = GFX_SkillIcon_Perch,
    },
#endif

#if (defined(SID_MountainMan) && COMMON_SKILL_VALID(SID_MountainMan))
    [SID_MountainMan] = {
        .name = MSG_SKILL_MountainMan_NAME,
        .desc = MSG_SKILL_MountainMan_DESC,
        .capacity = MSG_SKILL_MountainMan_CAPACITY,
        .price = MSG_SKILL_MountainMan_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Saint) && COMMON_SKILL_VALID(SID_Saint))
    [SID_Saint] = {
        .name = MSG_SKILL_Saint_NAME,
        .desc = MSG_SKILL_Saint_DESC,
        .capacity = MSG_SKILL_Saint_CAPACITY,
        .price = MSG_SKILL_Saint_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AidRefusal) && COMMON_SKILL_VALID(SID_AidRefusal))
    [SID_AidRefusal] = {
        .name = MSG_SKILL_AidRefusal_NAME,
        .desc = MSG_SKILL_AidRefusal_DESC,
        .capacity = MSG_SKILL_AidRefusal_CAPACITY,
        .price = MSG_SKILL_AidRefusal_PRICE,
        .icon = GFX_SkillIcon_AidRefusal,
    },
#endif

#if (defined(SID_WhiteMage) && COMMON_SKILL_VALID(SID_WhiteMage))
    [SID_WhiteMage] = {
        .name = MSG_SKILL_WhiteMage_NAME,
        .desc = MSG_SKILL_WhiteMage_DESC,
        .capacity = MSG_SKILL_WhiteMage_CAPACITY,
        .price = MSG_SKILL_WhiteMage_PRICE,
        .icon = GFX_SkillIcon_WhiteMage,
    },
#endif

#if (defined(SID_GuerillaTactics) && COMMON_SKILL_VALID(SID_GuerillaTactics))
    [SID_GuerillaTactics] = {
        .name = MSG_SKILL_GuerillaTactics_NAME,
        .desc = MSG_SKILL_GuerillaTactics_DESC,
        .capacity = MSG_SKILL_GuerillaTactics_CAPACITY,
        .price = MSG_SKILL_GuerillaTactics_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MoreForMe) && COMMON_SKILL_VALID(SID_MoreForMe))
    [SID_MoreForMe] = {
        .name = MSG_SKILL_MoreForMe_NAME,
        .desc = MSG_SKILL_MoreForMe_DESC,
        .capacity = MSG_SKILL_MoreForMe_CAPACITY,
        .price = MSG_SKILL_MoreForMe_PRICE,
        .icon = GFX_SkillIcon_MoreForMe,
    },
#endif

#if (defined(SID_Mimicry) && COMMON_SKILL_VALID(SID_Mimicry))
    [SID_Mimicry] = {
        .name = MSG_SKILL_Mimicry_NAME,
        .desc = MSG_SKILL_Mimicry_DESC,
        .capacity = MSG_SKILL_Mimicry_CAPACITY,
        .price = MSG_SKILL_Mimicry_PRICE,
        .icon = GFX_SkillIcon_Mimicry,
    },
#endif

#if (defined(SID_ShedSkin) && COMMON_SKILL_VALID(SID_ShedSkin))
    [SID_ShedSkin] = {
        .name = MSG_SKILL_ShedSkin_NAME,
        .desc = MSG_SKILL_ShedSkin_DESC,
        .capacity = MSG_SKILL_ShedSkin_CAPACITY,
        .price = MSG_SKILL_ShedSkin_PRICE,
        .icon = GFX_SkillIcon_ShedSkin,
    },
#endif

#if (defined(SID_PhaseShift) && COMMON_SKILL_VALID(SID_PhaseShift))
    [SID_PhaseShift] = {
        .name = MSG_SKILL_PhaseShift_NAME,
        .desc = MSG_SKILL_PhaseShift_DESC,
        .capacity = MSG_SKILL_PhaseShift_CAPACITY,
        .price = MSG_SKILL_PhaseShift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CursedHeal) && COMMON_SKILL_VALID(SID_CursedHeal))
    [SID_CursedHeal] = {
        .name = MSG_SKILL_CursedHeal_NAME,
        .desc = MSG_SKILL_CursedHeal_DESC,
        .capacity = MSG_SKILL_CursedHeal_CAPACITY,
        .price = MSG_SKILL_CursedHeal_PRICE,
        .icon = GFX_SkillIcon_CursedHeal,
    },
#endif

#if (defined(SID_Decadon) && COMMON_SKILL_VALID(SID_Decadon))
    [SID_Decadon] = {
        .name = MSG_SKILL_Decadon_NAME,
        .desc = MSG_SKILL_Decadon_DESC,
        .capacity = MSG_SKILL_Decadon_CAPACITY,
        .price = MSG_SKILL_Decadon_PRICE,
        .icon = GFX_SkillIcon_Decadon,
    },
#endif

#if (defined(SID_Camouflage) && COMMON_SKILL_VALID(SID_Camouflage))
    [SID_Camouflage] = {
        .name = MSG_SKILL_Camouflage_NAME,
        .desc = MSG_SKILL_Camouflage_DESC,
        .capacity = MSG_SKILL_Camouflage_CAPACITY,
        .price = MSG_SKILL_Camouflage_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Warpath) && COMMON_SKILL_VALID(SID_Warpath))
    [SID_Warpath] = {
        .name = MSG_SKILL_Warpath_NAME,
        .desc = MSG_SKILL_Warpath_DESC,
        .capacity = MSG_SKILL_Warpath_CAPACITY,
        .price = MSG_SKILL_Warpath_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SwiftAttack) && COMMON_SKILL_VALID(SID_SwiftAttack))
    [SID_SwiftAttack] = {
        .name = MSG_SKILL_SwiftAttack_NAME,
        .desc = MSG_SKILL_SwiftAttack_DESC,
        .capacity = MSG_SKILL_SwiftAttack_CAPACITY,
        .price = MSG_SKILL_SwiftAttack_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BlacksmithPlus) && COMMON_SKILL_VALID(SID_BlacksmithPlus))
    [SID_BlacksmithPlus] = {
        .name = MSG_SKILL_BlacksmithPlus_NAME,
        .desc = MSG_SKILL_BlacksmithPlus_DESC,
        .capacity = MSG_SKILL_BlacksmithPlus_CAPACITY,
        .price = MSG_SKILL_BlacksmithPlus_PRICE,
        .icon = GFX_SkillIcon_BlacksmithPlus,
    },
#endif

#if (defined(SID_DeathBlight) && COMMON_SKILL_VALID(SID_DeathBlight))
    [SID_DeathBlight] = {
        .name = MSG_SKILL_DeathBlight_NAME,
        .desc = MSG_SKILL_DeathBlight_DESC,
        .capacity = MSG_SKILL_DeathBlight_CAPACITY,
        .price = MSG_SKILL_DeathBlight_PRICE,
        .icon = GFX_SkillIcon_DeathBlight,
    },
#endif

#if (defined(SID_Trace) && COMMON_SKILL_VALID(SID_Trace))
    [SID_Trace] = {
        .name = MSG_SKILL_Trace_NAME,
        .desc = MSG_SKILL_Trace_DESC,
        .capacity = MSG_SKILL_Trace_CAPACITY,
        .price = MSG_SKILL_Trace_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Scavenger) && COMMON_SKILL_VALID(SID_Scavenger))
    [SID_Scavenger] = {
        .name = MSG_SKILL_Scavenger_NAME,
        .desc = MSG_SKILL_Scavenger_DESC,
        .capacity = MSG_SKILL_Scavenger_CAPACITY,
        .price = MSG_SKILL_Scavenger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Upgrade) && COMMON_SKILL_VALID(SID_Upgrade))
    [SID_Upgrade] = {
        .name = MSG_SKILL_Upgrade_NAME,
        .desc = MSG_SKILL_Upgrade_DESC,
        .capacity = MSG_SKILL_Upgrade_CAPACITY,
        .price = MSG_SKILL_Upgrade_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Shotput) && COMMON_SKILL_VALID(SID_Shotput))
    [SID_Shotput] = {
        .name = MSG_SKILL_Shotput_NAME,
        .desc = MSG_SKILL_Shotput_DESC,
        .capacity = MSG_SKILL_Shotput_CAPACITY,
        .price = MSG_SKILL_Shotput_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ShotputPlus) && COMMON_SKILL_VALID(SID_ShotputPlus))
    [SID_ShotputPlus] = {
        .name = MSG_SKILL_ShotputPlus_NAME,
        .desc = MSG_SKILL_ShotputPlus_DESC,
        .capacity = MSG_SKILL_ShotputPlus_CAPACITY,
        .price = MSG_SKILL_ShotputPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Extrovert) && COMMON_SKILL_VALID(SID_Extrovert))
    [SID_Extrovert] = {
        .name = MSG_SKILL_Extrovert_NAME,
        .desc = MSG_SKILL_Extrovert_DESC,
        .capacity = MSG_SKILL_Extrovert_CAPACITY,
        .price = MSG_SKILL_Extrovert_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Transpose) && COMMON_SKILL_VALID(SID_Transpose))
    [SID_Transpose] = {
        .name = MSG_SKILL_Transpose_NAME,
        .desc = MSG_SKILL_Transpose_DESC,
        .capacity = MSG_SKILL_Transpose_CAPACITY,
        .price = MSG_SKILL_Transpose_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Lifefont) && COMMON_SKILL_VALID(SID_Lifefont))
    [SID_Lifefont] = {
        .name = MSG_SKILL_Lifefont_NAME,
        .desc = MSG_SKILL_Lifefont_DESC,
        .capacity = MSG_SKILL_Lifefont_CAPACITY,
        .price = MSG_SKILL_Lifefont_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DivineReprieve) && COMMON_SKILL_VALID(SID_DivineReprieve))
    [SID_DivineReprieve] = {
        .name = MSG_SKILL_DivineReprieve_NAME,
        .desc = MSG_SKILL_DivineReprieve_DESC,
        .capacity = MSG_SKILL_DivineReprieve_CAPACITY,
        .price = MSG_SKILL_DivineReprieve_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EternalVanity) && COMMON_SKILL_VALID(SID_EternalVanity))
    [SID_EternalVanity] = {
        .name = MSG_SKILL_EternalVanity_NAME,
        .desc = MSG_SKILL_EternalVanity_DESC,
        .capacity = MSG_SKILL_EternalVanity_CAPACITY,
        .price = MSG_SKILL_EternalVanity_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackStrength) && COMMON_SKILL_VALID(SID_KnackStrength))
    [SID_KnackStrength] = {
        .name = MSG_SKILL_KnackStrength_NAME,
        .desc = MSG_SKILL_KnackStrength_DESC,
        .capacity = MSG_SKILL_KnackStrength_CAPACITY,
        .price = MSG_SKILL_KnackStrength_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackMagic) && COMMON_SKILL_VALID(SID_KnackMagic))
    [SID_KnackMagic] = {
        .name = MSG_SKILL_KnackMagic_NAME,
        .desc = MSG_SKILL_KnackMagic_DESC,
        .capacity = MSG_SKILL_KnackMagic_CAPACITY,
        .price = MSG_SKILL_KnackMagic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackSkill) && COMMON_SKILL_VALID(SID_KnackSkill))
    [SID_KnackSkill] = {
        .name = MSG_SKILL_KnackSkill_NAME,
        .desc = MSG_SKILL_KnackSkill_DESC,
        .capacity = MSG_SKILL_KnackSkill_CAPACITY,
        .price = MSG_SKILL_KnackSkill_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackSpeed) && COMMON_SKILL_VALID(SID_KnackSpeed))
    [SID_KnackSpeed] = {
        .name = MSG_SKILL_KnackSpeed_NAME,
        .desc = MSG_SKILL_KnackSpeed_DESC,
        .capacity = MSG_SKILL_KnackSpeed_CAPACITY,
        .price = MSG_SKILL_KnackSpeed_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackLuck) && COMMON_SKILL_VALID(SID_KnackLuck))
    [SID_KnackLuck] = {
        .name = MSG_SKILL_KnackLuck_NAME,
        .desc = MSG_SKILL_KnackLuck_DESC,
        .capacity = MSG_SKILL_KnackLuck_CAPACITY,
        .price = MSG_SKILL_KnackLuck_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackDefense) && COMMON_SKILL_VALID(SID_KnackDefense))
    [SID_KnackDefense] = {
        .name = MSG_SKILL_KnackDefense_NAME,
        .desc = MSG_SKILL_KnackDefense_DESC,
        .capacity = MSG_SKILL_KnackDefense_CAPACITY,
        .price = MSG_SKILL_KnackDefense_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackResistance) && COMMON_SKILL_VALID(SID_KnackResistance))
    [SID_KnackResistance] = {
        .name = MSG_SKILL_KnackResistance_NAME,
        .desc = MSG_SKILL_KnackResistance_DESC,
        .capacity = MSG_SKILL_KnackResistance_CAPACITY,
        .price = MSG_SKILL_KnackResistance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_KnackHealth) && COMMON_SKILL_VALID(SID_KnackHealth))
    [SID_KnackHealth] = {
        .name = MSG_SKILL_KnackHealth_NAME,
        .desc = MSG_SKILL_KnackHealth_DESC,
        .capacity = MSG_SKILL_KnackHealth_CAPACITY,
        .price = MSG_SKILL_KnackHealth_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ScrollSavant) && COMMON_SKILL_VALID(SID_ScrollSavant))
    [SID_ScrollSavant] = {
        .name = MSG_SKILL_ScrollSavant_NAME,
        .desc = MSG_SKILL_ScrollSavant_DESC,
        .capacity = MSG_SKILL_ScrollSavant_CAPACITY,
        .price = MSG_SKILL_ScrollSavant_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SwordSaint) && COMMON_SKILL_VALID(SID_SwordSaint))
    [SID_SwordSaint] = {
        .name = MSG_SKILL_SwordSaint_NAME,
        .desc = MSG_SKILL_SwordSaint_DESC,
        .capacity = MSG_SKILL_SwordSaint_CAPACITY,
        .price = MSG_SKILL_SwordSaint_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SteadyRider) && COMMON_SKILL_VALID(SID_SteadyRider))
    [SID_SteadyRider] = {
        .name = MSG_SKILL_SteadyRider_NAME,
        .desc = MSG_SKILL_SteadyRider_DESC,
        .capacity = MSG_SKILL_SteadyRider_CAPACITY,
        .price = MSG_SKILL_SteadyRider_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SteadyRiderPlus) && COMMON_SKILL_VALID(SID_SteadyRiderPlus))
    [SID_SteadyRiderPlus] = {
        .name = MSG_SKILL_SteadyRiderPlus_NAME,
        .desc = MSG_SKILL_SteadyRiderPlus_DESC,
        .capacity = MSG_SKILL_SteadyRiderPlus_CAPACITY,
        .price = MSG_SKILL_SteadyRiderPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_WyvernCrash) && COMMON_SKILL_VALID(SID_WyvernCrash))
    [SID_WyvernCrash] = {
        .name = MSG_SKILL_WyvernCrash_NAME,
        .desc = MSG_SKILL_WyvernCrash_DESC,
        .capacity = MSG_SKILL_WyvernCrash_CAPACITY,
        .price = MSG_SKILL_WyvernCrash_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ExplosiveHeal) && COMMON_SKILL_VALID(SID_ExplosiveHeal))
    [SID_ExplosiveHeal] = {
        .name = MSG_SKILL_ExplosiveHeal_NAME,
        .desc = MSG_SKILL_ExplosiveHeal_DESC,
        .capacity = MSG_SKILL_ExplosiveHeal_CAPACITY,
        .price = MSG_SKILL_ExplosiveHeal_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BanditBane) && COMMON_SKILL_VALID(SID_BanditBane))
    [SID_BanditBane] = {
        .name = MSG_SKILL_BanditBane_NAME,
        .desc = MSG_SKILL_BanditBane_DESC,
        .capacity = MSG_SKILL_BanditBane_CAPACITY,
        .price = MSG_SKILL_BanditBane_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ExplosiveBuff) && COMMON_SKILL_VALID(SID_ExplosiveBuff))
    [SID_ExplosiveBuff] = {
        .name = MSG_SKILL_ExplosiveBuff_NAME,
        .desc = MSG_SKILL_ExplosiveBuff_DESC,
        .capacity = MSG_SKILL_ExplosiveBuff_CAPACITY,
        .price = MSG_SKILL_ExplosiveBuff_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Sellsword) && COMMON_SKILL_VALID(SID_Sellsword))
    [SID_Sellsword] = {
        .name = MSG_SKILL_Sellsword_NAME,
        .desc = MSG_SKILL_Sellsword_DESC,
        .capacity = MSG_SKILL_Sellsword_CAPACITY,
        .price = MSG_SKILL_Sellsword_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PeoplesKnight) && COMMON_SKILL_VALID(SID_PeoplesKnight))
    [SID_PeoplesKnight] = {
        .name = MSG_SKILL_PeoplesKnight_NAME,
        .desc = MSG_SKILL_PeoplesKnight_DESC,
        .capacity = MSG_SKILL_PeoplesKnight_CAPACITY,
        .price = MSG_SKILL_PeoplesKnight_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Predation) && COMMON_SKILL_VALID(SID_Predation))
    [SID_Predation] = {
        .name = MSG_SKILL_Predation_NAME,
        .desc = MSG_SKILL_Predation_DESC,
        .capacity = MSG_SKILL_Predation_CAPACITY,
        .price = MSG_SKILL_Predation_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PredationPlus) && COMMON_SKILL_VALID(SID_PredationPlus))
    [SID_PredationPlus] = {
        .name = MSG_SKILL_PredationPlus_NAME,
        .desc = MSG_SKILL_PredationPlus_DESC,
        .capacity = MSG_SKILL_PredationPlus_CAPACITY,
        .price = MSG_SKILL_PredationPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ScrollScribe) && COMMON_SKILL_VALID(SID_ScrollScribe))
    [SID_ScrollScribe] = {
        .name = MSG_SKILL_ScrollScribe_NAME,
        .desc = MSG_SKILL_ScrollScribe_DESC,
        .capacity = MSG_SKILL_ScrollScribe_CAPACITY,
        .price = MSG_SKILL_ScrollScribe_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SpiritedSteps) && COMMON_SKILL_VALID(SID_SpiritedSteps))
    [SID_SpiritedSteps] = {
        .name = MSG_SKILL_SpiritedSteps_NAME,
        .desc = MSG_SKILL_SpiritedSteps_DESC,
        .capacity = MSG_SKILL_SpiritedSteps_CAPACITY,
        .price = MSG_SKILL_SpiritedSteps_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Switcher) && COMMON_SKILL_VALID(SID_Switcher))
    [SID_Switcher] = {
        .name = MSG_SKILL_Switcher_NAME,
        .desc = MSG_SKILL_Switcher_DESC,
        .capacity = MSG_SKILL_Switcher_CAPACITY,
        .price = MSG_SKILL_Switcher_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DesertTiger) && COMMON_SKILL_VALID(SID_DesertTiger))
    [SID_DesertTiger] = {
        .name = MSG_SKILL_DesertTiger_NAME,
        .desc = MSG_SKILL_DesertTiger_DESC,
        .capacity = MSG_SKILL_DesertTiger_CAPACITY,
        .price = MSG_SKILL_DesertTiger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GracefulWielder) && COMMON_SKILL_VALID(SID_GracefulWielder))
    [SID_GracefulWielder] = {
        .name = MSG_SKILL_GracefulWielder_NAME,
        .desc = MSG_SKILL_GracefulWielder_DESC,
        .capacity = MSG_SKILL_GracefulWielder_CAPACITY,
        .price = MSG_SKILL_GracefulWielder_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SummonPlus) && COMMON_SKILL_VALID(SID_SummonPlus))
    [SID_SummonPlus] = {
        .name = MSG_SKILL_SummonPlus_NAME,
        .desc = MSG_SKILL_SummonPlus_DESC,
        .capacity = MSG_SKILL_SummonPlus_CAPACITY,
        .price = MSG_SKILL_SummonPlus_PRICE,
        .icon = GFX_SkillIcon_SummonPlus,
    },
#endif

#if (defined(SID_LethalityPlus) && COMMON_SKILL_VALID(SID_LethalityPlus))
    [SID_LethalityPlus] = {
        .name = MSG_SKILL_LethalityPlus_NAME,
        .desc = MSG_SKILL_LethalityPlus_DESC,
        .capacity = MSG_SKILL_LethalityPlus_CAPACITY,
        .price = MSG_SKILL_LethalityPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Heroics) && COMMON_SKILL_VALID(SID_Heroics))
    [SID_Heroics] = {
        .name = MSG_SKILL_Heroics_NAME,
        .desc = MSG_SKILL_Heroics_DESC,
        .capacity = MSG_SKILL_Heroics_CAPACITY,
        .price = MSG_SKILL_Heroics_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FarWard) && COMMON_SKILL_VALID(SID_FarWard))
    [SID_FarWard] = {
        .name = MSG_SKILL_FarWard_NAME,
        .desc = MSG_SKILL_FarWard_DESC,
        .capacity = MSG_SKILL_FarWard_CAPACITY,
        .price = MSG_SKILL_FarWard_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GrimReaper) && COMMON_SKILL_VALID(SID_GrimReaper))
    [SID_GrimReaper] = {
        .name = MSG_SKILL_GrimReaper_NAME,
        .desc = MSG_SKILL_GrimReaper_DESC,
        .capacity = MSG_SKILL_GrimReaper_CAPACITY,
        .price = MSG_SKILL_GrimReaper_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Hide) && COMMON_SKILL_VALID(SID_Hide))
    [SID_Hide] = {
        .name = MSG_SKILL_Hide_NAME,
        .desc = MSG_SKILL_Hide_DESC,
        .capacity = MSG_SKILL_Hide_CAPACITY,
        .price = MSG_SKILL_Hide_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ShadowStep) && COMMON_SKILL_VALID(SID_ShadowStep))
    [SID_ShadowStep] = {
        .name = MSG_SKILL_ShadowStep_NAME,
        .desc = MSG_SKILL_ShadowStep_DESC,
        .capacity = MSG_SKILL_ShadowStep_CAPACITY,
        .price = MSG_SKILL_ShadowStep_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Zen) && COMMON_SKILL_VALID(SID_Zen))
    [SID_Zen] = {
        .name = MSG_SKILL_Zen_NAME,
        .desc = MSG_SKILL_Zen_DESC,
        .capacity = MSG_SKILL_Zen_CAPACITY,
        .price = MSG_SKILL_Zen_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HugePower) && COMMON_SKILL_VALID(SID_HugePower))
    [SID_HugePower] = {
        .name = MSG_SKILL_HugePower_NAME,
        .desc = MSG_SKILL_HugePower_DESC,
        .capacity = MSG_SKILL_HugePower_CAPACITY,
        .price = MSG_SKILL_HugePower_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HugePowerPlus) && COMMON_SKILL_VALID(SID_HugePowerPlus))
    [SID_HugePowerPlus] = {
        .name = MSG_SKILL_HugePowerPlus_NAME,
        .desc = MSG_SKILL_HugePowerPlus_DESC,
        .capacity = MSG_SKILL_HugePowerPlus_CAPACITY,
        .price = MSG_SKILL_HugePowerPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TintedLens) && COMMON_SKILL_VALID(SID_TintedLens))
    [SID_TintedLens] = {
        .name = MSG_SKILL_TintedLens_NAME,
        .desc = MSG_SKILL_TintedLens_DESC,
        .capacity = MSG_SKILL_TintedLens_CAPACITY,
        .price = MSG_SKILL_TintedLens_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TintedLensPlus) && COMMON_SKILL_VALID(SID_TintedLensPlus))
    [SID_TintedLensPlus] = {
        .name = MSG_SKILL_TintedLensPlus_NAME,
        .desc = MSG_SKILL_TintedLensPlus_DESC,
        .capacity = MSG_SKILL_TintedLensPlus_CAPACITY,
        .price = MSG_SKILL_TintedLensPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TrickRoom) && COMMON_SKILL_VALID(SID_TrickRoom))
    [SID_TrickRoom] = {
        .name = MSG_SKILL_TrickRoom_NAME,
        .desc = MSG_SKILL_TrickRoom_DESC,
        .capacity = MSG_SKILL_TrickRoom_CAPACITY,
        .price = MSG_SKILL_TrickRoom_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Emulate) && COMMON_SKILL_VALID(SID_Emulate))
    [SID_Emulate] = {
        .name = MSG_SKILL_Emulate_NAME,
        .desc = MSG_SKILL_Emulate_DESC,
        .capacity = MSG_SKILL_Emulate_CAPACITY,
        .price = MSG_SKILL_Emulate_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ItemLore) && COMMON_SKILL_VALID(SID_ItemLore))
    [SID_ItemLore] = {
        .name = MSG_SKILL_ItemLore_NAME,
        .desc = MSG_SKILL_ItemLore_DESC,
        .capacity = MSG_SKILL_ItemLore_CAPACITY,
        .price = MSG_SKILL_ItemLore_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DualWield) && COMMON_SKILL_VALID(SID_DualWield))
    [SID_DualWield] = {
        .name = MSG_SKILL_DualWield_NAME,
        .desc = MSG_SKILL_DualWield_DESC,
        .capacity = MSG_SKILL_DualWield_CAPACITY,
        .price = MSG_SKILL_DualWield_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DualWieldPlus) && COMMON_SKILL_VALID(SID_DualWieldPlus))
    [SID_DualWieldPlus] = {
        .name = MSG_SKILL_DualWieldPlus_NAME,
        .desc = MSG_SKILL_DualWieldPlus_DESC,
        .capacity = MSG_SKILL_DualWieldPlus_CAPACITY,
        .price = MSG_SKILL_DualWieldPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Ruinator) && COMMON_SKILL_VALID(SID_Ruinator))
    [SID_Ruinator] = {
        .name = MSG_SKILL_Ruinator_NAME,
        .desc = MSG_SKILL_Ruinator_DESC,
        .capacity = MSG_SKILL_Ruinator_CAPACITY,
        .price = MSG_SKILL_Ruinator_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MarkOfTheBeast) && COMMON_SKILL_VALID(SID_MarkOfTheBeast))
    [SID_MarkOfTheBeast] = {
        .name = MSG_SKILL_MarkOfTheBeast_NAME,
        .desc = MSG_SKILL_MarkOfTheBeast_DESC,
        .capacity = MSG_SKILL_MarkOfTheBeast_CAPACITY,
        .price = MSG_SKILL_MarkOfTheBeast_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Rampart) && COMMON_SKILL_VALID(SID_Rampart))
    [SID_Rampart] = {
        .name = MSG_SKILL_Rampart_NAME,
        .desc = MSG_SKILL_Rampart_DESC,
        .capacity = MSG_SKILL_Rampart_CAPACITY,
        .price = MSG_SKILL_Rampart_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RampartPlus) && COMMON_SKILL_VALID(SID_RampartPlus))
    [SID_RampartPlus] = {
        .name = MSG_SKILL_RampartPlus_NAME,
        .desc = MSG_SKILL_RampartPlus_DESC,
        .capacity = MSG_SKILL_RampartPlus_CAPACITY,
        .price = MSG_SKILL_RampartPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SlowStart) && COMMON_SKILL_VALID(SID_SlowStart))
    [SID_SlowStart] = {
        .name = MSG_SKILL_SlowStart_NAME,
        .desc = MSG_SKILL_SlowStart_DESC,
        .capacity = MSG_SKILL_SlowStart_CAPACITY,
        .price = MSG_SKILL_SlowStart_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Truant) && COMMON_SKILL_VALID(SID_Truant))
    [SID_Truant] = {
        .name = MSG_SKILL_Truant_NAME,
        .desc = MSG_SKILL_Truant_DESC,
        .capacity = MSG_SKILL_Truant_CAPACITY,
        .price = MSG_SKILL_Truant_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Zenkai) && COMMON_SKILL_VALID(SID_Zenkai))
    [SID_Zenkai] = {
        .name = MSG_SKILL_Zenkai_NAME,
        .desc = MSG_SKILL_Zenkai_DESC,
        .capacity = MSG_SKILL_Zenkai_CAPACITY,
        .price = MSG_SKILL_Zenkai_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Salve) && COMMON_SKILL_VALID(SID_Salve))
    [SID_Salve] = {
        .name = MSG_SKILL_Salve_NAME,
        .desc = MSG_SKILL_Salve_DESC,
        .capacity = MSG_SKILL_Salve_CAPACITY,
        .price = MSG_SKILL_Salve_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Forcefield) && COMMON_SKILL_VALID(SID_Forcefield))
    [SID_Forcefield] = {
        .name = MSG_SKILL_Forcefield_NAME,
        .desc = MSG_SKILL_Forcefield_DESC,
        .capacity = MSG_SKILL_Forcefield_CAPACITY,
        .price = MSG_SKILL_Forcefield_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SkillSwap) && COMMON_SKILL_VALID(SID_SkillSwap))
    [SID_SkillSwap] = {
        .name = MSG_SKILL_SkillSwap_NAME,
        .desc = MSG_SKILL_SkillSwap_DESC,
        .capacity = MSG_SKILL_SkillSwap_CAPACITY,
        .price = MSG_SKILL_SkillSwap_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Shuffle) && COMMON_SKILL_VALID(SID_Shuffle))
    [SID_Shuffle] = {
        .name = MSG_SKILL_Shuffle_NAME,
        .desc = MSG_SKILL_Shuffle_DESC,
        .capacity = MSG_SKILL_Shuffle_CAPACITY,
        .price = MSG_SKILL_Shuffle_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ShufflePlus) && COMMON_SKILL_VALID(SID_ShufflePlus))
    [SID_ShufflePlus] = {
        .name = MSG_SKILL_ShufflePlus_NAME,
        .desc = MSG_SKILL_ShufflePlus_DESC,
        .capacity = MSG_SKILL_ShufflePlus_CAPACITY,
        .price = MSG_SKILL_ShufflePlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Persuade) && COMMON_SKILL_VALID(SID_Persuade))
    [SID_Persuade] = {
        .name = MSG_SKILL_Persuade_NAME,
        .desc = MSG_SKILL_Persuade_DESC,
        .capacity = MSG_SKILL_Persuade_CAPACITY,
        .price = MSG_SKILL_Persuade_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PersuadePlus) && COMMON_SKILL_VALID(SID_PersuadePlus))
    [SID_PersuadePlus] = {
        .name = MSG_SKILL_PersuadePlus_NAME,
        .desc = MSG_SKILL_PersuadePlus_DESC,
        .capacity = MSG_SKILL_PersuadePlus_CAPACITY,
        .price = MSG_SKILL_PersuadePlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Kamikaze) && COMMON_SKILL_VALID(SID_Kamikaze))
    [SID_Kamikaze] = {
        .name = MSG_SKILL_Kamikaze_NAME,
        .desc = MSG_SKILL_Kamikaze_DESC,
        .capacity = MSG_SKILL_Kamikaze_CAPACITY,
        .price = MSG_SKILL_Kamikaze_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Freelancer) && COMMON_SKILL_VALID(SID_Freelancer))
    [SID_Freelancer] = {
        .name = MSG_SKILL_Freelancer_NAME,
        .desc = MSG_SKILL_Freelancer_DESC,
        .capacity = MSG_SKILL_Freelancer_CAPACITY,
        .price = MSG_SKILL_Freelancer_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Reinforcements) && COMMON_SKILL_VALID(SID_Reinforcements))
    [SID_Reinforcements] = {
        .name = MSG_SKILL_Reinforcements_NAME,
        .desc = MSG_SKILL_Reinforcements_DESC,
        .capacity = MSG_SKILL_Reinforcements_CAPACITY,
        .price = MSG_SKILL_Reinforcements_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ResolvedHeart) && COMMON_SKILL_VALID(SID_ResolvedHeart))
    [SID_ResolvedHeart] = {
        .name = MSG_SKILL_ResolvedHeart_NAME,
        .desc = MSG_SKILL_ResolvedHeart_DESC,
        .capacity = MSG_SKILL_ResolvedHeart_CAPACITY,
        .price = MSG_SKILL_ResolvedHeart_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Ballistary) && COMMON_SKILL_VALID(SID_Ballistary))
    [SID_Ballistary] = {
        .name = MSG_SKILL_Ballistary_NAME,
        .desc = MSG_SKILL_Ballistary_DESC,
        .capacity = MSG_SKILL_Ballistary_CAPACITY,
        .price = MSG_SKILL_Ballistary_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Bide) && COMMON_SKILL_VALID(SID_Bide))
    [SID_Bide] = {
        .name = MSG_SKILL_Bide_NAME,
        .desc = MSG_SKILL_Bide_DESC,
        .capacity = MSG_SKILL_Bide_CAPACITY,
        .price = MSG_SKILL_Bide_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CoinFlip) && COMMON_SKILL_VALID(SID_CoinFlip))
    [SID_CoinFlip] = {
        .name = MSG_SKILL_CoinFlip_NAME,
        .desc = MSG_SKILL_CoinFlip_DESC,
        .capacity = MSG_SKILL_CoinFlip_CAPACITY,
        .price = MSG_SKILL_CoinFlip_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LastStand) && COMMON_SKILL_VALID(SID_LastStand))
    [SID_LastStand] = {
        .name = MSG_SKILL_LastStand_NAME,
        .desc = MSG_SKILL_LastStand_DESC,
        .capacity = MSG_SKILL_LastStand_CAPACITY,
        .price = MSG_SKILL_LastStand_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif


#if (defined(SID_Flicker) && COMMON_SKILL_VALID(SID_Flicker))
    [SID_Flicker] = {
        .name = MSG_SKILL_Flicker_NAME,
        .desc = MSG_SKILL_Flicker_DESC,
        .capacity = MSG_SKILL_Flicker_CAPACITY,
        .price = MSG_SKILL_Flicker_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Rearm) && COMMON_SKILL_VALID(SID_Rearm))
    [SID_Rearm] = {
        .name = MSG_SKILL_Rearm_NAME,
        .desc = MSG_SKILL_Rearm_DESC,
        .capacity = MSG_SKILL_Rearm_CAPACITY,
        .price = MSG_SKILL_Rearm_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_LightningRod) && COMMON_SKILL_VALID(SID_LightningRod))
    [SID_LightningRod] = {
        .name = MSG_SKILL_LightningRod_NAME,
        .desc = MSG_SKILL_LightningRod_DESC,
        .capacity = MSG_SKILL_LightningRod_CAPACITY,
        .price = MSG_SKILL_LightningRod_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Geomancy) && COMMON_SKILL_VALID(SID_Geomancy))
    [SID_Geomancy] = {
        .name = MSG_SKILL_Geomancy_NAME,
        .desc = MSG_SKILL_Geomancy_DESC,
        .capacity = MSG_SKILL_Geomancy_CAPACITY,
        .price = MSG_SKILL_Geomancy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Pulse) && COMMON_SKILL_VALID(SID_Pulse))
    [SID_Pulse] = {
        .name = MSG_SKILL_Pulse_NAME,
        .desc = MSG_SKILL_Pulse_DESC,
        .capacity = MSG_SKILL_Pulse_CAPACITY,
        .price = MSG_SKILL_Pulse_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FormShift) && COMMON_SKILL_VALID(SID_FormShift))
    [SID_FormShift] = {
        .name = MSG_SKILL_FormShift_NAME,
        .desc = MSG_SKILL_FormShift_DESC,
        .capacity = MSG_SKILL_FormShift_CAPACITY,
        .price = MSG_SKILL_FormShift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HalfShift) && COMMON_SKILL_VALID(SID_HalfShift))
    [SID_HalfShift] = {
        .name = MSG_SKILL_HalfShift_NAME,
        .desc = MSG_SKILL_HalfShift_DESC,
        .capacity = MSG_SKILL_HalfShift_CAPACITY,
        .price = MSG_SKILL_HalfShift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AbsorbAlternation) && COMMON_SKILL_VALID(SID_AbsorbAlternation))
    [SID_AbsorbAlternation] = {
        .name = MSG_SKILL_AbsorbAlternation_NAME,
        .desc = MSG_SKILL_AbsorbAlternation_DESC,
        .capacity = MSG_SKILL_AbsorbAlternation_CAPACITY,
        .price = MSG_SKILL_AbsorbAlternation_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GoodListener) && COMMON_SKILL_VALID(SID_GoodListener))
    [SID_GoodListener] = {
        .name = MSG_SKILL_GoodListener_NAME,
        .desc = MSG_SKILL_GoodListener_DESC,
        .capacity = MSG_SKILL_GoodListener_CAPACITY,
        .price = MSG_SKILL_GoodListener_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ScrollScribePlus) && COMMON_SKILL_VALID(SID_ScrollScribePlus))
    [SID_ScrollScribePlus] = {
        .name = MSG_SKILL_ScrollScribePlus_NAME,
        .desc = MSG_SKILL_ScrollScribePlus_DESC,
        .capacity = MSG_SKILL_ScrollScribePlus_CAPACITY,
        .price = MSG_SKILL_ScrollScribePlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_UnarmedCombat) && COMMON_SKILL_VALID(SID_UnarmedCombat))
    [SID_UnarmedCombat] = {
        .name = MSG_SKILL_UnarmedCombat_NAME,
        .desc = MSG_SKILL_UnarmedCombat_DESC,
        .capacity = MSG_SKILL_UnarmedCombat_CAPACITY,
        .price = MSG_SKILL_UnarmedCombat_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Acidic) && COMMON_SKILL_VALID(SID_Acidic))
    [SID_Acidic] = {
        .name = MSG_SKILL_Acidic_NAME,
        .desc = MSG_SKILL_Acidic_DESC,
        .capacity = MSG_SKILL_Acidic_CAPACITY,
        .price = MSG_SKILL_Acidic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AcidicPlus) && COMMON_SKILL_VALID(SID_AcidicPlus))
    [SID_AcidicPlus] = {
        .name = MSG_SKILL_AcidicPlus_NAME,
        .desc = MSG_SKILL_AcidicPlus_DESC,
        .capacity = MSG_SKILL_AcidicPlus_CAPACITY,
        .price = MSG_SKILL_AcidicPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SereneGrace) && COMMON_SKILL_VALID(SID_SereneGrace))
    [SID_SereneGrace] = {
        .name = MSG_SKILL_SereneGrace_NAME,
        .desc = MSG_SKILL_SereneGrace_DESC,
        .capacity = MSG_SKILL_SereneGrace_CAPACITY,
        .price = MSG_SKILL_SereneGrace_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_StickyHold) && COMMON_SKILL_VALID(SID_StickyHold))
    [SID_StickyHold] = {
        .name = MSG_SKILL_StickyHold_NAME,
        .desc = MSG_SKILL_StickyHold_DESC,
        .capacity = MSG_SKILL_StickyHold_CAPACITY,
        .price = MSG_SKILL_StickyHold_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Levitate) && COMMON_SKILL_VALID(SID_Levitate))
    [SID_Levitate] = {
        .name = MSG_SKILL_Levitate_NAME,
        .desc = MSG_SKILL_Levitate_DESC,
        .capacity = MSG_SKILL_Levitate_CAPACITY,
        .price = MSG_SKILL_Levitate_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Protean) && COMMON_SKILL_VALID(SID_Protean))
    [SID_Protean] = {
        .name = MSG_SKILL_Protean_NAME,
        .desc = MSG_SKILL_Protean_DESC,
        .capacity = MSG_SKILL_Protean_CAPACITY,
        .price = MSG_SKILL_Protean_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Protect) && COMMON_SKILL_VALID(SID_Protect))
    [SID_Protect] = {
        .name = MSG_SKILL_Protect_NAME,
        .desc = MSG_SKILL_Protect_DESC,
        .capacity = MSG_SKILL_Protect_CAPACITY,
        .price = MSG_SKILL_Protect_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GoldenGlory) && COMMON_SKILL_VALID(SID_GoldenGlory))
    [SID_GoldenGlory] = {
        .name = MSG_SKILL_GoldenGlory_NAME,
        .desc = MSG_SKILL_GoldenGlory_DESC,
        .capacity = MSG_SKILL_GoldenGlory_CAPACITY,
        .price = MSG_SKILL_GoldenGlory_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Calibration) && COMMON_SKILL_VALID(SID_Calibration))
    [SID_Calibration] = {
        .name = MSG_SKILL_Calibration_NAME,
        .desc = MSG_SKILL_Calibration_DESC,
        .capacity = MSG_SKILL_Calibration_CAPACITY,
        .price = MSG_SKILL_Calibration_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Reinforce) && COMMON_SKILL_VALID(SID_Reinforce))
    [SID_Reinforce] = {
        .name = MSG_SKILL_Reinforce_NAME,
        .desc = MSG_SKILL_Reinforce_DESC,
        .capacity = MSG_SKILL_Reinforce_CAPACITY,
        .price = MSG_SKILL_Reinforce_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SupremeOverlord) && COMMON_SKILL_VALID(SID_SupremeOverlord))
    [SID_SupremeOverlord] = {
        .name = MSG_SKILL_SupremeOverlord_NAME,
        .desc = MSG_SKILL_SupremeOverlord_DESC,
        .capacity = MSG_SKILL_SupremeOverlord_CAPACITY,
        .price = MSG_SKILL_SupremeOverlord_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SealedZone) && COMMON_SKILL_VALID(SID_SealedZone))
    [SID_SealedZone] = {
        .name = MSG_SKILL_SealedZone_NAME,
        .desc = MSG_SKILL_SealedZone_DESC,
        .capacity = MSG_SKILL_SealedZone_CAPACITY,
        .price = MSG_SKILL_SealedZone_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Thrust) && COMMON_SKILL_VALID(SID_Thrust))
    [SID_Thrust] = {
        .name = MSG_SKILL_Thrust_NAME,
        .desc = MSG_SKILL_Thrust_DESC,
        .capacity = MSG_SKILL_Thrust_CAPACITY,
        .price = MSG_SKILL_Thrust_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChainAttack) && COMMON_SKILL_VALID(SID_ChainAttack))
    [SID_ChainAttack] = {
        .name = MSG_SKILL_ChainAttack_NAME,
        .desc = MSG_SKILL_ChainAttack_DESC,
        .capacity = MSG_SKILL_ChainAttack_CAPACITY,
        .price = MSG_SKILL_ChainAttack_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_NinthCircle) && COMMON_SKILL_VALID(SID_NinthCircle))
    [SID_NinthCircle] = {
        .name = MSG_SKILL_NinthCircle_NAME,
        .desc = MSG_SKILL_NinthCircle_DESC,
        .capacity = MSG_SKILL_NinthCircle_CAPACITY,
        .price = MSG_SKILL_NinthCircle_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_InevitableEnd) && COMMON_SKILL_VALID(SID_InevitableEnd))
    [SID_InevitableEnd] = {
        .name = MSG_SKILL_InevitableEnd_NAME,
        .desc = MSG_SKILL_InevitableEnd_DESC,
        .capacity = MSG_SKILL_InevitableEnd_CAPACITY,
        .price = MSG_SKILL_InevitableEnd_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MPChanneling) && COMMON_SKILL_VALID(SID_MPChanneling))
    [SID_MPChanneling] = {
        .name = MSG_SKILL_MPChanneling_NAME,
        .desc = MSG_SKILL_MPChanneling_DESC,
        .capacity = MSG_SKILL_MPChanneling_CAPACITY,
        .price = MSG_SKILL_MPChanneling_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Eldritch) && COMMON_SKILL_VALID(SID_Eldritch))
    [SID_Eldritch] = {
        .name = MSG_SKILL_Eldritch_NAME,
        .desc = MSG_SKILL_Eldritch_DESC,
        .capacity = MSG_SKILL_Eldritch_CAPACITY,
        .price = MSG_SKILL_Eldritch_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Osmose) && COMMON_SKILL_VALID(SID_Osmose))
    [SID_Osmose] = {
        .name = MSG_SKILL_Osmose_NAME,
        .desc = MSG_SKILL_Osmose_DESC,
        .capacity = MSG_SKILL_Osmose_CAPACITY,
        .price = MSG_SKILL_Osmose_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BloodMagic) && COMMON_SKILL_VALID(SID_BloodMagic))
    [SID_BloodMagic] = {
        .name = MSG_SKILL_BloodMagic_NAME,
        .desc = MSG_SKILL_BloodMagic_DESC,
        .capacity = MSG_SKILL_BloodMagic_CAPACITY,
        .price = MSG_SKILL_BloodMagic_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HalfMP) && COMMON_SKILL_VALID(SID_HalfMP))
    [SID_HalfMP] = {
        .name = MSG_SKILL_HalfMP_NAME,
        .desc = MSG_SKILL_HalfMP_DESC,
        .capacity = MSG_SKILL_HalfMP_CAPACITY,
        .price = MSG_SKILL_HalfMP_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TurboMP) && COMMON_SKILL_VALID(SID_TurboMP))
    [SID_TurboMP] = {
        .name = MSG_SKILL_TurboMP_NAME,
        .desc = MSG_SKILL_TurboMP_DESC,
        .capacity = MSG_SKILL_TurboMP_CAPACITY,
        .price = MSG_SKILL_TurboMP_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Carnage) && COMMON_SKILL_VALID(SID_Carnage))
    [SID_Carnage] = {
        .name = MSG_SKILL_Carnage_NAME,
        .desc = MSG_SKILL_Carnage_DESC,
        .capacity = MSG_SKILL_Carnage_CAPACITY,
        .price = MSG_SKILL_Carnage_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Crisis) && COMMON_SKILL_VALID(SID_Crisis))
    [SID_Crisis] = {
        .name = MSG_SKILL_Crisis_NAME,
        .desc = MSG_SKILL_Crisis_DESC,
        .capacity = MSG_SKILL_Crisis_CAPACITY,
        .price = MSG_SKILL_Crisis_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Samaritan) && COMMON_SKILL_VALID(SID_Samaritan))
    [SID_Samaritan] = {
        .name = MSG_SKILL_Samaritan_NAME,
        .desc = MSG_SKILL_Samaritan_DESC,
        .capacity = MSG_SKILL_Samaritan_CAPACITY,
        .price = MSG_SKILL_Samaritan_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Turret) && COMMON_SKILL_VALID(SID_Turret))
    [SID_Turret] = {
        .name = MSG_SKILL_Turret_NAME,
        .desc = MSG_SKILL_Turret_DESC,
        .capacity = MSG_SKILL_Turret_CAPACITY,
        .price = MSG_SKILL_Turret_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Overdraft) && COMMON_SKILL_VALID(SID_Overdraft))
    [SID_Overdraft] = {
        .name = MSG_SKILL_Overdraft_NAME,
        .desc = MSG_SKILL_Overdraft_DESC,
        .capacity = MSG_SKILL_Overdraft_CAPACITY,
        .price = MSG_SKILL_Overdraft_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Crescendo) && COMMON_SKILL_VALID(SID_Crescendo))
    [SID_Crescendo] = {
        .name = MSG_SKILL_Crescendo_NAME,
        .desc = MSG_SKILL_Crescendo_DESC,
        .capacity = MSG_SKILL_Crescendo_CAPACITY,
        .price = MSG_SKILL_Crescendo_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_PenanceStare) && COMMON_SKILL_VALID(SID_PenanceStare))
    [SID_PenanceStare] = {
        .name = MSG_SKILL_PenanceStare_NAME,
        .desc = MSG_SKILL_PenanceStare_DESC,
        .capacity = MSG_SKILL_PenanceStare_CAPACITY,
        .price = MSG_SKILL_PenanceStare_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_QuickSalve) && COMMON_SKILL_VALID(SID_QuickSalve))
    [SID_QuickSalve] = {
        .name = MSG_SKILL_QuickSalve_NAME,
        .desc = MSG_SKILL_QuickSalve_DESC,
        .capacity = MSG_SKILL_QuickSalve_CAPACITY,
        .price = MSG_SKILL_QuickSalve_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DilligentMind) && COMMON_SKILL_VALID(SID_DilligentMind))
    [SID_DilligentMind] = {
        .name = MSG_SKILL_DilligentMind_NAME,
        .desc = MSG_SKILL_DilligentMind_DESC,
        .capacity = MSG_SKILL_DilligentMind_CAPACITY,
        .price = MSG_SKILL_DilligentMind_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Domino) && COMMON_SKILL_VALID(SID_Domino))
    [SID_Domino] = {
        .name = MSG_SKILL_Domino_NAME,
        .desc = MSG_SKILL_Domino_DESC,
        .capacity = MSG_SKILL_Domino_CAPACITY,
        .price = MSG_SKILL_Domino_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_RadiantEdict) && COMMON_SKILL_VALID(SID_RadiantEdict))
    [SID_RadiantEdict] = {
        .name = MSG_SKILL_RadiantEdict_NAME,
        .desc = MSG_SKILL_RadiantEdict_DESC,
        .capacity = MSG_SKILL_RadiantEdict_CAPACITY,
        .price = MSG_SKILL_RadiantEdict_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EvasionLock) && COMMON_SKILL_VALID(SID_EvasionLock))
    [SID_EvasionLock] = {
        .name = MSG_SKILL_EvasionLock_NAME,
        .desc = MSG_SKILL_EvasionLock_DESC,
        .capacity = MSG_SKILL_EvasionLock_CAPACITY,
        .price = MSG_SKILL_EvasionLock_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Pyrotechnics) && COMMON_SKILL_VALID(SID_Pyrotechnics))
    [SID_Pyrotechnics] = {
        .name = MSG_SKILL_Pyrotechnics_NAME,
        .desc = MSG_SKILL_Pyrotechnics_DESC,
        .capacity = MSG_SKILL_Pyrotechnics_CAPACITY,
        .price = MSG_SKILL_Pyrotechnics_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Replicate) && COMMON_SKILL_VALID(SID_Replicate))
    [SID_Replicate] = {
        .name = MSG_SKILL_Replicate_NAME,
        .desc = MSG_SKILL_Replicate_DESC,
        .capacity = MSG_SKILL_Replicate_CAPACITY,
        .price = MSG_SKILL_Replicate_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MasterForge) && COMMON_SKILL_VALID(SID_MasterForge))
    [SID_MasterForge] = {
        .name = MSG_SKILL_MasterForge_NAME,
        .desc = MSG_SKILL_MasterForge_DESC,
        .capacity = MSG_SKILL_MasterForge_CAPACITY,
        .price = MSG_SKILL_MasterForge_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BravelyDefault) && COMMON_SKILL_VALID(SID_BravelyDefault))
    [SID_BravelyDefault] = {
        .name = MSG_SKILL_BravelyDefault_NAME,
        .desc = MSG_SKILL_BravelyDefault_DESC,
        .capacity = MSG_SKILL_BravelyDefault_CAPACITY,
        .price = MSG_SKILL_BravelyDefault_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Quagmire) && COMMON_SKILL_VALID(SID_Quagmire))
    [SID_Quagmire] = {
        .name = MSG_SKILL_Quagmire_NAME,
        .desc = MSG_SKILL_Quagmire_DESC,
        .capacity = MSG_SKILL_Quagmire_CAPACITY,
        .price = MSG_SKILL_Quagmire_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Fogger) && COMMON_SKILL_VALID(SID_Fogger))
    [SID_Fogger] = {
        .name = MSG_SKILL_Fogger_NAME,
        .desc = MSG_SKILL_Fogger_DESC,
        .capacity = MSG_SKILL_Fogger_CAPACITY,
        .price = MSG_SKILL_Fogger_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Duplicate) && COMMON_SKILL_VALID(SID_Duplicate))
    [SID_Duplicate] = {
        .name = MSG_SKILL_Duplicate_NAME,
        .desc = MSG_SKILL_Duplicate_DESC,
        .capacity = MSG_SKILL_Duplicate_CAPACITY,
        .price = MSG_SKILL_Duplicate_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Reroll) && COMMON_SKILL_VALID(SID_Reroll))
    [SID_Reroll] = {
        .name = MSG_SKILL_Reroll_NAME,
        .desc = MSG_SKILL_Reroll_DESC,
        .capacity = MSG_SKILL_Reroll_CAPACITY,
        .price = MSG_SKILL_Reroll_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SpellBlade) && COMMON_SKILL_VALID(SID_SpellBlade))
    [SID_SpellBlade] = {
        .name = MSG_SKILL_SpellBlade_NAME,
        .desc = MSG_SKILL_SpellBlade_DESC,
        .capacity = MSG_SKILL_SpellBlade_CAPACITY,
        .price = MSG_SKILL_SpellBlade_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Caltrops) && COMMON_SKILL_VALID(SID_Caltrops))
    [SID_Caltrops] = {
        .name = MSG_SKILL_Caltrops_NAME,
        .desc = MSG_SKILL_Caltrops_DESC,
        .capacity = MSG_SKILL_Caltrops_CAPACITY,
        .price = MSG_SKILL_Caltrops_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DamageToMP) && COMMON_SKILL_VALID(SID_DamageToMP))
    [SID_DamageToMP] = {
        .name = MSG_SKILL_DamageToMP_NAME,
        .desc = MSG_SKILL_DamageToMP_DESC,
        .capacity = MSG_SKILL_DamageToMP_CAPACITY,
        .price = MSG_SKILL_DamageToMP_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Blight) && COMMON_SKILL_VALID(SID_Blight))
    [SID_Blight] = {
        .name = MSG_SKILL_Blight_NAME,
        .desc = MSG_SKILL_Blight_DESC,
        .capacity = MSG_SKILL_Blight_CAPACITY,
        .price = MSG_SKILL_Blight_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FaustianBargain) && COMMON_SKILL_VALID(SID_FaustianBargain))
    [SID_FaustianBargain] = {
        .name = MSG_SKILL_FaustianBargain_NAME,
        .desc = MSG_SKILL_FaustianBargain_DESC,
        .capacity = MSG_SKILL_FaustianBargain_CAPACITY,
        .price = MSG_SKILL_FaustianBargain_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Seafarer) && COMMON_SKILL_VALID(SID_Seafarer))
    [SID_Seafarer] = {
        .name = MSG_SKILL_Seafarer_NAME,
        .desc = MSG_SKILL_Seafarer_DESC,
        .capacity = MSG_SKILL_Seafarer_CAPACITY,
        .price = MSG_SKILL_Seafarer_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Hiker) && COMMON_SKILL_VALID(SID_Hiker))
    [SID_Hiker] = {
        .name = MSG_SKILL_Hiker_NAME,
        .desc = MSG_SKILL_Hiker_DESC,
        .capacity = MSG_SKILL_Hiker_CAPACITY,
        .price = MSG_SKILL_Hiker_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_FocusEnergy) && COMMON_SKILL_VALID(SID_FocusEnergy))
    [SID_FocusEnergy] = {
        .name = MSG_SKILL_FocusEnergy_NAME,
        .desc = MSG_SKILL_FocusEnergy_DESC,
        .capacity = MSG_SKILL_FocusEnergy_CAPACITY,
        .price = MSG_SKILL_FocusEnergy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MineDetector) && COMMON_SKILL_VALID(SID_MineDetector))
    [SID_MineDetector] = {
        .name = MSG_SKILL_MineDetector_NAME,
        .desc = MSG_SKILL_MineDetector_DESC,
        .capacity = MSG_SKILL_MineDetector_CAPACITY,
        .price = MSG_SKILL_MineDetector_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_TrapBuster) && COMMON_SKILL_VALID(SID_TrapBuster))
    [SID_TrapBuster] = {
        .name = MSG_SKILL_TrapBuster_NAME,
        .desc = MSG_SKILL_TrapBuster_DESC,
        .capacity = MSG_SKILL_TrapBuster_CAPACITY,
        .price = MSG_SKILL_TrapBuster_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EXPShare) && COMMON_SKILL_VALID(SID_EXPShare))
    [SID_EXPShare] = {
        .name = MSG_SKILL_EXPShare_NAME,
        .desc = MSG_SKILL_EXPShare_DESC,
        .capacity = MSG_SKILL_EXPShare_CAPACITY,
        .price = MSG_SKILL_EXPShare_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GorillaTactics) && COMMON_SKILL_VALID(SID_GorillaTactics))
    [SID_GorillaTactics] = {
        .name = MSG_SKILL_GorillaTactics_NAME,
        .desc = MSG_SKILL_GorillaTactics_DESC,
        .capacity = MSG_SKILL_GorillaTactics_CAPACITY,
        .price = MSG_SKILL_GorillaTactics_PRICE,
        .icon = GFX_SkillIcon_GorillaTactics,
    },
#endif

#if (defined(SID_GraceOfFire) && COMMON_SKILL_VALID(SID_GraceOfFire))
    [SID_GraceOfFire] = {
        .name = MSG_SKILL_GraceOfFire_NAME,
        .desc = MSG_SKILL_GraceOfFire_DESC,
        .capacity = MSG_SKILL_GraceOfFire_CAPACITY,
        .price = MSG_SKILL_GraceOfFire_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_GraceOfWater) && COMMON_SKILL_VALID(SID_GraceOfWater))
    [SID_GraceOfWater] = {
        .name = MSG_SKILL_GraceOfWater_NAME,
        .desc = MSG_SKILL_GraceOfWater_DESC,
        .capacity = MSG_SKILL_GraceOfWater_CAPACITY,
        .price = MSG_SKILL_GraceOfWater_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_WitchsBrew) && COMMON_SKILL_VALID(SID_WitchsBrew))
    [SID_WitchsBrew] = {
        .name = MSG_SKILL_WitchsBrew_NAME,
        .desc = MSG_SKILL_WitchsBrew_DESC,
        .capacity = MSG_SKILL_WitchsBrew_CAPACITY,
        .price = MSG_SKILL_WitchsBrew_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Undying) && COMMON_SKILL_VALID(SID_Undying))
    [SID_Undying] = {
        .name = MSG_SKILL_Undying_NAME,
        .desc = MSG_SKILL_Undying_DESC,
        .capacity = MSG_SKILL_Undying_CAPACITY,
        .price = MSG_SKILL_Undying_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_DoubleTime) && COMMON_SKILL_VALID(SID_DoubleTime))
    [SID_DoubleTime] = {
        .name = MSG_SKILL_DoubleTime_NAME,
        .desc = MSG_SKILL_DoubleTime_DESC,
        .capacity = MSG_SKILL_DoubleTime_CAPACITY,
        .price = MSG_SKILL_DoubleTime_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_HighGrace) && COMMON_SKILL_VALID(SID_HighGrace))
    [SID_HighGrace] = {
        .name = MSG_SKILL_HighGrace_NAME,
        .desc = MSG_SKILL_HighGrace_DESC,
        .capacity = MSG_SKILL_HighGrace_CAPACITY,
        .price = MSG_SKILL_HighGrace_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Hasty) && COMMON_SKILL_VALID(SID_Hasty))
    [SID_Hasty] = {
        .name = MSG_SKILL_Hasty_NAME,
        .desc = MSG_SKILL_Hasty_DESC,
        .capacity = MSG_SKILL_Hasty_CAPACITY,
        .price = MSG_SKILL_Hasty_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Ripple) && COMMON_SKILL_VALID(SID_Ripple))
    [SID_Ripple] = {
        .name = MSG_SKILL_Ripple_NAME,
        .desc = MSG_SKILL_Ripple_DESC,
        .capacity = MSG_SKILL_Ripple_CAPACITY,
        .price = MSG_SKILL_Ripple_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Minions) && COMMON_SKILL_VALID(SID_Minions))
    [SID_Minions] = {
        .name = MSG_SKILL_Minions_NAME,
        .desc = MSG_SKILL_Minions_DESC,
        .capacity = MSG_SKILL_Minions_CAPACITY,
        .price = MSG_SKILL_Minions_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChargeDefense) && COMMON_SKILL_VALID(SID_ChargeDefense))
    [SID_ChargeDefense] = {
        .name = MSG_SKILL_ChargeDefense_NAME,
        .desc = MSG_SKILL_ChargeDefense_DESC,
        .capacity = MSG_SKILL_ChargeDefense_CAPACITY,
        .price = MSG_SKILL_ChargeDefense_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChargeSpeed) && COMMON_SKILL_VALID(SID_ChargeSpeed))
    [SID_ChargeSpeed] = {
        .name = MSG_SKILL_ChargeSpeed_NAME,
        .desc = MSG_SKILL_ChargeSpeed_DESC,
        .capacity = MSG_SKILL_ChargeSpeed_CAPACITY,
        .price = MSG_SKILL_ChargeSpeed_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChargeSkill) && COMMON_SKILL_VALID(SID_ChargeSkill))
    [SID_ChargeSkill] = {
        .name = MSG_SKILL_ChargeSkill_NAME,
        .desc = MSG_SKILL_ChargeSkill_DESC,
        .capacity = MSG_SKILL_ChargeSkill_CAPACITY,
        .price = MSG_SKILL_ChargeSkill_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ChargeCrit) && COMMON_SKILL_VALID(SID_ChargeCrit))
    [SID_ChargeCrit] = {
        .name = MSG_SKILL_ChargeCrit_NAME,
        .desc = MSG_SKILL_ChargeCrit_DESC,
        .capacity = MSG_SKILL_ChargeCrit_CAPACITY,
        .price = MSG_SKILL_ChargeCrit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Bloodthirst) && COMMON_SKILL_VALID(SID_Bloodthirst))
    [SID_Bloodthirst] = {
        .name = MSG_SKILL_Bloodthirst_NAME,
        .desc = MSG_SKILL_Bloodthirst_DESC,
        .capacity = MSG_SKILL_Bloodthirst_CAPACITY,
        .price = MSG_SKILL_Bloodthirst_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SliceNDice) && COMMON_SKILL_VALID(SID_SliceNDice))
    [SID_SliceNDice] = {
        .name = MSG_SKILL_SliceNDice_NAME,
        .desc = MSG_SKILL_SliceNDice_DESC,
        .capacity = MSG_SKILL_SliceNDice_CAPACITY,
        .price = MSG_SKILL_SliceNDice_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ManaRush) && COMMON_SKILL_VALID(SID_ManaRush))
    [SID_ManaRush] = {
        .name = MSG_SKILL_ManaRush_NAME,
        .desc = MSG_SKILL_ManaRush_DESC,
        .capacity = MSG_SKILL_ManaRush_CAPACITY,
        .price = MSG_SKILL_ManaRush_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Symbiosis) && COMMON_SKILL_VALID(SID_Symbiosis))
    [SID_Symbiosis] = {
        .name = MSG_SKILL_Symbiosis_NAME,
        .desc = MSG_SKILL_Symbiosis_DESC,
        .capacity = MSG_SKILL_Symbiosis_CAPACITY,
        .price = MSG_SKILL_Symbiosis_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Residium) && COMMON_SKILL_VALID(SID_Residium))
    [SID_Residium] = {
        .name = MSG_SKILL_Residium_NAME,
        .desc = MSG_SKILL_Residium_DESC,
        .capacity = MSG_SKILL_Residium_CAPACITY,
        .price = MSG_SKILL_Residium_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Malidiction) && COMMON_SKILL_VALID(SID_Malidiction))
    [SID_Malidiction] = {
        .name = MSG_SKILL_Malidiction_NAME,
        .desc = MSG_SKILL_Malidiction_DESC,
        .capacity = MSG_SKILL_Malidiction_CAPACITY,
        .price = MSG_SKILL_Malidiction_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_AirWalk) && COMMON_SKILL_VALID(SID_AirWalk))
    [SID_AirWalk] = {
        .name = MSG_SKILL_AirWalk_NAME,
        .desc = MSG_SKILL_AirWalk_DESC,
        .capacity = MSG_SKILL_AirWalk_CAPACITY,
        .price = MSG_SKILL_AirWalk_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_BloodyAlchemy) && COMMON_SKILL_VALID(SID_BloodyAlchemy))
    [SID_BloodyAlchemy] = {
        .name = MSG_SKILL_BloodyAlchemy_NAME,
        .desc = MSG_SKILL_BloodyAlchemy_DESC,
        .capacity = MSG_SKILL_BloodyAlchemy_CAPACITY,
        .price = MSG_SKILL_BloodyAlchemy_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_SPConversion) && COMMON_SKILL_VALID(SID_SPConversion))
    [SID_SPConversion] = {
        .name = MSG_SKILL_SPConversion_NAME,
        .desc = MSG_SKILL_SPConversion_DESC,
        .capacity = MSG_SKILL_SPConversion_CAPACITY,
        .price = MSG_SKILL_SPConversion_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Enlightenment) && COMMON_SKILL_VALID(SID_Enlightenment))
    [SID_Enlightenment] = {
        .name = MSG_SKILL_Enlightenment_NAME,
        .desc = MSG_SKILL_Enlightenment_DESC,
        .capacity = MSG_SKILL_Enlightenment_CAPACITY,
        .price = MSG_SKILL_Enlightenment_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif


#if (defined(SID_GateOfBabylon) && COMMON_SKILL_VALID(SID_GateOfBabylon))
    [SID_GateOfBabylon] = {
        .name = MSG_SKILL_GateOfBabylon_NAME,
        .desc = MSG_SKILL_GateOfBabylon_DESC,
        .capacity = MSG_SKILL_GateOfBabylon_CAPACITY,
        .price = MSG_SKILL_GateOfBabylon_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_CritThrift) && COMMON_SKILL_VALID(SID_CritThrift))
    [SID_CritThrift] = {
        .name = MSG_SKILL_CritThrift_NAME,
        .desc = MSG_SKILL_CritThrift_DESC,
        .capacity = MSG_SKILL_CritThrift_CAPACITY,
        .price = MSG_SKILL_CritThrift_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_ManaTransfusion) && COMMON_SKILL_VALID(SID_ManaTransfusion))
    [SID_ManaTransfusion] = {
        .name = MSG_SKILL_ManaTransfusion_NAME,
        .desc = MSG_SKILL_ManaTransfusion_DESC,
        .capacity = MSG_SKILL_ManaTransfusion_CAPACITY,
        .price = MSG_SKILL_ManaTransfusion_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MPBoost) && COMMON_SKILL_VALID(SID_MPBoost))
    [SID_MPBoost] = {
        .name = MSG_SKILL_MPBoost_NAME,
        .desc = MSG_SKILL_MPBoost_DESC,
        .capacity = MSG_SKILL_MPBoost_CAPACITY,
        .price = MSG_SKILL_MPBoost_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_MPBoostPlus) && COMMON_SKILL_VALID(SID_MPBoostPlus))
    [SID_MPBoostPlus] = {
        .name = MSG_SKILL_MPBoostPlus_NAME,
        .desc = MSG_SKILL_MPBoostPlus_DESC,
        .capacity = MSG_SKILL_MPBoostPlus_CAPACITY,
        .price = MSG_SKILL_MPBoostPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Perseverance) && COMMON_SKILL_VALID(SID_Perseverance))
    [SID_Perseverance] = {
        .name = MSG_SKILL_Perseverance_NAME,
        .desc = MSG_SKILL_Perseverance_DESC,
        .capacity = MSG_SKILL_Perseverance_CAPACITY,
        .price = MSG_SKILL_Perseverance_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EmergencyExit) && COMMON_SKILL_VALID(SID_EmergencyExit))
    [SID_EmergencyExit] = {
        .name = MSG_SKILL_EmergencyExit_NAME,
        .desc = MSG_SKILL_EmergencyExit_DESC,
        .capacity = MSG_SKILL_EmergencyExit_CAPACITY,
        .price = MSG_SKILL_EmergencyExit_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_EmergencyExitPlus) && COMMON_SKILL_VALID(SID_EmergencyExitPlus))
    [SID_EmergencyExitPlus] = {
        .name = MSG_SKILL_EmergencyExitPlus_NAME,
        .desc = MSG_SKILL_EmergencyExitPlus_DESC,
        .capacity = MSG_SKILL_EmergencyExitPlus_CAPACITY,
        .price = MSG_SKILL_EmergencyExitPlus_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Songstress) && COMMON_SKILL_VALID(SID_Songstress))
    [SID_Songstress] = {
        .name = MSG_SKILL_Songstress_NAME,
        .desc = MSG_SKILL_Songstress_DESC,
        .capacity = MSG_SKILL_Songstress_CAPACITY,
        .price = MSG_SKILL_Songstress_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

#if (defined(SID_Equlibrium) && COMMON_SKILL_VALID(SID_Equlibrium))
    [SID_Equlibrium] = {
        .name = MSG_SKILL_Equlibrium_NAME,
        .desc = MSG_SKILL_Equlibrium_DESC,
        .capacity = MSG_SKILL_Equlibrium_CAPACITY,
        .price = MSG_SKILL_Equlibrium_PRICE,
        .icon = GFX_SkillIcon_WIP,
    },
#endif

};