# AI Rescue Retreat

<p align="center">
  <img src="../Gifs/AI_Rescue_Retreat.gif" alt="AI Rescue Retreat" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

This feature lets AI-controlled units use rescue and drop actions as a tactical retreat tool.

When the toggle is enabled, the AI can:

- Rescue an adjacent allied unit that is below half HP.
- Move while carrying that ally toward the farthest reachable tile from hostile units.
- Drop the carried unit only when the AI finds a legal adjacent drop tile.

The goal is to make enemy support units feel more coordinated without changing the core rescue rules or inventing special-case movement outside the normal action system.

## 🛠️ Plan

The behavior is built around the existing heal-or-escape AI branch.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | Check the designer-config toggle. | Designers can disable the rescue-retreat behavior without removing the implementation. |
| 2 | Look for an adjacent allied unit under half HP. | The AI only attempts rescue when there is a nearby tactical target. |
| 3 | Confirm normal rescue legality. | The AI still respects the engine's aid/con rules. |
| 4 | If already rescuing, search for the farthest reachable carry position. | The carrier prefers to move away from hostile units instead of simply taking the first safe tile. |
| 5 | Drop only on a legal adjacent tile. | The carried unit is released through the normal drop action instead of a custom placement shortcut. |

The same branch falls back to normal healing and escape behavior if rescue is not available.

## 🗂️ Code Locations

All modifications are gated behind `gpKernelDesignerConfig->rescue_drop_ai_use` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config flag | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Adds the runtime boolean that turns the rescue-retreat AI on or off. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Keeps the behavior enabled by default so existing rescue-capable AI keeps working unless a designer disables it. |
| Rescue target selection | `AiGetAdjacentRescueTarget` and `AiTryRescueWeakAdjacentAlly` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Finds an adjacent allied unit below half HP and turns that into a rescue decision. |
| Carry retreat logic | `AiFindBestDropTile`, `AiFindFarthestCarryPosition`, and `AiTryRescueCarryDrop` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Chooses a reachable retreat tile by enemy distance and then drops the carried unit on a legal adjacent tile. |
| AI decision hook | `DecideHealOrEscape` in [`AiOptimization.c`](../../Kernel/Wizardry/Core/AiHack/AiOptimization/Source/AiOptimization.c) | Calls the rescue-retreat branch before the normal heal-or-move fallback logic. |
| Rescue action wiring | `AiStartRescueAction` and `AiStartDropAction` in [`AiAction.c`](../../Kernel/Wizardry/Core/AiHack/AiAction/Source/AiAction.c) | Executes the underlying unit actions once the AI has chosen rescue or drop. |

## 📝 TODO

- Consider adding a designer-facing note for the HP threshold if that value ever needs to become configurable.
- Review whether AI rescue should prioritize specific ally classes or unit roles once more advanced heuristics are available.
- Add a small scenario test for a carrier that can retreat but cannot drop immediately.

## 🐛 Limitations & Bugs

- The AI still obeys the engine's normal rescue rules, so aid and con mismatches can block the behavior entirely.
- Units with no legal retreat tile will still fall back to the remaining heal-or-escape logic.
- The feature only reacts to adjacent allies; it does not search the map for a distant rescue target.

Please report any issues in the repository's Issues tab.