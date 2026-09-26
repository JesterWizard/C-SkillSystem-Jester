# Time Your Levels

<p align="center">
  <img src="../Gifs/Timing_Stat_System.gif" alt="Timing Stat Level-Up" width="600"/>
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

Vanilla GBA Fire Emblem rolls each stat independently. [Allocate Stat Points](AllocateStatPoints.md) lets the player choose which stats go up, but the number of gains is fixed.

Timing Stat Level-Up turns that list into a skill check. After the EXP bar, the same eight-stat menu appears, and each row has an arrow that runs back and forth on a dash track. Press A while the arrow is green to grant `+1` to that stat. Miss, and that stat stays put.

A practiced player can hit every listed stat in one level-up. That is the intended ceiling, not a bug.

This is a full replacement for growth-based level-ups. Set `lvup_stat_timing` to `0` to restore vanilla growths (unless Allocate is also on). If both configs are on, timing wins.

Player-facing rules:
- Only blue units play the minigame.
- The list appears automatically after the EXP bar on the map, in battle animations, and in BEXP.
- Stats are in the left window. The timing tracks are in a matching-height window to the right.
- Each uncapped stat can be attempted once per level-up.
- A green arrow is a hit. A gold arrow is a miss if you press A. After a press, that arrow stays frozen on the track.
- A moves the hand to the next open row. After the last attempt, the list waits 60 frames and then closes on its own.
- B does nothing. You cannot skip remaining stats.
- Stats at their class cap (including Limit Breaker) have no arrow and cannot be raised.
- SELECT no longer toggles growth rates on the unit page.

---

## 🛠️ Plan

The hit window is the config value. Higher is easier. The meter runs `0`–`99` on a triangle wave. Green is the top of that meter: `20` means meter `80`–`99` is a hit.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | `lvup_stat_timing` is `0` and `lvup_stat_points` is `0`. | Vanilla `UnitLvupCore` growths and both level-up screens run as usual. |
| 2 | `lvup_stat_timing` is `N` (`N > 0`). | Growth `change*` fields stay `0`. After the EXP bar, the timing list opens instead of the vanilla level-up screen. |
| 3 | Left window lists HP, Str, Mag, Skl, Spd, Lck, Def, and Res. Right window holds a dash track per row with a bouncing arrow. Every unlocked arrow moves at the same speed (`TIMING_SPEED` 3, period 200). Start phases are staggered (`i * 25`) so they do not peak together. | The selected row's idle callback redraws the right frame, advances every unlocked arrow, and stamps every track. |
| 4 | A on a live row. | If the arrow is in the hit window, write `+1` into that battle unit's `change*` field (and `bu->unit.curHP` for HP). The row locks and the arrow freezes where it was pressed (green on hit, gold on miss). The hand jumps to the next open uncapped row. |
| 5 | A on the last remaining row. | The row locks as above. `ProcScr_TimingClose` sleeps 60 frames, then ends the menu. `UpdateUnitFromBattle` commits the `change*` fields. Missed stats stay at `0`. |
| 6 | `lvup_stat_timing` is `N` (`N > 0`). | Stat-screen growth display is hidden: SELECT does not toggle rates, and labels stay gold. |

Do not enable this together with [Allocate Stat Points](AllocateStatPoints.md) unless you want timing to replace allocate.

Display values are `stored stat + change*` so the list shows the new totals before battle-end commit. HP also increments `bu->unit.curHP` so the gained HP is healed when the unit is updated.

Session state lives on `ProcLvupStatPoints` (`allocated`, `phase[8]`, `delay`). There is no extra save RAM.

---

## 🗂️ Code Locations

All runtime behavior is gated behind `gpKernelDesignerConfig->lvup_stat_timing` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). `0` disables the feature. Any other value is the hit-window size, clamped to `1`–`99` (`1` is a single tick at the peak, `20` is a fair default, `99` is almost always a hit).

| Feature | Location | Description |
|--------|----------|-------------|
| Designer config field | `KernelDesigerConfig` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Declares `lvup_stat_timing`. |
| Shared growth skip | `KernelLvupReplacesGrowths` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | True when allocate or timing is on. |
| Default value | `gKernelDesigerConfig` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Set to `0` to restore growths. Nonzero is the hit window and overrides allocate. |
| Point grant | `CheckBattleUnitLevelUp` in [`Levelup.c`](../../Kernel/Wizardry/Lvup/Source/Levelup.c) | Skips `UnitLvupCore` when either custom mode is on and leaves `change*` at `0`. |
| Map level-up | `StartManimLevelUp` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Starts the timing menu as a blocking child after the map EXP bar. |
| Battle-anim level-up | `NewEkrLevelup` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Starts the timing menu in place of the banim level-up screen and holds `CheckEkrLvupDone` until it closes. |
| BEXP level-up | `CallLevelUpProc` in [`BEXP.c`](../../Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/BEXP.c) | Starts the timing menu instead of `ProcScr_ManimLevelUp`. |
| Declarations | [`lvup.h`](../../include/kernel/lvup.h) | Exports `StartLvupStatPointsMenu`. |
| Timing list | `sStatTimingMenuDef` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | Two-column UI: left stat names/values, right dash tracks. `StatTimingMenu_OnIdle` advances arrows and redraws the right `DrawUiFrame`. |
| Hit / miss | `StatTimingMenu_OnSelectStat` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | A in the window writes `change*`. A always locks the row, freezes the arrow, and moves the hand. |
| Auto-close | `ProcScr_TimingClose` in [`StatPoints.c`](../../Data/UnitMenu/Source/StatPoints.c) | After the last attempt, sleeps 60 frames then `EndMenu`. B is ignored (`StatTimingMenu_OnCancel`). |
| Battle commit | `UpdateUnitFromBattle` in [`BattleUnitHook.c`](../../Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c) | Adds `change*` onto the real unit after the menu closes. |
| Growth display hide | `PageNumCtrl_DisplayBlinkIcons` in [`ToggleUnitPage.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/ToggleUnitPage.c) | Disables SELECT growth toggle when either custom mode is on. |
| Growth label color | `PutDrawTextRework` in [`Util.c`](../../Kernel/Wizardry/StatScreen/DrawUnitPage/Source/Util.c) and `DisplayHpStr` in [`DrawPageLeft.c`](../../Kernel/Wizardry/StatScreen/DrawPages/DrawPageLeft.c) | Keep stat labels gold instead of coloring them by growth. |
| Command text | `MSG_MenuCommand_Timing_DESC` in [`Skills_Menu.txt`](../../Contents/Texts/Source/texts/Skills_Menu.txt) | Help text for the timing list. |

---

## 📝 TODO

- [ ] Expose a per-stat speed table if a project wants slower HP and faster luck.
- [ ] Decide whether gaining two levels at once should grant two attempts per stat.
- [ ] Decide whether in-chapter promotion and trainee promotion should still fire when the level-up screen is skipped.

---

## 🐛 Limitations & Bugs

- Each listed stat can be attempted only once per level-up, even if the unit gained more than one level from the EXP bar.
- The EXP bar still plays. The timing list replaces the stat-gain window.
- Level-up quotes (`talk_on_level_up`) never run, because they are started from `StartManimLevelUp`.
- In-chapter promotion and trainee promotion that fire from `ManimLevelUp_ScrollOut` do not run while this feature is on. See [Trainee In-Chapter Promotion](TraineeInChapterPromotion.md).
- Non-blue units do not play the minigame. They also receive no growths if they level up while the config is on, because `UnitLvupCore` is skipped for every faction.
- If both `lvup_stat_timing` and `lvup_stat_points` are set, timing is used and allocate is ignored.
- Do not draw the right window with this repo's `DrawUiFrame2` (Popup.c). That lyn-replace stamps 2x2 blocks and clears BG0. The working path is vanilla `DrawUiFrame` on the menu `backBg`.

Please report any issues in the repository's Issues tab.
