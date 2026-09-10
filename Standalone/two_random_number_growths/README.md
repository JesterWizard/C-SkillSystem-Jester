# Two Random Number Growths

During level-up stat gain calculation, fractional growth rolls use **2RN** instead of vanilla **1RN**:

- `0%` growth: always `+0`
- `100%` growth: always `+1` (guaranteed point from the `>= 100` loop)
- `200%` growth: always `+2`
- `1-99%` growth: uses `Roll2RN(growth)` for the remainder roll

This matches the integrated C Skill System behavior for that config flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `GetStatIncrease` at `0x0802B9A0` (Thumb entry `0x0802B9A1`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/GetStatIncrease_2RN.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/GetStatIncrease_2RN.c`:

```bash
make -C Standalone/two_random_number_growths
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `two_random_number_growths` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-2rn.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/GetStatIncrease_2RN.c` | C implementation |
| `Source/GetStatIncrease_2RN.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `GetStatIncrease` (`0x0802B9A0` / `0x0802B9A1`), including the full C Skill System level-up rewrite.
- Installing on a ROM that already modified the first 8 bytes at `0x0802B9A0`.

`Installer.event` uses `PROTECT` on both the hook site (`$2B9A0`, 8 bytes) and the free-space body (`$1000000` through end of `GetStatIncrease_2RN`). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
