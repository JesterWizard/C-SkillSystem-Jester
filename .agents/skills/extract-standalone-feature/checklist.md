# Standalone Feature Package Contract

Use this checklist when extracting a C Skill System feature into `Standalone/<feature_name>/`.

## Required layout

```text
Standalone/<feature_name>/
  Installer.event
  Source/                    # C or asm implementation
  makefile                   # compiles Source/*.c to *.lyn.event
  README.md
```

## Package rules

- Target **clean FE8U (USA)** unless the feature explicitly documents another ROM.
- Install through **Event Assembler** using `Installer.event`.
- Keep the patch **self-contained**: no dependency on C Skill System kernel, relocation tables, designer config, or project memmap regions.
- Put address `#define`s and hook placement in **`Installer.event`**.
- Implement logic in **`Source/*.c`** and compile with a local **`makefile`** to `Source/*.lyn.event`.
- Prefer the **smallest vanilla hook** that preserves unrelated behavior.
- Place new code in **standalone free space** starting at **`$1000000`** using EA `$offset` ORG.
- Use **`$offset` form** in `ORG` and `PROTECT`; EA treats these as ROM file offsets, not full `0x08xxxxxx` addresses.
   - Add **`PUSH` / `POP`**, **`PROTECT`**, and **`ALIGN 4`** where appropriate.
   - Protect both the **hook site** and the **installed body** (`PROTECT freeSpaceStart CURRENTOFFSET`) so overlapping patches fail with a clear EA error.
- Document **conflicts**, **free-space address**, and **hook address** in `README.md`.

## Dependency audit worksheet

Before extracting, answer:

1. Where is the feature configured? (`configs.h`, designer config, data tables)
2. What is the runtime entry point? (vanilla replacement, hook table, menu, item effect)
3. What shared symbols does it use? (`gpKernelDesignerConfig`, SkillSys APIs, save/RAM symbols)
4. Can a **leaf hook** implement the behavior without importing the whole subsystem?
5. Does it need **ROM only**, or also **RAM / save / text / graphics**?
6. Which **generated artifacts** must be checked in so users do not need to compile C?

## Validation gates

- [ ] `make` succeeds in the standalone folder
- [ ] EA assembles with `ColorzCore A FE8 -input:Installer.event -output:<rom>`
- [ ] Hook bytes and pointer target verified
- [ ] No writes to ROM offset `0` / header corruption
- [ ] No unresolved C Skill System globals in standalone build
- [ ] README lists hook, free space, conflicts, build, and install steps
- [ ] If RAM/save is used: run `make ramcheck` / `make savecheck` and keep allocations even-sized

## Reference example

See `Standalone/two_random_number_growths/`:

- Leaf hook on vanilla `GetStatIncrease`
- C source in `Source/GetStatIncrease_2RN.c`
- Local `makefile` produces `Source/GetStatIncrease_2RN.lyn.event`
- Addresses, hook, and free-space ORG in `Installer.event`
- Body placed at `$1000000`
