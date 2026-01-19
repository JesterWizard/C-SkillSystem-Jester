#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

const SkillActionFunc_t gSkillActionFuncTable[MAX_SKILL_NUM + 1] = {
#if (defined(SID_HealingFocus) && COMMON_SKILL_VALID(SID_HealingFocus))
	[SID_HealingFocus] = Action_HealingFocus,
#endif

#if (defined(SID_Teleportation) && COMMON_SKILL_VALID(SID_Teleportation))
	[SID_Teleportation] = Action_Teleportation,
#endif

#if (defined(SID_LightRune) && COMMON_SKILL_VALID(SID_LightRune))
	[SID_LightRune] = Action_LightRune,
#endif

#if (defined(SID_Mine) && COMMON_SKILL_VALID(SID_Mine))
	[SID_Mine] = Action_Mine,
#endif

#if (defined(SID_RallyDefense) && COMMON_SKILL_VALID(SID_RallyDefense))
	[SID_RallyDefense] = Action_Rally,
#endif

#if (defined(SID_RallyLuck) && COMMON_SKILL_VALID(SID_RallyLuck))
	[SID_RallyLuck] = Action_Rally,
#endif

#if (defined(SID_RallyMagic) && COMMON_SKILL_VALID(SID_RallyMagic))
	[SID_RallyMagic] = Action_Rally,
#endif

#if (defined(SID_RallyMovement) && COMMON_SKILL_VALID(SID_RallyMovement))
	[SID_RallyMovement] = Action_Rally,
#endif

#if (defined(SID_RallyResistance) && COMMON_SKILL_VALID(SID_RallyResistance))
	[SID_RallyResistance] = Action_Rally,
#endif

#if (defined(SID_RallySkill) && COMMON_SKILL_VALID(SID_RallySkill))
	[SID_RallySkill] = Action_Rally,
#endif

#if (defined(SID_RallySpeed) && COMMON_SKILL_VALID(SID_RallySpeed))
	[SID_RallySpeed] = Action_Rally,
#endif

#if (defined(SID_RallyStrength) && COMMON_SKILL_VALID(SID_RallyStrength))
	[SID_RallyStrength] = Action_Rally,
#endif

#if (defined(SID_RallySpectrum) && COMMON_SKILL_VALID(SID_RallySpectrum))
	[SID_RallySpectrum] = Action_Rally,
#endif

#if (defined(SID_GoddessDance) && COMMON_SKILL_VALID(SID_GoddessDance))
	[SID_GoddessDance] = Action_GoddessDance,
#endif

#if (defined(SID_Stride) && COMMON_SKILL_VALID(SID_Stride))
	[SID_Stride] = Action_Stride,
#endif

#if (defined(SID_Swarp) && COMMON_SKILL_VALID(SID_Swarp))
	[SID_Swarp] = Action_Swarp,
#endif

#if (defined(SID_Blacksmith) && COMMON_SKILL_VALID(SID_Blacksmith))
	[SID_Blacksmith] = Action_Blacksmith,
#endif

#if (defined(SID_DivineReprieve) && COMMON_SKILL_VALID(SID_DivineReprieve))
	[SID_DivineReprieve] = Action_DivineReprieve,
#endif

#if (defined(SID_Replicate) && COMMON_SKILL_VALID(SID_Replicate))
	[SID_Replicate] = Action_Replicate,
#endif

#if (defined(SID_MasterForge) && COMMON_SKILL_VALID(SID_MasterForge))
	[SID_MasterForge] = Action_MasterForge,
#endif

#if (defined(SID_BravelyDefault) && COMMON_SKILL_VALID(SID_BravelyDefault))
	[SID_BravelyDefault] = Action_BravelyDefault,
#endif

#if (defined(SID_Fogger) && COMMON_SKILL_VALID(SID_Fogger))
	[SID_Fogger] = Action_Fogger,
#endif

#if (defined(SID_Duplicate) && COMMON_SKILL_VALID(SID_Duplicate))
	[SID_Duplicate] = Action_Duplicate,
#endif

#if (defined(SID_Reroll) && COMMON_SKILL_VALID(SID_Reroll))
	[SID_Reroll] = Action_Reroll,
#endif

#if (defined(SID_SpellBlade) && COMMON_SKILL_VALID(SID_SpellBlade))
	[SID_SpellBlade] = Action_SpellBlade,
#endif

#if (defined(SID_FocusEnergy) && COMMON_SKILL_VALID(SID_FocusEnergy))
	[SID_FocusEnergy] = Action_FocusEnergy,
#endif

#if (defined(SID_GorillaTactics) && COMMON_SKILL_VALID(SID_GorillaTactics))
	[SID_GorillaTactics] = Action_GorillaTactics,
#endif

#if (defined(SID_GraceOfFire) && COMMON_SKILL_VALID(SID_GraceOfFire))
	[SID_GraceOfFire] = Action_GraceOfFire,
#endif

#if (defined(SID_GraceOfWater) && COMMON_SKILL_VALID(SID_GraceOfWater))
	[SID_GraceOfWater] = Action_GraceOfWater,
#endif

#if (defined(SID_Swap) && COMMON_SKILL_VALID(SID_Swap))
    [SID_Swap] = Action_Swap,
#endif

#if (defined(SID_Transform) && COMMON_SKILL_VALID(SID_Transform))
    [SID_Transform] = Action_Transform,
#endif

#if (defined(SID_DrawBack) && COMMON_SKILL_VALID(SID_DrawBack))
    [SID_DrawBack] = Action_DrawBack,
#endif

#if (defined(SID_Pivot) && COMMON_SKILL_VALID(SID_Pivot))
    [SID_Pivot] = Action_Pivot,
#endif

#if (defined(SID_Reposition) && COMMON_SKILL_VALID(SID_Reposition))
    [SID_Reposition] = Action_Reposition,
#endif

#if (defined(SID_Shove) && COMMON_SKILL_VALID(SID_Shove))
    [SID_Shove] = Action_Shove,
#endif

#if (defined(SID_Smite) && COMMON_SKILL_VALID(SID_Smite))
    [SID_Smite] = Action_Smite,
#endif

#if (defined(SID_ArdentSacrifice) && COMMON_SKILL_VALID(SID_ArdentSacrifice))
    [SID_ArdentSacrifice] = Action_ArdentSacrifice,
#endif

#if (defined(SID_ReciprocalAid) && COMMON_SKILL_VALID(SID_ReciprocalAid))
    [SID_ReciprocalAid] = Action_ReciprocalAid,
#endif

#if (defined(SID_Sacrifice) && COMMON_SKILL_VALID(SID_Sacrifice))
    [SID_Sacrifice] = Action_Sacrifice,
#endif

#if (defined(SID_Capture) && COMMON_SKILL_VALID(SID_Capture))
    [SID_Capture] = Action_Capture,
#endif

#if (defined(SID_Doppleganger) && COMMON_SKILL_VALID(SID_Doppleganger))
    [SID_Doppleganger] = Action_Doppleganger,
#endif

#if (defined(SID_Dismount) && COMMON_SKILL_VALID(SID_Dismount))
    [SID_Dismount] = Action_Dismount,
#endif

#if (defined(SID_Entrepreneur) && COMMON_SKILL_VALID(SID_Entrepreneur))
    [SID_Entrepreneur] = Action_Entrepreneur,
#endif

#if (defined(SID_Arise) && COMMON_SKILL_VALID(SID_Arise))
    [SID_Arise] = Action_Arise,
#endif

#if (defined(SID_AssignDecoy) && COMMON_SKILL_VALID(SID_AssignDecoy))
    [SID_AssignDecoy] = Action_AssignDecoy,
#endif

#if (defined(SID_AssignDecoyPlus) && COMMON_SKILL_VALID(SID_AssignDecoyPlus))
    [SID_AssignDecoyPlus] = Action_AssignDecoyPlus,
#endif

#if (defined(SID_ShadowFlash) && COMMON_SKILL_VALID(SID_ShadowFlash))
    [SID_ShadowFlash] = Action_ShadowFlash,
#endif

#if (defined(SID_DeathBlight) && COMMON_SKILL_VALID(SID_DeathBlight))
    [SID_DeathBlight] = Action_DeathBlight,
#endif

#if (defined(SID_LoadstarRush) && COMMON_SKILL_VALID(SID_LoadstarRush))
	[SID_LoadstarRush] = Action_LoadstarRush,
#endif

#if (defined(SID_WyvernCrash) && COMMON_SKILL_VALID(SID_WyvernCrash))
	[SID_WyvernCrash] = Action_WyvernCrash,
#endif

#if (defined(SID_Sellsword) && COMMON_SKILL_VALID(SID_Sellsword))
	[SID_Sellsword] = Action_Sellsword,
#endif

#if (defined(SID_Switcher) && COMMON_SKILL_VALID(SID_Switcher))
	[SID_Switcher] = Action_Switcher,
#endif

#if (defined(SID_LethalityPlus) && COMMON_SKILL_VALID(SID_LethalityPlus))
	[SID_LethalityPlus] = Action_LethalityPlus,
#endif

#if (defined(SID_GrimReaper) && COMMON_SKILL_VALID(SID_GrimReaper))
	[SID_GrimReaper] = Action_GrimReaper,
#endif

#if (defined(SID_Hide) && COMMON_SKILL_VALID(SID_Hide))
	[SID_Hide] = Action_Hide,
#endif

#if (defined(SID_SkillSwap) && COMMON_SKILL_VALID(SID_SkillSwap))
    [SID_SkillSwap] = Action_SkillSwap,
#endif

#if (defined(SID_SkillSwapPlus) && COMMON_SKILL_VALID(SID_SkillSwapPlus))
    [SID_SkillSwapPlus] = Action_SkillSwapPlus,
#endif

#if (defined(SID_Shuffle) && COMMON_SKILL_VALID(SID_Shuffle))
    [SID_Shuffle] = Action_Shuffle,
#endif

#if (defined(SID_ShufflePlus) && COMMON_SKILL_VALID(SID_ShufflePlus))
    [SID_ShufflePlus] = Action_ShufflePlus,
#endif

#if (defined(SID_Persuade) && COMMON_SKILL_VALID(SID_Persuade))
    [SID_Persuade] = Action_Persuade,
#endif

#if (defined(SID_PersuadePlus) && COMMON_SKILL_VALID(SID_PersuadePlus))
    [SID_PersuadePlus] = Action_PersuadePlus,
#endif

#if (defined(SID_Kamikaze) && COMMON_SKILL_VALID(SID_Kamikaze))
    [SID_Kamikaze] = Action_Kamikaze,
#endif

#if (defined(SID_Reinforcements) && COMMON_SKILL_VALID(SID_Reinforcements))
    [SID_Reinforcements] = Action_Reinforcements,
#endif

#if (defined(SID_ResolvedHeart) && COMMON_SKILL_VALID(SID_ResolvedHeart))
	[SID_ResolvedHeart] = Action_ResolvedHeart,
#endif

#if (defined(SID_Ballistary) && COMMON_SKILL_VALID(SID_Ballistary))
	[SID_Ballistary] = Action_Ballistary,
#endif

#if (defined(SID_Bide) && COMMON_SKILL_VALID(SID_Bide))
	[SID_Bide] = Action_Bide,
#endif

#if (defined(SID_CoinFlip) && COMMON_SKILL_VALID(SID_CoinFlip))
	[SID_CoinFlip] = Action_CoinFlip,
#endif

#if (defined(SID_Geomancy) && COMMON_SKILL_VALID(SID_Geomancy))
	[SID_Geomancy] = Action_Geomancy,
#endif

#if (defined(SID_ScrollScribePlus) && COMMON_SKILL_VALID(SID_ScrollScribePlus))
	[SID_ScrollScribePlus] = Action_ScrollScribePlus,
#endif

#if (defined(SID_DancePlus) && COMMON_SKILL_VALID(SID_DancePlus))
	[SID_DancePlus] = Action_DancePlus,
#endif

#if (defined(SID_Acidic) && COMMON_SKILL_VALID(SID_Acidic))
	[SID_Acidic] = Action_Acidic,
#endif

#if (defined(SID_AcidicPlus) && COMMON_SKILL_VALID(SID_AcidicPlus))
	[SID_AcidicPlus] = Action_Acidic,
#endif

#if (defined(SID_Reinforce) && COMMON_SKILL_VALID(SID_Reinforce))
	[SID_Reinforce] = Action_Reinforce,
#endif

#if (defined(SID_SealedZone) && COMMON_SKILL_VALID(SID_SealedZone))
	[SID_SealedZone] = Action_SealedZone,
#endif

#if (defined(SID_Thrust) && COMMON_SKILL_VALID(SID_Thrust))
	[SID_Thrust] = Action_Thrust,
#endif

#if (defined(SID_DoubleTime) && COMMON_SKILL_VALID(SID_DoubleTime))
	[SID_DoubleTime] = Action_DoubleTime,
#endif

#if (defined(SID_BloodyAlchemy) && COMMON_SKILL_VALID(SID_BloodyAlchemy))
	[SID_BloodyAlchemy] = Action_BloodyAlchemy,
#endif

#if (defined(SID_EmergencyExit) && COMMON_SKILL_VALID(SID_EmergencyExit))
	[SID_EmergencyExit] = Action_EmergencyExit,
#endif

#if (defined(SID_EmergencyExitPlus) && COMMON_SKILL_VALID(SID_EmergencyExitPlus))
	[SID_EmergencyExitPlus] = Action_EmergencyExitPlus,
#endif

#if (defined(SID_Songstress) && COMMON_SKILL_VALID(SID_Songstress))
	[SID_Songstress] = Action_Songstress,
#endif

#if (defined(SID_Gungnir) && COMMON_SKILL_VALID(SID_Gungnir))
	[SID_Gungnir] = Action_Gungnir,
#endif

#if (defined(SID_Transcendence) && COMMON_SKILL_VALID(SID_Transcendence))
	[SID_Transcendence] = Action_Transcendence,
#endif

#if (defined(SID_Necromancy) && COMMON_SKILL_VALID(SID_Necromancy))
	[SID_Necromancy] = Action_Necromancy,
#endif
};
