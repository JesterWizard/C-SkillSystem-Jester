# Skill types: where to look and how to template each

Start every skill with [Common.md](Common.md) (enum, text, SkillInfo, price and capacity, icon, guards). Then open the one type doc that matches what the skill does (table below). Templates in the type docs use `SID_Example` / `Example` as placeholder names. Paths in this file are relative to the repo root.

## Pick a type

| If the skill... | Type doc | Text file | Hook / main code |
|---|---|---|---|
| is a passive bonus to one unit's HP, POW, MAG, SKL, SPD, LCK, DEF, RES, MOV or CON | [StatusGetter.md](StatusGetter.md) | `Contents/Texts/Source/texts/Skills_StatusGetter.txt` | `g<Stat>Getters` lists in `Kernel/Wizardry/UnitStatusGetter/data.event` (`gPowGetters` at `:10`); `Kernel/Wizardry/UnitStatusGetter/source/` |
| changes growth rates, EXP gained, or the level-up roll | [LevelUp.md](LevelUp.md) | `Contents/Texts/Source/texts/Skills_Lvup.txt` | no list; direct calls from `Kernel/Wizardry/Lvup/Source/` and `Kernel/Wizardry/BattleSys/Source/BattleExp.c` |
| changes a battle unit's attack, defense, hit, avoid, crit or speed before the first strike; sets the weapon-triangle bonus, effectiveness, or an aura from nearby units | [BattleCalc.md](BattleCalc.md) | `Skills_BattleStatus.txt`, `Skills_Blow.txt`, `Skills_Stance.txt`, `Skills_WeaponTriangle.txt`, `Skills_BattleEffective.txt`, `Skills_RangeDebuff.txt` (all in `Contents/Texts/Source/texts/`) | `PreBattleCalcFuncs` list in `Kernel/Wizardry/BattleSys/data.event:4`; `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c` |
| rolls during a strike to change hit, crit, damage or survival, or is Nihil | [BattleProc.md](BattleProc.md) | `Skills_BattleAct.txt`, `Skills_BattleNihil.txt` (in `Contents/Texts/Source/texts/`) | `BattleDamageCalcFuncs` list in `Kernel/Wizardry/BattleSys/data.event:17`; `CheckBattleSkillActivate` in `Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c` |
| gives a follow-up, moves a strike earlier (Vantage, Desperation), or changes the number of strikes | [BattleOrder.md](BattleOrder.md) | `Contents/Texts/Source/texts/Skills_BattleOrder.txt` | `BattleUnwind` (`Kernel/Wizardry/BattleSys/Source/LynJump.event:101`); `Kernel/Wizardry/BattleSys/Source/BattleOrder.c` |
| adds a command to the Skills submenu of the unit action menu | [MenuSkills.md](MenuSkills.md) | `Contents/Texts/Source/texts/Skills_Menu.txt` | `Kernel/Wizardry/SkillEffects/MenuSkills/`; `Kernel/Data/SkillSys/Data/SkillMenuInfo.c`, `SkillActionInfo.c` |
| runs after a unit's action resolves (combat, staff, item, wait, rescue) | [PostAction.md](PostAction.md) | `Contents/Texts/Source/texts/Skills_PostAction.txt` | `gPostActionFuncs` (`Kernel/Wizardry/PostActionHook/data.event:3`) |
| runs when a faction's phase starts | [PrePhase.md](PrePhase.md) | `Contents/Texts/Source/texts/Skills_PrePhase.txt` | `gPrePhaseFuncs` (`Kernel/Wizardry/PrePhaseHook/data.event:3`) |
| runs when a faction's turn ends, or (chapter-init section) at chapter start | [EndTurn.md](EndTurn.md) | `Skills_EndTurn.txt`; chapter-init skills use `Skills_<Category>.txt` (varies) | `gEndTurnFuncs` (`Kernel/Wizardry/EndTurnHook/data.event:2`); `ChapterInitHooks` (`Kernel/Wizardry/ChapterInitHook/data.event`) |
| changes terrain move costs, ignores or projects barriers, or adds movement to allies | [Movement.md](Movement.md) | `Contents/Texts/Source/texts/Skills_Movement.txt` | `SetWorkingMoveCosts` (`Kernel/Wizardry/WeaponRange/Source/MiscHooks.c:579`); `PreGenerateMovementMap` (`Kernel/Wizardry/Movement/Source/Movement.c:11`) |
| changes the min or max range of a weapon, tome or staff, or which units a staff may target | [WeaponRange.md](WeaponRange.md) | `Contents/Texts/Source/texts/Skills_WeaponRange.txt` | `gWeaponRangeGetters` (`Kernel/Wizardry/WeaponRange/data.event:2`); `GetItemMinRangeRework` (`Kernel/Wizardry/WeaponRange/Source/RangeGetter.c:16`) |
| changes what an AI unit targets, how it moves, restocks its weapon, or adds an AI-only action | [AiSkills.md](AiSkills.md) | `Skills_AI.txt` (Teleportation's name is in `Skills_Menu.txt`) | `gPreAi2DecideHook` (`Kernel/Wizardry/AiHack/AiDecide/data.event:6`); `Kernel/Wizardry/SkillEffects/AiSkills/` |
| writes a stat debuff, buff or unit status (Panic, Chill, Seal, Hone and similar) | [Debuff.md](Debuff.md) | `Contents/Texts/Source/texts/Skills_Debuff.txt` | `gPrePhaseFuncs`, `gPostActionFuncs`, stat getter lists; engine in `Kernel/Wizardry/DebuffRework/` |
| is a combat art (a per-attack option, `SID_COMBAT_*`) | [CombatArt.md](CombatArt.md) | `Contents/Texts/Source/texts/Skills_CombatArt.txt` | `Kernel/Wizardry/CombatArt/` (engine); `Data/CombatArt/Data/` (tables) |
| changes shop price, heal amount, item use, or activation rate | [Misc.md](Misc.md) | `Contents/Texts/Source/texts/Skills_Misc.txt` | `gHealAmountGetters` (`Kernel/Wizardry/ItemSys/ItemSys.event`); `Kernel/Wizardry/SkillEffects/Misc/`, `ShopSkills/` |

Common.md is not a type: it applies to every skill, so it has no row.

### Text file to type doc

Every `Contents/Texts/Source/texts/Skills_*.txt` file (21 files, all included by `Contents/Texts/Source/kernel.txt`), mapped to the type doc that owns its skills.

| Text file | Type doc |
|---|---|
| `Skills_AI.txt` | [AiSkills.md](AiSkills.md) |
| `Skills_BattleAct.txt` | [BattleProc.md](BattleProc.md) |
| `Skills_BattleEffective.txt` | [BattleCalc.md](BattleCalc.md) |
| `Skills_BattleNihil.txt` | [BattleProc.md](BattleProc.md) |
| `Skills_BattleOrder.txt` | [BattleOrder.md](BattleOrder.md) |
| `Skills_BattleStatus.txt` | [BattleCalc.md](BattleCalc.md) |
| `Skills_Blow.txt` | [BattleCalc.md](BattleCalc.md) |
| `Skills_CombatArt.txt` | [CombatArt.md](CombatArt.md) |
| `Skills_Debuff.txt` | [Debuff.md](Debuff.md) |
| `Skills_EndTurn.txt` | [EndTurn.md](EndTurn.md) |
| `Skills_Lvup.txt` | [LevelUp.md](LevelUp.md) |
| `Skills_Menu.txt` | [MenuSkills.md](MenuSkills.md) |
| `Skills_Misc.txt` | [Misc.md](Misc.md) |
| `Skills_Movement.txt` | [Movement.md](Movement.md) |
| `Skills_PostAction.txt` | [PostAction.md](PostAction.md) |
| `Skills_PrePhase.txt` | [PrePhase.md](PrePhase.md) |
| `Skills_RangeDebuff.txt` | [BattleCalc.md](BattleCalc.md) |
| `Skills_Stance.txt` | [BattleCalc.md](BattleCalc.md) |
| `Skills_StatusGetter.txt` | [StatusGetter.md](StatusGetter.md) |
| `Skills_WeaponRange.txt` | [WeaponRange.md](WeaponRange.md) |
| `Skills_WeaponTriangle.txt` | [BattleCalc.md](BattleCalc.md) |

The file name is a convention only. The build includes every file, so the mapping tells you where a skill is documented, not what the build reads.

## Finding an existing skill

Run this from the repo root:

```bash
grep -rn --exclude-dir=.claude "SID_Example" Kernel Data include Contents
```

`--exclude-dir=.claude` is required. `.claude/worktrees/` holds stale copies of the tree, and their hits are not the live code.

Shell caveat: in some shells `grep` is a wrapper around ugrep that honours `.gitignore`. Gitignored generated files, such as `include/constants/skills.h`, are then skipped. Use `/bin/grep -rn ...` to include them.

What the hit locations mean, using `SID_Pivot` as the example (its real hits, checked on this tree):

| Hit location | Meaning |
|---|---|
| `include/constants/skills-equip.enum.txt` | The definition. The line's position among active lines is the ID (`SID_Pivot` = line 35 = `0x022`). This is the only enum the build reads. |
| `include/constants/skills-class.enum.txt`, `skills-person.enum.txt`, `skills-item.enum.txt` | `// SID_x` placeholders. No build effect. |
| `include/constants/skills.h`, `include/constants/skills.event` | Generated from the enum by `make`. Never edit. |
| `Data/SkillSys/Source/SkillInfo.c` | The name, description, icon, price and capacity entry. |
| `Data/SkillSys/Source/SkillAnimInfo.c`, `SkillExtraInfo.c` | Animation priority and animation, and the `SKILL_EFF0`-`9` values. |
| `Data/SkillSys/Source/SkillTable-*.c` | Fixed grants (person, job, item, weapon, forge, generic, wrank). |
| `Kernel/Data/SkillSys/Data/SkillMenuInfo.c`, `SkillActionInfo.c` | Menu-skill registration (menu skills only). |
| `Kernel/Wizardry/SkillEffects/<Type>/<Name>.c` | The effect code. The `#if defined(SID_x)` guard is here. |
| A `data.event` file, `POIN <Func>` line | The skill's function is registered in a hook list. |
| `Kernel/Wizardry/SkillSys/PrepSkill/SkillSynthRecipes.event` | Synthesis recipes that produce or consume the skill. |

Two things the SID grep does not find:

- The name and description text. Grep for `MSG_SKILL_Example_` (no `SID_` prefix) in `Contents/Texts/Source/texts/`. The price and capacity macros, `MSG_SKILL_Example_PRICE` and `MSG_SKILL_Example_CAPACITY`, are in `include/kernel/Skills_Price.h` and `include/kernel/Skills_Capacity.h`.
- The `.lyn.event` include. Grep for `<Name>.lyn.event` in `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event` and the other installers. Generated `.lyn.event` files hold compiled code, so they do not contain the SID name.

## Glossary

Definitions are cited from source. `path:line` is where the thing is defined or declared.

- **SID** — skill ID, written `SID_Example`. A `u16` in `0x001`-`0x3FF`. The value is the active line's position in `include/constants/skills-equip.enum.txt:2` (`SID_Dismount` = `0x001`). Generated into `include/constants/skills.h` by `Tools/scripts/enum2h.py` (`makefile:404`). IDs are positional, so new skills are appended at the end (Common.md).
- **`COMMON_SKILL_VALID(sid)`** — `0x000 < sid < 0x400`. The guard every skill-specific block uses: `include/kernel/skill-system.h:90`.
- **`EQUIP_SKILL_VALID(sid)`** — equippable-skill range check. `0x000 < sid < 0x400` when `CONFIG_TURN_ON_ALL_SKILLS` is defined (`include/kernel/skill-system.h:85`), `< 0x0FF` otherwise (`:87`).
- **`SKILL_EFF0`** … **`SKILL_EFF9`** — per-SID numeric parameters, `gpSkillExtraInfo[sid].priv[n]` (`include/kernel/skill-system.h:383-392`). Values come from rows of `gSkillExtraInfo` (`Data/SkillSys/Source/SkillExtraInfo.c:7`, e.g. `[SID_HpBonus] = {{ 5 }}`). Example use: `SKILL_EFF0(SID_Aptitude)` at `Kernel/Wizardry/Lvup/Source/GrowthGetter.c:22`. `SKILL_EFF1` is used at `Kernel/Wizardry/Lvup/Source/Levelup.c:248`.
- **`SkillExtraInfo`** — `struct SkillExtraInfo { s8 priv[10]; }` (`include/kernel/skill-system.h:379-381`), the element type behind the SKILL_EFF macros. Pointer `gpSkillExtraInfo` at `:382`.
- **`SkillInfo`** — `struct SkillInfo { icon; name, desc, price, capacity; iconPal; }` (`include/kernel/skill-system.h:125-129`). Table `gSkillInfos` at `Data/SkillSys/Source/SkillInfo.c:9`. Read-only pointer `gpSkillInfos` at `include/kernel/skill-system.h:131`.
- **`SkillTester(unit, sid)`** — live check of a unit's skills (`include/kernel/skill-system.h:183`). Implemented in ARM (`Kernel/Wizardry/SkillSys/kernel/JudgeSkill_NORMAL.s:6` or `JudgeSkill_ALL_EQUIPPABLE.s:6`, chosen by `Kernel/Wizardry/SkillSys/SkillSys.event:4-7`). Reads the RAM skill slots and the person, job and item tables. It does not see weapon, forge or shield skills. Hot path (CONTRIBUTING.md).
- **`SkillListTester(unit, sid)`** — tester over the cached list from `GetUnitSkillList` (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:508`, declared `include/kernel/skill-system.h:184`). Sees weapon, forge, shield and rescue-derived skills. `ResetSkillLists()` (`SkillList.c:533`) forces a rebuild.
- **`BattleFastSkillTester(bu, sid)`** — ARM routine (`Kernel/Wizardry/SkillSys/kernel/SkillList_.s:6`, declared `include/kernel/skill-system.h:185`) over the bit lists that `SetupBattleSkillFastList` (`SkillList.c:538`) fills for the battle. Valid only for `gBattleActor` and `gBattleTarget` during the battle calc (Common.md).
- **`SkillTesterPlus(unit, sid)`** — `SkillTester`, plus: when the unit has `SID_Upgrade`, it also returns true if the unit has the base skill that `gSkillUpgradePlusLookup[sid]` names (plus SID to base SID, `Kernel/Wizardry/SkillSys/kernel/SkillList.c:29-32`). Defined at `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c:3478`; prototype `include/jester_headers/custom-functions.h:156`. Used by the Gift skills (`Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c:104`).
- **Dynamic / equippable skill** — a skill in one of the unit's RAM skill slots, written with `SET_SKILL` and read with `GET_SKILL` (`include/kernel/skill-system.h:54-60`). With `CONFIG_TURN_ON_ALL_SKILLS` the unit has 5 slots (`:21`) of 10 bits each, packed into `unit->supports`. Otherwise 7 slots (`:23`).
- **Fixed skill** — a skill granted by a table, not by a slot. Tables are keyed by person, job, item, weapon and forge: `gConstSkillTable_Person`, `gConstSkillTable_Job`, `gConstSkillTable_Item`, `gConstSkillTable_Weapon`, `gConstSkillTable_Forge` (pointers at `include/kernel/skill-system.h:119-123`; rows in `Data/SkillSys/Source/SkillTable-*.c`). Read by `GenerateSkillListExt` (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:304`). The skill is held while its source is present (for example, while the item is in inventory).
- **`gBattleActor` / `gBattleTarget`** — the two `struct BattleUnit` globals for the current battle (`Tools/FE-CLib-Mokha/include/bmbattle.h:156-157`). The actor is the unit initiating the strike, the target is the other side.
- **`gActiveUnit`** — `struct Unit *` for the unit being acted with on the map (`Tools/FE-CLib-Mokha/include/variables.h:131`). Example: `Kernel/Wizardry/SkillSys/kernel/SkillEventCmd.c:40`.
- **`gActionData`** — per-action scratch struct (`Tools/FE-CLib-Mokha/include/bmmind.h:46`; struct at `:9`). For skills, `unk08` (`bmmind.h:15`) is overloaded. It holds a SID while the action runs, for example `== SID_LethalityPlus` at `Kernel/Wizardry/BattleSys/Source/BattleDamage.c:363`, `== SID_Thrust` at `Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c:296`, and `SID_Pivot` at `Kernel/Wizardry/SkillEffects/MenuSkills/Pivot.c:49`. It also holds a RAM slot index in the skill-remove menu (`Kernel/Wizardry/SkillSys/SkillScroll/Source/RemoveSkillMenu.c:307`). It is set to `-1` by `Kernel/Wizardry/SkillSys/ArmsScroll/ArmsScroll.c:37`.
- **lyn / `.lyn.event`** — `lyn` turns a compiled `.o` into an Event Assembler file (`LYN := $(EA_DIR)/Tools/lyn`, `makefile:96`; rule `%.lyn.event: %.c`, `makefile:211-217`). The `.lyn.event` is committed next to its `.c` and `#include`d by an installer.
- **`LYN_REPLACE_CHECK(Func)`** — macro at `include/common-chax.h:26`. Takes the function's address in a static, so the function is always emitted. Put it before a function that replaces a vanilla one. Pairs with a `LynJump.event` (CONTRIBUTING.md, Mandatory Requirements).
- **`LynJump.event`** — per-directory EA file (`PUSH`, `ORG`, `ALIGN 4`, `WORD`, `POIN`, `POP`) that redirects a vanilla ROM call site to a C function. Example: `Data/SkillTree/Source/LynJump.event`. `Kernel/Wizardry/BattleSys/Source/LynJump.event:101` redirects `BattleUnwind`. The `$(CHAX_NUPS)` rule runs the lyn-jump detector (`makefile:457-459`).
- **Hook list (`POIN` list in `data.event`)** — a zero-terminated list of function pointers the engine walks. Example: `gPrePhaseFuncs:` at `Kernel/Wizardry/PrePhaseHook/data.event:3`. Registering a skill means adding a `POIN <Func>` line. The walker is `HookProc_Main` (`Kernel/Wizardry/KernelLib/Source/hook-proc.c:52`).
- **`MiscSkillEffects.event`** — `Kernel/Wizardry/SkillEffects/MiscSkillEffects.event`. The installer that `#include`s most skill `.lyn.event` files (Pivot is at line 67).
- **`PreBattleCalc` list** — `PreBattleCalcFuncs:` at `Kernel/Wizardry/BattleSys/data.event:4`. The pointer is `gpPreBattleCalcFuncs` (`Kernel/Preload/KernelReloc.event:43`). It is walked at `Kernel/Wizardry/BattleSys/Source/PreBattleCalc.c:2541`.
- **Efx skill anim** — `RegisterActorEfxSkill(round, sid)` and `RegisterTargetEfxSkill(round, sid)` store the SID for one battle hit round (`include/kernel/efx-skill.h:23-24`; `Kernel/Wizardry/SkillSys/EfxSkill/Utils.c:22` and `:33`). The per-SID animation tables are in `Data/SkillSys/Source/SkillAnimInfo.c`.
- **Map skill anim** — `NewMuSkillAnimOnActiveUnit(sid, callback1, callback2)` (`Kernel/Wizardry/SkillSys/SkillMapAnim/MapAnimSkillMini.c:207`) plays the skill's map animation on the active unit. `NewMuSkillAnimOnActiveUnitWithDeamon` (`:227`) takes a parent proc; used by `Kernel/Wizardry/SkillEffects/PrePhaseSkills/ChillSkills.c:70`.
- **`CONFIG_UNIT_ACTION_EXPA_ExecSkill`** — unit-action ID `0x23` (`include/configs/configs.h:16`) for running a menu skill. `Kernel/Wizardry/UnitAction/Source/UnitActionData.c:34` maps it to `ActionExecSkill`.
- **`CheckBattleSkillActivate(actor, target, sid, rate)`** — the roll for battle procs (`Kernel/Wizardry/SkillEffects/Misc/SkillActRate.c:77`, declared `include/kernel/skill-system.h:189`). Returns false during the forecast (`BATTLE_CONFIG_SIMULATE`, `SkillActRate.c:79-80`) and when the target has Foresight (`:82-85`). Otherwise it rolls `rate` through `CheckSkillActivateExt`.
- **Nihil** — `SID_Nihil`, a battle skill with text in `Skills_BattleNihil.txt`. When one side has it, the other side's battle skill list is cleared (`Kernel/Wizardry/SkillSys/kernel/SkillList.c:640-646`, `DisableUnitSkilLList` at `:526`; the misspelling is in the source).
- **`PLAYSTEXPA_BIT_*`** — bit IDs for one-shot flags in the play-state bitfield `gPlayStExpa` (enum `play_st_expa_bits` at `include/kernel/playst-expa.h:5`; `PLAYSTEXPA_BIT_Thrust_Used` at `:24`; `gPlayStExpa` at `:45`). Tested and set with `PlayStExpa_CheckBit` / `PlayStExpa_SetBit` (`:47-48`). Used at `Kernel/Wizardry/UnitHooks/Source/BattleUnitHook.c:296-297`. When the flag is reset is not traced here (unverified).
- **Stat debuff (`SetUnitStatDebuff`)** — sets one bit in the unit's stat-debuff bitmask. Each bit adds fixed stat deltas from `Data/Debuff/StatDebuffInfo.c`. `SetUnitStatDebuff` is at `Kernel/Wizardry/DebuffRework/StatDebuff/StatDebuff.c:125` and `ClearUnitStatDebuff` at `:139`. The other layer, one unit-status slot with a duration, is `SetUnitStatusExt` (`Kernel/Wizardry/DebuffRework/Source/MiscHooks.c:60`).
- **Designer config (`gpKernelDesignerConfig`)** — runtime toggles read through a pointer. Declared at `include/kernel/kernel-lib.h:232`, with the struct spelled `KernelDesigerConfig` (keep the typo when you write it). Pointer defined at `Preload/Reloc.event:45`, value `gKernelDesigerConfig` at `Data/DesignerConfig/designer-config.c:5` (for example `.menu_skill_ai_use` at `:99`). The battle variant is `Data/DesignerConfig/battle-designer-config.c`.
- **`CHAX`** — the C-first framework this buildfile is built on (`Documentation/CHAX.md`). The macro `#define CHAX 1` is at `include/common-chax.h:12`. Code guards on it with `#if CHAX` (example: `Kernel/Wizardry/UnitHooks/Source/ClearUnit.c:12`). The same header defines `NOINLINE` (`:24`) and `LYN_REPLACE_CHECK` (`:26`).

## Known stale docs

Do not copy from these without checking the paths against the tree.

- `Documentation/Features/SkillSys.md` — partly wrong.
  - Text path: it says texts are in `Contents/Texts/Source/kernel.txt` (`:111`). That file only `#include`s the per-category `Contents/Texts/Source/texts/Skills_*.txt` files (`kernel.txt:13-33`).
  - SkillInfo path: it omits the `Source/` directory (`:115`). The file is `Data/SkillSys/Source/SkillInfo.c`.
  - Relative links: they are one `../` short. From `Documentation/Features/` they resolve into `Documentation/`; they need `../../` to reach the repo root.
  - Section 6.3 (battle order, `:146`) is out of date. See [BattleOrder.md](BattleOrder.md).
  - Its statement that `0x300-0x3FF` is item-only does not match the enum. See [Common.md](Common.md), "Skill ID ranges".
- `Documentation/Features/MenuSkillAI.md` — the table at `:133` says the AI gate is `MenuSkills_Usability` in `SkillMenu.c`. The AI path does not use it. The AI selects a menu skill by calling the per-skill `isAvailable` (`Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c:121`), and only when `gpKernelDesignerConfig->menu_skill_ai_use` is set (`MiscAiSkills.c:103`). See [AiSkills.md](AiSkills.md) and [MenuSkills.md](MenuSkills.md).
