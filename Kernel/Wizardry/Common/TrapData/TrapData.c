#include "common-chax.h"
#include "action-expa.h"
#include "debuff.h"
#include "event-rework.h"
#include "kernel-lib.h"
#include "kernel/traps.h"

extern bool Generic_CanUnitBeOnPos(struct Unit *unit, s8 x, s8 y, int x2, int y2);

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

static int SanitizeSpinTileDirection(int direction)
{
    switch (direction) {
    case SPIN_TILE_DIR_LEFT:
    case SPIN_TILE_DIR_RIGHT:
    case SPIN_TILE_DIR_UP:
    case SPIN_TILE_DIR_DOWN:
        return direction;

    default:
        return SPIN_TILE_DIR_LEFT;
    }
}

static void GetSpinTileOffset(int direction, int *outX, int *outY)
{
    *outX = 0;
    *outY = 0;

    switch (SanitizeSpinTileDirection(direction)) {
    case SPIN_TILE_DIR_LEFT:
        *outX = -1;
        break;

    case SPIN_TILE_DIR_RIGHT:
        *outX = 1;
        break;

    case SPIN_TILE_DIR_UP:
        *outY = -1;
        break;

    case SPIN_TILE_DIR_DOWN:
        *outY = 1;
        break;
    }
}

static bool DoesSpinTileBlockTrap(const struct Trap *trap)
{
    if (!trap)
        return false;

    switch (trap->type) {
    case TRAP_LIGHT_RUNE:
    case TRAP_OBSTACLE:
    case TRAP_BOULDER_TILE:
        return true;

    default:
        return false;
    }
}

static bool DoesSpinTileBlockUnit(const struct Unit *unit, int x, int y)
{
    FOR_UNITS_ONMAP_ALL(otherUnit, {
        if (otherUnit == unit)
            continue;

        if (otherUnit->xPos != x || otherUnit->yPos != y)
            continue;

        return true;
    });

    return false;
}

static bool CanSpinTileMoveUnitTo(struct Unit *unit, int x, int y)
{
    if (DoesSpinTileBlockUnit(unit, x, y))
        return false;

    if (!Generic_CanUnitBeOnPos(unit, x, y, -1, -1))
        return false;

    if (DoesSpinTileBlockTrap(GetTrapAt(x, y)))
        return false;

    return true;
}

static bool IsSpinTileVisited(const u8 *visited, int x, int y)
{
    int index = y * 64 + x;

    return (visited[index >> 3] & (1 << (index & 7))) != 0;
}

static void SetSpinTileVisited(u8 *visited, int x, int y)
{
    int index = y * 64 + x;

    visited[index >> 3] |= 1 << (index & 7);
}

struct SpinTileSequenceProc {
    PROC_HEADER;
    struct Unit *unit;
    int waitTimer;
    u8 visited[(64 * 64) / 8];
};

enum {
    SPIN_TILE_STEP_DELAY = 10,
};

static bool GetSpinTileNextHop(struct Unit *unit, int startX, int startY, const u8 *visited, int *outX, int *outY)
{
    struct Trap *trap;
    int offsetX;
    int offsetY;
    int nextX;
    int nextY;

    trap = GetTypedTrapAt(startX, startY, TRAP_SPIN_TILE);
    if (!trap)
        return false;

    GetSpinTileOffset(trap->data[TRAP_EXTDATA_SPIN_TILE_DIRECTION], &offsetX, &offsetY);

    nextX = startX + offsetX;
    nextY = startY + offsetY;

    if (!CanSpinTileMoveUnitTo(unit, nextX, nextY))
        return false;

    if (IsSpinTileVisited(visited, nextX, nextY))
        return false;

    *outX = nextX;
    *outY = nextY;
    return true;
}

static bool HasSpinTileNextHop(struct Unit *unit, int startX, int startY)
{
    u8 visited[(64 * 64) / 8] = { 0 };
    int nextX;
    int nextY;

    return GetSpinTileNextHop(unit, startX, startY, visited, &nextX, &nextY);
}

static int GetSpinTileFacingFromDelta(int deltaX, int deltaY)
{
    if (deltaX < 0)
        return MU_FACING_LEFT;

    if (deltaX > 0)
        return MU_FACING_RIGHT;

    if (deltaY < 0)
        return MU_FACING_UP;

    return MU_FACING_DOWN;
}

static struct MuProc *EnsureSpinTileMu(struct Unit *unit)
{
    struct MuProc *mu = GetUnitMu(unit);

    if (mu) {
        FreezeSpriteAnim(mu->sprite_anim);
        return mu;
    }

    HideUnitSprite(unit);
    mu = StartMu(unit);
    if (!mu)
        return NULL;

    FreezeSpriteAnim(mu->sprite_anim);
    SetMuDefaultFacing(mu);
    return mu;
}

static void CleanupSpinTileSequence(struct SpinTileSequenceProc *proc)
{
    struct MuProc *mu;

    if (!UNIT_IS_VALID(proc->unit))
        return;

    gActionData.xMove = proc->unit->xPos;
    gActionData.yMove = proc->unit->yPos;

    mu = GetUnitMu(proc->unit);
    if (mu)
        EndMu(mu);

    ShowUnitSprite(proc->unit);
    RefreshEntityBmMaps();
    RenderBmMap();
    RefreshUnitSprites();
}

static bool StartSpinTileHop(struct SpinTileSequenceProc *proc)
{
    struct MuProc *mu;
    int prevX;
    int prevY;
    int nextX;
    int nextY;
    int deltaX;
    int deltaY;

    if (!GetSpinTileNextHop(proc->unit, proc->unit->xPos, proc->unit->yPos, proc->visited, &nextX, &nextY))
        return false;

    mu = EnsureSpinTileMu(proc->unit);
    if (!mu)
        return false;

    prevX = proc->unit->xPos;
    prevY = proc->unit->yPos;

    SetSpinTileVisited(proc->visited, nextX, nextY);

    deltaX = nextX - prevX;
    deltaY = nextY - prevY;

    proc->unit->xPos = nextX;
    proc->unit->yPos = nextY;
    proc->waitTimer = SPIN_TILE_STEP_DELAY;

    gActionData.xMove = nextX;
    gActionData.yMove = nextY;
    gActionDataExpa.refrain_action = true;

    mu->x_q4 = nextX << 8;
    mu->y_q4 = nextY << 8;
    SetMuFacing(mu, GetSpinTileFacingFromDelta(deltaX, deltaY));

    RefreshEntityBmMaps();

    RenderBmMap();
    PlaySoundEffect(0x6A);
    return true;
}

static void SpinTileSequence_Init(struct SpinTileSequenceProc *proc)
{
    memset(proc->visited, 0, sizeof(proc->visited));
    proc->waitTimer = 0;

    if (!UNIT_IS_VALID(proc->unit)) {
        Proc_Break(proc);
        return;
    }

    SetSpinTileVisited(proc->visited, proc->unit->xPos, proc->unit->yPos);

    if (!StartSpinTileHop(proc))
        Proc_Break(proc);
}

static void SpinTileSequence_Loop(struct SpinTileSequenceProc *proc)
{
    if (!UNIT_IS_VALID(proc->unit)) {
        Proc_Break(proc);
        return;
    }

    if (proc->waitTimer > 0) {
        proc->waitTimer--;
        return;
    }

    if (!StartSpinTileHop(proc))
        Proc_Break(proc);
}

static const struct ProcCmd ProcScr_PostAction_SpinTileSequence[] = {
    PROC_NAME("PostAction_SpinTileSequence"),
    PROC_SET_END_CB(CleanupSpinTileSequence),
    PROC_CALL(SpinTileSequence_Init),
    PROC_REPEAT(SpinTileSequence_Loop),
    PROC_END,
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
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_RUNE_PALETTE]);

    case TRAP_HEAL_TILE:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_HEALTILE_PALETTE]);

    case TRAP_TOGGLE_TORCH:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_TOGGLE_TORCH_PALETTE]);

    case TRAP_TELEPORT_TILE:
        return SanitizeTrapMapSpritePalette(trap->data[TRAP_EXTDATA_TELEPORT_PALETTE]);

    case TRAP_GRASS_TILE:
    case TRAP_BOULDER_TILE:
    case TRAP_SPIN_TILE:
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
        trap->data[TRAP_EXTDATA_RUNE_PALETTE] = sanitizedPalette;
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
    case TRAP_SPIN_TILE:
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

        case TRAP_SPIN_TILE:
            AddSpinTile(data->xPos, data->yPos, data->subtype);
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

struct Trap *AddSpinTile(int x, int y, int direction)
{
    struct Trap *trap;

    if (!IsPositionValid(x, y))
        return NULL;

    trap = GetTypedTrapAt(x, y, TRAP_SPIN_TILE);
    if (!trap)
        trap = AddTrap(x, y, TRAP_SPIN_TILE, 0);

    if (!trap)
        return NULL;

    trap->data[TRAP_EXTDATA_SPIN_TILE_DIRECTION] = SanitizeSpinTileDirection(direction);
    return trap;
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

    if (!trap)
        return NULL;

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

bool PostAction_SpinTile(ProcPtr parent)
{
    struct Trap *trap;
    struct SpinTileSequenceProc *proc;

    if (!UNIT_IS_VALID(gActiveUnit))
        return false;

    if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

    trap = GetTypedTrapAt(gActiveUnit->xPos, gActiveUnit->yPos, TRAP_SPIN_TILE);
    if (!trap)
        return false;

    if (!HasSpinTileNextHop(gActiveUnit, gActiveUnit->xPos, gActiveUnit->yPos)) {
        RefreshEntityBmMaps();
        RenderBmMap();
        RefreshUnitSprites();
        return false;
    }

    gActionDataExpa.refrain_action = true;

    proc = Proc_StartBlocking(ProcScr_PostAction_SpinTileSequence, parent);
    proc->unit = gActiveUnit;
    return true;
}