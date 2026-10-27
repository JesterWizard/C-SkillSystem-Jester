# Trap Authoring Guide

---

## Index

1. [Introduction](#introduction)
2. [Implementation Plan](#implementation-plan)
3. [Trap IDs and Data Model](#trap-ids-and-data-model)
4. [Chapter-Start Trap Placement](#chapter-start-trap-placement)
5. [Runtime Trap Placement with ASMC](#runtime-trap-placement-with-asmc)
6. [Applying Existing Traps](#applying-existing-traps)
7. [Trap Palette Selection](#trap-palette-selection)
8. [Repeat House Notes](#repeat-house-notes)
9. [Dragon Vein Notes](#dragon-vein-notes)
10. [Grass Trap Notes](#grass-trap-notes)
11. [Boulder Trap Notes](#boulder-trap-notes)
12. [Spin Tile Notes](#spin-tile-notes)
13. [Trap Sprite Registration](#trap-sprite-registration)
14. [Trap Sprite Display in RefreshUnitSprites](#trap-sprite-display-in-refreshunitsprites)
15. [Code Locations](#code-locations)
16. [TODO](#todo)
17. [Limitations & Bugs](#limitations--bugs)

## Introduction

This document explains how to add new traps in this project from a contributor workflow perspective:

- Define and understand trap IDs and extdata fields.
- Place traps at chapter load via chapter trap headers.
- Spawn traps at runtime via event ASMC calls.
- Select which trap map sprite palette is used.
- Optionally make a trap behave like a terrain override instead of only a sprite overlay.
- Award EXP from a repeatable house visit trap without giving the player a second reward on later visits.
- Register map sprite graphics for new trap visuals.
- Render new trap types in the map sprite refresh loop.

Player-facing result: your new trap can exist at map start, be created by events mid-chapter, and show a map icon correctly with the palette you expect.

## Implementation Plan

Add a trap in this order to avoid partial integration:

1. Pick or add a trap type ID in the trap enum.
2. Ensure trap loading behavior exists in LoadTrapData for chapter-start data.
3. Add chapter trap entries in the chapter's TrapData arrays.
4. Add optional ASMC logic so events can spawn the trap mid-map.
5. Register trap map sprite graphics in TrapData_Installer.event.
6. Add trap rendering logic in RefreshUnitSprites.
7. Build and verify:

- Trap appears on chapter start.
- Trap appears when ASMC event runs.
- Sprite uses expected tile, palette, and lifetime behavior.

## Trap IDs and Data Model

### Trap enum in bmtrick.h

File: `Tools/FE-CLib-Mokha/include/bmtrick.h`

The trap enum is the canonical ID list for trap types, including project additions like `TRAP_HEAL_TILE`.

- Use these IDs everywhere (event data, C logic, rendering).
- Avoid hardcoded numeric trap IDs in game logic.
- Keep extdata semantics aligned with `TRAP_EXTDATA_*` constants.

Important extdata examples from this file:

- `TRAP_EXTDATA_TRAP_TURNFIRST`, `TRAP_EXTDATA_TRAP_TURNNEXT`, `TRAP_EXTDATA_TRAP_COUNTER`, `TRAP_EXTDATA_TRAP_DAMAGE`
- `TRAP_MAPSPRITE_PAL_DEFAULT`, `TRAP_MAPSPRITE_PAL_LIGHT_RUNE`, `TRAP_MAPSPRITE_PAL_PLAYER`, `TRAP_MAPSPRITE_PAL_ENEMY`, `TRAP_MAPSPRITE_PAL_NPC`, `TRAP_MAPSPRITE_PAL_GREY`
- `TRAP_EXTDATA_RUNE_TURNSLEFT`
- `TRAP_EXTDATA_HEALTILE_TURNSLEFT`, `TRAP_EXTDATA_HEALTILE_PALETTE`
- `TRAP_EXTDATA_TOGGLE_TORCH_DURATION`, `TRAP_EXTDATA_TOGGLE_TORCH_PALETTE`
- `TRAP_EXTDATA_TELEPORT_DEST_X`, `TRAP_EXTDATA_TELEPORT_DEST_Y`, `TRAP_EXTDATA_TELEPORT_PALETTE`
- `TRAP_EXTDATA_GRASS_TILE_TURNSLEFT`
- `TRAP_EXTDATA_SPIN_TILE_DIRECTION`

Spin tile direction constants:

- `SPIN_TILE_DIR_LEFT`
- `SPIN_TILE_DIR_RIGHT`
- `SPIN_TILE_DIR_UP`
- `SPIN_TILE_DIR_DOWN`

Palette handling summary:

- `TRAP_LIGHT_RUNE` stores its palette in `trap->extra`.
- `TRAP_HEAL_TILE`, `TRAP_TOGGLE_TORCH`, and `TRAP_TELEPORT_TILE` store their palette in trap extdata.
- `TRAP_GRASS_TILE` and `TRAP_SPIN_TILE` use dedicated custom OBJ palette banks instead of the shared trap palette enum.
- `TRAP_BOULDER_TILE` has no palette selector; its art is indexed against the shared map sprite palette and always draws in the player bank.
- `SetTrapMapSpritePalette(...)` sanitizes out-of-range values and falls back to `TRAP_MAPSPRITE_PAL_DEFAULT`.

### LoadTrapData in TrapData.c

File: `Kernel/Wizardry/TrapData/TrapData.c`

`LoadTrapData(const struct TrapData * data)` iterates the chapter trap table and dispatches by trap type. For each entry, it calls the corresponding constructor, for example:

- `TRAP_FIRETILE` -> `AddFireTile(...)`
- `TRAP_GAS` -> `AddGasTrap(...)`
- `TRAP_MINE` -> `AddTrap(...)`
- `TRAP_LIGHTARROW` -> `AddArrowTrap(...)`
- `TRAP_GORGON_EGG` -> `AddGorgonEggTrap(...)`
- `TRAP_HEAL_TILE` -> `AddHealTile(...)`
- `TRAP_TOGGLE_TORCH` -> `AddToggleTorch(...)`
- `TRAP_TELEPORT_TILE` -> `AddTeleportTile(...)`
- `TRAP_GRASS_TILE` -> `AddGrassTile(...)`
- `TRAP_BOULDER_TILE` -> `AddBoulderTile(...)`
- `TRAP_SPIN_TILE` -> `AddSpinTile(...)`

When adding a new trap type that needs special initialization, add a `case` here.

Related trap table struct:

- File: `Tools/FE-CLib-Mokha/include/bmtrap.h`
- Struct: `struct TrapData { type, xPos, yPos, subtype, turn_counter, turn }`

## Chapter-Start Trap Placement

### Chapter trap headers

Example file:

- `CustomCampaign/Chapters/01/events/traps.c`

Chapter trap arrays are encoded in 6-byte groups matching `struct TrapData`:

1. type
2. xPos
3. yPos
4. subtype
5. turn_counter
6. turn

For the reworked trap types, these bytes now also carry palette data:

| Trap | `subtype` byte | `turn_counter` byte | `turn` byte |
|--------|----------|-------------|
| Heal tile | palette | turns left | unused |
| Toggle torch | starts lit | duration | palette |
| Teleport tile | destination X | destination Y | palette |
| Grass tile | unused | turns left (`0` = permanent) | unused |
| Boulder tile | unused | unused | unused |
| Spin tile | direction (`SPIN_TILE_DIR_*`) | unused | unused |

Example pattern:

```c
static const u8 TrapData_ThisEvent[] = {
    HEAL_TILE(3, 1, 2, TRAP_MAPSPRITE_PAL_PLAYER),
    TOGGLE_TORCH(6, 4, 3, true, TRAP_MAPSPRITE_PAL_NPC),
    TELEPORT_TILE(8, 2, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY),
    SPIN_TILE(10, 5, SPIN_TILE_DIR_RIGHT),
    TRAP_NONE
};

static const u8 TrapData_ThisEventHard[] = {
    TRAP_NONE
};
```

Prefer the helper macros from `bmtrick.h` for palette-aware trap entries instead of writing the bytes manually.

### Binding trap arrays to chapter startup

Example file:

- `CustomCampaign/Chapters/00/events/events.c`

The chapter event group binds these arrays:

```c
.traps = TrapData_ThisEvent,
.extraTrapsInHard = TrapData_ThisEventHard,
```

At chapter load, these are consumed by the trap loader pipeline that reaches `LoadTrapData`.

## Runtime Trap Placement with ASMC

### Event-side call site

Example file:

- `CustomCampaign/Chapters/00/events/events.c`

An ASMC is invoked from event script with:

```c
ASMC(SetGameOptions)
```

### ASMC function that spawns a trap

Example file:

- `CustomCampaign/Chapters/00/events/asmc.h`

`SetGameOptions()` includes:

```c
AddHealTile(3, 3, 10, 2, TRAP_MAPSPRITE_PAL_PLAYER);
```

This demonstrates runtime trap spawning through C code called by event ASMC.

Recommended pattern for custom runtime trap setup:

1. Define a C function in the chapter ASMC source (or shared event utility C file).
2. Inside it, call one of:

- `AddTrap(x, y, trapType, meta)`
- `AddHealTile(x, y, healAmount, turnsLeft, palette)`
- `AddToggleTorch(x, y, duration, startsLit, palette)`
- `AddTeleportTile(x, y, destX, destY, palette)`
- `AddTeleportTilePair(x1, y1, x2, y2)`
- `AddLightRune(x, y, palette)`
- `AddGrassTile(x, y, turnsLeft)`
- `AddBoulderTile(x, y)`
- `AddSpinTile(x, y, direction)`
- other trap-specific constructors

1. Call that function from event script via `ASMC(YourFunctionName)`.

## Applying Existing Traps

This section covers the contributor-facing workflow for traps that already exist in the rework.

### Runtime usage from ASMC

For event-driven trap placement, call the trap constructor directly inside your ASMC C function.

| Trap | Runtime call | What the parameters mean |
| -------- | ---------- | ------------- |
| Heal tile | `AddHealTile(x, y, healAmount, turnsLeft, palette)` | Place a healing tile at `(x, y)` with a heal amount, optional lifetime, and selected sprite palette |
| Toggle torch | `AddToggleTorch(x, y, duration, startsLit, palette)` | Place a torch tile at `(x, y)` with a lit duration, initial lit state, and selected sprite palette |
| Teleport tile pair | `AddTeleportTilePair(x1, y1, x2, y2)` | Create two linked teleport tiles, one at each coordinate pair |
| One-way teleport tile | `AddTeleportTile(x, y, destX, destY, palette)` | Create a single teleport tile that sends units from `(x, y)` to `(destX, destY)` with a selected sprite palette |
| Light rune | `AddLightRune(x, y, palette)` | Create a light rune and set the palette used by its map sprite |
| Grass tile | `AddGrassTile(x, y, turnsLeft)` | Create a grass trap tile that draws with `Pal_Grass_Tile`, behaves like forest terrain, and optionally expires after `turnsLeft` turns (`0` = permanent) |
| Boulder tile | `AddBoulderTile(x, y)` | Create a movable boulder trap that blocks non-fliers, renders with the shared map sprite palette, and can be pushed with the `Interact` command |
| Spin tile | `AddSpinTile(x, y, direction)` | Create a directional shove tile that halts grounded movement on contact and then follows connected spin tiles one step at a time until the chain ends or a landing tile is blocked |

Runtime example from chapter ASMC code:

```c
void SetGameOptions()
{
    AddHealTile(2, 4, 10, 0, TRAP_MAPSPRITE_PAL_PLAYER);
    AddToggleTorch(6, 6, 3, true, TRAP_MAPSPRITE_PAL_NPC);
    AddTeleportTile(8, 3, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY);
    AddLightRune(10, 5, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
    AddGrassTile(3, 3, 3);
    AddBoulderTile(9, 4);
    AddSpinTile(11, 4, SPIN_TILE_DIR_LEFT);
}
```

For warp tiles specifically, use `AddTeleportTilePair` with two coordinate pairs.

- The first pair is the first tile's map position.
- The second pair is the linked destination tile's map position.
- The helper creates both trap entries automatically.

Example:

```c
AddTeleportTilePair(1, 1, 3, 3);
```

This creates:

- a teleport tile at `(1, 1)` that sends the unit to `(3, 3)`
- a teleport tile at `(3, 3)` that sends the unit back to `(1, 1)`

`AddTeleportTilePair(...)` uses `TRAP_MAPSPRITE_PAL_DEFAULT` for both tiles. Use `AddTeleportTile(...)` directly if you want one-way teleport behavior with an explicit palette choice.

### Chapter-start usage in TrapData arrays

For chapter-start trap placement, use the trap table helpers from `bmtrick.h` instead of manually counting bytes for teleport entries.

Example:

```c
static const u8 TrapData_ThisEvent[] = {
    GRASS_TILE(2, 2, 3),
    HEAL_TILE(3, 3, 2, TRAP_MAPSPRITE_PAL_PLAYER),
    TOGGLE_TORCH(6, 5, 3, true, TRAP_MAPSPRITE_PAL_NPC),
    TELEPORT_TILE(8, 2, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY),
    TELEPORT_TILE_PAIR(1, 1, 3, 3),
    BOULDER_TILE(9, 4),
    SPIN_TILE(11, 4, SPIN_TILE_DIR_LEFT),
    TRAP_NONE
};
```

Use `TELEPORT_TILE(x, y, destX, destY)` if you want one-way behavior in the chapter trap table.

Practical rule:

- Use `AddTeleportTilePair(...)` when spawning paired warp tiles from ASMC code.
- Use `TELEPORT_TILE_PAIR(...)` when placing paired warp tiles in chapter startup trap arrays.
- Use the single-tile variants only when you intentionally want one-way teleport behavior.

## Trap Palette Selection

Trap palette selection is now part of the contributor-facing trap API.

Available palette constants:

| Palette constant | Visual intent |
| -------- | ---------- |
| `TRAP_MAPSPRITE_PAL_DEFAULT` | Use the sprite's own palette: the light-rune bank for light runes, the player bank for every other trap |
| `TRAP_MAPSPRITE_PAL_LIGHT_RUNE` | Use the light-rune palette bank. Only light rune art is indexed for this bank, so other traps treat it as `DEFAULT` |
| `TRAP_MAPSPRITE_PAL_PLAYER` | Use the player/blue palette bank |
| `TRAP_MAPSPRITE_PAL_ENEMY` | Use the enemy/red palette bank |
| `TRAP_MAPSPRITE_PAL_NPC` | Use the NPC/green palette bank |
| `TRAP_MAPSPRITE_PAL_GREY` | Use the grey/fourth palette bank |

Where to set the palette:

- Runtime constructors: pass the palette as the final argument to `AddHealTile`, `AddToggleTorch`, `AddTeleportTile`, and `AddLightRune`.
- Chapter-start trap tables: use the `HEAL_TILE`, `TOGGLE_TORCH`, and `TELEPORT_TILE` helper macros with the palette constant as their final argument.
- Paired teleport helpers: `TELEPORT_TILE_PAIR` and `AddTeleportTilePair` both use `TRAP_MAPSPRITE_PAL_DEFAULT`.

The heal, torch, teleport and boulder PNGs must be indexed against the shared map sprite palette (the palette in `Heal_Tile.png`, which matches the player bank). Recoloring works by swapping that palette for the enemy/NPC/grey bank, so a color that is not one of those 16 entries cannot be recolored and will draw wrong.

Examples:

```c
AddToggleTorch(4, 8, 3, true, TRAP_MAPSPRITE_PAL_NPC);
AddTeleportTile(10, 2, 3, 9, TRAP_MAPSPRITE_PAL_ENEMY);
AddLightRune(7, 6, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
```

```c
static const u8 TrapData_ThisEvent[] = {
    HEAL_TILE(2, 2, 0, TRAP_MAPSPRITE_PAL_PLAYER),
    TOGGLE_TORCH(5, 4, 3, false, TRAP_MAPSPRITE_PAL_GREY),
    TELEPORT_TILE(9, 1, 13, 8, TRAP_MAPSPRITE_PAL_ENEMY),
    TRAP_NONE
};
```

If you pass an invalid palette value, `SetTrapMapSpritePalette(...)` clamps it back to `TRAP_MAPSPRITE_PAL_DEFAULT`.

## Repeat House Notes

The repeat house trap is a chapter object that awards EXP the first time a unit visits it.

Behavior summary:

- Runtime API: `AddRepeatHouse(x, y, exp)`
- Chapter-start macro: `REPEAT_HOUSE(x, y, exp)`
- Trap type: `TRAP_REPEAT_HOUSE`
- EXP source: the constructor stores the requested EXP in `trap->extra`
- Default EXP: `CONFIG_REPEAT_HOUSE_DEFAULT_EXP` is used when the stored EXP is `0`
- Visit gate: the reward only fires when the active unit's action type is `UNIT_ACTION_VISIT`
- One-time reward: `TRAP_EXTDATA_REPEAT_HOUSE_VISITED` blocks later visits from awarding EXP again

What that means for the player:

- A house can hand out a small EXP reward instead of only a fixed event reward.
- The reward is consumed the first time the unit visits the tile.
- Later visits to the same house do nothing, so the reward cannot be farmed by reopening the same house.
- Chapter authors can tune the reward per house by changing the `exp` argument in the trap data or runtime constructor.

Implementation note:

- `AddRepeatHouse(...)` creates the trap and clears the visited flag because `AddTrap(...)` may leave `trap->data[]` uninitialized.
- `PostAction_RepeatHouse(...)` checks both the active unit and the visit action before it looks up the trap at the unit's position.
- The reward is applied through `AddExp_Event(exp)`, so the house behaves like an event-driven EXP grant rather than a combat result.
- A value of `0` for the `exp` parameter is treated as "use the default" rather than "award no EXP".

## Dragon Vein Notes

The dragon vein is a chapter or runtime trap that a unit with the Dragon's Blood skill activates from the map.

Behavior summary:

- Runtime API: `AddDragonVein(x, y, effect, param)`
- Chapter-start macro: `DRAGON_VEIN(x, y, effect, param)`
- Trap type: `TRAP_DRAGON_VEIN`
- Effect: the `DRAGON_VEIN_*` effect is stored in `trap->extra`
- Parameter: the effect parameter is stored in `trap->data[TRAP_EXTDATA_DRAGON_VEIN_PARAM]`
- Map sprite: drawn with map sprite `0x69`
- Activation: a unit with the Dragon's Blood skill standing on the vein uses the "Dragon Vein" command
- Lifetime: each vein is removed after one use

Effects:

- `DRAGON_VEIN_HEAL`: allies recover `param` HP (`0` = full heal)
- `DRAGON_VEIN_DAMAGE`: enemies lose `param` HP (`0` = 10), never below 1
- `DRAGON_VEIN_STATUS`: enemies get the status index `param`
- `DRAGON_VEIN_MAPCHANGE`: triggers the chapter's map change id `param`

Chapter-start data:

- The `DRAGON_VEIN(...)` macro writes the effect into the entry's `subtype` field and the parameter into `turn_counter`.
- `LoadTrapData(...)` passes those two fields to `AddDragonVein(...)`.

Implementation note:

- `AddDragonVein(...)` creates the trap with `AddTrap(...)` and stores the parameter in `TRAP_EXTDATA_DRAGON_VEIN_PARAM`.
- The activation command and one-use removal are the intended behaviour and are not wired in this change; this section describes the design.

## Grass Trap Notes

The grass trap is the current example of a trap that does more than draw a sprite.

Behavior summary:

- Runtime API: `AddGrassTile(x, y, turnsLeft)`
- Chapter-start macro: `GRASS_TILE(x, y, turnsLeft)`
- Sprite source: `Gfx_Grass_Tile`
- Palette source: `Pal_Grass_Tile`
- Gameplay terrain: the tile is forced to `TERRAIN_FOREST` when the trap is placed
- Lifetime: `turnsLeft` controls how many `DecayTraps()` passes remain before the trap removes itself; `0` means permanent

What that means for the player:

- The map sprite uses the custom grass graphics and custom grass palette.
- Standing on the tile gives standard forest terrain bonuses.
- Movement cost and passability follow forest terrain rules.
- The normal terrain UI should report the tile as forest, just like a standard forest tile.
- When a timed grass trap expires, the original terrain under it is restored automatically.

Implementation note:

- This grass trap currently achieves its terrain behavior by updating `gBmMapTerrain[y][x]` to `TERRAIN_FOREST` when placed.
- The original terrain is saved in `trap->extra` when the grass trap is created.
- If the trap expires, `DecayTraps()` restores that saved terrain before removing the grass trap.
- Replacing a grass trap on the same tile refreshes its lifetime instead of overwriting the saved original terrain.
- The trap itself remains responsible for the visual overlay, while the engine's normal terrain systems handle the bonuses and terrain display.

## Boulder Trap Notes

The boulder trap is the current example of a trap that behaves like a map object instead of a terrain override.

Behavior summary:

- Runtime API: `AddBoulderTile(x, y)`
- Chapter-start macro: `BOULDER_TILE(x, y)`
- Sprite source: `Gfx_Boulder_Tile`
- Palette source: the shared map sprite palette (OBJ bank `0xC`); the PNG is indexed against it, so no dedicated bank is needed
- Movement rule: non-fliers treat the tile as blocked in movement-map generation and generic position checks
- Interaction rule: the `Interact` command can push an adjacent boulder directly away from the acting unit
- Push range: `floor(GetUnitPower(unit) / 10)` tiles, capped by the first blocked destination in that direction

What that means for the player:

- Wyverns, pegasi, dracozombies, and other fliers ignore the boulder as a passability blocker.
- Grounded units cannot move onto the boulder tile.
- If a grounded unit has at least 10 strength, it can push an adjacent boulder one tile.
- Every additional 10 strength adds one more tile of push distance, as long as each intervening landing tile is valid.
- Boulder pushes fail if the next landing tile is occupied by a unit, already has a trap, or is a blocked terrain such as a shop, village, wall, chest, water, or map edge.

Implementation note:

- The boulder does not overwrite `gBmMapTerrain`; it stays a trap overlay.
- Movement blocking is applied through the extended movement barrier map in `PreGenerateMovementMap(...)`.
- Non-movement placement checks also reject boulder tiles through `Generic_CanUnitBeOnPos(...)`.
- Push behavior is implemented in the shared `Interact` command path, alongside toggle torches.

## Spin Tile Notes

The spin tile is the current example of a trap that resolves entirely through movement and post-action hooks.

Behavior summary:

- Runtime API: `AddSpinTile(x, y, direction)`
- Chapter-start macro: `SPIN_TILE(x, y, direction)`
- Direction constants: `SPIN_TILE_DIR_LEFT`, `SPIN_TILE_DIR_RIGHT`, `SPIN_TILE_DIR_UP`, `SPIN_TILE_DIR_DOWN`
- Sprite sources: `Gfx_Spin_Tile_LEFT`, `Gfx_Spin_Tile_RIGHT`, `Gfx_Spin_Tile_UP`, `Gfx_Spin_Tile_DOWN`
- Palette source: `Pal_Spin_Tile`
- Movement rule: grounded units stop as soon as their path reaches the tile, even if the selected destination was farther away
- Flier rule: fliers ignore the mid-path stop and only resolve the trap if they finish an action while standing on the tile
- Forced move rule: after resolution, the unit is pushed one tile at a time in the configured direction if each landing tile is valid and unobstructed
- Timing rule: connected spin-tile chains pause for 30 frames between hops so the unit visibly lands on each tile before continuing
- Blockers: the forced move fails on map edge, occupied tiles, invalid terrain, light runes, boulders, and other tiles the unit cannot legally occupy

What that means for the player:

- Grounded units cannot walk through a spin tile in one move; the trap catches them on contact.
- If the arrow points at an open tile, the unit is immediately displaced one space after the trap resolves.
- Connected spin tiles continue to resolve one tile at a time, so a straight run of tiles will carry the unit across the whole chain.
- If the next destination tile is blocked, the unit stays on the last valid spin tile and the shove stops there.
- Connected spin tiles keep their configured directions and graphics stable after activation.
- When a chain continues, the unit visibly appears on each intermediate tile for 30 frames before the next hop begins.
- Fliers can pass over the tile during movement, but ending an action on it still risks the forced shove.

Implementation note:

- The contact stop is injected in `UnitApplyWorkingMovementScript(...)` instead of the hidden-mine path so thieves and assassins do not inherit vanilla mine immunity.
- `PostAction_SpinTile(...)` handles the actual shove after the unit is standing on the trap.
- `GetPickTrapType(...)` returns `TRAP_NONE` for spin tiles so vanilla `ExecTrap(...)` is not asked to process a custom trap type.

## Trap Sprite Registration

### TrapData_Installer.event

File: `Kernel/Wizardry/TrapData/TrapData_Installer.event`

This file installs trap map sprite graphics into the map sprite table (`0x8AF880 + 0x8 * spriteID`) using entries like:

```ea
#define Gfx_Heal_Tile_ID 0x68

Gfx_Heal_Tile:
#incext Png2Dmp "images/Heal_Tile.png" --lz77

PUSH
    ORG (0x8AF880+0x8*Gfx_Heal_Tile_ID)
        SHORT 0x0002
        SHORT 0x0000
        POIN Gfx_Heal_Tile
POP
```

Current registered trap-like sprite IDs in this file:

- `0x68` Heal Tile
- `0x69` Dragon Vein Tile
- `0x6A` Lit Torch Tile
- `0x6B` Unlit Torch Tile
- `0x6C` Teleport Tile
- `0x6D` Grass Tile
- `0x6E` Boulder Tile
- `0x6F` Spin Tile

When adding a new trap sprite:

1. Reserve a new sprite ID.
2. Add compressed graphic label with `#incext Png2Dmp ... --lz77`.
3. Write table entry using the same `ORG`/`SHORT`/`POIN` pattern.
4. Use that sprite ID in rendering (`UseUnitSprite(newId)`).

## Trap Sprite Display in RefreshUnitSprites

### Render hook location

File: `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c`
Function: `RefreshUnitSprites(void)`

After unit sprite handling, this function iterates all traps:

```c
for (trap = GetTrap(0); trap->type != 0; trap++)
```

It already draws map sprites for several trap types, including:

- Ballista-like entries (`trap->type == 1` with subtype checks)
- `TRAP_LIGHT_RUNE`
- `TRAP_HEAL_TILE`
- `TRAP_TOGGLE_TORCH`
- `TRAP_TELEPORT_TILE`
- `TRAP_GRASS_TILE`
- `TRAP_BOULDER_TILE`
- `TRAP_SPIN_TILE`

### Pattern to display a new trap type

Add a block in that trap loop:

```c
if (trap->type == TRAP_YOUR_TYPE)
{
    smsHandle = AddUnitSprite(trap->yPos * 16);
    smsHandle->yDisplay = trap->yPos * 16;
    smsHandle->xDisplay = trap->xPos * 16;

    smsHandle->oam2Base = UseUnitSprite(YOUR_SPRITE_ID) - 0x5000 + 0x80;
    smsHandle->config = UNIT_ICON_SIZE_16x16;
}
```

Implementation notes:

- `UseUnitSprite(id)` must match the ID installed in `TrapData_Installer.event`.
- The subtraction term (`-0x5000` or `-0x4000`) depends on sprite source conventions already used in this file; match an existing trap pattern for consistency.
- `smsHandle->config` controls shape/size; use an existing trap's config as your baseline.
- Build the OAM2 value with `GetTrapSpriteOam2(spriteId, trap)`. It picks the OBJ palette bank through `GetTrapMapSpriteObjPal(...)`, which reads the stored palette via `GetTrapMapSpritePalette(...)`.
- The grass and spin traps are the current exceptions: they use dedicated OBJ palette banks (9 and 8) loaded from `Pal_Grass_Tile` and `Pal_Spin_Tile`. Those banks are shared with portraits and map battle MUs, so `ReloadCustomTrapSpritePalettes` only loads one while a trap of that type is on the map.

## Code Locations

| Feature | Location | Description |
| -------- | ---------- | ------------- |
| Trap type IDs and extdata enums | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Canonical trap constants (`TRAP_*`, `TRAP_EXTDATA_*`) and trap API declarations |
| Trap palette enum and helper macros | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Defines `TRAP_MAPSPRITE_PAL_*`, `HEAL_TILE`, `TOGGLE_TORCH`, and palette-aware `TELEPORT_TILE` helpers |
| Trap table struct | `Tools/FE-CLib-Mokha/include/bmtrap.h` | `struct TrapData` format used by chapter trap arrays |
| Chapter trap loader | `LoadTrapData` in `Kernel/Wizardry/TrapData/TrapData.c` | Dispatches trap entries from chapter data to constructors |
| Trap palette storage and sanitizing | `GetTrapMapSpritePalette` and `SetTrapMapSpritePalette` in `Kernel/Wizardry/TrapData/TrapData.c` | Stores per-trap palette choices and falls back to default when invalid |
| Repeat house trap creation | `AddRepeatHouse` in `Kernel/Wizardry/TrapData/TrapData.c` | Creates a repeat house trap with a stored EXP reward and resets the visited flag |
| Repeat house post-action hook | `PostAction_RepeatHouse` in `Kernel/Wizardry/TrapData/TrapData.c` | Awards EXP on the first `UNIT_ACTION_VISIT` and marks the trap as visited |
| Effective terrain override | `GetEffectiveTerrainAt`, `AddGrassTile`, and `DecayTraps` in `Kernel/Wizardry/TrapData/TrapData.c` | Makes special trap tiles count as a different gameplay terrain, stamps forest terrain for grass tiles, and restores the original terrain when timed grass traps expire |
| Boulder trap creation | `AddBoulderTile` in `Kernel/Wizardry/TrapData/TrapData.c` | Creates startup or runtime boulder traps without changing underlying terrain |
| Spin trap creation and shove logic | `AddSpinTile` and `PostAction_SpinTile` in `Kernel/Wizardry/TrapData/TrapData.c` | Stores directional spin tile data and resolves connected spin-tile chains one hop at a time with 30-frame pauses until the next hop would be blocked or repeat |
| Spin trap sprite selection | `RefreshUnitSprites` in `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c` | Chooses the left/right/up/down spin tile sprite ID from the trap's current direction |
| Trap graphics installer | `Kernel/Wizardry/TrapData/TrapData_Installer.event` | Registers trap map sprite sheets and table entries |
| Chapter trap declaration | `CustomCampaign/Chapters/01/events/traps.c` | Defines startup trap arrays for normal/hard modes |
| Chapter trap binding | `CustomCampaign/Chapters/00/events/events.c` | Connects trap arrays via `.traps` and `.extraTrapsInHard` |
| Event ASMC call site | `CustomCampaign/Chapters/00/events/events.c` | Invokes ASMC function during chapter event flow |
| ASMC runtime trap spawn | `CustomCampaign/Chapters/00/events/asmc.h` | Example runtime trap creation with `AddTeleportTilePair(...)` |
| Teleport trap helpers | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Defines `TELEPORT_TILE`, `TELEPORT_TILE_PAIR`, and teleport trap extdata |
| Teleport trap runtime behavior | `Kernel/Wizardry/TrapData/TrapData.c` | Loads, constructs, and resolves teleport tile effects |
| Grass trap runtime behavior | `Kernel/Wizardry/TrapData/TrapData.c` | Creates the grass trap and stamps the map terrain to forest for gameplay and terrain UI |
| Boulder trap movement blocking | `PreGenerateMovementMap` in `Kernel/Wizardry/Movement/Source/Movement.c` | Marks boulder coordinates as barriers for non-fliers during movement-map generation |
| Boulder trap placement rejection | `Generic_CanUnitBeOnPos` in `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c` | Prevents generic reposition and forced-placement helpers from placing non-fliers onto boulders |
| Spin tile movement stop | `UnitApplyWorkingMovementScript` in `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c` | Halts grounded movement the moment a path reaches a spin tile while letting fliers pass over it |
| Boulder trap interaction | `InteractCommandUsability` and `InteractCommandEffect` in `Kernel/Wizardry/SkillEffects/MenuSkills/Pick.c` | Detects adjacent boulders, computes push distance from strength, and moves the trap if the path is clear |
| Trap map sprite palette bank | `GetTrapMapSpriteObjPal` in `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c` | Maps a trap's type and stored palette choice onto an OBJ palette bank (also used by the debugger's trap preview) |
| Grass/spin trap custom palettes | `LoadTrapMapSpritePalette` and `ReloadCustomTrapSpritePalettes` in `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c` | Loads `Pal_Grass_Tile`/`Pal_Spin_Tile` into their dedicated OBJ banks while such a trap is on the map |
| Trap map sprite draw loop | `RefreshUnitSprites` in `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c` | Adds trap sprites to SMS/OAM handle list and applies trap palette selection |
| Light rune menu skill usage | `Kernel/Wizardry/SkillEffects/MenuSkills/LightRune.c` | Example runtime light-rune placement using an explicit palette |
| Example chapter ASMC usage | `CustomCampaign/Chapters/00/events/asmc.h` | Example runtime placement using `AddGrassTile(3, 3, 3)` |

## TODO

- Document removal/update workflows for runtime traps (remove, replace, refresh behavior).
- Add a short reference table that maps each palette constant to a screenshot or gif once assets exist.
- Document a shared removal helper if more terrain-override trap types are added in the future.
- Add a short example showing how to place a repeat house in a chapter trap header and tune its EXP reward.
- Decide whether `Interact` should eventually expose a chooser when a unit is adjacent to multiple interactable trap/object types.
- Add direction-specific SMS artwork if contributors want the tile overlay itself to visually distinguish left, right, up, and down on the map.

## Limitations & Bugs

- `AddTeleportTilePair(...)` and `TELEPORT_TILE_PAIR(...)` do not take a palette parameter; both use `TRAP_MAPSPRITE_PAL_DEFAULT`.
- Grass and spin traps use OBJ banks 9 and 8, which are also used by the third and fourth portrait slots and by special map battle MUs (Manakete). While one of those traps is on the map, a map battle with a special MU can show its palette wrong.
- `REPEAT_HOUSE(x, y, exp)` stores the EXP reward in the trap's `extra` field, so a value of `0` intentionally means "use the default EXP" rather than "award nothing".
- Repeat house rewards only trigger from the visit action, so scripts or alternate unit actions that end on the tile will not award EXP unless they also set `UNIT_ACTION_VISIT`.
- Grass trap lifetime is stored in a signed extdata byte, so keep `turnsLeft` within normal trap-duration ranges.
- Boulder pushes currently block on units, traps, map bounds, and a fixed list of terrain-coded obstacles/buildings; they do not inspect arbitrary custom tile events that live on otherwise normal terrain.
- Spin tile direction remains fixed to the configured trap data; activation does not mutate the tile's stored direction.