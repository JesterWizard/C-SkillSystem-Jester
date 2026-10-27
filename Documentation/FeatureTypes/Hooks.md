# Engine hooks and vanilla replacement

> Code: `Kernel/Wizardry/GameInitHook`, `Kernel/Wizardry/ChapterInitHook`, `Kernel/Wizardry/PrePhaseHook`, `Kernel/Wizardry/PostActionHook`, `Kernel/Wizardry/EndTurnHook`, `Kernel/Wizardry/BeginActionHook`, `Kernel/Wizardry/MapAnimBattleHook`, `Kernel/Wizardry/UnitHooks`, `Kernel/Wizardry/BattleSys`  ·  Entry points: hook lists in `data.event` files read through `gp*` pointers in `Kernel/Preload/KernelReloc.event`; `LYN_REPLACE_CHECK` + `LynJump.event` for vanilla replacement  ·  Existing docs: `Documentation/SkillTypes/PostAction.md`, `Documentation/SkillTypes/PrePhase.md`, `Documentation/SkillTypes/EndTurn.md`, `Documentation/SkillTypes/StatusGetter.md`, `Documentation/SkillTypes/BattleCalc.md`, `Documentation/CONTRIBUTING.md`

## What it covers

Non-skill features that need code at a fixed engine point: game start, chapter start, phase start, after an action, end of turn, unit load, copy, clear or kill, battle calc, AI decision, or a vanilla function that has no hook point. Examples: `Kernel/Wizardry/Chatlog` (a `ChapterInitHooks` entry), `Kernel/Wizardry/PopupIconShine` (a vanilla replacement), `Kernel/Wizardry/FreeMovement` (an external PrePhase slot). Skill-only lists are covered in `Documentation/SkillTypes/`. This file lists every hook point and how to replace a vanilla function.

## How the engine calls it

There are four mechanisms.

1. **Hook list.** A zero-terminated array of function pointers (last word `WORD 0`). The label sits in a `data.event` (or a `.event`) file. The C runner reads it through a `gp*` pointer, and the pointer is a `POIN` in `Kernel/Preload/KernelReloc.event`. Most runners loop `for (it = gp...; *it; it++)`, for example `Kernel/Wizardry/UnitHooks/Source/UnitKill.c:26-27`.
2. **Proc-walked list.** The PrePhase, PostAction, EndTurn and MapAnimBattle lists run through `KernelStartBlockingHookProc` (`Kernel/Wizardry/KernelLib/Source/hook-proc.c:84-93`), declared at `include/kernel/kernel-lib.h:335`. Entries are `bool (*)(ProcPtr)` (`include/common-chax.h:44`). `HookProc_Main` (`hook-proc.c:52-68`) calls them in order. `false` continues in the same frame. `true` stops the walk for this frame, the proc yields, and the walk resumes at the next entry on the next frame, after any blocking child has ended (`hook-proc.c:64-66`). The index is advanced before each call (`hook-proc.c:58`), so an entry that returned `true` is not called again. A `0` entry ends the list and sends the proc to label 2 (`hook-proc.c:58-61`).
3. **External slot.** A single function pointer `gpExternal*` at a fixed reloc address. Feature code may set it at runtime. A `CallExternal*` entry at the end of a list calls it when it is non-zero (see the External table below). Use it when one feature must attach to a list without editing the list.
4. **Vanilla replacement.** A vanilla function is replaced by a C function with a veneer in `LynJump.event`. Most list runners below are reached this way: the C function that runs the list is itself a replacement (for example `UnitKill`, `ClearUnit`, `CopyUnit`, `UnitBeginAction`, `BattleGenerate`, `ComputeBattleUnitStats`).

Install points for the proc-walked and non-replacement lists:

- GameInit: `Kernel/Wizardry/GameInitHook/source/LynJump.event:2` replaces `StartGame` (`ORG $9f08`). The list runs inside `StartGame` (`Kernel/Wizardry/GameInitHook/source/GameInit.c:31-37`).
- ChapterInit: `Kernel/Wizardry/ChapterInitHook/ChapterInitHook.event:4-6` changes the pointer at `ORG $59A254` to `ChapterInitHook`. Which vanilla routine calls that pointer was not traced (unverified).
- PrePhase: `Kernel/Wizardry/PrePhaseHook/PrePhaseHook.event:7-8` (`PROC_CALL_ROUTINE_2(PrePhaseHook)` at `0x59A2C0`, macro in `Tools/FE-CLib-Mokha/include/proc.inc:134`).
- PostAction: `Kernel/Wizardry/PostActionHook/PostActionHook.event:11-12` and `:15-16` (`PROC_JUMP` into `ProcScr_PostActionHookPlayPhase` and `ProcScr_PostActionHookCpPerform`).
- EndTurn: `Kernel/Wizardry/EndTurnHook/EndTurnHook.event:8-9` and `:12-13`.
- MapAnimBattle: `Kernel/Wizardry/MapAnimBattleHook/MapAnimBattleHook.event:6-8` (`ORG 0x07B8BC`, replaces the battle proc script).
- BeginAction: `Kernel/Wizardry/BeginActionHook/Source/LynJump.event:2` replaces `UnitBeginAction` (`ORG $1865c`).

### Starting a proc from a hook

- `Proc_Start(script, parent)` and `Proc_StartBlocking(script, parent)` are declared in `Tools/FE-CLib-Mokha/include/proc.h:136-137`. Both return `ProcPtr`.
- `ProcPtr` is `void*` (`proc.h:6`). Scripts are arrays of `struct ProcCmd` built with the `PROC_*` macros (`proc.h:16-42`). `PROC_CALL(fn)` calls `fn(proc)` (`ProcFunc`, `proc.h:7`). `PROC_NAME` is `proc.h:17`, `PROC_SLEEP` is `:30`, `PROC_YIELD` is `:42`, `PROC_HEADER` (for your own proc struct) is `:46`.
- Blocking: `KernelStartBlockingHookProc` starts its walker with `Proc_StartBlocking(ProcScr_HookCommon, parent)` (`hook-proc.c:88`). The same call is used by `Kernel/Wizardry/KernelLib/Source/event-call.c:33`.
- A list entry that starts a blocking child returns `true`. An entry that starts nothing returns `false`. This rule is from `Documentation/SkillTypes/PostAction.md`.
- How the parent resumes when the child ends is not traced in this repo (unverified).

```c
static const struct ProcCmd ProcScr_ExampleWait[] = {
	PROC_NAME("ExampleWait"),
	PROC_SLEEP(30),
	PROC_END,
};

bool ExampleEntry(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_ExampleWait, parent);
	return true;
}
```

## Where to look

Every hook list in the engine. Labels are in the `data.event` file (or `.event` file) named in the first column. Pointers are in `Kernel/Preload/KernelReloc.event`. Signatures were checked against the C runner, not only the comment. Where the comment and the runner disagree, the runner wins.

| Hook list (label, data.event:line) | Pointer (KernelReloc.event:line) | Signature | When it runs | Return meaning |
|---|---|---|---|---|
| `gGameInitHookTable`, `Kernel/Wizardry/GameInitHook/data.event:3` | `:13` | `void (*)(void)`, `Kernel/Wizardry/GameInitHook/source/GameInit.c:6` | Once at boot, inside `StartGame` (`GameInit.c:12-37`) | None. Every entry runs. |
| `ChapterInitHooks`, `Kernel/Wizardry/ChapterInitHook/data.event:3` | `:12` | `void (*)(ProcPtr proc)`, `Kernel/Wizardry/ChapterInitHook/Source/ChapterInit.c:3`. The comment at `data.event:1` says `void (*)(void)`, which is wrong. | Chapter init. `ChapterInitHook` (`ChapterInit.c:7-16`) runs the list, then `UndeployEveryone()` (`ChapterInit.c:15`). | None. Every entry runs. |
| `gPrePhaseFuncs`, `Kernel/Wizardry/PrePhaseHook/data.event:3` | None. Read directly (`Kernel/Wizardry/PrePhaseHook/Source/PrePhaseHook.c`, lines 5 and 17). | `bool (*)(ProcPtr)` (`data.event:1` comment) | Phase start. `PrePhaseHook` (`PrePhaseHook.c:15-19`), proc-walked. | `false` continues in the same frame. `true` stops for this frame and resumes at the next entry next frame, after any blocking child has ended. `PrePhaseHook_VanillaEnd` (`PrePhaseHook.c:9-13`) starts the vanilla Gorgon-egg hatch display as a blocking child and returns `true`; it is the last entry (`data.event:36`). Add new entries above `CallExternalPrePhaseHook` (`data.event:35`) so they run before the vanilla end. |
| `gPostActionFuncs`, `Kernel/Wizardry/PostActionHook/data.event:3` | `:15` | `bool (*)(ProcPtr)` (`Kernel/Wizardry/PostActionHook/data.event:1`, `include/kernel/post-action.h:5`) | After a unit's action, in the player and CPU phases (`PostActionHook.event:11-16`). Runner `PostActionHook` at `Kernel/Wizardry/PostActionHook/source/PostActionHook.c:7-10`. | Same as PrePhase. `PostActionEnd` (`source/PostActionHook.c:12-23`) returns `true` and is the last entry (`data.event:50`). Add new entries above `//End` (`data.event:46`). See [PostAction.md](../SkillTypes/PostAction.md). |
| `gEndTurnFuncs`, `Kernel/Wizardry/EndTurnHook/data.event:2` | `:57` | `bool (*)(ProcPtr)` (`Kernel/Wizardry/EndTurnHook/Source/EndTurnHook.c:5`) | End of a faction's turn: the player End command and the end of the CPU phase (`EndTurnHook.c:35-45`, `EndTurnHook.event:8-13`). | Same as PrePhase. See [EndTurn.md](../SkillTypes/EndTurn.md). |
| `gBeginActionHooks`, `Kernel/Wizardry/BeginActionHook/BeginActionHook.event:14` (no `BeginActionHook/data.event`) | `:11` | `void (*)(struct Unit *unit)` (`Kernel/Wizardry/BeginActionHook/Source/BeginActionHook.c:7`) | At the end of `UnitBeginAction` (`BeginActionHook.c:121-163`, inside `#if CHAX`). | None. |
| `gPreMapAnimBattleRoundHooks`, `Kernel/Wizardry/MapAnimBattleHook/MapAnimBattleHook.event:12` | `:36` | `bool (*)(ProcPtr)` (`Kernel/Wizardry/MapAnimBattleHook/Source/PreHit.c:27`) | Before each battle round's map anim. `PreMapAnimBattleRound_OnStart` (`PreHit.c:11-28`) inside `ProcScr_MapAnimBattle_Rework` (`PreHit.c:39-71`). | Same as PrePhase (proc-walked). |
| `gLoadUnitHooks`, `Kernel/Wizardry/UnitHooks/data.event:2` | `:20` | `int (*)(struct Unit *unit, const struct CharacterData *character)` (`Kernel/Wizardry/UnitHooks/Source/LoadUnit.c:13`) | Unit load. `UnitLoadStatsFromChracter` (`LoadUnit.c:7-14`) runs the list. Veneer `Kernel/Wizardry/UnitHooks/Source/LynJump.event:1-6` (`ORG $17e34`). | Ignored. The first entry is the vanilla stat loader `UnitLoadStatsFromChracterVanilla` (`UnitHooks/data.event:3`, `LoadUnit.c:17`). |
| `gExternalBattleToUnitHook`, `Kernel/Wizardry/UnitHooks/data.event:11` | `:63` | `int (*)(struct BattleUnit *bu, struct Unit *unit)` (`Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c:17`) | After a battle, in `UpdateUnitFromBattle` (`BattleUnitHook.c:308`). Veneer `UnitHooks/Source/LynJump.event:15-20` (`ORG $2c1ec`). | Ignored. |
| `gExternalUnitToBattleHook`, `Kernel/Wizardry/UnitHooks/data.event:18` | `:64` | `int (*)(struct Unit *unit, struct BattleUnit *bu)` (`BattleUnitHook.c:21`) | When a battle unit is built, in `InitBattleUnit` (`BattleUnitHook.c:236-251`, loop at `:248`). Veneer `UnitHooks/Source/LynJump.event:8-13` (`ORG $2a584`). | Ignored. `UnitToBattle_SetupSkillList` must stay first (`UnitHooks/data.event:19-20`). |
| `gCopyUnitHooks`, `Kernel/Wizardry/UnitHooks/data.event:28` | `:18` | `int (*)(struct Unit *from, struct Unit *to)` (`Kernel/Wizardry/UnitHooks/Source/CopyUnit.c:3`). The comment at `data.event:28` says `void`. | `CopyUnit` after the `memcpy` (`CopyUnit.c:7-21`). Veneer `UnitHooks/Source/LynJump.event:22-27` (`ORG $1781c`). | Ignored. |
| `gClearUnitHooks`, `Kernel/Wizardry/UnitHooks/data.event:33` | `:19` | `int (*)(struct Unit *unit)` (`Kernel/Wizardry/UnitHooks/Source/ClearUnit.c:3`). The comment says `void`. | `ClearUnit`, before the `CpuFill16` (`ClearUnit.c:8-19`). Entries see the unit before it is wiped. Veneer `ORG $177f4` (`UnitHooks/Source/LynJump.event:29-34`). | Ignored. |
| `gKillUnitHooks`, `Kernel/Wizardry/UnitHooks/data.event:39` | `:51` | `int (*)(struct Unit *unit)` (`Kernel/Wizardry/UnitHooks/Source/UnitKill.c:6`) | `UnitKill`, before `AddDeadUnit` (`UnitKill.c:22-30`, loop at `:26`). Veneer `ORG $183fc` (`UnitHooks/Source/LynJump.event:36-41`). | Ignored. |
| `gUnitRefrainHooks`, `Kernel/Wizardry/UnitHooks/data.event:45` | `:99` | `int (*)(struct Unit *unit)` (`Kernel/Wizardry/UnitHooks/Source/UnitRefrain.c:14`) | Once per blue unit inside `ResetAllPlayerUnitState` (`UnitRefrain.c:18-46`, loop at `:39`). Veneer `ORG $85484`. The caller of that function was not traced (unverified). | Ignored. |
| `gNewSaveHooks`, `Kernel/Wizardry/SaveData/data.event:119` | `:16` | `void (*)(void)` (`include/kernel/save-data.h:134`) | New-game save: `WriteNewGameSave` (`Kernel/Wizardry/SaveData/Source/ModularSaveCore.c:160`, loop at `:185`). | None. |
| `PreBattleGenerateFuncs`, `Kernel/Wizardry/BattleSys/data.event:27` | `:42` | `void (*)(void)` (`Kernel/Wizardry/BattleSys/Source/BattleGenerate.c:7`) | Once per battle, from `BattleGenerate` (`BattleGenerate.c:6-21`, which is a replacement, veneer `Kernel/Wizardry/BattleSys/Source/LynJump.event:161`). | None. |
| `PreBattleCalcFuncs`, `Kernel/Wizardry/BattleSys/data.event:4` | `:43` | `void (*)(struct BattleUnit *buA, struct BattleUnit *buB)` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:20`) | `ComputeBattleUnitStats` (`PreBattleCalc.c:2534-2546`, loop at `:2541`). Veneer `ORG $2a95c` (`BattleSys/Source/LynJump.event:2`). The comment at `BattleGenerate.c:1-5` says it runs for both foes. | None. Entries edit the battle units' stats. |
| `BattleDamageCalcFuncs`, `Kernel/Wizardry/BattleSys/data.event:17` | `:9` | `void (*)(struct BattleUnit *buA, struct BattleUnit *buB)` (`Kernel/Wizardry/BattleSys/Source/BattleDamage.c:47`) | Every hit: `BattleHit_CalcDamage` (`BattleDamage.c:106`, loop at `:186`). | None. Entries write to the hit state. |
| `BattleRealDamageCalcFuncs`, `Kernel/Wizardry/BattleSys/data.event:23` | `:10` | `int (*)(int old, struct BattleUnit *buA, struct BattleUnit *buB)` (`BattleDamage.c:50`) | Real damage: `CalcBattleRealDamage` (`BattleDamage.c:53-88`, loop at `:84-85`). | Returned value becomes the running damage. `KernelPad2` is the identity (`Kernel/Wizardry/KernelLib/Source/pad.c:6-9`). |
| `gHealAmountGetters`, `Kernel/Wizardry/ItemSys/ItemSys.event:15` | `:8` | `int (*)(int old, struct Unit *actor, struct Unit *target)` (`Kernel/Wizardry/ItemSys/Source/StaffHeal.c:17`) | `HealAmountGetter` (`StaffHeal.c:20-30`). | Returned value becomes the running amount. The result is clamped to 0..80 (`StaffHeal.c:28`). |
| `gHpGetters`, `gPowGetters`, `gMagGetters`, `gSklGetters`, `gSpdGetters`, `gLckGetters`, `gDefGetters`, `gResGetters`, `gMovGetters`, `gConGetters`, `gAidGetters`. Labels at `Kernel/Wizardry/UnitStatusGetter/data.event:4, 10, 17, 25, 32, 39, 46, 53, 60, 67, 72` | `:22-32` (in order) | `int (int old, struct Unit *unit)` (`include/kernel/status-getter.h:6`) | Every stat read, in each `_GetUnit<Stat>` getter. Example: `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:25` in `_GetUnitPower` (`:11`). Hot path (`Documentation/CONTRIBUTING.md:69`). | Returned value becomes the running stat. |
| `gWeaponRangeGetters`, `Kernel/Wizardry/WeaponRange/data.event:2` | `:34` | `int (*)(int old, struct Unit *unit, u16 item)` (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:12`) | Max range: `GetItemMaxRangeRework` (`RangeGetter.c:34-48`, loop at `:43-44`), only when `IsUnitStruct(unit)`. | Returned value becomes the running max range. |
| `gPreAi1DecideHook`, `Kernel/Wizardry/AiHack/AiDecide/data.event:2` | `:39` | `bool (*)(void)` (`Kernel/Wizardry/AiHack/AiDecide/Source/PreAiDecide.c:5`) | Before the AI1 script: `AiTryExecScriptA` (`PreAiDecide.c:12-33`, loop at `:25`). Veneer `Kernel/Wizardry/AiHack/AiDecide/Source/LynJump.event:2` (`ORG $3c4bc`). | `true`: `AiTryExecScriptA` returns `true` and the AI1 script does not run (`PreAiDecide.c:25-27`). |
| `gPreAi2DecideHook`, `Kernel/Wizardry/AiHack/AiDecide/data.event:5` | `:40` | `bool (*)(void)` (`PreAiDecide.c:5`) | Before the AI2 script: `AiTryExecScriptB` (`PreAiDecide.c:36-57`, loop at `:49`). Veneer `AiDecide/Source/LynJump.event:8` (`ORG $3c54c`). | Same as AI1. |

Table entries (C tables, not `data.event` lists). These are indexed by an id, not walked to the end:

| Table (file:line) | Pointer (KernelReloc.event:line) | Signature | Used by | Notes |
|---|---|---|---|---|
| `gSkillActionFuncTable`, `Kernel/Data/SkillSys/Data/SkillActionInfo.c:5` | `:14` | `bool (*)(ProcPtr)` (`include/kernel/skill-system.h:329`), indexed by SID | `ActionExecSkill` (`Kernel/Wizardry/SkillSys/kernel/SkillAction.c:4-12`) | Skill-specific. Returns `false` when the SID has no entry. |
| `gUnitActionTable`, `Kernel/Wizardry/UnitAction/Source/UnitActionData.c:13` | `:41` | `bool (*)(ProcPtr)` (`include/kernel/action-expa.h:11`), indexed by `gActionData.unitActionType` | `ApplyUnitAction` (`Kernel/Wizardry/UnitAction/Source/UnitAction.c:36-62`, read at `:58`) | Add an entry with `InjectUnitAction(index, func)` (`include/kernel/CustomDefinitions.event:23`). A NULL entry returns `true`. |
| `gAiActionTable`, `Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c:4` | `:38` | `struct AiActionConf` with `.exec` and `.idle` (`AiAction.h:17`) | `CpPerform_PerformAction` (`Kernel/Wizardry/AiHack/AiAction/Source/AiAction.c:6-23`) | Add an entry with `InjectAiAction` (`include/kernel/CustomDefinitions.event:20`). |
| `IconGetters`, `Kernel/Wizardry/IconDisplay/Source/data.c:6` (pointer `gpIconGetters`) | `:55` | `const u8 *(*)(const u8 index)` (`include/kernel/icon-rework.h:33`), one per icon sheet | `GetIconGfx` (`Kernel/Wizardry/IconDisplay/Source/Icon.c:4-10`) | Returns icon graphics. |
| `_thumb(pr_...)` plus `jumpToHack(...)` pairs | `Kernel/Preload/KernelReloc.event:68-95` | Thumb veneers to C functions for specific features (combat art, gaiden magic, upper menu, attack command, debuff OAM) | ASM sites in those features | Not a general hook point. `_thumb` is in `include/kernel/macros.event:5`. `jumpToHack` is defined by the EventAssembler `Hack Installation.txt` extension. Do not add one without reading both. |

External single slots. These are defined in `Preload/Reloc.event`, which generates `Kernel/fake-nonsec-reloc.event` (`makefile:511-517`, `Kernel/kernel.event:13`). The build reads the generated copy.

| Slot (Preload/Reloc.event:line) | Type (runner) | Called from | Set by |
|---|---|---|---|
| `gpExternalGameInitHook` (`:51`) | Declared as a pointer to a function pointer (`Kernel/Wizardry/GameInitHook/source/GameInit.c:9`) | `GameInit.c:35-37`, after the GameInit list | Nothing in the repo. The type looks inconsistent with the other slots. Unverified. |
| `gpExternalPostActionHook` (`:155`) | `bool (*)(ProcPtr)` (`include/kernel/post-action.h:6`) | `CallExternalPostActionHook` (`Kernel/Wizardry/PostActionHook/source/usr-func.c:31-37`), last real entry of `gPostActionFuncs` (`PostActionHook/data.event:47`) | Nothing in the repo. |
| `gpExternalPrePhaseHook` (`:156`) | `bool (*)(ProcPtr)` (`Kernel/Wizardry/PrePhaseHook/Source/PrePhaseHook.c:21`) | `CallExternalPrePhaseHook` (`PrePhaseHook.c:23-36`), listed at `PrePhaseHook/data.event:35` | `Kernel/Wizardry/FreeMovement/cFreeMU/src/FreeMU_Extra.c:29` sets it when free movement is on, `:36` clears it. |
| `gpExternalPrePhaseHealCalc` (`:157`) | `int (*)(int cur, struct Unit *unit)` (`Kernel/Wizardry/SkillEffects/PrePhaseSkills/PrePhaseHeal.c:12`) | `PrePhaseHeal.c:143-144` | Nothing in the repo. |
| `gpExternalSkillListGenerator` (`:158`) | `void (*)(struct Unit *, struct SkillList *, u16 *)` (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:28`) | `SkillList.c:464-465` | Nothing in the repo. |
| `gpExternalHpGetters` ... `gpExternalAidGetters` (`:160-170`) | `StatusGetterFunc_t` (`include/kernel/status-getter.h:20-30`) | Each `_GetUnit<Stat>` getter after its list, for example `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:28-29` | Nothing in the repo. `gpExternalConGetters` is declared but has no call site (checked). |
| `gpExternalEndTurnHook` (`:172`) | `bool (*)(ProcPtr)` (`Kernel/Wizardry/EndTurnHook/Source/usr-func.c:3`) | `CallExternalEndTurnHook` (`usr-func.c:5-11`), last real entry of `gEndTurnFuncs` (`EndTurnHook/data.event:5`) | Nothing in the repo. |

Non-hook `gp*` pointers in `KernelReloc.event`. These are data tables and config, not hooks: `:7` `gSkillMenuInfos`, `:45` `gKernelArmFunConfig` (`Kernel/Wizardry/ArmFunc/Source/ArmFuncConf.c:4`), `:46` `gMapTaskConfs` and `:47` `gHpBarChr` (`Kernel/Wizardry/MapTask/Source/MapTaskData.c` lines 4 and 31), `:49` `pr_gpIERevampTable` (`Kernel/Wizardry/ItemSys/IERevamp/IERevamp.event:33`), `:59-61` `gPopupComponents`, `gBattlePopupConfigs`, `gPromotPopupConfigs` (`Kernel/Wizardry/PopupRework/Source/PopupR.c:155`, `PopRConfig.c:73,103`), `:66` `sStatDebuffStatusPool` (`Kernel/Wizardry/DebuffRework/StatDebuff/StatDebuffData.c:4`), `:96-97` `AutoNarrowFontConf` and `AutoNarrowFontConfToUnicode` (`Kernel/Wizardry/Utf8`).

Hook lists that are not in `KernelReloc.event`: `gPrePhaseFuncs` (read directly, see the table) and the `StartGame`/`UnitBeginAction` replacements, which read their lists by name. Check with `grep -n "^gp" Kernel/Preload/KernelReloc.event` before assuming a list has a pointer.

### Files

| File | Role |
|---|---|
| `Kernel/Preload/KernelReloc.event` | `gp*` pointers to every hook list (`:7-99`) |
| `Preload/Reloc.event` | Source of the external slots (`:51`, `:155-172`). Generated copy: `Kernel/fake-nonsec-reloc.event` |
| `Kernel/Wizardry/wizardry.event` | Includes. Hook `data.event` files are in the `WIZARDRY_DATA` block (`:8-36`). Hook `.event` files are in `WIZARDRY_TEXT` (`:66-100`, includes `UnitHooks.event` at `:86`). `WIZARDRY_CUSTOM` starts at `:129` |
| `Kernel/Wizardry/KernelLib/Source/hook-proc.c` | Proc-walked list runner (`:52-93`) |
| `Kernel/Wizardry/KernelLib/Source/pad.c` | `KernelPad1` (void no-op), `KernelPad2` (returns its first argument), `KernelPad3` (`true`), `KernelPad4` (`false`) |
| `include/common-chax.h` | `HookProcFunc_t` (`:44`), `CHAX` (`:12`), `LYN_REPLACE_CHECK` (`:26-27`) |
| `include/kernel/kernel-lib.h` | `CheckKernelHookSkippingFlag` (`:334`), `KernelStartBlockingHookProc` (`:335`) |
| `Tools/FE-CLib-Mokha/include/proc.h` | `Proc_Start`, `Proc_StartBlocking`, `PROC_*` macros |
| `Tools/FE-CLib-Mokha/reference/fireemblem8.sym` | Vanilla addresses, format `08XXXXXX Name` |
| `Tools/FE-CLib-Mokha/reference/fireemblem8.ref.s` | Same addresses as `fun 0x08XXXXXX, Name` (Thumb bit set) |
| `Tools/scripts/lynjump-detector.sh` | `post_chax` check (see Replacing a vanilla function) |
| `Tools/scripts/lynjump-protector.sh` | Adds a `PROTECT` line for each replaced vanilla function |
| `makefile` (`:36-39`, `:440-460`) | `all` runs `pre_build`, `chax`, `post_chax`. `post_chax` depends on the built `.gba` |
| `Documentation/CONTRIBUTING.md` (`:21-44`) | The rule for `LYN_REPLACE_CHECK` and the `LynJump.event` format |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the mechanism. If a list already exists at the place you need, add an entry to it (template a). Use a vanilla replacement only when there is no list (template b). See "Replacing a vanilla function" below.
3. Write the C function with the exact signature from the table row. Put it in `Kernel/Wizardry/<Feature>/Source/<Feature>.c`. Use a non-static function, because the list references it by symbol.
4. Add the entry to the list. Put it above the end marker (`// End` or the pad entries), not after the terminator. For PrePhase, PostAction and EndTurn, keep the vanilla end entries last (`PrePhaseHook/data.event:33-36`, `PostActionHook/data.event:46-50`, `EndTurnHook/data.event:4-5`).
5. Include the generated `Source/<Feature>.lyn.event` from the feature installer. The installer is included from `Kernel/Wizardry/wizardry.event` inside `WIZARDRY_CUSTOM` (from `:129`).
6. Add a runtime toggle if the feature needs one (`Documentation/CONTRIBUTING.md` and `Documentation/Features/SystemConfig.md` are the references; see the SystemConfig pattern in the feature guide).
7. Build and test. Not covered by this doc.

### Replacing a vanilla function

Use this when no hook point exists. Replacement is one owner per function: the C body must reproduce all vanilla behaviour.

1. Find the vanilla address. Look up the name in `Tools/FE-CLib-Mokha/reference/fireemblem8.sym` (`08XXXXXX Name`). The `ORG` value is the address minus `0x08000000`. Example: `08011254 PopupIconUpdateProc_Loop` gives `ORG $11254` (`Kernel/Wizardry/PopupIconShine/Source/LynJump.event:2`). The same name appears as `fun 0x08011255, PopupIconUpdateProc_Loop` in `fireemblem8.ref.s:1765` (Thumb bit set).
2. Write the C function with the same name and signature as the vanilla function. The prototype must be visible from an included header (FE-CLib or the kernel header).
3. Put `LYN_REPLACE_CHECK(Name);` directly above the definition. The macro is `static const void *const lyn_exists_check_ ## name = &name` (`include/common-chax.h:26-27`). It takes the address of `Name` at file scope, so the build fails if `Name` is not declared. Per `Documentation/CONTRIBUTING.md:21-25`, it guards against breakage from C-Lib changes.
4. Create `Source/LynJump.event` in the same directory with the veneer (format from `Documentation/CONTRIBUTING.md:37-44`). The veneer is 16 bytes: `WORD $46C04778 $E59FC000 $E12FFF1C` (Thumb `bx pc`, then ARM `ldr r12,[pc]` and `bx r12`) followed by `POIN Name`. The `.lyn.event` that `lyn` generates from the `.c` contains the same block (`Kernel/Wizardry/PopupIconShine/Source/PopupIconShine.lyn.event:1-5`).
5. Include `Source/LynJump.event` and `Source/<Feature>.lyn.event` from the installer, and put the installer in `WIZARDRY_CUSTOM`. Code included before `:129` goes into the nearly full kernel text (`CLAUDE.md` pitfalls).
6. `make`. `post_chax` runs `Tools/scripts/lynjump-detector.sh` (`makefile:440-460`):
   - Round 1: every `LYN_REPLACE_CHECK(X)` in a `.c` must have a `LynJump.event` in the same directory that contains the text `X` (`lynjump-detector.sh:5-17`).
   - Round 2: every `ORG $` followed by `POIN X` within three lines of a `*.lyn.event` must have `LYN_REPLACE_CHECK(X);` in the matching `.c` (`lynjump-detector.sh:19-28`).
7. `lynjump-protector.sh` adds `PROTECT` lines for each `ORG $` in a generated `.lyn.event`. It looks up the function length in the `.sym` file (`lynjump-protector.sh:6-25`).

Why a list is preferred: a list composes. Several features can add entries. A replacement owns the whole function. The detector checks names only, so two features that replace the same vanilla function are not reported (the script has no duplicate check; `lynjump-detector.sh`). Which of the two wins was not tested (unverified).

## Template

### (a) New entry in an existing list (ChapterInit)

Adapted from `Kernel/Wizardry/Chatlog/Source/Chatlog.c:7-10` (`ChapterInit_ResetChatlog`, a list entry) and the `(ProcPtr proc)` form with `(void)proc` and a designer-config gate from `Kernel/Wizardry/Goals/Timer.c:10-21` (`ChapterInit_ResetChapterTimer`). The GameInit form is `void GameInit_Example(void)`, adapted from `GameInit_ResetShieldInfo` (`Kernel/Wizardry/Shield/Source/ShieldCore.c:9-12`).

File `Kernel/Wizardry/<Feature>/Source/<Feature>.c` (named Example here):

```c
#include "common-chax.h"
#include "kernel-lib.h"

/* ChapterInitHooks entry. Runs once per fresh chapter, before UndeployEveryone(). */
void ChapterInit_Example(ProcPtr proc)
{
	(void)proc;

	/* Runtime toggle (add a field to KernelDesigerConfig first; see SystemConfig). */
	if (gpKernelDesignerConfig->example_enabled != true)
		return;

	/* Reset per-chapter state here. Suspend restore is a separate path (see Timer.c:9). */
}
```

Installer `Kernel/Wizardry/<Feature>/<Feature>_Installer.event` (named Example here):

```
#ifndef EXAMPLE_INSTALLED
#define EXAMPLE_INSTALLED

#include "Source/Example.lyn.event"

#endif /* EXAMPLE_INSTALLED */
```

Registration, in `Kernel/Wizardry/ChapterInitHook/data.event`, before `WORD 0` (`:20`):

```
    POIN ChapterInit_Example
```

Include, in `Kernel/Wizardry/wizardry.event`, inside the `WIZARDRY_CUSTOM` block (next to `PopupIconShine/PopupIconShine_Installer.event` at `:161`):

```
#include "Example/Example_Installer.event"
```

### (b) Vanilla replacement with LYN_REPLACE_CHECK and LynJump.event

Adapted from `Kernel/Wizardry/PopupIconShine/Source/LynJump.event:1-6`, `Kernel/Wizardry/PopupIconShine/Source/PopupIconShine.c:178-179` and `Kernel/Wizardry/PopupIconShine/PopupIconShine_Installer.event:8-9`. Replace `ExampleVanillaFunc` and the address placeholder with the real name and the `.sym` address minus `0x08000000`.

File `Kernel/Wizardry/<Feature>/Source/<Feature>.c` (named Example here):

```c
#include "common-chax.h"
#include "kernel-lib.h"

/* Same name and signature as the vanilla function. Copy the vanilla body first, then change it. */
LYN_REPLACE_CHECK(ExampleVanillaFunc);
void ExampleVanillaFunc(struct Unit *unit)
{
	/* ... */
}
```

File `Kernel/Wizardry/<Feature>/Source/LynJump.event`:

```
PUSH
ORG $<vanilla address>
ALIGN 4
WORD $46C04778 $E59FC000 $E12FFF1C
POIN ExampleVanillaFunc
POP
```

Installer `Kernel/Wizardry/<Feature>/<Feature>_Installer.event` (named Example here):

```
#ifndef EXAMPLE_INSTALLED
#define EXAMPLE_INSTALLED

#include "Source/LynJump.event"
#include "Source/Example.lyn.event"

#endif /* EXAMPLE_INSTALLED */
```

Include in `Kernel/Wizardry/wizardry.event` inside `WIZARDRY_CUSTOM`:

```
#include "Example/Example_Installer.event"
```

## Worked examples

- `Kernel/Wizardry/PopupIconShine/` : vanilla replacement. One `LynJump.event` veneer (`ORG $11254`), `LYN_REPLACE_CHECK` at `PopupIconShine.c:178`, installer in `WIZARDRY_CUSTOM` (`wizardry.event:161`).
- `Kernel/Wizardry/Chatlog/` : ChapterInit list entry. `ChapterInit_ResetChatlog` (`Source/Chatlog.c:7-10`), registered at `Kernel/Wizardry/ChapterInitHook/data.event:17`, with an installer that includes `Source/Chatlog.lyn.event`.
- `Kernel/Wizardry/Goals/` : ChapterInit entry that takes `ProcPtr`, gated by a designer-config flag. `ChapterInit_ResetChapterTimer` (`Timer.c:10-21`), registered at `ChapterInitHook/data.event:15`, included by `Goals/Installer.event:2`.
- `Kernel/Wizardry/FreeMovement/cFreeMU/src/FreeMU_Extra.c` : external PrePhase slot. Sets `gpExternalPrePhaseHook` on enable (`:29`) and clears it on disable (`:36`).

## Gotchas

- The ChapterInit comment at `Kernel/Wizardry/ChapterInitHook/data.event:1` says `void (*)(void)`. The runner passes `proc` (`ChapterInit.c:3,12`). Use `(ProcPtr proc)` in new entries. Many existing entries are declared `(void)` (for example `ChapterInit_ResetChatlog`, `Chatlog.c:7`). The callee ignores the argument, so this works in practice (inference from the calling convention, not tested).
- Several list typedefs say `int` while the data.event comment says `void`: `CopyUnit`, `ClearUnit`, `UnitKill`, `UnitRefrain`, and the `LoadUnit` entries. The runners ignore the result, so either return type works.
- A `true` return in a proc-walked list stops the walk for one frame (`hook-proc.c:64-66`). Return `true` only after starting a blocking child.
- Entries placed after a `true`-returning entry are not skipped; they run on a later frame, after the blocking child ends. `PrePhaseHook_VanillaEnd` (`PrePhaseHook.c:9-13`) and `PostActionEnd` (`PostActionHook.c:22`) both return `true`. Put new entries above them so they run before the vanilla end steps.
- Entry order matters where noted: `UnitToBattle_SetupSkillList` must stay first (`UnitHooks/data.event:19-20`). The first `gLoadUnitHooks` entry is the vanilla stat loader (`UnitHooks/data.event:3`). The other orderings were not traced.
- Stat getters run on every stat read (`Documentation/CONTRIBUTING.md:69`). Keep them cheap. Stat-getter lists also appear in `Documentation/SkillTypes/StatusGetter.md`.
- Every list ends with `WORD 0`. The runners stop at a zero entry. The pad functions (`KernelPad1`, `KernelPad2`, `KernelPad3`, `KernelPad4`, `Kernel/Wizardry/KernelLib/Source/pad.c:4-19`) are no-op slots that fill fixed-size lists (inference from their use in `BattleSys/data.event:14,20,24,34` and `UnitStatusGetter/data.event:8`).
- The `gpExternal*` slots are read as plain `WORD 0` at build time (`Preload/Reloc.event`). Nothing in the repo sets most of them. A feature that sets one must also clear it. `gpExternalGameInitHook` is declared as a pointer to a function pointer (`GameInit.c:9`), and the call at `:35-37` dereferences it once more than the other slots. Unverified whether it works as a direct function pointer.
- `post_chax` runs only when the `.gba` is rebuilt, because its rule depends on `$(FE8_CHX)` (`makefile:457-460`). A stale `.ups` can hide a missing veneer until the next full build.
- `lynjump-detector.sh` walks the whole tree with `find .` (`lynjump-detector.sh:6,20`), which includes `.claude/worktrees/` copies. Its name checks are substring matches (`grep $_function $LYN_JUMP_FILE`, `:12`). Both are weaker than they look.
- `Tools/scripts/lynjump-replacement.sh` rewrites every `.c` file in the tree in place (`lynjump-replacement.sh:5-11`). The makefile does not call it (checked). Do not run it.
- Where lyn gets the `ORG` for a replacement: the reference file has `fun 0x080177F5, ClearUnit` (`fireemblem8.ref.s:2138`), and `ClearUnit.lyn.event` has `ORG $177f4` (`Kernel/Wizardry/UnitHooks/Source/ClearUnit.lyn.event:1-6`). The inference is that lyn matches a defined function name to the reference symbol. The exact lyn behaviour was not traced (unverified).
- Replacement bodies from installers included before `WIZARDRY_CUSTOM` go into kernel text. `UnitHooks.event` is included at `wizardry.event:86` (`WIZARDRY_TEXT`), so its replacements are in the kernel text. `CLAUDE.md` lists kernel text as nearly full.
- The vanilla caller of the ChapterInit pointer (`ORG $59A254`) and of `ResetAllPlayerUnitState` was not traced (unverified).
- `gBeginActionHooks` has no `data.event`. Its list is in `Kernel/Wizardry/BeginActionHook/BeginActionHook.event:14`, and the runner is guarded by `#if CHAX` (`BeginActionHook.c:160`).
