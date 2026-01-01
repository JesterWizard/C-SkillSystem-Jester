#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

struct MenuItemDef const* const gSkillMenuInfos[MAX_SKILL_NUM + 1] = {
#if (defined(SID_Dance) && COMMON_SKILL_VALID(SID_Dance))
	[SID_Dance] = &(const struct MenuItemDef) {
		.name = "　踊る",
		.nameMsgId = 0x67E,
		.helpMsgId = 0x6C2,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = DanceCommandUsability,
		.onDraw = NULL,
		.onSelected = PlayCommandEffect,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Steal) && COMMON_SKILL_VALID(SID_Steal))
	[SID_Steal] = &(const struct MenuItemDef) {
		.name = "　盗む",
		.nameMsgId = 0x67F,
		.helpMsgId = 0x6C4,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = StealCommandUsability,
		.onDraw = NULL,
		.onSelected = StealCommandEffect,
		.onIdle = 0,
		.onSwitchIn = 0,
		.onSwitchOut = 0
	},
#endif

#if (defined(SID_Summon) && COMMON_SKILL_VALID(SID_Summon))
	[SID_Summon] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = 0x693,
		.helpMsgId = 0x6DD,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = SummonCommandUsability,
		.onDraw = 0,
		.onSelected = SummonCommandEffect,
		.onIdle = 0,
		.onSwitchIn = 0,
		.onSwitchOut = 0
	},
#endif

#if (defined(SID_LockTouch) && COMMON_SKILL_VALID(SID_LockTouch))
	[SID_LockTouch] = &(const struct MenuItemDef) {
		.name = "　かぎ開",
		.nameMsgId = 0x694,
		.helpMsgId = 0x6DE,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = PickCommandUsability,
		.onDraw = NULL,
		.onSelected = PickCommandEffect,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_HealingFocus) && COMMON_SKILL_VALID(SID_HealingFocus))
	[SID_HealingFocus] = &(const struct MenuItemDef) {
		.name = "　瞑想",
		.nameMsgId = MSG_SKILL_HealingFocus_NAME,
		.helpMsgId = MSG_SKILL_HealingFocus_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = HealingFocus_Usability,
		.onDraw = NULL,
		.onSelected = HealingFocus_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Teleportation) && COMMON_SKILL_VALID(SID_Teleportation))
	[SID_Teleportation] = &(const struct MenuItemDef) {
		.name = "　転移",
#if !CONFIG_MENU_SKILL_DISP_MSG_EN_N
		.nameMsgId = MSG_SKILL_Teleportation_NAME,
#endif
		.helpMsgId = MSG_SKILL_Teleportation_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Teleportation_Usability,
		.onDraw = NULL,
		.onSelected = Teleportation_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_LightRune) && COMMON_SKILL_VALID(SID_LightRune))
	[SID_LightRune] = &(const struct MenuItemDef) {
		.name = "　光の結界",
		.nameMsgId = MSG_SKILL_LightRune_NAME,
		.helpMsgId = MSG_SKILL_LightRune_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = LightRune_Usability,
		.onDraw = NULL,
		.onSelected = LightRune_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Mine) && COMMON_SKILL_VALID(SID_Mine))
	[SID_Mine] = &(const struct MenuItemDef) {
		.name = "　地雷",
		.nameMsgId = MSG_SKILL_Mine_NAME,
		.helpMsgId = MSG_SKILL_Mine_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Mine_Usability,
		.onDraw = NULL,
		.onSelected = Mine_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_RallyDefense) && COMMON_SKILL_VALID(SID_RallyDefense))
	[SID_RallyDefense] = &(const struct MenuItemDef) {
		.name = "　ラリー防御",
		.nameMsgId = MSG_SKILL_RallyDefense_NAME,
		.helpMsgId = MSG_SKILL_RallyDefense_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallyLuck) && COMMON_SKILL_VALID(SID_RallyLuck))
	[SID_RallyLuck] = &(const struct MenuItemDef) {
		.name = "　ラリー運",
		.nameMsgId = MSG_SKILL_RallyLuck_NAME,
		.helpMsgId = MSG_SKILL_RallyLuck_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallyMagic) && COMMON_SKILL_VALID(SID_RallyMagic))
	[SID_RallyMagic] = &(const struct MenuItemDef) {
		.name = "　ラリーマジック",
		.nameMsgId = MSG_SKILL_RallyMagic_NAME,
		.helpMsgId = MSG_SKILL_RallyMagic_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallyMovement) && COMMON_SKILL_VALID(SID_RallyMovement))
	[SID_RallyMovement] = &(const struct MenuItemDef) {
		.name = "　ラリーの動き",
		.nameMsgId = MSG_SKILL_RallyMovement_NAME,
		.helpMsgId = MSG_SKILL_RallyMovement_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallyResistance) && COMMON_SKILL_VALID(SID_RallyResistance))
	[SID_RallyResistance] = &(const struct MenuItemDef) {
		.name = "　ラリー耐性",
		.nameMsgId = MSG_SKILL_RallyResistance_NAME,
		.helpMsgId = MSG_SKILL_RallyResistance_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallySkill) && COMMON_SKILL_VALID(SID_RallySkill))
	[SID_RallySkill] = &(const struct MenuItemDef) {
		.name = "　ラリースキル",
		.nameMsgId = MSG_SKILL_RallySkill_NAME,
		.helpMsgId = MSG_SKILL_RallySkill_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallySpeed) && COMMON_SKILL_VALID(SID_RallySpeed))
	[SID_RallySpeed] = &(const struct MenuItemDef) {
		.name = "　ラリースピード",
		.nameMsgId = MSG_SKILL_RallySpeed_NAME,
		.helpMsgId = MSG_SKILL_RallySpeed_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallyStrength) && COMMON_SKILL_VALID(SID_RallyStrength))
	[SID_RallyStrength] = &(const struct MenuItemDef) {
		.name = "　ラリーパワー",
		.nameMsgId = MSG_SKILL_RallyStrength_NAME,
		.helpMsgId = MSG_SKILL_RallyStrength_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_RallySpectrum) && COMMON_SKILL_VALID(SID_RallySpectrum))
	[SID_RallySpectrum] = &(const struct MenuItemDef) {
		.name = "　ラリースペクトラム",
		.nameMsgId = MSG_SKILL_RallySpectrum_NAME,
		.helpMsgId = MSG_SKILL_RallySpectrum_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Rally_Usability,
		.onDraw = NULL,
		.onSelected = Rally_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Rally_Hover,
		.onSwitchOut = Rally_Unhover,
	},
#endif

#if (defined(SID_GoddessDance) && COMMON_SKILL_VALID(SID_GoddessDance))
	[SID_GoddessDance] = &(const struct MenuItemDef) {
		.name = "　女神の踊り",
		.nameMsgId = MSG_SKILL_GoddessDance_NAME,
		.helpMsgId = MSG_SKILL_GoddessDance_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = GoddessDance_Usability,
		.onDraw = NULL,
		.onSelected = GoddessDance_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = GoddessDance_Hover,
		.onSwitchOut = GoddessDance_Unhover,
	},
#endif

#if (defined(SID_Stride) && COMMON_SKILL_VALID(SID_Stride))
	[SID_Stride] = &(const struct MenuItemDef) {
		.name = "　大股",
		.nameMsgId = MSG_SKILL_Stride_NAME,
		.helpMsgId = MSG_SKILL_Stride_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Stride_Usability,
		.onDraw = NULL,
		.onSelected = Stride_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = Stride_Hover,
		.onSwitchOut = Stride_Unhover,
	},
#endif

#if (defined(SID_Swarp) && COMMON_SKILL_VALID(SID_Swarp))
	[SID_Swarp] = &(const struct MenuItemDef) {
		.name = "　スワープ",
		.nameMsgId = MSG_SKILL_Swarp_NAME,
		.helpMsgId = MSG_SKILL_Swarp_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Swarp_Usability,
		.onDraw = NULL,
		.onSelected = Swarp_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Blacksmith) && COMMON_SKILL_VALID(SID_Blacksmith))
	[SID_Blacksmith] = &(const struct MenuItemDef) {
		.name = "　鍛冶屋",
		.nameMsgId = MSG_SKILL_Blacksmith_NAME,
		.helpMsgId = MSG_SKILL_Blacksmith_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Blacksmith_Usability,
		.onDraw = NULL,
		.onSelected = Blacksmith_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_DivineReprieve) && COMMON_SKILL_VALID(SID_DivineReprieve))
	[SID_DivineReprieve] = &(const struct MenuItemDef) {
		.name = "　神裁の恩寵",
		.nameMsgId = MSG_SKILL_DivineReprieve_NAME,
		.helpMsgId = MSG_SKILL_DivineReprieve_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = DivineReprieve_Usability,
		.onDraw = NULL,
		.onSelected = DivineReprieve_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Replicate) && COMMON_SKILL_VALID(SID_Replicate))
	[SID_Replicate] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_Replicate_NAME,
		.helpMsgId = MSG_SKILL_Replicate_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Replicate_Usability,
		.onDraw = NULL,
		.onSelected = Replicate_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_MasterForge) && COMMON_SKILL_VALID(SID_MasterForge))
	[SID_MasterForge] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_MasterForge_NAME,
		.helpMsgId = MSG_SKILL_MasterForge_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = MasterForge_Usability,
		.onDraw = NULL,
		.onSelected = MasterForge_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_BravelyDefault) && COMMON_SKILL_VALID(SID_BravelyDefault))
	[SID_BravelyDefault] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_BravelyDefault_NAME,
		.helpMsgId = MSG_SKILL_BravelyDefault_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = BravelyDefault_Usability,
		.onDraw = NULL,
		.onSelected = BravelyDefault_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_Fogger) && COMMON_SKILL_VALID(SID_Fogger))
	[SID_Fogger] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_Fogger_NAME,
		.helpMsgId = MSG_SKILL_Fogger_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Fogger_Usability,
		.onDraw = NULL,
		.onSelected = Fogger_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_StealPlus) && COMMON_SKILL_VALID(SID_StealPlus))
	[SID_StealPlus] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_StealPlus_NAME,
		.helpMsgId = MSG_SKILL_StealPlus_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = StealPlus_Usability,
		.onDraw = NULL,
		.onSelected = StealPlus_Effect,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_Duplicate) && COMMON_SKILL_VALID(SID_Duplicate))
	[SID_Duplicate] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_Duplicate_NAME,
		.helpMsgId = MSG_SKILL_Duplicate_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Duplicate_Usability,
		.onDraw = NULL,
		.onSelected = Duplicate_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_Reroll) && COMMON_SKILL_VALID(SID_Reroll))
	[SID_Reroll] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_Reroll_NAME,
		.helpMsgId = MSG_SKILL_Reroll_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Reroll_Usability,
		.onDraw = NULL,
		.onSelected = Reroll_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_SpellBlade) && COMMON_SKILL_VALID(SID_SpellBlade))
	[SID_SpellBlade] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_SpellBlade_NAME,
		.helpMsgId = MSG_SKILL_SpellBlade_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = SpellBlade_Usability,
		.onDraw = NULL,
		.onSelected = SpellBlade_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = SpellBlade_Hover,
		.onSwitchOut = SpellBlade_Unhover
	},
#endif

#if (defined(SID_FocusEnergy) && COMMON_SKILL_VALID(SID_FocusEnergy))
	[SID_FocusEnergy] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_FocusEnergy_NAME,
		.helpMsgId = MSG_SKILL_FocusEnergy_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = FocusEnergy_Usability,
		.onDraw = NULL,
		.onSelected = FocusEnergy_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = FocusEnergy_Hover,
		.onSwitchOut = FocusEnergy_Unhover
	},
#endif

#if (defined(SID_GorillaTactics) && COMMON_SKILL_VALID(SID_GorillaTactics))
	[SID_GorillaTactics] = &(const struct MenuItemDef) {
		.name = "　",
		.nameMsgId = MSG_SKILL_GorillaTactics_NAME,
		.helpMsgId = MSG_SKILL_GorillaTactics_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = GorillaTactics_Usability,
		.onDraw = NULL,
		.onSelected = GorillaTactics_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_GraceOfFire) && COMMON_SKILL_VALID(SID_GraceOfFire))
	[SID_GraceOfFire] = &(const struct MenuItemDef) {
		.name = "　神裁の恩寵",
		.nameMsgId = MSG_SKILL_GraceOfFire_NAME,
		.helpMsgId = MSG_SKILL_GraceOfFire_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = GraceOfFire_Usability,
		.onDraw = NULL,
		.onSelected = GraceOfFire_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_GraceOfWater) && COMMON_SKILL_VALID(SID_GraceOfWater))
	[SID_GraceOfWater] = &(const struct MenuItemDef) {
		.name = "　神裁の恩寵",
		.nameMsgId = MSG_SKILL_GraceOfWater_NAME,
		.helpMsgId = MSG_SKILL_GraceOfWater_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = GraceOfWater_Usability,
		.onDraw = NULL,
		.onSelected = GraceOfWater_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Swap) && COMMON_SKILL_VALID(SID_Swap))
    [SID_Swap] = &(const struct MenuItemDef) {
        .name = "　交換",
        .nameMsgId = MSG_SKILL_Swap_NAME,
        .helpMsgId = MSG_SKILL_Swap_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Swap_Usability,
        .onDraw = NULL,
        .onSelected = Swap_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Transform) && COMMON_SKILL_VALID(SID_Transform))
    [SID_Transform] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Transform_NAME,
        .helpMsgId = MSG_SKILL_Transform_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Transform_Usability,
        .onDraw = NULL,
        .onSelected = Transform_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_DrawBack) && COMMON_SKILL_VALID(SID_DrawBack))
    [SID_DrawBack] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_DrawBack_NAME,
        .helpMsgId = MSG_SKILL_DrawBack_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = DrawBack_Usability,
        .onDraw = NULL,
        .onSelected = DrawBack_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Pivot) && COMMON_SKILL_VALID(SID_Pivot))
    [SID_Pivot] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Pivot_NAME,
        .helpMsgId = MSG_SKILL_Pivot_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Pivot_Usability,
        .onDraw = NULL,
        .onSelected = Pivot_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Reposition) && COMMON_SKILL_VALID(SID_Reposition))
    [SID_Reposition] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Reposition_NAME,
        .helpMsgId = MSG_SKILL_Reposition_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Reposition_Usability,
        .onDraw = NULL,
        .onSelected = Reposition_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Shove) && COMMON_SKILL_VALID(SID_Shove))
    [SID_Shove] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Shove_NAME,
        .helpMsgId = MSG_SKILL_Shove_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Shove_Usability,
        .onDraw = NULL,
        .onSelected = Shove_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Smite) && COMMON_SKILL_VALID(SID_Smite))
    [SID_Smite] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Smite_NAME,
        .helpMsgId = MSG_SKILL_Smite_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Smite_Usability,
        .onDraw = NULL,
        .onSelected = Smite_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ArdentSacrifice) && COMMON_SKILL_VALID(SID_ArdentSacrifice))
    [SID_ArdentSacrifice] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_ArdentSacrifice_NAME,
        .helpMsgId = MSG_SKILL_ArdentSacrifice_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ArdentSacrifice_Usability,
        .onDraw = NULL,
        .onSelected = ArdentSacrifice_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ReciprocalAid) && COMMON_SKILL_VALID(SID_ReciprocalAid))
    [SID_ReciprocalAid] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_ReciprocalAid_NAME,
        .helpMsgId = MSG_SKILL_ReciprocalAid_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ReciprocalAid_Usability,
        .onDraw = NULL,
        .onSelected = ReciprocalAid_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Sacrifice) && COMMON_SKILL_VALID(SID_Sacrifice))
    [SID_Sacrifice] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Sacrifice_NAME,
        .helpMsgId = MSG_SKILL_Sacrifice_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Sacrifice_Usability,
        .onDraw = NULL,
        .onSelected = Sacrifice_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Capture) && COMMON_SKILL_VALID(SID_Capture))
    [SID_Capture] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Capture_NAME,
        .helpMsgId = MSG_SKILL_Capture_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Capture_Usability,
        .onDraw = NULL,
        .onSelected = Capture_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Doppleganger) && COMMON_SKILL_VALID(SID_Doppleganger))
    [SID_Doppleganger] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Doppleganger_NAME,
        .helpMsgId = MSG_SKILL_Doppleganger_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Doppleganger_Usability,
        .onDraw = NULL,
        .onSelected = Doppleganger_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Dismount) && COMMON_SKILL_VALID(SID_Dismount))
    [SID_Dismount] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Dismount_NAME,
        .helpMsgId = MSG_SKILL_Dismount_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Dismount_Usability,
        .onDraw = NULL,
        .onSelected = Dismount_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Entrepreneur) && COMMON_SKILL_VALID(SID_Entrepreneur))
    [SID_Entrepreneur] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Entrepreneur_NAME,
        .helpMsgId = MSG_SKILL_Entrepreneur_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Entrepreneur_Usability,
        .onDraw = NULL,
        .onSelected = Entrepreneur_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Arise) && COMMON_SKILL_VALID(SID_Arise))
    [SID_Arise] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Arise_NAME,
        .helpMsgId = MSG_SKILL_Arise_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Arise_Usability,
        .onDraw = NULL,
        .onSelected = Arise_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_AssignDecoy) && COMMON_SKILL_VALID(SID_AssignDecoy))
    [SID_AssignDecoy] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_AssignDecoy_NAME,
        .helpMsgId = MSG_SKILL_AssignDecoy_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = AssignDecoy_Usability,
        .onDraw = NULL,
        .onSelected = AssignDecoy_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_AssignDecoyPlus) && COMMON_SKILL_VALID(SID_AssignDecoyPlus))
    [SID_AssignDecoyPlus] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_AssignDecoyPlus_NAME,
        .helpMsgId = MSG_SKILL_AssignDecoyPlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = AssignDecoyPlus_Usability,
        .onDraw = NULL,
        .onSelected = AssignDecoyPlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ShadowFlash) && COMMON_SKILL_VALID(SID_ShadowFlash))
    [SID_ShadowFlash] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_ShadowFlash_NAME,
        .helpMsgId = MSG_SKILL_ShadowFlash_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ShadowFlash_Usability,
        .onDraw = NULL,
        .onSelected = ShadowFlash_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = ShadowFlash_Hover,
        .onSwitchOut = ShadowFlash_Unhover,
    },
#endif

#if (defined(SID_DeathBlight) && COMMON_SKILL_VALID(SID_DeathBlight))
    [SID_DeathBlight] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_DeathBlight_NAME,
        .helpMsgId = MSG_SKILL_DeathBlight_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = DeathBlight_Usability,
        .onDraw = NULL,
        .onSelected = DeathBlight_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_LoadstarRush) && COMMON_SKILL_VALID(SID_LoadstarRush))
    [SID_LoadstarRush] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_LoadstarRush_NAME,
        .helpMsgId = MSG_SKILL_LoadstarRush_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = LoadstarRush_Usability,
        .onDraw = NULL,
        .onSelected = LoadstarRush_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_WyvernCrash) && COMMON_SKILL_VALID(SID_WyvernCrash))
    [SID_WyvernCrash] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_WyvernCrash_NAME,
        .helpMsgId = MSG_SKILL_WyvernCrash_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = WyvernCrash_Usability,
        .onDraw = NULL,
        .onSelected = WyvernCrash_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Sellsword) && COMMON_SKILL_VALID(SID_Sellsword))
    [SID_Sellsword] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_Sellsword_NAME,
        .helpMsgId = MSG_SKILL_Sellsword_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Sellsword_Usability,
        .onDraw = NULL,
        .onSelected = Sellsword_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Switcher) && COMMON_SKILL_VALID(SID_Switcher))
    [SID_Switcher] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_Switcher_NAME,
        .helpMsgId = MSG_SKILL_Switcher_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Switcher_Usability,
        .onDraw = NULL,
        .onSelected = Switcher_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_SummonPlus) && COMMON_SKILL_VALID(SID_SummonPlus))
    [SID_SummonPlus] = &(const struct MenuItemDef) {
        .name = "　召喚",
        .nameMsgId = MSG_SKILL_SummonPlus_NAME,
        .helpMsgId = MSG_SKILL_SummonPlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = SummonCommandUsability,
        .onDraw = NULL,
        .onSelected = SummonPlusCommandEffect,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL
    },
#endif

#if (defined(SID_LethalityPlus) && COMMON_SKILL_VALID(SID_LethalityPlus))
    [SID_LethalityPlus] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_LethalityPlus_NAME,
        .helpMsgId = MSG_SKILL_LethalityPlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = LethalityPlus_Usability,
        .onDraw = NULL,
        .onSelected = LethalityPlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_GrimReaper) && COMMON_SKILL_VALID(SID_GrimReaper))
    [SID_GrimReaper] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_GrimReaper_NAME,
        .helpMsgId = MSG_SKILL_GrimReaper_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = GrimReaper_Usability,
        .onDraw = NULL,
        .onSelected = GrimReaper_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Hide) && COMMON_SKILL_VALID(SID_Hide))
    [SID_Hide] = &(const struct MenuItemDef) {
        .name = "　光の結界",
        .nameMsgId = MSG_SKILL_Hide_NAME,
        .helpMsgId = MSG_SKILL_Hide_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Hide_Usability,
        .onDraw = NULL,
        .onSelected = Hide_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_SkillSwap) && COMMON_SKILL_VALID(SID_SkillSwap))
    [SID_SkillSwap] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_SkillSwap_NAME,
        .helpMsgId = MSG_SKILL_SkillSwap_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = SkillSwap_Usability,
        .onDraw = NULL,
        .onSelected = SkillSwap_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Shuffle) && COMMON_SKILL_VALID(SID_Shuffle))
    [SID_Shuffle] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Shuffle_NAME,
        .helpMsgId = MSG_SKILL_Shuffle_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Shuffle_Usability,
        .onDraw = NULL,
        .onSelected = Shuffle_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ShufflePlus) && COMMON_SKILL_VALID(SID_ShufflePlus))
    [SID_ShufflePlus] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_ShufflePlus_NAME,
        .helpMsgId = MSG_SKILL_ShufflePlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ShufflePlus_Usability,
        .onDraw = NULL,
        .onSelected = ShufflePlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Persuade) && COMMON_SKILL_VALID(SID_Persuade))
    [SID_Persuade] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Persuade_NAME,
        .helpMsgId = MSG_SKILL_Persuade_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Persuade_Usability,
        .onDraw = NULL,
        .onSelected = Persuade_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_PersuadePlus) && COMMON_SKILL_VALID(SID_PersuadePlus))
    [SID_PersuadePlus] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_PersuadePlus_NAME,
        .helpMsgId = MSG_SKILL_PersuadePlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = PersuadePlus_Usability,
        .onDraw = NULL,
        .onSelected = PersuadePlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Kamikaze) && COMMON_SKILL_VALID(SID_Kamikaze))
    [SID_Kamikaze] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Kamikaze_NAME,
        .helpMsgId = MSG_SKILL_Kamikaze_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Kamikaze_Usability,
        .onDraw = NULL,
        .onSelected = Kamikaze_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = Kamikaze_Hover,
        .onSwitchOut = Kamikaze_Unhover,
    },
#endif

#if (defined(SID_Reinforcements) && COMMON_SKILL_VALID(SID_Reinforcements))
    [SID_Reinforcements] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Reinforcements_NAME,
        .helpMsgId = MSG_SKILL_Reinforcements_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Reinforcements_Usability,
        .onDraw = NULL,
        .onSelected = Reinforcements_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ResolvedHeart) && COMMON_SKILL_VALID(SID_ResolvedHeart))
    [SID_ResolvedHeart] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_ResolvedHeart_NAME,
        .helpMsgId = MSG_SKILL_ResolvedHeart_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ResolvedHeart_Usability,
        .onDraw = NULL,
        .onSelected = ResolvedHeart_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Ballistary) && COMMON_SKILL_VALID(SID_Ballistary))
    [SID_Ballistary] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Ballistary_NAME,
        .helpMsgId = MSG_SKILL_Ballistary_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Ballistary_Usability,
        .onDraw = NULL,
        .onSelected = Ballistary_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Bide) && COMMON_SKILL_VALID(SID_Bide))
    [SID_Bide] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Bide_NAME,
        .helpMsgId = MSG_SKILL_Bide_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Bide_Usability,
        .onDraw = NULL,
        .onSelected = Bide_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_CoinFlip) && COMMON_SKILL_VALID(SID_CoinFlip))
    [SID_CoinFlip] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_CoinFlip_NAME,
        .helpMsgId = MSG_SKILL_CoinFlip_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = CoinFlip_Usability,
        .onDraw = NULL,
        .onSelected = CoinFlip_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Geomancy) && COMMON_SKILL_VALID(SID_Geomancy))
    [SID_Geomancy] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Geomancy_NAME,
        .helpMsgId = MSG_SKILL_Geomancy_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Geomancy_Usability,
        .onDraw = NULL,
        .onSelected = Geomancy_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_ScrollScribePlus) && COMMON_SKILL_VALID(SID_ScrollScribePlus))
    [SID_ScrollScribePlus] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_ScrollScribePlus_NAME,
        .helpMsgId = MSG_SKILL_ScrollScribePlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = ScrollScribePlus_Usability,
        .onDraw = NULL,
        .onSelected = ScrollScribePlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Acidic) && COMMON_SKILL_VALID(SID_Acidic))
    [SID_Acidic] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Acidic_NAME,
        .helpMsgId = MSG_SKILL_Acidic_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Acidic_Usability,
        .onDraw = NULL,
        .onSelected = Acidic_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_AcidicPlus) && COMMON_SKILL_VALID(SID_AcidicPlus))
    [SID_AcidicPlus] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_AcidicPlus_NAME,
        .helpMsgId = MSG_SKILL_AcidicPlus_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = AcidicPlus_Usability,
        .onDraw = NULL,
        .onSelected = AcidicPlus_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Reinforce) && COMMON_SKILL_VALID(SID_Reinforce))
    [SID_Reinforce] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Reinforce_NAME,
        .helpMsgId = MSG_SKILL_Reinforce_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Reinforce_Usability,
        .onDraw = NULL,
        .onSelected = Reinforce_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_SealedZone) && COMMON_SKILL_VALID(SID_SealedZone))
    [SID_SealedZone] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_SealedZone_NAME,
        .helpMsgId = MSG_SKILL_SealedZone_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = SealedZone_Usability,
        .onDraw = NULL,
        .onSelected = SealedZone_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Thrust) && COMMON_SKILL_VALID(SID_Thrust))
    [SID_Thrust] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Thrust_NAME,
        .helpMsgId = MSG_SKILL_Thrust_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Thrust_Usability,
        .onDraw = NULL,
        .onSelected = Thrust_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_DoubleTime) && COMMON_SKILL_VALID(SID_DoubleTime))
    [SID_DoubleTime] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_DoubleTime_NAME,
        .helpMsgId = MSG_SKILL_DoubleTime_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = DoubleTime_Usability,
        .onDraw = NULL,
        .onSelected = DoubleTime_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_BloodyAlchemy) && COMMON_SKILL_VALID(SID_BloodyAlchemy))
	[SID_BloodyAlchemy] = &(const struct MenuItemDef) {
		.name = "　神裁の恩寵",
		.nameMsgId = MSG_SKILL_BloodyAlchemy_NAME,
		.helpMsgId = MSG_SKILL_BloodyAlchemy_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = BloodyAlchemy_Usability,
		.onDraw = NULL,
		.onSelected = BloodyAlchemy_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_GateOfBabylon) && COMMON_SKILL_VALID(SID_GateOfBabylon))
	[SID_GateOfBabylon] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_GateOfBabylon_NAME,
		.helpMsgId = MSG_SKILL_GateOfBabylon_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = YobimaCommandUsability,
		.onDraw = NULL,
		.onSelected = YobimaCommandEffect,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_EmergencyExit) && COMMON_SKILL_VALID(SID_EmergencyExit))
	[SID_EmergencyExit] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_EmergencyExit_NAME,
		.helpMsgId = MSG_SKILL_EmergencyExit_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = EmergencyExit_Usability,
		.onDraw = NULL,
		.onSelected = EmergencyExit_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_EmergencyExitPlus) && COMMON_SKILL_VALID(SID_EmergencyExitPlus))
	[SID_EmergencyExitPlus] = &(const struct MenuItemDef) {
		.name = "　召喚",
		.nameMsgId = MSG_SKILL_EmergencyExitPlus_NAME,
		.helpMsgId = MSG_SKILL_EmergencyExitPlus_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = EmergencyExitPlus_Usability,
		.onDraw = NULL,
		.onSelected = EmergencyExitPlus_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL
	},
#endif

#if (defined(SID_Songstress) && COMMON_SKILL_VALID(SID_Songstress))
	[SID_Songstress] = &(const struct MenuItemDef) {
		.name = "　女神の踊り",
		.nameMsgId = MSG_SKILL_Songstress_NAME,
		.helpMsgId = MSG_SKILL_Songstress_DESC,
		.color = TEXT_COLOR_SYSTEM_GOLD,
		.isAvailable = Songstress_Usability,
		.onDraw = NULL,
		.onSelected = Songstress_OnSelected,
		.onIdle = NULL,
		.onSwitchIn = NULL,
		.onSwitchOut = NULL,
	},
#endif

#if (defined(SID_Gungnir) && COMMON_SKILL_VALID(SID_Gungnir))
    [SID_Gungnir] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Gungnir_NAME,
        .helpMsgId = MSG_SKILL_Gungnir_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Gungnir_Usability,
        .onDraw = NULL,
        .onSelected = Gungnir_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

#if (defined(SID_Transcendence) && COMMON_SKILL_VALID(SID_Transcendence))
    [SID_Transcendence] = &(const struct MenuItemDef) {
        .name = "　",
        .nameMsgId = MSG_SKILL_Transcendence_NAME,
        .helpMsgId = MSG_SKILL_Transcendence_DESC,
        .color = TEXT_COLOR_SYSTEM_GOLD,
        .isAvailable = Transcendence_Usability,
        .onDraw = NULL,
        .onSelected = Transcendence_OnSelected,
        .onIdle = NULL,
        .onSwitchIn = NULL,
        .onSwitchOut = NULL,
    },
#endif

};
