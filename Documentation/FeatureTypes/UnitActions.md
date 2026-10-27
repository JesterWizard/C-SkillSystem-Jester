# Unit actions and AI actions

> Code: `Kernel/Wizardry/UnitAction/`, `Kernel/Wizardry/AiHack/AiAction/`, `Kernel/Wizardry/AiHack/AiDecide/`, `Kernel/Wizardry/AiHack/AiOptimization/`, `Kernel/Wizardry/AI/`  ·  Entry points: `ApplyUnitAction` (`Kernel/Wizardry/UnitAction/Source/UnitAction.c:36`), `gUnitActionTable` (`Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13`), `CpPerform_PerformAction` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c:6`), `gAiActionTable` (`Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c:4`)  ·  Existing docs: `Documentation/SkillTypes/MenuSkills.md`, `Documentation/SkillTypes/AiSkills.md`, `Documentation/Features/GaidenMagic.md`, `Documentation/Features/MokhaAOE.md`

## What it covers

The two dispatch tables that decide what a unit does on its turn. The unit action table (`gUnitActionTable`, indexed by `gActionData.unitActionType`) is run by `ApplyUnitAction` for player and CPU units. The AI action table (`gAiActionTable`, indexed by `gAiDecision.actionId`) is run by `CpPerform_PerformAction` for CPU units, and it then calls `ApplyUnitAction` when the action needs a unit action. A new unit action is a `CONFIG_UNIT_ACTION_EXPA_*` id with a function. A new AI action is a `CONFIG_AI_ACTION_EXPA_*` id with an exec/idle pair and a decision that selects it. Examples in this repo: `ExecSkill` (menu skills), GaidenMagic combat and staff, Gambit (unit actions), Rescue and Drop, Teleportation (AI actions).

## How the engine calls it

Player path:

1. A menu OnSelected or target-select callback writes `gActionData.unitActionType`, plus `targetIndex`, `xOther` / `yOther` and `unk08` as the action needs them. Examples: `Kernel/Wizardry/SkillEffects/MenuSkills/Arise.c:47-48`, `Kernel/Wizardry/MokhaAOE/EngineHacks/Gambit/GambitTargetSelect/src/TargetSelectCore.c:126-128`, `Kernel/Wizardry/GaidenMagic/Source/BMagMenu.c:331`. The menu-skill route is in [MenuSkills.md](../SkillTypes/MenuSkills.md) (steps 5-7).
2. The vanilla caller invokes `ApplyUnitAction(proc)` (`Kernel/Wizardry/UnitAction/Source/UnitAction.c:36`). It is installed at ROM `0x3200C` by `Kernel/Wizardry/UnitAction/Source/LynJump.event`. The caller is not traced here (unverified, as in MenuSkills.md step 6).
3. `ApplyUnitAction` sets `gActiveUnit` from `gActionData.subjectIndex` (`UnitAction.c:40`) and sets `US_HAS_MOVED` (`:41`).
4. For `UNIT_ACTION_COMBAT` and `UNIT_ACTION_STAFF` it first calls `TryChangeAction` (`:43-48`, body at `:16-33`), which remaps gaiden-magic slots through `TryChangeGaidenMagicAction` (`Kernel/Wizardry/GaidenMagic/Source/Core.c:306`).
5. An id at or above `CONFIG_UNIT_ACTION_AMT` returns `true` at once (`UnitAction.c:50-51`).
6. `memset(&gActionDataExpa, 0, ...)` (`:56`).
7. `it = gpUnitActionTable[gActionData.unitActionType]`. A NULL entry returns `true` (`:58-60`). Otherwise the function returns `it(proc)` (`:62`).
8. The table is `gUnitActionTable` (`Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13-37`), read through `gpUnitActionTable` (`Kernel/Preload/KernelReloc.event:41`). Vanilla ids run to `0x22` (`Tools/FE-CLib-Mokha/include/types.h:322-355`). Expansion ids are in `include/configs/configs.h:16-20`. The table has `CONFIG_UNIT_ACTION_AMT` entries (`0x30`); the generated `UnitActionData.lyn.event` holds 48 entries.
9. An entry is added either as a row in the C table (`UnitActionData.c:34-36`, ExecSkill and GaidenMagic) or by an event macro `InjectUnitAction(index, func)` (`include/kernel/CustomDefinitions.event:23`). Gambit uses the macro at `Kernel/Wizardry/MokhaAOE/EngineHacks/Gambit/GambitAction/GambitAction.event:4`.

Return convention (observed in the examples; the caller is unverified):

- `true`: the action is complete, e.g. `_ActionWait` (`UnitAction.c:66-70`).
- `false`: a blocking child proc runs under `proc` and ends the action, e.g. `ActionGaidenMagicCombat` (`Kernel/Wizardry/GaidenMagic/Source/MagAction.c:97-110`) and `GambitAction` (`Kernel/Wizardry/MokhaAOE/EngineHacks/Gambit/GambitAction/src/GambitActionCore.c:373-396`).
- `ActionExecSkill` returns what the skill's own function returns (`Kernel/Wizardry/SkillSys/kernel/SkillAction.c:4-12`).

AI path:

1. `CpPerform_PerformAction` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c:6-34`) is hooked at ROM `0x3A4E4` by `Kernel/Wizardry/AiHack/AiAction/Source/LynJump.event`. For an id at or above `CONFIG_AI_ACTION_AMT` it sets the idle to `AiDummyAction` and returns (`AiAction.c:18-21`). Otherwise it sets `proc->func` to the row's `.idle` (`:23-27`) and runs the row's `.exec` if it is set (`:29-32`).
2. The table is `gAiActionTable` (`Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c:4-73`). The struct is `AiActionConf` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.h:5-8`), and the pointer is in `Kernel/Preload/KernelReloc.event:38`.
3. A decision is written by `AiSetDecision(xMove, yMove, actionId, targetId, itemSlot, xTarget, yTarget)` (`Tools/FE-CLib-Mokha/include/cp_common.h:188`), which fills `gAiDecision` (`cp_common.h:24-39`). Rescue and Drop are decided in `AiTryRescueWeakAdjacentAlly` (`Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:193-210`) and `AiTryRescueCarryDrop` (`:212`). Both are reached from `DecideHealOrEscape` only when `rescue_drop_ai_use` is set (`AiOptimization.c:759`).
4. A second decision hook is `gPreAi2DecideHook` (`Kernel/Wizardry/AiHack/AiDecide/data.event:6`). `AiTryExecScriptB` runs it and stops when an entry returns `true` (`Kernel/Wizardry/AiHack/AiDecide/Source/PreAiDecide.c:49-52`). Example: `Ai2Decide_TryTeleportation` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiDecide.c:34-40`).
5. The Rescue exec, `AiStartRescueAction` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c:79-91`), sets the subject, sets `UNIT_ACTION_RESCUE`, moves the unit and calls `ApplyUnitAction`. An AI action that reuses an existing unit action needs no new unit id.
6. The camera and target cursor are set by `CpPerform_MoveCameraOntoTarget` (`Kernel/Wizardry/AiHack/AiAction/Source/AiActionCursor.c:49`, onward), a switch on `gAiDecision.actionId`. An id with no case returns at the `default:` label (`AiActionCursor.c:74-79`, under `#if CHAX`). Rescue (`:150-161`) and Drop (`:163`) have cases.
7. `CpPerform_Cleanup` (`Kernel/Wizardry/AiHack/AiAction/Source/AiPerformEnd.c:3`, onward) reads `gActionData.xMove` / `yMove` (`:34-35`) and falls back to the unit's position when they are out of range (`:42-46`).

Id ranges:

- Unit actions: vanilla `0x01-0x22` (`types.h:322-355`). Expansion: `0x23` ExecSkill, `0x24` GaidenMagicCombat, `0x25` GaidenMagicStaff, `0x26` Gambit (`configs.h:16-19`). Free: `0x27-0x2F` (`CONFIG_UNIT_ACTION_AMT` = `0x30`, `configs.h:20`).
- AI actions: vanilla `0-13` (`cp_common.h:140-153`). Expansion: `14` Teleportation, `15` MenuSkill, `16` Rescue, `17` Drop (`configs.h:11-14`). Free: `18-19` (`CONFIG_AI_ACTION_AMT` = 20, `configs.h:15`).

## Where to look

| File | Role |
|---|---|
| `include/configs/configs.h:11-20` | `CONFIG_AI_ACTION_EXPA_*`, `CONFIG_AI_ACTION_AMT`, `CONFIG_UNIT_ACTION_EXPA_*`, `CONFIG_UNIT_ACTION_AMT`. |
| `include/kernel/CustomDefinitions.event:20-23` | `InjectAiAction` and `InjectUnitAction` macros. |
| `include/kernel/action-expa.h` | `gActionDataExpa` (`:4-8`), `UnitActionFunc_t` (`:9`), `gpUnitActionTable`. |
| `Kernel/Wizardry/UnitAction/Source/UnitAction.c` | `ApplyUnitAction` (`:36-63`), `TryChangeAction` (`:16-33`), `_ActionWait` (`:66-70`). |
| `Kernel/Wizardry/UnitAction/Source/UnitActionData.c` | `gUnitActionTable` (`:13-37`). |
| `Kernel/Wizardry/UnitAction/Source/UnitActionData.lyn.event` | Generated table (48 entries). Committed build output. |
| `Kernel/Wizardry/UnitAction/Source/LynJump.event` | Hook at ROM `0x3200C`. |
| `Kernel/Wizardry/UnitAction/data.event`, `UnitAction.event` | Include the table and the hook. |
| `Kernel/Preload/KernelReloc.event` (lines 38, 41) | `gpAiActionTable`, `gpUnitActionTable`. |
| `Tools/FE-CLib-Mokha/include/types.h:322-356` | Vanilla `UNIT_ACTION_*` values. |
| `Tools/FE-CLib-Mokha/include/cp_common.h` | `struct AiDecision` (`:24-39`), `AI_ACTION_*` enum (`:138-154`), `AiSetDecision` (`:188`). |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c` | `CpPerform_PerformAction` (`:6-34`), AI exec functions (`:36-125`). |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c` | `gAiActionTable` (`:4-73`). Generated `AiActionConf.lyn.event` holds 20 entries. |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiAction.h` | `AiActionConf` struct and exec prototypes. |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiActionCursor.c` | `CpPerform_MoveCameraOntoTarget` (`:49`), per-id camera and cursor. |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiPerformEnd.c` | `CpPerform_Cleanup`. |
| `Kernel/Wizardry/AiHack/AiAction/Source/LynJump.event` | AI hooks at ROM `0x3A4E4` and others. |
| `Kernel/Wizardry/AiHack/AiDecide/data.event` | `gPreAi1DecideHook`, `gPreAi2DecideHook` lists. |
| `Kernel/Wizardry/AiHack/AiDecide/Source/PreAiDecide.c` | Runs the decide hooks (`:25-28`, `:49-52`). |
| `Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c` | Rescue and Drop decisions (`:193-250`), `DecideHealOrEscape` (`:753`). |
| `Kernel/Wizardry/AI/Source/AI.c` | AI unit ordering (`BuildAiUnitList` `:310`, `SortAiUnitList` `:349`). Not action code. |
| `Kernel/Wizardry/SkillSys/kernel/SkillAction.c` | ExecSkill (`0x23`), dispatch by SID. |
| `Kernel/Wizardry/SkillEffects/AiSkills/` | AI actions and decisions for skills (Teleportation, MiscAiSkills). |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick ids. A unit action takes a free value in `0x27-0x2F`. An AI action takes `18` or `19`, or `CONFIG_AI_ACTION_AMT` is raised (`configs.h:15`). Unit and AI ids are separate spaces. Grep the repo for the value before you use it.
3. Define the ids in `include/configs/configs.h`, next to the other `CONFIG_*_ACTION_EXPA_*` lines.
4. Write the unit action function `bool ActionX(ProcPtr proc)` in `Kernel/Wizardry/<Feature>/Source/<Feature>.c`, non-static. Return `true` when the action is complete, or start a blocking child and return `false` (see Template).
5. Register it. Either add a row to `gUnitActionTable` in `Kernel/Wizardry/UnitAction/Source/UnitActionData.c` with a prototype above the table, or add `InjectUnitAction(CONFIG_UNIT_ACTION_EXPA_X, ActionX)` to the feature's `.event` (`GambitAction.event:4`). The GaidenMagic lines at `GaidenMagic.event:14-15` are commented out because the C table already has them.
6. Set `gActionData.unitActionType` where the player or the AI chooses the action (menu OnSelected or target-select callback, see the Template). Set `targetIndex`, `xOther`, `yOther` and `unk08` as the action reads them.
7. For an AI action, add `CONFIG_AI_ACTION_EXPA_<Name>` in `configs.h`, a prototype in `AiAction.h`, and a row in `AiActionConf.c` with `.exec` and `.idle`. The other route is an `InjectAiAction(index, exec, idle)` line in the feature's `.event` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation.event:7`). The exec sets `gActionData` and calls `ApplyUnitAction`, as in `AiStartRescueAction`.
8. If the AI action should move the camera and show a cursor, add a `case` to the switch in `CpPerform_MoveCameraOntoTarget` (`AiActionCursor.c`), next to the Rescue and Drop cases.
9. Add the decision: a `bool` function that calls `AiSetDecision` and returns `true` when it chose the action. Hook it into `gPreAi2DecideHook` (`AiDecide/data.event:6`) or into a decide routine such as `DecideHealOrEscape`, and gate it behind a designer flag (Rescue uses `rescue_drop_ai_use`).
10. Build with `make`. The `.lyn.event` tables are regenerated. They are committed build output, so check for hand edits before regenerating (CLAUDE.md).
11. Write `Documentation/Features/<Feature>.md` and add a bullet to the root `README.md` (preamble, step 6).

## Template

### Unit action

`include/configs/configs.h`, with the other `CONFIG_UNIT_ACTION_EXPA_*` lines:

```c
#define CONFIG_UNIT_ACTION_EXPA_Example 0x27
```

**Kernel/Wizardry/Example/Source/Example.c** (placeholder path). The blocking shape is adapted from `ActionGaidenMagicCombat` (`Kernel/Wizardry/GaidenMagic/Source/MagAction.c:97-110`, `Proc_StartBlocking` then `return false`) and the `PROC_REPEAT` wait from `Kernel/Wizardry/BattleSys/HpCost/Source/MapAnimHpCost.c:13-25`:

```c
#include "common-chax.h"
#include "action-expa.h"

/* gActiveUnit is the subject (set by ApplyUnitAction, UnitAction.c:40). */

static void ExampleAction_Begin(ProcPtr proc)
{
	/* The effect goes here: change map or unit state, start an anim, etc. */
}

static void ExampleAction_Wait(ProcPtr proc)
{
	/* Break once the effect (and any anim) has finished. */
	Proc_Break(proc);
}

STATIC_DECLAR const struct ProcCmd ProcScr_ExampleAction[] = {
	PROC_CALL(ExampleAction_Begin),
	PROC_REPEAT(ExampleAction_Wait),
	PROC_END,
};

/* Called from gUnitActionTable through ApplyUnitAction. */
bool ActionExample(ProcPtr proc)
{
	if (!UNIT_IS_VALID(gActiveUnit))
		return true;

	Proc_StartBlocking(ProcScr_ExampleAction, proc);
	return false;
}
```

Table row, `Kernel/Wizardry/UnitAction/Source/UnitActionData.c`, with a prototype above the table next to `ActionGaidenMagicCombat` (`:10`):

```c
bool ActionExample(ProcPtr proc);

	[CONFIG_UNIT_ACTION_EXPA_Example] = ActionExample,
```

Or, from an event file of the feature: `InjectUnitAction(CONFIG_UNIT_ACTION_EXPA_Example, ActionExample)`.

Menu side. This is shaped after `Arise_OnSelected` (`Kernel/Wizardry/SkillEffects/MenuSkills/Arise.c:39-51`) without its usability check. Wiring it to a menu row is not shown. For a Skills-submenu row see [MenuSkills.md](../SkillTypes/MenuSkills.md):

```c
u8 Example_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_Example;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}
```

### AI action

`include/configs/configs.h`, with the other `CONFIG_AI_ACTION_EXPA_*` lines:

```c
#define CONFIG_AI_ACTION_EXPA_Example 18
```

`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.h`:

```c
void AiStartExampleAction(struct CpPerformProc *proc);
```

`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c`, adapted from `AiStartRescueAction` (`:79-91`). It reuses `UNIT_ACTION_RESCUE` in the Rescue exec; the line marked below is the new-unit-action variant:

```c
void AiStartExampleAction(struct CpPerformProc *proc)
{
	gActionData.subjectIndex = gActiveUnitId;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_Example;	/* or UNIT_ACTION_RESCUE to reuse one */
	gActionData.targetIndex = gAiDecision.targetId;
	gActionData.xOther = gAiDecision.xTarget;
	gActionData.yOther = gAiDecision.yTarget;

	gActiveUnit->xPos = gAiDecision.xMove;
	gActiveUnit->yPos = gAiDecision.yMove;

	ApplyUnitAction(proc);
}
```

`Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c`, row adapted from the Rescue row (`:33-36`):

```c
	[CONFIG_AI_ACTION_EXPA_Example] = {
		.exec = AiStartExampleAction,
		.idle = AiDummyAction,
	},
```

`Kernel/Wizardry/AiHack/AiAction/Source/AiActionCursor.c`, in the switch of `CpPerform_MoveCameraOntoTarget`, next to the Drop case (`:163-166`):

```c
	case CONFIG_AI_ACTION_EXPA_Example:
		x = gAiDecision.xTarget;
		y = gAiDecision.yTarget;
		break;
```

Decision, a sketch in the style of `Ai2Decide_TryTeleportation` (`TeleportationAiDecide.c:34-40`). Its gate on `gAiScriptKind` is the same as Teleportation's:

```c
bool AiTryExample(void)
{
	if (gAiScriptKind != AI_SCRIPT_AI2)
		return false;

	/* choose targetId, xTarget, yTarget here; return false when none */
	AiSetDecision(gActiveUnit->xPos, gActiveUnit->yPos, CONFIG_AI_ACTION_EXPA_Example, 0, 0, 0, 0);
	return true;
}
```

Hook: add `POIN AiTryExample` to `gPreAi2DecideHook` in `Kernel/Wizardry/AiHack/AiDecide/data.event`, above its `WORD 0 0 0 0` terminator (`:5-7`).

## Worked examples

- `Kernel/Wizardry/SkillSys/kernel/SkillAction.c`: ExecSkill (`0x23`). One unit action that dispatches to a per-SID function through `gpSkillActionFuncTable`.
- `Kernel/Wizardry/GaidenMagic/`: GaidenMagicCombat (`0x24`, `Source/MagAction.c:97-110`) and GaidenMagicStaff (`0x25`, `:112`). Setters at `Source/BMagMenu.c:331` and `Source/Core.c:323-325`. The slot remap is `TryChangeGaidenMagicAction` (`Source/Core.c:306`).
- `Kernel/Wizardry/MokhaAOE/EngineHacks/Gambit/`: Gambit (`0x26`). Injected with `InjectUnitAction` (`GambitAction.event:4`), a blocking child proc (`src/GambitActionCore.c:373-396`), and a target-select callback that sets the type (`GambitTargetSelect/src/TargetSelectCore.c:124-129`).
- `Kernel/Wizardry/AiHack/AiAction/`: AI Rescue (`16`) and Drop (`17`). Exec and rows at `Source/AiAction.c:79-105` and `Source/AiActionConf.c:33-40`. Decisions at `Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:193-250`.
- `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/`: AI Teleportation (`14`). Injected with `InjectAiAction` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation.event:7`), decided through `gPreAi2DecideHook` (`TeleportationAiDecide.c:34-40`).
- `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c:329-353`: AI MenuSkill (`15`). Its exec sets `CONFIG_UNIT_ACTION_EXPA_ExecSkill` and calls `ApplyUnitAction`. Its idle, `AiAction_MenuSkillIdle` (`:351-353`), replaces `proc->func` with `AiAction_MenuSkillIdleExt` (`:343-350`).

## Gotchas

- Verified: an id at or above `0x30` makes `ApplyUnitAction` set `US_HAS_MOVED` (`UnitAction.c:41`) and return `true` with no action (`:50-51`). The unit is marked as moved and nothing runs.
- Verified: a NULL table row also returns `true` with no action (`UnitAction.c:58-60`). A forgotten row fails silently.
- Verified: `gActionDataExpa` is zeroed just before dispatch (`UnitAction.c:56`). Set per-action flags inside the action function, not before calling `ApplyUnitAction`.
- Verified: several MenuSkills files have `#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill` with a `#define ... 20` fallback (e.g. `Kernel/Wizardry/SkillEffects/MenuSkills/ShadowFlash.c:13-15`). The fallback is dead: `common-chax.h:7` includes `configs/configs.h`, which defines the id as `0x23` (`configs.h:16`). Copy the configs.h value.
- Verified: an AI id at or above `CONFIG_AI_ACTION_AMT` is replaced by `AiDummyAction` (`AiAction.c:18-21`), so the unit does nothing that turn.
- Verified: `CpPerform_MoveCameraOntoTarget` returns for any id without a case (`AiActionCursor.c:74-79`). Without a case the AI action runs with no camera move and no target cursor.
- Verified: the Rescue and Drop exec functions do not set `gActionData.xMove` / `yMove` (`AiAction.c:79-105`), which `CpPerform_Cleanup` reads (`AiPerformEnd.c:34-35`). Whether a new AI action needs them is unverified. `cleanupX` falls back to the unit's position when out of range (`:42-46`).
- Verified: AI ids and unit ids are separate. Rescue reuses `UNIT_ACTION_RESCUE` (`AiAction.c:82`) and MenuSkill reuses `CONFIG_UNIT_ACTION_EXPA_ExecSkill` (`MiscAiSkills.c:329-341`). A new AI action needs a new unit id only if it has its own unit-action function.
- Verified: Rescue and Drop decisions run only with `rescue_drop_ai_use` (`AiOptimization.c:759`; default in `Data/DesignerConfig/designer-config.c:127`). A new AI decision with no flag runs for every CPU unit.
- Verified: `CONFIG_AI_ACTION_EXPA_Teleportation` must keep its value, because the `InjectAiAction` line in `Teleportation.event:7` uses the same constant (`configs.h:11` says so in its comment).
- Verified: the AI table is sized by `CONFIG_AI_ACTION_AMT`. The generated `AiActionConf.lyn.event` holds 20 entries (40 words). Raising the constant changes that generated table. Other fixed-size copies of the AI table are unverified.
- Verified: `AiActionConf.lyn.event` and `UnitActionData.lyn.event` are committed build output (CLAUDE.md). Check for hand edits before regenerating.
- Unverified: the vanilla caller of `ApplyUnitAction` for player actions and how it uses the return value (see [MenuSkills.md](../SkillTypes/MenuSkills.md) step 6).
- Unverified: whether `Proc_StartBlocking` children need any end callback beyond `Proc_Break` for the template's shape. The template follows `MapAnimHpCost.c` and `ActionGaidenMagicCombat`, which do not use one.
