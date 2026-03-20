#include "common-chax.h"

LYN_REPLACE_CHECK(LoadTrapData);
void LoadTrapData(const struct TrapData * data)
{
    if (!data || !data->type)
        return;

    while (data->type)
    {
        switch (data->type) {
        case TRAP_BALLISTA:
            AddBallista(data->xPos, data->yPos, data->subtype);
            break;

        case TRAP_FIRETILE:
            AddFireTile(data->xPos, data->yPos, data->turn_counter, data->turn);
            break;

        case TRAP_GAS:
            AddGasTrap(data->xPos, data->yPos, data->subtype, data->turn_counter, data->turn);
            break;

        case TRAP_8:
            AddTrap8(data->xPos, data->yPos);
            break;

        case TRAP_9:
            AddTrap9(data->xPos, data->yPos, data->subtype);
            break;

        case TRAP_MINE:
            AddTrap(data->xPos, data->yPos, data->type, 0);
            break;

        case TRAP_LIGHTARROW:
            AddArrowTrap(data->xPos, data->turn_counter, data->turn);
            break;

        case TRAP_GORGON_EGG:
            AddGorgonEggTrap(data->xPos, data->yPos, data->subtype, data->turn_counter, data->turn);
            break;

        case TRAP_HEAL_TILE:
            AddHealTile(data->xPos, data->yPos, 10, data->turn_counter);
            break;

        case TRAP_TOGGLE_TORCH:
            AddToggleTorch(data->xPos, data->yPos, data->turn_counter, data->subtype);
            break;
        }

        data++;
    }
}

LYN_REPLACE_CHECK(AddDamagingTrap);
struct Trap* AddDamagingTrap(int x, int y, int trapType, int meta, int turnCountdown, int turnInterval, int damage)
{
    struct Trap* trap = AddTrap(x, y, trapType, meta);

    trap->data[TRAP_EXTDATA_TRAP_TURNFIRST] = turnCountdown;
    trap->data[TRAP_EXTDATA_TRAP_TURNNEXT]  = turnInterval;
    trap->data[TRAP_EXTDATA_TRAP_COUNTER]   = turnCountdown;
    trap->data[TRAP_EXTDATA_TRAP_DAMAGE]    = damage;

    return trap;
}

LYN_REPLACE_CHECK(DecayTraps);
void DecayTraps(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_TORCHLIGHT:
            trap->extra--;

            if (trap->extra == 0)
            {
                RemoveTrap(trap);
                trap--;
            }

            break;

        case TRAP_LIGHT_RUNE:
            trap->data[TRAP_EXTDATA_RUNE_TURNSLEFT]--;

            if (trap->data[TRAP_EXTDATA_RUNE_TURNSLEFT] == 0)
            {
                RemoveLightRune(trap);
                trap--;
            }

            break;

        case TRAP_HEAL_TILE:
            trap->data[TRAP_EXTDATA_HEALTILE_TURNSLEFT]--;
            if (trap->data[TRAP_EXTDATA_HEALTILE_TURNSLEFT] == 0)
            {
                RemoveTrap(trap);
                trap--;
            }

            break;

        case TRAP_TOGGLE_TORCH:
            if (trap->extra > 0)
                trap->extra--;

            break;

        } // switch (trap->type)
    }
}

void AddHealTile(int x, int y, int healAmount, int turnsLeft)
{
    struct Trap* trap = AddTrap(x, y, TRAP_HEAL_TILE, healAmount);
    trap->data[TRAP_EXTDATA_HEALTILE_TURNSLEFT] = turnsLeft;
}

void AddToggleTorch(int x, int y, int duration, int startsLit)
{
    struct Trap *trap;

    if (duration <= 0)
        duration = 3;

    trap = AddTrap(x, y, TRAP_TOGGLE_TORCH, 0);
    trap->data[TRAP_EXTDATA_TOGGLE_TORCH_DURATION] = duration;
    trap->extra = startsLit ? duration : 0;
}