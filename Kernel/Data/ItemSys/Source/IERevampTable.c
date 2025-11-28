#include "common-chax.h"
#include "item-sys.h"

struct IERevamp const IERevampTable[IER_MAX] = {
	[IER_STAFF_HEAL] = {
		.usability      = IER_Usability_AdjacentHeal,
		.prep_usability = NULL,
		.effect         = IER_Effect_AdjacentHeal,
		.action_effect  = IER_Action_StandardHeal,
		.prep_effect    = NULL,
	},
	[IER_STAFF_MEND] = {
		.usability      = IER_Usability_AdjacentHeal,
		.prep_usability = NULL,
		.effect         = IER_Effect_AdjacentHeal,
		.action_effect  = IER_Action_StandardHeal,
		.prep_effect    = NULL,
	},
	[IER_STAFF_RECOVER] = {
		.usability      = IER_Usability_AdjacentHeal,
		.prep_usability = NULL,
		.effect         = IER_Effect_AdjacentHeal,
		.action_effect  = IER_Action_StandardHeal,
		.prep_effect    = NULL,
	},
	[IER_STAFF_PHYSIC] = {
		.usability      = IER_Usability_RangedHeal,
		.prep_usability = NULL,
		.effect         = IER_Effect_RangedHeal,
		.action_effect  = IER_Action_StandardHeal,
		.prep_effect    = NULL,
	},
	[IER_STAFF_FORTIFY] = {
		.usability      = IER_Usability_RangedHeal,
		.prep_usability = NULL,
		.effect         = IER_Effect_CommonStaff,
		.action_effect  = IER_Action_Fortify,
		.prep_effect    = NULL,
	},
	[IER_STAFF_RESTORE] = {
		.usability      = IER_Usability_Restore,
		.prep_usability = NULL,
		.effect         = IER_Effect_Restore,
		.action_effect  = IER_Action_Restore,
		.prep_effect    = NULL,
	},
	[IER_STAFF_SILENCE] = {
		.usability      = IER_Usability_Silence,
		.prep_usability = NULL,
		.effect         = IER_Effect_Silence,
		.action_effect  = IER_Action_StatusStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_SLEEP] = {
		.usability      = IER_Usability_Sleep,
		.prep_usability = NULL,
		.effect         = IER_Effect_Sleep,
		.action_effect  = IER_Action_StatusStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_BERSERK] = {
		.usability      = IER_Usability_Berserk,
		.prep_usability = NULL,
		.effect         = IER_Effect_Berserk,
		.action_effect  = IER_Action_StatusStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_WARP] = {
		.usability      = IER_Usability_Warp,
		.prep_usability = NULL,
		.effect         = IER_Effect_Warp,
		.action_effect  = IER_Action_WarpStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_RESCUE] = {
		.usability      = IER_Usability_Rescue,
		.prep_usability = NULL,
		.effect         = IER_Effect_RescueStaff,
		.action_effect  = IER_Action_RescueStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_TORCH] = {
		.usability      = IER_Usability_Torch,
		.prep_usability = NULL,
		.effect         = IER_Effect_Torch,
		.action_effect  = IER_Action_TorchStaff,
		.prep_effect    = NULL,
	},
	[IER_STAFF_REPAIR] = {
		.usability      = IER_Usability_Hammerne,
		.prep_usability = NULL,
		.effect         = IER_Effect_Repair,
		.action_effect  = IER_Action_Hammerne,
		.prep_effect    = NULL,
	},
	[IER_STAFF_UNLOCK] = {
		.usability      = IER_Usability_Unlock,
		.prep_usability = NULL,
		.effect         = IER_Effect_Unlock,
		.action_effect  = IER_Action_UnlockStaff,
		.prep_effect    = NULL,
	},
#ifdef CONFIG_ITEM_INDEX_FORCE_STAFF
	[IER_STAFF_FORCE] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_TEMPEST_STAFF
	[IER_STAFF_TEMPEST] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_ACUITY_STAFF
	[IER_STAFF_ACUITY] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_SPRINT_STAFF
	[IER_STAFF_SPRINT] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_FORTUNE_STAFF
	[IER_STAFF_FORTUNE] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_IRON_STAFF
	[IER_STAFF_IRON] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_OMNI_STAFF
	[IER_STAFF_OMNI] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#endif
	[IER_STAFF_BARRIER] = {
		.usability      = IER_Usability_Barrier,
		.prep_usability = NULL,
		.effect         = IER_Effect_Barrier,
		.action_effect  = IER_Action_BarrierStaff,
		.prep_effect    = NULL,
	},
#ifdef CONFIG_ITEM_INDEX_SLOW_STAFF
	[IER_STAFF_SLOW] = {
		.usability      = IER_Usability_Slow,
		.prep_usability = NULL,
		.effect         = IER_Effect_Slow,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_FORGE_STAFF
	[IER_STAFF_FORGE] = {
		.usability      = IER_Usability_Forge,
		.prep_usability = NULL,
		.effect         = IER_Effect_Forge,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_REWARP_STAFF
	[IER_STAFF_REWARP] = {
		.usability      = IER_Usability_Rewarp,
		.prep_usability = NULL,
		.effect         = IER_Effect_Rewarp,
		.action_effect  = IER_Action_Rewarp,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_POISON_STAFF
	[IER_STAFF_POISON] = {
		.usability      = IER_Usability_Poison,
		.prep_usability = NULL,
		.effect         = IER_Effect_Poison,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_DELAY_STAFF
	[IER_STAFF_DELAY] = {
		.usability      = IER_Usability_Delay,
		.prep_usability = NULL,
		.effect         = IER_Effect_Delay,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_ENTRAP_STAFF
	[IER_STAFF_ENTRAP] = {
		.usability      = IER_Usability_Entrap,
		.prep_usability = NULL,
		.effect         = IER_Effect_Entrap,
		.action_effect  = IER_Action_Entrap,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_QUICKEN_STAFF
	[IER_STAFF_QUICKEN] = {
		.usability      = IER_Usability_Quicken,
		.prep_usability = NULL,
		.effect         = IER_Effect_Quicken,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_HIDE_STAFF
	[IER_STAFF_HIDE] = {
		.usability      = IER_Usability_Hide,
		.prep_usability = NULL,
		.effect         = IER_Effect_Hide,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_PROVOKE_STAFF
	[IER_STAFF_PROVOKE] = {
		.usability      = IER_Usability_Provoke,
		.prep_usability = NULL,
		.effect         = IER_Effect_Provoke,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_PETRIFY_STAFF
	[IER_STAFF_PETRIFY] = {
		.usability      = IER_Usability_Petrify,
		.prep_usability = NULL,
		.effect         = IER_Effect_Petrify,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_SOOTH_STAFF
	[IER_STAFF_SOOTH] = {
		.usability      = IER_Usability_Sooth,
		.prep_usability = NULL,
		.effect         = IER_Effect_Sooth,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_ENFEEBLE_STAFF
	[IER_STAFF_ENFEEBLE] = {
		.usability      = IER_Usability_Enfeeble,
		.prep_usability = NULL,
		.effect         = IER_Effect_Enfeeble,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif
#ifdef CONFIG_ITEM_INDEX_INVEST_STAFF
	[IER_STAFF_INVEST] = {
		.usability      = IER_Usability_Invest,
		.prep_usability = NULL,
		.effect         = IER_Effect_Invest,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
#endif

	/**
	 * Boosters
	 */
	[IER_BOOSTER_HP] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_POW] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_SKL] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_SPD] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_LCK] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_DEF] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_RES] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_MOV] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},
	[IER_BOOSTER_CON] = {
		.usability      = IER_Usability_Booster,
		.prep_usability = IER_Usability_Booster,
		.effect         = NULL,
		.action_effect  = IER_Action_Booster,
		.prep_effect    = IER_PrepEffect_StatBooster,
	},

	/**
	 * Promotion
	 */
	[IER_HEROCREST] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_KNIGHTCREST] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_ORIONSBOLT] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_ELYSIANWHIP] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_GUIDINGRING] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},

	/**
	 * Key
	 */
	[IER_CHESTKEY] = {
		.usability      = IER_Usability_ChestKey,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_KeyItem,
		.prep_effect    = NULL,
	},
	[IER_DOORKEY] = {
		.usability      = IER_Usability_DoorKey,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_KeyItem,
		.prep_effect    = NULL,
	},
	[IER_LOCKPICK] = {
		.usability      = IER_Usability_LockPick,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_KeyItem,
		.prep_effect    = NULL,
	},

	/**
	 * Vulnerary
	 */
	[IER_VULNERARY] = {
		.usability      = IER_Usability_Vulnerary,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_VulneraryItem,
		.prep_effect    = NULL,
	},
	[IER_ELIXIR] = {
		.usability      = IER_Usability_Vulnerary,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_ElixirItem,
		.prep_effect    = NULL,
	},

	[IER_PUREWATER] = {
		.usability      = IER_Usability_PureWater,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_PureWaterItem,
		.prep_effect    = NULL,
	},
	[IER_ANTITOXIN] = {
		.usability      = IER_Usability_Antitoxin,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_AntitoxinItem,
		.prep_effect    = NULL,
	},
	[IER_TORCH] = {
		.usability      = IER_Usability_Torch,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_TorchItem,
		.prep_effect    = NULL,
	},
	[IER_CHESTKEY_BUNDLE] = {
		.usability      = IER_Usability_ChestKey,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_KeyItem,
		.prep_effect    = NULL,
	},
	[IER_MINE] = {
		.usability      = IER_Usability_Mine,
		.prep_usability = NULL,
		.effect         = IER_Effect_Mine,
		.action_effect  = IER_Action_Mine,
		.prep_effect    = NULL,
	},
	[IER_LIGHTRUNE] = {
		.usability      = IER_Usability_LightRune,
		.prep_usability = NULL,
		.effect         = IER_Effect_LightRune,
		.action_effect  = IER_Action_LightRune,
		.prep_effect    = NULL,
	},
#ifdef CONFIG_ITEM_INDEX_RUNE_STAFF
	[IER_STAFF_RUNE] = {
		.usability      = IER_Usability_LightRune,
		.prep_usability = NULL,
		.effect         = IER_Effect_LightRune,
		.action_effect  = IER_Action_LightRune,
		.prep_effect    = NULL,
	},
#endif
	[IER_FILLAS_MIGHT] = {
		.usability      = IER_Usability_DanceRing,
		.prep_usability = NULL,
		.effect         = IER_Effect_SpeicalDance,
		.action_effect  = IER_Action_DanceRing,
		.prep_effect    = NULL,
	},
	[IER_NINISS_GRACE] = {
		.usability      = IER_Usability_DanceRing,
		.prep_usability = NULL,
		.effect         = IER_Effect_SpeicalDance,
		.action_effect  = IER_Action_DanceRing,
		.prep_effect    = NULL,
	},
	[IER_THORS_IRE] = {
		.usability      = IER_Usability_DanceRing,
		.prep_usability = NULL,
		.effect         = IER_Effect_SpeicalDance,
		.action_effect  = IER_Action_DanceRing,
		.prep_effect    = NULL,
	},
	[IER_SETS_LITANY] = {
		.usability      = IER_Usability_DanceRing,
		.prep_usability = NULL,
		.effect         = IER_Effect_SpeicalDance,
		.action_effect  = IER_Action_DanceRing,
		.prep_effect    = NULL,
	},

	/**
	 * Promotion
	 */
	[IER_MASTERSEAL] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_METISSTOME] = {
		.usability      = IER_Usability_MetisStone,
		.prep_usability = IER_Usability_MetisStone,
		.effect         = NULL,
		.action_effect  = NULL,
		.prep_effect    = NULL,
	},
	[IER_HEAVENSEAL] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_OCEANSEAL] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_LUNARBRACE] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},
	[IER_SOLARBRACE] = {
		.usability      = IER_Usability_Promotion,
		.prep_usability = IER_Usability_Promotion,
		.effect         = NULL,
		.action_effect  = IER_Action_Promotion,
		.prep_effect    = IER_PrepEffect_Promotion,
	},

	/**
	 * Vulnerary
	 */
	[IER_VULNERARY_2] = {
		.usability      = IER_Usability_Vulnerary,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_VulneraryItem,
		.prep_effect    = NULL,
	},

	[IER_UNK_GREENNOTE] = {
		.usability      = NULL,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = NULL,
		.prep_effect    = NULL,
	},
	[IER_UNK_REDNOTE] = {
		.usability      = NULL,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = NULL,
		.prep_effect    = NULL,
	},
	[IER_JUNAFRUIT] = {
		.usability      = IER_Usability_JunaFruit,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_JunaFruitItem,
		.prep_effect    = IER_PrepEffect_JunaFruit,
	},
	[IER_CHAX_STAFF_LATONA] = {
		.usability      = IER_Usability_Latona,
		.prep_usability = NULL,
		.effect         = IER_Effect_CommonStaff,
		.action_effect  = IER_Action_Latona,
		.prep_effect    = NULL,
	},
	[IER_CHAX_SKILL_SCROLL] = {
		.usability      = IER_Usability_SkillScroll,
		.prep_usability = IER_PrepUsability_SkillScroll,
		.effect         = IER_Effect_SkillScroll,
		.action_effect  = IER_ActionEffect_SkillScroll,
		.prep_effect    = IER_PrepEffect_SkillScroll,
	},
	[IER_CHAX_STAFF_NIGHTMARE] = {
		.usability      = IER_Usability_NightMare,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_NightMare,
		.prep_effect    = NULL,
	},
#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL
	[IER_ARMS_SCROLL] = {
		.usability      = IER_Usability_ArmsScroll,
		.prep_usability = IER_PrepUsability_ArmsScroll,
		.effect         = NULL,
		.action_effect  = IER_ActionEffect_ArmsScroll,
		.prep_effect    = IER_PrepEffect_ArmsScroll,
	}
#endif
};
