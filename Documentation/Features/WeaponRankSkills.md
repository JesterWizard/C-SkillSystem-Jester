# Rank Up, Skill Up

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla FE8 only uses weapon ranks to gate weapons. Three Houses also uses ranks as a skill tree: reaching a letter rank teaches a skill, and later ranks replace earlier levels of the same skill.

C-SkillSys already had Prowess skills whose bonuses scaled with current rank, but they were not granted when the rank was earned. This system teaches skills from a rank table, with Prowess as the default data: **Lv1 at D, Lv2 at C, Lv3 at B, Lv4 at A, Lv5 at S**.

---

## 🛠️ Plan

Weapon-rank skills are a data table, not a hard-coded Prowess special case.

| Rank | Level | Default Prowess bonuses (HIT / Crit Avo, Avo) |
|------|-------|-----------------------------------------------|
| E | — | No Prowess |
| D | Lv1 | +5 / +7 |
| C | Lv2 | +6 / +10 |
| B | Lv3 | +7 / +13 |
| A | Lv4 | +8 / +16 |
| S | Lv5 | +10 / +20 |

### How It Works

`gSkillWRankTable` is indexed by weapon type, then `WPN_LEVEL_*`, then up to two skill slots. Each entry has:

- `sid`: skill to learn
- `replaces`: previous skill to forget and unequip (0 if none)

When a unit's letter rank increases, the kernel walks every newly reached rank in order. For Prowess that means Lv2 forgets Lv1, Lv3 forgets Lv2, and so on, so only the current level occupies an equip slot.

The same pass runs on unit load, autolevel WEXP, promotion, reclass, and Arms Scroll use, so a unit who joins at C already has Prowess Lv2 instead of Lv1.

Prowess bonuses apply only while that weapon type is equipped.

Toggle: `gpKernelDesignerConfig->weapon_rank_skills` in `designer-config.c`.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Rank table | `gSkillWRankTable` in [`SkillTable-wrank.c`](../../Data/SkillSys/Source/SkillTable-wrank.c) | Default Prowess chain per weapon type; extra skills can share a rank slot |
| Learn / replace | `TryAddSkillWRank` / `TryAddSkillWRankRange` in [`WRankSkill.c`](../../Kernel/Wizardry/SkillSys/kernel/WRankSkill.c) | Grants table skills for current or newly reached ranks |
| Unit load | `UnitAutoLoadSkills` in [`LoadSkill.c`](../../Kernel/Wizardry/SkillSys/kernel/LoadSkill.c) | Teaches ranks the unit already has |
| Prep / battle sync | `UpdatePrepEquipSkillList` / `UnitToBattle_SetupSkillList` | Upgrades Prowess for units who already hold a higher rank (saves, autolevel) |
| Battle rank-up | `TryAddSkillWRankFromBattleUnit` in [`WRankSkill.c`](../../Kernel/Wizardry/SkillSys/kernel/WRankSkill.c), called from [`BattleExp.c`](../../Kernel/Wizardry/BattleSys/Source/BattleExp.c) | Replaces the previous Prowess in the unit's equipped skill slots, then shows `[old icon] [old name] upgraded to Lv [new]` after the WRank popup |
| Promotion | `TryAddSkillPromotion` in [`LoadSkill.c`](../../Kernel/Wizardry/SkillSys/kernel/LoadSkill.c) | Grants skills for new class ranks |
| Autolevel WEXP | `UnitAutolevelWExp` in [`AutoLevel.c`](../../Kernel/Wizardry/Lvup/Source/AutoLevel.c) | Grants skills after forced ranks |
| Arms Scroll | `ArmsScroll_ApplyRankUpSkills` in [`ArmsScroll.c`](../../Kernel/Wizardry/SkillSys/ArmsScroll/ArmsScroll.c) | Grants the next Prowess level with the rank-up |
| Combat bonuses | `ApplyProwessBonuses` in [`PreBattleCalc.c`](../../Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c) | HIT / Avo / Crit Avo while using that weapon type |

---

## ✅ TODO

- Person/job rank tables (Combat Art already has `gCombatArtRomPTable` / `gCombatArtRomJTable`)
- Staff Prowess
- Unique skills at specific ranks (Faire, Breaker, and similar)

---

## ⚠️ Limitations & Bugs

- Only the eight vanilla rank-bearing types (`ITYPE_SWORD` through `ITYPE_DARK`) are indexed. Custom types need a table expansion.
- If every equip slot is full, the new skill is still learned and can be equipped in prep, matching other dynamic skills.
- Temporarily forcing S rank (for example Gaiden staff helpers) can grant S-rank table skills if those entries are filled later.
- Please [open an issue](https://github.com/FireEmblemUniverse/fe8u-cskillsys/issues) if you find a bug.
