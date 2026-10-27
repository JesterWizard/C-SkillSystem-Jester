# Combat art skills

> Text file: `Contents/Texts/Source/texts/Skills_CombatArt.txt` (names and descriptions; menu text in `Contents/Texts/Source/texts/CombatArt.txt` and `CombatArt_{Sword,Lance,Axe,Bow}.txt`)  ·  Code: `Kernel/Wizardry/CombatArt/` (engine), `Data/CombatArt/Data/` (tables)  ·  Hook: `PreBattleCalcFuncs` and `BattleDamageCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event`), `gPostActionFuncs` (`Kernel/Wizardry/PostActionHook/data.event`)

## What it is

A combat art is a per-attack option (a `CID_*`) picked during target selection. It changes battle stats, range, double-attack rules, and can add a unit-status debuff on hit. Arts come from three sources: a weapon table, a per-character / per-class rank table, or a skill (`SID_COMBAT_*`) that is looked up in `gCombatArtSkillTable`. Only the skill route is a "combat art skill" in the sense of `Skills_CombatArt.txt`.

Current build state: no `SID_COMBAT_*` is defined. All 27 entries are commented out in `include/constants/skills-equip.enum.txt:915-942`, and `include/constants/skills.h` (generated from that file by `makefile:402-404`) has none. Every `#if defined(SID_COMBAT_*)` block is therefore compiled out, including the table rows in `Data/CombatArt/Data/CombatArtSkill.c`. Only weapon-table and rank-table arts currently work.

## How the engine calls it

1. Candidate list: `GetCombatArtList(unit, item)` (`Kernel/Wizardry/CombatArt/Source/CombatArtList.c:98`) calls `CalcCombatArtListExt` (line 11). Candidates come from:
   - each skill of the unit via `gpCombatArtSkillTable[sid]` (`CombatArtList.c`, skill loop near the top);
   - skills tied to the equipped item via `gpConstSkillTable_Weapon`;
   - `gpCombatArtWeaponTable[ITEM_INDEX]` (`Data/CombatArt/Data/CombatArtWeapon.c:5`);
   - weapon-rank tables `gpCombatArtDefaultTable`, `gpCombatArtRomPTable[pid]`, `gpCombatArtRomJTable[jid]` (`Data/CombatArt/Data/RomTables.c`), per sword / lance / axe / bow rank.
   Filters: `wtype` must match unless `CA_WTYPE_ANY`; `ITEM_USES(item) >= cost`; list capped at `COMBART_LIST_MAX_AMT` (`include/kernel/combat-art.h:100`, value 6).
2. Selection: `ActionMenu.c:42,115`, `ItemSelectMenu.c:48,82` and `SelectTarget.c:79` call `RegisterCombatArtStatus(unit, cid)` (`Kernel/Wizardry/CombatArt/Source/CombatArtStatus.c:18`). This stores `uid` and `cid` in `gCombatArtStatus`. `GetCombatArtInForce(unit)` (`CombatArtStatus.c:7`) returns the cid only when the uid matches.
3. Pre-battle: `PreBattleCalcCombatArt` (`Kernel/Wizardry/CombatArt/Source/MiscHooks.c:93`) is in `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:9`). It adds `battle_status` (atk, def, hit, avo, crit, silencer, dodge) and has a `switch (cid)` special case (line 114, Soulblade adds the actor's RES).
4. Damage: `BattleDamageCalc_CombatArt` (`MiscHooks.c:198`) is in `BattleDamageCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:18`). It switches on the in-force cid for Detonate, BloodTribute and CrimsonStrike. The last two are behind `SID_COMBAT_*` guards and are compiled out now.
5. Post-action: `PostAction_CombatArtEffect` (`Kernel/Wizardry/CombatArt/Source/PostActionHooks.c:50`) is in `gPostActionFuncs`. If the art has `debuff != UNIT_STATUS_NONE` and the attack hit, `ExecCombatArtEffectAnim` runs. It calls `SetUnitStatusExt(tunit, info->debuff, 1)` (`PostActionHooks.c:38`) on every target in the 1x1 (or 2x2 when `aoe_debuff`) area.

## Where to look

| File | Role |
| :--- | :--- |
| `include/constants/combat-arts.h` | `enum CombatArtIndex` (`CID_Default` = 0 ... `CID_Practice`). Indexes every CID table. |
| `include/kernel/combat-art.h` | `COMBART_VALID` (line 7), `struct CombatArtInfo` (line 10), `gCombatArtInfos[0x100]` extern (line 59), list struct and `COMBART_LIST_MAX_AMT` (100), table externs. |
| `include/constants/skills-equip.enum.txt` | `SID_COMBAT_*` definitions. Lines 915-942, currently commented out. |
| `Data/CombatArt/Data/CombatArtInfo.c` | `gCombatArtInfos`: name, desc, icon, wtype, cost, `battle_status`, effectiveness, magic_attack, debuff, aoe_debuff. |
| `Data/CombatArt/Data/CombatArtSkill.c` | `gCombatArtSkillTable[]`: `SID_COMBAT_*` to `CID_*`. |
| `Data/CombatArt/Data/CombatArtWeapon.c` | `gCombatArtWeaponTable[]`: item to `CID_*` (e.g. `ITEM_DARK_NOSFERATU` gives `CID_BloodTribute`). |
| `Data/CombatArt/Data/RomTables.c` | `gCombatArtRomPTable` (per character, line 5), `gCombatArtRomJTable` (per class, line 19), `gCombatArtDefaultTable` (line 46). |
| `Data/CombatArt/CombatArt.event` | Includes the table `.lyn.event` files and BKSEL graphics. |
| `Kernel/Wizardry/CombatArt/Source/CombatArtList.c` | List builder. |
| `Kernel/Wizardry/CombatArt/Source/CombatArtStatus.c` | `GetCombatArtInForce`, `RegisterCombatArtStatus`. |
| `Kernel/Wizardry/CombatArt/Source/MiscHooks.c` | `PreBattleCalcCombatArt` (93), `WeaponRangeGetterCombatArt`, `BattleDamageCalc_CombatArt` (198). |
| `Kernel/Wizardry/CombatArt/Source/PostActionHooks.c` | Debuff on hit (`PostAction_CombatArtEffect`, 50). |
| `Kernel/Wizardry/CombatArt/CombatArtMenu/*.c` | Menus that build the list and call `RegisterCombatArtStatus`. |
| `Kernel/Wizardry/CombatArt/Source/SelectTarget.c` | Target selection for arts. |
| `Contents/Texts/Source/texts/Skills_CombatArt.txt` | `MSG_SKILL_COMBAT_<Name>_NAME` and `MSG_SKILL_COMBAT_<Name>_DESC`, used by `CombatArtInfo.c`. |
| `Patches/PATCH_Arts_Skill.txt` | FEBuilder-side description of `gCombatArtSkillTable`. |
| `Documentation/Features/CombatArt.md` | Editor-side (FEBuilder) guide to the art table. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md). Use `SID_COMBAT_<Name>` for the skill and `CID_<Name>` for the art.
2. Add `CID_<Name>,` to `enum CombatArtIndex` in `include/constants/combat-arts.h`, in the group that matches its weapon. Must stay below `0xFF` (`COMBART_VALID`).
3. Add `[CID_<Name>] = { ... }` to `gCombatArtInfos` in `Data/CombatArt/Data/CombatArtInfo.c`. Fields: `.name`, `.desc` (`MSG_SKILL_COMBAT_<Name>_DESC`), `.icon` (existing examples: `GFX_CombatArtIcon_SwordArtAtk`), `.wtype` (`ITYPE_*` or `CA_WTYPE_ANY`), `.cost`, `.battle_status.*`, `.effectiveness`, `.magic_attack`, `.double_attack`, `.debuff`, `.aoe_debuff`.
4. Add the name and description to `Contents/Texts/Source/texts/Skills_CombatArt.txt`.
5. Grant it, choosing one or more of:
   - by skill: uncomment or add `SID_COMBAT_<Name>` in `include/constants/skills-equip.enum.txt` (the COMBAT ARTS block), then add `[SID_COMBAT_<Name>] = CID_<Name>,` to `Data/CombatArt/Data/CombatArtSkill.c` under the guard. The skill also needs its normal `SkillInfo.c` entry, icon and `Skills_Price.h` / `Skills_Capacity.h` defines (see the common steps);
   - by weapon: `[ITEM_<X>] = CID_<Name>,` in `Data/CombatArt/Data/CombatArtWeapon.c`;
   - by rank: entries in `gCombatArtDefaultTable`, `gCombatArtRomPTable` or `gCombatArtRomJTable` in `Data/CombatArt/Data/RomTables.c`.
6. If the art needs logic beyond the table, add a case to the `switch (cid)` in `PreBattleCalcCombatArt` (`MiscHooks.c:114`) or to the switch in `BattleDamageCalc_CombatArt` (`MiscHooks.c:198`). Guard the case with the same `SID_COMBAT_*` check if the logic depends on the skill.

## Template

Adapted from `Data/CombatArt/Data/CombatArtInfo.c` (`CID_WrathStrike` and `CID_Grounder` entries) and `Data/CombatArt/Data/CombatArtSkill.c`.

```c
/* include/constants/combat-arts.h, inside enum CombatArtIndex */
CID_Example,

/* Data/CombatArt/Data/CombatArtInfo.c, inside gCombatArtInfos[0x100] */
[CID_Example] = {
    .name = MSG_SKILL_COMBAT_Example_NAME,
    .desc = MSG_SKILL_COMBAT_Example_DESC,
    .icon = GFX_CombatArtIcon_SwordArtAtk,
    .wtype = ITYPE_SWORD,
    .cost = 4,
    .battle_status.atk = 5,
    .battle_status.hit = 20,
    .effectiveness = COMBART_EFF_FLIER,
},

/* Data/CombatArt/Data/CombatArtSkill.c, inside gCombatArtSkillTable */
#if defined(SID_COMBAT_Example) && (COMMON_SKILL_VALID(SID_COMBAT_Example))
    [SID_COMBAT_Example] = CID_Example,
#endif
```

Also uncomment or add in `include/constants/skills-equip.enum.txt` (inside the COMBAT ARTS block):

```
SID_COMBAT_Example
```

Note: `SID_COMBAT_Example` must also exist in `skills.h` (regenerated by the build) for the `#if` to pass.

## Worked examples

- `CID_Hexblade` via `ITEM_SWORD_SLIM` (`Data/CombatArt/Data/CombatArtWeapon.c:6`): weapon-granted, `magic_attack = true`, `battle_status.atk`.
- `CID_BreakShot` (`Data/CombatArt/Data/CombatArtInfo.c:361`, `.debuff = NEW_UNIT_STATUS_PIERCE_ARMOR` at 369) and `CID_WaningShot` (line 382, `.debuff = NEW_UNIT_STATUS_WEAKEN` at 391). Applied in `PostAction_CombatArtEffect`.
- `CID_Soulblade` (`CombatArtInfo.c`, line 30): `magic_attack`, plus the special case in `PreBattleCalcCombatArt` that adds the actor's RES (`MiscHooks.c:114-116`).
- `CID_BloodTribute` via `ITEM_DARK_NOSFERATU` (`CombatArtWeapon.c:8`): HP-cost art; the damage hook is in `MiscHooks.c` behind `SID_COMBAT_BloodTribute`, so it is inactive now.

## Gotchas

- All `SID_COMBAT_*` are commented out in `include/constants/skills-equip.enum.txt:915-942`, so none exist in `skills.h` and every `SID_COMBAT_*` guard is false. Skill-granted arts and their special effects do nothing in the current build. Verify with `grep -c "#define SID_COMBAT" include/constants/skills.h` (returns 0).
- Skill SIDs must be below 0x400 (`COMMON_SKILL_VALID`, `include/kernel/skill-system.h:90`). Put new SIDs in `skills-equip.enum.txt`. `skills-class`, `skills-person` and `skills-item` enums are not built (`makefile:405-411` commented out), so a `SID_COMBAT_*` line there has no effect.
- `gCombatArtSkillTable` is indexed by SID (`MAX_SKILL_NUM + 1`), so the table size follows the skill range.
- Weapon filter: arts with `wtype` other than `CA_WTYPE_ANY` are dropped when the equipped weapon type differs (`CombatArtList.c`, the `wtype` check in the final loop).
- Cost filter: an art is dropped when `ITEM_USES(item) < cost`, so the list shrinks as durability falls.
- Debuffs use the unit-status slot. `SetUnitStatusExt` overwrites whatever status the target already had (see `Documentation/SkillTypes/Debuff.md`).
- Text typos in `Skills_CombatArt.txt` (cosmetic): "Effectiv against armored units." (Helm Splitter), "Inllict the status" (Break Shot+), "calvary" (Knight Kneeler), "Acts as  magic attack." (Soul Blade, double space).
- Effects of the "misc" arts (`CID_Galeforce`, `CID_Gamble`, `CID_Practice`, `CID_Smash`, `CID_SwiftStrikes` and others with `SID_COMBAT_*` guards) were not traced in code. Unverified.

