# Text Engine Rework — ASM → C Migration Handoff

Status: **in progress.** Interpreter and core helpers are in C. Remaining installer hooks are still raw EA/ASM. A portrait/box display bug is still unresolved.

## Goal

Migrate the FE8 text engine rework (`Kernel/Wizardry/Misc/TextEngineRework`) from assembly to C so new features like **portrait shaking** can be added in C. Every portrait must go through the new C system (no vanilla fallback), with **1-1 behavioral parity with the old ASM system**.

## What has been done

All of these are now C functions in `Source/TextEngineRework.c`, hooked into the ROM via `Source/LynJump.event` + `TextEngineRework_Installer.event`:

| Function | Hooks |
|----------|-------|
| `TalkInterpret` | trampoline at `$6FD0` (replaces `DialogueInterpret.asm`) |
| `GetStringTextWidthWithDialogueCodes` | trampoline at `$8B44` |
| `ChangeTextColorID` | `jumpToHack` at `$6F00` |
| `WriteFaceXPosTableToRAM` | `jumpToHack` at `$6C28` |
| `UpdateFontBeforeBoxWidthCalc` | `callHack_r3` at `$6DC4` |
| `UpdateFontGlyphSet`, `UpdateTextBoxBgPalette`, `Copy_Text_Attributes`, `DecompressTextBoxGraphics` | called from C / `gProc_DialogueBoxAppearingAnimation` |

Verified in a fresh ROM build (build passes, lyn-jump detection passes):
- Old ASM files (`DialogueInterpret.asm`, `LoadFaceFlipping.asm`, `Copy_Text_Attributes.asm`, etc.) are **not included in the build** and their old hook sites contain vanilla bytes (e.g. `$7858`). They are dead code.
- Compiled C `TextEngine_LoadFace` in the ROM is a faithful replica of vanilla `TalkLoadFace` (flip rule, faceId `-0x100`, `StartFaceAuto(hpos*8, 80, kind|flip)`, fade-in, layer, lock).

## Current bug (unresolved)

With the freshly rebuilt ROM: **portraits do not display, and the text box is also broken** (user confirmed text does not render correctly). All vanilla conversations affected. The old ASM code has been ruled out as the cause.

Next debugging steps for a new chat:
1. Confirm whether **any** dialogue (including vanilla chapter intro) renders text+box at all.
2. Check `gProc_DialogueBoxAppearingAnimation` / `DecompressTextBoxGraphics` — broken box rendering points here first, before portraits.
3. Check the `$88CE` box-height inline patch (it was hand-written `SHORT`s; may not match the current vanilla layout at that offset).
4. Use the C interpreter's return-value contract (0/1/2/3) against the `Talk_OnIdle` call site at `$6C82`.

## Instructions: replace the remaining hooks with C equivalents

Current `TextEngineRework_Installer.event` still contains these non-C hooks. Convert each as follows:

### 1. `$7AB4` — `TalkFaceMove_OnInit` variable-speed skip (inline ASM)
- Current: `SHORT 0x6DF0 0x2800 0xD125 ...` (if `FaceProc+0x5C != 0`, skip the vanilla distance calculation).
- Convert: write a C function `int TalkFaceMove_OnInitOverride(struct Proc *proc)` that does:
  - read `proc->unk5C` (offset `0x5C`); if non-zero, return early (skip vanilla math),
  - otherwise replicate the vanilla `TalkFaceMove_OnInit` behavior (`GetTalkFaceHPos`, distance write at `+0x66`).
- Hook: replace the `SHORT` block with `ORG $7AB4; jumpToHack(CFunction)`. Add the new C function to `Source/TextEngineRework.c` and a matching `LYN_REPLACE_CHECK` + `LynJump.event` entry only if it fully replaces the vanilla function (see note below).

### 2. `$7A96` — `StartTalkFaceMove` return value (inline ASM)
- Current: `SHORT 0x0639 0x1609 ...` (force `CreateMovingFaceProc` to return the new proc).
- Convert: write a C wrapper `struct Proc *StartTalkFaceMoveC(...)` that calls the vanilla `StartTalkFaceMove` (now at its natural address) and returns the proc pointer. Hook at `$7A96` via `callHack_rX` with the proc pointer restored into `r0`.

### 3. `$6848` + `$88CE` — text box height (inline SHORT patches)
- Current: `SHORT 0x2503` (`mov r5,#3`, force 3 text structs in `InitTalk`) and the 7-short block in `StartTalkOpen` (`$88CE`) that computes box height from `lines`.
- Convert: these are small single-behavior tweaks. Preferred C form:
  - Reimplement the `$88CE` block as `void StartTalkOpen_SetBoxHeight(struct Proc *boxProc)` in C (uses `state->lines * 2 + 2`, writes `+0x64/+0x66`), hooked at `$88CE` with `callHack_r3`.
  - Keep `$6848` as a data patch (it is one instruction inside `InitTalk`; pulling the whole function into C is overkill) **or** move `InitTalk` into C and set `state->lines`/struct count there.
- Rule of thumb: if the patch is a whole function body → new C function + `jumpToHack`. If it is one instruction inside a large vanilla function → keep as a `SHORT`/`BYTE` data patch, and document why.

### 4. `$7D42` + `$7FA6` — scroll fixes (`SHORT 0x3006`)
- Current: two single-instruction tweaks (`add r0,#6`) in `TalkShiftClearAll_OnInit` (`$7D42`) and `TalkShiftClear_OnInit` (`$7FA6`) that move the box-clear origin down for 3-line boxes.
- Convert: pull the two `*_OnInit` routines into C, or leave as data patches (see rule of thumb in #3). If left, no change needed.

### 5. `$8950` + `$895C` — `GetTalkFaceHPos` table pointer
- Current: patches `GetTalkFaceHPos` (`$8934`) to read the X-position table from `state+0x50` (written by C `WriteFaceXPosTableToRAM`).
- Convert: reimplement `GetTalkFaceHPos` entirely in C (it currently reads the table through the patch) and hook at `$8934` via `LynJump.event`; then delete the `$8950`/`$895C` patches.

### 6. `$CCC0E` / `$CCC20` — promotion box graphics fix (data patch)
- Current: `WORD 0x6003800` (decompress dest) and `BYTE $8E` (length) inside `ClassChgLoadUI`.
- Convert: this is pure data. Reimplement as `void ClassChgLoadUI_C(...)` only if `ClassChgLoadUI` gets migrated; otherwise leave the `ORG`/`WORD`/`BYTE` patches as-is.

### 7. `$83F4` — `gProc_DialogueBoxAppearingAnimation` proc
- Current: EA proc definition (`PROC_CALL_ROUTINE(Copy_Text_Attributes|1)`, `PROC_LOOP_ROUTINE(DecompressTextBoxGraphics|1)`) with its pointer written at `$83F4`.
- Convert: define the proc in C using the library macros (`PROC_CALL`, `PROC_LOOP`/`PROC_REPEAT`, `PROC_END` from `Tools/FE-CLib-Mokha/include/proc.h`), export it, and keep the `ORG $83F4; POIN <C proc symbol>` line. This is the highest-value conversion for the box-rendering bug.

## Conversion recipe (repeatable)

1. Write the C function in `Source/TextEngineRework.c` (replicate the ASM logic exactly; keep side effects and register/return conventions).
2. If it replaces a whole vanilla function: add `LYN_REPLACE_CHECK(Name);` and a `PUSH/ORG/ALIGN 4/WORD trampoline/POIN Name/POP` block in `Source/LynJump.event`, included **before** `TextEngineRework.lyn.event` in the installer.
3. If it is called from existing C: just export it and call it.
4. Delete the replaced `ORG`/`SHORT`/`jumpToHack` block from `TextEngineRework_Installer.event`.
5. Rebuild (`make -j4`), confirm lyn-jump detection passes, and disassemble the hook sites in `fe8-kernel-dev.gba` to confirm the trampoline points at the C code.
6. Old ASM files are kept in the folder as reference — do not re-enable them.

## Key files

| File | Role |
|------|------|
| `Source/TextEngineRework.c` | All migrated C code |
| `Source/LynJump.event` | Explicit `ORG`/`POIN` trampolines for whole-function replacements |
| `Source/TextEngineRework.lyn.event` | Auto-generated (lyn tool) — do not edit by hand |
| `TextEngineRework_Installer.event` | Remaining hooks to migrate |
| `_Text_Engine_Tables.txt` | Font/palette/box/boop tables |
| `_Text_Engine_Defs.asm` | Constants + addresses (reference only) |
| `*.asm` / `*.lyn.event` in folder root | Old ASM system — dead code, reference only |

## TODO

- Resolve the text-box/portrait display bug before further migration.
- Migrate hooks #1–#5 and #7 above (see instructions).
- Then add portrait shaking as a new C feature.

## Limitations & Bugs

- Unresolved: text box and portraits not rendering in the fresh ROM.
- Any hand-written `SHORT` patch is fragile if the vanilla function layout shifts — prefer C reimplementation for whole-function logic.
