# Abstract

This page is a high-level overview of the main systems included in C-SkillSys, with links to the detailed documentation for each feature.

## Installation

Choose one of the following installation methods.

1. FEBuilder LTS setup: download [the latest FEBuilder release (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master) and install C-SkillSys as a patch.

1. Release build setup: download the UPS patch or EA buildfile from the [release page](https://github.com/FireEmblemUniverse/fe8u-cskillsys/releases) and apply it to your ROM.

   - [Patch ROM via UPS](https://feuniverse.us/t/how-to-patch-a-rom-v5/10329)
   - [EA buildfile tutorial](https://tutorial.feuniverse.us/)

1. Source build setup: follow the [custom build guide](./CustomBuild.md).

> [!WARNING]
>
> 1. It is strongly recommended to install C-SkillSys on a clean FE8 ROM.
> 2. Read the [Limitations](./Limitations.md) document before adding further hacks.
> 3. If you use the release build or source build options, manually place the completed `Patches` directory into `FEBuilderGBA\config\patch2\FE8U`.

## Systems

## Skill System

- [Skills glossary](./SkillInfo.md)
- [SkillSystem](./SkillSys.md)

The project includes a large skill framework with more than 800 implemented skills.

- Skills can be learned and equipped from the prep screen, depending on designer configuration.
- Skill scrolls are supported.
- Skill activation animations work in both battle animations and map animations.

## Combat Art

Combat Arts add specialized combat options that trade normal attack behavior for stronger bonuses or extra effects.

- [Combat Art documentation](./CombatArt.md)

## Battle Calculation

The battle system changes several core combat formulas.

- STR and MAG are split.
- Attack speed decay changes from `weight - con` to `weight - (con + atk * 20%)`.
- Nosferatu HP drain is reduced from 100% to 50%.
- Critical damage is 300% by default, but skills can modify it.
- Effectiveness damage is 200% by default, but skills can modify it.
- Skills can apply real damage that bypasses normal vanilla damage calculation.
- Mounted units can suffer `avo -20%` indoors.

## Weapon

Weapons with **S rank** gain extra value.

- Units using an S-rank weapon gain `atk +1`.
- S-rank weapons also ignore attack speed decay.

## Combo Attack

When a unit attacks an enemy while an ally is in supporting range, that ally can join the combat as a combo attacker.

## Surrounder

Surrounder adds penalties for being boxed in by enemies.

- For each adjacent enemy side, the unit suffers `avo -10%`.
- If all four sides are occupied by enemies, the unit also suffers `def -5`.
- Fliers outdoors are not affected by this system.

## Ranged Attack

For non-ballista combat, hit rate falls as attack distance increases.

- Hit is reduced by `10%` for each point of distance.

## Convoy

The convoy capacity is expanded to 200 items.

## Suspend

![image](./Images/Home_Suspend.png)

Suspend data is automatically saved only at the start of player phase.

This creates a simple undo-style checkpoint, and the behavior can be configured at runtime.

## Gaiden Style Magic

Gaiden-style black and white magic are supported, allowing units to spend HP to cast spells.

## Debuff

Debuffs are supported for both general and combat-specific effects.

- [Debuff documentation](./Debuff.md)

- Basic debuffs last no more than 3 turns.
- Some combat-related debuffs can stack or coexist, but last only 1 turn.

## Shield

Shield items are documented separately.

- [Shield documentation](./ShieldItem.md)

## System Config

System-wide options are documented separately.

- [System Config documentation](./SystemConfig.md)

## Credits

[Credits documentation](./Credits.md)
