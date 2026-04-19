# Limitations

This page summarizes the current hard limits and compatibility risks in C-SkillSys. Read it before stacking additional patches or changing low-level unit and battle behavior.

## Current Limits

1. Ally character count is limited to **51**.

Make sure the character ID also stays within the supported range. Units outside that range may fail to learn or equip skills.

1. Vanilla move-path display supports no more than 20 steps.

If unit mobility increases beyond that practical limit, move-path display can overflow and crash the game. If this becomes a problem, disable move-path through the FEBuilder [designer config patch](../Patches/PATCH_DesignerConfig.txt).

1. `BattleUnit` is only valid for `gBattleActor` and `gBattleTarget`.

Other battle-unit pointers are not safe to use. In vanilla, the only code path that allocates an extra battle unit is `UnitAutolevelRealistic`, and that path has already been rewritten in the kernel.

1. A single `LOAD<x>` call can load no more than **20** units.

If you need more, split the load across multiple `LOAD<x>` commands.

## Known FEBuilder Conflicts

The following FEBuilderGBA patches are known to conflict directly with C-SkillSys and should be treated as forbidden unless you have explicitly resolved the conflict yourself.

- Talk AI
- Anima Triangle
- AoE Area of Effect
- Status Ailment Swords
- Base stat based promotion for reclassing
- When calculating the attack speed, calculate with CON + Skill / 4
- Change from death to HP1 with Lethality
- Use magic motion when physical attack hits
- Fixed Exp value of staff
- Increase the RAM area that can record the number of clear turns
- CritCap. Fixed critical from appearing in excess of 100%
- Con-Reducing Diet Drug
- Limit Weapon Rank Display
- Multi-Class Pick Skill Installer
- Exclude buff states from terrain recovery
- Remove Creature Campaign
- Remove Link Arena Option
- Remove Sound Room
- Remove Support Viewer
- Fates EXP
- Attack and capture enemies with reduced speed like FE5
- Rogue Robbery
- Staff Range Fix
- FixedGrowthsMode
- Fourth-Allegiance
- Range Display Fix
- Change weapon's range text
- HeroesMovement
- Increase Enemy's hit rate
- Icon Display
- Lose Weapon Ranks on Promotion
- Magic Sword
- Magic Sword Rework
- MapAddInRange Efficiency Fix
- MeleeAndMagicFix
- Fix Weak Promoted Enemies
- Define Multiple Weapons That Cannot Double Attack
- Character/Class: Level Cap Editor
- Change to have multiple units that can call supply
- Passive Boosts Patches
- Change Max HP limit to use set in class
- Staff_Heal_Exp
- Simple CANTO Fix
- EXP Value in dance, steal, Summons, etc
- Allow enemy growths over 100%
- Staff Basal Hit Value
- Battle crit threshold
- Status given by Filla's Might
- Status given by Ninis's Grace
- Status given by Set's Litany
- Status given by Thor's Ire
- Great Shield Activation Rate
- Great Shield Damage Reduction
- Great Shield First Class
- Great Shield Last Class
- Silencer Activation Rate vs Boss
- Silencer Activation Rate vs Exp 0
- Silencer Activation Rate
- Silencer-Immune Class
- Slayer Skill First Class
- Slayer Skill Last Class
- Sure Strike Skill First Class
- Sure Strike Skill Last Class
- Stat Bar Max Length
- Modify Maximum Number Of Supports
- Skill Pick
- Skill Pierce from this class
- Skill Pierce to this class
- Turnwheel
- ExModularSave

This list is not exhaustive. A patch not listed here is not automatically safe.

## High-Risk Patch Categories

Be especially careful with patches that modify:

- Unit status calculation
- Battle status calculation
- EXP calculation
- Staff logic
- Ring logic
- Skill logic
- Save data

## Unit Level Changes in ASM

Do **not** directly change unit level in ASM.

Level-up history is stored in the BWL table. If you change unit level directly, the recorded level can desync from the actual level and skill-learning logic may stop behaving correctly. See the Learn Skills section in [SkillSys.md](./SkillSys.md).

Use the kernel API instead:

```c
// bwl.h
void WriteUnitLevelSafe(struct Unit *unit, int new_level);
```
