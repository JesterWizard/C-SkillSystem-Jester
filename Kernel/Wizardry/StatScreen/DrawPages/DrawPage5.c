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

static void PutGaidenCost(int number, u16 *tm)
{
    if (number == 0)
        return;

    PutNumberSmall(tm + 2, TEXT_COLOR_SYSTEM_GRAY, number);
}

static void WhiteMagicList(struct Unit * unit)
{
    PutDrawText(
        &gStatScreen.text[WHITE_MAGIC_STARTING_TEXT_FIELD],
        gUiTmScratchA + TILEMAP_INDEX(0x1, 0x1),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0,
        "White Magic");

	if (gpKernelDesignerConfig->gaiden_magic == true)
	{
        struct GaidenMagicList *gmaglist = GetGaidenMagicList(unit);

		for (int i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			u16 item = gmaglist->wmags[i];

			if (item == 0)
				break;

            DrawIcon(gUiTmScratchA + TILEMAP_INDEX(0x0, 0x3 + (i * 2)), GetItemIconId(item), 0x4000);

            PutDrawText(
                &gStatScreen.text[WHITE_MAGIC_STARTING_TEXT_FIELD + (i + 1)],
                gUiTmScratchA + TILEMAP_INDEX(0x2, 0x3 + (i * 2)),
                TEXT_COLOR_SYSTEM_GOLD,
                0, 0,
                Utf8ToNarrowFonts(GetItemName(item)));

            PutGaidenCost(GetGaidenWeaponHpCost(unit, item), gUiTmScratchA + TILEMAP_INDEX(0x5, 0x3 + (i * 2)));
		}
    }
};

static void BlackMagicList(struct Unit * unit)
{
    PutDrawText(
        &gStatScreen.text[BLACK_MAGIC_STARTING_TEXT_FIELD],
        gUiTmScratchA + TILEMAP_INDEX(0x9, 0x1),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0,
        "Black Magic");

	if (gpKernelDesignerConfig->gaiden_magic == true)
	{
        struct GaidenMagicList *gmaglist = GetGaidenMagicList(unit);

		for (int i = 0; i < GAIDEN_MAGIC_LIST_LEN; i++) {
			u16 item = gmaglist->bmags[i];

			if (item == 0)
				break;

            DrawIcon(gUiTmScratchA + TILEMAP_INDEX(0x8, 0x3 + (i * 2)), GetItemIconId(item), 0x4000);

            PutDrawText(
                &gStatScreen.text[BLACK_MAGIC_STARTING_TEXT_FIELD + (i +1)],
                gUiTmScratchA + TILEMAP_INDEX(0xA, 0x3 + (i * 2)),
                TEXT_COLOR_SYSTEM_GOLD,
                0, 0,
                Utf8ToNarrowFonts(GetItemName(item)));

            PutGaidenCost(GetGaidenWeaponHpCost(unit, item), gUiTmScratchA + TILEMAP_INDEX(0xE, 0x3 + (i * 2)));
		}
    }
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

    WhiteMagicList(unit);
    BlackMagicList(unit);
    DisplayMP(unit);
};

void DrawPage5Rework(void)
{
    int i;
    for (i = 15; i < 21; i++)
        ClearText(&gStatScreen.text[i]);

    for (i = 29; i < 35; i++)
        ClearText(&gStatScreen.text[i]);

    DrawPage5MagicList();
}
