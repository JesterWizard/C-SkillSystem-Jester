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
- `minsp N` - run N frames one instruction at a time and print the lowest main stack pointer. The HQ sound mixer (`Contents/External/m4a_hq_mixer`) copies its code to IWRAM 0x03006CB0-0x03007708, right below the stack. An SP below 0x03007708 overwrites it, and the next sound interrupt crashes the game (it resets to the title). Check this after any change to the AI or the battle simulation: on the AI benchmark the enemy phase reaches about 0x03007740.
- `sram FILE` - load a battery save before reset. Must be the first command.

Boot script:

- `Tools/cloud/boot-to-prologue.txt` boots a fresh ROM with no save to the prologue map. It takes about 1 minute.
- It writes `boot.png` and the save state `boot.ss`.
- Run it from a scratch folder: `gbarun /path/to/fe8-kernel-dev.gba /path/to/Tools/cloud/boot-to-prologue.txt` (fe8-kernel-dev.gba is what build.sh produces; fe8.gba is the clean ROM.).

Prep screen route:

- `Tools/cloud/boot-to-prep.txt` runs the prologue boot, then the in-game Vesly debugger (B on an empty tile of the map).
  It opens "Ch. State" (page 2), and writes chapter index 4 into `gPlaySt.chapterIndex` (`0x0202BCFE`, `gPlaySt` is `0x0202BCF0`).
  It selects "Save & restart ch." (row 7), which starts chapter 4. Chapter 4's opening shows the Preparations screen.
  The script confirms the save slot (A, A) and waits. It takes about 3 seconds.
- Check `gPlaySt` in `fe8-kernel-dev.sym` after a rebuild. Its address moves if the struct moves.
- Prep menu input: a press shorter than about 4 frames can drop. Hold a direction for 4 frames and wait about 40 frames between presses.
- The prep menu's Hex row is the last one. Press DOWN 10 times from the top to reach it.

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
  - saves: the game save's and suspend save's chunks fit their SRAM blocks, which do not overlap. Eirika waits (the game
    writes its suspend save), a soft reset and Resume Chapter bring back her HP, her position and every district save
    chunk. The debugger's "Save & restart ch." and Restart Chapter do the same for the game save;
  - in Serafew, Eirika walks to every district spot (fishing, River Crossing, Tower Stack, guild board), A opens each
    screen and B brings the map back; a River Crossing game over pays the guild board's tokens; a met quest is claimed;
    the board sells the Omni Tonic and a skill scroll.
- No option (about 35 seconds): the same, with all four districts. `--only Jehanna tokens` runs named tests (`boot`,
  `statscreen`, `battle`, `battlequote`, `savelayout`, `suspend`, `gamesave`, a district's name, `tokens`, `claim`, `prizes`).
- It prints PASS or FAIL per check, writes `report.txt` and `sheet.png` (every opened screen, for a look by eye) to
  `--out` (default: a new temp folder), and exits with the number of failed checks.
- A district test ROM starts a new game in the district (chapter row 0 takes the district's map and events). Eirika
  walks there step by step from the spots in its `events/traps.c`: a step that does not move her (a wandering NPC, a
  tile free movement refuses) marks that tile blocked and the path is planned again. A spot no path reaches fails as
  unreachable, which is how the Port Kiris crossing spot on floor terrain was found.
- RAM addresses come from `fe8-kernel-dev.sym` and `include/link/config-memmap.o`, so they follow the build.
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
