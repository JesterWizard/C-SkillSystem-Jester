#include "common-chax.h"
#include "item-sys.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "strmag.h"

static bool CustomStavesEnabled(void)
{
    return gpKernelDesignerConfig->custom_staves == true;
}

//! FE8U = 0x08034B48
LYN_REPLACE_CHECK(DrawUnitResChangeText);
void DrawUnitResChangeText(struct Text* text, struct Unit* unit, int bonus) {
    ClearText(text);

    char * statName = "Null";
    int statNumber = 0;

    int itemId = GetItemIndex(gActiveUnit->items[0]);

    switch (itemId)
    {
    case ITEM_STAFF_FORCE:
        statName = "Str";
        statNumber = GetUnitPower(unit);
        break;
    case ITEM_STAFF_TEMPEST:
        statName = "Mag";
        statNumber = GetUnitMagic(unit);
        break;
    case ITEM_STAFF_ACUITY:
        statName = "Skl";
        statNumber = GetUnitSkill(unit);
        break;
    case ITEM_STAFF_SPRINT:
        statName = "Spd";
        statNumber = GetUnitSpeed(unit);
        break;
    case ITEM_STAFF_FORTUNE:
        statName = "Lck";
        statNumber = GetUnitLuck(unit);
        break;
    case ITEM_STAFF_IRON:
        statName = "Def";
        statNumber = GetUnitDefense(unit);
        break;
    case ITEM_STAFF_BARRIER:
        statName = "Res";
        statNumber = GetUnitResistance(unit);
        break;
    case ITEM_STAFF_OMNI:
        statName = "Omni";
        statNumber = 0;
        break;

    default:
        break;
    }

    Text_InsertDrawString(text, 0, 3, statName);
    Text_InsertDrawNumberOrBlank(text, 32, 2, statNumber);
    Text_InsertDrawNumberOrBlank(text, 56, 2, statNumber + bonus);

    Text_InsertDrawString(text, 40, 3, " - ");

    return;
}

//! FE8U = 0x080350A4
LYN_REPLACE_CHECK(RefreshUnitResChangeInfoWindow);
void RefreshUnitResChangeInfoWindow(struct Unit* unit) {
    if (!CustomStavesEnabled() && GetItemIndex(gActiveUnit->items[0]) != ITEM_STAFF_BARRIER)
        return;

    int y = 0;
    int x = GetUnitInfoWindowX(unit, 10);

    struct UnitInfoWindowProc* proc = UnitInfoWindow_DrawBase(0, unit, x, y, 10, 1);

    DrawUnitResChangeText(proc->lines + 0, unit, 7 - unit->barrierDuration);
    PutText(proc->lines + 0, gBG0TilemapBuffer + TILEMAP_INDEX(x + 1, y + 3));

    return;
}