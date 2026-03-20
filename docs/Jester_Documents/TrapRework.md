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
8. [Trap Sprite Registration](#trap-sprite-registration)
9. [Trap Sprite Display in RefreshUnitSprites](#trap-sprite-display-in-refreshunitsprites)
10. [Code Locations](#code-locations)
11. [TODO](#todo)
12. [Limitations & Bugs](#limitations--bugs)

## Introduction

This document explains how to add new traps in this project from a contributor workflow perspective:

- Define and understand trap IDs and extdata fields.
- Place traps at chapter load via chapter trap headers.
- Spawn traps at runtime via event ASMC calls.
- Select which trap map sprite palette is used.
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

Palette handling summary:

- `TRAP_LIGHT_RUNE` stores its palette in `trap->extra`.
- `TRAP_HEAL_TILE`, `TRAP_TOGGLE_TORCH`, and `TRAP_TELEPORT_TILE` store their palette in trap extdata.
- `SetTrapMapSpritePalette(...)` sanitizes out-of-range values and falls back to `TRAP_MAPSPRITE_PAL_DEFAULT`.

### LoadTrapData in TrapData.c

File: `Kernel/Wizardry/Common/TrapData/TrapData.c`

`LoadTrapData(const struct TrapData * data)` iterates the chapter trap table and dispatches by trap type. For each entry, it calls the corresponding constructor, for example:

- `TRAP_FIRETILE` -> `AddFireTile(...)`
- `TRAP_GAS` -> `AddGasTrap(...)`
- `TRAP_MINE` -> `AddTrap(...)`
- `TRAP_LIGHTARROW` -> `AddArrowTrap(...)`
- `TRAP_GORGON_EGG` -> `AddGorgonEggTrap(...)`
- `TRAP_HEAL_TILE` -> `AddHealTile(...)`
- `TRAP_TOGGLE_TORCH` -> `AddToggleTorch(...)`
- `TRAP_TELEPORT_TILE` -> `AddTeleportTile(...)`

When adding a new trap type that needs special initialization, add a `case` here.

Related trap table struct:

- File: `Tools/FE-CLib-Mokha/include/bmtrap.h`
- Struct: `struct TrapData { type, xPos, yPos, subtype, turn_counter, turn }`

## Chapter-Start Trap Placement

### Chapter trap headers

Example file:

- `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Traps.h`

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

Example pattern:

```c
static const u8 TrapData_ThisEvent[] = {
    HEAL_TILE(3, 1, 2, TRAP_MAPSPRITE_PAL_PLAYER),
    TOGGLE_TORCH(6, 4, 3, true, TRAP_MAPSPRITE_PAL_NPC),
    TELEPORT_TILE(8, 2, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY),
    TRAP_NONE
};

static const u8 TrapData_ThisEventHard[] = {
    TRAP_NONE
};
```

Prefer the helper macros from `bmtrick.h` for palette-aware trap entries instead of writing the bytes manually.

### Binding trap arrays to chapter startup

Example file:

- `Data/FE8_Rewritten_Terper/Event/Source/00/00.c`

The chapter event group binds these arrays:

```c
.traps = TrapData_ThisEvent,
.extraTrapsInHard = TrapData_ThisEventHard,
```

At chapter load, these are consumed by the trap loader pipeline that reaches `LoadTrapData`.

## Runtime Trap Placement with ASMC

### Event-side call site

Example file:

- `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Events.h`

An ASMC is invoked from event script with:

```c
ASMC(SetGameOptions)
```

### ASMC function that spawns a trap

Example file:

- `Data/FE8_Rewritten_Terper/Event/Source/00/Source/ASMCs.h`

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

Runtime example from chapter ASMC code:

```c
void SetGameOptions()
{
    AddHealTile(2, 4, 10, 0, TRAP_MAPSPRITE_PAL_PLAYER);
    AddToggleTorch(6, 6, 3, true, TRAP_MAPSPRITE_PAL_NPC);
    AddTeleportTile(8, 3, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY);
    AddLightRune(10, 5, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
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

`AddTeleportTilePair(...)` currently applies the light rune palette to both tiles internally. Use `AddTeleportTile(...)` directly if you want one-way teleport behavior with an explicit palette choice.

### Chapter-start usage in TrapData arrays

For chapter-start trap placement, use the trap table helpers from `bmtrick.h` instead of manually counting bytes for teleport entries.

Example:

```c
static const u8 TrapData_ThisEvent[] = {
    HEAL_TILE(3, 3, 2, TRAP_MAPSPRITE_PAL_PLAYER),
    TOGGLE_TORCH(6, 5, 3, true, TRAP_MAPSPRITE_PAL_NPC),
    TELEPORT_TILE(8, 2, 12, 7, TRAP_MAPSPRITE_PAL_ENEMY),
    TELEPORT_TILE_PAIR(1, 1, 3, 3),
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
| `TRAP_MAPSPRITE_PAL_DEFAULT` | Use the sprite's original/default palette |
| `TRAP_MAPSPRITE_PAL_LIGHT_RUNE` | Use the light-rune palette bank |
| `TRAP_MAPSPRITE_PAL_PLAYER` | Use the player/blue palette bank |
| `TRAP_MAPSPRITE_PAL_ENEMY` | Use the enemy/red palette bank |
| `TRAP_MAPSPRITE_PAL_NPC` | Use the NPC/green palette bank |
| `TRAP_MAPSPRITE_PAL_GREY` | Use the grey/fourth palette bank |

Where to set the palette:

- Runtime constructors: pass the palette as the final argument to `AddHealTile`, `AddToggleTorch`, `AddTeleportTile`, and `AddLightRune`.
- Chapter-start trap tables: use the `HEAL_TILE`, `TOGGLE_TORCH`, and `TELEPORT_TILE` helper macros with the palette constant as their final argument.
- Paired teleport helper macros: `TELEPORT_TILE_PAIR` still defaults both entries to `TRAP_MAPSPRITE_PAL_DEFAULT`.

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

## Trap Sprite Registration

### TrapData_Installer.event

File: `Kernel/Wizardry/Common/TrapData/TrapData_Installer.event`

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

When adding a new trap sprite:

1. Reserve a new sprite ID.
2. Add compressed graphic label with `#incext Png2Dmp ... --lz77`.
3. Write table entry using the same `ORG`/`SHORT`/`POIN` pattern.
4. Use that sprite ID in rendering (`UseUnitSprite(newId)`).

## Trap Sprite Display in RefreshUnitSprites

### Render hook location

File: `Kernel/Wizardry/Misc/MirrorMapSprites/MirrorSprites.c`
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
- Palette remapping is applied through `ApplyTrapSpritePalette(...)`, which reads the stored palette via `GetTrapMapSpritePalette(...)`.

## Code Locations

| Feature | Location | Description |
| -------- | ---------- | ------------- |
| Trap type IDs and extdata enums | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Canonical trap constants (`TRAP_*`, `TRAP_EXTDATA_*`) and trap API declarations |
| Trap palette enum and helper macros | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Defines `TRAP_MAPSPRITE_PAL_*`, `HEAL_TILE`, `TOGGLE_TORCH`, and palette-aware `TELEPORT_TILE` helpers |
| Trap table struct | `Tools/FE-CLib-Mokha/include/bmtrap.h` | `struct TrapData` format used by chapter trap arrays |
| Chapter trap loader | `LoadTrapData` in `Kernel/Wizardry/Common/TrapData/TrapData.c` | Dispatches trap entries from chapter data to constructors |
| Trap palette storage and sanitizing | `GetTrapMapSpritePalette` and `SetTrapMapSpritePalette` in `Kernel/Wizardry/Common/TrapData/TrapData.c` | Stores per-trap palette choices and falls back to default when invalid |
| Trap graphics installer | `Kernel/Wizardry/Common/TrapData/TrapData_Installer.event` | Registers trap map sprite sheets and table entries |
| Chapter trap declaration | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Traps.h` | Defines startup trap arrays for normal/hard modes |
| Chapter trap binding | `Data/FE8_Rewritten_Terper/Event/Source/00/00.c` | Connects trap arrays via `.traps` and `.extraTrapsInHard` |
| Event ASMC call site | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Events.h` | Invokes ASMC function during chapter event flow |
| ASMC runtime trap spawn | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/ASMCs.h` | Example runtime trap creation with `AddTeleportTilePair(...)` |
| Teleport trap helpers | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Defines `TELEPORT_TILE`, `TELEPORT_TILE_PAIR`, and teleport trap extdata |
| Teleport trap runtime behavior | `Kernel/Wizardry/Common/TrapData/TrapData.c` | Loads, constructs, and resolves teleport tile effects |
| Trap map sprite palette remap | `ApplyTrapSpritePalette` in `Kernel/Wizardry/Misc/MirrorMapSprites/MirrorSprites.c` | Maps stored trap palette choices onto SMS OAM palette bits |
| Trap map sprite draw loop | `RefreshUnitSprites` in `Kernel/Wizardry/Misc/MirrorMapSprites/MirrorSprites.c` | Adds trap sprites to SMS/OAM handle list and applies trap palette selection |
| Light rune menu skill usage | `Kernel/Wizardry/Misc/SkillEffects/MenuSkills/LightRune.c` | Example runtime light-rune placement using an explicit palette |

## TODO

- Document removal/update workflows for runtime traps (remove, replace, refresh behavior).
- Add a short reference table that maps each palette constant to a screenshot or gif once assets exist.

## Limitations & Bugs

- `AddTeleportTilePair(...)` does not currently take a palette parameter; it uses the light rune palette internally.
- `TELEPORT_TILE_PAIR(...)` still defaults to `TRAP_MAPSPRITE_PAL_DEFAULT`, so pair helpers and runtime pair creation do not currently share the same palette default.