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

Requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

```bash
make -C Standalone/two_random_number_growths
```

This compiles `Source/GetStatIncrease_2RN.c` into `Source/GetStatIncrease_2RN.lyn.event`.

## Installation

Run `make` first so `Source/GetStatIncrease_2RN.lyn.event` exists.

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/two_random_number_growths/Installer.event`, then click **Load Script**.

FEBuilder applies the patch to the open ROM. For more detail on Insert EA, see [Installing ASM / C using Insert EA](https://feuniverse.us/t/installing-asm-c-using-insert-ea/32968).

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
make -C Standalone/two_random_number_growths
cp /path/to/fe8.gba /path/to/fe8-2rn.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/two_random_number_growths/Installer.event \
  -output:/path/to/fe8-2rn.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/GetStatIncrease_2RN.c` | C implementation |
| `Source/GetStatIncrease_2RN.lyn.event` | Generated lyn output (build with `make`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `GetStatIncrease` (`0x0802B9A0` / `0x0802B9A1`), including the full C Skill System level-up rewrite.
- Installing on a ROM that already modified the first 8 bytes at `0x0802B9A0`.

`Installer.event` uses `PROTECT` on both the hook site (`$2B9A0`, 8 bytes) and the free-space body (`$1000000` through end of `GetStatIncrease_2RN`). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
