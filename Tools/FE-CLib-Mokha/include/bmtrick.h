#ifndef GUARD_BMTRICK_H
#define GUARD_BMTRICK_H

enum
{
    TRAP_MAX_COUNT = 64,
};

enum
{
    TRAP_NONE       = 0,
    TRAP_BALLISTA   = 1,
    TRAP_OBSTACLE   = 2, // walls & snags
    TRAP_MAPCHANGE  = 3,
    TRAP_FIRETILE   = 4,
    TRAP_GAS        = 5,
    TRAP_MAPCHANGE2 = 6, // TODO: figure out
    TRAP_LIGHTARROW = 7,
    TRAP_8          = 8,
    TRAP_9          = 9,
    TRAP_TORCHLIGHT = 10,
    TRAP_MINE       = 11,
    TRAP_GORGON_EGG = 12, // TODO: figure out
    TRAP_LIGHT_RUNE = 13,
    TRAP_14         = 14,
    TRAP_FIRE_THIEF = 15,
    TRAP_MINE_ASSASSIN = 16,
    TRAP_HEAL_TILE = 17,
    TRAP_TOGGLE_TORCH = 18,
    TRAP_TELEPORT_TILE = 19,
    TRAP_GRASS_TILE = 20,
};

enum
{
    TRAP_MAPSPRITE_PAL_DEFAULT = 0,
    TRAP_MAPSPRITE_PAL_LIGHT_RUNE,
    TRAP_MAPSPRITE_PAL_PLAYER,
    TRAP_MAPSPRITE_PAL_ENEMY,
    TRAP_MAPSPRITE_PAL_NPC,
    TRAP_MAPSPRITE_PAL_GREY,
    TRAP_MAPSPRITE_PAL_FOURTH = TRAP_MAPSPRITE_PAL_GREY,
};

enum
{
    // Ballista extdata definitions
    TRAP_EXTDATA_BLST_RIDDEN   = 1, // "is ridden" boolean
    TRAP_EXTDATA_BLST_ITEMUSES = 2, // ballista item uses

    // Trap (Fire/Gas/Arrow) extdata definitions
    TRAP_EXTDATA_TRAP_TURNFIRST = 0, // start turn countdown
    TRAP_EXTDATA_TRAP_TURNNEXT  = 1, // repeat turn countdown
    TRAP_EXTDATA_TRAP_COUNTER   = 2, // turn counter
    TRAP_EXTDATA_TRAP_DAMAGE    = 3, // trap damage (needs confirmation)

    // Light Rune extdata definitions
    TRAP_EXTDATA_RUNE_PALETTE          = 0, // map sprite palette selector
    TRAP_EXTDATA_RUNE_TURNSLEFT        = 2, // turns left before wearing out

    // Heal Tile extdata definitions
    TRAP_EXTDATA_HEALTILE_TURNSLEFT = 0, // turns remaining (0 = permanent)
    TRAP_EXTDATA_HEALTILE_PALETTE   = 1, // map sprite palette selector

    // Toggle Torch extdata definitions
    TRAP_EXTDATA_TOGGLE_TORCH_DURATION = 0, // turns to stay lit when switched on
    TRAP_EXTDATA_TOGGLE_TORCH_PALETTE  = 1, // map sprite palette selector

    // Teleport tile extdata definitions
    TRAP_EXTDATA_TELEPORT_DEST_X = 0,
    TRAP_EXTDATA_TELEPORT_DEST_Y = 1,
    TRAP_EXTDATA_TELEPORT_PALETTE = 2,
};

struct Trap
{
    /* 00 */ u8 xPos;
    /* 01 */ u8 yPos;

    /* 02 */ u8 type;

    /* 03 */ u8 extra; // extra data (meaning varies based on trap type)
    /* 04 */ s8 data[4]; // more extra data (see above enum for per trap type entry allocations)
};

#define TRAP_INDEX(aTrap) ((aTrap) - GetTrap(0))

void ClearTraps(void);
struct Trap* GetTrapAt(int x, int y);
struct Trap* GetTypedTrapAt(int x, int y, int trapType);
int GetTrapMapSpritePalette(const struct Trap* trap);
void SetTrapMapSpritePalette(struct Trap* trap, int palette);
struct Trap* AddTrap(int x, int y, int trapType, int meta);
struct Trap* AddDamagingTrap(int x, int y, int trapType, int meta, int turnCountdown, int turnInterval, int damage);
struct Trap* RemoveTrap(struct Trap* trap);
struct Trap* AddTeleportTile(int x, int y, int destX, int destY, int palette);
void AddTeleportTilePair(int x1, int y1, int x2, int y2);
struct Trap* AddGrassTile(int x, int y);
void AddFireTile(int x, int y, int turnCountdown, int turnInterval);
void AddGasTrap(int x, int y, int facing, int turnCountdown, int turnInterval);
void AddArrowTrap(int x, int turnCountdown, int turnInterval);
void sub_802E36C(int x, int y, int turnCountdown, int turnInterval);
void AddTrap8(int x, int y);
void AddTrap9(int x, int y, int meta);
void InitMapObstacles(void);
void ApplyEnabledMapChanges(void);
void RefreshAllLightRunes(void);
int GetObstacleHpAt(int x, int y);
const struct MapChange* GetMapChange(int id);
int GetMapChangeIdAt(int x, int y);
void ApplyMapChangesById(int mapChangeId);
void EnableMapChange(int mapChangeId);
void DisableMapChange(int id);
s8 IsMapChangeEnabled(int id);
void UnitHideIfUnderRoof(struct Unit* unit);
void UpdateRoofedUnits(void);
void GenerateTrapDamageTargets(void);
void GenerateDisplayedTrapDamageTargets(void);
void CountDownTraps(void);
void ResetCountedDownTraps(void);
void sub_802EA00(void);
void sub_802EA1C(void);
void PostTrapExecFlag(void);
struct Trap* AddLightRune(int x, int y, int palette);
struct Trap* RemoveLightRune(struct Trap* trap);
void DecayTraps(void);
void DisableAllLightRunes(void);
void EnableAllLightRunes(void);
struct Trap* GetTrap(int id);
int GetEffectiveTerrainAt(int x, int y);

#define TELEPORT_TILE(x, y, destX, destY, palette) \
    TRAP_TELEPORT_TILE, (x), (y), (destX), (destY), (palette)

#define TELEPORT_TILE_PAIR(x1, y1, x2, y2) \
    TELEPORT_TILE((x1), (y1), (x2), (y2), TRAP_MAPSPRITE_PAL_DEFAULT), \
    TELEPORT_TILE((x2), (y2), (x1), (y1), TRAP_MAPSPRITE_PAL_DEFAULT)

#define HEAL_TILE(x, y, turnsLeft, palette) \
    TRAP_HEAL_TILE, (x), (y), (palette), (turnsLeft), 0

#define TOGGLE_TORCH(x, y, duration, startsLit, palette) \
    TRAP_TOGGLE_TORCH, (x), (y), (startsLit), (duration), (palette)

#define GRASS_TILE(x, y) \
    TRAP_GRASS_TILE, (x), (y), 0, 0, 0

struct Trap* AddHealTile(int x, int y, int healAmount, int turnsLeft, int palette);
struct Trap* AddToggleTorch(int x, int y, int duration, int startsLit, int palette);

#endif // GUARD_BMTRICK_H