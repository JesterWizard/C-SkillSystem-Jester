# Skill Dry-Run: Biorhythm

This dry-run validates the extraction skill against a second feature **without implementing it**.

## Feature trace

- Integrated source: `Kernel/Wizardry/Misc/Biorhythm/Biorhythm.c`
- Installer: `Kernel/Wizardry/Misc/Biorhythm/Biorhythm_Installer.event`
- Generated body: `Kernel/Wizardry/Misc/Biorhythm/Biorhythm.lyn.event`
- Runtime entry: `GetBiorhythmBonus()` called from `Kernel/Wizardry/Core/BattleSys/Source/PreBattleCalc.c`
- Data: `gBiorhythmPInfoConfigList[0x100]`

## Dependency closure

| Dependency | Standalone handling |
|---|---|
| `GetBiorhythmBonus` | Ship generated `.lyn.event` or recompile C into package `Generated/` |
| `gBiorhythmPInfoConfigList` | Include table in installer/data event |
| `PreBattleCalc` hook chain | **Do not** import full battle system; hook vanilla pre-battle calc leaf or inject one private callback |
| `gPlaySt.chapterTurnNumber` | Vanilla symbol; safe |
| `UNIT_CHAR_ID`, `BattleUnit` | Vanilla structs; safe |
| Skill System / designer config | Not required |

## Recommended standalone strategy

1. Identify the vanilla pre-battle stat calculation hook used by `PreBattleCalc.c` or the smallest FE8U function that applies temporary battle bonuses before combat.
2. Install a private dispatcher at that leaf hook instead of importing `PreBattleCalc.lyn.event`.
3. Place `GetBiorhythmBonus` and `gBiorhythmPInfoConfigList` together in free space starting at `$1000000` plus the size of any already-installed standalone patches.
4. Ship checked-in `Generated/Biorhythm.lyn.event` so FEBuilder users do not compile C.

## Planned package layout

```text
Standalone/biorhythm/
  Installer.event
  Source/Biorhythm.c
  Source/Biorhythm.lyn.event
  makefile
  README.md
```

## Hook / conflict notes

- Conflicts with any patch replacing the same pre-battle bonus hook.
- Does **not** conflict with `Standalone/two_random_number_growths/` if hook sites differ.
- No save/RAM allocation expected.

## Skill gate result

Proceed without user approval: **yes**, once the exact vanilla pre-battle hook address is confirmed from clean FE8U.

Blockers found by dry-run:

- Must confirm the precise vanilla hook rather than copying the integrated `PreBattleCalc` registry wholesale.
- Table size (`0x100` entries) requires explicit free-space sizing in README.
