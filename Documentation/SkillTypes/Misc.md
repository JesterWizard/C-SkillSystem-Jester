# Misc skills (shop price, heal amount, item use, legendary, activation rate)

> Text file: `Contents/Texts/Source/texts/Skills_Misc.txt`  ·  Code: `Kernel/Wizardry/SkillEffects/Misc/`, `Kernel/Wizardry/SkillEffects/ShopSkills/`, `Kernel/Wizardry/SkillEffects/LegendarySkills/` (dormant, see Gotchas)  ·  Hook: `GetItemPurchasePrice` / `GetItemSellPrice` (ROM-patched by `Kernel/Wizardry/SkillEffects/ShopSkills/LynJump.event`), `gHealAmountGetters` (`Kernel/Wizardry/ItemSys/ItemSys.event`), `ItemSubMenu_UseItem` and `CanUnitUseHealItem` (LYN replace in `Kernel/Wizardry/SkillEffects/Misc/ItemSkills.c`)

## What it is

The category for skills whose effect is a price, heal-amount, item-use or activation-rate modifier, not a phase, post-action or battle hook. `Skills_Misc.txt` holds many unrelated skills (level-up, rescue, staff, forge, ...). Only the sub-kinds below have code in the folders above. Other `Skills_Misc` entries are implemented elsewhere; see Gotchas for those with no code at all.

Sub-kinds:

- Shop price: Bargain, Deal, Trader.
- Heal amount: Gentle Flower.
- Item use: Salve, Acidic, Acidic+, Item Lore.
- Activation rate: shared helper `SkillActRate.c`, used by many battle skills.
- Legendary: directory exists, no source, not included (dormant).

## How the engine calls it

### Shop price

- `GetItemPurchasePrice` at `Kernel/Wizardry/SkillEffects/ShopSkills/Price.c:6` (`LYN_REPLACE_CHECK` at line 5). Order: base cost; prep-screen +50% (`BM_FLAG_PREPSCREEN`); Entrepreneur surcharge (line 14, inside the `SID_Bargain` guard); `ITEM_SILVERCARD` halves and returns (line 18); Bargain via `SkillListTester` returns 50% (lines 23-27); Deal via `SkillListTester` returns 80% (lines 30-34).
- `GetItemSellPrice` at `Price.c:41`. Trader (`SkillTester(gActiveUnit, SID_Trader)`, line 44) gives 3/4 of cost; default is 1/2.
- Both are replaced at ROM addresses `$b5220` and `$B5268` by `Kernel/Wizardry/SkillEffects/ShopSkills/LynJump.event`.
- Included from `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event:170-171`.

### Heal amount

- `HealAmountGetter(base, actor, target)` at `Kernel/Wizardry/ItemSys/Source/StaffHeal.c:20`. Calls every entry of `gHealAmountGetters` in order, then `LIMIT_AREA(status, 0, 80)` (line 28).
- `gHealAmountGetters` (`Kernel/Wizardry/ItemSys/ItemSys.event:15-18`): `HealAmount_SkillsGetter`, `WhiteWindHealAmountGetter`, then a `KernelPad2` terminator.
- `HealAmount_SkillsGetter` at `Kernel/Wizardry/SkillEffects/Misc/HealAmount.c:6`. Gentle Flower (`SID_GentleFlower`): if the actor, the target, or any unit in the 2x2 around either has the skill, the amount is multiplied by 1.5 (`status += status / 2`).
- Callers: `StaffHeal.c` (lines 125, 248, 323, 479, 494, 656) and `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-components.c:560`.

### Item use

- `ItemSubMenu_UseItem` at `Kernel/Wizardry/SkillEffects/Misc/ItemSkills.c:276` (LYN replace). If `SkillTester(gActiveUnit, SID_Salve)` and the item is a heal or booster, it opens target selection with `Salve_OnSelectTarget` (`ItemSkills.c:189`, line 355). Acidic and Acidic+ open selection with `Acidic_OnSelectTarget` (`ItemSkills.c:242`, lines 300 and 323). Otherwise `DoItemUse`.
- `CanUnitUseHealItem` at `ItemSkills.c:380` (LYN replace). Acidic needs an adjacent enemy (1x1; 3x3 for Acidic+). Salve needs a damaged ally (or self) adjacent.
- Item Lore (`SID_ItemLore`): `ExecHeal_Target` doubles the amount (`ItemSkills.c:18-24`). Vulnerary and Elixir also double in `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c:3525` (`ExecVulneraryItem`) and `:3550` (`ExecElixirItem`).
- Item to skill grant: `gConstSkillTable_Item[ITEM][2]` (`Data/SkillSys/Source/SkillTable-item.c:5`), read in `Kernel/Wizardry/SkillSys/kernel/SkillList.c:368-369` and `:434-435`. The table is currently empty.

### Activation rate

- `CheckSkillActivateExt` at `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c:6`. Requires the tester to pass. Right Arch auto-passes. Right King / Right God add `SKILL_EFF0`. Hero adds `SKILL_EFF0` below half HP. Domino adds +10 if an allied unit is in the 3x3 (lines 32-49). Serene Grace doubles. Rate is clamped 0-100 (`LIMIT_AREA`), then `Roll2RN(rate)`.
- Wrappers: `CheckSkillActivate` (`:67`, uses `SkillTester`), `CheckActiveUnitSkillActivate` (`:72`, uses `SkillListTester`), `CheckBattleSkillActivate` (`:77`, uses `BattleFastSkillTester`; returns false during simulation and when the target has Foresight).
- Callers: `Kernel/Wizardry/ItemSys/Source/ItemUse.c:41` (Weapon Heal), `Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c` (many, e.g. line 19).
- Included from `MiscSkillEffects.event:133`.

### Legendary

- `Kernel/Wizardry/SkillEffects/LegendarySkills/JudgeLegend.lyn.event` (`GetUnitLegendarySkill`, `TryActivateLegendSkill`, `SkillTesterLegendActivated`, `PrePhase_UpdateLengendSkillStatus`) and `LegendEffect.lyn.event` (`PreBattleCalcLegendSkills`, `SpdGetterLegendSkills`, `DefGetterLegendSkills`, `ResGetterLegendSkills`). Both are compiled dumps. Neither is included by any `.event` file. See Gotchas.

## Where to look

| File | Role |
| :--- | :--- |
| `Contents/Texts/Source/texts/Skills_Misc.txt` | Names and descriptions for all misc skills (`MSG_SKILL_<Name>_NAME` and `MSG_SKILL_<Name>_DESC`). |
| `Kernel/Wizardry/SkillEffects/ShopSkills/Price.c` | Bargain, Deal, Entrepreneur surcharge, Trader. |
| `Kernel/Wizardry/SkillEffects/ShopSkills/LynJump.event` | ROM hooks for the two price functions. |
| `Kernel/Wizardry/SkillEffects/Misc/HealAmount.c` | Gentle Flower getter. |
| `Kernel/Wizardry/ItemSys/ItemSys.event` | `gHealAmountGetters` list (lines 15-18). |
| `Kernel/Wizardry/ItemSys/Source/StaffHeal.c` | `HealAmountGetter` (20), its clamp, and the callers. |
| `Kernel/Wizardry/SkillEffects/Misc/ItemSkills.c` | Salve, Acidic, Acidic+, Item Lore (`ExecHeal_Target`), `ItemSubMenu_UseItem`, `CanUnitUseHealItem`. |
| `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c` | Item Lore in `ExecVulneraryItem` / `ExecElixirItem` (3525, 3550), Secret (3183), Scroll Savant (3506). |
| `Data/SkillSys/Source/SkillTable-item.c` | `gConstSkillTable_Item`: item to skill grant table (empty now). |
| `Kernel/Wizardry/SkillSys/kernel/SkillList.c` | Reads the item table (368-369, 434-435). |
| `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c` | Activation-rate helper and wrappers. |
| `Kernel/Wizardry/SkillEffects/LegendarySkills/` | Dormant legendary code (lyn dumps only). |
| `Data/SkillSys/Source/LegendSkill.lyn.event` | `gLegendSkillPool` (empty, not included). |
| `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event` | Includes: HealAmount 132, SkillActRate 133, ItemSkills 134, LynJump 170, Price 171. |
| `include/constants/skills-equip.enum.txt` | Every misc SID (the 0x300-0x391 block is here, see Gotchas). |
| `Documentation/Features/MiscSkillEffect.md` | Older notes on post-action position, Thunderstorm and weather skills. Those are not in this folder. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the sub-kind:
   - Price: add a `SkillListTester` (or `SkillTester`) branch to `GetItemPurchasePrice` / `GetItemSellPrice` in `Price.c`. Keep the silver-card early return where it is unless you mean to change the order.
   - Heal amount: write a getter (template below), add it to `gHealAmountGetters`, and include its `.lyn.event` from `MiscSkillEffects.event`.
   - Item use: add a branch in `ItemSubMenu_UseItem` / `CanUnitUseHealItem` in `ItemSkills.c`, following the Salve or Acidic pattern (`gActionData.unk08 = SID_<Name>` and a target-select proc). Doubling/bonus logic goes into `ExecHeal_Target` or the item's Exec function.
   - Item grant: add `[ITEM_<X>] = { SID_<A>, SID_<B> }` to `Data/SkillSys/Source/SkillTable-item.c`. Use two slots per item (`[iid * 2 + 0]` and `[iid * 2 + 1]`).
   - Activation roll: call `CheckSkillActivate`, `CheckActiveUnitSkillActivate` or `CheckBattleSkillActivate` from the battle code. Do not add a new roll in `SkillActRate.c` unless it is shared.
   - Legendary: not supported in the current tree. See Gotchas.
3. Add the SID to `include/constants/skills-equip.enum.txt` (not to the item or class enum files).
4. Text in `Skills_Misc.txt`, then the usual `SkillInfo.c` / icon / price / capacity entries.

## Template

Adapted from `Kernel/Wizardry/SkillEffects/Misc/HealAmount.c` (`HealAmount_SkillsGetter`, Gentle Flower). Put the file in `Kernel/Wizardry/SkillEffects/Misc/`.

```c
/* Kernel/Wizardry/SkillEffects/Misc/HealExample.c */
#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

/* Registered in gHealAmountGetters (Kernel/Wizardry/ItemSys/ItemSys.event):
 *     POIN HealAmount_Example
 */
int HealAmount_Example(int status, struct Unit *actor, struct Unit *target)
{
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
    /* actor can be NULL: IER-components.c calls HealAmountGetter(amount, NULL, unit). */
    if (actor && SkillTester(actor, SID_Example))
        status += status / 4;
#endif

    return status;
}
```

Registration, in `Kernel/Wizardry/ItemSys/ItemSys.event`:

```
gHealAmountGetters:
    POIN HealAmount_SkillsGetter
    POIN HealAmount_Example
    POIN WhiteWindHealAmountGetter
    POIN KernelPad2 KernelPad2 KernelPad2 KernelPad2
    WORD 0
```

Include, in `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event`:

```
#include "Misc/HealExample.lyn.event"
```

## Worked examples

- Bargain / Deal / Trader: `Kernel/Wizardry/SkillEffects/ShopSkills/Price.c` - price modifiers, `SkillListTester` vs `SkillTester`, ROM hooks in `ShopSkills/LynJump.event`.
- Gentle Flower: `Kernel/Wizardry/SkillEffects/Misc/HealAmount.c` - heal getter that scans a 2x2 around the actor and the target.
- Salve / Acidic / Acidic+: `Kernel/Wizardry/SkillEffects/Misc/ItemSkills.c` - item-use submenu replacement with target selection.
- Item Lore: `Kernel/Wizardry/SkillEffects/Misc/ItemSkills.c` (line 22) and `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c` (3525, 3550) - doubles item heal in two code paths.

## Gotchas

- Unverified: `SID_CatchEmAll` (Catch Em All: "gives this unit every skill"). It has no `#define` in `include/constants/skills.h` and no line in `skills-equip.enum.txt`, so its `SkillInfo.c` entry (line 6740) is compiled out. Its text and price/capacity defines exist. Nothing else references it.
- Unverified: `SID_Savior` (Savior: "no rescue penalties"). It is defined (`skills.h:840`, value 0x343) and has a `SkillInfo.c` entry, but the only code reference is the enum itself. The rescue-penalty code was not located.
- Every other `Skills_Misc` SID has at least one code reference (checked by grep). Those references were not all traced, so their behaviour is unverified unless it is covered by another doc.
- SID range: `COMMON_SKILL_VALID` is `sid > 0 && sid < 0x400` (`include/kernel/skill-system.h:90`). The highest SID in `skills.h` is 0x391. The "item" range 0x300-0x3FF is just the tail of `skills-equip.enum.txt`. `skills-item.enum.txt` is not built (`makefile:411` commented out), so new misc SIDs go in the equip enum.
- `Data/SkillSys/Source/SkillTable-item.c` is an item-to-skill table, not an enum. It is empty in this tree.
- Entrepreneur surcharge (`Price.c:14`) sits inside `#if (defined(SID_Bargain) ...)`. Removing `SID_Bargain` also removes that surcharge.
- Silver Card returns before Bargain or Deal (`Price.c:18-20`), so Bargain / Deal never stack with it.
- Heal clamp: `HealAmountGetter` clamps the final value to 0-80 (`StaffHeal.c:28`). Gentle Flower's 1.5x is applied before the clamp, so it cannot exceed 80.
- Null actor: `IER-components.c:560` calls `HealAmountGetter(amount, NULL, unit)`. `HealAmount_SkillsGetter` calls `SkillTester(actor, SID_GentleFlower)` without a null check (`HealAmount.c:14`). `SkillTester` source is not in this repo; NULL behaviour is unverified.
- Item Lore covers Vulnerary, Elixir and the Salve / Acidic heal path only. Staff heals in `StaffHeal.c` do not check `SID_ItemLore`.
- `SkillActRate.c:46` uses a comma operator: `if (tester(unit_ally, SID_Domino), AreUnitsAllied(...))`. The tester result is discarded, so any allied unit in the 3x3 gives +10 whether or not it has Domino. Read from code; not played.
- Legendary (`Kernel/Wizardry/SkillEffects/LegendarySkills/`): no `.c` or `.h` source for any function in those files, and no `.event` includes them. `Data/SkillSys/Source/LegendSkill.lyn.event` holds an empty `gLegendSkillPool` and is not included by `Data/SkillSys/SkillSys.event`. No `SID_*Legend*` exists. Treat as dormant. Do not wire it in without recovering the source.
- `Kernel/Wizardry/SkillEffects/Misc/` also holds `JesterFunctionsStructs.c` and `LynJump.event`. Not documented here.
