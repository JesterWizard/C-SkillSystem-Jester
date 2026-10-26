# Skip Intro

Gets a new game onto the Prologue map as fast as possible and removes the chapter title cards:

- **Opening monologue** ("In an age long past...") shown when a new game starts: skipped.
- **Prologue world-map narration** ("The continent of Magvel...", the five nations): skipped. The world map still loads; the narration script is replaced by the short tail vanilla runs when you press START during it.
- **Chapter title card** ("Prologue: The Fall of Renais", "Chapter 1: ...") at the start of every chapter: skipped.

Chapter opening events (dialogue on the battle map) and the boot sequence (Nintendo / Intelligent Systems logos, the opening movie, the title screen) are unchanged.

This matches the integrated C Skill System behaviour for the `skip_intro` flag, adapted to vanilla FE8U (see below). It installs on **clean FE8U** without the kernel.

### Mapping from the integrated version

| Integrated (`skip_intro = true`) | This package |
|---|---|
| `BmMain_StartIntroFx` returns at once (`MiscFunctions.c`) | Same: `bx lr` at the top of `BmMain_StartIntroFx` |
| `StartIntroMonologue` returns at once (`CustomCampaign/Event/World_Map_Events.c`) | Same: `bx lr` at the top of `StartIntroMonologue` |
| `WorldMap_CallBeginningEvent` runs `EventScrWM_PrologueSkip` instead of the custom campaign's voiced Prologue script for `gmapEventId == 1` | `Events_WM_Beginning[1]` (vanilla's Prologue world-map script) is repointed to an equivalent short script. Vanilla's own `WorldMap_CallBeginningEvent` is not replaced. |

The integrated kernel also has a base-chapter intro flag (`CONFIG_BASE_CHAPTERS`) in `BmMain_StartIntroFx`; that is kernel-only and is not part of this package.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks: `BmMain_StartIntroFx` (`0x0801550C`), `StartIntroMonologue` (`0x080C541C`), `Events_WM_Beginning[1]` (`0x088B39F0`)
- Free space: `$1000000` (the replacement world-map script)

## Build

There is nothing to build: the patch is two `bx lr` instructions, one pointer and a short event script, all in `Installer.event`.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `skip_intro` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-skip-intro.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: two `bx lr` patches, the `Events_WM_Beginning[1]` pointer and the replacement Prologue world-map script |

## Hooks & Free Space

| Site | `ORG` | Overwritten |
|------|-------|-------------|
| `BmMain_StartIntroFx` | `$1550C` | 2 bytes (`bx lr`) |
| `StartIntroMonologue` | `$C541C` | 2 bytes (`bx lr`) |
| `Events_WM_Beginning[1]` | `$8B39F0` | 4 bytes (`POIN`) |

**Free space:** `$1000000`, 100 bytes (`SkipIntro_PrologueWorldMap`). No RAM.

The replacement script keeps the world-map state that vanilla leaves behind (Eirika spawned as world-map lord 0 on node 0 and then hidden, camera centred on her, the skip/no-fade bits) and runs vanilla's post-skip cleanup (`WM_FXCLEAR1/2`, remove sprites 2–6). The original script at `0x08A39768` is left in the ROM, unused.

## Conflicts

- Any patch that replaces or hooks `BmMain_StartIntroFx`, `StartIntroMonologue` or `WorldMap_CallBeginningEvent`, including the full C Skill System.
- Editing the Prologue world-map event in FEBuilder after installing edits this package's short script (it is the one `Events_WM_Beginning[1]` points to).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on every hook site and on the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Limitations

- All title cards are skipped, including ones with no story text, and there is no option to keep some.
- Only the Prologue world-map narration is skipped. Later world-map scenes (travel between chapters, chapter intros on the world map) still play and can still be skipped with START as in vanilla.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
