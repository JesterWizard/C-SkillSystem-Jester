#pragma once

#include "common-chax.h"

struct AiActionConf {
	void (*exec)(struct CpPerformProc *proc);
	bool (*idle)(struct CpPerformProc *proc);
};

void AiAction_MenuSkill(struct CpPerformProc *proc);
bool AiAction_MenuSkillIdle(struct CpPerformProc *proc);
void AiAction_RefreshDance(struct CpPerformProc *proc);
void AiStartRescueAction(struct CpPerformProc *proc);
void AiStartDropAction(struct CpPerformProc *proc);

// extern const struct AiActionConf gAiActionTable[];
extern struct AiActionConf const *const gpAiActionTable;
