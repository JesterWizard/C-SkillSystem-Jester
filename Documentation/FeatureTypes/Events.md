# Event features

> Code: `Kernel/Wizardry/EventRework/`, `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c`, `Kernel/Wizardry/DebuffRework/Source/EventCmd.c`, `CustomCampaign/`  ·  Entry points: `gEventLoCmdTableRe` (event command table, `Kernel/Wizardry/EventRework/Source/EventTable.c:3`), `ASMC(Func)` (`Tools/FE-CLib-Mokha/include/EAstdlib.h:43`), `KernelCallEvent` (`include/kernel/kernel-lib.h:329`)  ·  Existing docs: `Documentation/Features/HowToMakeAChapter.md`, `CustomCampaign/README.md`, `Documentation/Features/Minigames/Snek.md`

## What it covers

Features that plug into the event script engine instead of a unit, battle or map system. Four kinds: a new event command (a new opcode, dispatched through `gEventLoCmdTableRe`; example: the skill ops, with handlers in `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c` and script macros such as `Evt_AddSkill` in `include/kernel/event-rework.h`), an ASMC (a C function that an event calls with `ASMC(Func)`; example: `CallSnekMinigameASMC`), event flags (numbered IDs that event commands set and test), and chapter content (events, maps and ROM chapter data under `CustomCampaign/`). Chapter wiring is documented in full in `Documentation/Features/HowToMakeAChapter.md`; this file only summarises it.

## How the engine calls it

### Event command (opcode)

1. A command word is 16 bits: the command ID is the high byte, the length nibble and sub-op nibble are the low byte (`_EvtCmd`, `Tools/FE-CLib-Mokha/include/eventscript.h:570-576`). `EVT_SUB_CMD` is the low nibble and `EVT_CMD_LEN` is bits 4-7 (`Tools/FE-CLib-Mokha/include/event.h:133`, `:137`).
2. The engine dispatches on the command ID through a table of `EventFuncType` (`event.h:104`, `:190`). The vanilla entries are `_EventLoCmds` (`eventscript.h:161`), which covers IDs `0x00`-`0x45` (last entry `EV_CMD_GLOWING_CROSS`, `eventscript.h:231`). IDs `0x80` and up go to the world-map table `gEventHiCmdTable` (`event.h:191`).
3. `Kernel/Wizardry/EventRework/Source/EventTable.c:3-5` builds `gEventLoCmdTableRe[0x80]` from `_EventLoCmds`. `Kernel/Wizardry/EventRework/EventRework.event:4-7` writes a pointer at ROM offset `$0CEE0` to that table. The code that reads that pointer is in the vanilla ROM and is not traced here (unverified).
4. Feature code is injected into the table with `InjectEventLoCmd(index, routine)`, which expands to `PUSH; ORG gEventLoCmdTableRe + (index * 4); POIN routine; POP` (`include/kernel/CustomDefinitions.event:26`). Users: `Kernel/Wizardry/SkillSys/SkillSys.event:37-38` (`EVENT_CMD_SKILL` = `0x46`) and `Kernel/Wizardry/DebuffRework/DebuffRework.event:16-17` (`EVENT_CMD_DEBUFF` = `0x47`). `wizardry.event` includes both at `:102-103`, before the `WIZARDRY_CUSTOM` block at `:129`.
5. The handler gets the event engine proc. `EventSkillOperation` (`Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c:194-196`) switches on `EVT_SUB_CMD(proc->pEventCurrent)`. Each sub-op reads its parameters from `argv` (`SkillEventCmd.c:36-40`) and returns `EVC_ADVANCE_CONTINUE` (`SkillEventCmd.c:50`; value at `event.h:74`).
6. Event scripts reach the command through macros. C form: `Evt_AddSkill(sid, pid)` in `include/kernel/event-rework.h:39`. EA-text form: `Evt_AddSkill` in `include/kernel/CustomDefinitions.event:29`. C code uses the C form, for example `Debug/Event/Source/debug-event.c:504`.

### ASMC

7. `ASMC(Func)` is `EvtAsmCall(func)` (`Tools/FE-CLib-Mokha/include/EAstdlib.h:43`, `eventscript.h:616`). It emits command `EV_CMD_ASMC` = `0x0D` (`eventscript.h:20`, `:175`) followed by the function address.
8. The command is handled by `Event0D_AsmCall(struct EventEngineProc *)` (`Tools/FE-CLib-Mokha/include/event.h:371`). The vanilla handler sits at `0x0800DB01` (`Kernel/Wizardry/FreeMovement/cFreeMU/src/_include/_Definitions.h.s:639`).
9. The callee is a global C function compiled by the makefile rule `%.lyn.event: %.c` (`makefile:211-217`) and included by a feature installer. Example: `Kernel/Wizardry/Goals/Installer.event:3` includes `Escape.lyn.event`.
10. Arguments: `CallForgeMenuASMC(struct EventEngineProc *)` (`Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c:446`) and `CallSnekMinigameASMC` (`Kernel/Wizardry/Minigames/Snek/Snek.c:479`) take the event proc. `void` functions such as `IsEventTileASMC` (`Kernel/Wizardry/Goals/Escape.c:189`) ignore it. That the engine passes the proc is inferred from these signatures; the dispatch code is not in the repo (unverified).
11. `ASMC2(Func)` is `EvtAsmCallEnableSkip` (`EAstdlib.h:44`, `eventscript.h:617`). Its comment says the function is not run while skipping. No active use found in the repo.

### C calling events

12. `KernelCallEvent(eventscr, exec_type, parent)` (`include/kernel/kernel-lib.h:329`, `Kernel/Wizardry/KernelLib/Source/event-call.c:24-36`). With `parent == NULL` it calls `CallEvent` inline and returns (`event-call.c:28-31`). With a parent it starts a blocking `ProcScr_KernelEventCaller` that waits while `EventEngineExists` (`event-call.c:16-22`, `:33`). Example: `CallEscapeEndingEventASMC` (`Kernel/Wizardry/Goals/Escape.c:145-150`). Execution types are in `EventExecType` (`event.h:43-45`).

### Event flags

13. Flag IDs are written and tested by the macros `ENUT` (set), `ENUF` (clear) and `CHECK_EVENTID` (`Tools/FE-CLib-Mokha/include/EAstdlib.h:14`, `:16`, `:18`), which map to `EvtSetFlag` / `EvtClearFlag` (`eventscript.h:585-587`) and `EventCheckFlag` (`eventscript.h:589`). C reads a flag with `CheckFlag(int)` (`Tools/FE-CLib-Mokha/include/eventinfo.h:186`), e.g. `Kernel/Wizardry/Goals/Timer.c:168` with `EVFLAG_WIN`.

### Chapter content

14. The build gate is `Kernel/Wizardry/wizardry.event:129-131` (`WIZARDRY_CUSTOM`, then `CONFIG_FE8_REWRITE`), which includes `CustomCampaign/Installer.event`. That file includes `Event/Event_Installer.event`, `Chapters/Chapter_Installer.event` and `Music/Music_Installer.event` (lines 3-5).
15. `CustomCampaign/Event/Event_Installer.event` registers each chapter's event group with `NewEventPointerTable_Event(id, ChapterNNEvent)` (`:19` for `Chapter01Event`; macro at `CustomCampaign/Event/PListSplit.event:37`) and includes the chapter's `events.lyn.event`.
16. `CustomCampaign/Chapters/01/events/events.c:161` defines `Chapter01Event` as `struct ChapterEventGroup` (declared at `Tools/FE-CLib-Mokha/include/chapterdata.h:118`). The group points at scene scripts such as `.beginningSceneEvents` (`events.c:182`) and `.endingSceneEvents` (`:183`).
17. `CustomCampaign/Chapters/Chapter_Installer.event` writes each chapter's `ROMChapterData` blob at a fixed ORG (`:6` for chapter 01).

## Where to look

| File | Role |
|------|------|
| `include/constants/event-cmds.h` | Command IDs. Only `EVENT_CMD_SKILL` (`0x46`) and `EVENT_CMD_DEBUFF` (`0x47`) are defined. |
| `Kernel/Wizardry/EventRework/Source/EventTable.c` | `gEventLoCmdTableRe[0x80]`, the command table. |
| `Kernel/Wizardry/EventRework/EventRework.event` | Repoints the table pointer (`ORG $0CEE0`) and includes `Source/LynJump.event` and `Source/EventRework.lyn.event`. |
| `Kernel/Wizardry/EventRework/Source/EventRework.c` | Replaces `GetUnitStructFromEventParameter` (`:5`, with `LYN_REPLACE_CHECK`). Handles the negative character IDs. |
| `include/kernel/CustomDefinitions.event` | EA-text macros: `InjectEventLoCmd` (`:26`), skill ops `Evt_*` (`:29-36`), debuff ops `EVT_DEBUFF_*` (`:5-17`). |
| `include/kernel/event-rework.h` | C macros and sub-op enum for skill ops (`EventSkillSubOps` `:27-37`, `Evt_*` `:39-47`), character IDs `CHAR_EVT_*` (`:6-10`). |
| `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c` | Example command (`0x46`). |
| `Kernel/Wizardry/DebuffRework/Source/EventCmd.c` | Example command (`0x47`), `EventDebuffOperation` at `:123`. |
| `Tools/FE-CLib-Mokha/include/eventscript.h` | `EV_CMD_*` IDs (`:17-76`), `_EventLoCmds` (`:161`), `_EvtCmd` / `_EvtArg0` (`:570-576`), `EvtAsmCall` (`:616`). |
| `Tools/FE-CLib-Mokha/include/event.h` | `EVC_*` return codes (`:74-80`), `EventFuncType` (`:104`), `EV_EXEC_*` (`:43-45`), `EVT_SUB_CMD` / `EVT_CMD_LEN` (`:133-137`), `EVT_SLOT_*` (`:99`), `gEventSlots` (`:116`), `Event0D_AsmCall` (`:371`). |
| `Tools/FE-CLib-Mokha/include/EAstdlib.h` | Short names: `ASMC` / `ASMC2` (`:43-44`), `SVAL`, `BNE`, `ENUT` / `ENUF` / `CHECK_EVENTID` (`:14-18`). |
| `include/jester_headers/event-call.h` | Not a call API. It holds `extern` declarations for world-map event scripts (`EventScrWM_*`), `gUdefs` / `gUdefCnt`, and `Event19_Checks`. Included by `CustomCampaign/Event/TutorialsEvents.c` and `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c`. For the call API see `KernelCallEvent` in `include/kernel/kernel-lib.h:329`. |
| `include/jester_headers/flags.h` | `enum` of `GUIDE_*` flag IDs (`0xB4`-`0xE7`). Included by `CustomCampaign/Chapters/_shared/headers.h:10` and `CustomCampaign/Event/TutorialsEvents.c:7`. No other file uses these names (see Gotchas). |
| `Tools/FE-CLib-Mokha/include/constants/event-flags.h` | Named flag IDs (`EVFLAG_WIN`, `EVFLAG_DEFEAT_BOSS`, `EVFLAG_TMP(n)`). Free range noted at `:12`. |
| `Kernel/Wizardry/Goals/Escape.c` | Example ASMC set and `KernelCallEvent` use. |
| `Kernel/Wizardry/Minigames/Snek/Snek.c` | Example blocking ASMC (`CallSnekMinigameASMC`, `:479`). |
| `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c` | Example ASMC that opens a menu (`CallForgeMenuASMC`, `:446`). |
| `CustomCampaign/Installer.event`, `CustomCampaign/Event/Event_Installer.event`, `CustomCampaign/Chapters/Chapter_Installer.event` | Chapter wiring (see HowToMakeAChapter). |
| `CustomCampaign/README.md` | Campaign folder layout and the three wiring points. |
| `Documentation/Features/HowToMakeAChapter.md` | Full chapter guide. Not repeated here. |

## Add one: steps

### New event command

1. Do the common steps in [Common.md](Common.md).
2. Pick an unused command ID in `0x46`-`0x7F`. `0x00`-`0x45` are vanilla (`eventscript.h:161`), `0x46` and `0x47` are taken (`include/constants/event-cmds.h:4-5`), and `0x80`+ is the world-map table, which `gEventLoCmdTableRe` does not reach. The next free ID is `0x48`. Add `#define EVENT_CMD_<NAME> 0x48` to `include/constants/event-cmds.h`.
3. Add a sub-op enum and C macros to `include/kernel/event-rework.h`, copying the `EventSkillSubOps` / `Evt_AddSkill` pattern (`:27-47`). Use a new enum name.
4. Add the EA-text macro to `include/kernel/CustomDefinitions.event`. The first `BYTE` is `(length << 4) | sub-op`, the second is the command ID (see Template).
5. Write the handler in `Kernel/Wizardry/<Feature>/Source/<Name>EventCmd.c`. Switch on `EVT_SUB_CMD(proc->pEventCurrent)` and return `EVC_ADVANCE_CONTINUE`.
6. Include the generated file and inject the handler in the feature's installer: `#include "Source/<Name>EventCmd.lyn.event"` then `InjectEventLoCmd(EVENT_CMD_<NAME>, <Handler>)`. Pattern: `Kernel/Wizardry/SkillSys/SkillSys.event:37-38`.
7. Build and test: not covered by this doc.

### New ASMC

1. Do the common steps in [Common.md](Common.md).
2. Write `void <Name>ASMC(void)` (or with `struct EventEngineProc *proc`) as a non-static global in `Kernel/Wizardry/<Feature>/Source/<Feature>.c`. Return results through `gEventSlots[EVT_SLOT_x]` (`Kernel/Wizardry/Goals/Escape.c:189-197`). To block the event until a menu or proc ends, start it with `Proc_StartBlocking(ProcScr_X, proc)` (`Kernel/Wizardry/Minigames/Snek/Snek.c:481`).
3. Add `#include "Source/<Feature>.lyn.event"` to the feature installer (pattern: `Kernel/Wizardry/Goals/Installer.event`).
4. Call it from an event: `ASMC(<Name>ASMC)` inside an `EventScr` array, then read the slot with `SVAL` and `BNE` (pattern: `Escape.c:12-14`).
5. Use `ASMC2` only when the function should be skipped during skip mode.
6. Build and test: not covered by this doc.

## Template

### ASMC

Adapted from `Kernel/Wizardry/Goals/Escape.c:189-197` (`IsEventTileASMC`) and its event use at `Escape.c:11-25`.

```c
/* Kernel/Wizardry/<Feature>/Source/<Feature>.c */
#include "common-chax.h"
#include "kernel-lib.h"
#include "event-rework.h"

void ExampleASMC(void)
{
	gEventSlots[EVT_SLOT_C] = (gActiveUnit != NULL) ? 1 : 0;
}
```

```
# Event script (any EventScr array in the build)
const EventScr EventScr_Example[] = {
    ASMC(ExampleASMC)
    SVAL(EVT_SLOT_7, 1)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    /* fall-through: slot C == 1 */
LABEL(0x0)
    ENDA
};
```

### New event command

Adapted from `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c:34-51` (`EventAddSkillOnActive`), the dispatch at `:194-225`, and the registration at `Kernel/Wizardry/SkillSys/SkillSys.event:37-38`.

```c
/* include/constants/event-cmds.h */
#define EVENT_CMD_EXAMPLE 0x48

/* include/kernel/event-rework.h (C macros) */
enum EventExampleSubOps {
	EVSUBCMD_EXAMPLE_STORE = 1,
};

#define Evt_Example(arg0) _EvtArg0(EVENT_CMD_EXAMPLE, 4, EVSUBCMD_EXAMPLE_STORE, arg0), _EvtParams2(0, 0),
```

```c
/* Kernel/Wizardry/<Feature>/Source/ExampleEventCmd.c */
#include "common-chax.h"
#include "kernel-lib.h"
#include "event-rework.h"

u8 EventExampleOperation(struct EventEngineProc *proc)
{
	const u16 *argv = proc->pEventCurrent;

	switch (EVT_SUB_CMD(proc->pEventCurrent)) {
	case EVSUBCMD_EXAMPLE_STORE:
		gEventSlots[EVT_SLOT_C] = argv[1];
		break;

	default:
		Errorf("Event format error at %p", proc->pEventCurrent);
		hang();
	}

	return EVC_ADVANCE_CONTINUE;
}
```

```
# include/kernel/CustomDefinitions.event (EA-text form). 0x41 = length 4, sub-op 1.
#define Evt_Example(arg0)  "BYTE 0x41 EVENT_CMD_EXAMPLE; SHORT arg0 0 0"

# Kernel/Wizardry/<Feature>/<Feature>.event
#include "Source/ExampleEventCmd.lyn.event"
InjectEventLoCmd(EVENT_CMD_EXAMPLE, EventExampleOperation)
```

## Worked examples

- `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c`: command `0x46`. Eight sub-ops in one switch (`:194-225`). Registered at `Kernel/Wizardry/SkillSys/SkillSys.event:38`. Shows argument reading (`argv`, `EVT_CMD_LEN`) and the `default: hang()` error path.
- `Kernel/Wizardry/Goals/Escape.c`: two ASMCs (`RemoveActiveUnitASMC` `:153`, `IsEventTileASMC` `:189`) used by `EventScr_PostAction_Escape` (`:11-25`), which branches on the slot result. `CallEscapeEndingEventASMC` (`:145`) calls `KernelCallEvent` with a NULL parent.
- `Kernel/Wizardry/Minigames/Snek/Snek.c`: `CallSnekMinigameASMC` (`:479-482`) starts a blocking proc with the event proc as parent, so the event waits for the game. Installed by `Kernel/Wizardry/Minigames/Snek/Snek_Installer.event:1`. Narrative in `Documentation/Features/Minigames/Snek.md`.
- `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c`: `CallForgeMenuASMC` (`:446-457`) resets the screen, sets `gEventSlots[EVT_SLOT_7] = 0` and calls `StartMenu(&gForgeMenuDef, proc)`. It is also called from C at `Kernel/Wizardry/VeslyDebugger/C_Code.c:5135`. Included at `Kernel/Wizardry/wizardry.event:142`.

## Gotchas

- Command table bounds: `gEventLoCmdTableRe` has 0x80 entries (`EventTable.c:3`). `InjectEventLoCmd` writes `index * 4` with no bounds check (`CustomDefinitions.event:26`). Keep the ID below `0x80`.
- Two encodings of one command: the C macros compute the header byte from the enum, but the EA-text macros hardcode it (`CustomDefinitions.event:29` writes `BYTE 0x41`, `event-rework.h:39` computes it). Renumbering a sub-op means changing both.
- Unknown sub-op: `EventSkillOperation` calls `hang()` in its `default` branch (`SkillEventCmd.c:221-223`). A script with a wrong sub-op freezes the game.
- An ASMC blocks the event only if it starts a proc with the event proc as parent. `CallSnekMinigameASMC` does this (`Snek.c:481`). `KernelCallEvent` with a NULL parent runs the event inline (`event-call.c:28-31`). Other parent choices are not verified.
- Keep ASMC functions global. Every ASMC in the repo is non-static (`Escape.c:153`, `:189`; `Snek.c:479`; `ForgeMenu.c:446`), and the event refers to them by address. Why a `static` one would fail is unverified.
- Commented-out ASMC calls. In `CustomCampaign/Chapters/00/events/events.c:207-222`, `ASMC(CallForgeMenuASMC)` and `ASMC(CallSnekMinigameASMC)` sit inside a `/* ... */` block. Grep finds no other active event call to either function (`CallForgeMenuASMC` is called from C only, `Kernel/Wizardry/VeslyDebugger/C_Code.c:5135`).
- Flag numbers: `Tools/FE-CLib-Mokha/include/constants/event-flags.h:12` marks flags `7`-`40` as free. Chapter 01 uses flag `11` as a temporary flag: `CustomCampaign/Chapters/01/events/events.c:49` sets it, `:118` clears it, and `:136` uses it as the trigger ID of a turn event (`TURN`).
- `include/jester_headers/flags.h` is an enum of `GUIDE_*` values that no code uses. A grep for each name finds only this file. Do not assume these IDs mean an event flag. The event-flag names in use are `EVFLAG_*` (see the table).
- Chapter scripts are C. `CustomCampaign/Chapters/NN/events/*.c` compiles to `events.lyn.event` through the makefile rules at `makefile:198` and `:208`. The `events/` folders contain no `.event` source files. The per-chapter `.event` files are in `map/` and `music/`.
- The campaign is included only when `WIZARDRY_CUSTOM` and `CONFIG_FE8_REWRITE` are both defined (`Kernel/Wizardry/wizardry.event:129-131`).
- Unverified: the exact dispatch code that reads the table pointer at `$0CEE0` and passes the proc to ASMC functions. Both are in the vanilla ROM and are inferred from the table layout and the callee signatures.
