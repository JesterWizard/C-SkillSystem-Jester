# Test-only helpers for the Hex add-on

Everything in this folder is for testing `../Installer.event` in an emulator. **Test only, do not install.** Never give a ROM built with these files to anyone.

| File | Purpose |
|------|---------|
| `make_test_rom.sh` | Builds the test ROM in one Event Assembler run: the parent's `test/ToChapter4.event` (chapter-4 start), the parent's `Installer.event`, then this folder's `../Installer.event`. |
| `hex_screen.gbarun` | A gbarun script for the Hex row and screen: the prep route to the Hex row, the menu, Play, the hand picker, a full match, the stakes, back to the Preparations menu, the Cards viewer, and the convoy's Supply list. Writes `S2b_*.png` (1x; make the 2x copies with NEAREST). |
| `cards_supply.gbarun` | A gbarun script for the card add-on alone: the Supply list, R on Ross, and taking two cards. Writes `cards_supply.png`, `cards_help.png`, `cards_inventory.png`. |

## Build the test ROM

1. The parent's chapter-4 entry, from your own clean `fe8.gba` (read only). It is git-ignored:

   ```bash
   python3 Standalone/unlimited_prep_menu_options/test/make_ch4_entry.py /path/to/clean/fe8.gba
   ```

2. The test ROM. The clean ROM is copied to the output, which is then patched:

   ```bash
   bash Standalone/unlimited_prep_menu_options/hex/test/make_test_rom.sh /path/to/clean/fe8.gba /path/to/work/hex_test.gba
   ```

   It ends with `No errors. Please continue being awesome.`

## World-map state (`wm.ss`)

The scripts start from a world-map save state named `wm.ss` in the folder you run them from. Make it with the parent's route (`Standalone/unlimited_prep_menu_options/test/README.md`, "Route to the Preparations menu"): boot the test ROM, reach the world map at "Border Mulan", then `save wm.ss`. A state made before a rebuild of the add-on's hooks may not load (see the parent's note).

## Run

```bash
cd /path/to/work          # has wm.ss and hex_test.gba
gbarun hex_test.gba hex_screen.gbarun
gbarun hex_test.gba cards_supply.gbarun
```

gbarun notes: `press KEY n` holds the key for `n` frames. It does not repeat the press, so `press DOWN 2` moves once. Send one `press` per step. Lines starting with `#` are ignored.

## Test RAM writes (emulator only)

These writes are needed on the routes. They are not part of the install.

| Address | Write | Why |
|---------|-------|-----|
| `0x0202BCFE` (`gPlaySt+0x0E`) = `0x02` | Convoy access | Vanilla `HasConvoyAccess` (`0x0803161C`) reads this byte; it is 0 on this route, so Supply is greyed. Used by both scripts. |
| `0x0202BE4C` (`gUnitArrayBlue[0].pCharacterData`) = `0x08803D64` | Eirika's character data | The only deployable unit on this route is Artur. Vanilla `SupplyUsability` (`0x08023F64`) needs the active unit to be a lord or next to one. |
| `0x0203A81C` (`gConvoyItemArray`), 8 words | Starter card words | `cards_supply.gbarun` only. The Hex route gives its cards through the screen (the convoy gets the 7 starters itself). |

## What `hex_screen.gbarun` checks

Each step is a shot; check these in the picture:

- **`S2b_prep.png`**: the Preparations menu scrolled to the Hex row, with its help text "Play Hex with your army / and view your cards.".
- **`S2b_menu.png`**: A on Hex: the Play / Cards menu on the rune mural with blue windows.
- **`S2b_opp.png`**: Play: "Choose an opponent" with this chapter's living units and the highlighted unit's deck in the red column.
- **`S2b_pick.png`**: the hand picker ("Your cards", the copies still available, the blue hand filling up).
- **`S2b_pickdone.png`**: seven cards: "Your hand", Accept or Change, "Accept this hand?".
- **`S2b_match.png`**: mid-match board after two turns.
- **`S2b_end.png`**: the result banner after the match.
- **`S2b_take.png`**: the stakes after A on the banner (Take for a win, or "Your cards are safe this time." for a loss).
- **`S2b_back.png`**: B from the Hex menu: the Preparations menu, the cursor on Hex, no popup.
- **`S2b_cards.png`**: Cards: "Cards N/32", the owned counts, `???` for a type never owned, the preview of Ross.
- **`S2b_supply.png`**: Items → Supply → the card category: the starter cards (and a won card if you won) with `--` uses. The stock reads `7/100` (or `8/100` after a win).

## Match outcome

The match is random (the first player and the AI's choices come from the game clock), and the outcome differs between runs of the same script. The saved screenshots are from one run (a loss: "Defeat...", then "Your cards are safe this time."). A win was checked in an earlier run of the same route: "Victory!", then "Gained Ewan!" and the Supply list with a second Ewan (stock `8/100`). Those screenshots were overwritten.

## Results (checked in emulator session S2b)

- The route in `hex_screen.gbarun` runs to the end without an error. All the shots above were taken and read.
- The card route `cards_supply.gbarun` runs on this build too (session S2b): the Supply list with the card names and `--`, Ross's description on R ("A young villager. He longs to be a warrior, like his father."), and `6/100` after the two takes with Ross and Amelia in Artur's inventory.
