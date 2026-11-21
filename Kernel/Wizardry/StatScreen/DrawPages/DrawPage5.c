#include "common-chax.h"
#include "hooks.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "constants/texts.h"
#include "bwl.h"
#include "gaiden-magic.h"
#include "utf8.h"
#include "jester_headers/custom-functions.h"

/*
** gStatScreen.text is 35 fields long
** 15 - Page 5 - White Magic Title
** 16 - Page 5 - White Magic 1
** 17 - Page 5 - White Magic 2
** 18 - Page 5 - White Magic 3
** 19 - Page 5 - White Magic 4
** 20 - Page 5 - White Magic 5
** 21 - Page 1 - ATK

** 28 - Page 5 - MP
** 29 - Page 5 - Black Magic Title
** 30 - Page 5 - Black Magic 1
** 31 - Page 5 - Black Magic 2
** 32 - Page 5 - Black Magic 3
** 33 - Page 5 - Black Magic 4
** 34 - Page 5 - Black Magic 5
*/

#define PAGE4_PINFO_MAX 8
#define WHITE_MAGIC_STARTING_TEXT_FIELD 15 // Needs 6 consecutive entries
#define BLACK_MAGIC_STARTING_TEXT_FIELD 29 // Needs 6 consecutive entries

/*
 * Rows per column: 7 (a unit holds up to 7 of each), or 5 when the MP line
 * takes the bottom rows. Rows 6 and 7 borrow the stats page's rescue,
 * affinity and status fields and the unused field 2 (all 7+ tiles wide).
 */
#define PAGE5_MAGIC_ROWS 7
#define PAGE5_MAGIC_ROWS_WITH_MP 5

static const u8 sPage5WhiteFields[PAGE5_MAGIC_ROWS] = {
    STATSCREEN_TEXT_ITEM1, STATSCREEN_TEXT_ITEM2, STATSCREEN_TEXT_ITEM3, STATSCREEN_TEXT_ITEM4,
    STATSCREEN_TEXT_BSRANGE, STATSCREEN_TEXT_RESCUENAME, STATSCREEN_TEXT_STATUS,
};

static const u8 sPage5BlackFields[PAGE5_MAGIC_ROWS] = {
    STATSCREEN_TEXT_SUPPORT1, STATSCREEN_TEXT_SUPPORT2, STATSCREEN_TEXT_SUPPORT3, STATSCREEN_TEXT_SUPPORT4,
    STATSCREEN_TEXT_BWL, STATSCREEN_TEXT_AFFINLABEL, STATSCREEN_TEXT_UNUSUED,
};

static int GetPage5MagicRows(void)
{
    return IsGaidenMagicUsesSystem() ? PAGE5_MAGIC_ROWS : PAGE5_MAGIC_ROWS_WITH_MP;
}

/*
 * A spell's colour: gold if the unit can cast it, grey if it is not learned
 * yet or (Three Houses magic) its uses this chapter are spent.
 */
static int GetPage5MagicColor(struct Unit *unit, int item, bool known)
{
    if (!known)
        return TEXT_COLOR_SYSTEM_GRAY;

    if (IsGaidenMagicUsesSystem() && !HasGaidenMagicUses(unit, item))
        return TEXT_COLOR_SYSTEM_GRAY;

    return TEXT_COLOR_SYSTEM_GOLD;
}

static void PutGaidenCost(struct Unit * unit, int item, bool known, u16 *tm)
{
    int number;

    /* Three Houses-style magic: remaining uses this chapter */
    if (IsGaidenMagicUsesSystem())
    {
        /* A spell not learned yet has no uses this chapter: no number */
        if (!known)
            return;

        number = GetGaidenMagicUses(unit, item);

        if (number != GAIDEN_MAGIC_USES_INFINITE)
            PutNumberSmall(tm + 2, number ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, number);

        return;
    }

    if (!known)
        return;

    number = GetGaidenWeaponHpCost(unit, item);

    if (number == 0)
        return;

    PutNumberSmall(tm + 2, TEXT_COLOR_SYSTEM_GRAY, number);
}

static void PutPage5MagicColumn(struct Unit *unit, const u8 *items, int count, const u8 *fields, int x)
{
    for (int i = 0; i < count && i < GetPage5MagicRows(); i++) {
        int item = items[i];
        bool known = IsGaidenMagicKnown(unit, item);

        DrawIcon(gUiTmScratchA + TILEMAP_INDEX(x, 0x3 + (i * 2)), GetItemIconId(item), 0x4000);

        PutDrawText(
            &gStatScreen.text[fields[i]],
            gUiTmScratchA + TILEMAP_INDEX(x + 2, 0x3 + (i * 2)),
            GetPage5MagicColor(unit, item, known),
            0, 0,
            Utf8ToNarrowFonts(GetItemName(item)));

        PutGaidenCost(unit, item, known, gUiTmScratchA + TILEMAP_INDEX(x + 5, 0x3 + (i * 2)));
    }
}

static void WhiteMagicList(struct Unit * unit, struct GaidenMagicFullList *full)
{
    PutDrawText(
        &gStatScreen.text[WHITE_MAGIC_STARTING_TEXT_FIELD],
        gUiTmScratchA + TILEMAP_INDEX(0x1, 0x1),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0,
        "White Magic");

    if (full)
        PutPage5MagicColumn(unit, full->wmags, full->wmag_cnt, sPage5WhiteFields, 0x0);
};

static void BlackMagicList(struct Unit * unit, struct GaidenMagicFullList *full)
{
    PutDrawText(
        &gStatScreen.text[BLACK_MAGIC_STARTING_TEXT_FIELD],
        gUiTmScratchA + TILEMAP_INDEX(0x9, 0x1),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0,
        "Black Magic");

    if (full)
        PutPage5MagicColumn(unit, full->bmags, full->bmag_cnt, sPage5BlackFields, 0x8);
};

void DisplayMP(struct Unit * unit)
{
    struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(unit));
    
    PutDrawText(
        &gStatScreen.text[28],
        gUiTmScratchA + TILEMAP_INDEX(0x5, 0xE),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0,
        "MP:");

    int currentMP;
    int maxMP;

    if (UNIT_FACTION(unit) != FACTION_BLUE)
    {
        currentMP = 0;
        maxMP = 0;
    }
    else
    {
        currentMP = bwl->currentMP;
        maxMP = GetUnitMaxMP(gStatScreen.unit);
    }

    PutNumber((gUiTmScratchA + TILEMAP_INDEX(0x8, 0xE)), TEXT_COLOR_SYSTEM_BLUE, currentMP);
    PutSpecialChar((gUiTmScratchA + TILEMAP_INDEX(0x9, 0xE)), TEXT_COLOR_SYSTEM_WHITE, TEXT_SPECIAL_SLASH);
    PutNumber((gUiTmScratchA + TILEMAP_INDEX(0xB, 0xE)), TEXT_COLOR_SYSTEM_BLUE, maxMP);
};

static void DrawPage5MagicList(void)
{
    struct Unit * unit = gStatScreen.unit;
    struct GaidenMagicFullList full;
    struct GaidenMagicFullList *pfull = NULL;

    if (gpKernelDesignerConfig->gaiden_magic == true) {
        GetGaidenMagicFullList(unit, &full);
        pfull = &full;
    }

    WhiteMagicList(unit, pfull);
    BlackMagicList(unit, pfull);

    /* Three Houses-style spells use uses per chapter, not MP */
    if (!IsGaidenMagicUsesSystem())
        DisplayMP(unit);
};

void DrawPage5Rework(void)
{
    int i;
    for (i = 15; i < 21; i++)
        ClearText(&gStatScreen.text[i]);

    for (i = 29; i < 35; i++)
        ClearText(&gStatScreen.text[i]);

    for (i = PAGE5_MAGIC_ROWS_WITH_MP; i < PAGE5_MAGIC_ROWS; i++) {
        ClearText(&gStatScreen.text[sPage5WhiteFields[i]]);
        ClearText(&gStatScreen.text[sPage5BlackFields[i]]);
    }

    DrawPage5MagicList();
}

/*
 * R help on a spell row (HelpBoxPage5.event): the row's spell in the same
 * list the page draws. Rows are 0x10 apart from y 0x28; x 0x66 is the White
 * Magic column, 0xA6 the Black Magic one.
 */
static int GetPage5HelpItem(const struct HelpBoxInfo *info)
{
    struct GaidenMagicFullList full;
    int row = (info->yDisplay - 0x28) >> 4;

    if (gpKernelDesignerConfig->gaiden_magic != true || row < 0 || row >= GetPage5MagicRows())
        return ITEM_NONE;

    GetGaidenMagicFullList(gStatScreen.unit, &full);

    if (info->xDisplay < 0xA6)
        return row < full.wmag_cnt ? full.wmags[row] : ITEM_NONE;

    return row < full.bmag_cnt ? full.bmags[row] : ITEM_NONE;
}

void HbPopulate_Page5Magic(struct HelpBoxProc *proc)
{
    int item = GetPage5HelpItem(proc->info);

    proc->item = item;
    proc->mid = GetItemDescId(item);
}

void HbRedirect_Page5Magic(struct HelpBoxProc *proc)
{
    if (GetPage5HelpItem(proc->info) != ITEM_NONE)
        return;

    switch (proc->moveKey) {
    case DPAD_DOWN:
        TryRelocateHbDown(proc);
        break;

    case DPAD_LEFT:
        TryRelocateHbLeft(proc);
        break;

    case DPAD_RIGHT:
        TryRelocateHbRight(proc);
        break;

    default:
        TryRelocateHbUp(proc);
        break;
    }
}
