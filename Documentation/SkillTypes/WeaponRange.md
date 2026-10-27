# Weapon-range skills

> Text file: `Contents/Texts/Source/texts/Skills_WeaponRange.txt`  ·  Code: `Kernel/Wizardry/WeaponRange/`  ·  Hook: `gWeaponRangeGetters` (`Kernel/Wizardry/WeaponRange/data.event:2`) for max range; `GetItemMinRangeRework` (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:16`) for min range

## What it is

Weapon-range skills change the minimum and maximum range of weapons, magic tomes and staves for one unit (Bow Range +1/+2, Magic Range +1/+2, Running Start, High Ground, Point Blank, Missile Arm, Counterattack, Staff Savant, Sword Saint), and decide which allies or units a staff may target (Self Healing, Saint, Aid Refusal, Cursed Heal). Every range read by the game goes through the same getter chain, so one edit changes attack maps, staff maps, danger zones and AI reach together.

## How the engine calls it

1. Range reads go to `GetUnitMinRange` (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:129`) and `GetUnitMaxRange` (`RangeGetter.c:178`). Both loop `unit->items`, keep items where `CanUnitUseWeapon` is true, and add gaiden-magic weapons when `gpKernelDesignerConfig->gaiden_magic` is set. Each item's value comes from `GetItemMinRangeRework` / `GetItemMaxRangeRework`.
2. `GetItemMinRangeRework(item, unit)` (`RangeGetter.c:16`) starts from `GetItemMinRange(item)`. Point Blank sets bows to 1 (`:21-24`), Missile Arm sets lances to 1 (`:26-29`). There is no min-range getter list.
3. `GetItemMaxRangeRework(item, unit)` (`RangeGetter.c:34`) starts from `GetItemMaxRange(item)`. If that is 0 it uses `GetUnitMagBy2Range(unit)` (`:37-40`). Then it walks `gpWeaponRangeGetters` (`:43-44`). The list is in `Kernel/Wizardry/WeaponRange/data.event:2`: `WeaponRangeGetterSkills` (`RangeGetter.c:50`), `WeaponRangeGetterCombatArt` (`Kernel/Wizardry/CombatArt/Source/MiscHooks.c:79`), `StatusGetterCheckCpas`. `data.event` is included from `Kernel/Wizardry/wizardry.event:32`.
4. `WeaponRangeGetterSkills(range, unit, item)` (`RangeGetter.c:50`) applies Counterattack (`:53-56`) and Thrust (`:58-61`) first, then switches on `GetItemType(item)` (`:63`): staff (`:64`, Staff Savant), sword (`:71`, Sword Saint), lance (`:78`, Missile Arm), bow (`:85`, Bow Range +1/+2, Running Start `:96`, High Ground `:101`), anima/light/dark (`:107`, Magic Range +1/+2, High Ground `:120`).
5. Reach masks are built from min and max by `GetItemReachBitsRework` (`Kernel/Wizardry/WeaponRange/Source/Utils.c:24`). Its callers are `GetUnitWeaponReachBits` (`Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:55`), `GetUnitStaffReachBits` (`MiscHooks.c:128`), `GenerateUnitCompleteAttackRange` (`MiscHooks.c:287`) and `GenerateUnitCompleteStaffRange` (`MiscHooks.c:369`). `calculate_map_range_faster` (`MiscHooks.c:302`, `:332` in the attack builder; `:376`, `:401` in the staff builder) switches those builders to min/max bounds instead of per-item masks.
6. Staff targeting is separate from range. `MakeTargetListForAdjacentHealInternal` (`Kernel/Wizardry/WeaponRange/Source/StaffRange.c:68`) and `MakeTargetListForRangedHealInternal` (`StaffRange.c:86`) add the caster for Self Healing (`:77-80`, `:95-98`). `TryAddUnitToHealTargetList` (`StaffRange.c:105`) checks Saint (`:109-112`), Aid Refusal (`:118-121`) and Cursed Heal (`:127-130`).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/WeaponRange/Source/RangeGetter.c` | `GetItemMinRangeRework` (`:16`), `GetItemMaxRangeRework` (`:34`), `WeaponRangeGetterSkills` (`:50`), `GetUnitMinRange` (`:129`), `GetUnitMaxRange` (`:178`). Main file for this type. |
| `Kernel/Wizardry/WeaponRange/data.event` | `gWeaponRangeGetters` list (max-range getters). |
| `Kernel/Wizardry/WeaponRange/WeaponRange.event` | Includes the WeaponRange sources. `data.event` is not included here (commented out); it is included from `Kernel/Wizardry/wizardry.event:32`. |
| `Kernel/Wizardry/WeaponRange/Source/StaffRange.c` | Staff target lists: Self Healing, Saint, Aid Refusal, Cursed Heal, plus siege-repair obstacles. |
| `Kernel/Wizardry/WeaponRange/Source/MiscHooks.c` | Reach-mask builders `GetUnitWeaponReachBits` (`:55`), `GetUnitStaffReachBits` (`:128`), `GenerateUnitCompleteAttackRange` (`:287`), `DisplayUnitEffectRange` (`:206`), `GenerateDangerZoneRange` (`:411`). |
| `Kernel/Wizardry/WeaponRange/Source/Utils.c` | `GetItemReachBitsRework` (`:24`), `IsItemCoveringRangeRework`, `AddMap`. |
| `Kernel/Wizardry/WeaponRange/Source/MiscHooksWeaponRange.event` | Installs the BL hooks that call `GetItemMinRangeRework` / `GetItemMaxRangeRework` from vanilla range code. |
| `Kernel/Wizardry/WeaponRange/Source/LynJump.event` | Lyn jump entries for the WeaponRange functions. |
| `Kernel/Wizardry/WeaponRange/Source/RangeString.c` | Range text shown in the unit display (`GetUnitDisplayRangeString` at `:69`, not read in detail). |
| `Kernel/Wizardry/CombatArt/Source/MiscHooks.c:79` | `WeaponRangeGetterCombatArt`, the second entry in `gWeaponRangeGetters`. |
| `Kernel/Wizardry/ItemSys/Source/StaffHeal.c` | Staff effects (not range): Bloom (`:64`, `:181`, `:300`), Saint (`:75`), Cursed Heal (`:88`, `:291`), Aid Refusal (`:84`), Self Healing (`:366`), Explosive Heal (`:305`). |
| `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-extra.c:567` | Explosive Buff effect (single-target buff staves also hit adjacent allies). Effect code, not range. |
| `Data/SkillSys/Source/SkillExtraInfo.c:546-567` | SKILL_EFF values for Magic Range +1 (`{{ 1 }}`), Bow Range +1 (`{{ 1 }}`), Staff Savant (`{{ 1 }}`), High Ground (`{{ 2 }}`). |
| `include/kernel/kernel-lib.h` | `ARRAY_COUNT_RANGE*` and `gVecs_*` declarations. `StaffHeal.c:271` walks `gVecs_3x3` for the heal effect around the target. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the place by effect:
   - Numeric change to the range of one weapon type: add an `if (SkillTester(...))` block in the matching `case` of `WeaponRangeGetterSkills` (`RangeGetter.c:50`). No list change is needed.
   - Change that applies to every item, not one type: add it before the `switch` in `WeaponRangeGetterSkills`. Counterattack does this (see Gotchas).
   - Minimum range: edit `GetItemMinRangeRework` (`RangeGetter.c:16`). There is no min-range list.
   - New getter function: add it to `gWeaponRangeGetters` in `Kernel/Wizardry/WeaponRange/data.event` before `StatusGetterCheckCpas`, and check that it has the signature `int (int range, struct Unit *unit, u16 item)`.
   - Staff target rule: edit the list builders in `StaffRange.c` (`TryAddUnitToHealTargetList` `:105`, or the Self Healing blocks `:77`, `:95`).
3. Use `SkillTester(unit, sid)` (see Gotchas, hot path). Use `SKILL_EFF0(sid)` for numbers, and add the entry to `Data/SkillSys/Source/SkillExtraInfo.c`.
4. Update the text and build in game. Not run here.

## Template

Adapted from Staff Savant (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:64-69`). Place the block inside the matching `case` of `WeaponRangeGetterSkills` (the `ITYPE_BOW` case is at `RangeGetter.c:85`). `RangeGetter.c` already includes `skill-system.h` and `constants/skills.h`. No `.event` change is needed for an edit inside an existing getter.

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
		if (SkillTester(unit, SID_Example))
			range = range + SKILL_EFF0(SID_Example);
#endif
```

Matching `SkillExtraInfo.c` entry (inside `Data/SkillSys/Source/SkillExtraInfo.c`, same guard style as `:546-549`):

```c
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = {{ 1 }},
#endif
```

## Worked examples

- `Bow Range +1` / `Bow Range +2` (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:85-93`): additive, `SKILL_EFF0`, bow case only.
- `Point Blank` (`RangeGetter.c:21-24`): sets min range to 1 for bows, in the min-range function.
- `Counterattack` (`RangeGetter.c:53-56`): sets range to the larger map dimension for every item, before the type switch.
- `Saint` and `Aid Refusal` (`Kernel/Wizardry/WeaponRange/Source/StaffRange.c:109-121`): target-list filters for healing staves.

## Gotchas

- Hot path. `GetUnitMinRange` and `GetUnitMaxRange` loop over items and call every getter per item, and `WeaponRangeGetterSkills` calls `SkillTester` up to six times per item (two before the type switch, up to four inside one case). `GenerateUnitCompleteAttackRange` (`MiscHooks.c:287`) calls both plus `GetUnitWeaponReachBits(unit, -1)`, and `DisplayUnitEffectRange` and `GenerateDangerZoneRange` call it per enemy. Use `SkillTester`, as the existing getters do. Do not use `BattleFastSkillTester` here (only valid for `gBattleActor` / `gBattleTarget`, Preamble fact 10). Do not add tests inside the per-tile loops.
- Counterattack probably affects staves too. The block at `RangeGetter.c:53-56` runs before the item-type switch, so it applies to staves as well as weapons. `GetUnitStaffReachBits` (`MiscHooks.c:128`) builds staff reach from `GetItemReachBitsRework`, which calls `GetItemMaxRangeRework`. That would give a Counterattack unit a staff reach of the whole map. Unverified in game.
- Min range has no getter list. Only the two hard-coded checks in `GetItemMinRangeRework` (`RangeGetter.c:16-31`) set min range. Adding a min-range skill to `gWeaponRangeGetters` will not work.
- Running Start reads the global `gActionData.moveCount` (`RangeGetter.c:96-99`). Its value depends on the current action; `DisplayUnitEffectRange` also reads it, in `MovGetter(...) - gActionData.moveCount` (`MiscHooks.c:210`). The text says "+1 attack range", but the code applies it only in the bow case (`:85`).
- Text/code: High Ground text says "bows and tomes". The code applies it to bows (`:101-104`) and anima / light / dark (`:120-123`), which are the tome types. The value 2 matches `SkillExtraInfo.c:566-567`.
- The `status == 0` path (`RangeGetter.c:37-40`) runs before the skill getters. Skill getters can still raise the result, as Counterattack does.
- Thrust (`RangeGetter.c:58-61`) is in the same function, but it is not in `Skills_WeaponRange.txt`. It depends on `PlayStExpa` bits.
- Staff Savant adds to every staff (`RangeGetter.c:64-69`). Staff range for non-heal staves also goes through this function.
- Staff targeting lives in `StaffRange.c`, but staff effects (Bloom cross-shape heal, Cursed Heal damage, Explosive Heal spread) are applied in `Kernel/Wizardry/ItemSys/Source/StaffHeal.c`. A targeting change alone does not change what the staff does.
