#pragma once

#include "common-chax.h"

extern char sWmManageSkillsEmpty[2];
extern u16 sWmManageSkillsMode;

bool WMManageSkills_IsMode(void);
void WMManageSkills_SetMode(bool active);

void WMManageSkills_DrawPrepSkillRows(u16 *tm, struct Text *th, struct Unit *unit, int rowCount);

/* Text override functions */
int WMManageSkills_GetChooseUnitLabelMsgId(void);
int WMManageSkills_GetOptionRemoveAllMsgId(void);
int WMManageSkills_GetOptionEditSkillsMsgId(void);

/* Action handlers */
void WMManageSkills_RemoveAllEquippedSkills(struct Unit *unit);
void WMManageSkills_LaunchEditSkillsScreen(struct Unit *unit);
void WMManageSkills_HandlePopupOptionSelected(struct Unit *unit, int optionIndex);
