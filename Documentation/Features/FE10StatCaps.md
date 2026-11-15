# FE10 Stat Caps

Class stat caps follow Fire Emblem: Radiant Dawn (FE10). Unpromoted classes take FE10's 1st tier and promoted classes its 2nd tier. The values come from serenesforest's maximum-stats table.

## Rule

- An unpromoted FE8 class takes the caps of an FE10 1st-tier class.
- A promoted FE8 class takes the caps of an FE10 2nd-tier class. FE10's 3rd tier is kept for planned third-tier classes (see Third tier below).
- Classes with no FE10 counterpart keep their old caps. These are manaketes, monsters, Bard, Dancer, Fleet, civilians, bosses (Demon King, Draco Zombie), ballistae, Tent, Phantom and Gorgon eggs.

## Where the caps live

| Stat | Source |
|---|---|
| HP, Str, Skl, Spd, Def, Res | `struct ClassData` `max*` fields in `Data/ClassData/ClassData.c` (`maxHP`, `maxPow`, `maxSkl`, `maxSpd`, `maxDef`, `maxRes`). Con is unchanged. |
| Magic | `.cap` in `Data/StrMag/Source/JobMagicInfo.c`. 0, or no entry, means 20 (`GetUnitMaxMagic` in `Kernel/Wizardry/StrMagSplit/Source/core.c`). |
| Luck | `gClassLuckCap`, by class id, in `Data/MaxStatus/ClassLuckCap.c`. 0 means 30. |

The per-character modifiers in `Data/MaxStatus/MaxStatus.c` (`gUnitMaxStatusConfigTable`) still add on top of the class cap.

## Code

- `UNIT_LCK_MAX` (`Tools/FE-CLib-Mokha/include/bmunit.h`) reads `gClassLuckCap`. It was a fixed 30.
- `UNIT_MHP_MAX` (`include/kernel/kernel-lib.h`) returns the class's maxHP (60 if 0) for player and green units, and 120 for enemies. With the designer config's expanded HP it returns 254. For players it used to be a fixed 60.
- The Skill cap in `GetUnitMaxStatusSkl` (`Kernel/Wizardry/UnitStatusGetter/UnitMaxStatus/UnitMaxStatus.c`) used to add the character's Magic modifier by mistake. That is fixed.
- Every `.lyn.event` that uses these macros was regenerated.

## Unpromoted (FE10 1st tier)

| FE8 class | FE10 class | HP | Str | Mag | Skl | Spd | Lck | Def | Res |
|---|---|---|---|---|---|---|---|---|---|
| EPHRAIM_LORD | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| EIRIKA_LORD | Myrmidon | 30 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| CAVALIER | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| CAVALIER_F | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| ARMOR_KNIGHT | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| ARMOR_KNIGHT_F | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| THIEF | Thief | 30 | 15 | 10 | 20 | 20 | 40 | 15 | 10 |
| MERCENARY | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| MERCENARY_F | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| MYRMIDON | Myrmidon | 30 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| MYRMIDON_F | Myrmidon | 30 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| ARCHER | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| ARCHER_F | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| WYVERN_RIDER | Dracoknight | 40 | 20 | 5 | 20 | 20 | 30 | 20 | 10 |
| WYVERN_RIDER_F | Dracoknight | 40 | 20 | 5 | 20 | 20 | 30 | 20 | 10 |
| MAGE | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| MAGE_F | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| SHAMAN | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| SHAMAN_F | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| RECRUIT | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| RECRUIT_T1 | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |
| JOURNEYMAN | Fighter | 45 | 20 | 5 | 20 | 20 | 30 | 20 | 10 |
| JOURNEYMAN_T1 | Fighter | 45 | 20 | 5 | 20 | 20 | 30 | 20 | 10 |
| PUPIL | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| PUPIL_T1 | Fire Mage | 30 | 10 | 20 | 20 | 20 | 30 | 10 | 20 |
| FIGHTER | Fighter | 45 | 20 | 5 | 20 | 20 | 30 | 20 | 10 |
| BRIGAND | Bandit | 45 | 20 | 10 | 20 | 20 | 35 | 20 | 10 |
| PIRATE | Bandit | 45 | 20 | 10 | 20 | 20 | 35 | 20 | 10 |
| MONK | Priest (M) | 30 | 10 | 20 | 15 | 15 | 35 | 10 | 20 |
| PRIEST | Priest (M) | 30 | 10 | 20 | 15 | 15 | 35 | 10 | 20 |
| CLERIC | Priest (F) | 30 | 10 | 20 | 15 | 15 | 40 | 10 | 20 |
| TROUBADOUR | Priest (F) | 30 | 10 | 20 | 15 | 15 | 40 | 10 | 20 |
| PEGASUS_KNIGHT | Pegasus Knight | 30 | 15 | 10 | 20 | 20 | 35 | 15 | 15 |
| SOLDIER | Soldier | 40 | 20 | 10 | 20 | 20 | 30 | 20 | 10 |

## Promoted (FE10 2nd tier)

| FE8 class | FE10 class | HP | Str | Mag | Skl | Spd | Lck | Def | Res |
|---|---|---|---|---|---|---|---|---|---|
| EPHRAIM_MASTER_LORD | Halberdier (M) | 45 | 26 | 15 | 26 | 26 | 30 | 25 | 17 |
| EIRIKA_MASTER_LORD | Swordmaster (F) | 40 | 23 | 15 | 30 | 30 | 30 | 22 | 17 |
| PALADIN | Lance Paladin (M) | 45 | 25 | 15 | 24 | 24 | 30 | 25 | 16 |
| PALADIN_F | Lance Paladin (F) | 45 | 25 | 15 | 24 | 24 | 30 | 23 | 18 |
| GENERAL | Lance General | 45 | 27 | 15 | 25 | 23 | 30 | 30 | 20 |
| GENERAL_F | Sword General (F) | 45 | 27 | 15 | 24 | 22 | 30 | 30 | 22 |
| HERO | Hero | 50 | 27 | 10 | 30 | 30 | 30 | 26 | 15 |
| HERO_F | Hero | 50 | 27 | 10 | 30 | 30 | 30 | 26 | 15 |
| SWORDMASTER | Swordmaster (M) | 40 | 24 | 15 | 30 | 30 | 30 | 23 | 15 |
| SWORDMASTER_F | Swordmaster (F) | 40 | 23 | 15 | 30 | 30 | 30 | 22 | 17 |
| ASSASSIN | Rogue (M) | 40 | 22 | 15 | 26 | 30 | 40 | 20 | 15 |
| ASSASSIN_F | Rogue (F) | 40 | 21 | 15 | 25 | 30 | 40 | 20 | 17 |
| ROGUE | Rogue (M) | 40 | 22 | 15 | 26 | 30 | 40 | 20 | 15 |
| SNIPER | Sniper | 45 | 27 | 10 | 30 | 26 | 30 | 24 | 16 |
| SNIPER_F | Sniper | 45 | 27 | 10 | 30 | 26 | 30 | 24 | 16 |
| RANGER | Bow Paladin (M) | 45 | 23 | 15 | 25 | 24 | 30 | 24 | 18 |
| RANGER_F | Bow Paladin (F) | 40 | 23 | 15 | 27 | 26 | 30 | 23 | 20 |
| WYVERN_LORD | Dragonmaster (M) | 50 | 29 | 5 | 26 | 24 | 30 | 26 | 13 |
| WYVERN_LORD_F | Dragonmaster (F) | 50 | 28 | 5 | 26 | 25 | 30 | 25 | 14 |
| WYVERN_KNIGHT | Dragonmaster (F) | 50 | 28 | 5 | 26 | 25 | 30 | 25 | 14 |
| WYVERN_KNIGHT_F | Dragonmaster (F) | 50 | 28 | 5 | 26 | 25 | 30 | 25 | 14 |
| SAGE | Fire Sage (M) | 40 | 15 | 25 | 23 | 25 | 30 | 17 | 23 |
| SAGE_F | Fire Sage (F) | 40 | 15 | 25 | 23 | 25 | 30 | 15 | 25 |
| MAGE_KNIGHT | Cleric | 40 | 17 | 24 | 25 | 24 | 40 | 18 | 28 |
| MAGE_KNIGHT_F | Cleric | 40 | 17 | 24 | 25 | 24 | 40 | 18 | 28 |
| BISHOP | Bishop (M) | 40 | 15 | 25 | 22 | 23 | 35 | 19 | 30 |
| BISHOP_F | Bishop (F) | 40 | 15 | 25 | 22 | 23 | 40 | 17 | 32 |
| DRUID | Dark Sage | 40 | 15 | 26 | 23 | 24 | 30 | 17 | 23 |
| DRUID_F | Dark Sage | 40 | 15 | 26 | 23 | 24 | 30 | 17 | 23 |
| SUMMONER | Druid | 40 | 15 | 30 | 25 | 23 | 25 | 16 | 26 |
| SUMMONER_F | Druid | 40 | 15 | 30 | 25 | 23 | 25 | 16 | 26 |
| GREAT_KNIGHT | Axe Paladin (M) | 45 | 26 | 15 | 25 | 23 | 30 | 24 | 16 |
| GREAT_KNIGHT_F | Axe Paladin (F) | 45 | 26 | 15 | 25 | 25 | 30 | 22 | 16 |
| RECRUIT_T2 | Lance Paladin (M) | 45 | 25 | 15 | 24 | 24 | 30 | 25 | 16 |
| JOURNEYMAN_T2 | Warrior | 53 | 30 | 5 | 26 | 27 | 30 | 23 | 13 |
| PUPIL_T2 | Fire Sage (M) | 40 | 15 | 25 | 23 | 25 | 30 | 17 | 23 |
| WARRIOR | Warrior | 53 | 30 | 5 | 26 | 27 | 30 | 23 | 13 |
| BERSERKER | Warrior | 53 | 30 | 5 | 26 | 27 | 30 | 23 | 13 |
| FALCON_KNIGHT | Falcon Knight | 40 | 24 | 15 | 24 | 27 | 35 | 22 | 25 |
| VALKYRIE | Cleric | 40 | 17 | 24 | 25 | 24 | 40 | 18 | 28 |

## Third tier (planned, off)

The third-tier rows are in `Data/ClassData/ThirdTierClasses.c` (`gThirdTierClassPlans`, struct `ThirdTierClassPlan` in `include/kernel/third-tier-classes.h`: the promoted class it follows, a name, the eight caps). That file is compiled only with `CONFIG_THIRD_TIER_CLASSES` in `include/configs/configs.h`, which is commented out, so the ROM has none of it (the generated `.lyn.event` is empty). Nothing reads the table yet.

| FE8 promoted class | Third-tier class | HP | Str | Mag | Skl | Spd | Lck | Def | Res |
|---|---|---|---|---|---|---|---|---|---|
| EPHRAIM_MASTER_LORD | Sentinel (M) | 60 | 35 | 20 | 35 | 33 | 30 | 34 | 27 |
| EIRIKA_MASTER_LORD | Trueblade (F) | 50 | 31 | 20 | 40 | 40 | 35 | 25 | 26 |
| PALADIN | Silver Knight (Lance, M) | 60 | 35 | 20 | 34 | 32 | 30 | 30 | 25 |
| PALADIN_F | Silver Knight (Lance, F) | 55 | 33 | 20 | 34 | 34 | 35 | 28 | 27 |
| GENERAL | Marshall (Lance) | 60 | 36 | 20 | 34 | 31 | 30 | 37 | 30 |
| GENERAL_F | Marshall (Sword, F) | 60 | 36 | 20 | 33 | 32 | 30 | 35 | 32 |
| HERO | Vanguard | 65 | 37 | 15 | 40 | 37 | 30 | 32 | 23 |
| HERO_F | Vanguard | 65 | 37 | 15 | 40 | 37 | 30 | 32 | 23 |
| SWORDMASTER | Trueblade (M) | 55 | 32 | 20 | 40 | 40 | 30 | 26 | 24 |
| SWORDMASTER_F | Trueblade (F) | 50 | 31 | 20 | 40 | 40 | 35 | 25 | 26 |
| ASSASSIN | Assassin | 55 | 32 | 15 | 40 | 40 | 25 | 30 | 25 |
| ASSASSIN_F | Assassin | 55 | 32 | 15 | 40 | 40 | 25 | 30 | 25 |
| ROGUE | Whisper (M) | 50 | 28 | 20 | 38 | 40 | 35 | 26 | 25 |
| SNIPER | Marksman | 60 | 36 | 15 | 40 | 34 | 30 | 32 | 25 |
| SNIPER_F | Marksman | 60 | 36 | 15 | 40 | 34 | 30 | 32 | 25 |
| RANGER | Silver Knight (Bow, F) | 50 | 33 | 20 | 36 | 35 | 35 | 27 | 30 |
| RANGER_F | Silver Knight (Bow, F) | 50 | 33 | 20 | 36 | 35 | 35 | 27 | 30 |
| WYVERN_LORD | Dragonlord (M) | 60 | 38 | 10 | 38 | 32 | 30 | 38 | 20 |
| WYVERN_LORD_F | Dragonlord (F) | 55 | 35 | 10 | 38 | 35 | 35 | 36 | 22 |
| WYVERN_KNIGHT | Dragonlord (F) | 55 | 35 | 10 | 38 | 35 | 35 | 36 | 22 |
| WYVERN_KNIGHT_F | Dragonlord (F) | 55 | 35 | 10 | 38 | 35 | 35 | 36 | 22 |
| SAGE | Arch Sage (Fire, M) | 50 | 24 | 38 | 34 | 32 | 30 | 25 | 36 |
| SAGE_F | Arch Sage (Fire, F) | 50 | 24 | 38 | 34 | 32 | 30 | 23 | 38 |
| MAGE_KNIGHT | Valkyrie | 50 | 25 | 35 | 35 | 34 | 35 | 24 | 36 |
| MAGE_KNIGHT_F | Valkyrie | 50 | 25 | 35 | 35 | 34 | 35 | 24 | 36 |
| BISHOP | Saint (M) | 50 | 25 | 36 | 30 | 30 | 35 | 22 | 40 |
| BISHOP_F | Saint (F) | 45 | 23 | 38 | 30 | 31 | 40 | 21 | 40 |
| DRUID | Arch Sage (Dark) | 50 | 25 | 39 | 34 | 31 | 30 | 24 | 36 |
| DRUID_F | Arch Sage (Dark) | 50 | 25 | 39 | 34 | 31 | 30 | 24 | 36 |
| SUMMONER | Summoner | 50 | 26 | 40 | 31 | 30 | 25 | 23 | 39 |
| SUMMONER_F | Summoner | 50 | 26 | 40 | 31 | 30 | 25 | 23 | 39 |
| GREAT_KNIGHT | Gold Knight (Axe, M) | 60 | 36 | 20 | 35 | 31 | 30 | 30 | 24 |
| GREAT_KNIGHT_F | Gold Knight (Axe, F) | 60 | 34 | 20 | 35 | 33 | 30 | 28 | 26 |
| RECRUIT_T2 | Silver Knight (Lance, M) | 60 | 35 | 20 | 34 | 32 | 30 | 30 | 25 |
| JOURNEYMAN_T2 | Reaver | 68 | 40 | 10 | 36 | 35 | 30 | 30 | 20 |
| PUPIL_T2 | Arch Sage (Fire, M) | 50 | 24 | 38 | 34 | 32 | 30 | 25 | 36 |
| WARRIOR | Reaver | 68 | 40 | 10 | 36 | 35 | 30 | 30 | 20 |
| BERSERKER | Reaver | 68 | 40 | 10 | 36 | 35 | 30 | 30 | 20 |
| FALCON_KNIGHT | Seraph Knight | 50 | 32 | 20 | 36 | 36 | 35 | 28 | 34 |
| VALKYRIE | Valkyrie | 50 | 25 | 35 | 35 | 34 | 35 | 24 | 36 |

To enable a third-tier class:

1. Give it a class id (append to the class enum).
2. Add a `ClassData_NEW` entry with these caps (`max*` fields).
3. Add its `JobMagicInfo` `.cap` and `gClassLuckCap` entries.
4. Add a map sprite, battle animations, and name and description texts (new messages at the end of `Contents/Texts/Source/texts/Districts.txt`).
5. Set its promotion from the FE8 promoted class.

Then turn the switch on if anything should read `gThirdTierClassPlans`.

## Notes

- FE10's 1st-tier caps are low for some off-stats. A Mage's Def and a Myrmidon's Res cap at 10. Physical classes' Magic caps at 5-10. Many unpromoted HP caps are 30-45. Units hit these caps before they promote, as in FE10.
- A unit already above a new cap keeps that stat. A level-up or promotion clamps it to the cap (`CheckCap.c`).
- FE10 has no Con. Con caps are unchanged.
- Promoted classes now top out around 30 for most stats (FE10's second tier). HP reaches 53 (Warrior) and Luck 40. Caps of 40 are reserved for the third tier, apart from Luck.

## Changing caps

1. In `Data/ClassData/ClassData.c`, edit the class's `max*` fields (FE10 1st tier for unpromoted classes, 2nd tier for promoted classes).
2. In `Data/StrMag/Source/JobMagicInfo.c`, edit the class's `.cap`.
3. In `Data/MaxStatus/ClassLuckCap.c`, edit the class's `gClassLuckCap` entry.
4. Rebuild with `Tools/cloud/build.sh`.

The FE10 values were written by a script (`fe10caps.py`, not in the repo). Its tables are reproduced above. It writes T1 to unpromoted classes and T2 to promoted classes. It also generates `Data/ClassData/ThirdTierClasses.c` from T3, for the planned third-tier classes.
