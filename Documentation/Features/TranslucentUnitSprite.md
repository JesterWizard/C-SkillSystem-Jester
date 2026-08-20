# Pathfinding Ghost Preview

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla FE8 only shows the move arrow while pathfinding. Lex Talionis-style UIs also preview **where the unit will stand** with a faded copy of its moving-map sprite at the cursor.

This kernel option adds that ghost while the move path is active, then clears it when pathfinding ends (A confirm or B cancel).

---

## 🛠️ Plan

| Flag | Default | Effect |
|------|---------|--------|
| `translucent_unit_sprite` | `true` | While drawing a move path, show a blended MU ghost at the cursor tip |
| `remove_move_path` | `false` | When `true`, path arrows (and thus this ghost hook) are skipped |

### Player-facing rules

| Situation | Result |
|-----------|--------|
| Cursor on path tip, path length ≥ 1 | Ghost drawn; live MU faces the last path step |
| Cursor back on the origin tile | No ghost; MU returns to the selected bounce facing |
| Cursor off the path tip | No ghost |
| A / B ends pathfinding | Ghost gone with the path UI |

### Blend model

The ghost must coexist with translucent **range squares** (BG2) without making every map sprite flicker:

| Target | Layers | Why |
|--------|--------|-----|
| **A** | BG2 | Range squares blend over the map |
| **B** | BG2 + BG3 (not OBJ) | Ghost OBJ mode-1 finds BG2 under it and fades against the blue/red square; omitting OBJ from B avoids a global alpha pass on normal sprites |

Alpha weights are `8/8`. The ghost OBJ list is copied into EWRAM (`sPathfindingGhostObjBuf`) before `PutSpriteExt`, because OAM flush is deferred and a stack buffer would dangle.

---

## 🗂️ Code Locations

Gated by `gpKernelDesignerConfig->translucent_unit_sprite` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). Requires `remove_move_path == false` so `DrawUpdatedPathArrow` still runs.

| Feature | Location | Description |
|--------|----------|-------------|
| Config field | [`kernel-lib.h`](../../include/kernel/kernel-lib.h), [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | `translucent_unit_sprite` toggle |
| Ghost draw + blend | [`RemoveMovePath.c`](../../Kernel/Wizardry/Essentials/RemoveMovePath/Source/RemoveMovePath.c) | `ApplyPathfindingBlend`, `DisplayBlendedMuAp`, `DrawPathfindingUnitGhost`; hooked from `DrawUpdatedPathArrow` |
| Path / movement script | Same file | `PlayerPhase_DisplayUnitMovement` respects `remove_move_path` |
| EWRAM obj scratch | `sPathfindingGhostObjBuf` in [`config-memmap.s`](../../include/link/config-memmap.s) | 50 bytes for the blended sprite obj list |
| Install | [`RemoveMovePath.event`](../../Kernel/Wizardry/Essentials/RemoveMovePath/RemoveMovePath.event) via [`wizardry.event`](../../Kernel/Wizardry/wizardry.event) | Ships with essentials |

---

## 📝 TODO

- [ ] Optional GIF of path tip ghost vs origin-tile (no ghost)
- [ ] Document interaction with custom range overlays that do not use BG2

---

## 🐛 Limitations & Bugs

- Ghost only appears while the vanilla path-arrow path is drawn (`remove_move_path` must stay off).
- Blend setup is tuned for mGBA/hardware with BG2 range + BG3 map; alternate range rendering may need a different TargetB mix.
- Standing / hidden MUs skip the ghost (`MU_FACING_STANDING`, `hidden_b`).
- High-move units can still hit the separate vanilla path-length limit (see [Limitations.md](../Limitations.md) and `remove_move_path`).

Please report issues in the repository’s **Issues** tab.

---
