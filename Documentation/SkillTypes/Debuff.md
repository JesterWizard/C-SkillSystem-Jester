# Debuff and status skills

> Text file: `Contents/Texts/Source/texts/Skills_Debuff.txt`  ·  Code: `Kernel/Wizardry/DebuffRework/` (engine), `Kernel/Wizardry/SkillEffects/PrePhaseSkills/`, `Kernel/Wizardry/SkillEffects/PostActionSkills/SealSkills.c`  ·  Hook: `gPrePhaseFuncs` (`Kernel/Wizardry/PrePhaseHook/data.event`), `gPostActionFuncs` (`Kernel/Wizardry/PostActionHook/data.event`), `gPowGetters` and the other stat getter lists (`Kernel/Wizardry/UnitStatusGetter/data.event`), `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event`)

## What it is

Skills that write debuff or buff state onto units. Covered here:

- Aversa's Night (`SID_AversaNight`): pre-phase. Sets Panic plus a stat debuff on enemies in an ally's 2x2 when their HP is low.
- Hone / Fortify (`SID_PowHone` ... `SID_FortifyArmor`, `SID_HoneChaos`): pre-phase. Sets stat buffs on allies in the user's 2x2.
- Chill (`SID_ChillPow` ... `SID_ChillRes`): pre-phase. Debuffs the enemy with the highest stat until that enemy acts.
- Seal (`SID_SealStrength` ... `SID_SealDefense`): post-action. Debuffs the other combatant by -6 after combat.
- Magic Seal / Physical Seal / Full Metal Body (`SID_MagicSeal`, `SID_PhysicalSeal`, `SID_FullMetalBody`): not hooks. Area checks queried by the engine when a unit uses an item. Full Metal Body only blocks the Seal group (see Gotchas).

There are two storage layers. Use the right one:

1. Stat debuff bitmask: many bits can be set at once, each bit adds fixed stat deltas. `SetUnitStatDebuff` / `ClearUnitStatDebuff` / `CheckUnitStatDebuff`. Values in `Data/Debuff/StatDebuffInfo.c`.
2. Unit status: one slot per unit (index plus duration). `SetUnitStatusExt` / `SetUnitStatusIndex`. Table in `Data/Debuff/DebuffInfo.c`. Panic, Heavy Gravity, Weaken and the other `NEW_UNIT_STATUS_*` entries live here.

## How the engine calls it

Pre-phase (list `gPrePhaseFuncs`, `Kernel/Wizardry/PrePhaseHook/data.event:22-24`):

- `PrePhaseFunc_AversaNight` at `Kernel/Wizardry/SkillEffects/PrePhaseSkills/AversaNight.c:7`. Tests `SkillTester(unit, SID_AversaNight)` (line 17). Per target: `SetUnitStatusIndex(tunit, NEW_UNIT_STATUS_PANIC)` and `SetUnitStatDebuff(tunit, UNIT_STAT_DEBUFF_AversaNight)` (lines 55-56). Threshold is `SKILL_EFF0(SID_AversaNight)` (`Data/SkillSys/Source/SkillExtraInfo.c:799`, value 3).
- `PrePhaseFunc_HoneFortify` at `Kernel/Wizardry/SkillEffects/PrePhaseSkills/HoneFortify.c:9`. Per unit on the phase faction, tests each Hone/Fortify SID with `SkillTester`. `SID_HoneChaos` picks one stat at random with `NextRN_N(8)` (around line 33). Job-gated buffs use `CheckClassCavalry` / `CheckClassFlier` / `CheckClassArmor` and `CheckSameClassType` (lines 197, 202). Targets are the allies in `gVecs_2x2` around the user. Each applies `SetUnitStatDebuff(tunit, UNIT_STAT_BUFF_*)`.
- `PrePhaseFunc_Chill` at `Kernel/Wizardry/SkillEffects/PrePhaseSkills/ChillSkills.c:290`. Starts `ProcScr_PrePhaseChill`. `PrePhaseChill_CollectActors` finds, for each of the 7 stats, the unit with the Chill SID. `PrePhaseChill_CollectTargets` finds the enemy with the highest value of that stat. `PrePhaseChill_ApplyDebuff` calls `SetUnitStatDebuff` with `ChillDebuffList[]`.

Post-action (list `gPostActionFuncs`, `Kernel/Wizardry/PostActionHook/data.event:18` and `:36`):

- `PostAction_SealSkills` at `Kernel/Wizardry/SkillEffects/PostActionSkills/SealSkills.c:14`. Returns early unless `gActiveUnit` is the subject or target (lines 21-22). Returns early if the target has `SID_FullMetalBody` (line 25). For each Seal SID, applies `SetUnitStatDebuff` to the other combatant (`isSubject` decides which one). Seal Resistance uses `SkillTester` (line 60); the other six use `SkillListTester`.
- `PostAction_RemoveChillDebuff` at `Kernel/Wizardry/SkillEffects/PrePhaseSkills/ChillSkills.c:296`. Clears all 7 Chill bits on `gActiveUnit`.

Read path (per unit, every time a stat is read):

- Stat getters `PowGetterStatDebuff`, `MagGetterStatDebuff`, ... (`Kernel/Wizardry/DebuffRework/StatDebuff/StatDebuff.c:393` onward) are listed in `Kernel/Wizardry/UnitStatusGetter/data.event` (e.g. `PowGetterStatDebuff`, line 13). They add `unit_status` values from `GetStatDebuffMsgBuf` (`StatDebuff.c:368`), which sums every set bit.
- Battle: `PreBattleCalcStatDebuffs` (`StatDebuff.c:268`) is in `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:11`) and sums `battle_status`.

Tick and lifetime:

- `TickActiveFactionTurn` at `Kernel/Wizardry/DebuffRework/Source/StatusDepletion.c:26` (`LYN_REPLACE_CHECK`). Calls `TickUnitStatDebuff(unit, STATUS_DEBUFF_TICK_ON_ALLY)` for the active faction (line 87) and `STATUS_DEBUFF_TICK_ON_ENEMY` for the red faction (line 110). `TickUnitStatDebuff` is at `StatDebuff.c:248`.
- `tick_type` per bit (`Data/Debuff/StatDebuffInfo.c`): `STATUS_DEBUFF_NO_TICK` bits are never cleared by the tick. Chill is `STATUS_DEBUFF_NO_TICK` and cleared by `PostAction_RemoveChillDebuff`.
- Unit copy / battle snapshot / clear: `StatDebuff_OnUnitToBattle` and `StatDeuff_OnClearUnit` (`Kernel/Wizardry/UnitHooks/data.event:24` and `:34`). Save chunk `MSU_SaveStatDebuff` (`Kernel/Wizardry/SaveData/data.event:107`).

Magic Seal / Physical Seal (area checks, not hooks):

- Magic: `IsPositionMagicSealedSlow` / cache in `Kernel/Wizardry/UnitStatusGetter/source/MiscGetter.c` (around line 121). Range is `RECT_DISTANCE <= 10`. `IsUnitMagicSealed` also returns true when `statusIndex == UNIT_STATUS_SILENCED`.
- Physical: `IsUnitPhysicalSealed` (`MiscGetter.c`, around line 230). Consumers: `Kernel/Wizardry/BattleSys/Source/BattleWeapon.c:562-563` (blocks non-magic weapons) and `Kernel/Wizardry/GaidenMagic/Source/Core.c:194-198`.

## Where to look

| File | Role |
| :--- | :--- |
| `Contents/Texts/Source/texts/Skills_Debuff.txt` | Skill names and descriptions (`MSG_SKILL_<Name>_NAME` and `MSG_SKILL_<Name>_DESC`). |
| `Contents/Texts/Source/texts/Debuff.txt` | Names of unit-status debuffs shown in the UI (`MSG_DEBUFF_*`). |
| `include/kernel/debuff.h` | `enum UNIT_STAT_DEBUFF_IDX` (line 196), `UNIT_STAT_DEBUFF_MAX_REAL` (303), `UNIT_STAT_DEBUFF_MAX = 128` (304), `struct StatDebuffStatus` (310), `enum UNIT_STATUS_IDENTIFIER` (18), `NEW_UNIT_STATUS_MAX = 64` (65), `struct DebuffInfo` (129), prototypes (365-367). |
| `Data/Debuff/StatDebuffInfo.c` | `gStatDebuffInfos`: per-bit `positive_type`, `tick_type`, `unit_status`, `battle_status`. Seal bits at 341 onward, AversaNight at 375, Chill at 498, Hone buffs from 65. |
| `Data/Debuff/DebuffInfo.c` | `gDebuffInfos[NEW_UNIT_STATUS_MAX]`: unit-status table (Panic at 186). |
| `Kernel/Wizardry/DebuffRework/StatDebuff/StatDebuff.c` | Set/Clear/Check (125, 139, 153), tick (248), battle calc (268), message buffer (368), getters (393-553), `Assert` on enum size (638). |
| `Kernel/Wizardry/DebuffRework/Source/Core.c` | `SetUnitStatusIndex` (27). |
| `Kernel/Wizardry/DebuffRework/Source/MiscHooks.c` | `SetUnitStatus` (16), `SetUnitStatusExt` (60). |
| `Kernel/Wizardry/DebuffRework/Source/StatusDepletion.c` | `TickActiveFactionTurn` (26). |
| `Kernel/Wizardry/SkillEffects/PrePhaseSkills/AversaNight.c` | Aversa's Night. |
| `Kernel/Wizardry/SkillEffects/PrePhaseSkills/HoneFortify.c` | Hone, Fortify, Hone Chaos. |
| `Kernel/Wizardry/SkillEffects/PrePhaseSkills/ChillSkills.c` | Chill (7 stats) and its removal. |
| `Kernel/Wizardry/SkillEffects/PostActionSkills/SealSkills.c` | Seal (7 stats) and Full Metal Body check. |
| `Kernel/Wizardry/UnitStatusGetter/source/MiscGetter.c` | Magic and Physical seal area checks. |
| `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event` | Includes the `.lyn.event` for each file above (AversaNight 142, HoneFortify 143, SealSkills 15, ChillSkills 152). |
| `Kernel/Wizardry/PrePhaseHook/data.event`, `Kernel/Wizardry/PostActionHook/data.event` | Hook lists. |
| `Data/SkillSys/Source/SkillExtraInfo.c` | `SKILL_EFF0` values, e.g. `[SID_AversaNight] = {{ 3 }}` (line 799). |
| `include/constants/skills-equip.enum.txt` | SID definitions (all Seal/Hone/Chill/Aversa SIDs are here). |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Decide the layer:
   - Stacking stat delta: add `UNIT_STAT_DEBUFF_<Name>` to `enum UNIT_STAT_DEBUFF_IDX` in `include/kernel/debuff.h`, before `UNIT_STAT_DEBUFF_MAX_REAL`. Keep the count under 128 (`Assert` at `StatDebuff.c:638`). Add `[UNIT_STAT_DEBUFF_<Name>] = { .positive_type = ..., .tick_type = ..., .unit_status = { ... } }` to `Data/Debuff/StatDebuffInfo.c`. Use `STATUS_DEBUFF_TICK_ON_ENEMY` or `STATUS_DEBUFF_TICK_ON_ALLY` for automatic expiry, or `STATUS_DEBUFF_NO_TICK` and clear it yourself.
   - Single status: add `NEW_UNIT_STATUS_<Name>` to `enum UNIT_STATUS_IDENTIFIER` in `include/kernel/debuff.h` (the count is capped at `NEW_UNIT_STATUS_MAX = 64`) and an entry in `Data/Debuff/DebuffInfo.c`.
3. Write the effect function in `Kernel/Wizardry/SkillEffects/PrePhaseSkills/<Name>.c` (pre-phase) or `PostActionSkills/<Name>.c` (post-action). Wrap the body in `#if defined(SID_<Name>) && (COMMON_SKILL_VALID(SID_<Name>))`.
4. Register it in `Kernel/Wizardry/PrePhaseHook/data.event` or `Kernel/Wizardry/PostActionHook/data.event`. Keep the `//End` comment line last.
5. Add `#include "PrePhaseSkills/<Name>.lyn.event"` (or `PostActionSkills/...`) to `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event`.
6. Prototype in `include/kernel/skill-system.h` if other files call it.
7. Text: `MSG_SKILL_<Name>_NAME` and `MSG_SKILL_<Name>_DESC` in `Skills_Debuff.txt`. If a new unit-status name is shown, add `MSG_DEBUFF_*` to `Debuff.txt`.
8. If the effect must end at a specific point (Chill is removed on post-action), add a `ClearUnitStatDebuff` call there.

## Template

Adapted from `Kernel/Wizardry/SkillEffects/PostActionSkills/SealSkills.c` (the `SID_SealDefense` block). Needs `UNIT_STAT_DEBUFF_Example` added to the enum and `StatDebuffInfo.c` (step 2).

```c
/* Kernel/Wizardry/SkillEffects/PostActionSkills/Example.c */
#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "constants/skills.h"

/* Registered in gPostActionFuncs (Kernel/Wizardry/PostActionHook/data.event):
 *     POIN PostAction_Example
 */
bool PostAction_Example(ProcPtr parent)
{
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
    struct Unit *subject = GetUnit(gActionData.subjectIndex);
    struct Unit *target  = GetUnit(gActionData.targetIndex);
    bool isSubject = (gActiveUnit == subject);
    bool isTarget  = (gActiveUnit == target);

    if (!isSubject && !isTarget)
        return false;

    if (SkillListTester(gActiveUnit, SID_Example)) {
        if (isSubject)
            SetUnitStatDebuff(target, UNIT_STAT_DEBUFF_Example);
        else
            SetUnitStatDebuff(subject, UNIT_STAT_DEBUFF_Example);
    }
#endif

    return false;
}
```

Also add to `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event`:

```
#include "PostActionSkills/Example.lyn.event"
```

Also add `POIN PostAction_Example` in `Kernel/Wizardry/PostActionHook/data.event` before `POIN ResetComboAtkList`.

## Worked examples

- Aversa's Night: `Kernel/Wizardry/SkillEffects/PrePhaseSkills/AversaNight.c` - pre-phase, sets a unit-status (Panic) and a stat debuff; threshold from `SKILL_EFF0`. See Gotchas for an unresolved bug.
- Hone / Fortify: `Kernel/Wizardry/SkillEffects/PrePhaseSkills/HoneFortify.c` - ally buffs over a 2x2, random stat for Hone Chaos, job-gated Hone Cavalry / Flier / Armor and Fortify Armor.
- Chill: `Kernel/Wizardry/SkillEffects/PrePhaseSkills/ChillSkills.c` - per-stat target selection, a proc with camera and animation, removal in `PostAction_RemoveChillDebuff`.
- Seal: `Kernel/Wizardry/SkillEffects/PostActionSkills/SealSkills.c` - post-combat debuff on the other combatant, Full Metal Body early return.
- Magic / Physical Seal: `Kernel/Wizardry/UnitStatusGetter/source/MiscGetter.c` - area query instead of a debuff bit; consumed by `Kernel/Wizardry/BattleSys/Source/BattleWeapon.c`.

## Gotchas

- Two layers, two APIs. `SetUnitStatDebuff` (bit, stacks) is not `SetUnitStatusExt` (single slot, has duration). Aversa's Night uses both.
- `SetUnitStatusExt` overwrites the target's current unit status: it calls `SetUnitStatusIndex` then `SetUnitStatusDuration` (`Kernel/Wizardry/DebuffRework/Source/MiscHooks.c:60-72`). A second status on the same unit replaces the first.
- Aversa's Night as written never applies (static reading, not run). `max_hp` starts at 0 (`AversaNight.c:10`) and the only update is `if (max_hp > GetUnitMaxHp(unit))` (lines 20-21), which is never true. The check `tunit->curHP >= (max_hp - SKILL_EFF0(...))` (line 40) is then always true, so every target is skipped. The description says "enemy's HP < unit's HP", which the code does not implement either.
- Seal Defense bit is `STATUS_DEBUFF_NO_TICK` (`Data/Debuff/StatDebuffInfo.c:341-345`). Nothing in the repo calls `ClearUnitStatDebuff` for `UNIT_STAT_DEBUFF_SEAL_DEF`. The only clears are the Chill bits (`ChillSkills.c`) and the Transform / Laguz buff bits (`Kernel/Wizardry/UnitHooks/Source/UnitRefrain.c:119-134`). Static reading: it may persist until the unit is cleared. Check in game.
- Full Metal Body only blocks the post-action Seal group (`SealSkills.c:24-25`). The repo has no other `SID_FullMetalBody` reference, so Magic Seal and Physical Seal (area checks in `MiscGetter.c`) do not honour it. Text says "immune to all the seal skills".
- Seal Resistance uses `SkillTester` while the other six Seals use `SkillListTester` (`SealSkills.c:60` vs `:30`). Weapon- or forge-granted copies of the skill are only seen by `SkillListTester`.
- `PostAction_SealSkills` is declared `void` (`SealSkills.c:14`) but `gPostActionFuncs` is typed `bool (*)(ProcPtr)` (`PostActionHook/data.event:1`). Effect of this mismatch is unverified.
- Hone Cavalry text says "+6 ATK/SPD". `UNIT_STAT_BUFF_JOB_HONE` gives `pow`, `mag`, `skl`, `spd` +6 (`StatDebuffInfo.c`, around 356). Text and data disagree (SKL is included).
- Job Hone / Fortify is applied only when `CheckSameClassType(UNIT_CLASS_ID(unit), UNIT_CLASS_ID(tunit))` is true (`HoneFortify.c:197`, `:202`). Text says "Cavalry allies", which is not what the code checks. Semantics of `CheckSameClassType` are not traced here.
- Chill is cleared by `PostAction_RemoveChillDebuff` on `gActiveUnit` only (`ChillSkills.c:296-304`). A chilled enemy keeps the debuff until it is the active unit at post-action. Read from code, not played.
- Text key vs SID: `MSG_SKILL_Aversa_*` belongs to `SID_AversaNight` (`Data/SkillSys/Source/SkillInfo.c:3321-3322`). Grep for `SID_AversaNight` in code.
- The clear / load / copy hooks are spelt `StatDeuff_OnClearUnit`, `StatDeuff_OnLoadUnit` and `StatDeuff_OnCopyUnit` (the prefix is missing a `b`). Grep with that spelling.
- Stat-debuff bits live in `sStatDebuffStatusAlly` / `Enemy` / `Npc` (and `Fourth` with `CONFIG_FOURTH_ALLEGIANCE`). The lookup is `gpStatDebuffStatusPool[unit->index & 0xFF]` (`StatDebuff.c:16`, pool table in `Kernel/Wizardry/DebuffRework/StatDebuff/StatDebuffData.c`, pointer in `Kernel/Preload/KernelReloc.event:66`). Go through `GetUnitStatDebuffStatus`, not the arrays.
