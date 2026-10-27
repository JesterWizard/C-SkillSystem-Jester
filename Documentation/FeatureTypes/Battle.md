# Battle features (non-skill)

> Code: `Kernel/Wizardry/BattleSys/`, `Kernel/Wizardry/BanimHack/`, `Kernel/Wizardry/CritCutin/`, `Kernel/Wizardry/MapAnimBattleHook/`, `Kernel/Wizardry/BattleForcastFix/`, `Data/BattleSys/`  ·  Entry points: `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:4`), `BattleDamageCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:17`), `PreBattleGenerateFuncs` (`Kernel/Wizardry/BattleSys/data.event:27`), `Banim_C07` (`Kernel/Wizardry/BanimHack/Source/C07.c:118`), `gPreMapAnimBattleRoundHooks` (`Kernel/Wizardry/MapAnimBattleHook/MapAnimBattleHook.event:12`)  ·  Existing docs: `Documentation/Features/BattleSystem.md`, `Documentation/Features/CritCutin.md`, `Documentation/Features/BanimFeatures.md`, `Documentation/Features/GaidenMagic.md`

## What it covers

Features that change battle rules or battle presentation without being a skill: they have no SID, no skill text and no `SkillInfo` entry. Examples in this repo: `LeaderShip` (a hit and avoid bonus from nearby allies, a pre-battle rule), `WrankBonus` (weapon-rank stat bonuses read from a data table), `HpCost` (the gaiden-magic HP cost, a per-round rule with its own map-anim step) and `CritCutin` (a portrait cut-in before critical hits, presentation only). Skill effects on battle stats, strikes and order are in the skill docs linked below.

## How the engine calls it

1. `BattleGenerate` (`Kernel/Wizardry/BattleSys/Source/BattleGenerate.c:69`, replaces vanilla via `LYN_REPLACE_CHECK` at `:68`). Under `#if CHAX` (`:71-72`) it calls `PreBattleGenerateHook` (`:60-66`), which walks `gpPreBattleGenerateFuncs` once per battle calc.
2. `BattleGenerate` calls `ComputeBattleUnitStats(&gBattleActor, &gBattleTarget)` and then with the two units swapped (`BattleGenerate.c:75-76`).
3. `ComputeBattleUnitStats` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:2534-2535`) walks `gpPreBattleCalcFuncs` (`:2541-2545`). Each entry is `void (*)(struct BattleUnit *attacker, struct BattleUnit *defender)` (`PreBattleCalc.c:20`). The list is `PreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event:4-15`). Its last real entry is `PreBattleCalcEnd` (`:13`), which clamps several stats (attack, defense, speed, hit, avoid, crit, dodge) to zero (`PreBattleCalc.c:365-380`).
4. `ComputeBattleUnitEffectiveStats` (`BattleGenerate.c:78-79`), then `BattleUnwind` or `BattleUnwindScripted` (`:85-87`), then `ModifyBattleStatusForUI` (`:90`).
5. Per strike: `BattleGenerateHitAttributes` (`Kernel/Wizardry/BattleSys/Source/BattleHit.c:212-213`) calls `BattleHit_CalcDamage` (`BattleHit.c:257`). That walks `gpBattleDamageCalcFuncs` (`Kernel/Wizardry/BattleSys/Source/BattleDamage.c:106`, loop at `:186-187`). The damage flow is in [BattleSystem.md](../Features/BattleSystem.md).
6. Real damage: `CalcBattleRealDamage` (`BattleDamage.c:53`) walks `gpBattleRealDamageCalcFuncs` (`:84`). That list holds only `KernelPad2` placeholders (`Kernel/Wizardry/BattleSys/data.event:23-25`).
7. Forecast: `ModifyBattleStatusForUI` (`Kernel/Wizardry/BattleSys/Source/BattleUI.c:40-45`) runs only when `BATTLE_CONFIG_SIMULATE` is set. The forecast shows the stats that `BattleGenerate` computed, so a pre-battle rule appears in the forecast and a rule that rolls RN during a strike does not (see [BattleSystem.md](../Features/BattleSystem.md)).
8. Attack banim: the `C07` command is injected with `InjectBanimCmd(0x07, Banim_C07, pBanim_C07)` (`Kernel/Wizardry/BanimHack/BanimHack.event:8`). `Banim_C07` (`Kernel/Wizardry/BanimHack/Source/C07.c:118-141`) starts `ProcScr_BanimC07Handler` (`:83-113`) and holds the attacker's script (`:123`) until the handler reaches `C07Handler_End` (`:78-81`).
9. Per-round map anim: `PreMapAnimBattleRound_OnStart` (`Kernel/Wizardry/MapAnimBattleHook/Source/PreHit.c:11-28`) calls `KernelStartBlockingHookProc(gpPreMapAnimBattleRoundHooks, proc)` (`:27`). The list is `Kernel/Wizardry/MapAnimBattleHook/MapAnimBattleHook.event:12-16`. The battle map-anim script is replaced at ROM `0x07B8BC` (`MapAnimBattleHook.event:7-8`).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/BattleSys/data.event` | The battle lists: `PreBattleCalcFuncs` (`:4`), `BattleDamageCalcFuncs` (`:17`), `BattleRealDamageCalcFuncs` (`:23`), `PreBattleGenerateFuncs` (`:27`). |
| `Kernel/Preload/KernelReloc.event` (lines 9-10, 36, 42-43) | `gp*` pointers to the lists (`gpBattleDamageCalcFuncs`, `gpBattleRealDamageCalcFuncs`, `gpPreMapAnimBattleRoundHooks`, `gpPreBattleGenerateFuncs`, `gpPreBattleCalcFuncs`). |
| `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c` | `ComputeBattleUnitStats` walker (`:2534-2545`), `PreBattleCalcEnd` clamp (`:365-380`), built-in list entries. |
| `Kernel/Wizardry/BattleSys/Source/BattleGenerate.c` | `BattleGenerate` (`:69-90`), `PreBattleGenerateHook` (`:60-66`). |
| `Kernel/Wizardry/BattleSys/Source/BattleHit.c` | `BattleGenerateHitAttributes` (`:212`), which calls `BattleHit_CalcDamage` (`:257`). |
| `Kernel/Wizardry/BattleSys/Source/BattleDamage.c` | `CalcBattleRealDamage` (`:53`), `BattleHit_CalcDamage` (`:106`), damage list loop (`:186`). |
| `Kernel/Wizardry/BattleSys/Source/BattleUI.c` | `ModifyBattleStatusForUI` (`:40-45`), simulate only. |
| `Kernel/Wizardry/BattleSys/Source/BattleForcast.c` | Forecast-side battle stats (`:20`, `:41`). |
| `Kernel/Wizardry/BattleForcastFix/Source/BattleForcastFix.c` | `sub_8022E38` rewrite (`:7-15`), the target-selection cancel fix. |
| `Kernel/Wizardry/BattleSys/<Feature>/` | One folder per battle feature: `LeaderShip/`, `WrankBonus/`, `HpCost/`, `RoundAnim/` (each with `<Feature>.event` and `Source/`), and `NegateDef/` (a bare `NegateDef.c` and `NegateDef.lyn.event`). |
| `Kernel/Wizardry/BattleSys/BattleSys.event` | Includes the feature folders (`:6` LeaderShip, `:10` WrankBonus) and the `.lyn.event` files of the vanilla rewrites. |
| `Data/BattleSys/` | Data tables and data-side events: `Source/LeaderShip.c`, `Source/WrankBonus.c`, `BattleSys.event`. Included from `Data/Data.event:14`. |
| `Data/DesignerConfig/battle-designer-config.c` | `gKernelBattleDesignerConfig` values (`:4-17`). |
| `include/kernel/kernel-lib.h:204-233` | `struct KernelBattleDesignerConfig` and `gpKernelBattleDesignerConfig`. Spelled `KernelBattle...`; the runtime toggle struct is `KernelDesigerConfig`. |
| `include/kernel/battle-system.h` | Battle prototypes, e.g. `PreBattleCalcLeadershipBonus` (`:278`), `TryBattleHpCost` / `AddBattleHpCost` (`:321-322`). |
| `Patches/PATCH_DesignerConfigBattle.txt` | FEBuilder layout of the battle designer struct. |
| `Kernel/Wizardry/BanimHack/BanimHack.event` | `InjectBanimCmd` for `C07` (`:8`) and `C03` (`:9`). |
| `Kernel/Wizardry/BanimHack/Source/C07.c` | `C07` handler proc (`:83-113`), `Banim_C07` (`:118`). |
| `include/kernel/banim-hack.h` | Prototypes for banim effects, e.g. `NewCritCutin` and `CritCutinExists` (`:25-27`). |
| `Kernel/Wizardry/CritCutin/` | Banim presentation example (`CritCutin_Installer.event`, `Source/CritCutin.c`). |
| `Kernel/Wizardry/BanimSync/` | Banim switching (`BanimSwitcher`, `TrySwitchBanim` in `Kernel/Wizardry/BanimSync/Source/utils.c:35`) and the ekr-battle-daemon hook (`BanimSync_Internal.event`). |
| `Kernel/Wizardry/EfxMagicDummyFix/Source/EfxMagicDummy.c` | `LYN_REPLACE_CHECK(EfxDummymagicMain)` (`:5`), an example of rewriting a vanilla banim effect. |
| `Contents/EfxAnim/EfxAnim.event` | Labels for spell BG animations, e.g. `EfxCritCutin` (`:10-11`). |
| `Contents/EfxAnim/Source/` | Frame lists `*.efx.txt`, built to `*.efx.event` by the `efx` target (`makefile:313-330`). |
| `Kernel/Wizardry/MapAnimBattleHook/` | Per-round battle map-anim hooks (`Source/PreHit.c`, list in `MapAnimBattleHook.event:12-16`). |
| `Kernel/Wizardry/MapAnims/` | Map-anim effects: `Misc/Heal.c`, `ThunderStorm/`, `NinianPray/`, `HeavyGravity/`. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the hook:
   - A rule for the pre-battle stats (attack, hit, avoid, crit, speed) that applies to every strike: `PreBattleCalcFuncs`. Use the template below.
   - A rule that changes damage per strike, or rolls RN during a strike: `BattleDamageCalcFuncs` (`data.event:17`). See [BattleProc.md](../SkillTypes/BattleProc.md) for the skill side.
   - Once-per-battle setup or flags: `PreBattleGenerateFuncs` (`data.event:27`). It runs only under `#if CHAX` (`BattleGenerate.c:71-72`).
   - A map-anim step between rounds: `gPreMapAnimBattleRoundHooks` (`MapAnimBattleHook.event:12`). Follow the `KernelStartBlockingHookProc` rules in [PostAction.md](../SkillTypes/PostAction.md).
   - An attack banim effect: a `C07` handler. See the banim note in the Template.
   - Turn order or strike count: [BattleOrder.md](../SkillTypes/BattleOrder.md).
3. If the feature needs an on/off switch, add a `u8` to `KernelDesigerConfig` in `include/kernel/kernel-lib.h`, set its default in `Data/DesignerConfig/designer-config.c`, and add a row to `Documentation/Features/SystemConfig.md` (the CritCutin pattern). Battle-only tunables go in `struct KernelBattleDesignerConfig` (`kernel-lib.h:204`) with a default in `battle-designer-config.c`. Whether `Patches/PATCH_DesignerConfigBattle.txt` must be resynced for a new field is unverified.
4. Write `Kernel/Wizardry/BattleSys/<Feature>/Source/<Feature>.c` with a non-static function. Keep it small: the list runs on every battle calc (CLAUDE.md, hot path).
5. Declare the function in `include/kernel/battle-system.h` next to `PreBattleCalcLeadershipBonus` (`:278`).
6. Add `Kernel/Wizardry/BattleSys/<Feature>/<Feature>.event` (one include of the generated `.lyn.event`), and include it from `Kernel/Wizardry/BattleSys/BattleSys.event` next to the `WrankBonus/WrankBonus.event` line (`:10`).
7. Register the function in `Kernel/Wizardry/BattleSys/data.event`: add `POIN PreBattleCalc<Feature>` after the last real entry and before `POIN PreBattleCalcEnd` (`:13`). Do not put it after `PreBattleCalcEnd` (see Gotchas).
8. Build with `make`. It generates `Source/<Feature>.lyn.event` from the `.c` (`makefile:187-193`). Commit the generated file with the `.c` (CLAUDE.md).
9. Check the forecast if the rule should show: a pre-battle rule shows automatically, an RN rule does not.
10. Write `Documentation/Features/<Feature>.md` and add a bullet to the root `README.md` feature list (preamble, step 6).

## Template

### Pre-battle rule (PreBattleCalc)

Adapted from `Kernel/Wizardry/BattleSys/LeaderShip/Source/LeaderShip.c:33-69` (the designer gate, the arena skip, and writing only to `actor`) and `Kernel/Wizardry/BattleSys/WrankBonus/Source/WrankBonus.c:62-78` (the `battleHitRate` field update). *example_rule* is a placeholder for the `u8` you add to `KernelDesigerConfig`.

**Kernel/Wizardry/BattleSys/Example/Source/Example.c** (placeholder path):

```c
#include "common-chax.h"
#include "battle-system.h"
#include "kernel-lib.h"

/**
 * Example rule: +10 hit for the attacker against a boss.
 * Called twice per battle calc, once with each side as attacker
 * (BattleGenerate.c:75-76), so only write to attacker.
 */
void PreBattleCalcExample(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	if (!gpKernelDesignerConfig->example_rule)
		return;

	if (gBattleStats.config & BATTLE_CONFIG_ARENA)
		return;

	if (UNIT_CATTRIBUTES(&defender->unit) & CA_BOSS)
		attacker->battleHitRate += 10;
}
```

**Kernel/Wizardry/BattleSys/Example/Example.event** (placeholder path):

```
#include "Source/Example.lyn.event"
```

`Kernel/Wizardry/BattleSys/BattleSys.event`, after the `WrankBonus` include:

```
#include "Example/Example.event"
```

`Kernel/Wizardry/BattleSys/data.event`, in `PreBattleCalcFuncs`, before `PreBattleCalcEnd`:

```
    POIN PreBattleCalcStatDebuffs PreBattleCalcLeadershipBonus
    POIN PreBattleCalcExample

    POIN PreBattleCalcEnd
```

`include/kernel/battle-system.h`, after line 278:

```c
void PreBattleCalcExample(struct BattleUnit *attacker, struct BattleUnit *defender);
```

### Attack banim effect (CritCutin)

From the CritCutin commit (`git show --stat 4339e8533`): the effect is a module with a start function and an exists function, declared in `include/kernel/banim-hack.h` (`:25-27`), and started from the `C07` handler after the earlier steps (`Kernel/Wizardry/BanimHack/Source/C07.c:107-108`).

`include/kernel/banim-hack.h`:

```c
/* Kernel/Wizardry/Example */
void NewExample(struct Anim *anim);
bool ExampleExists(void);
```

`Kernel/Wizardry/BanimHack/Source/C07.c`, in `ProcScr_BanimC07Handler`, after the `EkrSkill` block and before `PROC_YIELD` / `C07Handler_End`:

```c
	/**
	 * Example
	 */
	PROC_CALL(C07Handler_Example),
	PROC_WHILE(ExampleExists),
```

with, above the proc script:

```c
STATIC_DECLAR void C07Handler_Example(struct ProcC07Handler *proc)
{
	NewExample(proc->anim);
}
```

**Kernel/Wizardry/Example/Source/Example.c** (placeholder path, a module in `FreeSpaceDEMO`):

```c
#include "common-chax.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "banim-hack.h"

extern const struct EfxAnimConf EfxExample;   /* Contents/EfxAnim/EfxAnim.event */

struct ProcExample {
	PROC_HEADER;
	struct Anim *anim;
};

void NewExample(struct Anim *anim)
{
	struct ProcExample *proc;

	if (!gpKernelDesignerConfig->example_cutin)
		return;

	/* The round is anim->nextRoundId - 1 (C07.c:54, CritCutin.c) */
	proc = Proc_Start(ProcScr_Example, PROC_TREE_3);
	proc->anim = anim;
}

bool ExampleExists(void)
{
	return Proc_Exists(ProcScr_Example);
}
```

`ProcScr_Example` is a `STATIC_DECLAR const struct ProcCmd` array with the effect steps and `PROC_END`. The installer, **Kernel/Wizardry/Example/Example_Installer.event** (placeholder path), has an include guard and `#include "Source/Example.lyn.event"`. Add `#include "Example/Example_Installer.event"` to the `WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event` (`CritCutin_Installer.event` is at `:140`). The band frames go in **Contents/EfxAnim/Source/Example/Example.efx.txt** (placeholder path), and `EfxAnim.event` gets an `EfxExample:` label that includes `Source/Example/Example.efx.event`.

## Worked examples

- `Kernel/Wizardry/BattleSys/LeaderShip/`: a pre-battle rule (`PreBattleCalcLeadershipBonus`, `Source/LeaderShip.c:33`) with a designer flag (`leadership`), an arena skip, and per-faction bonus tables. Data in `Data/BattleSys/Source/LeaderShip.c`.
- `Kernel/Wizardry/BattleSys/WrankBonus/`: a pre-battle rule (`PreBattleCalc_WrankBonus`, `Source/WrankBonus.c:62`) that reads `gpWrankBonusConf`, plus help-box hooks (`:83-138`). Table in `Data/BattleSys/Source/WrankBonus.c:18`.
- `Kernel/Wizardry/BattleSys/HpCost/`: a per-round rule through `TryBattleHpCost` / `AddBattleHpCost` (`include/kernel/battle-system.h:321-322`), and a map-anim step `MapAnimRoundAnim_DisplayHpCost` (`Kernel/Wizardry/BattleSys/HpCost/Source/MapAnimHpCost.c:27-41`) in `gPreMapAnimBattleRoundHooks`.
- `Kernel/Wizardry/CritCutin/`: banim presentation started from `C07` (`Source/CritCutin.c:200-216`), gated by `crit_cutin`. Documented in `Documentation/Features/CritCutin.md`.

## Gotchas

- Verified: the PreBattleCalc list runs once per side, with attacker and defender swapped (`BattleGenerate.c:75-76`). A rule that writes to `defender` changes the other side's stats during this side's pass, so the result depends on the order of the two calls.
- Verified: `PreBattleCalcEnd` clamps negative results to zero (`PreBattleCalc.c:365-380`). Entries after it in `data.event` run after the clamp. The four `KernelPad1` slots after it (`data.event:14`) are no-ops (`Kernel/Wizardry/KernelLib/Source/pad.c:4`). Using one of them for a pre-battle rule would put the rule after the clamp, so place new rules before `PreBattleCalcEnd`.
- Verified: `PreBattleGenerateHook` is compiled only with `#if CHAX` (`BattleGenerate.c:71-72`), and it runs before the stats are computed (`:75`).
- Verified: `ModifyBattleStatusForUI` runs only in simulate mode (`BattleUI.c:42`). The forecast shows a rule only if the rule is in the pre-battle stats. An RN rule in `BattleHit_CalcDamage` does not show in the forecast (see [BattleSystem.md](../Features/BattleSystem.md)).
- Verified: the arena check is `gBattleStats.config & BATTLE_CONFIG_ARENA` (`LeaderShip.c:40`). Copy it if the rule should not run in arena.
- Verified: `GetWrankBonusConf` loops until `it->wrank == WPN_LEVEL_0` (`Kernel/Wizardry/BattleSys/WrankBonus/Source/WrankBonus.c:14`). The table in `Data/BattleSys/Source/WrankBonus.c` ends with a zeroed `{}` entry for this reason.
- Verified: `Banim_C07` holds the attacker's animation script (`Kernel/Wizardry/BanimHack/Source/C07.c:123`). The hold ends when `C07Handler_End` sets `ANIM_BIT3_BLOCKEND` (`:78-81`, read at `:134-140`). A new C07 step therefore delays the strike until its `PROC_WHILE` ends.
- Verified: the cut-in skips far-far range (`Kernel/Wizardry/CritCutin/Source/CritCutin.c:208`) and checks the round with `anim->nextRoundId - 1` (`CritCutin.c`, same pattern as `C07.c:54`). Bounds-check the round against `NEW_BATTLE_HIT_MAX` (`C07.c:55`).
- Verified: `grep -rn crit_cutin Patches` returns nothing, so the FEBuilder patch does not list `crit_cutin` (also noted in `Documentation/Features/CritCutin.md`, Limitations). A new designer field needs the same check.
- Unverified: whether a list function placed in the `WIZARDRY_CUSTOM` block (FreeSpaceDEMO, like `CritCutin_Installer.event` at `wizardry.event:140`) works when it is referenced from `PreBattleCalcFuncs`. This doc does not show an example of it.
- Verified: when the target is `NULL` (a trap or wall), `BattleGenerate` calls `ComputeBattleObstacleStats` (`BattleGenerate.c:81-82`), which sets the target stats directly (`:41-53`). Whether a new pre-battle rule also behaves correctly for an obstacle target is unverified; test it against a wall.
- Verified: `Kernel/Wizardry/BattleSys/Source/LynJump.event` is hand-written (`PUSH` / `ORG` / `POIN` hook blocks), not generated. CLAUDE.md warns that some `.lyn.event` files carry hand edits: check before regenerating.
