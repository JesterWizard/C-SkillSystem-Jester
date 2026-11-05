# Test-only helpers for the Hex Cards add-on (do not install)

Everything in this folder is for testing `../Installer.event` in an emulator. **Test only, do not install.** Never give a ROM built with these files to anyone.

| File | Purpose |
|------|---------|
| `make_test_rom.sh` | Builds the test ROM in one Event Assembler run: the parent's `test/ToChapter4.event` (chapter-4 start), the parent's `Installer.event`, then this folder's `../Installer.event`. |
| `cards_supply.gbarun` | A gbarun script: the prep route to chapter 4, the test RAM writes, the Supply list, R on Ross, and taking two cards. Writes `cards_supply.png`, `cards_help.png`, `cards_inventory.png`. |

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

The script starts from a world-map save state named `wm.ss` in the folder you run it from. Make it with the parent's route (`Standalone/unlimited_prep_menu_options/test/README.md`, "Route to the Preparations menu"): boot the test ROM, reach the world map at "Border Mulan", then `save wm.ss`. A state made before a rebuild of this add-on's hooks may not load (see the parent's note).

## Run

```bash
cd /path/to/work          # has wm.ss and hex_test.gba
gbarun hex_test.gba cards_supply.gbarun
```

gbarun notes: `press KEY n` holds the key for `n` frames. It does not repeat the press, so `press DOWN 2` moves once. Send one `press` per step.

## Test RAM writes (emulator only)

These writes are needed on this route. They are not part of the install.

| Address | Write | Why |
|---------|-------|-----|
| `0x0203A81C` (`gConvoyItemArray`, 100 words) | `0x00C1` Ross, `0x01C1` Amelia, `0x02C1` Ewan, `0x03C1` Franz, `0x1CC1` Glen, `0x1EC1` Lyon, `0x1FC1` Eirika, `0x2E01` Iron Sword | Card words `(type << 8) \| 0xC1`. The Iron Sword is a vanilla control. `GetConvoyItemCount` counts the non-zero slots, so no count byte is needed. |
| `0x0202BCFE` (`gPlaySt+0x0E`) = `0x02` | Convoy access | Vanilla `HasConvoyAccess` (`0x0803161C`) reads this byte; it is 0 on this route, so Supply is greyed. |
| `0x0202BE4C` (`gUnitArrayBlue[0].pCharacterData`) = `0x08803D64` | Eirika's character data | The only deployable unit on this route is Artur. Vanilla `SupplyUsability` (`0x08023F64`) needs the active unit to be a lord (character 1 or 15) or next to one. Artur is shown as Eirika in RAM from then on. |

Without these three writes the Supply entry does not open. The convoy write alone is not enough.

## What the script checks

- **`cards_supply.png`**: Supply, Take, the item category. The list shows `Ross`, `Amelia`, `Ewan`, `Franz`, `Glen`, `Lyon`, `Eirika` with `--` uses and the card icon. The stock reads `8/100`.
- **`cards_help.png`**: R on Ross shows "A young villager. He longs to be a warrior, like his father." (Ross's character description).
- **`cards_inventory.png`**: after two takes (Ross, then Amelia), Artur's inventory lists `Ross --` and `Amelia --`, and the stock reads `6/100`.

For the vanilla control, the weapon category lists `Iron Sword` with `46` uses. The name path for every other item is unchanged.

## Results (checked in emulator session S2a)

Screenshots are in the S2a scratchpad, not in the repo. Checked on the final build (icon `$84`): all three screens above, and the Iron Sword control. Screenshots are 1x (240x160) and 2x NEAREST.
