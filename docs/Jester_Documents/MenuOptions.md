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
   - `.icon` for the menu icon.
   - `.func` for the input handler that applies the change.
3. Add the storage field to `gPlaySt.config` if the option needs to persist.
4. Update `SetGameOption` and `GetGameOption` so the UI and gameplay logic read and write the same value.
5. Initialize the field in `InitPlayConfig`.
6. If the option has gameplay effects, hook the relevant systems where the value is consumed.

To remove an option, reverse the process:

1. Remove the option index from `gGameOptionsUiOrder_NEW`.
2. Remove or disable the matching `gGameOptions_NEW` entry.
3. Remove the `gPlaySt.config` field usage from `SetGameOption`, `GetGameOption`, and any gameplay hooks.
4. Remove the initialization from `InitPlayConfig` if the field is no longer needed.
5. Check for any remaining references in rendering or gameplay code so the menu does not point at dead data.

The important rule is that the visible menu entry and the config-backed behavior must stay in sync. If a row exists in the UI without a valid config field, it will be cosmetic only. If a config field exists without a menu entry, it becomes effectively hidden.

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Menu order | `gGameOptionsUiOrder_NEW` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Defines the order and membership of the configuration menu. |
| Menu data | `gGameOptions_NEW` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Defines titles, selectors, icons, and handlers for each option. |
| Row title rendering | `GetGameOptionRowTitle` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the title string for the selected option. |
| Row help rendering | `GetGameOptionRowHelpText` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the help text for the selected option value. |
| Row value rendering | `GetGameOptionRowValueText` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Returns the value label shown in the menu. |
| Input handling | `GenericOptionChangeHandler` and `MusicOptionChangeHandler` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Applies the player’s selection changes. |
| Config storage | `SetGameOption`, `GetGameOption`, and `InitPlayConfig` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Reads, writes, and initializes the persistent config values. |
| Menu drawing | `DrawGameOptionIcon`, `DrawGameOptionText`, `DrawOptionValueTexts`, and `DrawConfigUiSprites` in [Kernel/Wizardry/Common/MenuOptions/MenuOptions.c](../Jester_Documents/Kernel/Wizardry/Common/MenuOptions/MenuOptions.c) | Renders the visible menu rows, icons, and cursor state. |

## TODO

- Replace placeholder custom option text with real option-specific text if the project wants the custom rows to be user-facing instead of generic.
- Add dedicated option hook documentation for any custom `gPlaySt.config` fields that are still not wired into gameplay systems.
- Revisit the custom row layout once the bottom-row clipping issue is fully solved.

## Limitations & Bugs

- The custom menu options currently are not linked up to anything. The `gPlaySt.config` options need to be included in their relevant locations before they do meaningful work.
- The skill capacity option, which is currently the 13th option, is cut off vertically because of a visual buffer overflow that has not yet been resolved.
- The menu is table-driven, so adding an option without updating `SetGameOption`, `GetGameOption`, and `InitPlayConfig` will leave the UI and the backing config out of sync.
