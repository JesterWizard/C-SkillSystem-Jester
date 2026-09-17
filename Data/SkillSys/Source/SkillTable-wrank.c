#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

#define WRANK_SKILL(_sid, _replaces) { .sid = (_sid), .replaces = (_replaces) }

const struct SkillWRankRomTable gSkillWRankTable = {
	.skills = {
#if (defined(SID_SwordProwess1) && COMMON_SKILL_VALID(SID_SwordProwess1))
		[ITYPE_SWORD] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_SwordProwess1, 0) },
#if (defined(SID_SwordProwess2) && COMMON_SKILL_VALID(SID_SwordProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_SwordProwess2, SID_SwordProwess1) },
#endif
#if (defined(SID_SwordProwess3) && COMMON_SKILL_VALID(SID_SwordProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_SwordProwess3, SID_SwordProwess2) },
#endif
#if (defined(SID_SwordProwess4) && COMMON_SKILL_VALID(SID_SwordProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_SwordProwess4, SID_SwordProwess3) },
#endif
#if (defined(SID_SwordProwess5) && COMMON_SKILL_VALID(SID_SwordProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_SwordProwess5, SID_SwordProwess4) },
#endif
		},
#endif

#if (defined(SID_LanceProwess1) && COMMON_SKILL_VALID(SID_LanceProwess1))
		[ITYPE_LANCE] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_LanceProwess1, 0) },
#if (defined(SID_LanceProwess2) && COMMON_SKILL_VALID(SID_LanceProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_LanceProwess2, SID_LanceProwess1) },
#endif
#if (defined(SID_LanceProwess3) && COMMON_SKILL_VALID(SID_LanceProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_LanceProwess3, SID_LanceProwess2) },
#endif
#if (defined(SID_LanceProwess4) && COMMON_SKILL_VALID(SID_LanceProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_LanceProwess4, SID_LanceProwess3) },
#endif
#if (defined(SID_LanceProwess5) && COMMON_SKILL_VALID(SID_LanceProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_LanceProwess5, SID_LanceProwess4) },
#endif
		},
#endif

#if (defined(SID_AxeProwess1) && COMMON_SKILL_VALID(SID_AxeProwess1))
		[ITYPE_AXE] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_AxeProwess1, 0) },
#if (defined(SID_AxeProwess2) && COMMON_SKILL_VALID(SID_AxeProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_AxeProwess2, SID_AxeProwess1) },
#endif
#if (defined(SID_AxeProwess3) && COMMON_SKILL_VALID(SID_AxeProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_AxeProwess3, SID_AxeProwess2) },
#endif
#if (defined(SID_AxeProwess4) && COMMON_SKILL_VALID(SID_AxeProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_AxeProwess4, SID_AxeProwess3) },
#endif
#if (defined(SID_AxeProwess5) && COMMON_SKILL_VALID(SID_AxeProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_AxeProwess5, SID_AxeProwess4) },
#endif
		},
#endif

#if (defined(SID_BowProwess1) && COMMON_SKILL_VALID(SID_BowProwess1))
		[ITYPE_BOW] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_BowProwess1, 0) },
#if (defined(SID_BowProwess2) && COMMON_SKILL_VALID(SID_BowProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_BowProwess2, SID_BowProwess1) },
#endif
#if (defined(SID_BowProwess3) && COMMON_SKILL_VALID(SID_BowProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_BowProwess3, SID_BowProwess2) },
#endif
#if (defined(SID_BowProwess4) && COMMON_SKILL_VALID(SID_BowProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_BowProwess4, SID_BowProwess3) },
#endif
#if (defined(SID_BowProwess5) && COMMON_SKILL_VALID(SID_BowProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_BowProwess5, SID_BowProwess4) },
#endif
		},
#endif

#if (defined(SID_AnimaProwess1) && COMMON_SKILL_VALID(SID_AnimaProwess1))
		[ITYPE_ANIMA] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_AnimaProwess1, 0) },
#if (defined(SID_AnimaProwess2) && COMMON_SKILL_VALID(SID_AnimaProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_AnimaProwess2, SID_AnimaProwess1) },
#endif
#if (defined(SID_AnimaProwess3) && COMMON_SKILL_VALID(SID_AnimaProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_AnimaProwess3, SID_AnimaProwess2) },
#endif
#if (defined(SID_AnimaProwess4) && COMMON_SKILL_VALID(SID_AnimaProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_AnimaProwess4, SID_AnimaProwess3) },
#endif
#if (defined(SID_AnimaProwess5) && COMMON_SKILL_VALID(SID_AnimaProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_AnimaProwess5, SID_AnimaProwess4) },
#endif
		},
#endif

#if (defined(SID_LightProwess1) && COMMON_SKILL_VALID(SID_LightProwess1))
		[ITYPE_LIGHT] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_LightProwess1, 0) },
#if (defined(SID_LightProwess2) && COMMON_SKILL_VALID(SID_LightProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_LightProwess2, SID_LightProwess1) },
#endif
#if (defined(SID_LightProwess3) && COMMON_SKILL_VALID(SID_LightProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_LightProwess3, SID_LightProwess2) },
#endif
#if (defined(SID_LightProwess4) && COMMON_SKILL_VALID(SID_LightProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_LightProwess4, SID_LightProwess3) },
#endif
#if (defined(SID_LightProwess5) && COMMON_SKILL_VALID(SID_LightProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_LightProwess5, SID_LightProwess4) },
#endif
		},
#endif

#if (defined(SID_DarkProwess1) && COMMON_SKILL_VALID(SID_DarkProwess1))
		[ITYPE_DARK] = {
			[WPN_LEVEL_D] = { WRANK_SKILL(SID_DarkProwess1, 0) },
#if (defined(SID_DarkProwess2) && COMMON_SKILL_VALID(SID_DarkProwess2))
			[WPN_LEVEL_C] = { WRANK_SKILL(SID_DarkProwess2, SID_DarkProwess1) },
#endif
#if (defined(SID_DarkProwess3) && COMMON_SKILL_VALID(SID_DarkProwess3))
			[WPN_LEVEL_B] = { WRANK_SKILL(SID_DarkProwess3, SID_DarkProwess2) },
#endif
#if (defined(SID_DarkProwess4) && COMMON_SKILL_VALID(SID_DarkProwess4))
			[WPN_LEVEL_A] = { WRANK_SKILL(SID_DarkProwess4, SID_DarkProwess3) },
#endif
#if (defined(SID_DarkProwess5) && COMMON_SKILL_VALID(SID_DarkProwess5))
			[WPN_LEVEL_S] = { WRANK_SKILL(SID_DarkProwess5, SID_DarkProwess4) },
#endif
		},
#endif
	},
};
