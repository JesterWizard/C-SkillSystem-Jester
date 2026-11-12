# Cloud sessions

Tools for Claude Code cloud sessions. Local machines use the normal devkitARM setup from `Documentation/Setup.md`.

## What's here

- `gritshim.py` - Python stand-in for devkitPro `grit` and `gbalzss`. Covers only the argument sets the makefiles use.
- `build.sh` - build wrapper. Builds `fe8-kernel-dev.gba` and keeps committed generated files unchanged.
- `protected-outputs.txt` - generated files that `build.sh` must not rewrite. One path per line.
- `gbarun.c` - headless mGBA runner. Plays a key script and takes screenshots and save states.
- `boot-to-prologue.txt` - `gbarun` script that boots a fresh ROM to the prologue map.
- `boot-to-prep.txt` - `gbarun` script that boots a fresh ROM to the Preparations screen (writes `prep.ss` and `prep.png`).
- `smoketest.py` - emulator smoke tests of the built ROM: boot, the stat screen, a battle and level-up, save and suspend round trips, every district spot, tokens and quests.
  See [Smoke tests](#smoke-tests).
- `.claude/hooks/session-start.sh` - session setup hook. Runs `build.sh` prerequisites and the rest of the setup below.

## Session setup

`.claude/hooks/session-start.sh` runs at the start of every cloud session. It only runs when `CLAUDE_CODE_REMOTE=true`. It is registered in `.claude/settings.json`. It is safe to run again.

The hook does this:

- Installs missing apt packages: `gcc-arm-none-eabi`, `binutils-arm-none-eabi`, `libnewlib-arm-none-eabi`, `libmgba-dev`, `libpng-dev`, `gawk`, `moreutils`.
  - `gawk` replaces the default `mawk` as `awk`. `Tools/scripts/lynjump-protector.sh` uses `strtonum`, which only gawk has. Under mawk every regenerated `.lyn.event` gets broken `PROTECT 0x... 0x... + 0x` lines and Event Assembler fails with `Expected identifier or literal, got ERROR: 0x.`.
  - `moreutils` provides `sponge`, which the makefile uses for the `.sym` step. Without it the build fails with Error 127 after the ROM is built.
- Installs Pillow if it is missing.
- Builds a devkitPro stand-in in `~/.cache/c-skillsys/devkitpro`:
  - `devkitARM/bin` holds symlinks to the Ubuntu `arm-none-eabi-*` tools.
  - `tools/bin/grit` and `tools/bin/gbalzss` point to `Tools/cloud/gritshim.py`.
- Compiles `Tools/cloud/gbarun.c` to `~/.cache/c-skillsys/gbarun` and puts it on `PATH`.
- Exports `DEVKITPRO`, `DEVKITARM` and `PATH` for the session.
- Clones the pinned `Tools/check_patch` and `Tools/FE-PyTools` checkouts. They are gitlinks with no `.gitmodules`, so a fresh clone leaves them empty.
- Fetches `fireemblem8.ref.o`, `fireemblem8.ref.s` and `fireemblem8.sym` into `Tools/FE-CLib-Mokha/reference/`. The makefile needs them, but `.gitignore` excludes them, so a clone has no `reference/` dir. Without them the build stops with `No rule to make target 'Tools/FE-CLib-Mokha/reference/fireemblem8.sym'`.
  - They come from upstream [FE-CLib-Mokha](https://github.com/MokhaLeee/FE-CLib-Mokha) at the commit in `CLIB_REF_SHA` in the hook. They are cached in `~/.cache/c-skillsys`.
  - The pin is `3503999` (2025-04-10). Its headers are closest to the vendored `Tools/FE-CLib-Mokha/include`, and it defines every vanilla symbol the repo's C code uses. Newer upstream commits rename symbols (e.g. `GetPlayerLeaderUnitId`), which breaks regenerating some `.lyn.event` files. Bump the pin only together with the vendored headers.
- Copies `fe8.gba` from the user's uploads if `fe8.gba` is missing.

ROM note:

- `fe8.gba` must be a clean FE8U ROM with sha1 `c25b145e37456171ada4b0d440bf88a19f4d509f`.
- The hook looks for it in `~/.claude/uploads`.
- `fe8.gba` is gitignored. Never commit it.
- If the hook finds no ROM, its summary says `fe8.gba MISSING`. Ask the user to upload one.

## Building

Run `Tools/cloud/build.sh` instead of plain `make`.

Why:

- The Ubuntu compiler makes different `.lyn.event` output than devkitARM.
- A plain `make` would rewrite the committed `.lyn.event` files.
- `build.sh` marks every tracked `.lyn.event` file, every path in `protected-outputs.txt`, and every tracked grit output of a tracked `.png` (`.img.bin`, `.map.bin`, `.pal.bin`, `.4bpp`, `.4bpp.lz`, `.gbapal`, `.lz77`) as up to date (`make -o`). A file is not protected if it is modified or if its `.c`, `.s` or `.png` source is modified. A `.c` or `.png` you edit still regenerates its own outputs.
- The grit outputs need this because `gritshim.py` output can differ from real grit (e.g. the `Spell_b_012`/`Spell_b_039` CSA atlas maps), and a fresh clone's file times can make any `.png` look newer than its outputs.
- After the build it restores `Kernel/Wizardry/BuildVersionTitleScreen/BuildVersionTitleScreen_Installer.event`, which the build stamps.

Output:

- Error and warning lines (`error`, `Assertion`, `warning`).
- The kernel text size line (`Kernel______Text`).
- The list of compiled files (`[CC ]`).
- The last 3 lines of the log.
- `BUILD rc=N`. `N` is the exit code of `make`. `0` means success.

The full log is in `/tmp/c-skillsys-build.log`. Set `BUILD_LOG` to use another path.

The script stops early if `DEVKITPRO` is unset (run the hook) or `fe8.gba` is missing.

## Emulator

Run `gbarun ROM SCRIPT`. Paths in the script are relative to the current directory. Run it from a scratch folder, not the repo root.

Script commands, one per line. `#` starts a comment.

- `wait N` - run N frames with no keys.
- `press KEYS [N]` - hold KEYS for N frames (default 2), then release for 6 frames. KEYS are `A B L R START SELECT UP DOWN LEFT RIGHT`, joined with `+`.
- `hold KEYS N` - hold KEYS for N frames with no release gap.
- `shot FILE.png` - take a screenshot.
- `save FILE` - write a save state.
- `load FILE` - load a save state.
- `w8|w16|w32 ADDR VAL` - write memory.
- `r8|r16|r32 ADDR` - print memory.
- `dump ADDR LEN FILE` - dump memory to a file.
- `prof N ADDR [NAME [WITHIN]]` - run N frames one instruction at a time and time every call to the function at ADDR. Prints the call count, the total and the slowest call in CPU cycles and in frames (280896 cycles per frame). Take ADDR from `fe8-kernel-dev.sym`; for a vanilla function the kernel replaces, use the kernel address, not the vanilla one. Halted CPU time is skipped, so a few thousand frames take seconds. Example: profile `CpDecide_Main` after ending the turn to see how long each AI decision takes. With WITHIN (another function address), only calls made while that function runs count, e.g. `prof 1500 <GetUnitMaxHp> HpGetter <CpDecide_Main>` for the HP getter calls of the AI decisions alone.
- `minsp N` - run N frames one instruction at a time and print the lowest main stack pointer. The HQ sound mixer (`Contents/External/m4a_hq_mixer`) copies its code to IWRAM 0x03006CB0-0x03007708, right below the stack. An SP below 0x03007708 overwrites it, and the next sound interrupt crashes the game (it resets to the title). Check this after any change to the AI or the battle simulation: on the AI benchmark the enemy phase reaches about 0x03007740, and the prologue's enemy phase about 0x030078F8. A log call (`NoCashGBAPrintf`, `Warn`, `Errorf`, ...) formats with `vsprintf`, which needs about 0x250 bytes of stack: one on the battle simulation's path (the skill list's "Generic buffer used" warning) took the prologue's enemy phase to 0x030076D4, and the game froze at the next sound.
- `sram FILE` - load a battery save before reset. Must be the first command.

Boot script:

- `Tools/cloud/boot-to-prologue.txt` boots a fresh ROM with no save to the prologue map. It takes about 1 minute.
- It writes `boot.png` and the save state `boot.ss`: turn 1, the cursor on Eirika, nobody moved. With real-time battle
  off (`real_time_battle`, the default), no key is pressed once the opening is over: on the map a press would select
  Eirika. The tests that fight start from turn 2 instead (`turn2_state` in `smoketest.py`: End, and the enemy phase
  brings two soldiers next to her).
- Run it from a scratch folder: `gbarun /path/to/fe8-kernel-dev.gba /path/to/Tools/cloud/boot-to-prologue.txt` (fe8-kernel-dev.gba is what build.sh produces; fe8.gba is the clean ROM.).

Prep screen route:

- `Tools/cloud/boot-to-prep.txt` runs the prologue boot, then the in-game Vesly debugger (B on an empty tile of the map).
  It opens "Ch. State" (page 2), and writes chapter index 4 into `gPlaySt.chapterIndex` (`0x0202BCFE`, `gPlaySt` is `0x0202BCF0`).
  It selects "Save & restart ch." (row 7), which starts chapter 4. Chapter 4's opening shows the Preparations screen.
  The script confirms the save slot (A, A) and waits. It takes about 3 seconds.
- Check `gPlaySt` in `fe8-kernel-dev.sym` after a rebuild. Its address moves if the struct moves.
- Prep menu input: a press shorter than about 4 frames can drop. Hold a direction for 4 frames and wait about 40 frames between presses.
- The prep menu's Hex row is the last one. Press DOWN 10 times from the top to reach it.

World map route (`smoketest.py --only worldmap`, `test_worldmap`):

- Start from a district test state (`test_district` writes `<out>/Serafew/start.ss` and its test ROM `t.gba`).
- Before leaving, set up the world map in RAM:
  - `gWmStoryChapter` to a story chapter;
  - the lord's node, `gGMData` (`0x03005280`) `+0x11`;
  - node states at `+0x30 + 4*node` (1: shown and cleared; the world map marks the story node itself);
  - the district's unlock flag: permanent flag `f` is bit `(f - 0x65)` of `gPermanentFlagBits` (`0x03005250`).
- Leave through the gate (DOWN onto it, A, LEFT for Yes, A), confirm the save menu (A, A), and wait about 15 seconds
  for the beginning scene. The world map is then idle with the cursor on the lord.
- A on the lord's node opens the node menu. On a fresh save, vanilla opens the general menu instead while the next
  uncleared node is node 2 (a tutorial guard), so mark the earlier nodes cleared. L moves the cursor to the next node.
- Shops from the node menu (`--only wmshops`, `test_wmshops`): with the district locked, the Armory row is first and
  the Shop row one down. A opens vanilla's world-map base menu (`ProcScr_GmapBaseMenu`; byte `+0x29` of its proc is the
  kind, 0 armory, 1 vendor), A again opens the shop (`gProcScr_Shop`; its stock at `+0x30`, 20 `u16`), and B six times
  (90 frames apart) is back on the idle world map. `gbarun` can't press keys during `prof`, so the test dumps
  `sProcArray` (64 procs of `0x6C` bytes) and reads the procs instead.
- The postgame (`--only postgame`, `test_postgame`): set the story on Chapter 20 with the lord on Rausten Court, nodes
  0-24 and 26 shown and roads 0-30 open (`+0xA4` list, `+0xC4` count), as the story would leave them. Travel to Black
  Temple, skip Chapters 20-22 (START, A; the save menu after Chapter 22 writes the Creature Campaign save), and A on
  "Fin" goes on to the postgame world map. After a soft reset the test reads the save menu's proc (the slots' titles,
  `0x57` "Creature Campaign"; the Extras rows) and Restart Chapter loads the postgame again. Boot first: the end save
  goes to SRAM. A trip in the postgame: the
  cursor on the node, A, then about 10 x 120 frames, with the monsters (`gGMData.units[4..7]`) cleared each time.
- Saving from the world map (`--only wmsave`, `test_wmsave`): move the cursor off every node (write `ix`/`iy` at
  `+8`/`+0xC`, `<< 8`), A opens the general menu (Unit, Status, Options, Save, Home), DOWN 3 times, A, then A (the first
  slot) and A (Save). After a soft reset, A on the title, then A on the first slot (Restart Chapter) loads the world map.
- SRAM is not in a save state, and a soft reset treats SRAM the game never set up as empty. A test that saves after
  loading a state must let the game boot first (`wait 600` before `load`), in the same run.

Save state caveat:

- A save state only works with the ROM build it was made from. Text and graphics pointers move between builds.
- After a rebuild, boot again. Do not load an old state.

Useful RAM (FE8U):

- Player unit structs start at `0x0202BE4C`, 0x48 bytes each. Eirika is first. Seth is at `0x0202BE94`.
- Enemy units start at `0x0202CFBC + i*0x48`.
- Unit fields:
  - `+0x08` level.
  - `+0x09` EXP.
  - `+0x12` max HP.
  - `+0x13` current HP.
- With `CONFIG_TURN_ON_ALL_SKILLS` (on in `include/configs/configs.h`), skills are packed 10 bits each into `unit+0x32..+0x38`. That is 5 slots.
  - Example: `w8 <unit+0x32> 0x7D` and `w8 <unit+0x33> 0x01` give slot 0 the skill ID `0x17D`.

Stat screen tip:

- Press R on a unit on the map to open its stat screen.
- Press RIGHT to page to "Weapon & Skills".

## Smoke tests

`Tools/cloud/smoketest.py` plays the built ROM in `gbarun` and checks that the main screens still work. Build first.

```bash
python3 Tools/cloud/smoketest.py --quick
```

- `--quick` (about 20 seconds):
  - a new game reaches the prologue map; the stat screen opens and closes;
  - battle: Eirika attacks the soldier next to her. Str Bonus in a free skill slot raises the forecast's attack, the
    fight hurts the soldier, and with 99 EXP she levels up; her level-up quote shows the expression its tier asks for.
    Moved next to her, O'Neill's battle quote shows her `[Surprised]` (a test ROM maps her sad portrait to it);
  - EXP bar styles: with the Options value set to a style, the map EXP bar and the battle-animation one load that
    style's palette and window tiles (`--only expbar`);
  - Map Colors: the Options row set to Vivid and then Deep redraws the map in those colours, every map colour matching
    a model of the transform; a map palette animation (Chapter 18's, on the prologue in a test ROM) runs after a normal
    boot and is recoloured too (`--only mapcolors`);
  - Gallery: the prologue's opening unlocks two CGs; Extras > Gallery shows its rows, A shows an unlocked CG full size,
    right goes to the next, B back to the grid, down scrolls a row on screen, B back to Extras (`--only gallery`);
  - Chapter 1's end: the debugger starts Chapter 1 and Eirika seizes the gate; the Intermission (0x38) becomes the
    story chapter and Chapter 2 follows, not the prologue (`--only ch1end`);
  - New Game+: Extras has the row only once a save has been to the postgame; New Game+ from that save starts a new
    game in which Eirika has the skills she had equipped there and its skill list (`--only newgameplus`);
  - saves: the game save's and suspend save's chunks fit their SRAM blocks, which do not overlap. Eirika waits (the game
    writes its suspend save), a soft reset and Resume Chapter bring back her HP, her position and every district save
    chunk. The debugger's "Save & restart ch." and Restart Chapter do the same for the game save;
  - in Serafew, Eirika walks to every district spot (fishing, River Crossing, Tower Stack, guild board), A opens each
    screen and B brings the map back; a River Crossing game over pays the guild board's tokens; a met quest is claimed;
    the board sells the Omni Tonic and a skill scroll.
- No option: the same, with all four districts, and in Port Kiris the Pied Piper's Snek game (Yes, 80 points, B: 10
  tokens and the high score). `--only Jehanna tokens` runs named tests; `smoketest.py --help` lists them all (the world
  map, the stub chapters' battles, the postgame, saves and resumes among them).
- `--jobs N` runs N tests side by side, each in its own `gbarun` process (default: the CPU count). The full run took
  about 30 seconds with 4 jobs and 90 with `--jobs 1`. Tests that share a folder (`boot/`, `battle/`, `Serafew/`) run
  one after another in one job, and a test runs after the state it starts from (the boot, a district). The report and
  the contact sheet keep the same order whatever N is; the live PASS/FAIL lines print as tests finish.
- It prints PASS or FAIL per check, writes `report.txt` and `sheet.png` (every opened screen, for a look by eye) to
  `--out` (default: a new temp folder), and exits with the number of failed checks.
- A district test ROM starts a new game in the district (chapter row 0 takes the district's map and events). Eirika
  walks there step by step from the spots in its `events/traps.c`: a step that does not move her (a wandering NPC, a
  tile free movement refuses) marks that tile blocked and the path is planned again. A spot no path reaches fails as
  unreachable, which is how the Port Kiris crossing spot on floor terrain was found.
- RAM addresses come from `fe8-kernel-dev.sym` and `include/link/config-memmap.o`, so they follow the build. `make`
  deletes `config-memmap.o` as an intermediate, so `build.sh` builds it again at the end. Without it, the checks that
  need RAM symbols fail with "no symbols" (after a plain `make`, run `make include/link/config-memmap.o`).
- The save tests boot in the same `gbarun` run as the reset: a save state does not hold SRAM, so a test that loads one
  and resets finds a blank save. Before saving they write sentinel values (guild tokens, River Crossing and Tower Stack
  bests, Snek's high score) and dump every district chunk (the `SAVE_RAM` list in the script). After saving they wipe
  those chunks in RAM, so only a load brings them back. A chunk whose save or load is wrong fails by name. When you add
  a save chunk, add its save function and RAM symbol to `SAVE_RAM`.
- An A press as a wandering NPC starts its step (30 frames after the player stops) is ignored, in game too; the test
  tries again after another wait. The map can come back scrolled after a screen closes, so "back to the map" compares
  the screens' colours as well as their pixels.

## Toggling CONFIG_HEX_CARDS

`build.sh` leaves a tracked `.lyn.event` alone when its `.c` is unchanged in git, so after turning `CONFIG_HEX_CARDS` on or off the guarded outputs keep the old content. The off build then fails with `DuraItem.lyn.event:33:6 Undefined identifier: HexCard_NameId`.

Delete the guarded outputs and build again (see `Kernel/Wizardry/Minigames/Hex/README.md`):

```
rm -f Data/ItemSys/Source/Items.lyn.event Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/DuraItem.lyn.event Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ItemInfos.lyn.event Kernel/Wizardry/SkillEffects/MenuSkills/Reinforce.lyn.event Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.lyn.event Data/Hex/HexCardTypes.lyn.event
Tools/cloud/build.sh
```

## Coding style

- `./check_codingstyle.sh <file.c>` works once the hook has fetched `Tools/check_patch`.
- Run it from the repo root.
