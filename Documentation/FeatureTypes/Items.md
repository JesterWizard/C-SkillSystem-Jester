# Items: new items, item-use effects, staves

> Code: `Kernel/Wizardry/ItemSys/`, `Kernel/Data/ItemSys/`, `Data/ItemSys/`, `Data/CustomItems/`  ·  Entry points: `GetIERevamp` (`Kernel/Wizardry/ItemSys/IERevamp/Source/IER-core.c:6`) indexing `IERevampTable` (`Kernel/Data/ItemSys/Source/IERevampTable.c`); item rows in `gItemData_New` (`Data/ItemSys/Source/Items.c`)  ·  Existing docs: `Documentation/Features/ItemSys.md`, `Documentation/Features/Tonics.md`, `Documentation/Features/ShieldItem.md`, `Documentation/Features/ModularStaffEXP.md`, `Documentation/Features/CustomStaves/SkillStaff.md`

## What it covers

An item is one row of `gItemData_New` plus an item-effect-revamp (IER) row selected by `ItemData::useEffectId`. The IER row holds the item's map usability, prep-screen usability/effect, target-selection effect and battle-side action. This doc covers three cases: a new usable item (heal-type, reusing the Vulnerary functions), a new staff (target list, effect, action, staff EXP), and the optional AI and prep-screen hooks. Examples in repo: Vulnerary (`ITEM_VULNERARY`), Tonic (`ITEM_TONIC`, prep screen), Slow staff (`ITEM_STAFF_SLOW`), Again staff (`Data/CustomItems/AgainStaff/AgainStaff.c`).

Related, documented elsewhere:
- Item-held skills (a skill active while the item is in the unit's inventory): the table is `Data/SkillSys/Source/SkillTable-item.c`, documented in [SkillTypes/Common.md](../SkillTypes/Common.md). [SkillTypes/Misc.md](../SkillTypes/Misc.md) covers item-use and heal-amount skills (Salve, Acidic), not held skills.
- Weapon-specific battle effects and the battle-calc skill pipeline: [SkillTypes/BattleCalc.md](../SkillTypes/BattleCalc.md). The item-side effect IDs and their call sites are listed under "How the engine calls it" below.

## How the engine calls it

ROM hooks for the IER core are in `Kernel/Wizardry/ItemSys/IERevamp/Source/LynJump.event` (`CanUnitUseItem` at `0x28870`, `DoItemUse` at `0x28E60`, `CanUnitUseItemPrepScreen` at `0x29F00`, `ActionStaffDoorChestUseItem` at `0x2FC48`, `PrepItemUse_HandleItemEffect` at `0x9CB38`). The table pointer is patched in `Kernel/Wizardry/ItemSys/IERevamp/IERevamp.event` (`ORG 0x28E90`, `gpIERevampTable: POIN IERevampTable`) and read as `*pr_gpIERevampTable` (`include/kernel/item-sys.h:152`).

1. Usability (item menu). `CanUnitUseItem` (`IER-core.c:24`) rejects staves the unit cannot use (`CanUnitUseStaff`, `IER-core.c:26-27`), then calls `revamp->usability`. `GetIERevamp` (`IER-core.c:6-21`) returns NULL unless `IER_IS_VALID` (`include/kernel/item-sys.h:140`). NULL usability means the item cannot be used.
2. Selection. `DoItemUse` (`IER-core.c:43`) calls `revamp->effect` when set; this usually opens target selection (e.g. `IER_Effect_Slow` calls `DoUseAttackStaff`, `IER-components.c:359-362`). When `effect` is NULL it calls `SetItemUseAction(unit)` (no targeting).
3. Target choice. Heal staves use `StaffSelectOnSelect` (`Kernel/Wizardry/ItemSys/Source/StaffHeal.c:198`), which stores `gActionData.targetIndex` and calls `SetStaffUseAction(NULL)`.
4. Action. When the queued action resolves, `ActionStaffDoorChestUseItem` (`IER-core.c:60`) calls `revamp->action_effect(proc, unit, item)`, then starts `ProcScr_SetTargetStatus` if `gBattleTarget.statusOut >= 0`. Examples: `IER_Action_VulneraryItem` (`IER-components.c:555`), `IER_Action_CustomStaves` which calls `ExecCustomStaves` (`IER-components.c:638`, `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-extra.c:638`).
5. Heal amount. `GetUnitItemHealAmount` (`IER-extra.c:132`): with `gpKernelDesignerConfig->item_effect_revamp`, the amount is `GetItemMight` plus `IER_BYTE(item)` (`include/kernel/item-sys.h:156`, the `healByte` field). If that is 0, a switch on `useEffectId` supplies defaults. Staves add `UNIT_MAG`. The result is capped at 80. Staff heals then go through `HealAmountGetter` (`StaffHeal.c:20`).
6. Staff EXP. `BattleApplyItemExpGains` (`Kernel/Wizardry/BattleSys/Source/BattleExp.c:381`) grants EXP only when the weapon has `IA_STAFF` (`BattleExp.c:393`). `GetBattleUnitStaffExpRework` (`BattleExp.c:344`) calls `StaffEXP(ITEM_INDEX(weapon))` (`BattleExp.c:18`) when `modular_staff_exp` is set; otherwise it uses vanilla `GetBattleUnitStaffExp`.
7. Prep screen. `CanUnitUseItemPrepScreen` (`IER-core.c:83`) calls `prep_usability`. `PrepItemUse_HandleItemEffect` (`IER-core.c:97`) calls `prep_effect`. The Tonic is the worked case (`IER_PrepEffect_Tonic`, `IER-components.c:672`).
8. Name, description, icon. `GetItemData` (`Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ItemInfos.c:333`) returns `gItemData_New + index`. `GetItemIconId` (`ItemInfos.c:508`) returns `iconId`, except tonics and durability items, which have their own branches.
9. AI. `AiTryDoStaff` (`Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:812`) collects every `IA_STAFF` item (`AiOptimization.c:445`) and dispatches through `GetAiStaffFuncIndex` (`AiOptimization.c:830`, vanilla, no repo source). `AiTryHealSelf` (`IER-extra.c:248`) only considers `IER_VULNERARY` and `IER_ELIXIR`.

## Where to look

| File | Role |
|---|---|
| `Tools/FE-CLib-Mokha/include/constants/items.h` | `ITEM_*` index constants (`ITEM_VULNERARY = 0x6C`, `ITEM_STAFF_SLOW = 0xC8`, `ITEM_TONIC = 0xDF`) |
| `include/constants/item-icons.h` | `ITEM_ICON_*` icon ids (`ITEM_ICON_STAFF_SLOW`, `ITEM_ICON_TONIC_HP`) |
| `Data/ItemSys/Source/Items.c` | `gItemData_New[255]`: one `struct ItemData` row per item (struct at `Tools/FE-CLib-Mokha/include/bmitem.h:19-50`) |
| `Data/ItemSys/Source/Items.lyn.event` | Generated from `Items.o`. Do not hand-edit |
| `Contents/Texts/Source/texts/Items.txt` | Item name, description and use-description messages (`## MSG_...` blocks). Included from `Contents/Texts/Source/kernel.txt:42` |
| `include/kernel/item-sys.h` | IER enum (`enum chax_ierevamp_use_effects`, `IER_MAX`), `struct IERevamp`, `IER_BYTE`, `IER_DECLEARE_*` prototypes (`:214-219`) |
| `Kernel/Data/ItemSys/Source/IERevampTable.c` | `IERevampTable[IER_MAX]`: one row per `useEffectId` |
| `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-core.c` | `GetIERevamp`, `CanUnitUseItem`, `DoItemUse`, `ActionStaffDoorChestUseItem`, prep-screen replacements |
| `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-components.c` | Built-in `IER_Usability_*`, `IER_Effect_*`, `IER_Action_*`, `IER_PrepEffect_*` |
| `Kernel/Wizardry/ItemSys/IERevamp/Source/IER-extra.c` | `GetUnitItemHealAmount`, `ExecStatBoostItem`, `ApplyStatBoostItem` (tonic state), `AiTryHealSelf`, `ExecCustomStaves` |
| `Kernel/Wizardry/ItemSys/IERevamp/Source/LynJump.event` | ROM hooks that route vanilla item code into `IER-core.c` |
| `Kernel/Wizardry/ItemSys/ItemSys.event` | Includes the IER, prep-screen and spell-assoc sources; `gHealAmountGetters` |
| `Kernel/Wizardry/ItemSys/Source/StaffHeal.c` | `HealAmountGetter`, `StaffSelectOnSelect`, `ExecStandardHeal` |
| `Kernel/Wizardry/WeaponRange/Source/StaffRange.c` | `MakeTargetListFor*` for staves (`MakeTargetListForSlow` at `:268`) |
| `Kernel/Wizardry/BattleSys/Source/BattleExp.c` | `StaffEXP` (`:18`), `GetBattleUnitStaffExpRework` (`:344`), `BattleApplyItemExpGains` (`:381`) |
| `include/jester_headers/custom-functions.h` | Prototypes for `MakeTargetListFor*`, `ExecCustomStaves`, `StaffEXP` |
| `include/kernel/icon-rework.h` | Icon sheet encoding (`ICON_SHEET_*`, `ICON_SHEET`, `ICON_INDEX`) |
| `include/configs/configs.h` | `CONFIG_PR_ITEM_ICON` (`:27`), reserved item indices `CONFIG_ITEM_INDEX_*` (`:50-54`) |
| `Data/DesignerConfig/designer-config.c` | Runtime flags: `custom_staves` (`:39`), `item_effect_revamp` (`:75`), `modular_staff_exp` (`:100`), all `true` |
| `Data/CustomItems/CustomItems.event` | Installer list for custom items and staves (included from `Data/Data.event:28`) |
| `Data/CustomItems/<Name>/` | One folder per custom item or staff: `<Name>.c`, `<Name>.lyn.event`, `Installer.event`, icon `.png`/`.4bpp` |
| `Data/SkillSys/Source/SkillTable-item.c` | Item-held skill table `gConstSkillTable_Item` (read by `Kernel/Wizardry/SkillSys/kernel/SkillList.c:362-369`) |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick an item index. Add `ITEM_<NAME> = 0xNN` to `Tools/FE-CLib-Mokha/include/constants/items.h`. In that file the highest used value is `ITEM_KNIFE_IRON = 0xE6` (`:262`), so 0xE7 to 0xFE are free. Avoid 0xBC, 0xBE and 0xDA, which are `CONFIG_ITEM_INDEX_*` in `include/configs/configs.h:50-54`. Avoid 0xBF (see the comment at `Data/ItemSys/Source/Items.c:7`). The item index is the low byte of an item word (`ITEM_INDEX`, `bmitem.h:156`).
3. Add the messages to `Contents/Texts/Source/texts/Items.txt`: `## MSG_ITEM_<NAME>_NAME`, `## MSG_ITEM_<NAME>_DESC`, `## MSG_ITEM_<NAME>_USEDESC`, each followed by text ending in `[X]`. `[N]` is a newline. Message ids are assigned in file order (see Gotchas). `make` regenerates `Contents/Texts/build/msgs.h`, which `include/constants/texts.h` includes.
4. Pick an icon. `iconId` is `(sheet << 8) | index` (`include/kernel/icon-rework.h`). Vanilla items use sheet 0. Either reuse an index already used by another row (`grep -n iconId Data/ItemSys/Source/Items.c`), or add art (step 4a).
   - 4a. New art: a `Data/CustomItems/<Name>/Installer.event` writes the 4bpp to `CONFIG_PR_ITEM_ICON + ITEM_ICON_<NAME> * 0x80` (pattern: `Data/CustomItems/SlowStaff/Installer.event`). Add the icon define to `include/constants/item-icons.h`. Add `#include "<Name>/Installer.event"` to `Data/CustomItems/CustomItems.event`.
5. Add the row to `gItemData_New` in `Data/ItemSys/Source/Items.c`. `.number` must equal the index. Set `.useEffectId` to an IER id (step 6). An item with `useEffectId` 0 cannot be used (Gotchas).
6. Add the IER id to the enum in `include/kernel/item-sys.h`. Append it after the `#ifdef CONFIG_ITEM_INDEX_ARMS_SCROLL` block. The next free enumerator values are 0x5C, 0x5D and 0x5E (`IER_MAX = 0x60`, and `IER_TONIC` is the macro 0x5F, see Gotchas). Check the value stays below `IER_MAX` (`IER_IS_VALID`, `:140`).
7. Declare any new function in `include/kernel/item-sys.h` with the matching macro: `IER_DECLEARE_Usability`, `IER_DECLEARE_PrepUsability`, `IER_DECLEARE_Effect`, `IER_DECLEARE_Action`, `IER_DECLEARE_PrepEffect` (`:214-219`). Reusing a function that is already declared needs nothing.
8. Add a row to `IERevampTable[IER_MAX]` in `Kernel/Data/ItemSys/Source/IERevampTable.c`. The fields are, in order, `usability`, `prep_usability`, `effect`, `action_effect`, `prep_effect`. Use `NULL` for any you do not need.
9. Usable item (map use): `usability` gates the menu. `effect` is needed only if the item targets something; `NULL` uses `SetItemUseAction`. `action_effect` does the work. Heal items: reuse `IER_Usability_Vulnerary` and `IER_Action_VulneraryItem` and set `.healByte` (template A).
10. Prep-screen use (optional): set `prep_usability` and `prep_effect` (Tonic: `IER_Usability_Tonic` and `IER_PrepEffect_Tonic`, `IER-components.c:672`).
11. Staff: set `.attributes = IA_STAFF`, `.weaponType = ITYPE_STAFF`, `.weaponRank` and `.weaponExp` (template B). Then:
    - `usability`: check `gpKernelDesignerConfig->custom_staves` for custom staves, then `return HasSelectTarget(unit, MakeTargetListFor<Name>);`.
    - `effect`: `DoUseAttackStaff(unit, MakeTargetListFor<Name>);` for status/attack staves, `DoUseHealStaff` for heal-type, or `SetStaffUseAction(unit)` when the action needs no target list (`IER_Effect_CommonStaff`, `IER-components.c:389`).
    - Target list: write `MakeTargetListFor<Name>` with `InitTargets`, `AddMapForItem(unit, ITEM_STAFF_<NAME>)` and `ForEachAdjacentUnit` or `ForEachUnit`. Declare it in `include/jester_headers/custom-functions.h`.
    - EXP: add `case ITEM_STAFF_<NAME>: exp = N; break;` to `StaffEXP` (`BattleExp.c:18`). There is no default case, so a staff left out gets 0 EXP.
12. Status staff that reuses `IER_Action_CustomStaves`: add a `case ITEM_STAFF_<NAME>:` to the switch in `ExecCustomStaves` (`IER-extra.c:638`).
13. AI (optional). A self-heal item needs its `useEffectId` added to the switch in `AiTryHealSelf` (`IER-extra.c:248`). A new staff's AI behaviour depends on `GetAiStaffFuncIndex`, which is vanilla. Unverified whether it returns a usable index for new ids.
14. Build with `make`. It regenerates the `*.lyn.event` files from the `.o` (`Tools/FE-CLib-Mokha/wizardry.mk:31-32`). Testing is not covered here.

## Template

### A. Usable item (heal), adapted from `ITEM_VULNERARY`

Adapted from `ITEM_VULNERARY` (`Data/ItemSys/Source/Items.c:1740`) and `[IER_VULNERARY]` (`Kernel/Data/ItemSys/Source/IERevampTable.c:455`). It reuses `IER_Usability_Vulnerary` (`IER-components.c:133`) and `IER_Action_VulneraryItem` (`IER-components.c:555`), so it needs no new C.

`include/kernel/item-sys.h`, inside `enum chax_ierevamp_use_effects`, after the `IER_ARMS_SCROLL` block:

```c
	IER_EXAMPLE_HERB,	/* 0x5C when CONFIG_ITEM_INDEX_ARMS_SCROLL is defined */
```

`Tools/FE-CLib-Mokha/include/constants/items.h`, in the `ITEM_*` enum:

```c
    ITEM_EXAMPLE_HERB      = 0xE7,
```

`Data/ItemSys/Source/Items.c`, in `gItemData_New[255]`:

```c
	[ITEM_EXAMPLE_HERB] = {
		.nameTextId = MSG_ITEM_EXAMPLE_HERB_NAME,
		.descTextId = MSG_ITEM_EXAMPLE_HERB_DESC,
		.useDescTextId = MSG_ITEM_EXAMPLE_HERB_USEDESC,
		.number = ITEM_EXAMPLE_HERB,
		.weaponType = ITYPE_ITEM,
		.maxUses = 3,
		.encodedRange = 0x11,
		.costPerUse = 100,
		.iconId = 0x6a,			/* vulnerary icon, reused */
		.healByte = 15,			/* added to GetItemMight by GetUnitItemHealAmount */
		.useEffectId = IER_EXAMPLE_HERB,
	},
```

`Kernel/Data/ItemSys/Source/IERevampTable.c`, in `IERevampTable[IER_MAX]`:

```c
	[IER_EXAMPLE_HERB] = {
		.usability      = IER_Usability_Vulnerary,
		.prep_usability = NULL,
		.effect         = NULL,
		.action_effect  = IER_Action_VulneraryItem,
		.prep_effect    = NULL,
	},
```

`Contents/Texts/Source/texts/Items.txt`, appended:

```
## MSG_ITEM_EXAMPLE_HERB_NAME
Herb[X]
## MSG_ITEM_EXAMPLE_HERB_DESC
Restores a little HP.[X]
## MSG_ITEM_EXAMPLE_HERB_USEDESC
Restores a little HP.[X]
```

Custom-effect variant. For a new behaviour, declare the functions in `include/kernel/item-sys.h` (`IER_DECLEARE_Usability(IER_Usability_Example);`, `IER_DECLEARE_Action(IER_Action_Example);`) and define them in a new `Data/CustomItems/<Name>/<Name>.c`. Use the signatures below and the proc pattern of `IER_Action_Again` (`Data/CustomItems/AgainStaff/AgainStaff.c:105-108`):

```c
bool IER_Usability_Example(struct Unit *unit, int item)
{
	return CanUnitUseHealItem(unit);	/* placeholder: return false to grey out */
}

void IER_Action_Example(ProcPtr proc, struct Unit *unit, int item)
{
	/* placeholder: start the effect on proc, e.g. Proc_StartBlocking(...) */
}
```

Prep-screen use: a prep-usable item also needs `.prep_usability` and `.prep_effect`. The Tonic is the example: `.prep_usability = IER_Usability_Tonic` and `.prep_effect = IER_PrepEffect_Tonic`, which starts `ProcScr_PrepItemUseBooster` (`IER-components.c:672-675`).

### B. Staff (status staff), adapted from `ITEM_STAFF_SLOW`

Adapted from `ITEM_STAFF_SLOW` (`Data/ItemSys/Source/Items.c:3011`), `[IER_STAFF_SLOW]` (`IERevampTable.c:180`), `IER_Usability_Slow` and `IER_Effect_Slow` (`IER-components.c:230-234`, `:359-362`), `MakeTargetListForSlow` (`Kernel/Wizardry/WeaponRange/Source/StaffRange.c:253-281`) and `ExecCustomStaves` (`IER-extra.c:638`). The staff applies a status to an adjacent enemy and is gated by `custom_staves`.

`Tools/FE-CLib-Mokha/include/constants/items.h`:

```c
    ITEM_STAFF_EXAMPLE     = 0xE8,
```

`include/kernel/item-sys.h`, in the IER enum, after the last new entry from template A:

```c
	IER_STAFF_EXAMPLE,
```

`include/kernel/item-sys.h`, with the other declarations:

```c
IER_DECLEARE_Usability(IER_Usability_Example);
IER_DECLEARE_Effect(IER_Effect_Example);
```

`Data/ItemSys/Source/Items.c`:

```c
	[ITEM_STAFF_EXAMPLE] = {
		.nameTextId = MSG_ITEM_EXAMPLE_STAFF_NAME,
		.descTextId = MSG_ITEM_EXAMPLE_STAFF_DESC,
		.useDescTextId = MSG_ITEM_EXAMPLE_STAFF_USEDESC,
		.number = ITEM_STAFF_EXAMPLE,
		.weaponType = ITYPE_STAFF,
		.attributes = IA_STAFF,
		.maxUses = 15,
		.hit = 100,
		.encodedRange = 0x11,	/* high nibble = min range, low nibble = max range; 0x11 = 1-1 */
		.costPerUse = 500,
		.iconId = ITEM_ICON_STAFF_SLOW,	/* placeholder: reuse, or add art (template step 4a) */
		.weaponRank = WPN_EXP_C,
		.weaponExp = 4,
		.useEffectId = IER_STAFF_EXAMPLE,
	},
```

`Kernel/Data/ItemSys/Source/IERevampTable.c`:

```c
	[IER_STAFF_EXAMPLE] = {
		.usability      = IER_Usability_Example,
		.prep_usability = NULL,
		.effect         = IER_Effect_Example,
		.action_effect  = IER_Action_CustomStaves,
		.prep_effect    = NULL,
	},
```

`Data/CustomItems/<Name>/<Name>.c` (new file, `<Name>` = Example):

```c
#include "common-chax.h"
#include "item-sys.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "constants/items.h"
#include "jester_headers/custom-functions.h"

static void TryAddUnitToExampleTargetList(struct Unit *unit)
{
	if (AreUnitsAllied(gSubjectUnit->index, unit->index))
		return;

	if (unit->statusIndex != UNIT_STATUS_NONE)
		return;

	AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

void MakeTargetListForExample(struct Unit *unit)
{
	int x = unit->xPos;
	int y = unit->yPos;

	gSubjectUnit = unit;
	InitTargets(x, y);

	if (!gpKernelDesignerConfig->custom_staves)
		return;

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_EXAMPLE);
	ForEachAdjacentUnit(x, y, TryAddUnitToExampleTargetList);
}

bool IER_Usability_Example(struct Unit *unit, int item)
{
	if (!gpKernelDesignerConfig->custom_staves)
		return false;

	return HasSelectTarget(unit, MakeTargetListForExample);
}

void IER_Effect_Example(struct Unit *unit, int item)
{
	DoUseAttackStaff(unit, MakeTargetListForExample);
}
```

`include/jester_headers/custom-functions.h`:

```c
extern void MakeTargetListForExample(struct Unit *unit);
```

`Kernel/Wizardry/BattleSys/Source/BattleExp.c`, in `StaffEXP`:

```c
    case ITEM_STAFF_EXAMPLE:
        exp = 30;
        break;
```

`Kernel/Wizardry/ItemSys/IERevamp/Source/IER-extra.c`, in the switch of `ExecCustomStaves`:

```c
	case ITEM_STAFF_EXAMPLE:
        SetUnitStatus(unit_tar, UNIT_STATUS_POISON);	/* pick the status to apply */
        break;
```

`Data/CustomItems/<Name>/Installer.event` (the PUSH/POP block is only needed for new art):

```
#include "Example.lyn.event"

PUSH
    ORG CONFIG_PR_ITEM_ICON + ITEM_ICON_STAFF_EXAMPLE * 0x80
        #incbin "Example.4bpp"
POP
```

`Data/CustomItems/CustomItems.event`:

```
#include "Example/Installer.event"
```

## Worked examples

- `ITEM_VULNERARY` / `IER_VULNERARY` (`Data/ItemSys/Source/Items.c:1740`, `IERevampTable.c:455`): plain map heal. Usability `IER_Usability_Vulnerary` (`IER-components.c:133`), action `IER_Action_VulneraryItem` (`IER-components.c:555`), which calls `GetUnitItemHealAmount` and `HealAmountGetter` before `ExecVulneraryItem` (`Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c:3521`).
- `ITEM_TONIC` / `IER_TONIC` (`Data/ItemSys/Source/Items.c:3500`): a stat item that is also prep-usable and chapter-scoped. The tonic variant is stored in the item's use byte (`ITEM_USES`, `bmitem.h:157`). `ExecStatBoostItem` (`IER-extra.c:304`) and `ApplyStatBoostItem` (`IER-extra.c:335`) record it, and `IER_PrepEffect_Tonic` (`IER-components.c:672`) is the prep path. See [Tonics.md](../Features/Tonics.md).
- `ITEM_STAFF_SLOW` / `IER_STAFF_SLOW` (`Data/ItemSys/Source/Items.c:3011`): a custom status staff. Target list `MakeTargetListForSlow` (`StaffRange.c:268`), effect `IER_Effect_Slow`, action through `ExecCustomStaves` (`IER-extra.c:638`). Its icon is installed by `Data/CustomItems/SlowStaff/Installer.event`.
- `Data/CustomItems/AgainStaff/AgainStaff.c`: a custom staff that runs its own proc chain and shows the EXP bar. `IER_Usability_Again` (`:85`), `IER_Effect_Again` (`:93`), `IER_Action_Again` (`:105`). Installer: `Data/CustomItems/AgainStaff/Installer.event`.

## Gotchas

- An item whose `useEffectId` is 0 (`IER_START`, the same as `IER_INVALID`) cannot be used. `GetIERevamp` returns NULL unless `IER_IS_VALID` (`item-sys.h:140`, `IER-core.c:6-21`). `CanUnitUseItem` then returns false (`IER-core.c:24-40`).
- A new IER id with no row in `IERevampTable.c` is an all-NULL row, so the item is unusable (`IER-core.c:35-37`).
- `IER_TONIC` is both an enumerator (`item-sys.h:128`, value 0x5A by counting the enum) and a macro `IER_MAX - 1` (`item-sys.h:137`, value 0x5F). `Items.c` and `IERevampTable.c` use the macro. Do not use 0x5F for a new id. Free values are 0x5C to 0x5E. The enumerator count depends on whether `CONFIG_ITEM_INDEX_ARMS_SCROLL` and `CONFIG_ITEM_INDEX_MAG_BOOSTER` are defined; both are defined in `include/configs/configs.h`.
- Heal amounts from `healByte` only apply when `item_effect_revamp` is on (`IER-extra.c:132-140`). The flag is `true` in `Data/DesignerConfig/designer-config.c:75`. With the flag off, the vanilla id switch applies and an unknown id heals 0.
- An item whose `useEffectId` is not in the heal switch heals 0 unless `healByte` or `.might` is set (`IER-extra.c:132-160`).
- The AI self-heal path (`AiTryHealSelf`, `IER-extra.c:248`) matches only `IER_VULNERARY` and `IER_ELIXIR`. A new self-heal item is never used by it.
- `StaffEXP` has no default case (`BattleExp.c:18-86`). A new staff gets 0 EXP unless it has a `case`. `StaffEXP` is used only when `modular_staff_exp` is on (`BattleExp.c:344-350`, flag `true` at `designer-config.c:100`).
- Staff EXP is granted only when the weapon has `IA_STAFF` (`BattleExp.c:393`). Staff use needs `IA_STAFF` too: `CanUnitUseItem` calls `CanUnitUseStaff` for it (`IER-core.c:26-27`).
- With `custom_staves` off, `MakeTargetListForSlow` returns before adding targets (`StaffRange.c:275-276`), and `IER_Usability_Slow` returns false (`IER-components.c:230-233`). The same gate is needed in any custom staff.
- `ExecCustomStaves` falls through `default: break;` and still calls `BeginBattleAnimations()` (`IER-extra.c:686-690`). A status staff with no `case` plays an animation and applies nothing. This is inferred from the code, not tested.
- `IER_Action_Entrap` calls `ExecRescueStaff` (`IER-components.c:643-646`). It looks like a copy-paste error. Unverified in game.
- `ITEM_STAFF_SILENCE` sets `.useEffectId = 7` as a literal (`Items.c:1371`). That is `IER_STAFF_SILENCE` (`item-sys.h:17`, value 7). The same row sets `.weaponExp = IER_STAFF_SILENCE` (`Items.c:1372`), which looks like a copy error. Unverified in game.
- `Data/ItemSys/ItemSys.event:4-12` writes `useEffectId` values 55 and 57 through a string macro (`PrItemData`), against `CONFIG_PR_ITEM_TABLE`. The live table read by `GetItemData` is `gItemData_New` (`ItemInfos.c:333-335`). The numbers do not match the current enum (LATONA is 0x57, NIGHTMARE is 0x59 by counting). Do not copy this pattern. Unverified whether the block assembles to anything.
- `Data/ItemSys/Source/Items.c:7` says ID 0xBF does not replace text properly. Avoid it. `gItemData_New` has 255 entries (`Items.c:9`), so indices must stay below 0xFF.
- Message ids are assigned in file order from the last explicit `#0x` marker (`Contents/Texts/Source/kernel.txt:3`, `Contents/Texts/Scripts/textprocess-chax.py:150-170`). Adding messages to `Items.txt` moves the ids of every file included after it (`kernel.txt:42` and later). Named `MSG_` references follow `msgs.h`. Raw numeric text ids in code do not. Unverified whether any raw numeric id points past `Items.txt`.
- `iconId` high byte selects the icon sheet (`include/kernel/icon-rework.h`, `Kernel/Wizardry/IconDisplay/Source/Icon.c:4-8`). Tonic icons use sheet 9 (`ITEM_ICON_TONIC_HP` is `0x9E0`). A plain item must not set a sheet byte by accident.
- `Items.lyn.event`, `IERevampTable.lyn.event` and other `*.lyn.event` files are build output from the `.o` (`Tools/FE-CLib-Mokha/wizardry.mk:31-32`). Edit the `.c`, then run `make`. `CLAUDE.md` notes that some `.lyn.event` files have hand edits. Check before regenerating.
- Battle-side weapon effects are selected by `GetItemWeaponEffect` (`WPN_EFFECT_*` in `Tools/FE-CLib-Mokha/include/bmitem.h:109-114`). Call sites found: POISON sets `tar_debuff` (`Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c:333`, inside `BattleHit_InjectNegativeStatus` at `:290`); HPHALVE (`BattleHitAttr.c:15`, `BattleOrder.c:88`); HPDRAIN (`BattleHit.c:511`); DEVIL (`BattleHitAttr.c:55`). `WPN_EFFECT_PETRIFY` exists in the enum, but no call site was found in this pass. Unverified. A new weapon effect needs code at these sites; unverified beyond that.
- Item-held skills are added in `SkillList.c:362-369` for every inventory item up to the first `ITEM_NONE`. The table is `gConstSkillTable_Item`, indexed `iid * 2`, declared in `include/kernel/skill-system.h:121`. `SkillTable-item.c` is currently empty.
- Unverified: whether a new `.c` under `Data/CustomItems/` is compiled without an extra makefile entry. The source list was not traced.
- Unverified: AI use of new staves (`GetAiStaffFuncIndex` is vanilla and has no repo source).
