# Variable Unit Descriptions

Lets a character's stat screen description (the help box shown on their name, stat screen page 1) change depending on the current chapter. Use it to update a character's bio as the story goes on, hint at upcoming events, or add flavor text.

This matches the integrated C Skill System `variable_unit_descriptions` config flag (see [VariableCharacterDescriptions.md](../../Documentation/Features/VariableCharacterDescriptions.md)), but installs on **clean FE8U** without the kernel. The integrated demo text lives in the repo's text pipeline, so this package ships with an **empty** table: you add your own texts with FEBuilder and list their text IDs.

## Configuration

1. In FEBuilder's **Text Editor**, add the description texts you want and note their text IDs.
2. Edit `VariableUnitDescriptionTable` in `Installer.event`. One row per character and chapter:

```
VariableUnitDescriptionTable:
    BYTE 0x01 0x01; SHORT 0x0E01 // Eirika in Chapter 1 -> text 0x0E01
    BYTE 0x01 0x02; SHORT 0x0E02 // Eirika in Chapter 2 -> text 0x0E02
    BYTE 0x02 0x02; SHORT 0x0E03 // Seth in Chapter 2   -> text 0x0E03
    BYTE 0x00 0x00; SHORT 0x0000 // end of table
```

- First `BYTE`: character ID. Second `BYTE`: chapter ID (`gPlaySt.chapterIndex`; Prologue = `0x00`, Chapter 1 = `0x01`, ...). `SHORT`: text ID.
- The first matching row is used. With no matching row (or text ID `0`), the character's normal description from the Character Editor is shown, as in vanilla.
- The row with character ID `0` ends the table; keep it last.

The integrated demo used three texts per character for chapters 1, 2 and 3; write three rows per character to get the same setup.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: `HbPopulate_SSCharacter` at `0x08088BD4` (installer ORG `$88BD4`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/VariableUnitDescriptions.lyn.event` is checked in, and the table lives in `Installer.event`, so you can edit the table and install `Installer.event` as-is.

Run `make` only if you edit `Source/VariableUnitDescriptions.c`:

```bash
make -C Standalone/variable_unit_descriptions
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `variable_unit_descriptions` folder (or download this standalone package).
2. Open your FE8U ROM in FEBuilder, add your texts, and fill `VariableUnitDescriptionTable` in `Installer.event`.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

To change the table later, edit `Installer.event` and insert it again; it is rewritten at the same free-space address (make sure nothing else was installed right after it if the table grew).

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-variable-descriptions.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, free-space placement, and the description table |
| `Source/VariableUnitDescriptions.c` | Replacement for `HbPopulate_SSCharacter` |
| `Source/VariableUnitDescriptions.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `HbPopulate_SSCharacter` | `$88BD4` | 8 bytes |

**Free space:** `$1000000` (84 bytes plus 4 bytes per table row; continues at `CURRENTOFFSET`).

No RAM is used. No vanilla text is changed.

## Conflicts

- Any patch that hooks or replaces `HbPopulate_SSCharacter` (`$88BD4`), or repoints the stat screen help box `populate` pointers that call it (`$A00B50`, `$A00D2C`, `$A00ED0`), including the full C Skill System `DebuffRework` misc hooks.
- No other package in `Standalone/` hooks this address.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on the hook site and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Differences from the integrated build

- Data-driven table (character, chapter, text) instead of a hard-coded three-chapter switch, so any chapter can be used.
- A character/chapter without an entry keeps the normal description. In the integrated build, a character without a table entry got the generic "no description" text (`0x6BE`) in chapters 1-3.
- The integrated demo strings (`CustomCampaign/Text/VariableCharacterDescriptions`) are not included; they are not in vanilla FE8U.

## Limitations

- Only the chapter is checked. For flag- or event-based descriptions, edit `GetVariableUnitDescription` in the C source and rebuild.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/DebuffRework/Source/MiscHooks.c`).
