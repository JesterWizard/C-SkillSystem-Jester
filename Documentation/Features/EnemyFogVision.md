# They Can't See You Either

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla fog of war is one-sided. Player units lose information outside their sight radius, but enemy AI still knows every unit position on the map.

That breaks fog as a mutual information contest. Players can be punished for guessing into the dark, while enemies never have to guess at all.

This feature gives non-player AI a real vision budget. When `enemy_fog_vision` is enabled on a fog chapter, enemies can only fight, staff, chase, or skill-target units their faction can currently see.

It reuses the same per-class vision ranges as [Fog Vision](FogVision.md), and stays separate from the player-facing fog stages in [Fog Stages](FogStages.md). Enemy sight does not rewrite `gBmMapFog` or hide player units from the map.

---

## 🛠️ Plan

### Enable conditions

| Condition | Result |
|-----------|--------|
| `gpKernelDesignerConfig->enemy_fog_vision` is false | Vanilla omniscient AI |
| Chapter has no fog (`chapterVisionRange == 0`) | Vanilla omniscient AI |
| Acting unit is player (`FACTION_BLUE`) | Feature inactive |
| Map larger than the reserved vision buffer | Falls back to omniscience for that chapter |
| All conditions pass | Shared faction vision is built and AI targeting is gated |

### Vision model

```
Faction OR vision:

  Unit A sight + Unit B sight + torch traps
           |
           v
   gBmMapEnemyVision[y][x] != 0  =>  tile is known
```

| Rule | Behavior |
|------|----------|
| Shared faction vision | Every living unit in the acting faction contributes sight |
| Range source | `GetUnitFogViewRange(unit)` (class table + torch duration) |
| Skill bonus | `SID_HazeHunter` adds +5 when present |
| Torch traps | Active `TRAP_TOGGLE_TORCH` traps add range 5 |
| Skipped units | Hidden, unavailable, or rescued units do not contribute |
| Allied targets | Always legal; vision only blocks hostile / non-allied units |
| Player fog map | Untouched — no palette, sprite, or forecast side effects |

### AI gates

| Decision path | Effect when target is unseen |
|---------------|------------------------------|
| General enemy recognition (`AiIsUnitEnemy`) | Unit is not treated as a valid enemy |
| Offensive / rampage scans | Target skipped |
| Staff / menu-skill targeting | Target skipped |
| Character / class scripted searches | Target skipped |
| Realtime nearest-enemy search | Target skipped |
| Pathing toward hostiles | No chase toward unseen units |

Unseen units still occupy `gBmMapUnit`, so AI cannot walk through them. Vision only removes knowledge and targeting, not collision.

---

## 🗂️ Code Locations

All behavior is gated behind `gpKernelDesignerConfig->enemy_fog_vision` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| **Public API** | [`enemy-fog-vision.h`](../../include/kernel/enemy-fog-vision.h) | Declares `BuildEnemyFogVision`, `EnemyFogVisionCanSeeUnit`, and `EnemyFogVisionCanTargetUnit` |
| **Vision builder** | `BuildEnemyFogVision` in [`EnemyFogVision.c`](../../Kernel/Wizardry/Common/FogVision/EnemyFogVision.c) | Allocates the working map and ORs every acting-faction unit's sight plus torch traps |
| **Visibility helpers** | `EnemyFogVisionCanSeeUnit` / `EnemyFogVisionCanTargetUnit` in [`EnemyFogVision.c`](../../Kernel/Wizardry/Common/FogVision/EnemyFogVision.c) | Tile lookup and allied-target bypass; omniscient fallback when the map was not built |
| **Decision setup** | `CpDecide_Main` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Rebuilds enemy vision after entity maps refresh and before each AI unit decides |
| **Enemy recognition gate** | `AiIsUnitEnemy` in [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c) | Routes non-allied checks through faction vision |
| **Offensive bypass gate** | `AiAttemptOffensiveAction` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Extra visibility check for rampage / bypass paths |
| **Scripted target searches** | `AiFindTargetInReachByCharId` / `AiFindTargetInReachByClassId` in [`AI.c`](../../Kernel/Wizardry/Common/AI/Source/AI.c) | Character- and class-specific AI scripts ignore unseen units |
| **Menu skill AI** | menu-skill target pick in [`MiscAiSkills.c`](../../Kernel/Wizardry/Misc/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c) | Chooses the first visible target from the candidate list |
| **Realtime AI** | nearest-enemy search in [`RealtimeScheduler.c`](../../Kernel/Wizardry/Common/RealtimeBattle/Source/RealtimeScheduler.c) | Keeps realtime combat selection vision-aware |
| **Installer** | [`FogVision_Installer.event`](../../Kernel/Wizardry/Common/FogVision/FogVision_Installer.event) | Links `EnemyFogVision.lyn.event` with the fog vision package |
| **Working buffer** | `gBmMapEnemyVisionBuffer` / `gBmMapEnemyVision` in [`config-memmap.s`](../../include/link/config-memmap.s) | Dedicated `0x800` map buffer so enemy vision never writes player `gBmMapFog` |
| **Range table consumer** | `GetUnitFogViewRange` in [`FogVision.c`](../../Kernel/Wizardry/Common/FogVision/FogVision.c) | Shared per-class vision source for both player fog and enemy AI |

---

## 📝 TODO

- Decide whether vanilla AI danger-map safety scoring should also respect enemy vision.
- Consider per-unit vision instead of shared faction OR if designers want more Fog of War asymmetry later.
- Add a small scenario test covering torch traps, HazeHunter, and oversized-map fallback.

---

## 🐛 Limitations & Bugs

- Vision is shared by the whole acting faction, not calculated independently for each AI unit.
- Maps that do not fit in the `0x800` working buffer fall back to vanilla omniscience.
- Unseen units remain movement blockers; AI still pathfinds around occupied tiles it cannot “know” about as combat targets.
- The danger map / safety score path is still omniscient.
- Player fog stages, sprites, forecasts, and palettes are intentionally unaffected.

Please report any issues in the repository’s **Issues** tab.

---
