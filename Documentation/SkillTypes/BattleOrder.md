# Battle order skills

> Text file: `Contents/Texts/Source/texts/Skills_BattleOrder.txt`  ·  Code: `Kernel/Wizardry/BattleSys/Source/BattleOrder.c`, `Kernel/Wizardry/BattleSys/BattleHitExpasion/Source/BattleHitExpa.c`  ·  Hook: `BattleUnwind` (`Kernel/Wizardry/BattleSys/Source/LynJump.event:101`), `PreBattleGenerateFuncs` (`Kernel/Wizardry/BattleSys/data.event:27`)

## What it is

Battle order skills decide, per battle, whether a side gets a second strike (follow-up), whether a side strikes first in the round (Vantage, or the Desperation reorder), and how many strikes an attack gets (`GetBattleUnitHitCount`: Astra, Adept, DoubleLion and others). They set bits in `gBattleFlagExt.round_mask`, which indexes `BattleUnwindConfig`. They also pick the skill id shown in the round's animation (`BattleOrderSkills[]`). Order is not a separate table of skills: the strike sequence is always `BattleUnwindConfig[round_mask]`.

## How the engine calls it

1. `BattleGenerate` calls `PreBattleGenerateHook` (`Kernel/Wizardry/BattleSys/Source/BattleGenerate.c:72`, defined at `:60`), which walks the pointer `gpPreBattleGenerateFuncs` (`Kernel/Preload/KernelReloc.event:42`, loop at `BattleGenerate.c:64`). That pointer targets the `PreBattleGenerateFuncs` list (`Kernel/Wizardry/BattleSys/data.event:27-33`), which includes `PreBattleGenerate_GenerateOrderFlags` (`Kernel/Wizardry/BattleSys/Source/BattleOrder.c:548`). It sets:
   - `UNWIND_DESPERA` from `CheckDesperationOrder` (`BattleOrder.c:407`, call `:556`),
   - `UNWIND_VANTAGE` from `CheckVantageOrder` (`:475`, call `:559`),
   - `UNWIND_DOUBLE_ACT` and `UNWIND_DOUBLE_TAR` from `CheckCanTwiceAttackOrder` (`:67`, calls `:562` and `:565`),
   - then `PostCalc_ModifyBattleOrderRoundMask` (`:525`, Last Word, call `:568`).
2. `BattleGenerate` calls `BattleUnwind` (`BattleGenerate.c:87`; defined `BattleOrder.c:920`, hooked through `LynJump.event:101` and `BattleOrder.lyn.event:45`). Inside `BattleUnwind`:
   - `RegenerateBattleOrderFlagsAfterCalc` (`:571`, call `:931`) recomputes the speed-only double bits after stats are final. It acts only if `act_normal_judge_twice_order` or `tar_normal_judge_twice_order` is set. Those are set only in the speed branch at `BattleOrder.c:390-396`, which is reached only when no skill decided the double.
   - Duel: `RunDuelCombatLoop` (`:729`) replaces the whole unwind (check `:936`).
   - `BattleUnwindConfig[round_mask]` (`Kernel/Wizardry/BattleSys/Source/data.c:4`) gives up to four steps (`ACT_ATTACK`, `TAR_ATTACK`, `NOP_ATTACK`). The loop is at `BattleOrder.c:954-980`. Each step runs `TryExecuteComboAttackOnce` (`:970`, defined `:835`), then `BattleGenerateRoundHits` (`:975`), then the animation helpers `TryRegisterCombatArt`, `TryRegisterVantage` (`:659`), `TryRegisterDesperation` (`:668`) and `TryRegisterForcedDouble` (`:683`).
   - Accost repeats the whole round (`TryTriggerAccostRoundRepeat`, `:889`, `goto repeat_full_round` at `:1007`).
3. `BattleGenerateRoundHits` (`BattleOrder.c:1074`): calls `ResetRoundEfxSkills` (`Kernel/Wizardry/BattleSys/Source/Misc.c:32`), then `roundCount = GetBattleUnitHitCount(attacker)` (`:1153`). For each strike it calls `ApplyRoundSkillEffects` (`:1037`, call `:1108`), which dequeues one skill from `EnqueueRoundEfxSkill` and registers its animation for rounds above 0. Then it calls `BattleGenerateHit` (`BattleHit.c:581`, call `BattleOrder.c:1114`).
4. `GetBattleUnitHitCount` (`BattleOrder.c:1153`): starts at 1. Adds 1 for brave (`BattleCheckBraveEffect`, `:1259`). Then skill increments: RuinedBladePlus (`:1166`), LoadstarRush (`:1174`), Astra (`:1182`, uses `CheckBattleSkillActivate`), Adept (`:1196-1200`), ChargePlus, DoubleLion (`:1210-1213`), LastStand (`:1221`), Echo (`:1229`), Flurry (`:1235-1240`), UnarmedCombat, and the Quicken status.
5. Follow-up: `CheckCanTwiceAttackOrder` (`:67`) is called once per side from `PreBattleGenerate_GenerateOrderFlags` (`:562`, `:565`). `BattleGetFollowUpOrder` (`:1138`) calls it again for the vanilla follow-up path.
6. Combo attacks: `TryExecuteComboAttackOnce` (`:835`) calls `BattleComboGenerateHits` (`Kernel/Wizardry/ComboAttack/Source/battle-order-combo.c:150`). That is a separate feature, not a double or hit-count skill.

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/BattleSys/Source/BattleOrder.c` | All judgement and unwind code: `CheckCanTwiceAttackOrder` (`:67`), `CheckDesperationOrder` (`:407`), `CheckVantageOrder` (`:475`), `PostCalc_ModifyBattleOrderRoundMask` (`:525`), `PreBattleGenerate_GenerateOrderFlags` (`:548`), `RegenerateBattleOrderFlagsAfterCalc` (`:571`), `TryRegister*` (`:659-690`), `BattleUnwind` (`:920`), `GetBattleUnitHitCount` (`:1153`). |
| `Kernel/Wizardry/BattleSys/Source/data.c` | `BattleUnwindConfig[16][4]` (`:4`). The row index is `round_mask`. |
| `Kernel/Wizardry/BattleSys/data.event` | `PreBattleGenerateFuncs` (`:27`), the list that runs the order flags. |
| `Kernel/Wizardry/BattleSys/Source/LynJump.event` | `POIN BattleUnwind` (`:101`). |
| `Kernel/Wizardry/BattleSys/Source/BattleOrder.lyn.event` | Generated from `BattleOrder.c` (hook at `:45`, `PROTECT` on `BattleUnwind` at `:147`). Do not edit. |
| `Kernel/Wizardry/BattleSys/Source/Misc.c` | `ResetRoundEfxSkills` (`:32`), `EnqueueRoundEfxSkill` (`:37`), `DequeueRoundEfxSkill` (`:43`). |
| `include/kernel/battle-system.h` | `BORDER_*` slots (`:150-156`), `BattleOrderSkills` and `RegisterBattleOrderSkill` (`:161-163`), `UNWIND_*` bits (`:180-184`), `BattleUnwindConfig` (`:186`), `EfxSkillQueue` (`:188-192`). |
| `include/link/config-memmap.s` | RAM for `BattleOrderSkills` (`:238`, 8 bytes, 4 slots). |
| `Kernel/Wizardry/BattleSys/Source/BattleGenerate.c` | `PreBattleGenerateHook` (`:60`, call `:72`), `BattleUnwind` call (`:87`). |
| `Kernel/Wizardry/BattleSys/BattleHitExpasion/Source/BattleHitExpa.c` | `CheckBattleHitOverflow` (`:8`), `ClearBattleHits` (`:26`), which clears the round animation state. |
| `Kernel/Wizardry/ComboAttack/Source/battle-order-combo.c` | `BattleComboGenerateHits` (`:150`). Combo-attack feature, not a double or hit-count skill. |
| `Data/SkillSys/Source/SkillAnimInfo.c` | Animation priority entries for order skills (`EfxSkillAnimPriority`, `:12-36`) and `EfxSkillAnims` (`:352`). |
| `Kernel/Wizardry/SkillSys/EfxSkill/Utils.c` | `RegisterActorEfxSkill` (`:22`): per-round storage with priority check. |
| `Documentation/Features/SkillSys.md` | Section 6.3, partly stale (see Gotchas). |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the hook by what the skill does:
   - Gives a side a second strike: add a block to `CheckCanTwiceAttackOrder` (`BattleOrder.c:67`), in the `&gBattleActor == actor` branch or the `&gBattleTarget == actor` branch. It must `return true`, call `RegisterBattleOrderSkill(SID_X, BORDER_ACT_TWICE)` (or `BORDER_TAR_TWICE`), and set `gBattleTemporaryFlag.act_force_twice_order` (or `tar_force_twice_order`) for the animation. A skill that denies the double does `return false` before the speed judgement (see RapidFighter, `:274-282`).
   - Takes the first strike: add a block to `CheckVantageOrder` (`:475`). Set `gBattleTemporaryFlag.vantage_order = true`, call `RegisterBattleOrderSkill(SID_X, BORDER_VANTAGE)`, `return true`.
   - Reorders an existing double: follow `CheckDesperationOrder` (`:407`). The reorder only has an effect together with a double (see Gotchas).
   - Adds strikes: add `result = result + N` in `GetBattleUnitHitCount` (`:1153`) and call `EnqueueRoundEfxSkill(SID_X)` for the animation.
   - Changes the round mask after all of the above: `PostCalc_ModifyBattleOrderRoundMask` (`:525`), as used for Last Word.
3. Guard each block with `#if defined(SID_X) && (COMMON_SKILL_VALID(SID_X))`, as the file does.
4. Check the skill with `BattleFastSkillTester(actor, SID_X)`. These checks run in pre-battle and inside `BattleUnwind`, after the fast list is set up.
5. Add the animation entries for the skill in `Data/SkillSys/Source/SkillAnimInfo.c`: a priority in `EfxSkillAnimPriority[]` and `&EfxSkillVanilla` in `EfxSkillAnims[]`. Same as the proc skills in [BattleProc.md](BattleProc.md).
6. Build. Test in battle at the HP thresholds (50%), on both phases, and in the enemy's counter position.

## Template

Adapted from BoldFighter (`Kernel/Wizardry/BattleSys/Source/BattleOrder.c:192-197`). Actor-side forced double with an HP condition. Place it in `CheckCanTwiceAttackOrder`, in the `if (&gBattleActor == actor) {` branch, after the existing checks.

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
		if (BattleFastSkillTester(actor, SID_Example) && ref_actor_hp_above_half) {
			gBattleTemporaryFlag.act_force_twice_order = true;
			RegisterBattleOrderSkill(SID_Example, BORDER_ACT_TWICE);
			return true;
		}
#endif
```

Extra strike, adapted from Adept (`BattleOrder.c:1196-1200`). Place it in `GetBattleUnitHitCount`, before the final `return result;`:

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	if (actor == &gBattleActor && BattleFastSkillTester(actor, SID_Example) && actor->hpInitial == actor->unit.maxHP) {
		EnqueueRoundEfxSkill(SID_Example);
		result = result + 1;
	}
#endif
```

## Worked examples

- Vantage (`Kernel/Wizardry/BattleSys/Source/BattleOrder.c:475-522`): target-side first strike when HP is below 50%. Sets `vantage_order` and `BORDER_VANTAGE`. Read back by `TryRegisterVantage` (`:659`).
- Desperation (`BattleOrder.c:407-424`) with the reorder rows in `Kernel/Wizardry/BattleSys/Source/data.c:4-21`: shows the `UNWIND_DESPERA` bit, and why it only acts together with a double.
- Quick Riposte (`BattleOrder.c:344-352`): target-side double at HP above 50%, with `tar_force_twice_order` and `BORDER_TAR_TWICE`.
- Adept (`BattleOrder.c:1196-1200`): extra strike in `GetBattleUnitHitCount` at full HP.
- Double Lion (`BattleOrder.c:1210-1215`): extra strike, not a follow-up skill. It is listed as an order skill in `SkillSys.md` section 6.3.

## Gotchas

- The `*_force_twice_order` flags do not grant a double. The only reader is `TryRegisterForcedDouble` (`BattleOrder.c:685-689`), which registers the animation. The double itself is the return value of `CheckCanTwiceAttackOrder`, read into a `round_mask` bit (`:562-565`). A new double skill must `return true`.
- Desperation alone changes nothing. `BattleUnwindConfig[0b0010]` equals `BattleUnwindConfig[0b0000]` (`data.c:4-21`: both are ACT, TAR). Desperation matters only with a double: rows `0b0110` and `0b1110` put the actor's strikes first (ACT, ACT, TAR). `TryRegisterDesperation` (`BattleOrder.c:668-681`) registers its animation only for that ACT, ACT, TAR pattern.
- `RegisterBattleOrderSkill` overwrites `BattleOrderSkills[type]` (`include/kernel/battle-system.h:163`). There are four display slots: DESPERATION, VANTAGE, ACT_TWICE, TAR_TWICE (`:154-157`). The animation shows the last skill registered in that slot during the battle, which may not be the one that acted. A fifth slot needs `BORDER_MAX`, a larger RAM block (`config-memmap.s:238` is 8 bytes), and new code in the `TryRegister*` helpers.
- RapidFighter returns false after setting the animation flag, in both branches (`BattleOrder.c:280-281` and `:382-383`). The early return also skips the speed judgement, so the double is denied on the enemy phase, as the text says.
- WaryFighter (`BattleOrder.c:136`) is checked only inside the `&gBattleActor == actor` block. A WaryFighter holder on the gBattleActor side does not stop the gBattleTarget double. Observed in code, not run. The text does not say which side it protects.
- Flurry enqueues `SID_Echo` (`BattleOrder.c:1238`), not `SID_Flurry`. Probably a copy-paste error. Observed in code, not run.
- `EnqueueRoundEfxSkill` drops entries silently once the 14-slot pool is full (`Kernel/Wizardry/BattleSys/Source/Misc.c:39-40`).
- `Documentation/Features/SkillSys.md` section 6.3 is stale:
  - It says the order is "fixedly handled by four skills (Vantage, Desperation, QuickRiposte, DoubleLion)". The judgement is in many skills across `CheckCanTwiceAttackOrder`, `CheckDesperationOrder` and `CheckVantageOrder`.
  - DoubleLion is a hit-count skill (`GetBattleUnitHitCount`, `:1210-1213`), not an order skill.
  - It says the hit count is "mainly handled in `BattleComboGenerateHits`". That is the combo-attack function (`battle-order-combo.c:150`). Hit counts come from `GetBattleUnitHitCount` (`BattleOrder.c:1153`).
  - The round order is still `BattleUnwindConfig`, and the animation slots are still the four `BORDER_*` values in the `TryRegister*` helpers.
