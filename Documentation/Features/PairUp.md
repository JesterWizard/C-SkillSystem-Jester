# Pair-Up Commands

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Runtime Configuration](#runtime-configuration)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

The Pair-Up system is an upgrade of Rescue that the story gives later: explicit commands for joining units, changing companions, and changing the active unit. Until it is unlocked, the unit menu has vanilla's **Rescue** (with Drop, Take and Give), and a rescue works as in vanilla.

Pairing uses the existing FE8 rescue state fields internally:

- The leader remains on the map and receives the companion's stat bonuses.
- The support unit is hidden and follows the leader.
- The reciprocal `rescue` indices identify both units in the pair.

The initial ASM reference for this C port came from **CirclesEverywhere**. See [Circles' Compendium](https://feuniverse.us/t/circles-compendium/13510) for the original community ASM reference work.

---

## 🛠️ Plan

### Command flow

```text
Unit menu
   │
   ├─ pair_up_enabled?
   ├─ Active unit has not moved?
   └─ Adjacent valid ally?
          │
          ├─ Pair Up ─── active unit becomes leader
          ├─ Shelter ── selected ally becomes leader
          ├─ Transfer ─ swap companions between paired leaders
          └─ Switch ─── invert leader and support roles
```

### Commands

| Command | Behaviour |
|---------|-----------|
| **Pair Up** | The active unit pairs with an adjacent ally and gains 30% of that ally's supported stats. |
| **Shelter** | The active unit pairs into an adjacent ally. The selected ally becomes the active leader after the action. |
| **Transfer** | The active leader transfers their companion to an adjacent valid unit, swapping companions when the target is already paired. |
| **Switch** | The leader and support exchange roles. The new leader becomes the active unit. |

### Stat bonuses and preview

The support unit contributes 30% of its effective stat, rounded down, for:

`POW`, `MAG`, `SKL`, `SPD`, `LCK`, `DEF`, `RES`, and `MOV`.

The target-selection preview shows all eight values, including `+0`. Aid and CON are not part of the preview or pairing eligibility. The preview is updated whenever the target cursor moves. The unit windows and the preview sit on the right of the screen, or on the left when the target is on the right half, so the map cursor on the target never covers them (`PairUp_WindowX`).

---

## ⚙️ Runtime Configuration

Pair Up is on only when both hold (`PairUp_IsUnlocked` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c)):

- `pair_up_enabled` is set in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) (`.pair_up_enabled = true`, the default), and
- the permanent flag `GLOBAL_FLAG_PAIR_UP_UNLOCKED` (`0x9C`, [`custom-structs.h`](../../include/jester_headers/custom-structs.h)) is set. Nothing sets it yet: the event that gives the upgrade sets it with `ENUT 0x9C` (or `SetFlag(GLOBAL_FLAG_PAIR_UP_UNLOCKED)` in C).

Before that, the unit menu has vanilla's Rescue (`RescueUsability`, rewritten in `PairUp.c` with vanilla's checks, and vanilla's `RescueEffect`); `ActionRescue` runs vanilla's rescue; the rescuer's Skl and Spd are halved as in vanilla (`PairUp_RescueStatScale`), with no pair bonus; a rescued ally is not drawn beside its rescuer (`PutRescuedSupportSpritesOam`, `MirrorSprites.c`); and Pair Up, Shelter, Transfer and Switch are not shown. Rescue needs the rescuer's Aid to cover the unit's Con, as in vanilla (`TryAddUnitToRescueTargetList`, `UnitTarget.c`); Pair Up has no such limit.

After it, Rescue is not shown, Pair Up and the other three commands are, and every rescue state between allies is a pair: a rescue made before the upgrade becomes a pair.

**The Pair Up skill.** `SID_PairUp` is a menu skill: before the unlock, a unit with it has **Pair Up** in its Skills
submenu (`PairUp_SkillUsability` / `PairUp_SkillOnSelected`, `SkillMenuInfo.c`), and Pair Up works for that unit and the
unit it carries or is carried by (`PairUp_IsUnlockedFor`): their rescue is a pairing, with the bonuses and the carried
unit drawn. Its plain Rescue row pairs as well. Once the story unlocks Pair Up the row hides: the unit menu has its own.

**Canto.** A unit with Canto keeps its remaining movement after Rescue, Drop, Take or Give (and so after Pair Up)
(`CheckCanto`, `Canto.c`). `ApplyUnitAction` sets `US_HAS_MOVED` on every action, the bit that also means "has used
Canto", and those four actions are never offered while Canto is in use, so for them it is ignored.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|---------|----------|-------------|
| Runtime flag | `pair_up_enabled` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) / [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Allows the four pair-up commands |
| Unlock | `PairUp_IsUnlocked`, `GLOBAL_FLAG_PAIR_UP_UNLOCKED` (`0x9C`) | The story's upgrade from Rescue |
| Rescue (before the upgrade) | `RescueUsability`, `ActionRescue` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c); the Rescue row in [`UnitMenu.c`](../../Data/UnitMenu/Source/UnitMenu.c) | Vanilla's Rescue |
| Pair state | `PairUp_Attach`, `PairUp_Separate`, `PairUp_GetLeader`, and `PairUp_GetSupport` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Reads and updates the rescue-backed leader/support relationship |
| Pair and Shelter actions | `ActionRescue` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Applies Pair Up or Shelter mode and updates the active unit |
| Transfer | `PairUp_Transfer` and `PairUp_TransferOnSelect` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Moves companions between adjacent leaders |
| Switch | `PairUp_Switch` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Exchanges leader/support state and active-unit tracking |
| Stat calculation | `PairUp_GetStatBonus` and `PairUp_RescueStatScale` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Applies the rounded 30% bonuses |
| Movement bonus | `_GetUnitMov` in [`MovGetter.c`](../../Kernel/Wizardry/UnitStatusGetter/source/MovGetter.c) | Applies the pair-up MOV contribution |
| Target preview | `PairUp_DrawStatPreview` and `PairUp_SelectionOnSwitchIn` in [`PairUp.c`](../../Kernel/Wizardry/PairUp/Source/PairUp.c) | Draws and refreshes the live stat preview |
| Selection callbacks | `gSelectInfo_PairUp` in [`PairUpSelectInfo.c`](../../Kernel/Wizardry/PairUp/Source/PairUpSelectInfo.c) | Connects target selection to the custom preview |
| Unit-menu commands | Pair-up rows in [`UnitMenu.c`](../../Data/UnitMenu/Source/UnitMenu.c) | Registers Pair Up, Shelter, Transfer, and Switch |
| Command text | Pair-up messages in [`Skills_Menu.txt`](../../Contents/Texts/Source/texts/Skills_Menu.txt) | Defines names and descriptions |
| Event installation | [`PairUp.event`](../../Kernel/Wizardry/PairUp/PairUp.event) | Includes the generated pair-up code and selection data |

---

## 📝 TODO

- Set `GLOBAL_FLAG_PAIR_UP_UNLOCKED` in the story event that gives the upgrade (not decided yet).
- Add dedicated gameplay tests for each command and active-unit transition.
- Decide whether disabling the flag mid-chapter should automatically separate existing pairs.
- Replace the rescue marker byte with a dedicated pair-up mode field if the system gains additional action modes.

---

## 🐛 Limitations & Bugs

- Pair state is stored in FE8 rescue fields and uses `US_RESCUING`, `US_RESCUED`, and `US_HIDDEN`.
- The configuration flag controls command access; it does not migrate or clear pairs that already exist when the flag is changed.
- The stat preview layout is fixed to the current 14-tile, two-column panel.
- Existing rescue/drop engine paths still share some underlying state and animation behavior.

Please report issues in the repository's **Issues** tab.

---
