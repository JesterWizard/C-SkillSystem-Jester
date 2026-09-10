# Custom Talk Icon

<p align="center">
  <img src="./image.png" alt="Custom Talk Icon" width="600"/>
</p>

Shows a Lex Talionus-style talk icon above the conversation partner while a unit is selected and a talk or support is available.

When a player unit is selected, the patch finds the current talkee (character talk event or available support) and draws a two-part 32x8 OBJ icon above that unit. Vanilla FE8U does not draw map talk icons; this patch wraps MapTask’s `PutUnitSpritesOam` call so vanilla sprites and icons still run first.

Icon art is by Alice (from the integrated C Skill System campaign).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: `gProc_MapTask` `PROC_CALL(PutUnitSpritesOam)` pointer at `$59D934`
- Graphics: `LoadObjUIGfx` pointer at `$156AC`, sheet width at `$15690`
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/CustomTalkIcon.lyn.event` and `Gfx/WarningHpSheet_Jester.dmp` are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CustomTalkIcon.c`. Run `make gfx` if you edit the PNG:

```bash
make -C Standalone/custom_talk_icon
make -C Standalone/custom_talk_icon gfx
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event`, `Hack Installation.txt`, or a Png2Dmp step. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `custom_talk_icon` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-talk-icon.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, free-space placement, and graphics patch |
| `Gfx.event` | Replaces `LoadObjUIGfx` sheet with the preconverted DMP |
| `Gfx/WarningHpSheet_Jester.dmp` | LZ77 OBJ UI sheet used at install time |
| `Gfx/WarningHpSheet_Jester.png` | Source art (`make gfx` rebuilds the DMP) |
| `Source/CustomTalkIcon.c` | C implementation |
| `Source/CustomTalkIcon.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `PutUnitSpritesOam` (`0x080273A5`) or the MapTask proc pointer at `$59D934`, including the full C Skill System MapTask rewrite.
- Reinstalling this patch also restores vanilla `PutUnitSpriteIconsOam` at `$275E8` so leftover `jumpToHack` bytes from older revisions cannot recurse.
- Any patch that changes `LoadObjUIGfx` graphics at `$156AC` / `$15690` (for example HP bar or other OBJ UI sheet hacks).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site, `LoadObjUIGfx` edits, and the free-space body. Overlapping patches should fail assembly with a clear EA error.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). Talk icon by Alice.
