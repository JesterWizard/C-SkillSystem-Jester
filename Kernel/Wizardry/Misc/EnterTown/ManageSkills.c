#include "common-chax.h"
#include "kernel/manage-skills.h"
#include "kernel/skill-system.h"
#include "icon-rework.h"
#include "utf8.h"

bool WMManageSkills_IsMode(void)
{
    return (sWmManageSkillsMode != 0);
}

void WMManageSkills_SetMode(bool active)
{
    sWmManageSkillsMode = active ? 1 : 0;
}

void WMManageSkills_DrawPrepSkillRows(u16 *tm, struct Text *th, struct Unit *unit, int rowCount)
{
    int i;

    sWmManageSkillsEmpty[0] = '-';
    sWmManageSkillsEmpty[1] = 0;

    if (rowCount > 5)
        rowCount = 5;

    for (i = 0; i < rowCount; i++) {
        const int sid = GET_SKILL(unit, i);
        const int tmOff = i * 0x40;

        TileMap_FillRect(tm + tmOff, 12, 1, 0);
        ClearText(th + i);

        if (EQUIP_SKILL_VALID(sid)) {
            PutDrawText(
                th + i,
                tm + tmOff + 2,
                TEXT_COLOR_SYSTEM_WHITE,
                0,
                0,
                Utf8ToNarrowFonts(GetSkillNameStr_NormalFont(sid))
            );

            DrawIcon(tm + tmOff, SKILL_ICON(sid), 0x4000);
        } else {
            PutDrawText(
                th + i,
                tm + tmOff + 2,
                TEXT_COLOR_SYSTEM_GRAY,
                0,
                0,
                sWmManageSkillsEmpty
            );
        }
    }
}

/* Text override functions for manage-skills mode */
int WMManageSkills_GetChooseUnitLabelMsgId(void)
{
    /* TODO: Once text constants are properly generated,
     * return MSG_WM_MANAGE_SKILLS_CHOOSE_UNIT or MSG_PREP_SCREEN_LABEL_CHOOSE_UNIT based on mode */
    return 0;  /* 0 = default/vanilla behavior */
}

int WMManageSkills_GetOptionRemoveAllMsgId(void)
{
    /* TODO: Return MSG_WM_MANAGE_SKILLS_OPTION_REMOVE_ALL once generated */
    return 0;
}

int WMManageSkills_GetOptionEditSkillsMsgId(void)
{
    /* TODO: Return MSG_WM_MANAGE_SKILLS_OPTION_EDIT once generated */
    return 0;
}

/* Action handlers */
void WMManageSkills_RemoveAllEquippedSkills(struct Unit *unit)
{
    int i;
    
    if (!unit)
        return;

    /* Clear all equipped skills (up to 5 slots) */
    for (i = 0; i < 5; i++) {
        SET_SKILL(unit, i, 0);
    }
}

void WMManageSkills_LaunchEditSkillsScreen(struct Unit *unit)
{
    /* TODO: Launch the full PrepSkill2 equip/remove screen from manage-skills context */
    /* For now, this is a placeholder that should integrate with the existing prep-skill framework */
    if (!unit)
        return;
}
