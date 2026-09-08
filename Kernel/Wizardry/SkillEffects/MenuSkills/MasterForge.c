#include "common-chax.h"
#include "playst-expa.h"
#include "map-anims.h"
#include "skill-system.h"
#include "battle-system.h"
#include "action-expa.h"
#include "constants/skills.h"
#include "constants/texts.h"

extern u16 gUnknown_085A0D4C[];

#if defined(SID_MasterForge) && (COMMON_SKILL_VALID(SID_MasterForge))

static const struct MenuItemDef GenerateUnitItems[];
static u8 MasterForge_HelpBox(struct MenuProc * menu, struct MenuItemProc * item);
static u8 MasterForge_OnCancel(struct MenuProc * menu, struct MenuItemProc * item);
static int MasterForge_OnDraw(struct MenuProc * menu, struct MenuItemProc * item);
static u8 MasterForge_OnMenuSelect(struct MenuProc * menu, struct MenuItemProc * item);

static u8 MasterForge_HelpBox(struct MenuProc * menu, struct MenuItemProc * item)
{
 	// const u8 menuIndex = MENU_SKILL_INDEX(item->def);

	// /* 
	// ** Currently only the help text for each item displays.
	// ** Getting the stats to display in the help box is difficult
	// ** as it requires some sort of linking structs like those in Statscreen.c 
	// */
    // StartHelpBox(
    //     item->xTile * 8,
    //     item->yTile * 8,
    //     GetItemDescId(gActiveUnit->items[menuIndex])
    // );

    return 0;
}

static u8 MasterForge_OnCancel(struct MenuProc * menu, struct MenuItemProc * item)
{
    /* Reset action */
    gActionData.unitActionType = 0;

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);
    HideMoveRangeGraphics();

    gActionDataExpa.refrain_action = true;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 MasterForge_Usability(const struct MenuItemDef *def, int number)
{
    int partyGold = GetPartyGoldAmount();
    int slot = def->helpMsgId; // Use the slot index from the menu item
    u16 weapon = gActiveUnit->items[slot]; // Check the specific slot

    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (!weapon) // No item in this slot
        return MENU_NOTSHOWN;

    if (partyGold == 0)
        return MENU_NOTSHOWN;

    if (GetItemAttributes(weapon) & IA_UNBREAKABLE)
        return MENU_NOTSHOWN;

    // If not a weapon, do not display
    if (!GetItemAttributes(weapon)) {
        return MENU_NOTSHOWN;
    }

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_MasterForge_Used))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

static int MasterForge_OnDraw(struct MenuProc * menu, struct MenuItemProc * item)
{
    const u8 menuIndex = item->def->helpMsgId;
    u16 itemInSlot = gActiveUnit->items[menuIndex];
    
    // Don't draw anything for empty slots (they should be MENU_NOTSHOWN anyway)
    if (!itemInSlot)
        return 0;

    gActionData.unk08 = SID_MasterForge;

    Text_SetColor(&item->text, TEXT_COLOR_SYSTEM_WHITE);

    CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);

    Text_DrawString(&item->text, GetItemName(itemInSlot));

    ResetIconGraphics();
	LoadIconPalettes(4);

    DrawIcon(gBG0TilemapBuffer + TILEMAP_INDEX(item->xTile, item->yTile), GetItemIconId(itemInSlot), 0x4000);
    PutText(&item->text, TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 2, item->yTile));
	PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 11, item->yTile), TEXT_COLOR_SYSTEM_BLUE, GetItemUses(itemInSlot));

    PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, gActiveUnit->pCharacterData->portraitId, 0x200, 5);

    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return 0;
}


u8 MasterForge_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_MasterForge;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 MasterForge_OnMenuSelect(struct MenuProc * menu, struct MenuItemProc * item)
{
	const u8 menuIndex = MENU_SKILL_INDEX(item->def);
	u16* weapon = &gActiveUnit->items[menuIndex];

	// Extract the item ID
	u8 itemID = *weapon & 0xFF;

	// Set the uses to 255 and combine it with the item ID
	u16 newValue = (255 << 8) | itemID;

	// Update the weapon in the unit's inventory
	*weapon = newValue;

	SetPartyGoldAmount(0);

	NewPopup_VerySimple(MSG_AllGoldSpent, 0x5A, Proc_Find(gProcScr_PlayerPhase));

	PlayStExpa_SetBit(PLAYSTEXPA_BIT_MasterForge_Used);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

#define GenerateUnitItem(i) \
{ \
    .helpMsgId = (i), \
    .isAvailable = MasterForge_Usability, \
    .onDraw = MasterForge_OnDraw, \
    .onSelected = MasterForge_OnMenuSelect, \
}

static const struct MenuItemDef GenerateUnitItems[] = 
{
    GenerateUnitItem(0),
    GenerateUnitItem(1),
    GenerateUnitItem(2),
    GenerateUnitItem(3),
    GenerateUnitItem(4),
    { 0 }
};

const struct MenuDef gSelectInfo_MasterForge = {
    {15, 1, 15, 0},
    0,
    GenerateUnitItems,
    0, 0, 0,
    MasterForge_OnCancel,
    MenuAutoHelpBoxSelect,
    MasterForge_HelpBox
};

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
    StartSubtitleHelp(
        StartOrphanMenu(&gSelectInfo_MasterForge),
        GetStringFromIndex(MSG_MasterForgeSelect)
    );
}

bool Action_MasterForge(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif
