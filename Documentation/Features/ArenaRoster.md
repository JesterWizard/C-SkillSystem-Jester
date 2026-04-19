# Arena Roster - Pick Your Fights

<p align="center">
  <img src="../Gifs/Arena_Awards.gif" alt="Anima Triangle Demo" width="600"/>
</p>

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

The arena roster system replaces the normal blind arena roll with a curated opponent list.

From the player side, the change is simple: instead of paying for a random matchup, the player scrolls through a chapter-specific board of enemies, sees the wager and reward, and chooses the fight they want. Cleared entries gray out, chapter win limits can close the board, and item rewards can be deferred until the results flow is safe to show a popup.

From the implementation side, this is not a separate feature bolted onto the map. It is a rewrite of the arena UI flow in the arena module itself. The roster selector intercepts the wager step, configures `gArenaState` with the chosen enemy, reuses the normal arena battle path, then injects custom reward and persistence handling into the results path.

The system is gated primarily by `gpKernelDesignerConfig->arena_roster_menu`. Related arena flags still matter:

| Flag | Effect |
|------|--------|
| `arena_roster_menu` | Enables the roster-based arena flow and alternate arena UI proc |
| `arena_show_opponent_in_advance` | Draws the chosen opponent's details before the wager is confirmed |
| `arena_let_player_use_upgraded_weapons` | Allows the player weapon generated for arena combat to upgrade from the base arena weapon |
| `arena_limits` | Applies chapter turn/level restrictions before arena access |

---

## Plan

The feature is built around three layers: chapter data, runtime selection state, and arena flow hooks.

### Roster Model

Each chapter may define its own list of curated entries in `gArenaRosterTable`.

| Data type | Purpose | Notes |
|-----------|---------|-------|
| `ArenaRosterEntry` | One selectable fight | Stores PID/JID, level, entry weapon, price, reward type, and reward payload |
| `ArenaRosterChapter` | Chapter roster header | Binds a chapter id to an entry array and a `maxWins` cap |
| `ArenaRosterSuspendState` | Persistent progress | Stores chapter id, total wins, and a bitfield of cleared entries |
| `ArenaRosterRuntimeState` | Volatile UI state | Tracks current choice state, selected index, and deferred reward item popup |

At the moment, the shipped data is small and intentionally local to the module. The sample roster is `sArenaRosterChapter6Entries`, which is then exposed through `gArenaRosterTable`.

### Arena Flow

When roster mode is enabled, the arena proc route changes from the normal arena UI proc to `gProcScr_ArenaUiMain_NEW`.

The resulting flow is:

| Step | Main function(s) | Outcome |
|------|------------------|---------|
| Arena entry | `StartArenaScreen`, `ArenaUi_WelcomeDialogue` | Switches to the rewritten arena proc and uses the roster-aware welcome text |
| Selection open | `ArenaUi_WagerGoldDialogue`, `ArenaRosterStartSelection` | Launches a blocking selector proc instead of immediately asking for a blind wager |
| List navigation | `ArenaRosterSelector_Loop`, `ArenaRosterDrawList`, `ArenaRosterDrawSpritesOnly` | Lets the player move through valid entries, preview costs, and cancel cleanly |
| Match setup | `ArenaRosterApplySelection`, `ArenaGenerateOpponentUnit`, `ArenaGenerateBaseWeapons` | Writes the chosen enemy into `gArenaState` and prepares arena weapons/range |
| Confirmation | `ArenaUi_CheckConfirmation`, `ArenaUi_ConfirmWager` | Applies the normal gold confirmation path using the selected entry's wager |
| Results | `ArenaUi_ResultsDialogue`, `ArenaRosterHandleResultsDialogue` | Pays gold rewards, marks cleared entries, or stages an item reward popup |
| Cleanup | `ArenaRosterFlushDeferredPopup`, `ArenaUi_OnEnd` | Flushes delayed item popup state and restores the map/UI state |

### Reward Rules

Rewards are intentionally split into three modes:

| Reward type | Player-facing behavior | Implementation detail |
|-------------|------------------------|-----------------------|
| Double payout | Win returns twice the wager | `ArenaRosterGetRewardGold` returns `price * 2` |
| Triple payout | Win returns three times the wager | `ArenaRosterGetRewardGold` returns `price * 3` |
| Item reward | Win grants a fixed item instead of bonus gold | Result dialogue sets `pendingRewardItem`, then `ArenaRosterFlushDeferredPopup` shows the popup later |

The wager multiplier from `SID_Ludopathy` still applies to the buy-in side because the roster path reuses the arena wager flow.

### Persistence Model

Roster progress is chapter-scoped and survives both full saves and suspend saves.

| Event | Function(s) | Effect |
|-------|-------------|--------|
| Chapter start | `ChapterInit_ResetArenaRosterState` | Clears runtime choice state and resets or preserves suspend-loaded state as needed |
| Save write | `SaveArenaRosterProgress` | Writes `ArenaRosterSuspendState` into EMS save storage |
| Save read | `LoadArenaRosterProgress` | Restores progress and marks runtime state as loaded from suspend |
| Chapter mismatch | `ArenaRosterEnsureSuspendStateCurrent` | Automatically resets progress when the stored chapter id does not match the current chapter |

This matters because the system tracks per-entry clear flags in a compact bitfield. Without the chapter check, stale clear data from another chapter would incorrectly lock out roster entries.

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Runtime feature flags | `struct KernelDesigerConfig` in [`../include/kernel/kernel-lib.h`](../include/kernel/kernel-lib.h) and defaults in [`../Data/DesignerConfig/designer-config.c`](../Data/DesignerConfig/designer-config.c) | Declares and initializes `arena_roster_menu`, `arena_show_opponent_in_advance`, `arena_limits`, and related arena tuning flags |
| Arena entrypoint swap | `StartArenaScreen`, `ArenaUi_WelcomeDialogue`, `DrawArenaOpponentDetailsText` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Redirects the standard arena into roster mode and suppresses the vanilla blind-opponent details path |
| Chapter roster data | `sArenaRosterChapter6Entries`, `gArenaRosterTable`, `ArenaRosterGetChapter` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Defines which fights exist in each chapter and how many wins are allowed |
| Selector proc and UI | `ProcScr_ArenaRosterSelect`, `ArenaRosterSelector_Init`, `ArenaRosterSelector_Loop`, `ArenaRosterDrawList`, `ArenaRosterDrawSpritesOnly` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Implements the scrollable roster menu, cursor behavior, scrollbar updates, and enemy sprite preview |
| Match configuration | `ArenaRosterApplySelection`, `ArenaRosterGenerateOpponentUnit`, `ArenaRosterGetSelectedWeapon`, `ArenaGenerateBaseWeapons` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Converts a roster entry into the live arena matchup stored in `gArenaState` |
| Confirmation and results | `ArenaUi_WagerGoldDialogue`, `ArenaUi_CheckConfirmation`, `ArenaUi_ResultsDialogue`, `ArenaRosterHandleResultsDialogue`, `ArenaRosterGrantWinReward`, `ArenaRosterFlushDeferredPopup` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Reuses the arena's talk flow while adding roster-specific clear tracking and reward payout logic |
| Clear-state bookkeeping | `ArenaRosterClearProgress`, `ArenaRosterEntryCleared`, `ArenaRosterSetEntryCleared`, `ArenaRosterHasReachedWinLimit`, `ArenaRosterClearSelection` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) | Tracks which fights are already beaten and enforces per-chapter lockout rules |
| Save integration | `SaveArenaRosterProgress`, `LoadArenaRosterProgress` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) and EMS chunk registration in [`../Kernel/Wizardry/Common/SaveData/data.event`](../Kernel/Wizardry/Common/SaveData/data.event) | Persists roster progress in both regular save and suspend save data |
| Chapter init hook | `ChapterInit_ResetArenaRosterState` in [`../Kernel/Wizardry/Common/Arena/Arena.c`](../Kernel/Wizardry/Common/Arena/Arena.c) and hook registration in [`../Kernel/Wizardry/Common/ChapterInitHook/data.event`](../Kernel/Wizardry/Common/ChapterInitHook/data.event) | Ensures roster runtime state is reset as part of the chapter initialization pipeline |

---

## TODO

- Move chapter roster definitions out of `Arena.c` if the table grows beyond a small number of chapters.
- Replace hard-coded English labels such as `Good match`, `Okay match`, and payout text with text ids.
- Document the expected workflow for adding new chapter entries, especially valid reward/item combinations.
- Decide whether cleared item-reward entries should remain visible but disabled, or support repeatable completion in some chapters.

---

## Limitations & Bugs

- The current roster content is effectively a chapter-specific prototype. Only the chapter data that is explicitly added to `gArenaRosterTable` exists.
- Most of the implementation, data, UI, and persistence logic lives in one large source file. That keeps the feature easy to trace today, but it also makes future expansion harder to review and maintain.
- Reward/item text and matchup labels still contain hard-coded strings instead of a fully text-id-driven presentation path.
- The arena rewrite still relies on vanilla arena state and proc timing. That reuse is practical, but it means popup timing and dialogue state must be handled carefully, which is why item rewards are deferred.
- The comments in the module already note that some older arena adjustment logic now crashes on the arena screen. The roster system avoids that path, but arena-adjacent rewrites should still be tested carefully.

Please report issues or edge cases with save persistence, cancel/confirm flow, or reward popups before expanding the roster tables.