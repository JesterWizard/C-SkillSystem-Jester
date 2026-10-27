# AI skills

> Text file: `Contents/Texts/Source/texts/Skills_AI.txt` (Teleportation's name is in `Contents/Texts/Source/texts/Skills_Menu.txt`)  ·  Code: `Kernel/Wizardry/SkillEffects/AiSkills/`, `Kernel/Wizardry/AI/`, `Kernel/Wizardry/AiHack/`  ·  Hook: `gPreAi2DecideHook` (`Kernel/Wizardry/AiHack/AiDecide/data.event:6`) for Teleportation; `DecideScriptA` / `DecideScriptB` / `DecideHealOrEscape` / `AiTryDoStaff` for the menu-skill fallback; `AiAttemptOffensiveAction` and `AiGetDamageDealtCombatScoreComponent` for targeting and scoring

## What it is

Skills that change AI decisions rather than the unit's own stats or map. They change target choice (Shade, Shade+, Provoke, Lightning Rod, Guardian), force an AI unit into independent movement (Rampage), restock an AI unit's weapon (Rearm), or add an AI-only action (Teleportation). The generic AI menu-skill fallback (any menu skill the AI unit owns that passes its usability check) is covered in [MenuSkillAI.md](../../Documentation/Features/MenuSkillAI.md). It is not repeated here.

## How the engine calls it

1. Per AI unit, `CpDecide_Main` (`Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:316`) clears the decision and calls `AiDecideMainFunc()` (`:348-349`). The Guardian swap runs right after (`:352-357`, body in `SwapDefenderWithAllyIfNecessary`, `:269`).
2. `DecideScriptA` (`AiOptimization.c:675`) and `DecideScriptB` (`:715`) run the AI script loop, then `AiTryDoDanceCommand`, then `AiTryDoMenuSkills` (`:708`, `:747`). `DecideHealOrEscape` (`:754`, call `:807`) and `AiTryDoStaff` (`:813`, call `:839`) also call `AiTryDoMenuSkills`. The fallback itself is described in [MenuSkillAI.md](../../Documentation/Features/MenuSkillAI.md).
3. Rampage: `DecideScriptA` (`AiOptimization.c:689-693`) and `DecideScriptB` (`:729-733`) branch to the berserk action for Rampage units. The berserk unit list is built in `CpOrderBerserkInit` (`Kernel/Wizardry/AI/Source/AI.c:170`, Rampage filter at `:192-193`).
4. Teleportation decision: `AiTryExecScriptB` (`Kernel/Wizardry/AiHack/AiDecide/Source/PreAiDecide.c:36`) runs each entry of `gPreAi2DecideHook` (`:49`) before the AI-2 script. The only entry is `Ai2Decide_TryTeleportation` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiDecide.c:34`). It calls `AiTryTeleportationExt` (`TeleportationAiDecide.c:12`), which checks for an equipped weapon (`:16-17`), `SkillTester`, and an HP percentage gate `SKILL_EFF0` (`:20-24`). It then calls `GetTeleportationRandomPosition` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/Teleportation.c:19`) and `AiSetDecision(..., CONFIG_AI_ACTION_EXPA_Teleportation, ...)`.
5. Teleportation action: `CONFIG_AI_ACTION_EXPA_Teleportation` is 14 (`include/configs/configs.h:11`), installed with `InjectAiAction` in `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation.event`. `AiAction_Teleportation` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiAction.c:6`) runs `CONFIG_UNIT_ACTION_EXPA_ExecSkill` with `SID_Teleportation`. `AiAction_TeleportationIdle` (`TeleportationAiAction.c:31`) clears `US_HAS_MOVED_AI` and decrements `gAiState.unitIt`.
6. Target filters: `AiAttemptOffensiveAction` (`AiOptimization.c:489`) skips candidates with Shade+ (`SkillTesterPlus`, `:578-581`), skips candidates with Shade on a `Roll1RN(unit->lck)` roll (`:583-588`), and has the Rampage branch (`:593-600`).
7. Scoring: `AiGetDamageDealtCombatScoreComponent` (`Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c:358`, LYN-replaced via `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.lyn.event`) adds Provoke's `SKILL_EFF0` when the target has Provoke (`:362-364`), and adds Lightning Rod's `SKILL_EFF0` when the attacker's weapon is one of the listed items (`:375-390`).
8. Unit list: `CpOrderFunc_BeginDecide` (`Kernel/Wizardry/AI/Source/AI.c:216`) calls `BuildAiUnitList_Proc` (`AI.c:117`). Rearm is handled there: a unit with Rearm and no equipped weapon gets `GetRearmWeapon(...)` added (`AI.c:136-147`, `GetRearmWeapon` at `:44`).

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c` | `AiTryDoMenuSkills` (`:95`): menu-skill fallback. `AiAction_MenuSkill` (`:329`). `AiGetDamageDealtCombatScoreComponent` (`:358`): Provoke, Lightning Rod, Assign Decoy. |
| `Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills.event` | Hook sites (ORG) for `AiGetDamageDealtCombatScoreComponent` (`$3df34`) and the menu-skill functions. |
| `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiDecide.c` | `AiTryTeleportationExt` (`:12`), `Ai2Decide_TryTeleportation` (`:34`, gate on `AI_SCRIPT_AI2` and `ai2` values, `:39-54`). |
| `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/Teleportation.c` | `GetTeleportationRandomPosition` (`:19`): picks an enemy-faction origin unit, floods movement, picks a random tile. |
| `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiAction.c` | `AiAction_Teleportation` (`:6`), `AiAction_TeleportationIdle` (`:31`). |
| `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAction.c` | Teleport animation and position helpers (`skill_anim` `:25`, `set_position` `:35`; not read in detail). |
| `Kernel/Wizardry/SkillEffects/AiSkills/Teleportation.event` | `InjectAiAction(CONFIG_AI_ACTION_EXPA_Teleportation, ...)` and the Teleportation lyn hooks. |
| `Kernel/Wizardry/AiHack/AiDecide/data.event` | `gPreAi1DecideHook` and `gPreAi2DecideHook` lists. |
| `Kernel/Wizardry/AiHack/AiDecide/Source/PreAiDecide.c` | `AiTryExecScriptA` / `AiTryExecScriptB`: run the pre-decide hook lists. |
| `Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c` | `CpDecide_Main` (`:316`), `SwapDefenderWithAllyIfNecessary` (`:269`), `AiAttemptOffensiveAction` (`:489`, Shade `:578-588`, Rampage `:593`), `DecideScriptA` / `DecideScriptB` (`:675`, `:715`), `DecideHealOrEscape` (`:754`), `AiTryDoStaff` (`:813`). |
| `Kernel/Wizardry/AI/Source/AI.c` | `GetRearmWeapon` (`:44`), `BuildAiUnitList_Proc` (`:117`, Rearm `:136-147`), `CpOrderBerserkInit` (`:170`), `CpOrderFunc_BeginDecide` (`:216`). |
| `Kernel/Wizardry/AiHack/AiAction/Source/AiActionConf.c` | `gAiActionTable` entries, including `CONFIG_AI_ACTION_EXPA_MenuSkill` (`:69`). |
| `include/configs/configs.h:11-14` | `CONFIG_AI_ACTION_EXPA_Teleportation` (14), `CONFIG_AI_ACTION_EXPA_MenuSkill` (15). |
| `Kernel/Wizardry/UnitStatusGetter/source/PowGetter.c:199` | Rampage: `pow / 2` (`:201`). Also `SpdGetter.c:178` (`spd / 2`) and `SklGetter.c:162` (`skl / 2`). |
| `Kernel/Wizardry/UnitSelection/Source/UnitSelection.c:47` | Rampage units are not player-selectable (`PLAYER_SELECT_NOCONTROL`, `:47-50`). |
| `Data/SkillSys/Source/SkillExtraInfo.c:778-783`, `:1523-1525` | SKILL_EFF values: Teleportation HP threshold `{{ 50 }}`, Provoke `{{ 50 }}`, Lightning Rod `{{ 100 }}`. |
| `Data/DesignerConfig/designer-config.c:99` | `menu_skill_ai_use` default (false). |
| `Documentation/Features/MenuSkillAI.md` | Menu-skill AI fallback: list of usable menu skills, plan, limitations. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the hook by effect:
   - Target score in combat (Provoke-like): add a block to `AiGetDamageDealtCombatScoreComponent` (`MiscAiSkills.c:358`), before `return score;`.
   - Filter which enemies the AI attacks (Shade-like): add a `continue` in the candidate loop of `AiAttemptOffensiveAction` (`AiOptimization.c:578-600`).
   - Change how the AI picks or moves units (Rampage-like): `CpOrderBerserkInit` (`AI.c:170`) or `DecideScriptA` / `DecideScriptB`.
   - Give an AI unit an item at the start of its phase (Rearm-like): `BuildAiUnitList_Proc` (`AI.c:136`).
   - A new AI-only action: add an `AiAction` exec/idle pair, register it in `gAiActionTable` with `InjectAiAction` (see `Teleportation.event`), and call `AiSetDecision` from a pre-decide hook. The idle function must clear `US_HAS_MOVED_AI` and decrement `gAiState.unitIt` (pattern in `MiscAiSkills.c:343-348` and `TeleportationAiAction.c:21-28`).
   - Pre-decide hook (runs before the AI-2 script): append to `gPreAi2DecideHook` in `Kernel/Wizardry/AiHack/AiDecide/data.event`. The hook returns `true` when it acted.
   - Menu-skill fallback: add a case to `AiTryDoMenuSkills` (`MiscAiSkills.c:95`) using `AI_MENU_SELF_CASE` or `AI_MENU_TARGET_CASE`. The skill must also have a menu definition with `isAvailable` (see MenuSkillAI.md).
3. Use `BattleFastSkillTester(&gBattleTarget, sid)` inside battle-simulation scoring (as Provoke does). Use `SkillTester` elsewhere (see Gotchas).
4. Add SKILL_EFF values to `Data/SkillSys/Source/SkillExtraInfo.c`.
5. Build and test in game. Not run here.

## Template

Adapted from Provoke (`Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.c:362-364`) inside `AiGetDamageDealtCombatScoreComponent`. `MiscAiSkills.c` already includes `skill-system.h` and `constants/skills.h`. The function is already LYN-hooked (`Kernel/Wizardry/SkillEffects/AiSkills/MiscAiSkills/MiscAiSkills.lyn.event`), so no `.event` change is needed.

```c
#if defined(SID_Example) && (COMMON_SKILL_VALID(SID_Example))
	if (BattleFastSkillTester(&gBattleTarget, SID_Example))
		score += SKILL_EFF0(SID_Example);
#endif
```

Matching `Data/SkillSys/Source/SkillExtraInfo.c` entry (same guard style as `:782-784`):

```c
#if (defined(SID_Example) && COMMON_SKILL_VALID(SID_Example))
	[SID_Example] = {{ 50 }},
#endif
```

## Worked examples

- `Provoke` (`MiscAiSkills.c:362-364`, `SkillExtraInfo.c:782-783`): combat-score bonus of 50 when the target has the skill.
- `Shade` / `Shade+` (`Kernel/Wizardry/AiHack/AiOptimization/Source/AiOptimization.c:578-588`): candidate filter in `AiAttemptOffensiveAction`; Shade is a roll on LCK, Shade+ always excludes.
- `Guardian` (`AiOptimization.c:352-357`, body `:269-300`): after the AI decision, the attacked Guardian unit swaps with an adjacent higher-HP ally.
- `Teleportation` (`Kernel/Wizardry/SkillEffects/AiSkills/Teleportation/TeleportationAiDecide.c:34`): AI-2 pre-decide hook plus an AI action.

## Gotchas

- Hot path. `AiGetDamageDealtCombatScoreComponent` runs once per simulated combat, inside `AiAttemptOffensiveAction`. Existing code uses `BattleFastSkillTester` there (`MiscAiSkills.c:363`, `:376`) because the target is a `BattleUnit` in the battle snapshot. `AiTryDoMenuSkills` scans the actor's full skill list and is called from four places per AI decision (`AiOptimization.c:708`, `:747`, `:807`, `:839`). See MenuSkillAI.md for its cost.
- `menu_skill_ai_use` is false by default (`Data/DesignerConfig/designer-config.c:99`). `AiTryDoMenuSkills` returns false at once when it is off (`MiscAiSkills.c:103-104`).
- Doc inconsistency: [MenuSkillAI.md](../../Documentation/Features/MenuSkillAI.md) says the fallback is gated by `MenuSkills_Usability`. The code calls the per-skill `def->isAvailable` from `gpSkillMenuInfos[sid]` (`MiscAiSkills.c:113-121`). `MenuSkills_Usability` is the callback of the upper unit-skill menu entry (`Kernel/Wizardry/SkillSys/kernel/SkillMenu.c:12-21`). Code is the authority.
- Lightning Rod text vs code: the text says "Only units with this skill can be targeted by enemy siege tomes if they are in range". The only use of `SID_LightningRod` in the repo is the AI score bonus (`MiscAiSkills.c:375-390`, value 100 at `SkillExtraInfo.c:1524`). No targeting restriction was found. Unverified whether the restriction is implemented elsewhere.
- Rearm text vs code: the text says "if the unit has no equippable items this turn". The code checks only `GetUnitEquippedWeapon(unit) == 0` (`AI.c:138`). It runs during list build every AI decide phase (`AI.c:136-147`), not once per turn.
- Teleportation: `GetTeleportationRandomPosition(struct Unit *unit, ...)` (`Teleportation.c:19`) never uses its parameter. The loop declares a new `unit` (`:29`) that shadows it. The origin of the movement flood is a random unit of the enemy faction (`:26`, `:29`), and the flood runs with that unit's cost table and skills (`SetWorkingMoveCosts(GetUnitMovementCost(unit))`, `:36-39`). Only the Mov value comes from the AI unit (`MovGetter(gActiveUnit)`). The destination is then checked with `CanUnitCrossTerrain(gActiveUnit, ...)` (`:55`). Whether that split is intended is unverified.
- Teleportation gates: equipped weapon required (`TeleportationAiDecide.c:16-17`); `ai2` must be one of 0, 1, 2, 9, 10, 11 (`:46-54`); HP must be at least `SKILL_EFF0` percent (`:20-24`, value 50 at `SkillExtraInfo.c:779`).
- Guardian: `GetUnit(gAiDecision.targetId)` is read before any null check (`AiOptimization.c:352`). With no decision target (`targetId` 0) the result depends on `GetUnit(0)`, which is unverified. The swap only applies to enemies of the AI (`:355`) and to 4-neighbour allies with strictly higher current HP (`:269-288`).
- Shade is random per candidate per call (`AiOptimization.c:585-587`). Shade+ excludes deterministically (`:578-581`).
- Rampage in code: `DecideScriptA` / `DecideScriptB` branch to the berserk action (`AiOptimization.c:689`, `:729`). Stat bonuses are `pow / 2`, `spd / 2`, `skl / 2` (`PowGetter.c:199-201`, `SpdGetter.c:178-180`, `SklGetter.c:162-164`), which matches the text's "+50%".
- AI actions must clear `US_HAS_MOVED_AI` and decrement `gAiState.unitIt` in their idle function, or the unit is skipped or reprocessed (`MiscAiSkills.c:343-348`, `TeleportationAiAction.c:21-28`).
