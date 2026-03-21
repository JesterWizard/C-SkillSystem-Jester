#include "common-chax.h"
#include "action-expa.h"
#include "debuff.h"
#include "event-rework.h"
#include "kernel-lib.h"
#include "kernel/traps.h"

static void PrepareTeleportTileWarp(void)
{
    EndAllMus();
    RefreshUnitSprites();
}

static void SetTeleportActorUnit(void)
{
    gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

static void SetTeleportDestination(void)
{
    gActiveUnit->xPos = gActionData.xMove;
    gActiveUnit->yPos = gActionData.yMove;
}

static const EventScr EventScr_PostAction_TeleportTile[] = {
    EVBIT_MODIFY(0x4)
    ASMC(PrepareTeleportTileWarp)
    ASMC(SetTeleportActorUnit)
    CALL(EventScr_UidWarpOUT)
    STAL(20)
    ASMC(SetTeleportDestination)
    ASMC(SetTeleportActorUnit)
    CALL(EventScr_UidWarpIN)
    STAL(20)
    NOFADE
    ENDA
};

static struct Trap *(*const sVanillaAddLightRune)(int x, int y) = (void *) 0x0802EA59;

static struct Trap *RemoveGrassTile(struct Trap *trap)
{
    if (!trap || trap->type != TRAP_GRASS_TILE)
        return trap;

    if (IsPositionValid(trap->xPos, trap->yPos))
        gBmMapTerrain[trap->yPos][trap->xPos] = trap->extra;

    return RemoveTrap(trap);
}

static int SanitizeTrapMapSpritePalette(int palette)
{
    if (palette < TRAP_MAPSPRITE_PAL_DEFAULT || palette > TRAP_MAPSPRITE_PAL_GREY)
        return TRAP_MAPSPRITE_PAL_DEFAULT;

    return palette;
}

int GetTrapMapSpritePalette(const struct Trap *trap)
{
    if (!trap)
        return TRAP_MAPSPRITE_PAL_DEFAULT;

    switch (trap->type) {
    case TRAP_LIGHT_RUNE:
        return SanitizeTrapMapSpritePalette(trap->extra);

    case TRAP_HEAL_TILE:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_HEALTILE_PALETTE]);

    case TRAP_TOGGLE_TORCH:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_TOGGLE_TORCH_PALETTE]);

    case TRAP_TELEPORT_TILE:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_TELEPORT_PALETTE]);

    case TRAP_GRASS_TILE:
    case TRAP_BOULDER_TILE:
        return TRAP_MAPSPRITE_PAL_DEFAULT;

    default:
        return TRAP_MAPSPRITE_PAL_DEFAULT;
    }
}

void SetTrapMapSpritePalette(struct Trap *trap, int palette)
{
    int sanitizedPalette;

    if (!trap)
        return;

    sanitizedPalette = SanitizeTrapMapSpritePalette(palette);

    switch (trap->type) {
    case TRAP_LIGHT_RUNE:
        trap->extra = sanitizedPalette;
        break;

    case TRAP_HEAL_TILE:
        trap->data[TRAP_EXTDATA_HEALTILE_PALETTE] = sanitizedPalette;
        break;

    case TRAP_TOGGLE_TORCH:
        trap->data[TRAP_EXTDATA_TOGGLE_TORCH_PALETTE] = sanitizedPalette;
        break;

    case TRAP_TELEPORT_TILE:
        trap->data[TRAP_EXTDATA_TELEPORT_PALETTE] = sanitizedPalette;
        break;

    case TRAP_GRASS_TILE:
    case TRAP_BOULDER_TILE:
        break;
    }
}

int GetEffectiveTerrainAt(int x, int y)
{
    if (GetTypedTrapAt(x, y, TRAP_GRASS_TILE))
        return TERRAIN_FOREST;

    return GetTrueTerrainAt(x, y);
}

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
            AddHealTile(data->xPos, data->yPos, 10, data->turn_counter, data->subtype);
            break;

        case TRAP_TOGGLE_TORCH:
            AddToggleTorch(data->xPos, data->yPos, data->turn_counter, data->subtype, data->turn);
            break;

        case TRAP_TELEPORT_TILE:
            AddTeleportTile(data->xPos, data->yPos, data->subtype, data->turn_counter, data->turn);
            break;

        case TRAP_GRASS_TILE:
            AddGrassTile(data->xPos, data->yPos, data->turn_counter);
            break;

        case TRAP_BOULDER_TILE:
            AddBoulderTile(data->xPos, data->yPos);
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

        case TRAP_GRASS_TILE:
            if (trap->data[TRAP_EXTDATA_GRASS_TILE_TURNSLEFT] > 0)
            {
                trap->data[TRAP_EXTDATA_GRASS_TILE_TURNSLEFT]--;

                if (trap->data[TRAP_EXTDATA_GRASS_TILE_TURNSLEFT] == 0)
                {
                    RemoveGrassTile(trap);
                    trap--;
                }
            }

            break;

        } // switch (trap->type)
    }
}

struct Trap *AddHealTile(int x, int y, int healAmount, int turnsLeft, int palette)
{
    struct Trap *trap = AddTrap(x, y, TRAP_HEAL_TILE, healAmount);

    if (!trap)
        return NULL;

    trap->data[TRAP_EXTDATA_HEALTILE_TURNSLEFT] = turnsLeft;

    SetTrapMapSpritePalette(trap, palette);

    return trap;
}

struct Trap *AddGrassTile(int x, int y, int turnsLeft)
{
    struct Trap *trap;

    if (!IsPositionValid(x, y))
        return NULL;

    if (turnsLeft < 0)
        turnsLeft = 0;

    trap = GetTypedTrapAt(x, y, TRAP_GRASS_TILE);
    if (trap)
    {
        trap->data[TRAP_EXTDATA_GRASS_TILE_TURNSLEFT] = turnsLeft;
        return trap;
    }

    trap = AddTrap(x, y, TRAP_GRASS_TILE, gBmMapTerrain[y][x]);
    if (!trap)
        return NULL;

    trap->data[TRAP_EXTDATA_GRASS_TILE_TURNSLEFT] = turnsLeft;

    gBmMapTerrain[y][x] = TERRAIN_FOREST;

    return trap;
}

struct Trap *AddBoulderTile(int x, int y)
{
    struct Trap *trap;

    if (!IsPositionValid(x, y))
        return NULL;

    trap = GetTypedTrapAt(x, y, TRAP_BOULDER_TILE);
    if (trap)
        return trap;

    return AddTrap(x, y, TRAP_BOULDER_TILE, 0);
}

struct Trap *AddToggleTorch(int x, int y, int duration, int startsLit, int palette)
{
    struct Trap *trap;

    if (duration <= 0)
        duration = 3;

    trap = AddTrap(x, y, TRAP_TOGGLE_TORCH, 0);
    if (!trap)
        return NULL;

    trap->data[TRAP_EXTDATA_TOGGLE_TORCH_DURATION] = duration;
    trap->extra = startsLit ? duration : 0;

    SetTrapMapSpritePalette(trap, palette);

    return trap;
}

struct Trap* AddTeleportTile(int x, int y, int destX, int destY, int palette)
{
    struct Trap *trap = AddTrap(x, y, TRAP_TELEPORT_TILE, 0);

    if (!trap)
        return NULL;

    trap->data[TRAP_EXTDATA_TELEPORT_DEST_X] = destX;
    trap->data[TRAP_EXTDATA_TELEPORT_DEST_Y] = destY;

    SetTrapMapSpritePalette(trap, palette);

    return trap;
}

void AddTeleportTilePair(int x1, int y1, int x2, int y2)
{
    AddTeleportTile(x1, y1, x2, y2, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
    AddTeleportTile(x2, y2, x1, y1, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
}

LYN_REPLACE_CHECK(AddLightRune);
struct Trap *AddLightRune(int x, int y, int palette)
{
    struct Trap *trap = sVanillaAddLightRune(x, y);

    SetTrapMapSpritePalette(trap, palette);

    return trap;
}

bool PostAction_TeleportTile(ProcPtr parent)
{
    struct Trap *trap;
    int destX;
    int destY;

    if (!UNIT_IS_VALID(gActiveUnit))
        return false;

    if (gActionData.moveCount <= 0)
        return false;

    if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

    trap = GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_TELEPORT_TILE);
    if (!trap)
        return false;

    destX = trap->data[TRAP_EXTDATA_TELEPORT_DEST_X];
    destY = trap->data[TRAP_EXTDATA_TELEPORT_DEST_Y];

    if (destX < 0 || destY < 0 || destX >= gBmMapSize.x || destY >= gBmMapSize.y)
        return false;

    if ((destX == gActiveUnit->xPos) && (destY == gActiveUnit->yPos))
        return false;

    if (gBmMapUnit[destY][destX] != 0)
        return false;

    gActionData.xMove = destX;
    gActionData.yMove = destY;
    gActionDataExpa.refrain_action = true;

    KernelCallEvent(EventScr_PostAction_TeleportTile, EV_EXEC_CUTSCENE, parent);
    return true;
}