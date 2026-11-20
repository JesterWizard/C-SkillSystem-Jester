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
- The promotion starts when the level-up is done, in both animation modes: after the battle, once its result (level, EXP and stats) is written to the unit, from the post-action entry. With battle animations off it used to start from the map level-up box, before that write, which then put back the level and EXP the promotion had reset (the unit ended at level 10 or 20 in the new class).
- The promotion resets the unit to level 1 with EXP 0 in the new class, as an item promotion does. The hidden level keeps the level reached (20, or 10 for a trainee).

Eligibility is one helper, `AutoPromotionDue` in `MapLvup.c`, shared by `promotion_on_max_level` (an unpromoted unit at the level cap) and this feature.

## 🛠️ Plan

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `CheckBattleUnitLevelUp` caps the trainee at level 10 and sets `exp = UNIT_EXP_DISABLED`. | Unit state is correct by the time the level-up screen fires. |
| 2 | After the action, `PostAction_AutoPromotion` (a post-action entry) finds the player's unit of the battle that levelled up, once `BattleApplyUnitUpdates` has written its new level. | Entry point for the automatic promotions, with and without battle animations. |
| 3 | Guard check (`AutoPromotionDue`): `gpKernelDesignerConfig->promote_trainees_in_chapter` and `CA_MAXLEVEL10` and `level >= 10`. | Only fires for eligible trainee units when the feature is on. |
| 4 | `gActionData.subjectIndex` is set to the unit's index; `StartBmPromotion` is called with the post-action hook proc as parent. | The full promotion flow — including the trainee-specific dialogue proc (`StartPromoTraineeEvent`) and class-selection screen — runs as a blocking child. |
| 5 | `gActionData.subjectIndex` is reset to `0` after the promotion starts. | Prevents the enemy-phase AI from inheriting the promotion subject index. |

## 🗂️ Code Locations

All runtime behaviour is gated behind `gpKernelDesignerConfig->promote_trainees_in_chapter` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config flag | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Adds the runtime boolean that enables or disables in-chapter trainee promotion. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables the feature by default so current projects get immediate promotion unless they opt out. |
| Battle hook | `PostAction_AutoPromotion` in [`MapLvup.c`](../../Kernel/Wizardry/Lvupfx/Lvupfx/MapLvup.c), registered in [`data.event`](../../Kernel/Wizardry/PostActionHook/data.event) | Post-action entry (runs in both phases after the action's battle is over, with or without battle animations). Starts the promotion for the player's unit that levelled up in the battle (the attacker, or the defender in the enemy phase), if it is due and its level is already applied. `ManimLevelUp_ScrollOut` (the map level-up box) no longer starts it. |
| Promotion entry point | `StartBmPromotion` in `classchg.h` (vanilla) | Starts the full battle-map promotion proc, which internally routes trainee units through `PromoMain_SetupTraineeEvent` and `StartPromoTraineeEvent`. |
| Trainee dialogue proc | `StartPromoTraineeEvent` / `ProcScr_PromoSelectEvent_NEW` in [`MiscFunctions.c`](../../Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c) | Runs the trainee-specific character dialogue and fade before handing off to class selection. |
| Trainee message table | `sTraineePromoMsgLut` in [`MiscFunctions.c`](../../Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c) | Maps character IDs for Ross, Amelia, and Ewan to their respective trainee promotion dialogue strings. |
| Trainee level cap | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Lvup/Source/Levelup.c) | Caps the trainee's level at 10 and disables further exp gain (`UNIT_EXP_DISABLED`) during battle exp processing. |

## 📝 TODO

- Evaluate whether the feature should also fire for custom trainee-like classes added by projects (currently relies solely on `CA_MAXLEVEL10` class attribute).
- Consider whether a second-tier trainee (units that can promote twice) should also chain into an immediate second promotion after the first class change completes.

## 🐛 Limitations & Bugs

- Fixed: the map's side windows (the unit's minimug box, terrain and goal) stayed up through the promotion and drew
  over the class choice's description box (a trainee's choice showed Ross's face and name through the text) and the
  promotion animation's weapon panels. `StartAutoPromotion` ends them and their maker (`pr_SideWindowMakerScript`);
  `ProcScr_AutoPromotion` brings them back after the promotion in the player phase.

- In-chapter trainee promotion fires from the post-action entry, after a battle or a misc action's EXP, and after the debug menu's Level up (`LevelupAutoPromotion` in [`VeslyDebugger/C_Code.c`](../../Kernel/Wizardry/VeslyDebugger/C_Code.c), through `StartAutoPromotionIfDue` in `MapLvup.c`; the max-level promotion too). It does not fire from BEXP level-ups or script-driven exp grants that bypass it.
- Trainee units levelled to 10 during an enemy-phase battle will trigger the promotion event immediately after the enemy-phase animation ends. This is consistent with how `promotion_on_max_level` behaves for regular units but may feel surprising.
- The feature shares the `gActionData.subjectIndex = 0` cleanup pattern used by `promotion_on_max_level`; if both features are enabled simultaneously and a unit somehow satisfies both conditions, `promotion_on_max_level` fires first and the trainee check is skipped in that frame.

Please report any issues in the repository's Issues tab.
