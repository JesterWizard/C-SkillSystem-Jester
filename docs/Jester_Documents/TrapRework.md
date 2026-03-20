# Trap Authoring Guide

---

## Index

1. [Introduction](#introduction)
2. [Implementation Plan](#implementation-plan)
3. [Trap IDs and Data Model](#trap-ids-and-data-model)
4. [Chapter-Start Trap Placement](#chapter-start-trap-placement)
5. [Runtime Trap Placement with ASMC](#runtime-trap-placement-with-asmc)
6. [Applying Existing Traps](#applying-existing-traps)
7. [Trap Sprite Registration](#trap-sprite-registration)
8. [Trap Sprite Display in RefreshUnitSprites](#trap-sprite-display-in-refreshunitsprites)
9. [Code Locations](#code-locations)
10. [TODO](#todo)
11. [Limitations & Bugs](#limitations--bugs)

## Introduction

This document explains how to add new traps in this project from a contributor workflow perspective:

- Define and understand trap IDs and extdata fields.
- Place traps at chapter load via chapter trap headers.
- Spawn traps at runtime via event ASMC calls.
- Register map sprite graphics for new trap visuals.
- Render new trap types in the map sprite refresh loop.

Player-facing result: your new trap can exist at map start, be created by events mid-chapter, and show a map icon correctly.

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
- Sprite uses expected tile/palette and lifetime behavior.

## Trap IDs and Data Model

### Trap enum in bmtrick.h

File: `Tools/FE-CLib-Mokha/include/bmtrick.h`

The trap enum is the canonical ID list for trap types, including project additions like `TRAP_HEAL_TILE`.

- Use these IDs everywhere (event data, C logic, rendering).
- Avoid hardcoded numeric trap IDs in game logic.
- Keep extdata semantics aligned with `TRAP_EXTDATA_*` constants.

Important extdata examples from this file:

- `TRAP_EXTDATA_TRAP_TURNFIRST`, `TRAP_EXTDATA_TRAP_TURNNEXT`, `TRAP_EXTDATA_TRAP_COUNTER`, `TRAP_EXTDATA_TRAP_DAMAGE`
- `TRAP_EXTDATA_RUNE_TURNSLEFT`
- `TRAP_EXTDATA_HEALTILE_TURNSLEFT`
- `TRAP_EXTDATA_TOGGLE_TORCH_DURATION`
- `TRAP_EXTDATA_TELEPORT_DEST_X`, `TRAP_EXTDATA_TELEPORT_DEST_Y`

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

Example pattern:

```c
static const u8 TrapData_ThisEvent[] = {
    /* type */ TRAP_FIRETILE,
    /* xPos */ 3,
    /* yPos */ 1,
    /* subt */ 0,
    /* cnt  */ 1,
    /* turn */ 1,
};

static const u8 TrapData_ThisEventHard[] = {
    TRAP_NONE
};
```

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
AddHealTile(3, 3, 10, 2);
```

This demonstrates runtime trap spawning through C code called by event ASMC.

Recommended pattern for custom runtime trap setup:

1. Define a C function in the chapter ASMC source (or shared event utility C file).
2. Inside it, call one of:
- `AddTrap(x, y, trapType, meta)`
- `AddHealTile(x, y, healAmount, turnsLeft)`
- `AddToggleTorch(x, y, duration, startsLit)`
- `AddTeleportTile(x, y, destX, destY)`
- `AddTeleportTilePair(x1, y1, x2, y2)`
- other trap-specific constructors
3. Call that function from event script via `ASMC(YourFunctionName)`.

## Applying Existing Traps

This section covers the contributor-facing workflow for traps that already exist in the rework.

### Runtime usage from ASMC

For event-driven trap placement, call the trap constructor directly inside your ASMC C function.

| Trap | Runtime call | What the parameters mean |
|--------|----------|-------------|
| Heal tile | `AddHealTile(x, y, healAmount, turnsLeft)` | Place a healing tile at `(x, y)` with a heal amount and optional lifetime |
| Toggle torch | `AddToggleTorch(x, y, duration, startsLit)` | Place a torch tile at `(x, y)` with a lit duration and initial lit state |
| Teleport tile pair | `AddTeleportTilePair(x1, y1, x2, y2)` | Create two linked teleport tiles, one at each coordinate pair |
| One-way teleport tile | `AddTeleportTile(x, y, destX, destY)` | Create a single teleport tile that sends units from `(x, y)` to `(destX, destY)` |

Runtime example from chapter ASMC code:

```c
void SetGameOptions()
{
    AddTeleportTilePair(1, 1, 3, 3);
    AddTeleportTilePair(5, 5, 9, 5);
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

### Chapter-start usage in TrapData arrays

For chapter-start trap placement, use the trap table helpers from `bmtrick.h` instead of manually counting bytes for teleport entries.

Example:

```c
static const u8 TrapData_ThisEvent[] = {
    TELEPORT_TILE_PAIR(1, 1, 3, 3),
    TELEPORT_TILE_PAIR(5, 5, 9, 5),
    TRAP_NONE
};
```

Use `TELEPORT_TILE(x, y, destX, destY)` if you want one-way behavior in the chapter trap table.

Practical rule:

- Use `AddTeleportTilePair(...)` when spawning paired warp tiles from ASMC code.
- Use `TELEPORT_TILE_PAIR(...)` when placing paired warp tiles in chapter startup trap arrays.
- Use the single-tile variants only when you intentionally want one-way teleport behavior.

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

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Trap type IDs and extdata enums | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Canonical trap constants (`TRAP_*`, `TRAP_EXTDATA_*`) and trap API declarations |
| Trap table struct | `Tools/FE-CLib-Mokha/include/bmtrap.h` | `struct TrapData` format used by chapter trap arrays |
| Chapter trap loader | `LoadTrapData` in `Kernel/Wizardry/Common/TrapData/TrapData.c` | Dispatches trap entries from chapter data to constructors |
| Trap graphics installer | `Kernel/Wizardry/Common/TrapData/TrapData_Installer.event` | Registers trap map sprite sheets and table entries |
| Chapter trap declaration | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Traps.h` | Defines startup trap arrays for normal/hard modes |
| Chapter trap binding | `Data/FE8_Rewritten_Terper/Event/Source/00/00.c` | Connects trap arrays via `.traps` and `.extraTrapsInHard` |
| Event ASMC call site | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/Events.h` | Invokes ASMC function during chapter event flow |
| ASMC runtime trap spawn | `Data/FE8_Rewritten_Terper/Event/Source/00/Source/ASMCs.h` | Example runtime trap creation with `AddTeleportTilePair(...)` |
| Teleport trap helpers | `Tools/FE-CLib-Mokha/include/bmtrick.h` | Defines `TELEPORT_TILE`, `TELEPORT_TILE_PAIR`, and teleport trap extdata |
| Teleport trap runtime behavior | `Kernel/Wizardry/Common/TrapData/TrapData.c` | Loads, constructs, and resolves teleport tile effects |
| Trap map sprite draw loop | `RefreshUnitSprites` in `Kernel/Wizardry/Misc/MirrorMapSprites/MirrorSprites.c` | Adds trap sprites to SMS/OAM handle list |

## TODO

- Document removal/update workflows for runtime traps (remove, replace, refresh behavior).

## Limitations & Bugs