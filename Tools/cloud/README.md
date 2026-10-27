# Cloud sessions

Tools for Claude Code cloud sessions. Local machines use the normal devkitARM setup from `Documentation/Setup.md`.

## What's here

- `gritshim.py` - Python stand-in for devkitPro `grit` and `gbalzss`. Covers only the argument sets the makefiles use.
- `build.sh` - build wrapper. Builds `fe8-kernel-dev.gba` and keeps committed generated files unchanged.
- `protected-outputs.txt` - generated files that `build.sh` must not rewrite. One path per line.
- `gbarun.c` - headless mGBA runner. Plays a key script and takes screenshots and save states.
- `boot-to-prologue.txt` - `gbarun` script that boots a fresh ROM to the prologue map.
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
- `sram FILE` - load a battery save before reset. Must be the first command.

Boot script:

- `Tools/cloud/boot-to-prologue.txt` boots a fresh ROM with no save to the prologue map. It takes about 1 minute.
- It writes `boot.png` and the save state `boot.ss`.
- Run it from a scratch folder: `gbarun /path/to/fe8-kernel-dev.gba /path/to/Tools/cloud/boot-to-prologue.txt` (fe8-kernel-dev.gba is what build.sh produces; fe8.gba is the clean ROM.).

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

## Coding style

- `./check_codingstyle.sh <file.c>` works once the hook has fetched `Tools/check_patch`.
- Run it from the repo root.
