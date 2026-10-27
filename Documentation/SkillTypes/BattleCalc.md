# Pre-battle calc skills

> Text files: `Contents/Texts/Source/texts/Skills_BattleStatus.txt`, `Skills_Blow.txt`, `Skills_Stance.txt`, `Skills_WeaponTriangle.txt`, `Skills_BattleEffective.txt`, `Skills_RangeDebuff.txt`  ·  Code: `Kernel/Wizardry/BattleSys/Source/`  ·  Hook: `gpPreBattleCalcFuncs` (`Kernel/Wizardry/BattleSys/data.event`)

## What it is

Skills that change a battle unit's numbers (attack, defense, speed, hit, avoid, crit, dodge, silencer) during the pre-battle pass `ComputeBattleUnitStats`, before any round is rolled. They also set the weapon-triangle bonus and the effectiveness multiplier. Results land in the `battle*` fields of `struct BattleUnit` (`Tools/FE-CLib-Mokha/include/bmbattle.h:20`).

Sub-kinds (each has its own code location and template below):

- **Flat battle bonus** (`Skills_BattleStatus.txt`): unconditional, or gated by weapon type / range / magic. A `case` in the self switch of `PreBattleCalcAttackerSkills`. Example: `SID_Crit` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:856`).
- **Blow** (`Skills_Blow.txt`): applies only when this unit initiates the attack. Same switch, gated `attacker == &gBattleActor`. Example: `SID_BlowDeath` (`PreBattleCalc.c:653`).
- **Stance** (`Skills_Stance.txt`): applies only when this unit is the one attacked. Same switch, gated `attacker == &gBattleTarget`. Example: `SID_StanceBracing` (`PreBattleCalc.c:728`).
- **Weapon triangle** (`Skills_WeaponTriangle.txt`): table-driven, not a switch. Entries in `gWeaponTriangleConfs` carry a `sid`. Example: `SID_SwordBreaker` (`Data/BattleSys/Source/WeaponTriangle.c:29`).
- **Effectiveness** (`Skills_BattleEffective.txt`): two kinds. (a) Amplifiers on an effective hit, `SID_Resourceful` / `SID_SolidRock` in `ComputeBattleUnitAttack` (`PreBattleCalc.c:103-110`). (b) "Is effective against target" predicates and null-effective skills in `Kernel/Wizardry/BattleSys/Source/CheckEffective.c` (`SID_Slayer` at `:213`, `SID_Nullify` at `:42`).
- **Range / aura** (`Skills_RangeDebuff.txt`): skills of units standing near the computed unit, evaluated in `PreBattleCalcAuraEffect` (`PreBattleCalc.c:2105`). Allied holders buff, enemy holders debuff. Example: `SID_Hex` (enemy holder, adjacent, `PreBattleCalc.c:2349`), `SID_Bond` (allied holder, any cell, `:2283`).

## How the engine calls it

1. `BattleGenerate` (`Kernel/Wizardry/BattleSys/Source/BattleGenerate.c:69`). Under `#if CHAX` it calls `PreBattleGenerateHook()` (`:72`), which runs `PreBattleGenerateFuncs` (`Kernel/Wizardry/BattleSys/data.event:27`). `PreBattleGenerate_SetupWtaStatus` (`Kernel/Wizardry/BattleSys/Source/WeaponTriangle.c:195`) fills `gWtaStatus_act` / `gWtaStatus_tar`.
2. `BattleGenerate` calls `ComputeBattleUnitStats(&gBattleActor, &gBattleTarget)` (`BattleGenerate.c:75`) and then `ComputeBattleUnitStats(&gBattleTarget, &gBattleActor)` (`:76`). So the list runs twice per battle calc, once per side. `ComputeBattleUnitStats` is a lynjump replacement (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:2534`, vanilla hook at `Kernel/Wizardry/BattleSys/Source/LynJump.event:5`).
3. `ComputeBattleUnitStats` loops over `gpPreBattleCalcFuncs` and calls `(*it)(attacker, defender)` (`PreBattleCalc.c:2541`). The list is in `Kernel/Wizardry/BattleSys/data.event:4-15`, in this order:
   - `PreBattleCalcInit` (`PreBattleCalc.c:339`). Only when `attacker == &gBattleActor`: calls `Local_PreBattleCalcInitExt` for both sides (`:297`, `:357-361`). That computes base atk/def/spd/hit/avo/crit and snapshots them to `BattleUnitOriginalStatus`.
   - `PreBattleCalcSilencerRate` (`PreBattleCalc.c:2524`), then `PreBattleCalcWeaponTriangle` (`Kernel/Wizardry/BattleSys/Source/WeaponTriangle.c:251`).
   - `PreBattleCalcAttackerSkills` (`PreBattleCalc.c:597`): self skills. Blow and Stance cases live here. Then `PreBattleCalcDefenderSkills` (`PreBattleCalc.c:412`): foe skills, mostly damage reduction through `GetBaseDmg`, not battle stats.
   - `PreBattleCalcDebuffs` and `PreBattleCalcCombatArt`: defined outside `PreBattleCalc.c`; not in this doc's scope. Then `PreBattleCalcAuraEffect` (`PreBattleCalc.c:2105`): the range/aura pass, covered in this doc.
   - `PreBattleCalcShieldBonus` and `PreBattleCalc_WrankBonus`: defined outside `PreBattleCalc.c`; not in this doc's scope.
   - `PreBattleCalcStatDebuffs` and `PreBattleCalcLeadershipBonus`: defined outside `PreBattleCalc.c`; not in this doc's scope.
   - `PreBattleCalcEnd` (`PreBattleCalc.c:365`): clamps atk/def/spd/hit/avo/crit/dodge to 0.
4. Base formulas that skills can alter are called from `PreBattleCalcInit` via `Local_PreBattleCalcInitExt` (`PreBattleCalc.c:297-318`): `ComputeBattleUnitDefense` (`:166`), `ComputeBattleUnitAttack` (`:78`), `ComputeBattleUnitSpeed` (`:25`), `ComputeBattleUnitHitRate` (`:252`), `ComputeBattleUnitAvoidRate_Rework`, `ComputeBattleUnitCritRate` (`:219`). Examples inside them: `SID_Resourceful` (`:104`), `SID_SuperLuck` / `SID_CriticalForce` (`:219-236`).
5. Second pass. `BattleGenerate` then calls `ComputeBattleUnitEffectiveStats` for both sides (`BattleGenerate.c:78-79`, body at `Kernel/Wizardry/BattleSys/Source/BattleCalcReal.c:412`). It runs `BattleCalcReal_ModifyBattleStatusSkills` (`BattleCalcReal.c:14`) and `BattleCalcReal_ComputSkills` (`:140`), both under `#if CHAX`. Use this pass for skills that compare the two sides' final numbers (e.g. `SID_HeavyBlade`, `SID_Puissance`, `SID_CatchingUp` in `BattleCalcReal.c:14-90`).

### Which tester, and attacker vs defender

- `BattleFastSkillTester(bu, sid)` (`include/kernel/skill-system.h:185`). Only valid for `bu == &gBattleActor` or `&gBattleTarget`. Reads a bitmask built from the battle skill lists (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:538`). Used for self checks in the pre-battle code (`PreBattleCalc.c:227`, `:2423`, `:2430`, `:2444`) and in the WTA code (`WeaponTriangle.c:11-40`). Honours Nihil: `DisableUnitSkilLList` runs before the fast list is rebuilt (`SkillList.c:645-651`).
- `SkillTester(unit, sid)` (`skill-system.h:183`). Takes a map `struct Unit*`, not a battle unit. Used for aura holders (`PreBattleCalc.c:2134-2420`), `SkillTester(actor, SID_BanditBane)` / `SID_Protean` (`CheckEffective.c:228`, `:244`). Asm in `Kernel/Wizardry/SkillSys/kernel/JudgeSkill_NORMAL.s` (`JudgeSkill_ALL_EQUIPPABLE.s` when `CONFIG_TURN_ON_ALL_SKILLS` is set, `Kernel/Wizardry/SkillSys/SkillSys.event:4-8`).
- `SkillListTester(unit, sid)`. Uses `GetUnitSkillList(unit)`, which returns the battle list for `&gBattleActor.unit` / `&gBattleTarget.unit` (`SkillList.c:485`). Used for `SID_Nullify`, `SID_Slayer`, `SID_Skybreaker`, `SID_BeastShield`, `SID_WingedShield` (`CheckEffective.c:44`, `:214`, `:221`).
- Attacker vs defender. Inside the list functions, `attacker` is the unit being computed and `defender` is its opponent. `attacker == &gBattleActor` means "this unit initiates" (Blow). `attacker == &gBattleTarget` means "this unit is attacked" (Stance). The weapon-triangle code does the same: `status = (attacker == &gBattleActor) ? &gWtaStatus_act : &gWtaStatus_tar` (`WeaponTriangle.c:251-256`).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c` | `PreBattleCalcAttackerSkills` (flat, Blow, Stance cases, `:597`), `PreBattleCalcAuraEffect` (`:2105`), base formulas (`:78-265`), `ComputeBattleUnitStats` loop (`:2534`) |
| `Kernel/Wizardry/BattleSys/data.event` | `PreBattleCalcFuncs` list (`:4-15`). Four `KernelPad1` slots at `:14` |
| `Kernel/Preload/KernelReloc.event` | `gpPreBattleCalcFuncs` pointer (`:43`) |
| `Kernel/Wizardry/BattleSys/Source/BattleGenerate.c` | Call sites (`:72`, `:75-76`, `:78-79`) |
| `Kernel/Wizardry/BattleSys/Source/WeaponTriangle.c` | WTA handlers (`WtaHandler_Skill` at `:11`, `WtaHandler` at `:166`), `PreBattleGenerate_SetupWtaStatus` (`:195`), `PreBattleCalcWeaponTriangle` (`:251`) |
| `Data/BattleSys/Source/WeaponTriangle.c` | `gWeaponTriangleConfs` table; sentinel `{0}` at `:166` |
| `include/kernel/battle-system.h` | `struct WeaponTriangleConf` (`:19`), `gpWeaponTriangleConfs` (`:38`) |
| `Kernel/Wizardry/BattleSys/Source/CheckEffective.c` | Effectiveness predicates: `IsItemEffectiveAgainst` (`:52`), `IsBattleUnitEffectiveAgainst` (`:116`), `IsUnitEffectiveAgainst` (`:177`); null-effective (`:42`, `:109`) |
| `Kernel/Wizardry/BattleSys/Source/BattleCalcReal.c` | Second pass: `ComputeBattleUnitEffectiveStats` (`:412`) |
| `Kernel/Wizardry/KernelLib/Source/ranges.c` | `gVecs_3x3` (`:18`), `gRange1_In3x3` (`:63`), `gRange2_In3x3` (`:73`); count `ARRAY_COUNT_RANGE3x3` in `include/kernel/kernel-lib.h:342` |
| `Kernel/Wizardry/SkillSys/kernel/SkillList.c` | Battle skill lists (`GetUnitSkillList` `:485`), `SetupBattleSkillFastList` (`:538`), `UnitToBattle_SetupSkillList` (`:624`) |
| `Kernel/Wizardry/SkillSys/kernel/SkillList_.s` | `BattleFastSkillTester` asm (`:7`) |
| `Kernel/Wizardry/UnitHooks/data.event` | `gExternalUnitToBattleHook` (`:18`); `UnitToBattle_SetupSkillList` must stay first (`:20`) |
| `Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c` | `InitBattleUnit` (`:236`) runs the UnitToBattle hook list |
| `Data/SkillSys/Source/SkillExtraInfo.c` | `SKILL_EFF0/1` values per SID (e.g. `:475` BlowDeath, `:787` Crit) |
| `include/kernel/skill-system.h` | `SKILL_EFF0` / `SKILL_EFF1` (`:383-384`); `priv` is `s8 priv[10]` (`:380`) |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Choose the sub-kind from "What it is". Put the code where that sub-kind lives:
   - Flat, Blow, Stance: a new `case` in the `switch` in `PreBattleCalcAttackerSkills` (`PreBattleCalc.c:597`), guarded by `#if defined(SID_x) && (COMMON_SKILL_VALID(SID_x))`. Use the `attacker == &gBattleActor` or `&gBattleTarget` gate for Blow/Stance.
   - Aura: a new block in the ally branch (`PreBattleCalc.c:2134`) or enemy branch (`:2347`) of `PreBattleCalcAuraEffect`.
   - Weapon triangle: a new entry in `gWeaponTriangleConfs` before `{0}` (`Data/BattleSys/Source/WeaponTriangle.c:166`).
   - Effectiveness: an amplifier in `ComputeBattleUnitAttack` (`PreBattleCalc.c:103`), or a predicate in `IsUnitEffectiveAgainst` (`CheckEffective.c:177`).
3. If the effect reads `SKILL_EFF0/1`, add `[SID_x] = {{ n }}` to `Data/SkillSys/Source/SkillExtraInfo.c`, guarded by the same pattern.
4. Only if you need a new pass, add a function to `PreBattleCalcFuncs` by replacing a `KernelPad1` slot (`Kernel/Wizardry/BattleSys/data.event:14`). Place it before `PreBattleCalcEnd` (`:13`) if you want its result clamped to 0.

## Template

No new includes are needed for any sub-kind. `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c` already includes `skill-system.h` and `constants/skills.h`. `Kernel/Wizardry/BattleSys/Source/CheckEffective.c` includes `constants/skills.h`. `Data/BattleSys/Source/WeaponTriangle.c` includes `constants/skills.h`. The registration for every sub-kind is the code block itself, plus the SID in the common steps.

### A. Flat, Blow, Stance (switch case)

Adapted from `SID_BlowDeath` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:653-657`) and `SID_StanceBracing` (`:728-733`). Insert inside `switch (list->sid[_skill_list_cnt])` in `PreBattleCalcAttackerSkills`:

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
		case SID_Example:
			/* Blow: only when this unit initiates. Stance: use `attacker == &gBattleTarget`.
			   Flat: drop the gate. */
			if (attacker == &gBattleActor)
				attacker->battleAttack += SKILL_EFF0(SID_Example);

			break;
#endif
```

### B. Aura (range / holder skill)

Adapted from `SID_Hex` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:2349-2351`). Goes inside the enemy-holder `else` branch of `PreBattleCalcAuraEffect`. `unit` is the holder, a `struct Unit*`, so use `SkillTester`:

```c
			if (gRange1_In3x3[i] == 1) {
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
				if (SkillTester(unit, SID_Example))
					attacker->battleAvoidRate -= SKILL_EFF0(SID_Example);
#endif
			}
```

For an allied holder, put the same pattern in the `if (AreUnitsAllied(...))` branch (`PreBattleCalc.c:2134`). Use `gRange1_In3x3` / `gRange2_In3x3` for distance, or no test for any cell in the 3x3.

### C. Weapon triangle (table entry)

Adapted from `SID_SwordBreaker` (`Data/BattleSys/Source/WeaponTriangle.c:29-38`). Insert before the `{0}` sentinel:

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	{
		.wtype_a = ITYPE_LANCE,   /* attacker weapon type */
		.wtype_b = ITYPE_SWORD,   /* defender weapon type; must differ from wtype_a */
		.sid = SID_Example,
		.is_buff = true,
		.bonus_hit = 20,
		.bonus_avoid = 20
	},
#endif
```

### D. Effectiveness

Amplifier, adapted from `SID_Resourceful` (`Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:103-105`). It only applies inside `if (effective) {`:

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
		if (BattleFastSkillTester(attacker, SID_Example))
			effective_amplifier += SKILL_EFF0(SID_Example);
#endif
```

Predicate, adapted from `SID_Slayer` (`Kernel/Wizardry/BattleSys/Source/CheckEffective.c:213-218`). Insert in `IsUnitEffectiveAgainst` before the `check_null_effective:` label. Routing through `goto check_null_effective` keeps Nullify working:

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	if (SkillListTester(actor, SID_Example)) {
		if (CheckClassBeast(jid_target) && !CheckBeastNullEffective(target))
			goto check_null_effective;
	}
#endif
```

## Worked examples

- `SID_Crit` (`Skills_BattleStatus.txt`): `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:856-859`. Unconditional flat bonus, `SKILL_EFF0` from `SkillExtraInfo.c:787`. Runs on both sides (no gate).
- `SID_BlowDeath` (`Skills_Blow.txt`): `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:653-657`. Blow gated by `attacker == &gBattleActor` plus a magic check on the attacker.
- `SID_StanceBracing` (`Skills_Stance.txt`): `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:728-733`. Stance gated by `attacker == &gBattleTarget`, adds to defense.
- `SID_SwordBreaker` (`Skills_WeaponTriangle.txt`): `Data/BattleSys/Source/WeaponTriangle.c:29-38`. Table entry; applied by `WtaHandler_Skill` (`Kernel/Wizardry/BattleSys/Source/WeaponTriangle.c:11`) through `BattleFastSkillTester`.

Not in the four examples above, but in scope: `SID_Hex` and `SID_Bond` (aura, `PreBattleCalc.c:2349`, `:2283`), `SID_Resourceful` (effectiveness amplifier, `:103`), `SID_Slayer` (effectiveness predicate, `CheckEffective.c:213`).

## Gotchas

- `BattleFastSkillTester` is only valid for `&gBattleActor` and `&gBattleTarget`. The asm indexes by `bu - &gBattleActor` (`Kernel/Wizardry/SkillSys/kernel/SkillList_.s`), and `sizeof(struct BattleUnit)` is `0x80` (`Tools/FE-CLib-Mokha/include/bmbattle.h:20-79`). Any other pointer reads unrelated memory. The C version that returned `false` for other pointers is disabled (`SkillList.c:567`, `#if 0`).
- The fast list is only filled once the target is set up. `SetupBattleSkillFastList` runs from `UnitToBattle_SetupSkillList` when `bu == &gBattleTarget` (`SkillList.c:637-651`). `UnitToBattle_SetupSkillList` must stay first in `gExternalUnitToBattleHook` (`Kernel/Wizardry/UnitHooks/data.event:20`).
- `PreBattleCalcAttackerSkills` iterates the battle list (`GetUnitSkillList(&attacker->unit)`, `PreBattleCalc.c:612`), not the map unit's skills. A skill must be in the battle list to fire from a `case`.
- Unconditional cases run for both sides. `ComputeBattleUnitStats` is called twice (`BattleGenerate.c:75-76`). Gate with `attacker == &gBattleActor` or `&gBattleTarget` for one-sided effects.
- Arena: `PreBattleCalcDefenderSkills` (`PreBattleCalc.c:420`) and `PreBattleCalcAttackerSkills` (`:609`) return early when `BATTLE_CONFIG_ARENA | BATTLE_CONFIG_BIT2` is set. `PreBattleCalcAuraEffect` returns early on `BATTLE_CONFIG_ARENA` (`:2120`). New skills in these functions do nothing in arena.
- `SKILL_EFF0/1` are `s8` (`include/kernel/skill-system.h:380`). Values outside -128..127 cannot be stored.
- `SKILL_EFF0(SID_Resourceful)` and `SKILL_EFF0(SID_SolidRock)` only apply when the attack is effective (`PreBattleCalc.c:102-110`).
- Effectiveness and nullification. `IsUnitEffectiveAgainst` sends skill predicates to `check_null_effective` (`CheckEffective.c:250-252`), so Nullify / BeastShield / WingedShield apply. `SID_Protean` (unconditional) and `SID_WyvernCrash` (conditional) `return true` before that label (`CheckEffective.c:243-246`), so they bypass null-effective.
- WTA table: the loop stops when `wtype_a == wtype_b` (`WeaponTriangle.c:14`). The sentinel `{0}` (`Data/BattleSys/Source/WeaponTriangle.c:166`) matches `ITYPE_SWORD = 0` (`Tools/FE-CLib-Mokha/include/bmitem.h:88`). A real entry with equal types ends the table early.
- WTA handlers all run, because `ENABLE_ALL_WTA_HANDLER` is `1` (`WeaponTriangle.c:9`, `:166-180`). A skill entry does not suppress the weapon or vanilla triangle bonus.
- Aura cost. The aura loop calls `SkillTester` on up to 24 cells per computed unit (`ranges.c:18`, `kernel-lib.h:342`). `Documentation/CONTRIBUTING.md` lists `SkillTester` as performance sensitive.
- Aura holders use `SkillTester` (map unit), not `BattleFastSkillTester`. The self checks at the end of the function use `BattleFastSkillTester` (`PreBattleCalc.c:2422-2447`). Mixing these up is a type error, because `BattleFastSkillTester` takes `struct BattleUnit*` (`skill-system.h:185`).
- Unverified: `SkillTester` reads the map unit, not the battle list, so Nihil (which clears only the battle list, `SkillList.c:645`) would not suppress an aura holder. Read from `JudgeSkill_NORMAL.s`; not tested in game.
- Unverified: `Symbiosis` appends rescuee skills into the battle list in place, with no bounds or duplicate check (`PreBattleCalc.c:425-431`, `:614-620`). Not checked whether repeated `BattleGenerate` calls accumulate entries.
- A new list function placed after `PreBattleCalcEnd` (`data.event:13`) is not clamped to 0 (`PreBattleCalc.c:365-384`).
- Conditions that compare both sides' final stats belong in the second pass (`BattleCalcReal.c:412`), not the pre-battle list, because the pre-battle list runs one side at a time.
