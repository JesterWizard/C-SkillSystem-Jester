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

Allocate Stat Points replaces that lottery with a spendable pool. When `gpKernelDesignerConfig->lvup_stat_points` is greater than `0`, a player unit that levels up gains that many points instead of growth rolls. After the EXP bar finishes, the spend list opens in place of the traditional level-up screen. The unit spends the points immediately, adding `+1` to a chosen stat. Each stat can receive at most one point until the current pool is spent.

The feature is a full replacement for growth-based level-ups, not a bonus on top of them. Set the config to `0` to restore vanilla growths, the level-up screen, and growth display.

Player-facing rules:
- Only blue units earn and spend points.
- The spend list appears automatically after the EXP bar on the map, in battle animations, and in BEXP.
- Points must be spent in that menu. Nothing is stored for later.
- B does not close the list until every remaining point is spent, or until no stat can be raised further.
- A stat that already received a point this pool is greyed out. If the pool is larger than eight, a new round starts after every stat has been raised once.
- Stats at their class cap (including Limit Breaker) cannot be raised further.
- SELECT no longer toggles growth rates on the unit page.

---

## 🛠️ Plan

The system is a one-shot spend after the EXP bar. Remaining points and per-stat locks live on the menu proc, not in save RAM.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `lvup_stat_points` is `0`. | Vanilla `UnitLvupCore` growths and both level-up screens run as usual. |
| 2 | `lvup_stat_points` is `N` (`N > 0`) and a blue unit gains `L` levels. | Growth `change*` fields stay `0`. The spend menu later computes `N * L` from `bu->unit.level - bu->levelPrevious`. |
| 3 | Skip `StartManimLevelUp` / `NewEkrLevelup` drawing and open the spend list. | Map, battle-anim, and BEXP level-up screens never open. The EXP bar still plays, then the spend list blocks until the points are spent. |
| 4 | The submenu lists remaining points plus HP, Str, Mag, Skl, Spd, Lck, Def, and Res. | A selects a stat, spends one point, and writes `+1` into that battle unit's `change*` field (and `bu->unit.curHP` for HP). Capped stats and stats already raised this pool are greyed out. |
| 5 | Spending the last point, or B when nothing can be raised, ends the list. | `UpdateUnitFromBattle` commits the `change*` fields to the real unit. Unspent leftover points are discarded. |
| 6 | `lvup_stat_points` is `N` (`N > 0`). | Stat-screen growth display is hidden: SELECT does not toggle rates, and labels stay gold. |

Display values are `stored stat + change*` so the list shows the new totals before battle-end commit. HP also increments `bu->unit.curHP` so the gained HP is healed when the unit is updated.

---

## 🗂️ Code Locations

All runtime behavior is gated behind `gpKernelDesignerConfig->lvup_stat_points` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). `0` disables the feature. Any other value is the number of points granted per level gained.

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config field | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Declares `lvup_stat_points`. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Defaults to `3`. Set to `0` to restore growths, the level-up screen, and growth display. |
| Point grant | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Lvup/Source/Levelup.c) | Skips `UnitLvupCore` when the config is on and leaves `change*` at `0`. |
| Map level-up | `StartManimLevelUp` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Starts the spend menu as a blocking child after the map EXP bar. |
| Battle-anim level-up | `NewEkrLevelup` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Starts the spend menu in place of the banim level-up screen and holds `CheckEkrLvupDone` until it closes. |
| BEXP level-up | `CallLevelUpProc` in [`BEXP.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/BEXP.c) | Starts the spend menu instead of `ProcScr_ManimLevelUp`. |
| Declarations | [`lvup.h`](../../include/kernel/lvup.h) | Exports `StartLvupStatPointsMenu`. |
| Spend submenu | `StartLvupStatPointsMenu` and `sStatPointsMenuDef` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Session-local point byte and allocated bitfield on `ProcLvupStatPoints`. Applies `+1` to `change*`, one point per stat, and respects class caps plus Limit Breaker. |
| Battle commit | `UpdateUnitFromBattle` in [`BattleUnitHook.c`](../../Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c) | Adds `change*` onto the real unit after the menu closes. |
| Growth display hide | `PageNumCtrl_DisplayBlinkIcons` in [`ToggleUnitPage.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/ToggleUnitPage.c) | Disables SELECT growth toggle when the config is on. |
| Growth label color | `PutDrawTextRework` in [`Util.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/Util.c) and `DisplayHpStr` in [`DrawPageLeft.c`](../../Kernel/Wizardry/StatScreen/DrawPages/DrawPageLeft.c) | Keep stat labels gold instead of coloring them by growth. |
| Command text | `MSG_MenuCommand_Allocate_*` in [`Skills_Menu.txt`](../../Contents/Texts/Source/texts/Skills_Menu.txt) | Name and help text for Allocate. |

---

## 📝 TODO

- [ ] Add a screenshot or gif of the Allocate submenu.
- [ ] Decide whether in-chapter promotion and trainee promotion should still fire when the level-up screen is skipped.
- [ ] Expose a per-stat filter (for example, no HP) if a project wants a shorter spend list.

---

## 🐛 Limitations & Bugs

- Each stat can receive only one point until every listed stat has been raised once or the pool is empty. A new round then starts.
- The EXP bar still plays. The spend list replaces the stat-gain window.
- Level-up quotes (`talk_on_level_up`) never run, because they are started from `StartManimLevelUp`.
- In-chapter promotion and trainee promotion that fire from `ManimLevelUp_ScrollOut` do not run while this feature is on. See [Trainee In-Chapter Promotion](TraineeInChapterPromotion.md).
- Non-blue units do not receive points. They also receive no growths if they level up while the config is on, because `UnitLvupCore` is skipped for every faction.
- Unspendable leftover points (every listed stat already capped) are discarded when the menu closes.
- Existing saves created while the old per-pid EMS chunk existed will load with a shifted save layout. Use a new save when testing.

Please report any issues in the repository's Issues tab.
