# Battle proc skills

> Text file: `Contents/Texts/Source/texts/Skills_BattleAct.txt` (procs), `Contents/Texts/Source/texts/Skills_BattleNihil.txt` (Nihil)  ·  Code: `Kernel/Wizardry/SkillEffects/BattleDamage/`, `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c`, `Kernel/Wizardry/BattleSys/Source/`  ·  Hook: `BattleDamageCalcFuncs` list (`Kernel/Wizardry/BattleSys/data.event:17`), read through `gpBattleDamageCalcFuncs` (`Kernel/Preload/KernelReloc.event:9`), `CheckBattleSkillActivate`

## What it is

A proc is a skill that rolls at a call site with `CheckBattleSkillActivate` (`Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c:77`) during a battle strike. On success it changes that strike: hit or crit rate, attacker or defender stats, `gDmg` modifiers, HP drain, the survive-at-1-HP outcomes, or it nullifies the damage. Some skills in these files do not roll at all: Nihil (clears the opponent's battle skill list), HoldOut and Sturdy (survive a hit, `BattleHitAttr.c` `CheckBattleInori`), Levitate and Multiscale (deterministic damage checks). `CheckBattleSkillActivate` never rolls during the forecast (it returns false when `BATTLE_CONFIG_SIMULATE` is set, `SkillActRate.c:79-80`).

## How the engine calls it

Per strike:

1. `BattleGenerate` (`Kernel/Wizardry/BattleSys/Source/BattleGenerate.c:87`) calls `BattleUnwind` (`Kernel/Wizardry/BattleSys/Source/BattleOrder.c:920`), which calls `BattleGenerateRoundHits` (`BattleOrder.c:1074`), which calls `BattleGenerateHit` (`Kernel/Wizardry/BattleSys/Source/BattleHit.c:581`) once per strike.
2. `BattleGenerateHit` calls `BattleUpdateBattleStats` (`BattleHit.c:588`, defined at `:23`). Hit and crit procs roll there: AxeFaith (`:52`), SureShot (`:61`), Deadeye (`:71`).
3. `BattleGenerateHit` then calls `BattleGenerateHitAttributes` (`BattleHit.c:616`, defined at `:213`). It rolls the hit with `BattleRoll2RN(gBattleStats.hitRate)` (`:219`). On a miss, Divine Pulse (`:221`) and Grazing Blow are checked. On a hit, `BattleHit_CalcDamage` runs (`:257`). After that: Eclipse via `CheckBattleHpHalve` (`Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c:13`, called at `BattleHit.c:411`), `CheckDevilAttack` (`BattleHitAttr.c:28`, called at `:416`), and Bane (`BattleHit.c:430-433`, inline, not a switch).
4. `BattleGenerateHit` then calls `BattleGenerateHitEffects` (`BattleHit.c:617`, defined at `:295`). `BattleHit_CalcHpDrain` (`BattleHitAttr.c:166`, called at `BattleHit.c:509`) rolls Sol (`BattleHitAttr.c:198-199`) and adds the Aether flag `gBattleTemporaryFlag.skill_activated_aether` (set in `AttackerSkills.c:124`). Later, when the defender is at 0 HP, `CheckBattleInori` (`BattleHitAttr.c:89`, called at `BattleHit.c:678`) rolls Mercy (`BattleHitAttr.c:92`) and Miracle (`:99`).
5. Damage: `BattleHit_CalcDamage` (`Kernel/Wizardry/BattleSys/Source/BattleDamage.c:106`). The loop at `BattleDamage.c:186` walks the pointer `gpBattleDamageCalcFuncs` (`Kernel/Preload/KernelReloc.event:9`), which points at the `BattleDamageCalcFuncs` list (`Kernel/Wizardry/BattleSys/data.event:17-19`). The list holds `BattleDamageCalc_CombatArt`, `BattleDamageCalc_AttackerSkills` (`Kernel/Wizardry/SkillEffects/BattleDamage/AttackerSkills.c:8`) and `BattleDamageCalc_DefenderSkills` (`Kernel/Wizardry/SkillEffects/BattleDamage/DefenderSkills.c:9`). Each of the two skill functions walks the unit's battle skill list (`GetUnitSkillList`) with a `switch`. After the loop: the base formula at `BattleDamage.c:209`, the GREATSHLD early return at `:196-204`, the final formula at `:312-317`, and post-formula overrides from `:323` onward (Bide, Decadon, Levitate, Equalizer, Osmose and others).
6. Roll: `CheckBattleSkillActivate` (`SkillActRate.c:77`) first returns false on Foresight on the target (`:82-84`), then calls `CheckSkillActivateExt` (`:6`). That function runs the tester, then RightfulArch returns true at once (`:12`), RightfulKing and RightfulGod add to the rate (`:16-23`), Hero (`:26-29`), Domino (`:33-50`), SereneGrace doubles it (`:53-56`), clamps with `LIMIT_AREA(rate, 0, 100)` (`:59`), and rolls with `Roll2RN(rate)` (`:61`).
7. Nihil: `InitBattleUnit` (`Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c:236`, run at `:339`) walks the pointer `gpExternalUnitToBattleHook` (`Kernel/Preload/KernelReloc.event:64`), which points at the `gExternalUnitToBattleHook` list (`Kernel/Wizardry/UnitHooks/data.event:18-20`). The first entry is `UnitToBattle_SetupSkillList` (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:624`). It acts only for the target (`:637`). It checks Nihil on each side (`:640-649`), calls `DisableUnitSkilLList` (`:526`) on the other side, then rebuilds the fast list (`SetupBattleSkillFastList`, `:651`).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c` | Rate formula, Foresight, RightfulArch/King/God, Hero, Domino, SereneGrace, `Roll2RN`. Also `CheckSkillActivate` and `CheckActiveUnitSkillActivate` (non-battle). |
| `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.lyn.event` | Generated from the `.c` (`makefile:187`). Included by `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event:133`. Do not edit. |
| `Kernel/Wizardry/SkillEffects/BattleDamage/AttackerSkills.c` | Attacker `switch`: `gDmg.increase`, `gDmg.correction`, `gDmg.decrease`, `gDmg.crit_correction`, `gBattleStats.attack` and `gBattleStats.defense` changes. Also `RegisterActorEfxSkill`. |
| `Kernel/Wizardry/SkillEffects/BattleDamage/DefenderSkills.c` | Defender `switch`: GreatShield, Aegis, Pavise (damage nullify), Multiscale, Barricade, and other reductions. |
| `Kernel/Wizardry/BattleSys/Source/BattleDamage.c` | `CalcBattleRealDamage` (`:53`), `BattleHit_CalcDamage` (`:106`) with the formula and post-formula overrides, RampUp (`:254`). |
| `Kernel/Wizardry/BattleSys/Source/BattleHit.c` | Hit and crit procs in `BattleUpdateBattleStats` (`:23`), Bane (`:430`), Inori call site (`:678`), `BattleGenerateHit` (`:581`). |
| `Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c` | Eclipse (`:13`), Devil (`:28`), Mercy/Miracle (`CheckBattleInori`, `:89`), HP drain and Sol (`:166`, `:198`), Corrosion (`:455`). |
| `Kernel/Wizardry/BattleSys/Source/BattleCalcReal.c` | Foresight zeroes the attacker's crit rate (`:284-288`). |
| `Kernel/Wizardry/BattleSys/data.event` | `BattleDamageCalcFuncs` (`:17`), `BattleRealDamageCalcFuncs` (`:23`) hook tables. |
| `Kernel/Wizardry/SkillSys/EfxSkill/Utils.c` | `RegisterActorEfxSkill` (`:22`), `RegisterTargetEfxSkill` (`:33`): per-round storage, priority check. |
| `Kernel/Wizardry/SkillSys/EfxSkill/EkrSkill.c` | `NewEkrSkill` reads the per-round actor and target skill ids (`:68-69`). |
| `Kernel/Wizardry/SkillSys/EfxSkill/EfxSkill.c` | `NewEfxSkill` looks up `gpEfxSkillAnims[sid]` (`:89`) and falls back to `gpEfxSkillVanilla`. |
| `Data/SkillSys/Source/SkillAnimInfo.c` | `EfxSkillAnimPriority[]` (`:6`) and `EfxSkillAnims[]` (`:352`). Edit this file, not the `.lyn.event`. |
| `Data/SkillSys/SkillSys.event` | Includes `Source/SkillAnimInfo.lyn.event` (`:1`). |
| `Data/SkillSys/Source/SkillExtraInfo.c` | Per-skill numbers read with `SKILL_EFF0`..`SKILL_EFF3` (e.g. Aether `[SID_Aether] = {{ 80 }}`, `:439`). |
| `Kernel/Wizardry/SkillSys/kernel/SkillList.c` | `UnitToBattle_SetupSkillList` (`:624`), `DisableUnitSkilLList` (`:526`), `SetupBattleSkillFastList` (`:538`). Holds the Nihil block. |
| `Kernel/Wizardry/UnitHooks/data.event` | `gExternalUnitToBattleHook` list (`:18-20`). |
| `Kernel/Wizardry/SkillEffects/Nihil/Source/Nihil.lyn.event` | Stale binary. Not included by any `.event` (see Gotchas). |
| `Kernel/Wizardry/BattleSys/Source/BattleSkillActRate.lyn.event` | Stale binary. Not included by any `.event` (see Gotchas). |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the hook by what the proc changes:
   - Attacker damage, defense or attack change: add a `case` to the switch in `AttackerSkills.c`.
   - Defender damage reduction or nullify: add a `case` to the switch in `DefenderSkills.c`. Keep the argument order of `CheckBattleSkillActivate(defender, attacker, ...)` (see `DefenderSkills.c:66-67`).
   - Hit, crit, drain, survive-at-1-HP, or status: add a block to the matching function in `BattleHit.c` or `BattleHitAttr.c`, following the Bane or Mercy pattern.
   - The switches only see skills in the unit's battle skill list, so a skill that is not granted to the unit never fires.
3. Guard the code with `#if (defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example)))`.
4. Roll with `CheckBattleSkillActivate(actor, target, SID_Example, rate)`. The rate argument is chosen by the call site. Most use `attacker->unit.skl`. Miracle uses the defender's luck (`BattleHitAttr.c:99`).
5. On success, call `RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Example)` (or `RegisterTargetEfxSkill` for a defender-side proc) before changing the damage or stats. The register call only sets the animation for this round.
6. Put the numbers in `Data/SkillSys/Source/SkillExtraInfo.c` as `[SID_Example] = {{ N }}` and read them with `SKILL_EFF0(SID_Example)`.
7. Add the animation entries in `Data/SkillSys/Source/SkillAnimInfo.c`, both guarded: a priority in `EfxSkillAnimPriority[]` and `&EfxSkillVanilla` in `EfxSkillAnims[]`. See the template.
8. Build. Test in a real battle: the forecast will not show the proc.

## Template

Adapted from `DragonFang` (`Kernel/Wizardry/SkillEffects/BattleDamage/AttackerSkills.c:183-188`). The rate uses `attacker->unit.skl` unchanged (DragonFang itself uses `skl * 2`, see Gotchas). `gDmg.increase` is a percentage with base 100 (`BattleDamage.c` `PreBattleCalcInit_BaseDamage`, `:97`).

Attacker switch case, inside `switch (list->sid[_skill_list_cnt])` in `BattleDamageCalc_AttackerSkills`. Insert before the `case MAX_SKILL_NUM:` line:

```c
#if (defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example)))
		case SID_Example:
			if (CheckBattleSkillActivate(attacker, defender, SID_Example, attacker->unit.skl)) {
				RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Example);
				gDmg.increase += SKILL_EFF0(SID_Example);
			}
			break;
#endif
```

Animation entries in `Data/SkillSys/Source/SkillAnimInfo.c`:

```c
// in const u8 EfxSkillAnimPriority[MAX_SKILL_NUM + 1] = { ... };
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = EFX_PRIORITY_NORMAL,
#endif

// in struct EfxAnimConf const *const EfxSkillAnims[MAX_SKILL_NUM + 1] = { ... };
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = &EfxSkillVanilla,
#endif
```

Numbers in `Data/SkillSys/Source/SkillExtraInfo.c`:

```c
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = {{ 50 }},
#endif
```

## Worked examples

- Luna (`Kernel/Wizardry/SkillEffects/BattleDamage/AttackerSkills.c:89-95`): physical-only proc that sets `gBattleStats.defense = 0`. Shows the attacker switch, `!IsMagicAttack` gating, and `RegisterActorEfxSkill`.
- Aegis and Pavise (`Kernel/Wizardry/SkillEffects/BattleDamage/DefenderSkills.c:66-82`): damage nullify. They set `BATTLE_HIT_ATTR_GREATSHLD` and `gBattleStats.defense = BATTLE_MAX_DAMAGE`. The formula then returns 0 (`BattleDamage.c:196-204`). Shows defender-side roll order.
- Miracle (`Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c:98-103`): roll with the defender as the actor and the defender's luck as the rate. Called only when the defender would die (`BattleHit.c:678`).
- Sol (`Kernel/Wizardry/BattleSys/Source/BattleHitAttr.c:198-202`): HP drain. Adds 100% to `percentage` inside `BattleHit_CalcHpDrain` (`:166`).
- Nihil (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:640-649`): no roll. Disables the opponent's battle skill list.

## Gotchas

- A proc only fires if the skill is in the battle skill list. The attacker and defender switches iterate `GetUnitSkillList`, and the tester functions read the fast list (`BattleFastSkillTester`).
- Procs are suppressed in the forecast (`SkillActRate.c:79-80`). The forecast cannot show a proc or a proc-based damage change.
- Text and code disagree for two skills (not changed here):
  - DragonFang's text says "Skill %", but the rate is `attacker->unit.skl * 2` (`AttackerSkills.c:184`).
  - Aether's text says "Skill/2 %", but the rate is `attacker->unit.skl` (`AttackerSkills.c:123`).
- RampUp appears to double crit damage twice (observed in code, not run in game). `AttackerSkills.c:61-65` sets `gBattleStats.attack *= 2` inside the calc loop, which runs before `base_damage` is computed at `BattleDamage.c:209`. `BattleDamage.c:254-256` then sets `base_damage *= 2` again.
- Domino (`SkillActRate.c:46`) uses a comma expression: `if (tester(unit_ally, SID_Domino), AreUnitsAllied(...))`. The tester result is discarded, so any allied unit in the 3x3 grants the +10 (observed in code, not run).
- Symbiosis appends the rescuee's skill ids to the actor's or defender's list on every call of the attacker and defender functions (`AttackerSkills.c` and `DefenderSkills.c`, `list->sid[list->amt++] = ...`). There is no dedupe and no bound check against `sid[31]` (`include/kernel/skill-system.h:165`). Observed in code, not run.
- Nihil's text says it negates "combat-related skills". The code clears the whole battle list for that side (`DisableUnitSkilLList` sets `amt = 0`, `SkillList.c:526-531`). Any skill in the list is lost, including non-combat ones.
- `Kernel/Wizardry/SkillEffects/Nihil/Source/Nihil.lyn.event` is a stale binary with no `.c` source. No `.event` file includes it (grep for `Nihil.lyn` and `SkillEffects/Nihil` finds nothing). Editing it has no effect. The live Nihil code is in `SkillList.c`.
- `Kernel/Wizardry/BattleSys/Source/BattleSkillActRate.lyn.event` is also not included by any `.event`. If it were, it would define `CheckBattleSkillActivate` a second time. Do not add it to a build.
- Rates are clamped to 0..100 only after the bonuses (`SkillActRate.c:59`). RightfulArch makes every proc of its holder succeed: `CheckSkillActivateExt` returns true before the clamp (`:12`).
- `RegisterActorEfxSkill` replaces the stored skill only if the new skill's priority is not lower (`Utils.c:26`). A skill with no `EfxSkillAnimPriority` entry has priority 0 and loses to any registered animation of that round.
- Foresight needs both checks: the proc block (`SkillActRate.c:82-84`) and the crit zeroing (`BattleCalcReal.c:284-288`). `CheckSkillActivate` (the non-battle path) has no Foresight check.
