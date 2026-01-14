#include "common-chax.h"
#include "skill-system.h"
#include "combat-art.h"
#include "gaiden-magic.h"
#include "constants/texts.h"
#include "bmmenu.h"

#define MenuItemsEnd {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

extern u8 AoE_SpecificUsability(const struct MenuItemDef *def, int number);
extern u8 AoE_SpecificEffect(struct MenuProc * menu, struct MenuItemProc * menuItem);
extern int AoE_SpecificHover(struct MenuProc * menu, struct MenuItemProc * menuItem);

extern u8 GambitSelectMenu_Usability(const struct MenuItemDef *def, int number);
extern u8 GambitSelectMenu_Effect(struct MenuProc * menu, struct MenuItemProc * menuItem);
extern int GambitSelectMenu_Hover(struct MenuProc * menu, struct MenuItemProc * menuItem);
extern int GambitSelectMenu_Unhover(struct MenuProc * menu, struct MenuItemProc * menuItem);

u8 pr_CombatArtActionCommandUsability(const struct MenuItemDef *def, int number);
int pr_CombatArtActionCommandOnDarw(struct MenuProc *menu, struct MenuItemProc *item);
u8 pr_CombatArtActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_CombatArtActionCommandHover(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_CombatArtActionCommandUnhover(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 pr_GaidenBMagActionCommandUsability(const struct MenuItemDef *def, int number);
int pr_GaidenBMagActionCommandOnDarw(struct MenuProc *menu, struct MenuItemProc *item);
u8 pr_GaidenBMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_GaidenBMagActionCommandHover(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_GaidenBMagActionCommandUnhover(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 pr_GaidenWMagActionCommandUsability(const struct MenuItemDef *def, int number);
int pr_GaidenWMagActionCommandOnDarw(struct MenuProc *menu, struct MenuItemProc *item);
u8 pr_GaidenWMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_GaidenWMagActionCommandHover(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 pr_UpperMenuSkill_Usability(const struct MenuItemDef *self, int number);
u8 pr_UpperMenuSkill_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
u8 pr_AttackCommandUsabilityFix(const struct MenuItemDef *def, int number);
u8 pr_AttackBallistaCommandUsabilityFix(const struct MenuItemDef *def, int number);
int pr_AttackActionCommandHoverFix(struct MenuProc *menu, struct MenuItemProc *menuItem);
int pr_AttackActionCommandUnhoverFix(struct MenuProc *menu, struct MenuItemProc *menuItem);

LYN_REPLACE_CHECK(WeaponSelectMenu_IsAvailable);
u8 WeaponSelectMenu_IsAvailable(const struct MenuItemDef* def, int number) {
    int item = gActiveUnit->items[number];

    if (!(GetItemAttributes(item) & IA_WEAPON)) {
        return MENU_NOTSHOWN;
    }

    if (!CanUnitUseWeapon(gActiveUnit, item)) {
        return MENU_NOTSHOWN;
    }

    MakeTargetListForWeapon(gActiveUnit, item);

    if (GetSelectTargetCount() == 0) {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

LYN_REPLACE_CHECK(WeaponSelectMenu_Selected);
u8 WeaponSelectMenu_Selected(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    EquipUnitItemSlot(gActiveUnit, menuItem->itemNumber);
    gActionData.itemSlotIndex = 0;

    ClearBg0Bg1();

    MakeTargetListForWeapon(gActiveUnit, gActiveUnit->items[0]);

    NewTargetSelection(&gSelectInfo_Attack);

    sub_80832C8();

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_ENDFACE;
}

LYN_REPLACE_CHECK(WeaponSelectMenu_Draw);
int WeaponSelectMenu_Draw(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    int item = gActiveUnit->items[menuItem->itemNumber];

    s8 isUsable = CanUnitUseWeapon(gActiveUnit, item);

    DrawItemMenuLine(
        &menuItem->text,
        item,
        isUsable,
        gBG0TilemapBuffer + TILEMAP_INDEX(menuItem->xTile, menuItem->yTile)
    );

    return 0;
}

LYN_REPLACE_CHECK(WeaponSelectMenu_SwitchIn);
int WeaponSelectMenu_SwitchIn(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    int reach;

    UpdateMenuItemPanel(menuItem->itemNumber);

    BmMapFill(gBmMapMovement, -1);
    BmMapFill(gBmMapRange, 0);

    reach = GetUnitWeaponReachBits(gActiveUnit, menuItem->itemNumber);
    GenerateUnitStandingReachRange(gActiveUnit, reach);

    DisplayMoveRangeGraphics(2);

    return 0;
}

const struct MenuItemDef gWeaponSelectMenuItems[] = {
    {"", 0, 0, 0, 0x49, WeaponSelectMenu_IsAvailable, WeaponSelectMenu_Draw, WeaponSelectMenu_Selected, 0, WeaponSelectMenu_SwitchIn, BallistaRangeMenu_SwitchOut},
    {"", 0, 0, 0, 0x4A, WeaponSelectMenu_IsAvailable, WeaponSelectMenu_Draw, WeaponSelectMenu_Selected, 0, WeaponSelectMenu_SwitchIn, BallistaRangeMenu_SwitchOut},
    {"", 0, 0, 0, 0x4B, WeaponSelectMenu_IsAvailable, WeaponSelectMenu_Draw, WeaponSelectMenu_Selected, 0, WeaponSelectMenu_SwitchIn, BallistaRangeMenu_SwitchOut},
    {"", 0, 0, 0, 0x4C, WeaponSelectMenu_IsAvailable, WeaponSelectMenu_Draw, WeaponSelectMenu_Selected, 0, WeaponSelectMenu_SwitchIn, BallistaRangeMenu_SwitchOut},
    {"", 0, 0, 0, 0x4D, WeaponSelectMenu_IsAvailable, WeaponSelectMenu_Draw, WeaponSelectMenu_Selected, 0, WeaponSelectMenu_SwitchIn, BallistaRangeMenu_SwitchOut},
    MenuItemsEnd
};

const struct MenuDef gWeaponSelectMenuDef = {
    {1, 1, 14, 0},
    0,
    gWeaponSelectMenuItems,
    0, 0, 0,
    ItemMenu_ButtonBPressed,
    MenuAutoHelpBoxSelect,
    ConvoyMenu_HelpBox
};

const struct MenuItemDef gUnitActionMenuItemsRework[] = {
	{"　制圧", 0x67A, 0x6CC, 0, 0x4E, UnitActionMenu_CanSeize, 0, UnitActionMenu_Seize, 0, 0, 0}, // Seize
	{"　攻撃", 0x67B, 0x6C0, 0, 0x4F, pr_AttackCommandUsabilityFix, 0, UnitActionMenu_Attack, 0, pr_AttackActionCommandHoverFix, pr_AttackActionCommandUnhoverFix}, // Attack >
	{"　攻撃", 0x67B, 0x6C0, 0, 0x50, pr_AttackBallistaCommandUsabilityFix, 0, UnitActionMenu_Attack, 0, pr_AttackActionCommandHoverFix, pr_AttackActionCommandUnhoverFix}, // Attack w/Ballista >

	/* Combat Art */
	{
		"　戰技",
		MSG_COMBATART_UM_NAME,
		MSG_COMBATART_UM_DESC,
		TEXT_COLOR_SYSTEM_WHITE,
		0x6C,
		pr_CombatArtActionCommandUsability,
		pr_CombatArtActionCommandOnDarw,
		pr_CombatArtActionCommandEffect,
		NULL,
		pr_CombatArtActionCommandHover,
		pr_CombatArtActionCommandUnhover
	},

	{"　杖", 0x67C, 0x6C1, 0, 0x51, StaffCommandUsability, 0, StaffCommandEffect, 0, StaffCommandRange, HideMoveRangeGraphicsWrapper2}, // Staff
	{"　乗る", 0x691, 0x6D6, 4, 0x52, RideCommandUsability, 0, RideCommandEffect, 0, 0, 0}, // Ride (Ballista) >
	{"　降りる", 0x692, 0x6D7, 4, 0x53, ExitCommandUsability, 0, ExitCommandEffect, 0, 0, 0}, // Exit (Ballista) >

#if CHAX
	{
		"　黑魔法",
		MSG_GaidenBMagic_UM_NAME,
		MSG_GaidenBMagic_UM_DESC,
		TEXT_COLOR_SYSTEM_WHITE,
		0x80,
		pr_GaidenBMagActionCommandUsability,
		pr_GaidenBMagActionCommandOnDarw,
		pr_GaidenBMagActionCommandEffect,
		0,
		pr_GaidenBMagActionCommandHover,
		pr_GaidenBMagActionCommandUnhover
	},
	{
		"　白魔法",
		MSG_GaidenWMagic_UM_NAME,
		MSG_GaidenWMagic_UM_DESC,
		TEXT_COLOR_SYSTEM_WHITE,
		0x80,
		pr_GaidenWMagActionCommandUsability,
		pr_GaidenWMagActionCommandOnDarw,
		pr_GaidenWMagActionCommandEffect,
		0,
		pr_GaidenWMagActionCommandHover,
		HideMoveRangeGraphicsWrapper2
	},
#ifdef CONFIG_MENU_SKILL_NOT_IN_UPPER
	{
		"　特技",
		MSG_MenuSkill_UM_NAME,
		MSG_MenuSkill_UM_DESC,
		TEXT_COLOR_SYSTEM_WHITE,
		0x80,
		pr_UpperMenuSkill_Usability,
		NULL,
		pr_UpperMenuSkill_OnSelected,
		NULL, NULL, NULL
	},
#else
	{
		"　特技",
		0x4EC,
		0,
		TEXT_COLOR_SYSTEM_WHITE,
		0x54,
		MenuSkills_Usability,
		MenuSkills_OnDraw,
		MenuSkills_OnSelected,
		MenuSkills_Idle,
		MenuSkills_Hover,
		MenuSkills_Unhover
	},
	{
		"　特技",
		0x4EC,
		1,
		TEXT_COLOR_SYSTEM_WHITE,
		0x55,
		MenuSkills_Usability,
		MenuSkills_OnDraw,
		MenuSkills_OnSelected,
		MenuSkills_Idle,
		MenuSkills_Hover,
		MenuSkills_Unhover
	},
	{
		"　特技",
		0x4EC,
		2,
		TEXT_COLOR_SYSTEM_WHITE,
		0x56,
		MenuSkills_Usability,
		MenuSkills_OnDraw,
		MenuSkills_OnSelected,
		MenuSkills_Idle,
		MenuSkills_Hover,
		MenuSkills_Unhover
	},
	{
		"　特技",
		0x4EC,
		3,
		TEXT_COLOR_SYSTEM_WHITE,
		0x57,
		MenuSkills_Usability,
		MenuSkills_OnDraw,
		MenuSkills_OnSelected,
		MenuSkills_Idle,
		MenuSkills_Hover,
		MenuSkills_Unhover
	},
#endif /* MENU_SKILL_NOT_IN_UPPER */
#else
	{"　奏でる", 0x67D, 0x6C3, 0, 0x54, PlayCommandUsability, 0, PlayCommandEffect, 0, 0, 0}, // Play >
	{"　踊る", 0x67E, 0x6C2, 0, 0x55, DanceCommandUsability, 0, PlayCommandEffect, 0, 0, 0}, // Dance
	{"　盗む", 0x67F, 0x6C4, 0, 0x56, StealCommandUsability, 0, StealCommandEffect, 0, 0, 0}, // Steal
	{"　召喚", 0x693, 0x6DD, 0, 0x57, SummonCommandUsability, 0, SummonCommandEffect, 0, 0, 0}, // Summon (Shoukanshi) >
	{"　呼魔", 0x693, 0x6DD, 0, 0x58, YobimaCommandUsability, 0, YobimaCommandEffect, 0, 0, 0}, // Summon (Deamon King) >
	{"　かぎ開", 0x694, 0x6DE, 0, 0x59, PickCommandUsability, 0, PickCommandEffect, 0, 0, 0}, // Pick >
#endif

	{"　話す", 0x680, 0x6C9, 0, 0x5A, TalkCommandUsability, 0, TalkCommandEffect, 0, 0, 0}, // Talk >
	{"　支援", 0x681, 0x6CA, 0, 0x5B, SupportCommandUsability, 0, SupportCommandEffect, 0, 0, 0}, // Support
	{"　訪問", 0x682, 0x6CB, 0, 0x5C, VisitCommandUsability, 0, VisitCommandEffect, 0, 0, 0}, // Visit
	{"　宝箱", 0x683, 0x6CE, 0, 0x5D, ChestCommandUsability, 0, ChestCommandEffect, 0, 0, 0}, // Chest
	{"　扉", 0x684, 0x6CD, 0, 0x5E, DoorCommandUsability, 0, DoorCommandEffect, 0, 0, 0}, // Door >
	{"　武器屋", 0x685, 0x6CF, 0, 0x5F, ArmoryCommandUsability, 0, ArmoryCommandEffect, 0, 0, 0}, //Armory >
	{"　道具屋", 0x686, 0x6D0, 0, 0x60, VendorCommandUsability, 0, VendorCommandEffect, 0, 0, 0}, //Vendor >
	{"　秘密店", 0x687, 0x6D1, 0, 0x61, SecretShopCommandUsability, 0, SecretShopCommandEffect, 0, 0, 0}, //SecretShop >
	{"　闘技場", 0x688, 0x6D2, 0, 0x62, ArenaCommandUsability, 0, ArenaCommandEffect, 0, 0, 0}, // Arena
	{"　救出", 0x689, 0x6C5, 0, 0x63, RescueUsability, 0, RescueEffect, 0, 0, 0}, // Rescue >
#ifdef CONFIG_MOKHA_AOE
	{"", GambitAssemblyName, GambitSelectMenu_Desc, TEXT_COLOR_SYSTEM_GREEN, 0, GambitSelectMenu_Usability, 0, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
#endif
    {"　救出", MSG_MenuCommand_Refuge_NAME, MSG_MenuCommand_Refuge_DESC, TEXT_COLOR_SYSTEM_WHITE, 0x6C, Refuge_Usability, 0, Refuge_OnSelected, 0, 0, 0}, // Refuge > 
#ifdef CONFIG_LAGUZ_BARS
    {"　救出", MSG_MenuCommand_Transform_Laguz_NAME, MSG_MenuCommand_Transform_Laguz_DESC, TEXT_COLOR_SYSTEM_GOLD, 0x6C, Transform_Laguz_Usability, 0, Transform_Laguz_Effect, 0, 0, 0}, // Laguz Transform > 
#endif
	/**
	 * The 5th parameter in this array determins the type of AOE skill
	 * 0 - SLASH - Usable to anyone who has a SWORD they can wield
	 * 1 - CLEAVE - Usable to anyone who has a LANCE they can wield
	 * 2 - IMPALE - Usable by anyone who has an AXE they can wield
	 * 3 - SHOOT - Usable by anyone who has a BOW they can wield
	 * 4 - BLAST - Usable by anyone who has an ANIMA magic tome they can wield
	 * 5 - EXPEL - Usable by anyone who has a LIGHT magic tome they can wield
	 * 6 - HEX - Usable by anyone who has a DARK magic tome they can wield
	 * 7 - HEAL - Usable by anyone who can use the aforementioned stave
	 * 8 - RESTORE - Usable by anyone who can use the aforementioned stave
	 * 9 - MEND - Usable by anyone who can use the aforementioned stave
	 * 10 - PHYSIC - Usable by anyone who can use the aforementioned stave
	 * 11 - RECOVER - Usable by anyone who can use the aforementioned stave
	 * 12 - FORTIIFY - Usable by anyone who can use the aforementioned stave
	 * 13 - EXCALIBUR - Usable by anyone who can use the aforementioned weapon
	 * 14 - ELFIRE - Usable by anyone who can use the aforementioned weapon
	 * 15 - THUNDER - Usable by anyone who can use the aforementioned weapon
	 * 16 - FIMBULVETR - Usable by anyone who can use the aforementioned weapon
	 */
#ifdef CONFIG_VESLY_AOE
	{"", UM_AoESlash, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 0, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoECleave, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 1, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEImpale, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 2, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEShoot, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 3, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEBlast, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 4, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEExpel, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 5, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEHex, UM_AoEDesc, TEXT_COLOR_SYSTEM_WHITE, 6, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEHeal, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 7, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoERestore, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 8, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEMend, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 9, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEPhysic, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 10, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoERecover, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 11, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
	{"", UM_AoEFortify, UM_AoEHealHelp, TEXT_COLOR_SYSTEM_WHITE, 12, AoE_SpecificUsability, 0, AoE_SpecificEffect, 0, AoE_SpecificHover, 0},
#endif
	{"　降ろす", 0x68A, 0x6C6, 0, 0x64, DropUsability, 0, DropEffect, 0, 0, 0}, // Drop >
	{"　引受け", 0x68B, 0x6C8, 4, 0x65, TakeUsability, 0, TakeEffect, 0, 0, 0}, // Take >
	{"　引渡し", 0x68C, 0x6C7, 4, 0x66, GiveUsability, 0, GiveEffect, 0, 0, 0}, // Give >
	{"　持ち物", 0x68D, 0x6D3, 0, 0x67, ItemCommandUsability, 0, ItemCommandEffect, 0, 0, 0}, // Item >
	{"　交換", 0x68E, 0x6D4, 4, 0x68, ItemSubMenu_IsTradeAvailable, 0, TradeCommandEffect, 0, 0, 0}, // Trade >
	{"　輸送隊", 0x68F, 0x6D5, 4, 0x69, SupplyUsability, 0, SupplyCommandEffect, 0, 0, 0}, // Supply >
	{"　待機", 0x695, 0x6BF, 0, 0x6B, MenuAlwaysEnabled, 0, EffectWait, 0, 0, 0}, // Wait >
	MenuItemsEnd
};