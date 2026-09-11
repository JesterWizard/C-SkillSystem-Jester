# Expanded HP

<p align="center">
  <img src="./Feature - Expanded HP.gif" alt="Expanded HP" width="600"/>
</p>

Raises the engine HP ceiling from vanilla **60/120** (and signed-byte **127**) to **254**. HP is treated as unsigned so values 128–254 stay valid in combat. The stat screen and battle animation gauge show three-digit HP (200, not `--`). The minimug box also uses three digits with leading zeros (009 / 022 / 200) and never shows `--`.

Battle HP bars use extra palettes by current HP:

| HP | Bar color |
|----|-----------|
| 1–80 | Green |
| 81–160 | Yellow |
| 161–240 | Red |
| 241–254 | Blue |

This matches the integrated C Skill System `expanded_hp` flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`

Hooks (each `jumpToHack`, 8 bytes, except the number-graphics pointer):

| Function | ORG | Thumb entry |
|----------|-----|-------------|
| `StoreNumberStringOrDashesToSmallBuffer` | `$0391C` | `0x0800391D` |
| `LoadUnit` | `$17AC4` | `0x08017AC5` |
| `UnitCheckStatCaps` | `$181C8` | `0x080181C9` |
| `GetUnitCurrentHp` | `$19150` | `0x08019151` |
| `GetUnitMaxHp` | `$19190` | `0x08019191` |
| `SetUnitHp` | `$19368` | `0x08019369` |
| `AddUnitHp` | `$193A4` | `0x080193A5` |
| `InitBattleUnit` | `$2A584` | `0x0802A585` |
| `InitBattleUnitWithoutBonuses` | `$2A668` | `0x0802A669` |
| `BattleGenerateHitEffects` | `$2B600` | `0x0802B601` |
| `CheckBattleUnitStatCaps` | `$2BF24` | `0x0802BF25` |
| `NewEkrGauge` | `$50EF8` | `0x08050EF9` |
| `ekrGaugeMain` | `$51284` | `0x08051285` |
| `EfxFlashHPBarMain1` | `$54498` | `0x08054499` |
| `EfxFlashHPBarRestorePal` | `$5452C` | `0x0805452D` |
| `EfxHPBarColorChangeMain` | `$546E4` | `0x080546E5` |
| `DisplayLeftPanel` | `$86E44` | `0x08086E45` |
| `MMB_Loop_SlideIn` | `$8BCF8` | `0x0808BCF9` |
| `MMB_Loop_SlideOut` | `$8BE70` | `0x0808BE71` |
| `PutUnitMapUiWindow` | `$8C234` | `0x0808C235` |
| `ClearUnitMapUiStatus` | `$8C360` | `0x0808C361` |
| `UnitMapUiUpdate` | `$8C45C` | `0x0808C45D` |
| `DrawUnitMapUi` | `$8C5D0` | `0x0808C5D1` |
| `gGfx_PlayerInterfaceNumbers` (pointer) | `$8D0B4` | 4-byte `POIN` |

## Using HP above 60

Level-ups and autolevel can grow HP up to **254**. Promotion still recaps to the class's **Max HP** in class data. Raise class (and character) HP caps in FEBuilder if you want promoted units to keep HP above the vanilla class maximum.

## Test starting HP

`Installer.event` has `ExpandedHpTestUnits`. Each pair is a character ID and a starting HP (**1–254**). Character ID **0** ends the list. This is applied once when the unit is loaded (after autolevel), so you can test three-digit HP without rebuilding C.

The shipped table sets Eirika (`$01`) to **200**. To disable it, change that first byte to `$00`. Common IDs: Eirika `$01`, Seth `$02`, Franz `$04`, Ephraim `$0F`.

## Known limitations

- The combat forecast window and the chapter status screen still show `--` for HP above 99.
- Map combat with battle animations off still uses the vanilla two-digit HP display.

## Build

No build step is required to install the patch. `Source/ExpandedHp.lyn.event` and `Gfx/*.dmp` are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/*.c` or `Gfx/*.png`:

```bash
make -C Standalone/expanded_hp
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `expanded_hp` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-expanded-hp.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, test HP table, palettes, minimug graphics, and free-space placement |
| `Source/ExpandedHp.c` | Caps, unsigned getters/setters, battle HP math, stat screen, modular minimug, LoadUnit test HP |
| `Source/ExpandedHpGauge.c` | Battle animation HP digits and bar palettes |
| `Source/ExpandedHp.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `Gfx/HPBarYellow.dmp` / `HPBarRed.dmp` / `HPBarBlue.dmp` | Extra HP bar palettes |
| `Gfx/MMB_Tilemap.dmp` | 16-tile minimug box TSA |
| `Gfx/MMBNumberText.dmp` | Minimug HP digit graphics (0–9) |
| `makefile` | Compiles C to `.lyn.event` and HP-bar palettes to `.dmp` |

## Conflicts

- Any patch that replaces the hooked vanilla functions above, including the full C Skill System HP, battle, stat screen, minimug, or HP-bar rewrites.
- Other minimug-box number-font replacements that also write `$8D0B4`.
- Other standalone patches from this repo that also place their body at `$1000000`. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on each hook site (8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
