#include <common-chax.h>
#include <list-verify.h>
#include <battle-system.h>
#include <gaiden-magic.h>
#include <stat-screen.h>
#include <shield.h>
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/custom-structs.h"

#define LOCAL_TRACE 0

void ResetItemPageLists(void)
{
	memset(&gItemPageList, 0, sizeof(gItemPageList));
}

STATIC_DECLAR void UpdateItemPageListExt(struct Unit *unit, struct ItemPageList *list)
{
	int i, item, cnt = 0;

	/**
	 * Unit items
	 */
	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		struct ItemPageEnt *ent;

		item = unit->items[i];
		if (item == ITEM_NONE)
			break;

		ent = &list->ent[cnt++];
		if (cnt > CHAX_ITEM_PAGE_AMT)
			return;

		ent->item = item;
		ent->slot = i;

		if ((unit->state & US_DROP_ITEM) && ((GetUnitItemCount(unit) - 1) == i))
			ent->color = TEXT_COLOR_SYSTEM_GREEN;
		else
			ent->color = IsItemDisplayUsable(unit, item)
					   ? TEXT_COLOR_SYSTEM_WHITE
					   : TEXT_COLOR_SYSTEM_GRAY;
	}

}

static void dump_item_list(struct ItemPageList *list)
{
#ifdef CONFIG_USE_DEBUG
	int i;

	for (i = 0; i < CHAX_ITEM_PAGE_AMT; i++) {
		if (list->ent[i].item == ITEM_NONE)
			break;

		LTRACEF("[uid=0x%02X, pid=0x%02X] item=0x%04X, slot=%d",
			list->header.uid & 0xFF, ((const struct CharacterData *)list->header.pinfo)->number, list->ent[i].item, list->ent[i].slot);
	}
#endif
}

void UpdateItemPageList(struct Unit *unit, struct ItemPageList *list)
{
	ResetItemPageLists();
	UpdateItemPageListExt(unit, list);
	WriteUnitList(unit, &list->header);

	dump_item_list(list);
}

struct ItemPageList *GetUnitItemPageList(struct Unit *unit)
{
	struct ItemPageList *list = &gItemPageList;

	if (!JudgeUnitList(unit, &list->header))
		UpdateItemPageList(unit, list);

	return list;
}

NOINLINE STATIC_DECLAR void DrawItemLineDefault(const struct ItemPageEnt *ent, int line)
{
	DrawItemStatScreenLine(
		&gStatScreen.text[STATSCREEN_TEXT_ITEM0 + line],
		ent->item,
		ent->color,
		gUiTmScratchA + TILEMAP_INDEX(1, 1 + line * 2)
	);
}

NOINLINE STATIC_DECLAR void DrawItemLineGaidenMagic(const struct ItemPageEnt *ent, int line)
{
	int item = ent->item;
	int color = ent->color;
	struct Text *text = &gStatScreen.text[STATSCREEN_TEXT_ITEM0 + line];
	u16 *tm = gUiTmScratchA + TILEMAP_INDEX(1, 1 + line * 2);

	ClearText(text);
	Text_SetColor(text, color);
	Text_DrawString(text, GetItemName(ent->item));

	color = (ent->color == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_BLUE;

	if (gpKernelDesignerConfig->mp_system != true)
		PutGaidenMagicCostNumber(tm + 14, color, GetGaidenWeaponHpCost(gStatScreen.unit, item));

	PutText(text, tm + 2);
	DrawIcon(tm, GetItemIconId(item), 0x4000);

#if 0
	CallARM_FillTileRect(
		gUiTmScratchC + TILEMAP_INDEX(1, 2 + line * 2),
		gpTSA_ItemEquipLine, TILEREF(0x40, STATSCREEN_BGPAL_3));
#endif
}

NOINLINE STATIC_DECLAR void DrawItemEquipLine(int slot)
{
	int i, line;
	struct Unit *unit = gStatScreen.unit;
	struct ItemPageList *list = GetUnitItemPageList(unit);

	line = -1;

	for (i = 0; i < CHAX_ITEM_PAGE_AMT; i++) {
		if (list->ent[i].slot == slot) {
			line = i;
			break;
		}
	}

	if (line >= 0) {
		PutSpecialChar(
			gUiTmScratchA + TILEMAP_INDEX(16, 1 + line * 2),
			TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_35);

		CallARM_FillTileRect(
			gUiTmScratchC + TILEMAP_INDEX(1, 2 + line * 2),
			gpTSA_ItemEquipLine, TILEREF(0x40, STATSCREEN_BGPAL_3));
	}
}

NOINLINE STATIC_DECLAR void DrawItemPageSubfix(int slot)
{
	int i, weapon;
	const char *str;
	struct Unit *unit = gStatScreen.unit;

	Decompress(gpTSA_ItemPageSubfix, gGenericBuffer);
	CallARM_FillTileRect(
		gUiTmScratchC + TILEMAP_INDEX(1, 11),
		gGenericBuffer, TILEREF(0x40, STATSCREEN_BGPAL_3));

	DisplayTexts(gpPage1TextInfo);

	weapon = GetItemFromSlot(unit, slot);

#if (defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat)))
	if (SkillTester(gStatScreen.unit, SID_UnarmedCombat))
	{    
		if(!GetUnitEquippedWeapon(gStatScreen.unit))
		{
			gBattleActor.battleAttack = (gStatScreen.unit->pow);
			gBattleActor.battleHitRate = (gStatScreen.unit->skl * 2) + (gStatScreen.unit->lck / 2) + SKILL_EFF0(SID_UnarmedCombat);
			gBattleActor.battleCritRate = gStatScreen.unit->lck;
		}

	}
#endif

	PutNumberOrBlank(
		gUiTmScratchA + TILEMAP_INDEX(8,  13),
		TEXT_COLOR_SYSTEM_BLUE, gBattleActor.battleAttack);

	PutNumberOrBlank(
		gUiTmScratchA + TILEMAP_INDEX(8,  15),
		TEXT_COLOR_SYSTEM_BLUE, gBattleActor.battleHitRate);

	PutNumberOrBlank(
		gUiTmScratchA + TILEMAP_INDEX(15, 13),
		TEXT_COLOR_SYSTEM_BLUE, gBattleActor.battleCritRate);

	PutNumberOrBlank(
		gUiTmScratchA + TILEMAP_INDEX(15, 15),
		TEXT_COLOR_SYSTEM_BLUE, gBattleActor.battleAvoidRate);

	str = GetItemDisplayRangeString(weapon);
	Text_InsertDrawString(
		&gStatScreen.text[STATSCREEN_TEXT_BSRANGE],
		55 - GetStringTextLen(str),
		TEXT_COLOR_SYSTEM_BLUE, str);

	for (i = 0; i < 8; ++i) {
		gUiTmScratchA[TILEMAP_INDEX(1 + i, 11)] = TILEREF(0x60 + i, STATSCREEN_BGPAL_7);
		gUiTmScratchA[TILEMAP_INDEX(1 + i, 12)] = TILEREF(0x68 + i, STATSCREEN_BGPAL_7);
	}
}

LYN_REPLACE_CHECK(DisplayPage1);
void DisplayPage1(void)
{
	int i, slot;
	struct Unit *unit = gStatScreen.unit;
	struct ItemPageList *list;

	ResetItemPageLists();
	list = GetUnitItemPageList(unit);

	for (i = 0; i < CHAX_ITEM_PAGE_AMT; i++) {
		struct ItemPageEnt *ent = &list->ent[i];

		if (ent->item == ITEM_NONE)
			break;

		switch (ent->slot) {
		case 0 ... 4:
			DrawItemLineDefault(ent, i);
			break;

		/* This draws the gaiden magics in any remaining item slots, not a good idea honestly. We have a seperate stat screen page now */
		// if (gpKernelDesignerConfig->mp_system != true)
		// {
		// 	case CHAX_BUISLOT_GAIDEN_BMAG1 ... CHAX_BUISLOT_GAIDEN_BMAG7:
		// 	case CHAX_BUISLOT_GAIDEN_WMAG1 ... CHAX_BUISLOT_GAIDEN_WMAG7:
		// 		DrawItemLineGaidenMagic(ent, i);
		// 		break;
		// }

		default:
			break;
		}
	}

	slot = GetUnitEquippedWeaponSlot(unit);
	DrawItemEquipLine(slot);
	DrawItemPageSubfix(slot);

	if (gpKernelDesignerConfig->shield_system == true)
		DrawItemPage_ShieldEquipLine();
}

// Column X positions
#define COL_BMAG   166
#define COL_WMAG   102

// Row layout
#define ROW_START   40
#define ROW_STEP    16
#define ROW_COUNT    5

// Returns row index 0–4, or -1 if not on a valid row
static inline int GetGaidenMagicStatusRowIndex(int y) {
    int delta = y - ROW_START;
    if (delta < 0) return -1;

    if (delta % ROW_STEP != 0) return -1; // not aligned
    int i = delta / ROW_STEP;

    return (i < ROW_COUNT) ? i : -1;
}

static inline bool IsGaidenMagicItemMissingAtCursor(struct HelpBoxProc *proc, struct GaidenMagicList *list)
{
    int row = GetGaidenMagicStatusRowIndex(proc->info->yDisplay);
    if (row < 0)
        return false;  // Not on a valid row

    int x = proc->info->xDisplay;

    if (x == COL_BMAG) {
        return (list->bmags[row] == ITEM_NONE);
    }
    else if (x == COL_WMAG) {
        return (list->wmags[row] == ITEM_NONE);
    }

    return false; // Not pointing at a valid column
}

/*
 * Determine whether the current help-box cursor position on the promotions
 * page corresponds to a slot that has no class or no skill icon.
 *
 * Coordinate layout (pixel values stored in u8 xDisplay / yDisplay):
 *   Promo name slots:  x=PROMO_ICON_X,         y=0x20/0x48/0x70  (rows 0/1/2)
 *   Skill icon slots:  x=SKILL_ICON_BASE_X + col*SKILL_ICON_STRIDE (cols 0/1/2)
 *                      y=0x28/0x50/0x78 (rows 0/1/2)
 */

/* x-coordinate (pixels) of the promotion-class name column */
#define PROMO_ICON_X         0x6C
/* x-coordinate of the first skill icon column */
#define SKILL_ICON_BASE_X    0x90
/* pixel spacing between adjacent skill icon columns */
#define SKILL_ICON_STRIDE    0x10
/* y-coordinate boundary separating row 0 from row 1 */
#define PROMO_ROW1_Y_MIN     0x40
/* y-coordinate boundary separating row 1 from row 2 */
#define PROMO_ROW2_Y_MIN     0x68

static bool IsPromoPageCursorOnEmptySlot(struct HelpBoxProc *proc)
{
	struct Unit *unit = gStatScreen.unit;
	int charId = UNIT_CHAR_ID(unit);
	const UnitPromotions *promo_data = NULL;
	int x = proc->info->xDisplay;
	int y = proc->info->yDisplay;
	int row;

	for (int i = 0; unit_promotions[i].key != 0; i++) {
		if (unit_promotions[i].key == charId) {
			promo_data = &unit_promotions[i];
			break;
		}
	}

	if (promo_data == NULL)
		return false;

	/* Map y-coordinate to promo row 0/1/2 */
	if (y < PROMO_ROW1_Y_MIN)
		row = 0;
	else if (y < PROMO_ROW2_Y_MIN)
		row = 1;
	else
		row = 2;

	/* Promotion name slot */
	if (x == PROMO_ICON_X)
		return promo_data->promotions[row].classId == 0;

	/* Skill icon slot */
	int col = (x - SKILL_ICON_BASE_X) / SKILL_ICON_STRIDE;
	if (col < 0 || col > 2)
		return false;

	return promo_data->promotions[row].skills[col] == 0;
}

/**
 * Helpbox
 */
LYN_REPLACE_CHECK(HbRedirect_SSItem);
void HbRedirect_SSItem(struct HelpBoxProc *proc)
{
	bool isGaidenMagicPage = gpKernelDesignerConfig->gaiden_magic == true
						 && gStatScreen.page == TranslateStatPageId(PAGE_GAIDEN_MAGIC);

	if (!isGaidenMagicPage)
	{
		struct ItemPageList *list = GetUnitItemPageList(gStatScreen.unit);

		if (list->ent[0].item == ITEM_NONE)
			TryRelocateHbLeft(proc);

		if (list->ent[proc->info->mid].item == ITEM_NONE) {
			if (proc->moveKey == 0 || proc->moveKey == DPAD_RIGHT || proc->moveKey == DPAD_UP)
				TryRelocateHbUp(proc);
			else if (proc->moveKey == DPAD_DOWN)
				TryRelocateHbDown(proc);
		}
	}

	/* JESTER - A little something to turn off the R text for this page if there are no promotions available for the unit */
	if (gStatScreen.page == TranslateStatPageId(6))
		if (gEventSlots[EVT_SLOT_8] == 0x1000)
			TryRelocateHbLeft(proc);

	/* Prevent cursor from landing on empty promotion name or skill icon slots */
	if (gStatScreen.page == TranslateStatPageId(PAGE_PROMOTIONS) && gEventSlots[EVT_SLOT_8] != 0x1000)
	{
		if (IsPromoPageCursorOnEmptySlot(proc))
		{
			switch (proc->moveKey)
			{
			case DPAD_DOWN:
				TryRelocateHbDown(proc);
				break;
			case DPAD_LEFT:
				TryRelocateHbLeft(proc);
				break;
			case DPAD_RIGHT:
				TryRelocateHbRight(proc);
				break;
			default: /* DPAD_UP or initial (0) */
				TryRelocateHbUp(proc);
				break;
			}
		}
	}

	/* JESTER - A little something to turn off the RText box when moving up and down to empty positions in the Gaiden Magic list */
	if (isGaidenMagicPage)
	{
		struct GaidenMagicList *list_gaiden = GetGaidenMagicList(gStatScreen.unit);

		if ((proc->moveKey == DPAD_DOWN || proc->moveKey == DPAD_UP) && IsGaidenMagicItemMissingAtCursor(proc, list_gaiden))
			gKeyStatusPtr->newKeys = B_BUTTON;
	}
}

LYN_REPLACE_CHECK(HbPopulate_SSItem);
void HbPopulate_SSItem(struct HelpBoxProc *proc)
{
	bool isGaidenMagicPage = gpKernelDesignerConfig->gaiden_magic == true
						 && gStatScreen.page == TranslateStatPageId(PAGE_GAIDEN_MAGIC);
	int item = ITEM_NONE;

	if (!isGaidenMagicPage)
	{
		struct ItemPageList *list = GetUnitItemPageList(gStatScreen.unit);
		item = list->ent[proc->info->mid].item;

		proc->item = item;
		proc->mid  = GetItemDescId(item);
	}

	if (isGaidenMagicPage)
	{
		struct GaidenMagicList *list_gaiden = GetGaidenMagicList(gStatScreen.unit);

		/* 
		** This only works alongside the normal item page because every 
		** gaiden magic entry displays at unique coordinates
		*/

		/* White magic */
		for (int i = 0; i < 5; ++i)
		{
			if (proc->info->xDisplay == 102 && proc->info->yDisplay == (40 + i * 16))
			{
				proc->item = list_gaiden->wmags[i];
				proc->mid = GetItemDescId(list_gaiden->wmags[i]);
				break; // Exit the loop once a match is found
			}
		}

		/* Black magic */
		for (int i = 0; i < 5; ++i)
		{
			if (proc->info->xDisplay == 166 && proc->info->yDisplay == (40 + i * 16))
			{
				proc->item = list_gaiden->bmags[i];
				proc->mid = GetItemDescId(list_gaiden->bmags[i]);
				break; // Exit the loop once a match is found
			}
		}
	}
}