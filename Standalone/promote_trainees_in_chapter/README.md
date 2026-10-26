# Promote Trainees In Chapter

In vanilla FE8, a trainee (Ross, Amelia, Ewan, or any class with the "max level 10" attribute) that reaches level 10 during a chapter only promotes at the next preparation screen. With this patch the vanilla promotion screen opens right after the battle in which the trainee reached level 10, so the class change happens in the chapter.

- Works in player phase and enemy phase, with battle animations on or off.
- The unit picks its new class on the vanilla promotion screen. Choosing **Cancel** leaves it a level 10 trainee, which vanilla's preparation-screen promotion still picks up (not runtime-tested).
- No inventory item is used up by the promotion.

This matches the integrated C Skill System `promote_trainees_in_chapter` designer-config flag (see [TraineeInChapterPromotion.md](../../Documentation/Features/TraineeInChapterPromotion.md)), but installs on **clean FE8U** without the kernel. It hooks a different place than the integrated code; see [Differences from the C Skill System](#differences-from-the-c-skill-system).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: the `BATTLE_HandleCombatDeaths` step of vanilla `sProcScr_CombatAction` (routine pointer at `$59DB10`)
- Free space: `$1000000` (204 bytes, `$1000000` .. `$10000CC`)

## Build

No build step is required to install the patch. `Source/PromoteTraineesInChapter.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/PromoteTraineesInChapter.c`:

```bash
make -C Standalone/promote_trainees_in_chapter
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `promote_trainees_in_chapter` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-trainee-promotion.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hook and free-space placement |
| `Source/PromoteTraineesInChapter.c` | C implementation |
| `Source/PromoteTraineesInChapter.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Location | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `sProcScr_CombatAction` (`$59DABC`), command `PROC_CALL(BATTLE_HandleCombatDeaths)` at `$59DB0C` | `$59DB10` | 4 bytes (routine pointer, repointed with `POIN`) |

The new routine, `BATTLE_HandleCombatDeaths_PromoteTrainees`, runs vanilla `BATTLE_HandleCombatDeaths` first. By then the battle results are applied and defeated units are handled. It then checks both battle units: a unit whose class or character has `CA_MAXLEVEL10`, that gained a level in this battle and is now level 10 or higher, starts `StartBmPromotion` as a blocking child of the combat proc. The next script command yields, and the combat action ends when the promotion is over.

**Free space:** `$1000000`.

## Conflicts

- Any patch that repoints or rewrites `sProcScr_CombatAction` (`$59DABC` .. `$59DB24`), in particular the `BATTLE_HandleCombatDeaths` pointer at `$59DB10`, including the full C Skill System.
- [promotion_on_max_level](../promotion_on_max_level/) repoints the same `$59DB10` pointer, so only one of the two can be installed. To get both behaviours, install `promotion_on_max_level` alone with `PROMOTE_TRAINEES_AT_LEVEL_10` set to `1`.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$59DB10`, 4 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Differences from the C Skill System

- **Hook point.** The integrated code starts the promotion from `ManimLevelUp_ScrollOut`, at the end of the map level-up window, while the battle is still being animated. On vanilla FE8U that is too early: the battle results are applied to the units only after the animation, and the promotion reuses the battle structs. Tested on clean FE8U with that approach, the level-up gains were lost, the promotion bonuses were applied twice, the level and EXP went back to the pre-battle values and the battle's weapon use was lost. This patch starts the promotion after the battle results and deaths are handled instead, which gives the correct result.
- Because of that, it also works with battle animations on. The integrated version only runs from the map level-up window.
- The promotion handler's item slot is set to `-1`, so the promotion doesn't use up a use of the weapon the battle left in `gActionData.itemSlotIndex` (vanilla promotion would otherwise call `UnitUpdateUsedItem` on it).
- `gActionData.subjectIndex` is restored to its previous value after the promotion starts, rather than set to `0`.

## Limitations

- Only map combat runs through `sProcScr_CombatAction`. Arena fights, staff/dance EXP and event-given levels don't trigger the promotion; those trainees promote at the next preparation screen as in vanilla.
- The vanilla in-battle promotion flow shows the class choice screen directly. The trainee dialogue that vanilla plays at the preparation screen doesn't play here.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
