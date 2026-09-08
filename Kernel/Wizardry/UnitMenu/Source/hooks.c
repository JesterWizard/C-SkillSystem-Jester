#include "common-chax.h"
#include "combat-art.h"
#include "skill-system.h"

u8 UnitActionMenu_OnHelpBoxRe(struct MenuProc *menu, struct MenuItemProc *item)
{
	return MenuSkills_OnHelpBox(menu, item);
}

u8 UnitActionMenu_CancelRe(struct MenuProc* menu, struct MenuItemProc* item)
{
	ResetCombatArtStatus();
	ResetSkillLists();
	ResetCombatArtList();

    /*
    ** So we reset this value here after exiting the forge menu.
    ** This way, the left and right dPad buttons do not take effect in other menus
    */
    
    if (gpKernelDesignerConfig->forge_mechanic == true)
    {
        if (gActionData.unk08 == 10000)
        {
            gActionData.unk08 = 0;
        }
    }

    return MENU_ACT_SKIPCURSOR | MENU_ACT_CLEAR | MENU_ACT_END | MENU_ACT_SND6B;

	// return MenuCancelSelect(menu, item);
}
