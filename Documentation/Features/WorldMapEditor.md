# EA-Friendly World Map Authoring

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

World-map intros in this build are written as C `EventScr` arrays, not as FEBuilder event text. That keeps them version-controlled and compatible with the same lyn/EA pipeline as chapter events, but the raw opcode sequences repeat a lot of boilerplate: spawn lord, fade, reveal the next node, draw a road, voice `SOUN` beats, nation highlights, and portrait timing.

This feature adds a **macro layer** in `CustomCampaign/Chapters/_shared/worldmap-macros.h`. The macros expand to the same Event Assembler opcodes the vanilla engine already runs. There is no GUI and no runtime interpreter.

**Dynamic pathing** generates roads at runtime from the lord's current node coordinates to the next story node's coordinates. Scripts no longer hard-code a `WM_PATH_*` ID for standard SET_NODE reveals. The engine builds a Magvel-style 2-tile-wide ribbon into the vanilla MapRoute layer: straight H/V when nodes share an axis, a 45° run when the slope is steep, and a two-level 45° ramp when the offset is shallow. Cursor travel stays on the same open-path graph.

Node **geometry** (coordinates, chapter IDs) still lives in vanilla ROM tables (`gWMNodeData`) for now. `GetWmNodeX` / `GetWmNodeY` wrap those coordinates so a future node-generation pass can swap in RAM positions without rewriting path code.

---

## Plan

World-map choreography splits into two script types that the engine dispatches by `ROMChapterData.gmapEventId`:

| Script type | When it runs | Typical job |
|-------------|--------------|-------------|
| **SET_NODE** | After clearing a chapter, before the player regains control | Spawn the lord, fade the map, reveal/load the next destination node, draw the connecting path |
| **TRAVEL_TO_NODE** | When the player selects the next chapter on the world map | Play the travel cutscene: camera moves, narration, sprite motion, fade to chapter |

Dispatch lives in [`CustomCampaign/Event/World_Map_Events.c`](../../CustomCampaign/Event/World_Map_Events.c):

- `WorldMap_CallBeginningEvent` indexes `EventScrWM_SET_NODE[gmapEventId]`.
- `CallChapterWMIntroEvents` indexes `EventScrWM_TRAVEL_TO_NODE[gmapEventId]`.

The `gmapEventId` in each chapter's `chapter.c` must match the slot used in those tables. Examples:

| Chapter | `gmapEventId` | SET_NODE script | TRAVEL script |
|---------|---------------|-----------------|---------------|
| Prologue | `0x1` | `EventScrWM_Prologue_SET_NODE` | `EventScrWM_Prologue_TRAVEL_TO_NODE` |
| Ch. 4 | `0x5` | `EventScrWM_Ch4_SET_NODE` | `EventScrWM_Ch4_TRAVEL_TO_NODE` |
| Ch. 1 ending | `55` (special) | `EventScrWM_Ch1_ENDING` | — |

### Dynamic path flow

```text
WM_REVEAL_DEST(node)
  -> WmDrawPathFromCurrentToDest
       -> sub_80BCDE4(lordNode, nextNode)   // vanilla ROM path slot lookup
       -> AddAndDrawGmPath(pathId)         // vanilla fade + open-path bit
MapRoute composite (sub_80BBC54 hook)
  -> Occupancy-aware Magvel ribbon (straight / 45° / two-level ramp) into gUnknown_02019D00
WM_MOVESPRITETO (travel scripts)
  -> sub_80BCE34 hook walks the same generated polyline (corners, not vanilla movementPath)
```

| Path source | When used | Visual |
|-------------|-----------|--------|
| `WM_REVEAL_DEST(node)` | Standard SET_NODE template (ch. 2–7) | Generated Magvel ribbon between live node coords |
| `WM_DRAWPATH(id)` / `WM_DRAWPATH2(id)` | Ch. 1 ending, ch. 8–10, intermission specials | Same generator (open paths are all composited from node placement) |

### Macro cookbook

Include [`worldmap-include.h`](../../CustomCampaign/Chapters/_shared/worldmap-include.h) from every `worldmap.c`. That pulls in the macro header, script externs, and the shared campaign headers.

**Opcode aliases** (Mokha-only commands not in EAstdlib):

```c
WM_NOFADE              // skip world-map fade setup
WM_SETNODESTATENOT2    // node state helper used on Border Mulan
WM_SETUNITONNODE       // place a world-map unit on a node silently
```

**Portrait sides:**

```c
WM_FACE_LEFT / WM_FACE_RIGHT
WM_FACE_CLEAR_LEFT / WM_FACE_CLEAR_RIGHT
WM_FACE_SLIDE_LEFT / WM_FACE_SLIDE_RIGHT
```

**Voice narration during `WM_TEXT`:**

```c
WM_VOICE_LINE(SONG_VOICE_CH04_LINE_0001)
// expands to SOUN + TEXTCONT + TEXTEND
```

**Standard SET_NODE template** (used by chapters 2–7):

```c
const EventScr EventScrWM_Ch4_SET_NODE[] = {
    WM_OPEN_MAP(CHARACTER_EIRIKA, WM_NODE_BorgoRidge)
    WM_REVEAL_DEST(WM_NODE_ZahaWoods)
    WM_CLOSE_SET_NODE()
};
```

Chapters with extra setup (music, tutorials) insert commands between `WM_REVEAL_DEST` and `WM_CLOSE_SET_NODE`.

Use `WM_DRAWPATH(WM_PATH_xx)` only when a chapter must open a specific vanilla path slot (prologue-adjacent beats, ch. 8 branch paths, ch. 10 Port Kiris link, etc.). The visible road still comes from node placement, not from that slot's ROM gfx.

**Prologue-style nation narration:**

```c
WM_NATION_ON(0x0051, WM_FACE_LEFT, WM_NATION_Renais)
WM_VOICE_LINE(SONG_VOICE_CH00_PROLOGUE_LINE_0005)
STAL(30)
WM_NATION_OFF(WM_NATION_Renais, WM_FACE_CLEAR_LEFT)
```

Use `WM_NATION_OFF_CORE` when the original script omitted the trailing `STAL(32)` after clearing a portrait.

**Travel cutscene helpers:**

```c
WM_SHOW_FACE_WAIT(0, 0x0002, WM_FACE_RIGHT, 46)
WM_HIDE_FACE(0, WM_FACE_CLEAR_RIGHT)
```

Keep sprite `WM_PUTMOVINGSPRITE` coordinates as raw numbers; those are map-pixel paths, not shared boilerplate.

### Chapters already converted

| Area | Chapters | Macros used |
|------|----------|-------------|
| SET_NODE template | 2, 3, 4, 5 (normal branch), 6, 7 | `WM_OPEN_MAP`, `WM_REVEAL_DEST`, `WM_CLOSE_SET_NODE` |
| SET_NODE vanilla path slot | 1, 8, 9, 10, Intermission | `WM_DRAWPATH` / `WM_DRAWPATH2` kept to open those path IDs |
| Prologue SET_NODE | 0 | `WM_NOFADE`, `WM_VOICE_LINE`, `WM_NATION_ON/OFF`, `WM_SHOW_FACE_WAIT` |
| TRAVEL voice/face | 1–7, 9 | `WM_VOICE_LINE`, `WM_SHOW_FACE_WAIT`, `WM_HIDE_FACE` |
| Unique SET_NODE aliases | 1, 5 (Renvall), 8, Intermission | `WM_SETNODESTATENOT2`, `WM_SETUNITONNODE`, `WM_SETCAMTONODE` |

Chapter 8 TRAVEL and chapter 5x are flag/fade-only and have no voice or face beats. Sprite `WM_PUTMOVINGSPRITE` coordinates stay as raw numbers.

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Macro header | [`worldmap-macros.h`](../../CustomCampaign/Chapters/_shared/worldmap-macros.h) | Aliases, SET_NODE template, dynamic `WM_REVEAL_DEST`, voice/face/nation helpers |
| Dynamic path core | [`WmDynamicPath.c`](../../Kernel/Wizardry/WorldMap/WmDynamicPath.c) | Magvel 2-wide ribbon from live node coords; no extra RAM reservations |
| Lyn hooks | [`LynJump.event`](../../Kernel/Wizardry/WorldMap/LynJump.event) | `sub_80BBC54` (composite), `sub_80BCE34` (travel spline) |
| Shared include | [`worldmap-include.h`](../../CustomCampaign/Chapters/_shared/worldmap-include.h) | Pulls headers, script externs, and macros into every `worldmap.c` |
| Script extern table | [`worldmap-scripts.h`](../../CustomCampaign/Chapters/_shared/worldmap-scripts.h) | Declares each chapter's `EventScrWM_*` symbols plus `WmDrawPathFromCurrentToDest` |
| Dispatch tables | [`World_Map_Events.c`](../../CustomCampaign/Event/World_Map_Events.c) | `EventScrWM_SET_NODE[]` and `EventScrWM_TRAVEL_TO_NODE[]` indexed by `gmapEventId` |
| Short SET_NODE example | [`Chapters/04/events/worldmap.c`](../../CustomCampaign/Chapters/04/events/worldmap.c) | Dynamic path reveal plus TRAVEL voice lines |
| EA opcode aliases | [`EAstdlib.h`](../../Tools/FE-CLib-Mokha/include/EAstdlib.h) | Standard `WM_*` names mapped to `WmEvt*` |
| Node/path structs | [`worldmap.h`](../../Tools/FE-CLib-Mokha/include/worldmap.h) | `GMapNodeData`, `GMapPathData`, decomp route helpers |
| Node constants | [`constants/worldmap.h`](../../Tools/FE-CLib-Mokha/include/constants/worldmap.h) | `WM_NODE_*`, `WM_PATH_*`, `WM_NATION_*`, `WM_MU_*` |

---

## TODO

- Dump vanilla `gWMNodeData` into editable C under `CustomCampaign/` and lyn-repoint the table.
- Generate or relocate nodes at runtime; wire coordinates through `GetWmNodeX` / `GetWmNodeY`.
- Convert remaining TRAVEL scripts to `WM_VOICE_LINE` where the pattern is a straight voice beat with no sprite timing between lines.
- Document the ch1 `gmapEventId == 55` special case in `World_Map_Events.c` with a named constant.
- Migrate ch. 1 / 8 / 9 / 10 SET_NODE scripts to `WM_REVEAL_DEST` where a hard-coded path slot is no longer required.
- Optional: extra perpendicular jogs on long links to mimic Magvel's hand-wound Path 0 / Path 15 wiggles.

---

## Limitations & Bugs

- Vanilla FE8 caps the world map at **29 nodes** (`NODE_MAX` 0x1D) and **32 paths** (`WM_PATH_MAX` 0x20). Expanding either requires save-RAM and engine work beyond this layer.
- Roads are Magvel 2-wide H/V ribbons plus a 3-wide 45° band. Node pixels use `p >> 3` (same as vanilla gfx). H-runs skip the start and dest columns so they do not overshoot 8px east; V-runs paint dest so south joins meet the node. Occupancy uses a 1-tile halo at endpoints. Lord travel follows the same corners.
- Save data still stores only the 32-bit open-path mask. Dynamic geometry is regenerated from node coordinates on load, which is fine while nodes stay in `gWMNodeData`.
- All open paths are composited from node placement, including those opened with `WM_DRAWPATH`.
- Macros are C-only. They do not run inside raw `.event` files assembled without the chapter C build.
- `WM_NATION_OFF` always appends `STAL(32)`. Use `WM_NATION_OFF_CORE` when the original script skipped that wait.
- Report desyncs with the chapter ID, `gmapEventId`, and whether `worldmap.lyn.event` was rebuilt after editing `worldmap.c`.
