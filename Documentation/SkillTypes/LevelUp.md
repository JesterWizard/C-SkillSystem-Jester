# Level-up skills (growths, EXP)

> Text file: `Contents/Texts/Source/texts/Skills_Lvup.txt`  ·  Code: `Kernel/Wizardry/Lvup/Source/` (growths, level-up rolls), `Kernel/Wizardry/BattleSys/Source/BattleExp.c` (EXP)  ·  Hook: no `POIN` list. Direct calls to `GetUnitCommonGrowthBonus` / `GetUnit<Stat>GrowthBonus` (`GrowthGetter.c`) and `KernelModifyBattleUnitExp` (`BattleExp.c:108`)

## What it is

Skills that change three things: the growth rates used for level-up and autolevel rolls (`GetUnit<Stat>Growth`), the EXP a battle awards (`KernelModifyBattleUnitExp`), and the level-up roll itself (`UnitLvupCore`, which uses the growths). Neither path goes through a getter list: the engine calls these kernel functions directly, so an effect must go in the function that computes the value.

## How the engine calls it

Growths:

1. Level-up: `CheckBattleUnitLevelUp` (`Kernel/Wizardry/Lvup/Source/Levelup.c:398`) is the replacement for the vanilla routine, redirected from the call site at `Kernel/Wizardry/Lvup/Source/Levelup.lyn.event:2-5`. It applies the level gain and then calls `UnitLvupCore` unless `KernelLvupReplacesGrowths()` is true (`Levelup.c:436`, `include/kernel/kernel-lib.h:235`).
2. `UnitLvupCore` (`Levelup.c:341`) picks one of `UnitLvup_Vanilla` (`:83`), `UnitLvup_RandC` (`:261`), `UnitLvup_Fixed` (`:290`), `UnitLvup_100` (`:324`) or `UnitLvup_0` (`:339`) from the `lvup_mode_*` config.
3. Each mode reads growths with `GetUnit<Stat>Growth(unit)`, e.g. `Levelup.c:116` (`UnitLvup_Vanilla`), `:265` (`UnitLvup_RandC`), `:299` (`UnitLvup_Fixed`).
4. `GetUnit<Stat>Growth` (`Kernel/Wizardry/Lvup/Source/GrowthGetter.c:229-236`) is `GetUnit<Stat>GrowthBonus(GetUnitBase<Stat>Growth(unit), unit)`. The job-based variants `GetUnitJobBased<Stat>Growth` (`:239-246`) use the same bonus function with class growths; their call sites were not traced.
5. `GetUnit<Stat>GrowthBonus` (e.g. `GetUnitPowGrowthBonus`, `GrowthGetter.c:79`) first calls `GetUnitCommonGrowthBonus` (`:11`), which holds skills that apply to every stat: `SID_Blossom` (`:15-18`), `SID_Aptitude` (`:20-23`), `SID_PeoplesKnight` (`:25-40`), then `SID_NecroCopy` last (`:50-53`, "This must come last"). It then adds the per-stat blocks (`SID_DarkHorse`, `SID_Knack<Stat>`).
6. `UnitAutolevelRealistic` (`Kernel/Wizardry/Lvup/Source/AutoLevel.c:16-24`) uses the same `GetUnit<Stat>Growth` functions, so growth skills also affect autolevel.

EXP:

1. `BattleApplyExpGains` (`Kernel/Wizardry/BattleSys/Source/BattleExp.c:417`, `LYN_REPLACE_CHECK`, so it replaces the vanilla function) calls `ApplyBattleUnitExpGain` (`:301`), which calls `GetBattleUnitExpGainRework` (`:253`, call at `:306`).
2. `GetBattleUnitExpGainRework` calls `KernelModifyBattleUnitExp(result, actor, target)` (`:257`), then `ModifyUnitSpecialExp` (`:234`, `:258`).
3. Staff EXP: `GetBattleUnitStaffExpRework` (`:344`) calls `KernelModifyBattleUnitExp` (`:353`) and applies its own cap (Prodigy-aware, `:360-368`).
4. Misc +10 EXP: `BattleApplyMiscActionExpGains` (`:317`) calls `KernelModifyBattleUnitExp(10, ...)` (`:332`).
5. `KernelModifyBattleUnitExp` (`:108`) holds all battle EXP skill blocks, in order: Blossom (`:112-115`), Paragon (`:117-120`), Mentorship (`:122-150`), StaffParagon (`:152-170`), Prodigy cap (`:172-183`), Pulse (`:185-189`), Perseverance (`:191-195`), Carnage (`:197-200`), Crisis (`:202-205`), minimum 1 (`:208-209`), VoidCurse (`:212-215`).
6. Battle checks use `BattleFastSkillTester(bu, SID_x)` (defined in assembly, `Kernel/Wizardry/SkillSys/kernel/SkillList_.lyn.event:3`), which only sees `gBattleActor` and `gBattleTarget`.

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/Lvup/Source/GrowthGetter.c` | `GetUnitCommonGrowthBonus`, `GetUnit<Stat>GrowthBonus` (Blossom, Aptitude, PeoplesKnight, NecroCopy, DarkHorse, Knack*) |
| `Kernel/Wizardry/Lvup/Source/Levelup.c` | `CheckBattleUnitLevelUp`, `UnitLvupCore`, roll modes `UnitLvup_*` |
| `Kernel/Wizardry/Lvup/Source/Levelup.lyn.event` | Tracked build output of `Levelup.c` (make rule `makefile:187-193`), but its first block (`:2-6`, call-site redirect to `CheckBattleUnitLevelUp`) is not in the `.c`. Do not regenerate without checking that block survives (unverified) |
| `Kernel/Wizardry/Lvup/Source/AutoLevel.c` | `UnitAutolevelRealistic` (growth consumers) |
| `Kernel/Wizardry/BattleSys/Source/BattleExp.c` | `KernelModifyBattleUnitExp` and the EXP entry points (`BattleApplyExpGains`, `GetBattleUnitStaffExpRework`, `BattleApplyMiscActionExpGains`) |
| `Kernel/Wizardry/SkillSys/kernel/SkillList_.lyn.event` | `BattleFastSkillTester` (`:3`) |
| `Data/SkillSys/Source/SkillExtraInfo.c` | `SKILL_EFF0` values (Aptitude `:754-755`, Knack* `:1455-1456`, PeoplesKnight `:1491-1492`) |
| `include/kernel/lvup.h` | Growth prototypes (`GetUnitHpGrowth` `:15`, `GetUnitHpGrowthJobBonus` `:79`) |
| `include/kernel/kernel-lib.h` | `KernelLvupReplacesGrowths()` (`:235`) |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the function. Growth effect: `GrowthGetter.c`. If it applies to all stats, add the block to `GetUnitCommonGrowthBonus` before the NecroCopy block. If it applies to one stat, add it to that `GetUnit<Stat>GrowthBonus`. EXP effect: `KernelModifyBattleUnitExp` (`BattleExp.c:108`).
3. Use `SkillTester(unit, SID_x)` in growth code (the unit is a `struct Unit`). Use `BattleFastSkillTester(actor, SID_x)` or `(target, SID_x)` in `KernelModifyBattleUnitExp`.
4. Put the EXP block before the `LIMIT_AREA` clamp at `BattleExp.c:179-182` if the clamp should apply to it. Blocks after the clamp are not clamped (Pulse, Perseverance, Carnage, Crisis, VoidCurse; see Gotchas).
5. If the bonus uses `SKILL_EFF0`, add the entry to `Data/SkillSys/Source/SkillExtraInfo.c`.
6. Do not edit `UnitLvup_*` for a per-skill change. They already read the growth getters.

## Template

Growth. Adapted from `Aptitude` (`Kernel/Wizardry/Lvup/Source/GrowthGetter.c:20-23`). The block goes in `GetUnitCommonGrowthBonus()`, before the NecroCopy block. The `SkillExtraInfo.c` entry shape is taken from `Data/SkillSys/Source/SkillExtraInfo.c:754-755`.

```c
/* Kernel/Wizardry/Lvup/Source/GrowthGetter.c, inside GetUnitCommonGrowthBonus() */
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	if (SkillTester(unit, SID_Example))
		new = new + SKILL_EFF0(SID_Example);
#endif
```

```c
/* Data/SkillSys/Source/SkillExtraInfo.c */
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = {{ 20 }},
#endif
```

EXP. Adapted from `Paragon` (`Kernel/Wizardry/BattleSys/Source/BattleExp.c:117-120`), with `SKILL_EFF0` in place of the fixed x2. The `SkillExtraInfo.c` entry above is then also required.

```c
/* Kernel/Wizardry/BattleSys/Source/BattleExp.c, inside KernelModifyBattleUnitExp() */
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
    if (BattleFastSkillTester(actor, SID_Example))
        status = status + SKILL_EFF0(SID_Example);
#endif
```

No registration line is needed: both functions are already called by the engine.

## Worked examples

- `Aptitude`: `Kernel/Wizardry/Lvup/Source/GrowthGetter.c:20-23`. Adds `SKILL_EFF0` growth points to every stat through the common function.
- `Blossom`: growth `Kernel/Wizardry/Lvup/Source/GrowthGetter.c:15-18` (`new + status`, doubles growth) and EXP `Kernel/Wizardry/BattleSys/Source/BattleExp.c:112-115` (halves EXP). A two-sided skill; both halves are needed for the text to be true.
- `DarkHorse`: `Kernel/Wizardry/Lvup/Source/GrowthGetter.c:63-66` (HP growth + half max HP), `:85-88` (POW + POW), `:124-127` (SKL + SKL). Shows a per-stat growth block that reads another stat of the unit.
- `Mentorship`: `Kernel/Wizardry/BattleSys/Source/BattleExp.c:122-150`. Own-EXP branch plus adjacent-ally scan over `gVecs_2x2` (`Kernel/Wizardry/KernelLib/Source/ranges.c:10`, 12 tiles within distance 2). See Gotchas for its problems.

## Gotchas

- Growth and EXP are separate code paths. A skill with both effects (Blossom) needs both blocks.
- `Mentorship` has no `SkillExtraInfo.c` entry (`Data/SkillSys/Source/SkillExtraInfo.c` has no `SID_Mentorship`), so `SKILL_EFF0(SID_Mentorship)` reads 0 (`BattleExp.c:124`, `:145`). The adjacent-ally branch then computes `status * Div(status * 0, 100)` = 0 (`:145`), which the minimum-1 rule (`:208-209`) turns into 1 EXP. The own-EXP branch adds 0. The text says "1.2x", so the entry is missing (inference from the code; not run in game).
- `Mentorship`'s ally branch uses `status = status * Div(status * SKILL_EFF0(...), 100)` (`:145`). This is quadratic in `status`; the own-EXP branch (`:124`) uses `status + Div(status * SKILL_EFF0(...), 100)`. Likely the ally branch should be the second form. Intent not verified.
- `Mentorship`'s own-EXP branch (`:124`) also boosts the holder's own EXP, which the text does not mention.
- `VoidCurse` (`BattleExp.c:212-215`) sets EXP to 0 whenever the skill holder is the battle target. There is no death check. The text says "when defeated".
- `DarkHorse` on RES growth adds `unit->def`, not `unit->res` (`GrowthGetter.c:212-215`, the `status + unit->def` at `:214`). Likely bug; intent not verified.
- Prodigy's cap is in two places: `KernelModifyBattleUnitExp` (`BattleExp.c:172-183`) and `GetBattleUnitStaffExpRework` (`:360-368`). Change both.
- `KernelModifyBattleUnitExp` runs for staff EXP and misc +10 EXP too (`:332`, `:353`). An EXP skill added there applies to all three.
- Blocks after the `LIMIT_AREA(status, 1, 100)` clamp (`BattleExp.c:179-182`) can push EXP above 100 (Pulse's floor is also there, `:185-189`). Prodigy's higher cap (254) is set at `:175`.
- `SID_PeoplesKnight` text says "+3% per support level". The code counts each support partner with support value at least 80, 160 or 240 and adds 1x, 2x or 3x `SKILL_EFF0` (`GrowthGetter.c:25-40`). The text is not an exact description.
- `Aptitude` text says "+20% to all growth rates". The code adds `SKILL_EFF0` (20) points to each growth value (`GrowthGetter.c:22`).
- When `KernelLvupReplacesGrowths()` is true (stat-points or stat-timing config), `CheckBattleUnitLevelUp` skips `UnitLvupCore` (`Levelup.c:436`), and the stat-points menu is started from `Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/BEXP.c:372-373`. How that menu reads growths was not traced (`StartLvupStatPointsMenu` definition not located).
