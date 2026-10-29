# Arena Options

Three optional changes to the vanilla FE8U arena. Each one is a setting in `Installer.event` and installs its own hook, so you only patch what you turn on.

| Option | Setting | What it does |
|--------|---------|--------------|
| Upgraded player weapon | `ARENA_UPGRADED_WEAPONS` (on), `ARENA_WEAPON_UPGRADE` | The player fights with a better weapon than vanilla's iron tier. `1`: one tier up (Iron → Steel, Fire → Elfire, Lightning → Divine; Flux stays Flux). `2`: tier from level: unpromoted level 1-10 iron, 11-20 steel, promoted silver (Fimbulvetr / Divine / Flux for magic). Only tiers the unit can wield are used, so a unit with a low weapon rank stays on the best weapon it can use. |
| Arena limits | `ARENA_LIMITS` (off) | Per-chapter turn and level limits from a table. A unit over a limit sees the Arena command greyed out; selecting it shows why: *The arena is closed for the day.*, *Arena level limit: N.* or *Unpromoted units only.* |
| Roster menu | `ARENA_ROSTER_MENU` (off) | In chapters listed in the roster table, the arena master offers a list of fixed opponents (character, class, level, weapon, wager, prize) instead of one random opponent. Beaten opponents are greyed out for the rest of the chapter; after a set number of wins the arena is "cleaned out". Prizes are 2x or 3x the wager in gold, or an item. |

This is the C Skill System arena module (`Kernel/Wizardry/Arena/Arena.c`) designer-config flags `arena_let_player_use_upgraded_weapons`, `arena_calculate_weapon_based_on_level`, `arena_limits` and `arena_roster_menu`, installed on **clean FE8U** without the kernel. The integrated defaults (upgraded weapons and level-based tiers on, limits and roster off) are this package's defaults.

## Can it be installed with arena_show_opponent_in_advance?

Yes. This package never touches the two functions [arena_show_opponent_in_advance](../arena_show_opponent_in_advance/) replaces (`ArenaUi_WagerGoldDialogue`, `DrawArenaOpponentDetailsText`): the roster menu runs its own copy of the arena screen script and calls the vanilla `DrawArenaOpponentDetailsText`, which draws that package's window (with the matchup label) when it is installed and the vanilla window otherwise. Install both, each at its own free-space address.

That is why the roster is a separate package rather than settings inside arena_show_opponent_in_advance: the integrated roster code replaces those two functions, so folding it in would have made that package conflict with itself, while this way neither package hooks anything the other does.

## Configuration

Edit the settings at the top of `Installer.event`, then reinstall the patch. No C recompile is needed. To turn an option off, put `//` in front of its `#define` (its hook is then not installed).

| Constant | Default | Meaning |
|----------|---------|---------|
| `ARENA_UPGRADED_WEAPONS` | defined | Install the upgraded player weapon option |
| `ARENA_WEAPON_UPGRADE` | `2` | `1` = one tier up, `2` = tier from level |
| `ARENA_LIMITS` | not defined | Install the arena limits option |
| `ARENA_LIMIT_PROMOTED_BONUS` | `20` | Levels added to a promoted unit's level when comparing it with a level limit |
| `ARENA_ROSTER_MENU` | not defined | Install the roster menu option |
| `ARENA_ROSTER_SHOW_OPPONENT` | `1` | `1`: after picking an opponent, show the opponent window before the wager question (and after paying). `0`: keep the list on screen during the question and show no window. |
| `ARENA_ROSTER_FIRST_FLAG` | `0x21` | First chapter flag used to remember beaten roster opponents (see below) |

### Limits table

```
ArenaLimit(chapter, level limit, turn limit)
```

`0` means no limit of that kind. The arena closes after the turn limit (turn > limit). The level limit compares the unit's level, plus `ARENA_LIMIT_PROMOTED_BONUS` if promoted; a promoted unit whose limit is at most the bonus gets the *Unpromoted units only.* message. Chapters not in the table have no limits. The default table is the integrated one: chapter 6, level 5, turns 1-3.

### Roster table

```
ArenaRosterChapter(chapter, wins until cleaned out, entry count, entry list label)
ArenaRosterEntry(character, class, level, reward type, weapon, wager, reward item)
```

- `character` `0` uses the generic arena fighter and shows the class name in the list; any other character ID shows that character's name and uses their data.
- `reward type`: `0` = 2x the wager in gold, `1` = 3x, `2` = the reward item.
- `wins until cleaned out` `0` means the arena never closes; otherwise, once that many entries are beaten the arena master turns the unit away.
- The roster opponent always has S rank in the weapon's type, so it can use any weapon.

The default table is the integrated sample for chapter 6 (Joshua with a Killing Edge for an Audhulma prize, then five generic fighters). Chapters not in the table use the vanilla arena.

### Roster progress (save data)

Beaten entries are stored in **chapter flags**: entry `N` of a chapter (counting from 0) uses flag `ARENA_ROSTER_FIRST_FLAG + N`. Chapter flags are cleared when a chapter starts and kept in suspend data, which is exactly the lifetime the integrated roster progress had, so no save-format change is needed. Pick flags your chapter events don't use. FE8U only stores chapter flags `0x01` .. `0x28`, so a chapter can use at most `0x29 - ARENA_ROSTER_FIRST_FLAG` entries (8 with the default); extra entries are ignored.

## Text

- Built-in English strings (in `Source/ArenaOptions.c`): the roster welcome (*Welcome to the arena! Pick your opponent.*), *cleaned out* and item-prize lines, *Double Payout* / *Triple Payout*, and the three limit messages. To translate them, edit the C file and run `make`.
- Vanilla text IDs used unchanged: `0x8D2` (wager question), `0x8D4`, `0x8D6` - `0x8DA` (results), `0x8D5` / `0x8D3` (instructions, good luck), `0x853` / `0x854` (vanilla "can't enter the arena" help).

The limit messages are shown with the vanilla help box, which takes a text ID: the patch writes its string into the game's decoded-text buffer and opens the help box with text ID `0x854`, the ID it last decoded. If anything decoded another text in between, the vanilla `0x854` text would show instead.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000` (about 5.5 KB, `$1000000` .. `$1001610` with the default tables)

## Build

No build step is required to install the patch. `Source/ArenaOptions.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/ArenaOptions.c`:

```bash
make -C Standalone/arena_options
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `arena_options` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-arena-options.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, hooks, free-space placement, limits and roster tables |
| `Source/ArenaOptions.c` | C implementation |
| `Source/ArenaOptions.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` (`lyn -nohook`) |

## Hooks & Free Space

| Option | Function | Hook (`ORG`) | Overwritten |
|--------|----------|--------------|-------------|
| Upgraded weapon | `ArenaGenerateBaseWeapons` | `$31B70` | 8 bytes |
| Limits | `ArenaCommandUsability` | `$240EC` | 8 bytes |
| Limits | `ArenaCommandEffect` | `$24144` | 8 bytes |
| Roster | `StartArenaScreen` | `$B576C` | 8 bytes |

Each replacement keeps the vanilla behaviour for everything its option doesn't change. The roster menu runs `ProcScr_ArenaRosterMain` / `ProcScr_ArenaRosterResults`, copies of vanilla `gProcScr_ArenaUiMain` / `gProcScr_ArenaUiResults` with the welcome, wager and results steps replaced; chapters without a roster still start the vanilla script.

**Free space:** `$1000000` (code, the 4 setting bytes `gArenaOptionsConfig`, then the two tables). No RAM is reserved: the roster selection lives in the arena screen's proc.

## Conflicts

- **[arena_show_opponent_in_advance](../arena_show_opponent_in_advance/)**: compatible, see above.
- **[lvup_stat_points](../lvup_stat_points/)**: compatible (tested together); a level-up in an arena fight opens its menu between the fight and the results screen.
- Any patch that replaces vanilla `ArenaGenerateBaseWeapons`, `ArenaCommandUsability`, `ArenaCommandEffect` or `StartArenaScreen` (only for the options you install), including the full C Skill System arena rewrite.
- The roster's chapter flags must not be used by that chapter's events.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on every installed hook site and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Differences from the C Skill System

- **Weapon tier from level.** The integrated `ArenaGetUpgradedWeapon_NEW` steps through its upgrade table with `i += 4` and reads `table[i * 4]`, so only swords and anima tomes ever match (and it reads past the end of the table). Here every weapon type follows its tier chain. The integrated kernel also turns off vanilla's fallback weapon (`ArenaSetFallbackWeaponsMaybe`) for all arena fights, so a unit can get a weapon above its rank; here the upgrade stops at the best tier the unit can wield and the vanilla fallback stays.
- **Arena limits are not active in the integrated build.** Its hooks are only assembled under `CONFIG_ARENA_LIMITS`, which is never defined, so turning `arena_limits` on changes nothing there. This package ports what the code was meant to do, with its logic fixed: the integrated `CheckArenaLimits` adds `MaxLevel` (10) for promoted units and only refuses when the level is above 10 as well, and its "unpromoted only" branch can never be reached.
- **How a limit is shown.** The integrated code hides the Arena command and runs a text-box event from inside the menu availability check (every time the unit menu opens). Here the command is greyed out and the reason is shown in a help box when it is selected, like vanilla's "can't enter the arena" message.
- **Non-lethal arena dropped.** The integrated limits code also carries a `NonLethalArena` option (lose with 1 HP instead of dying). It is not installed in the integrated build either (`KillUnitIfNoHealth` is never hooked and the death-quote hook is behind the same undefined `CONFIG_ARENA_LIMITS`). Doing it properly on vanilla means suppressing the battle-animation death quote, which for lords also triggers the game over, through the defeat-talk functions [custom_defeat_quotes](../custom_defeat_quotes/) replaces, and stopping the map death fade; it was left out rather than shipped half-working.
- **Roster progress storage.** The integrated roster keeps its progress in a kernel save chunk (`SaveArenaRosterProgress`, 33 bytes, reset at chapter start). Here it is chapter flags (same lifetime, no save change), which limits a chapter to the entries that fit in flags `ARENA_ROSTER_FIRST_FLAG` .. `0x28` instead of 32.
- **Roster only where it exists.** With `arena_roster_menu` on, the integrated arena greets every chapter with *Pick your opponent.*, even chapters without a roster. Here chapters without a roster get the vanilla arena unchanged.
- **Item prizes.** The integrated roster hands the prize item over on the arena results screen, before vanilla copies the battle copy of the unit back (which overwrites the inventory on vanilla). Here the *Got …* popup comes on the map after the fight, so the item is kept (a full inventory gets vanilla's send-to-convoy / discard choice).
- **Opponent window.** The integrated roster draws its own opponent window; here it calls the vanilla `DrawArenaOpponentDetailsText` (or arena_show_opponent_in_advance's). The list's text tiles are released when the list closes, so the window drawn after paying doesn't overwrite the dialogue's tiles.
- **Skills.** The Ludopathy (double wager) and Concealed Weapon arena checks are Skill System skills and are not included.

## Limitations

- Suspending during a roster fight and resuming replays it the vanilla way, as a random-opponent fight with the vanilla payout; the roster entry is not marked as beaten. (The integrated roster behaves the same.)
- Roster progress is per chapter by design: it resets when the next chapter starts.
- Tested on FE8U with mGBA (with a test arena tile and test tables for the Prologue): upgraded weapon tiers, the level and turn limit messages, the roster list, a gold and an item prize, a beaten entry being refused, the "cleaned out" check, and the roster together with arena_show_opponent_in_advance and lvup_stat_points.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
