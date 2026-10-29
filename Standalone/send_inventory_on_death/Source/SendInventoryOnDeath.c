/*
 * Send Inventory On Death for clean FE8U.
 *
 * When a player unit is defeated in battle or in the arena, its items are
 * sent to the convoy (as long as the convoy has room) before it dies.
 * Ported from the C Skill System's KillUnitOnCombatDeath and
 * KillUnitOnArenaDeathMaybe (send_inventory_on_death flag).
 */
#include "global.h"
#include "bmunit.h"
#include "bmcontainer.h"
#include "bmsave.h"

#include "constants/classes.h"

static void SendInventoryToConvoy(struct Unit * unit)
{
    int i, sent;

    if (UNIT_FACTION(unit) != FACTION_BLUE)
        return;

    /* Summoned phantoms vanish with their weapon (vanilla never keeps it). */
    if (UNIT_IS_PHANTOM(unit))
        return;

    for (sent = 0; sent < UNIT_ITEM_COUNT; sent++)
    {
        if (unit->items[sent] == 0)
            break;

        if (GetConvoyItemCount() >= CONVOY_ITEM_COUNT)
            break;

        AddItemToConvoy(unit->items[sent]);
    }

    /* Take the sent items out of the unit's inventory, so they aren't
     * duplicated if the unit comes back (e.g. with Casual Mode). Items that
     * didn't fit in the convoy stay with the unit. */
    for (i = 0; i < UNIT_ITEM_COUNT; i++)
        unit->items[i] = (i + sent < UNIT_ITEM_COUNT) ? unit->items[i + sent] : 0;
}

/* Replaces vanilla KillUnitOnCombatDeath (FE8U: 0x08032728) */
void KillUnitOnCombatDeath_SendInventory(struct Unit * unitA, struct Unit * unitB)
{
    if (GetUnitCurrentHp(unitA) != 0)
        return;

    SendInventoryToConvoy(unitA);

    PidStatsRecordDefeatInfo(unitA->pCharacterData->number, unitB->pCharacterData->number, DEFEAT_CAUSE_COMBAT);

    UnitKill(unitA);
}

/* Replaces vanilla KillUnitOnArenaDeathMaybe (FE8U: 0x08032750) */
void KillUnitOnArenaDeath_SendInventory(struct Unit * unit)
{
    if (GetUnitCurrentHp(unit) != 0)
        return;

    SendInventoryToConvoy(unit);

    UnitKill(unit);

    PidStatsRecordDefeatInfo(unit->pCharacterData->number, 0, DEFEAT_CAUSE_ARENA);
}
