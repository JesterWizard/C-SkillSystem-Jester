# Test-only helpers (do not install)

Everything in this folder is for testing the Unlimited Prep Menu Options patch in an emulator.
**Test only, do not install.** Never give a ROM built with these files to anyone.

| File | Status | Purpose |
|------|--------|---------|
| `TestRow.c` / `TestRow.event` / `makefile` | Builds and assembles. Works in the emulator. | One extension row in slot 0: title "Test", help "Test row". It follows the add-on contract in `../README.md` ("Writing an extension row"): selecting it sets `proc->state` to `0x40` and goes to label `0xA`. Its `startSubmenu` starts a blocking child proc (`ProcScr_PMXTestRow`) with a red backdrop for 60 frames. The child's end sets `proc->cur_cmd = 0x40` and `proc->state = 1`, so the menu returns through label 9. |
| `ToChapter4.event` | Assembles. Works in the emulator: a new game starts on chapter 4. | Copies chapter 4's chapter-data entry over chapter 0's entry. `GetROMChapterStruct` (0x08034618) returns `gChapterDataTable` (ROM `0x088B0890`, file offset `$8B0890`) plus `id * 0x94`. The entry is `Ch4Entry.bin`, chapter 4's 0x94 bytes from the clean `fe8.gba`, copied by a script. |

## Assembly order (test ROM)

Build the ROMs with ColorzCore from a copy of the clean ROM, in this order. The ROM is the `-output`, and `-input` is the event script (see `../README.md`, "Event Assembler"):

1. `test/ToChapter4.event` alone (the chapter-4 route): `ColorzCore A FE8 -input:van4.event -output:van4.gba -I:<EA> -I:<patch>`.
2. `Installer.event` on top of `van4.gba` (`#include "Extensions/Hack Installation.txt"` first, as in the README).
3. For the test row only: `test/TestRow.event` in the **same run** as step 2. Slot 0 is written with `PUSH / ORG $1000004 / POIN / POP`, which only works while the slots are not protected (see the README, "Free space").

## Route to the Preparations menu (emulator)

Boot a fresh ROM, then (`gbarun` scripts, one key per line):

- From boot: `wait 200`, `press START`, `wait 400`, `press START`, `wait 120`, then five times `press A`, `wait 120`, then three times `press START`, `wait 60`. This reaches the world map at "Border Mulan". Save a state there (`save wm.ss`).
- From `wm.ss`: six times `press A`, `wait 50`, `press START`, `wait 50`, then `wait 40`. This reaches the chapter 4 Preparations menu (title "Chapter 04", rows Pick Units, Items, Support, Check Map, Save).

A world-map state (`wm.ss`, made from the chapter-4 ROM) loads into the rebuilt patched ROMs and reaches the prep menu. That is a test shortcut, not a guarantee. Do not load a save state made before a rebuild of the test row: the code addresses can move.

## Test row route (checked)

From `wm.ss`, the six `A` / `START` pairs above, then:

- Five times `press DOWN 6`, `wait 40`: the cursor is on Test (row 5).
- `press A 4`, `wait 40`: the child screen (red backdrop).
- `wait 150`: the menu is back with the cursor on Test, its help "Test row", and no popup.
- `press UP 6`, `wait 40`: the cursor is on Save, with the vanilla Save description.

Checked in emulator session S1c (screenshots are in the scratchpad, not in the repo).

## Support and Items (checked)

- Support (two `DOWN`s, then `A`): the unit list "Select Character" opens. The pat4 and van4 screenshots are byte-identical in size and look the same.
- Items (one `DOWN`, `A`, then `B`): back on the menu with the cursor on Items and its vanilla description.

## Fixed: the "Rank / Augury" popup after the test row

Before the fix, the test row returned with `Proc_Goto(proc, 0xD)`. Label 0xD of the AtMenu script runs the old `AtMenu_CtrlLoop` (`0x0809602C`). Its A branch sets `proc->state = 4` (hard-coded) and goes to label 8, then label 2, then `AtMenu_StartSubmenu` case 4 (`StartFortuneSubMenu(2, proc)`), which draws the "Rank" / "Augury" popup. B closes it. In the old build the popup showed about 60 frames after the A press with no second press. The reason A is seen as new by `AtMenu_CtrlLoop` at that time is not proven. A stale `gKeyStatusPtr->newKeys` A is the likely cause.

The fix: the child's end sets `state = 1` (label 9, which does not run `AtMenu_CtrlLoop`), the same as the kernel's Hex row. The menu code is unchanged.
