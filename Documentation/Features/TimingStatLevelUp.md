# Time Your Levels

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla GBA Fire Emblem rolls each stat independently. Allocate Stat Points lets the player choose which stats go up, but the number of gains is fixed.

Timing Stat Level-Up turns that list into a skill check. After the EXP bar, the same stat menu appears, and each row has an arrow that runs back and forth on a track. Press A while the arrow is green to grant `+1` to that stat. Miss, skip, or wait too long and that stat stays put.

A practiced player can hit every listed stat in one level-up. That is the intended ceiling, not a bug.

This is a full replacement for growth-based level-ups. Set `lvup_stat_timing` to `0` to restore vanilla growths (unless Allocate is also on). If both configs are on, timing wins.

Player-facing rules:
- Only blue units play the minigame.
- The list appears automatically after the EXP bar on the map, in battle animations, and in BEXP.
- Stats are in the left window. The timing tracks are in a second window to the right.
- Each uncapped stat can be attempted once per level-up.
- A green arrow is a hit. A gold arrow is a miss if you press A. After a press, that arrow stays frozen on the track.
- B closes the list and skips any remaining stats.
- Stats at their class cap (including Limit Breaker) cannot be raised.
- SELECT no longer toggles growth rates on the unit page.

---

## 🛠️ Plan

The hit window is the config value. Higher is easier. `20` means the top 20% of the track (meter `80`–`99`) is green.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `lvup_stat_timing` is `0` and `lvup_stat_points` is `0`. | Vanilla `UnitLvupCore` growths and both level-up screens run as usual. |
| 2 | `lvup_stat_timing` is `N` (`N > 0`). | Growth `change*` fields stay `0`. After the EXP bar, the timing list opens instead of the vanilla level-up screen. |
| 3 | Left window lists the stats. Right window holds a dash track per row with a bouncing arrow. Every arrow moves at the same speed, with staggered start offsets so they do not peak together. | The selected row's idle callback advances every unlocked arrow each frame. |
| 4 | A on a live row. | If the arrow is in the hit window, write `+1` into that battle unit's `change*` field (and `bu->unit.curHP` for HP). The row locks and the arrow freezes where it was pressed. |
| 5 | Every listed stat is locked, or the player presses B. | `UpdateUnitFromBattle` commits the `change*` fields. Missed and skipped stats stay at `0`. |
| 6 | `lvup_stat_timing` is `N` (`N > 0`). | Stat-screen growth display is hidden: SELECT does not toggle rates, and labels stay gold. |

Do not enable this together with [Allocate Stat Points](AllocateStatPoints.md) unless you want timing to replace allocate.

---

## 🗂️ Code Locations

All runtime behavior is gated behind `gpKernelDesignerConfig->lvup_stat_timing` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). `0` disables the feature. Any other value is the hit-window size (`1` is a single tick at the peak, `20` is a fair default, `99` is almost always a hit).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config field | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Declares `lvup_stat_timing`. |
| Shared growth skip | `KernelLvupReplacesGrowths` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | True when allocate or timing is on. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Defaults to `0`. |
| Point grant | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Lvup/Source/Levelup.c) | Skips `UnitLvupCore` when either custom mode is on and leaves `change*` at `0`. |
| Map / banim / BEXP entry | `StartManimLevelUp`, `NewEkrLevelup`, and `CallLevelUpProc` | Same hooks as allocate; timing is chosen when `lvup_stat_timing` is nonzero. |
| Timing list | `sStatTimingMenuDef` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Draws the bouncing arrows, resolves A as hit or miss, and applies `change*`. |
| Battle commit | `UpdateUnitFromBattle` in [`BattleUnitHook.c`](../../Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c) | Adds `change*` onto the real unit after the menu closes. |
| Growth display hide | `PageNumCtrl_DisplayBlinkIcons` in [`ToggleUnitPage.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/ToggleUnitPage.c) | Disables SELECT growth toggle when either custom mode is on. |
| Command text | `MSG_MenuCommand_Timing_DESC` in [`Skills_Menu.txt`](../../Contents/Texts/Source/texts/Skills_Menu.txt) | Help text for the timing list. |

---

## 📝 TODO

- [ ] Add a screenshot or gif of the bouncing-arrow list.
- [ ] Expose a per-stat speed table if a project wants slower HP and faster luck.
- [ ] Decide whether gaining two levels at once should grant two attempts per stat.

---

## 🐛 Limitations & Bugs

- Each listed stat can be attempted only once per level-up, even if the unit gained more than one level from the EXP bar.
- The EXP bar still plays. The timing list replaces the stat-gain window.
- Level-up quotes (`talk_on_level_up`) never run, because they are started from `StartManimLevelUp`.
- In-chapter promotion and trainee promotion that fire from `ManimLevelUp_ScrollOut` do not run while this feature is on. See [Trainee In-Chapter Promotion](TraineeInChapterPromotion.md).
- Non-blue units do not play the minigame. They also receive no growths if they level up while the config is on, because `UnitLvupCore` is skipped for every faction.
- If both `lvup_stat_timing` and `lvup_stat_points` are set, timing is used and allocate is ignored.

Please report any issues in the repository's Issues tab.
