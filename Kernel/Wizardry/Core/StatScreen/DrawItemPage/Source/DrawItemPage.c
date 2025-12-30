#include <common-chax.h>
#include <list-verify.h>
#include <battle-system.h>
#include <gaiden-magic.h>
#include <stat-screen.h>
#include <shield.h>
#include "skill-system.h"
#include "constants/skills.h"

#define LOCAL_TRACE 0

void ResetItemPageLists(void)
{
	memset(&gItemPageList, 0, sizeof(gItemPageList));
}

STATIC_DECLAR void UpdateItemPageListExt(struct Unit *unit, struct ItemPageList *list)
{
	int i, item, cnt = 0;
	FORCE_DECLARE struct GaidenMagicList *gmag_list;

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

#ifndef CONFIG_MP_SYSTEM
	/**
	 * Gaiden Magic
	 */
	gmag_list = GetGaidenMagicList(unit);

	for (i = 0; i < gmag_list->bmag_cnt; i++) {
		struct ItemPageEnt *ent;

		item = gmag_list->bmags[i];
		if (item == ITEM_NONE)
			break;

		ent = &list->ent[cnt++];
		if (cnt > CHAX_ITEM_PAGE_AMT)
			return;

		ent->item = item;
		ent->slot = CHAX_BUISLOT_GAIDEN_BMAG1 + i;
		ent->color = CanUnitUseGaidenMagic(unit, item)
				   ? TEXT_COLOR_SYSTEM_GOLD
				   : TEXT_COLOR_SYSTEM_GRAY;
	}

	for (i = 0; i < gmag_list->wmag_cnt; i++) {
		struct ItemPageEnt *ent;

		item = gmag_list->wmags[i];
		if (item == ITEM_NONE)
			break;

		ent = &list->ent[cnt++];
		if (cnt > CHAX_ITEM_PAGE_AMT)
			return;

		ent->item = item;
		ent->slot = CHAX_BUISLOT_GAIDEN_WMAG1 + i;
		ent->color = CanUnitUseGaidenMagic(unit, item)
				   ? TEXT_COLOR_SYSTEM_GOLD
				   : TEXT_COLOR_SYSTEM_GRAY;
	}
#endif
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

#ifndef CONFIG_MP_SYSTEM
	PutGaidenMagicCostNumber(tm + 14, color, GetGaidenWeaponHpCost(gStatScreen.unit, item));
#endif

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

#ifndef CONFIG_MP_SYSTEM
		case CHAX_BUISLOT_GAIDEN_BMAG1 ... CHAX_BUISLOT_GAIDEN_BMAG7:
		case CHAX_BUISLOT_GAIDEN_WMAG1 ... CHAX_BUISLOT_GAIDEN_WMAG7:
			DrawItemLineGaidenMagic(ent, i);
			break;
#endif

		default:
			break;
		}
	}

	slot = GetUnitEquippedWeaponSlot(unit);
	DrawItemEquipLine(slot);
	DrawItemPageSubfix(slot);

	// CONFIG_INSTALL_KERNEL_SHIELD
	if (gpKernelDesigerConfig->shield_en)
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

/**
 * Helpbox
 */
LYN_REPLACE_CHECK(HbRedirect_SSItem);
void HbRedirect_SSItem(struct HelpBoxProc *proc)
{

	struct ItemPageList *list = GetUnitItemPageList(gStatScreen.unit);

	/* JESTER - A little something to turn off the R text for this page if there are no promotions available for the unit */
#ifdef CONFIG_STAT_PAGE_PROMOTIONS
	if (gStatScreen.page == 6)
		if (gEventSlots[EVT_SLOT_8] == 0x1000)
			TryRelocateHbLeft(proc);
#endif

	if (list->ent[0].item == ITEM_NONE)
		TryRelocateHbLeft(proc);

	if (list->ent[proc->info->mid].item == ITEM_NONE) {
		if (proc->moveKey == 0 || proc->moveKey == DPAD_RIGHT || proc->moveKey == DPAD_UP)
			TryRelocateHbUp(proc);
		else if (proc->moveKey == DPAD_DOWN)
			TryRelocateHbDown(proc);
	}

	/* JESTER - A little something to turn off the RText box when moving up and down to empty positions in the Gaiden Magic list */
#ifdef CONFIG_USE_GAIDEN_MAGIC
	if (gpKernelDesigerConfig->gaiden_magic_en && gStatScreen.page == 4)
	{
		struct GaidenMagicList *list_gaiden = GetGaidenMagicList(gStatScreen.unit);

		if ((proc->moveKey == DPAD_DOWN || proc->moveKey == DPAD_UP) && IsGaidenMagicItemMissingAtCursor(proc, list_gaiden))
			gKeyStatusPtr->newKeys = B_BUTTON;
	}
#endif
}

LYN_REPLACE_CHECK(HbPopulate_SSItem);
void HbPopulate_SSItem(struct HelpBoxProc *proc)
{
	struct ItemPageList *list = GetUnitItemPageList(gStatScreen.unit);
	int item = list->ent[proc->info->mid].item;

	proc->item = item;
	proc->mid  = GetItemDescId(item);

#ifdef CONFIG_USE_GAIDEN_MAGIC
	if (gpKernelDesigerConfig->gaiden_magic_en)
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
#endif

}