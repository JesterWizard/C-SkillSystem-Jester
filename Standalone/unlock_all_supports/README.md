# Unlock All Supports

The title screen's **Extras > Support Conversation** viewer is always available and fully unlocked:

- The **Support Conversation** entry is always in the Extras menu, even on a brand-new save (vanilla needs a cleared game).
- Every character in the viewer's list is shown as met (vanilla hides characters you have not recruited in any playthrough).
- Every support pair that has a conversation can be watched at **C, B and A** (vanilla only shows ranks you reached in a completed playthrough).

This matches the integrated C Skill System behaviour of `unlock_all_supports` (together with the kernel's always-on support viewer list replacement), but installs on **clean FE8U** without the kernel. In-game support levels, support growth and saves are not changed.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: vanilla `IsExtraSupportViewerEnabled`, `GetGlobalSupportListFromSave` and `GGM_IsCharacterKnown` (see below)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/UnlockAllSupports.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/UnlockAllSupports.c`:

```bash
make -C Standalone/unlock_all_supports
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `unlock_all_supports` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-unlock-all-supports.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/UnlockAllSupports.c` | C implementation of the three replacements |
| `Source/UnlockAllSupports.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten | Replacement |
|----------|--------------|-------------|-------------|
| `IsExtraSupportViewerEnabled` (`0x080A332C`) | `$A332C` | 8 bytes | Always `TRUE`. Its only caller is `InitSaveMenuChoice`, so this only adds the Extras entry. |
| `GetGlobalSupportListFromSave` (`0x080A35EC`) | `$A35EC` | 8 bytes | Reports rank A (3) for every support partner that has an entry in the support talk list (`GetSupportTalkEntry`), 0 otherwise. The save is not read. |
| `GGM_IsCharacterKnown` (`0x080A37F0`) | `$A37F0` | 8 bytes | Always `TRUE`. |

**Free space:** `$1000000`, 0xA0 bytes (body continues at `CURRENTOFFSET` after install). No RAM is used.

## Conflicts

- Any patch that replaces `IsExtraSupportViewerEnabled`, `GetGlobalSupportListFromSave`, `GGM_IsCharacterKnown` or `InitSaveMenuChoice` (the C Skill System's `ExtrasMenu` module replaces the last three).
- Patches that change the support talk list format (`gSupportTalkList`, read through vanilla `GetSupportTalkEntry`) must keep `GetSupportTalkEntry` working.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on all three hook sites and on the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- "Known" characters also count for any other vanilla code that calls `GGM_IsCharacterKnown`; in vanilla that is only the support viewer.
- The viewer still needs a support conversation entry for a pair; pairs with support data but no talk-list entry stay locked (rank 0), as in the kernel.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c`).
