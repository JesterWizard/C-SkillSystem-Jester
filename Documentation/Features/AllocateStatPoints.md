# Spend Your Levels

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla GBA Fire Emblem rolls every stat independently on level-up. The player watches the screen, hopes the important stats went up, and has no say in the result.

Allocate Stat Points replaces that lottery with a spendable pool. When `gpKernelDesignerConfig->lvup_stat_points` is greater than `0`, a player unit that levels up gains that many points instead of growth rolls. The traditional level-up screen does not appear. The unit then spends the points from the action menu with **Allocate**, adding `+1` to a chosen stat.

The feature is a full replacement for growth-based level-ups, not a bonus on top of them. Set the config to `0` to restore vanilla growths and the level-up screen.

Player-facing rules:
- Only blue units earn and spend points.
- **Allocate** appears only when that unit has at least one unspent point.
- Spending a point does not end the unit's turn.
- Stats at their class cap (including Limit Breaker) cannot be raised further.

---

## 🛠️ Plan

The system is a grant-and-spend loop keyed by character ID, not roster slot.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `lvup_stat_points` is `0`. | Vanilla `UnitLvupCore` growths and both level-up screens run as usual. |
| 2 | `lvup_stat_points` is `N` (`N > 0`) and a blue unit gains `L` levels. | The unit's pool increases by `N * L`, capped at `255`. Growth `change*` fields stay `0`. |
| 3 | Skip `StartManimLevelUp` and finish `NewEkrLevelup` immediately. | Map and battle-animation level-up screens never open. The EXP bar still plays. |
| 4 | The unit action menu shows **Allocate** when the pool is non-zero. | The player can open the spend menu without waiting or ending the turn. |
| 5 | The submenu lists remaining points plus HP, Str, Mag, Skl, Spd, Lck, Def, and Res. | A selects a stat, spends one point, and applies `+1` immediately. Capped stats are greyed out. |
| 6 | B, or spending the last point, returns to the action menu. | Leftover points persist until spent, including across save and suspend. |

Points live in `gLvupStatPoints[pid - 1]` so they stay with the character when FE8 reorders the player roster (prep screen, chapter end). The array is 50 bytes, covering character IDs `1` through `50`.

`gLvupStatPoints` is reserved in `FreeRamSpace2`, not the main `FreeRamSpace` block. The low `0x438` bytes of `FreeRamSpace` already hold the 15 purple-unit structs from `FourthAllegiance/NewUnitRAMPointerSetter.event`. Putting this array there overwrites those units and crashes on new game.

---

## 🗂️ Code Locations

All runtime behavior is gated behind `gpKernelDesignerConfig->lvup_stat_points` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). `0` disables the feature. Any other value is the number of points granted per level gained.

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config field | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Declares `lvup_stat_points`. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Defaults to `3`. Set to `0` to restore growths and the level-up screen. |
| Point grant | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Lvup/Source/Levelup.c) | Skips `UnitLvupCore` when the config is on, then adds `N * levels gained` to the blue unit's pool. |
| Map level-up skip | `StartManimLevelUp` in [`MapLvup.c`](../../Kernel/Wizardry/Lvupfx/Lvupfx/MapLvup.c) | Returns immediately so the map level-up window never starts. |
| Battle-anim level-up skip | `NewEkrLevelup` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Starts the vanilla proc with `finished` already set so `CheckEkrLvupDone` returns true without drawing the screen. |
| Pool helpers | `GetLvupStatPoints`, `AddLvupStatPoints`, and `ResetLvupStatPoints` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Read, increment, and clear the per-pid pool. New games call `ResetLvupStatPoints` from `gNewSaveHooks`. |
| Save and suspend | `SaveLvupStatPoints` and `LoadLvupStatPoints` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Persist the 50-byte array through both EMS chunk lists in [`data.event`](../../Kernel/Wizardry/SaveData/data.event). |
| RAM reservation | `gLvupStatPoints` in [`config-memmap.s`](../../include/link/config-memmap.s) | 50-byte `_kernel_malloc2` block. Keep it out of `FreeRamSpace`. |
| Declarations | [`lvup.h`](../../include/kernel/lvup.h) | Exports the pool size, helpers, and Allocate command handlers. |
| Unit-menu command | `gUnitActionMenuItemsRework` in [`UnitMenu.c`](../../Data/UnitMenu/Source/UnitMenu.c) | Registers the Allocate command next to Prestige. |
| Allocate usability | `AllocateCommandUsability` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Hides the command unless the config is on, the unit is blue, and the pool is non-zero. |
| Allocate submenu | `AllocateCommandEffect` and `sStatPointsMenuDef` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Opens the spend list, applies `+1` to the chosen stat, and respects class caps plus Limit Breaker. |
| Command text | `MSG_MenuCommand_Allocate_*` in [`Skills_Menu.txt`](../../Contents/Texts/Source/texts/Skills_Menu.txt) | Name and help text for Allocate. |

---

## 📝 TODO

- [ ] Add a screenshot or gif of the Allocate submenu.
- [ ] Decide whether in-chapter promotion and trainee promotion should still fire when the level-up screen is skipped.
- [ ] Expose a per-stat filter (for example, no HP) if a project wants a shorter spend list.
- [ ] Show unspent points on the stat screen so the player does not have to open Allocate to see the pool.

---

## 🐛 Limitations & Bugs

- Character IDs above `50` cannot store points. The pool is one byte per pid in `1..50`.
- A unit can bank at most `255` unspent points.
- The EXP bar still plays. Only the stat-gain window is skipped.
- Level-up quotes (`talk_on_level_up`) never run, because they are started from `StartManimLevelUp`.
- In-chapter promotion and trainee promotion that fire from `ManimLevelUp_ScrollOut` do not run while this feature is on. See [Trainee In-Chapter Promotion](TraineeInChapterPromotion.md).
- Non-blue units do not receive points. They also receive no growths if they level up while the config is on, because `UnitLvupCore` is skipped for every faction.
- Existing saves created before this EMS chunk existed will load whatever bytes occupy that slot. Use a new save when testing.
- Do not move `gLvupStatPoints` into `FreeRamSpace`. That region is already claimed by fourth-allegiance unit structs.

Please report any issues in the repository's Issues tab.
