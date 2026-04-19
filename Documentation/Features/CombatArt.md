# Combat Art

![image](./Images/Home_CombatArt1.png)
![image](./Images/Home_CombatArt2.png)

Combat Arts add a Three Houses style command to combat preparation. During target selection, the unit can choose a special art that trades normal attack rules for stronger bonuses or extra effects.

## Overview

Combat Arts are built around a simple tradeoff.

- They can increase damage, hit, range, or add special effects.
- They usually cost more weapon durability.
- They normally prevent follow-up attacks unless the art explicitly allows them.

This makes them useful as burst tools, utility options, or ways to give a weapon type a more specialized role.

## Editor Setup

The kernel provides several FEBuilder patches for defining Combat Arts and deciding who can use them.

## Art Data

![image](./Images/CombatArt_Patch1.png)

Use this table to define the core behavior of each art.

| Field | Purpose |
| :---- | :------ |
| `Icon` | 16x16 paletted icon shown for the Combat Art. |
| `name` | Display name used in the selection menu. This must be set. |
| `desc` | Description text for the art. |
| `wtype` | Weapon type required for the art. Set to `0xFF` to allow all weapon types. |
| `range bonus` | Extra range added to the equipped weapon, especially useful for bows or magic-related arts. |
| `cost` | Extra weapon durability consumed per hit. |
| `atk/def/hit/avo/crit/silcencer/dodge` | Battle stat modifiers applied while using the art. |
| `display_en_n` | Controls how the RText window shows stat information and description text. |
| `double_attack` | Controls whether the user can make a follow-up attack. |
| `magic_attack` | Forces the attack to use magic attack rules, using the actor's `MAG` against the target's `RES`. |
| `effectiveness` | Applies a built-in effectiveness category. |
| `debuff` | Applies a debuff to the target on hit. |
| `aoe_debuff` | Applies a 2x2 area debuff effect on hit. |

### Display Setting

`display_en_n` controls how the RText panel is used.

- `0`: Show formatted battle stat information in RText and leave one line for the description.
- `1`: Skip the formatted battle stat block and show a full three-line description instead.

### Follow-Up Rules

`double_attack` controls whether the art can double.

- `0`: The unit cannot double while using this art.
- `1`: The unit can double if normal doubling rules are met.
- `2`: The unit always doubles, regardless of normal doubling rules.

### Effectiveness Setting

`effectiveness` uses the following built-in values.

- `0`: No effectiveness bonus.
- `1`: Effective against all targets.
- `2`: Effective against armor units. Requires the FEBuilder class type patch for Armor.
- `3`: Effective against cavalry units. Requires the FEBuilder class type patch for Cavalry.
- `4`: Effective against flying units. Requires the FEBuilder class type patch for Flier.
- `5`: Effective against dragon units. Requires the FEBuilder class type patch for Dragon.
- `6`: Effective against monster units. Requires the FEBuilder class type patch for Beast.

## Unlock Conditions

After defining an art, use the following lists to determine who can access it.

## Generic Weapon Rank List

![image](./Images/CombatArt_PatchWRankList.png)

Any unit that reaches the listed weapon rank can use the art.

## Character Weapon Rank List

![image](./Images/CombatArt_PatchWRankPList.png)

Only the specified character can use the art, and only after reaching the listed weapon rank.

## Class Weapon Rank List

![image](./Images/CombatArt_PatchWRankJList.png)

Only units in the specified class can use the art, and only after reaching the listed weapon rank.

## Skill Requirement List

![image](./Images/CombatArt_Patch2.png)

Units can use the art only while they have the required skill.

## Weapon Requirement List

![image](./Images/CombatArt_Patch3.png)

Units can use the art only while they have the required weapon equipped.
