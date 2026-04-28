# Trainee In-Chapter Promotion

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

In vanilla FE8, trainee units (Ross, Amelia, Ewan) who reach level 10 during a battle chapter are queued for their special promotion event at the start of the **next** chapter's preparation screen. This means the promotion is deferred rather than immediate, which can feel disconnected from the level-up moment.

This feature allows trainees to promote **immediately within the chapter** — right after their level-up screen finishes scrolling out — so the promotion event triggers as soon as they cap their trainee level during a map.

The system is gated behind `gpKernelDesignerConfig->promote_trainees_in_chapter`, so projects that prefer the vanilla deferred behaviour can keep it by setting the flag to `false`.

Player-facing rules:
- Only trainee-class units (`CA_MAXLEVEL10`) are affected.
- Promotion triggers after the post-battle level-up screen when the unit reaches level 10.
- The trainee dialogue and class-selection screen (`StartBmPromotion`) play out in full, just as they would at the prep screen.
- Setting `promote_trainees_in_chapter = false` reverts to the vanilla deferred promotion flow.

## 🛠️ Plan

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `CheckBattleUnitLevelUp` caps the trainee at level 10 and sets `exp = UNIT_EXP_DISABLED`. | Unit state is correct by the time the level-up screen fires. |
| 2 | `ManimLevelUp_ScrollOut` detects scroll completion and reads the unit attributes. | Entry point for all post-level-up side effects (promotion, trainee event). |
| 3 | Guard check: `gpKernelDesignerConfig->promote_trainees_in_chapter` and `CA_MAXLEVEL10` and `level >= 10`. | Only fires for eligible trainee units when the feature is on. |
| 4 | `gActionData.subjectIndex` is set to the unit's index; `StartBmPromotion` is called with the level-up proc as parent. | The full promotion flow — including the trainee-specific dialogue proc (`StartPromoTraineeEvent`) and class-selection screen — runs as a blocking child. |
| 5 | `gActionData.subjectIndex` is reset to `0` before `Proc_Break`. | Prevents the enemy-phase AI from inheriting the promotion subject index. |

## 🗂️ Code Locations

All runtime behaviour is gated behind `gpKernelDesignerConfig->promote_trainees_in_chapter` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config flag | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Adds the runtime boolean that enables or disables in-chapter trainee promotion. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables the feature by default so current projects get immediate promotion unless they opt out. |
| Level-up scroll hook | `ManimLevelUp_ScrollOut` in [`MapLvup.c`](../../Kernel/Wizardry/Common/Lvupfx/Lvupfx/MapLvup.c) | Checks for a level-10 trainee after the level-up screen finishes scrolling out and calls `StartBmPromotion` to begin the promotion flow. |
| Promotion entry point | `StartBmPromotion` in `classchg.h` (vanilla) | Starts the full battle-map promotion proc, which internally routes trainee units through `PromoMain_SetupTraineeEvent` and `StartPromoTraineeEvent`. |
| Trainee dialogue proc | `StartPromoTraineeEvent` / `ProcScr_PromoSelectEvent_NEW` in [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c) | Runs the trainee-specific character dialogue and fade before handing off to class selection. |
| Trainee message table | `sTraineePromoMsgLut` in [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c) | Maps character IDs for Ross, Amelia, and Ewan to their respective trainee promotion dialogue strings. |
| Trainee level cap | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Core/Lvup/Source/Levelup.c) | Caps the trainee's level at 10 and disables further exp gain (`UNIT_EXP_DISABLED`) during battle exp processing. |

## 📝 TODO

- Evaluate whether the feature should also fire for custom trainee-like classes added by projects (currently relies solely on `CA_MAXLEVEL10` class attribute).
- Consider whether a second-tier trainee (units that can promote twice) should also chain into an immediate second promotion after the first class change completes.

## 🐛 Limitations & Bugs

- In-chapter trainee promotion fires from `ManimLevelUp_ScrollOut`, which runs during the battle-animation level-up screen. It does not fire from BEXP level-ups or script-driven exp grants that bypass the map animation level-up proc.
- Trainee units levelled to 10 during an enemy-phase battle will trigger the promotion event immediately after the enemy-phase animation ends. This is consistent with how `promotion_on_max_level` behaves for regular units but may feel surprising.
- The feature shares the `gActionData.subjectIndex = 0` cleanup pattern used by `promotion_on_max_level`; if both features are enabled simultaneously and a unit somehow satisfies both conditions, `promotion_on_max_level` fires first and the trainee check is skipped in that frame.

Please report any issues in the repository's Issues tab.
