#include "common-chax.h"
#include "AiAction.h"

const struct AiActionConf gAiActionTable[CONFIG_AI_ACTION_AMT] = {
	[AI_ACTION_NONE] = {
		.exec = NULL,
		.idle = AiDummyAction,
	},
	[AI_ACTION_COMBAT] = {
		.exec = AiStartCombatAction,
		.idle = AiDummyAction,
	},
	[AI_ACTION_ESCAPE] = {
		.exec = AiStartEscapeAction,
		.idle = AiEscapeAction,
	},
	[AI_ACTION_STEAL] = {
		.exec = AiStartStealAction,
		.idle = AiWaitAndClearScreenAction,
	},
	[AI_ACTION_PILLAGE] = {
		.exec = NULL,
		.idle = AiPillageAction,
	},
	[AI_ACTION_STAFF] = {
		.exec = NULL,
		.idle = AiStaffAction,
	},
	[AI_ACTION_USEITEM] = {
		.exec = NULL,
		.idle = AiUseItemAction,
	},
	[CONFIG_AI_ACTION_EXPA_Rescue] = {
		.exec = AiStartRescueAction,
		.idle = AiDummyAction,
	},
	[CONFIG_AI_ACTION_EXPA_Drop] = {
		.exec = AiStartDropAction,
		.idle = AiDummyAction,
	},
	[AI_ACTION_REFRESH] = {
		.exec = AiAction_RefreshDance,
		.idle = AiDummyAction,
	},
	[AI_ACTION_TALK] = {
		.exec = NULL,
		.idle = AiTalkAction,
	},
	[AI_ACTION_RIDEBALLISTA] = {
		.exec = NULL,
		.idle = AiRideBallistaAction,
	},
	[AI_ACTION_EXITBALLISTA] = {
		.exec = NULL,
		.idle = AiExitBallistaAction,
	},
	[AI_ACTION_DKNIGHTMARE] = {
		.exec = AiDKNightmareAction,
		.idle = AiDummyAction,
	},
	[AI_ACTION_DKSUMMON] = {
		.exec = AiDKSummonAction,
		.idle = AiDummyAction,
	},
	[AI_ACTION_PICK] = {
		.exec = NULL,
		.idle = AiPickAction,
	},
	[CONFIG_AI_ACTION_EXPA_MenuSkill] = {
		.exec = AiAction_MenuSkill,
		.idle = AiAction_MenuSkillIdle,
	},
};
