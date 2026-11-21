# Menu Options

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [How To Add Or Remove An Option](#how-to-add-or-remove-an-option)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

## Introduction

The configuration menu is the player-facing entry point for runtime game settings. It controls the standard FE8-style options as well as the project-specific custom options added through `gGameOptions_NEW`.

For players, this menu decides what can be toggled in the configuration screen and how each option is presented. For contributors, the menu is the bridge between the option table, the UI order list, and the `gPlaySt.config` fields that store the actual values.

The current implementation keeps the visible menu order in `gGameOptionsUiOrder_NEW` and uses `gGameOptions_NEW` to define each option’s title, selectors, icon, and change handler.

## Plan

The menu follows a simple data-driven layout:

| Stage | Responsibility |
|------|----------------|
| Menu order | `gGameOptionsUiOrder_NEW` determines which option indices appear in the UI and in what order. |
| Option data | `gGameOptions_NEW` defines the title, selector text, icon, and handler for each option. |
| Rendering | The draw helpers read the current selected option and render the label, values, and icon. |
| Input handling | The option handlers update `gPlaySt.config` when the player changes a value. |
| Persistence | `InitPlayConfig` initializes the underlying config fields, and the rest of the game reads those values from `gPlaySt.config`. |

The menu is intentionally table-driven so new options can be added without rewriting the UI logic. In practice, a new option needs three pieces:

1. A position in `gGameOptionsUiOrder_NEW`.
2. A matching `struct GameOption` entry in `gGameOptions_NEW`.
3. A backing field in `gPlaySt.config` plus handler logic that reads and writes that field.

## How To Add Or Remove An Option

To add a new menu option, add it in both places that define the visible menu and the underlying behavior:

1. Add the option index to `gGameOptionsUiOrder_NEW` in the position you want it to appear.
2. Add a `struct GameOption` entry in `gGameOptions_NEW` with:
   - `.msgId` for the title.
   - `.selectors` for the value labels, help text, and x positions.
   - `.icon` for the menu icon. The row needs its own icon: a free slot of `Kernel/Wizardry/MenuOptions/FE8_All_System_Icons.png` (0x50-0x5E free at writing). No two rows share an icon.
   - `.func` for the input handler that applies the change.
3. Add the storage field to `gPlaySt.config` if the option needs to persist.
4. Update `SetGameOption` and `GetGameOption` so the UI and gameplay logic read and write the same value.
5. Initialize the field in `InitPlayConfig`.
6. If the option has gameplay effects, hook the relevant systems where the value is consumed.

To remove an option, reverse the process:

1. Remove the option index from `gGameOptionsUiOrder_NEW`. Shrink `gGameOptionsUiOrder_NEW`'s size to the rows left: a leftover slot shows as an extra Animation row (option 0).
2. Remove or disable the matching `gGameOptions_NEW` entry.
3. Remove the `gPlaySt.config` field usage from `SetGameOption`, `GetGameOption`, and any gameplay hooks.
4. Remove the initialization from `InitPlayConfig` if the field is no longer needed.
5. Check for any remaining references in rendering or gameplay code so the menu does not point at dead data.

A row with more values than fit on one line (up to four selectors do) can show one value at a time, as the UI Style
row does (`GAME_OPTION_EXP_BAR`, [VeslyUI Styles](./VeslyUIStyles.md#choosing-a-style)): `GetGameOptionRowCount` returns the number of
values, `GetGameOptionRowValueText` and `GetGameOptionRowHelpText` return the current one's name and help, and
`DrawOptionValueTexts` and `DrawConfigUiSprites` draw only that name, at the first selector's x.
The Map Colors row (`GAME_OPTION_MAP_COLORS`, [Map Colors](./MapColors.md)) has three values and, like
`GAME_OPTION_REAL_TIME_INTERVAL`, is special-cased in `GetGameOptionRowCount`, `GetGameOptionRowValueText` and
`GetGameOptionRowX` so it shows its own value names instead of ON/OFF.

The Recap row (`GAME_OPTION_RECAP`, [Previously on Your Quest](./Recap.md#turning-it-off)) is a plain ON/OFF row
(ON 112, OFF 135, icon 0x4E) backed by `gPlaySt.config.recap_off` (0 = ON). It is always visible and has no designer
gate. `InitPlayConfig` sets it to 0 for new games: the arrows are off by default.

The Threat Arrows row (`GAME_OPTION_THREAT_ARROWS`, [Threat Arrows](./ThreatArrows.md#toggle)) is a plain ON/OFF row
(ON 112, OFF 135, icon 0x50) backed by `gPlaySt.config.threat_arrows_on` (1 = ON, 0 = OFF, bit 6 of `PLAYST + 0x43`). It is
hidden when the designer config's `threat_arrows` is off. `InitPlayConfig` sets it to 0 for new games.

The Potential Page row (`GAME_OPTION_GROWTH_RADAR`, [Growth Radar](./GrowthRadar.md#toggle)) is a plain ON/OFF row
(ON 112, OFF 135, icon 0x52) backed by `gPlaySt.config.growth_radar_off` (bit 7 of `PLAYST + 0x43`, the last spare
bit; 0 = ON). It is hidden when the designer config's `stat_page_growth_radar` is off or growths are hidden (stat
points). `InitPlayConfig` sets it to 0 for new games: the page shows by default. No spare bit is left in
`gPlaySt.config` now.

The Level-Up Bars row (`GAME_OPTION_LVUP_STAT_BARS`, [Level-Up Stat Bars](./LvupStatBars.md#toggle)) is a plain ON/OFF
row (ON 112, OFF 135, icon 0x54) backed by `gPlaySt.config2.lvup_stat_bars_off` (bit 0 of `PLAYST + 0x21`; 0 = ON), the
first bit of `config2` (below). It is hidden when the designer config's `lvup_stat_bars` is off or the level-up screen
is replaced (`KernelLvupReplacesGrowths`: stat points or the timing minigame). `InitPlayConfig` sets it to 0.

The Chapter Results row (`GAME_OPTION_CHAPTER_RESULTS`, [Chapter Results](./ChapterResults.md#toggle)) is a plain
ON/OFF row (ON 112, OFF 135, icon 0x56: a results sheet with an orange seal) backed by
`gPlaySt.config2.chapter_results_off` (bit 1 of `PLAYST + 0x21`; 0 = ON), the last row. It is hidden when the designer
config's `chapter_results_screen` is off or `CONFIG_DISTRICT_GUILD` is undefined. `InitPlayConfig` sets it to 0.

### config2: more Options bits

`gPlaySt.config` is full, so new rows go in `gPlaySt.config2` (`struct PlaySt_OptionBits2` in
`Tools/FE-CLib-Mokha/include/types.h`): ten bytes at `PLAYST + 0x21`..`0x2A` (`0x0202BD11`..`0x0202BD1A`), which were
FE7's tactician name (`char playerName[0x2B - 0x20]`). `playerName` is now one byte, the only one this build uses: a
new game sets it to `'\0'`, and vanilla's readers (`GetTacticianName`: the text's [Tact] code, `TalkInterpret`,
`GenerateGameRankSaveData`) read it as a string, so they stop there. `struct PlaySt` keeps its size (0x4C) and every
other offset (`_Static_assert`s in `Kernel/Wizardry/LvupStatBars/Source/LvupStatBars.c`), so no save chunk and no
`.lyn.event` changes. The struct is `__attribute__((packed))`, not `BITPACKED`, whose `aligned(4)` would pad it to 12.

| Byte | Bits | Field | Row |
|---|---|---|---|
| `0x21` | 0 | `lvup_stat_bars_off` | Level-Up Bars (0 = ON) |
| `0x21` | 1 | `chapter_results_off` | Chapter Results (0 = ON) |
| `0x21` | 2-7 | `unused_21_2` | free |
| `0x22`-`0x2A` | all | `unused_22[9]` | free (72 bits) |

Rules:
- **0 is the default** of every `config2` option: name the bit for the non-default state (`*_off`, or `*_on` for a
  feature that starts off). A new game zeroes the ten bytes (`WriteNewGameSave` in
  `Kernel/Wizardry/SaveData/Source/ModularSaveCore.c`, next to `gPlaySt.playerName[0] = '\0'`), and saves from before
  `config2` hold 0 there (the old name bytes were never written in a normal game).
- Take a bit from `unused_21_2` (shrink it) or a byte of `unused_22` (shrink the array) so the struct stays 10 bytes;
  the `_Static_assert`s fail the build otherwise.
- Wire the row as above: `GAME_OPTION_*` in `uiconfig.h`, `gGameOptionsUiOrder_NEW` (size + 1), `gGameOptions_NEW`,
  `SetGameOption`/`GetGameOption` (`gPlaySt.config2.<bit>`), `InitPlayConfig`, `IsGameOptionUiVisible` for its
  designer gate, a free icon, and two messages at the end of `Contents/Texts/Source/texts/Districts.txt`.
- What still writes those bytes: vanilla's title-screen Extras support viewer (`SupportScreen_SetupUnits` when not
  started from the prep screen) copies "Mark" into the name (`0x20`-`0x24`) before any save is loaded; loading a save
  or starting a new game replaces or zeroes it. The debug startup menu's chapter select and world-map effect, and the
  unused tactician name screen (`StartTacticianNameSelect`, which nothing calls), also call `SetTacticianName`. Nothing
  in the kernel reads or writes `0x0202BD11`..`0x0202BD1A` otherwise (grep of the C, asm and event sources and the smoke
  tests, and a scan of the clean ROM for those addresses as literals).

The important rule is that the visible menu entry and the config-backed behavior must stay in sync. If a row exists in the UI without a valid config field, it will be cosmetic only. If a config field exists without a menu entry, it becomes effectively hidden.

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Menu order | `gGameOptionsUiOrder_NEW` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Defines the order and membership of the configuration menu. |
| Menu data | `gGameOptions_NEW` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Defines titles, selectors, icons, and handlers for each option. |
| Row title rendering | `GetGameOptionRowTitle` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the title string for the selected option. |
| Row help rendering | `GetGameOptionRowHelpText` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the help text for the selected option value. |
| Row value rendering | `GetGameOptionRowValueText` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the value label shown in the menu. |
| Input handling | `GenericOptionChangeHandler` and `MusicOptionChangeHandler` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Applies the player’s selection changes. |
| Config storage | `SetGameOption`, `GetGameOption`, and `InitPlayConfig` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Reads, writes, and initializes the persistent config values. |
| Menu drawing | `DrawGameOptionIcon`, `DrawGameOptionText`, `DrawOptionValueTexts`, and `DrawConfigUiSprites` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Features/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Renders the visible menu rows, icons, and cursor state. |

## TODO

- Replace placeholder custom option text with real option-specific text if the project wants the custom rows to be user-facing instead of generic.
- Add dedicated option hook documentation for any custom `gPlaySt.config` fields that are still not wired into gameplay systems.
- Revisit the custom row layout once the bottom-row clipping issue is fully solved.

## Limitations & Bugs

- The custom menu options currently are not linked up to anything. The `gPlaySt.config` options need to be included in their relevant locations before they do meaningful work.
- The skill capacity option, which is currently the 13th option, is cut off vertically because of a visual buffer overflow that has not yet been resolved.
- The menu is table-driven, so adding an option without updating `SetGameOption`, `GetGameOption`, and `InitPlayConfig` will leave the UI and the backing config out of sync.
