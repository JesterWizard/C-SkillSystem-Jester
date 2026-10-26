The GaidenMagic system in c-skillsys is a remake of `B.Mag` and `W.Mag` in *Fire Emblem: Gaiden*. The related code is mainly stored in:

- [Misc/GaidenMagic](../Kernel/Wizardry/GaidenMagic)
- [BattleSys/HpCost](../Kernel/Wizardry/BattleSys/HpCost)
- [BattleWeapon.c](../Kernel/Wizardry/BattleSys/Source/BattleWeapon.c).

Gaiden magic config lists have been actively adapted for FEB. Now users can directly use existing FEB form and patch to set unit gaiden magic list and item hp cost.

# Configs

We have put several config options to increase the freedom for developers. You can change the functionality via the following configs in [configs.h](../include/configs/configs.h):

- `USE_GAIDEN_MAGIC`: enables the gaiden magic system.
- `MUST_BE_MAGIC`: allows none-magic weapons exist in B.Mag list.
- `REQUIRES_WRANK`: determines whether the character can use magic that exceeds his weapon rank.
- `AI_EN`: allows AI to use B/W magic.
- `EXT_CONF_EN`: enables external magic list config table: `gGaidenChaxConfigs`
- `gaiden_magic_three_houses`: spells use a number of uses per chapter instead of an HP/MP cost. See [Three Houses-style magic](#three-houses-style-magic).

To improve the user experience for non-wizardry developers, the configs above can be directly modified by `gKernelDesigerConfig` via FEB patch free from custom build.

# Tables

C-SkillSys supports the same table as old skillsys, which can be directly setup by FEB:

- `gGaidenPinfoConfigList`: give unit magic on considering their level, can be configured by FEB form **Gaiden-style Spell**.
- `gGaidenWeaponCostList`: set hp cost for item, can be directly configurable via existing FEB patch: **GaidenSpellCostTable**.

On this basis, we have added a new dynamic table to increase the freedom of configuration:

- `gGaidenChaxConfigs`: equip magic by:
	- character index
	- class index
	- unit level
	- unit faction
	- skill
	- event flag

# Three Houses-style magic

Set `gaiden_magic_three_houses` in [designer-config.c](../../Data/DesignerConfig/designer-config.c) to make Gaiden magic work like *Fire Emblem: Three Houses*.

## Spells instead of tomes

Tomes (anima, light, dark) and staves stop being items:

- Player units learn spells from their weapon ranks. Each entry in a spell list is `{ rank, spell }`, checked against the rank of the spell's own weapon type (Fire uses Anima rank, Heal uses Staff rank). When a battle or staff use raises a rank, the unit gets an `X learned [spell]` popup after the rank-up popup.
- `gGaidenMagicRankPList[character]` in [GaidenMagicData.c](../../Data/GaidenMagic/GaidenMagicData.c) holds personal lists, like each student's list in Three Houses. Characters without one use `gGaidenMagicRankDefaultList`. A unit holds up to 7 B.Mag and 7 W.Mag. The stat page shows 5 of each.
- Casting a B.Mag spell now gives weapon EXP for its type, so ranks rise from casting.
- Enemies and NPCs cast the tomes and staves in their inventory as spells, with limited uses. Those items are hidden from their item page and never drop. AI spell use is on in this mode, whatever `gaiden_magic_ai_use` says.
- No unit can wield a tome or staff as an item (`CanUnitUseWeapon()`/`CanUnitUseStaff()` return false).
- Player units never hold them. They are stripped on unit load, at chapter start and at the start of each phase, which covers recruits and old saves. The convoy is cleaned too, shops don't sell them, and item gifts (`HandleNewItemGetFromDrop()`) and `AddItemToConvoy()` reject them.
- Monster weapons (Evil Eye, Stone...) are dark-typed but have no rank requirement, so they stay normal weapons.

> Events that give a tome or staff (chests, villages, `GIVEITEMTO`) still show the "Got" popup, but the item is not added. Remove those gifts from chapter events.

The existing level-based list (`gGaidenPinfoConfigList`) and `gGaidenChaxConfigs` still add spells in this mode.

## Uses

- Spells cost no HP or MP. `GetGaidenWeaponHpCost()` returns 0, so `gGaidenWeaponCostList` is ignored.
- Each spell has a number of uses. All uses come back at the start of every chapter.
- A combat spends one use, no matter how many times the caster hits. Follow-ups, brave hits and counters with the same spell are free.
- A W.Mag staff spends one use per cast.
- A spell with no uses left is grayed out in the B.Mag/W.Mag menus. It can't be auto-equipped for counters, and the AI skips it.
- The B.Mag/W.Mag menus and the Gaiden Magic stat page show the uses left instead of the cost. `--` means unlimited.
- `SID_GrandMagician` gives unlimited uses. In the normal mode it removes the cost.

Tables in [GaidenMagicData.c](../../Data/GaidenMagic/GaidenMagicData.c):

- `gGaidenMagicUsesList[item]`: base uses per chapter. `0` uses the item's max uses from item data. Unbreakable items are unlimited.
- `gGaidenMagicClassUsesBonus[class]`: like the *Black/White Magic Uses x2* class abilities. `GAIDEN_MAGIC_USES_BMAG_X2` doubles B.Mag uses and `GAIDEN_MAGIC_USES_WMAG_X2` doubles W.Mag (staff) uses. By default Sage and Druid double B.Mag, Bishop and Valkyrie double W.Mag, and Necromancer doubles both.

Uses are tracked in [ThreeHouses.c](../../Kernel/Wizardry/GaidenMagic/Source/ThreeHouses.c). A pool of 64 `(unit index, item, uses spent)` entries (`sGaidenMagicUsesPool`, 192 bytes) is saved in suspend data and cleared on chapter start and on game-save load. Spending works like this:

- `BattleGenerateHit()` sets `gaiden_magic_cast` in the battle global flags on the first spell attack of each side.
- `GaidenMagicUses_OnBattleToUnit()` spends the use when the real battle is applied to the unit, so forecasts and AI simulations never spend uses.
- `ActionGaidenMagicStaff()` spends the use for staves.
- Entries are dropped when a unit is cleared or killed. When the pool is full, entries of units that left the map are reused.

> Changing the suspend layout makes older suspend saves load wrong uses. Start a chapter fresh after updating.

# Limitations
- `B.Mag` can only support weapons.
- `B.Mag` supoorts non-magic weapons under the configuration of `CONFIG_GAIDEN_MAGIC_MUST_BE_MAGIC`.
- `W.Mag` can only supoort staff items.

> To following sections are mainly for wizardry.

# Weapon slot

The whole gaiden magic system in c-skillsys is built based on battle weapon-slot expansion. Based on vanilla function `SetBattleUnitWeapon()`, weapon slot 9-15 are allocated to gaiden B.Mag, and 16-22 to W.Mag. And kernel may find the related magic by function `GetGaidenMagicItem()`. For more details, see *Weapon slot expansion* section in [BattleSystem.md](./BattleSystem.md).

# Gaiden magic action

Both gaiden B.Mag and W.Mag hold their unique action seperated from existing combat/staff action to better handle weapon selection and hp cost issues:

```c
// configs.h
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat 0x24
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff 0x25
```

In order to reduce the modification of item effect functions, we directly insert a action transform in action routine, `TryChangeGaidenMagicAction()`, in which we may judge on `gActionData.itemSlotIndex`, and transform to gaiden magic action for gaiden magic weapon slots.

The item use effect (`DoItemUse()`) and action (`ActionStaffDoorChestUseItem()`) routine is just introduced the minimal modification: We just replace the function call on [item-getter in item-use](https://github.com/MokhaLeee/fireemblem8u/blob/master/src/bmitemuse.c#L296) to `AutoGetItemUseActionItem()`.

# HP cost

C-SkillSys hold a seperate HP-cost system in battle-sys. For more details, see *Hp cost* section in [BattleSystem.md](./BattleSystem.md).

# Kown issues

You need to take note that the unrestrained expansion of `gGaidenChaxConfigs` may cause potential performance issue. The weak GBA CPU may suffer a lot of time to calculate the magic list for each unit especially when the AI ​​calculates each enemies' combat radius to obtain dangerous area. Related call stack:

```c
// 1
AiAttemptOffensiveAction -->
AiFillReversedAttackRangeMap -->
GetUnitMaxRange -->
GetGaidenMagicList -->
UpdateGaidenMagicList

// 2
CpPerform_EquipBest -->
GetItemMaxRange -->
GetGaidenMagicList -->
UpdateGaidenMagicList
```
