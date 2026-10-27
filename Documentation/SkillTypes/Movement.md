# Movement skills

> Text file: `Contents/Texts/Source/texts/Skills_Movement.txt`  ·  Code: `Kernel/Wizardry/Movement/`  ·  Hook: `SetWorkingMoveCosts()` (`Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:579`, terrain costs) and `PreGenerateMovementMap()` (`Kernel/Wizardry/Movement/Source/Movement.c:11`, barrier and pioneer maps)

## What it is

Movement skills change the move-cost table used by the flood fill (terrain costs, e.g. Acrobat, Water Walking, Phase Shift), let a unit ignore or project barriers (Pass, Obstruct, Detailed Report), and add extra movement to allies (Guidance family, Soaring Wings, Flier Formation, Aerobatics). They run inside every movement-map build: player move select, path arrows, danger zone, and AI reach. Grid Master and Warpath are listed in the same text file but are canto/attack-command gates, not movement-map code (see Where to look).

## How the engine calls it

1. Vanilla `GenerateUnitMovementMap` is redirected by `ORG $1a38c` in `Kernel/Wizardry/WeaponRange/Source/MiscHooks.lyn.event:20` to `GenerateUnitMovementMap` at `Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:278`.
2. That function calls `SetWorkingMoveCosts(GetUnitMovementCost(unit))` (`MiscHooks.c:280`), then `GenerateMovementMap(unit->xPos, unit->yPos, MovGetter(unit), unit->index)` (`MiscHooks.c:283`). `AiFloodMovementAndRange` (`MiscHooks.c:167`) calls the same two functions. `DisplayUnitEffectRange` (`:210`) and `GenerateDangerZoneRange` (`:445`) call vanilla `GenerateUnitMovementMapExt` (not read), which is outside this doc's path.
3. `GetUnitMovementCost` (`Kernel/Wizardry/Movement/Source/MoveCost.c:16`, redirected by `ORG $18d4c` in `Kernel/Wizardry/Movement/Source/LynJump.event:8`) returns the class cost table: ballista table if `US_IN_BALLISTA` (`MoveCost.c:18-19`); otherwise row 1 for rain, row 2 for snow/snowstorm (`MoveCost.c:25-36`). Weatherman forces row 0 when weather is active (`MoveCost.c:40-41`).
4. `SetWorkingMoveCosts(const s8 mct[])` (`MiscHooks.c:579`, redirected by `ORG $1a4cc` in `MiscHooks.lyn.event:26`) copies `mct` into `gWorkingTerrainMoveCosts` (`MiscHooks.c:586-587`), then applies each terrain skill block in source order (Seafarer `:593` through Desert Tiger `:695`). Two helpers: `APPLY_COST_IF_LOWER` (`MiscHooks.c:533-537`, only lowers) and direct rewrite (Acrobat, `:648-657`, sets any cost > 1 to 1).
5. `GenerateMovementMap(int x, int y, int movement, int uid)` (`Kernel/Wizardry/Movement/Source/Movement.c:257`, redirected by `ORG $1a4ec` in `Movement/Source/LynJump.event:2`) resets the flood state, then calls `PreGenerateMovementMap(movement)` (`Movement.c:289`, inside `#if CHAX`; `CHAX` is defined unconditionally in `include/common-chax.h:12`).
6. `PreGenerateMovementMap` (`Movement.c:11`) sets `KernelMoveMapFlags = 0` (`:37`), returns early if there is no unit (`:40-41`), computes flier status and the self-skill flags once (`:45-65`), then loops over every map tile. Per tile it fills `KernelExtMoveBarrierMap` from enemy Obstruct/Detailed Report (`:81-135`) and `KernelExtMovePioneerMap` from allied skills (`:139-226`). Boulder and spin-tile traps add barriers (`:233-250`).
7. `CallARM_FillMovementMap()` (`Movement.c:293`) runs the ARM flood fill (`_MapFloodCoreRe`, `Kernel/Wizardry/Movement/Source/MapFloodArm.lyn.event:2`), which is installed by `GameInit_RedirectMapFloodFunc` (`Movement.c:298`, listed in `Kernel/Wizardry/GameInitHook/data.event:9`). The flood's use of `gWorkingTerrainMoveCosts` and the two ext maps is unverified (ARM code not read).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/WeaponRange/Source/MiscHooks.c` | `SetWorkingMoveCosts` (`:579`): all terrain-cost skills (Seafarer `:593`, Hiker `:603`, Ruinator `:633`, Phase Shift `:640`, Acrobat `:648`, Water Walking `:669` / `+` `:659`, Mountain Climber `:687` / `+` `:679`, Desert Tiger `:695`). `GenerateUnitMovementMap` (`:278`) calls it. |
| `Kernel/Wizardry/WeaponRange/Source/MiscHooks.lyn.event` | `ORG $1a4cc` (SetWorkingMoveCosts) and `ORG $1a38c` (GenerateUnitMovementMap) hook sites. |
| `Kernel/Wizardry/Movement/Source/Movement.c` | `PreGenerateMovementMap` (`:11`): Pass (`:51-53`), Obstruct (`:90-98`), Detailed Report (`:101-110`), pioneer skills (`:139-200`), trap barriers (`:233-250`). `GenerateMovementMap` (`:257`). |
| `Kernel/Wizardry/Movement/Source/MoveCost.c` | `GetUnitMovementCost` (`:16`): weather row selection and Weatherman (`:40`). |
| `Kernel/Wizardry/Movement/Source/LynJump.event` | Hook sites for `GenerateMovementMap` (`ORG $1a4ec`) and `GetUnitMovementCost` (`ORG $18d4c`). |
| `Kernel/Wizardry/Movement/Source/Movement.lyn.event` | lyn label and `PROTECT` for `GenerateMovementMap`. |
| `Kernel/Wizardry/Movement/Source/MapFloodArm.lyn.event` | ARM flood core `_MapFloodCoreRe` (not read in detail). |
| `Kernel/Wizardry/Movement/Movement.event` | Includes the movement sources above. `Kernel/Wizardry/Movement/MovementData.event` is commented out in that file. |
| `Kernel/Wizardry/Movement/MovePathFix/MovePathFix.event` | Path-arrow fix (not a skill). Its header comment says move-path supports at most 20 steps. |
| `include/kernel/map-movement.h` | `KernelMoveMapFlags`, `FMOVSTRE_PASS`, `FMOVSTRE_BARRIER`, `FMOVSTRE_PIONEER`. |
| `Kernel/Wizardry/ClassTypes/Source/class-types.c:44` | `CheckClassFlier`, used to pick flier vs non-flier skill branches. |
| `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c:2926` | `AttackCommandUsability`: SID_GridMasterPlus (`:2938`) and SID_Warpath (`:2943`) gates. Not movement-map code. |
| `Kernel/Wizardry/SkillEffects/PostActionSkills/Canto.c:95` | `TryMakeCantoUnit`: Grid Master / Grid Master+ canto after a menu movement skill (`:102`, `:113`). |
| `Kernel/Wizardry/SkillSys/kernel/SkillList.c:32` | `gSkillUpgradePlusLookup`: Plus-to-base map used by `SkillTesterPlus` (`Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c:3478`). |
| `Data/SkillSys/Source/SkillExtraInfo.c:922-959` | SKILL_EFF values for the movement skills (Guidance `{{ 2 }}`, Flier Guidance `{{ 3 }}`, Obstruct `{{ 25, 1 }}`, Detailed Report `{{ 50, 2, 2 }}`). |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Choose the hook by effect:
   - Terrain cost for the unit: add a block to `SetWorkingMoveCosts` (`MiscHooks.c:579`), before `#undef APPLY_COST_IF_LOWER`. Use `APPLY_COST_IF_LOWER` unless you need the Acrobat-style rewrite.
   - Barrier, pass, or ally pioneer bonus: edit `PreGenerateMovementMap` (`Movement.c:11`). Read the holder's state once before the tile loop (as `Aerobatics_activated` etc. do at `:60-65`), and only test per-tile units inside the loop.
   - Movement-table row (weather-like): edit `GetUnitMovementCost` (`MoveCost.c:16`).
3. Use `SkillTester(unit, sid)` (see Gotchas, hot path). Use `SkillTesterPlus` only for `...Plus` variants, which also match the base skill when the unit has `SID_Upgrade`.
4. If the skill reads `SKILL_EFF0` (or `SKILL_EFF1`, ...), add `[SID_Example] = {{ ... }},` to `Data/SkillSys/Source/SkillExtraInfo.c` inside the same `#if` guard pattern.
5. Build and test in game. Not run here.

## Template

Adapted from Desert Tiger (`Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:695-699`, the smallest terrain-cost block). Insert inside `SetWorkingMoveCosts`, before `#undef APPLY_COST_IF_LOWER`. No `.event` change is needed because the function is already hooked (`MiscHooks.lyn.event:26`). `MiscHooks.c` already includes `skill-system.h` and `constants/skills.h`.

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
    if (SkillTester(gActiveUnit, SID_Example))
    {
        APPLY_COST_IF_LOWER(TERRAIN_SAND, 1);
    }
#endif
```

For a Plus variant: `SkillTesterPlus(gActiveUnit, SID_ExamplePlus)` (see `MiscHooks.c:659-660` for WaterWalkingPlus).

## Worked examples

- `Acrobat` (`Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:648-657`): rewrites every cost above 1 to 1. Order-dependent: blocks after it see its result.
- `Pass` (`Kernel/Wizardry/Movement/Source/Movement.c:51-53`): sets `FMOVSTRE_PASS` when HP is at least 25%, which skips the barrier check at `:89`.
- `Obstruct` (`Movement.c:90-98`): foe skill; HP gate from `SKILL_EFF0` (percent), barrier radius from `SKILL_EFF1`.
- `Guidance` (`Movement.c:187-193`): ally skill; marks a pioneer radius of `SKILL_EFF0` around the holder.

## Gotchas

- Hot path. `SetWorkingMoveCosts` and `PreGenerateMovementMap` run for every movement-map build: player move select, danger zone, and AI reach (`Documentation/CONTRIBUTING.md` lists movement generation such as `GenerateUnitCompleteAttackRange` as performance-sensitive). Existing code uses `SkillTester` on the acting unit (`MiscHooks.c:634`, `Movement.c:52`) and on each enemy on the map inside the tile loop (`Movement.c:91`, `:102`). Follow that: `SkillTester` / `SkillTesterPlus`, hoisted out of loops where possible. Do not use `BattleFastSkillTester` here; it only works for `gBattleActor` / `gBattleTarget` during battle calc (Preamble fact 10).
- Text/config mismatch: Obstruct text says "If HP more than 50%", but the default `SKILL_EFF0` is 25 (`Data/SkillSys/Source/SkillExtraInfo.c:954-955`). The code uses the config value.
- Text/code mismatch: Ruinator text says "walk over Light Rune traps", but the code lowers the cost of terrain `TERRAIN_TILE_00` (`MiscHooks.c:633-637`). Light Rune is a trap type (`TRAP_LIGHT_RUNE`, `Kernel/Wizardry/FreeMovement/cFreeMU/src/_include/gbafe/trap.h:43`), and the trap loop in `PreGenerateMovementMap` only handles boulder and spin tiles (`Movement.c:233-250`). Unverified in game whether that is the intended tile.
- Seafarer and Hiker say "within 3 tiles". The code uses `gVecs_3x3` (`Kernel/Wizardry/KernelLib/Source/ranges.c:18-26`, 24 entries, Manhattan radius 3). The name `3x3` is misleading.
- Hiker is applied twice in `SetWorkingMoveCosts`: once from the cached ally list (`MiscHooks.c:603-609`) and again from a direct 3x3 loop (`:611-631`). The second loop is redundant.
- Flier-guidance guard mismatch: `Movement.c:151-155` is guarded by `SID_FlierGuidance` but reads `SKILL_EFF0(SID_FlierFormation)`. Both are 3 today; if they diverge, or one SID is removed, the bonus silently changes.
- Pass only bypasses the barrier check for the mover's own Pass (`Movement.c:51-53`, `:89`). Barrier and pioneer maps are written into `gGenericBuffer` and `gGenericBuffer + 0x800` (`Movement.c:70-75`). Callers must not hold those buffers during a movement-map build (the `WARN_GENERIC_BUF_USED` line is commented out, `Movement.c:288`).
- Movement text for Grid Master and Warpath is not movement-map code. They are gates in `TryMakeCantoUnit` and `AttackCommandUsability`.
- Weatherman is a movement skill that is not in `Skills_Movement.txt`. It lives in `GetUnitMovementCost` (`MoveCost.c:40-41`).
