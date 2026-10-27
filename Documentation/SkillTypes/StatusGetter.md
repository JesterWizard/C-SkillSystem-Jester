# Stat-getter skills

> Text file: `Contents/Texts/Source/texts/Skills_StatusGetter.txt`  ·  Code: `Kernel/Wizardry/UnitStatusGetter/source/` (one `<Stat>Getter.c` per stat), caps in `Kernel/Wizardry/UnitStatusGetter/UnitMaxStatus/`  ·  Hook: `g<Stat>Getters` lists in `Kernel/Wizardry/UnitStatusGetter/data.event`, skill block in `<Stat>GetterSkills()`

## What it is

Passive skills that add a flat or conditional bonus to one unit's HP, POW, MAG, SKL, SPD, LCK, DEF, RES, MOV or CON. The bonus is applied inside the stat getter, so it is seen everywhere the game reads that stat (stat screen, item-use screen, battle). Max-stat caps are a separate mechanism: a getter bonus does not raise the cap (see Gotchas).

## How the engine calls it

1. The vanilla stat readers are redirected to kernel functions: `Kernel/Wizardry/UnitStatusGetter/UnitStatusGetter.event:30-37` (for example `_thumb(PowGetter)` / `jumpToHack(_GetUnitPower)`). C code calls the public name, e.g. `PowGetter(unit)` at `Kernel/Wizardry/ItemSys/PrepItem/PrepItemUseScreen.c:135`.
2. `_GetUnitPower` (`Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:11`) starts from `unit->pow`. If the other battle unit has `SID_Unaware` it returns that raw value (`PowGetter.c:16-21`). Otherwise it applies `PairUp_RescueStatScale` (`:23`), walks `gpPowGetters` (`:25-26`), then `gpExternalPowGetters` (`:28-29`), then adds `PowTonic` and `OmniTonic` (`:31-32`).
3. `gPowGetters` (`Kernel/Wizardry/UnitStatusGetter/data.event:10-15`) is a zero-terminated list of `StatusGetterFunc_t` (`include/kernel/status-getter.h:6`). Entries in order: `PowGetterWeaponBonus`, `PowGetterSkills`, `PowPsychUpCheck`, `PowGetterDebuff`, `PowGetterStaffBoost`, `PowGetterStatDebuff`, `PowGetterShield`, `PowGetterStartMapEffects`, `StatusGetterCheckCpas`. Each list ends with `MSG_END` (`data.event:1`), which pads with `KernelPad2` (identity function, `Kernel/Wizardry/KernelLib/Source/pad.c:6-9`) and a 0 terminator. The other lists (`gHpGetters` `data.event:4`, `gMagGetters` `:17`, ..., `gConGetters` `:67`, `gAidGetters` `:72`) follow the same pattern.
4. `PowGetterSkills` (`PowGetter.c:52`) contains one `#if defined(SID_x) && (COMMON_SKILL_VALID(SID_x))` block per skill: `SkillTester(unit, SID_x)` (or `SkillTesterPlus` for upgrade-chain skills, `PowGetter.c:84`), then `status += SKILL_EFF0(SID_x)`. `SKILL_EFF0(sid)` is `gpSkillExtraInfo[sid].priv[0]` (`include/kernel/skill-system.h:383`); values come from `Data/SkillSys/Source/SkillExtraInfo.c`.
5. `SkillTester` is a Thumb trampoline into an ARM routine (`Kernel/Wizardry/SkillSys/kernel/JudgeSkill_NORMAL.lyn.event:3-4`). Battle-only `BattleFastSkillTester` is not used by getters (see Gotchas).
6. `PowPsychUpCheck` (`PowGetter.c:256`) re-runs `PowGetterWeaponBonus` and `PowGetterSkills` on the other battle unit and adds its stat debuff. The same pattern exists in `MagPsychUpCheck`, `SklPsychUpCheck`, `SpdPsychUpCheck`, `LckPsychUpCheck`, `DefPsychUpCheck`, `ResPsychUpCheck`.

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/UnitStatusGetter/UnitStatusGetter.event` | Redirects vanilla stat functions to `_GetUnit*`; includes the `source/*.lyn.event` files |
| `Kernel/Wizardry/UnitStatusGetter/data.event` | `gHpGetters`, `gPowGetters` ... `gConGetters`, `gAidGetters` lists |
| `Kernel/Wizardry/UnitStatusGetter/source/HpGetter.c` | `_GetUnitMaxHp`, `HpGetterSkills` (`SID_HpBonus`, `SID_Overdraft`) |
| `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c` | `_GetUnitPower`, `PowGetterSkills`, `PowPsychUpCheck` |
| `Kernel/Wizardry/UnitStatusGetter/source/MagGetter.c`, `SklGetter.c`, `SpdGetter.c`, `LckGetter.c`, `DefGetter.c`, `ResGetter.c` | `_GetUnitMagic`, `_GetUnitSkill`, `_GetUnitSpeed`, `_GetUnitLuck`, `_GetUnitDefense`, `_GetUnitResistance` and their `<Stat>GetterSkills` |
| `Kernel/Wizardry/UnitStatusGetter/source/MovGetter.c` | `_GetUnitMov`, `MovGetterSkills` |
| `Kernel/Wizardry/UnitStatusGetter/source/ConGetter.c` | `_GetUnitCon`, `ConGetterSkills` |
| `Kernel/Wizardry/UnitStatusGetter/source/AidGetter.c` | `_GetUnitAid`; no skill function (`gAidGetters` has only `StatusGetterCheckCpas`) |
| `include/kernel/status-getter.h` | `StatusGetterFunc_t`, `g*Getters` / `gpExternal*Getters` externs, `struct UnitMaxStatusConfig` (`:58-62`), `GetUnitMaxStatus*` prototypes (`:66-75`) |
| `Kernel/Wizardry/UnitStatusGetter/UnitMaxStatus/UnitMaxStatus.c` | `GetUnitMaxStatus<Stat>()`: base max plus per-character table entry, clamped to `UNIT_STATUS_MAX` (120, `:6`) |
| `Data/MaxStatus/MaxStatus.c` | `gUnitMaxStatusConfigTable[0x100]`, per-character max offsets (included from `Data/Data.event:20`; exported as `gpUnitMaxStatusConfigTable`, `Preload/Reloc.event:132-133`) |
| `Kernel/Wizardry/Lvup/Source/CheckCap.c` | Stat caps: `CheckBattleUnitStatCapsVanilla` (`:12`, LimitBreaker, OgreBody, Dismount), `UnitCheckStatCapsVanilla` (`:93`), `_UnitCheckStatCaps` (`:201`) |
| `Kernel/Wizardry/Lvup/Source/Promotion.c` | Promotion caps via `GetUnitMaxStatus*` (`:31-58`) |
| `Data/SkillSys/Source/SkillExtraInfo.c` | `SKILL_EFF0` values per skill |
| `Kernel/Wizardry/UnitStatusGetter/source/*.lyn.event` | Tracked build output of the matching `.c` (make rule `makefile:187-193`). Edit the `.c`. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Choose the stat(s) the skill changes and open that `Kernel/Wizardry/UnitStatusGetter/source/<Stat>Getter.c` (table above). Put the block inside the existing `<Stat>GetterSkills()` function. A skill that affects several stats needs a block in each getter; there is no shared hook (`Fury` is repeated in `PowGetter.c:78-80`, `DefGetter.c:91-93`, `LckGetter.c:67-69`).
3. Wrap each block in `#if defined(SID_<Name>) && (COMMON_SKILL_VALID(SID_<Name>))`. The file already includes `skill-system.h` and `constants/skills.h`.
4. If the bonus uses `SKILL_EFF0`, add `[SID_<Name>] = {{ n }},` to `Data/SkillSys/Source/SkillExtraInfo.c`, inside a guard of the same form as the existing entries (`:6-7`, `:10-11`).
5. If the skill changes a max stat, edit `CheckCap.c` and/or `UnitMaxStatus.c` as described under Gotchas. Do not put cap logic in the getter.
6. Adding a new getter function to a list (not a skill) is a kernel change: insert a `POIN` before `MSG_END` in the group in `data.event`.

## Template

Adapted from `StrBonus` in `PowGetterSkills` (`Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:67-70`) and its `SkillExtraInfo.c` entry (`Data/SkillSys/Source/SkillExtraInfo.c:10-11`).

```c
/* Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c, inside PowGetterSkills() */
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	if (SkillTester(unit, SID_Example))
		status += SKILL_EFF0(SID_Example);
#endif
```

```c
/* Data/SkillSys/Source/SkillExtraInfo.c */
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = {{ 5 }},
#endif
```

No other registration is needed: `PowGetterSkills` is already listed in `gPowGetters` (`Kernel/Wizardry/UnitStatusGetter/data.event:11`). For an HP-conditional bonus, copy the shape of `DefiantStr` (`PowGetter.c:72-76`, `cur_hp * 4 < max_hp`).

## Worked examples

- `StrBonus`: `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:67-70`. Flat `+SKILL_EFF0` bonus; the template source.
- `DefiantStr`: `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:72-76`. Conditional on HP below 25%.
- `HeavyMetal`: `Kernel/Wizardry/UnitStatusGetter/source/ConGetter.c:24-27`. The only CON skill hook; value in `SkillExtraInfo.c:1531-1532`.
- `Overdraft`: `Kernel/Wizardry/UnitStatusGetter/source/HpGetter.c:61-64`. Adds the raw `unit->maxHP` to the running HP, which doubles max HP. Uses the base value, not the running `status`.

## Gotchas

- Edit the `.c`, not the `.lyn.event`. The `.lyn.event` files are tracked in git and rebuilt from the `.c` by the make rule at `makefile:187-193` when the `.c` is newer. Check the `.lyn.event` after a build, since some `.lyn.event` files also hold hand-written patches (see `Kernel/Wizardry/Lvup/Source/Levelup.lyn.event:2-6` in the level-up doc).
- Order within the list matters for skills that read the running `status`: `Resolve` uses `status / 2` of the value so far (`PowGetter.c:176-180`); `BonusDoubler` adds `status - originalStatus`, i.e. only what `PowGetterSkills` itself added (`:238-240`); `SID_SlowStart` multiplies `status` (`:243-245`). The order inside `gPowGetters` is shown above.
- `SKILL_EFF0(sid)` reads whatever the `SkillExtraInfo.c` table holds. A skill with no entry gets 0. Check the entry exists whenever the code uses `SKILL_EFF0`.
- Getters run outside battle too (e.g. `PrepItemUseScreen.c:135`). Use `SkillTester` in a getter, not `BattleFastSkillTester` (which is only valid during the pre-battle calculation; see the common doc).
- Psych Up copies the other battle unit's `PowGetterWeaponBonus` + `PowGetterSkills` output (`PowGetter.c:256-279`). A new block in `*GetterSkills()` is therefore copied by Psych Up for that stat automatically (verified from the code path; not tested in game).
- Unaware on the other battle unit makes the getter return the raw stat with no skill or weapon bonus (`PowGetter.c:16-21`).
- Max-stat caps are not read from the getter at level-up. `UnitCheckStatCapsVanilla` (`Kernel/Wizardry/Lvup/Source/CheckCap.c:93`) caps POW, SKL, SPD, DEF, RES, LCK and MOV with the class maximum (`UNIT_*_MAX`). The only `GetUnitMaxStatus*` call in the cap code is `GetUnitMaxStatusMag` (`CheckCap.c:192-193`, `:219-220`). The other `GetUnitMaxStatus*` functions are read at promotion only (`Promotion.c:36-55`, applied only when `bonusCaps == 0`, `:31`). A skill that raises a non-MAG cap must therefore change `CheckCap.c` or `Promotion.c` too (unverified beyond this reading).
- `SID_LimitBreaker` / `SID_LimitBreakerPlus` add `SKILL_EFF0` to every cap: `CheckCap.c:24-31`, `:110-117`, `:208-215`, and to the promotion bonus (`Promotion.c:64-75`).
- `SID_OgreBody` sets max HP to `SKILL_EFF0(SID_OgreBody)` (`CheckCap.c:34-38`). Its copy in `UnitCheckStatCapsVanilla` is commented out (`:121-125`).
- Likely bug: `GetUnitMaxStatusSkl` adds `gpUnitMaxStatusConfigTable[pid].mag` (`UnitMaxStatus.c:43`), not `.skl` (the struct has a separate `skl` field, `include/kernel/status-getter.h:59`). Affects SKL caps read at promotion (`Promotion.c:45-47`). Intent not verified.
- Fury-style all-stat skills are repeated per getter; forgetting one getter makes the bonus stat-specific.
- The kernel comment says CON getters are not injected into most vanilla functions (`ConGetter.c:7-9`). Where CON changes are visible was not verified.
- `gpExternalHpGetters` ... `gpExternalAidGetters` (`status-getter.h:20-30`) are read by the kernel (e.g. `PowGetter.c:28`). Their definition is not in this repo (unverified).
