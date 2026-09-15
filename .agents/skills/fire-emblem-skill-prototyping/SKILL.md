---
name: fire-emblem-skill-prototyping
description: Prototype and integrate new Fire Emblem 8 C Skill System skills with correct menu, targeting, action, stat, text, and build wiring. Use when adding or modifying a skill, especially rescue, Pair Up, combat, passive, or menu skills that must match vanilla mechanics.
---

# Fire Emblem Skill Prototyping

Use this skill to turn a player-facing skill idea into a small, verifiable FE8 implementation. Prefer an existing project pattern over inventing a new subsystem.

## First: Write the behavior contract

Before editing code, record:

- Skill name, SID, category, capacity, price, icon, and text.
- Who can use it and when it appears.
- Valid targets, allegiance rules, range, boss/phantom/status restrictions, and stat thresholds.
- Whether it is a normal FE8 action, a Pair Up action, or a separate mechanic.
- What states, stats, inventory interactions, movement, canto, drop, death, and phase transitions should do afterward.
- Whether the skill needs AI behavior, save/suspend storage, new RAM, or only transient action data.

Treat “regular rescue” and “Pair Up” as different mechanics even though both use `US_RESCUING` and `US_RESCUED`.

## Repository reconnaissance

Find and read the closest working examples before implementing:

- Menu skill: `Kernel/Wizardry/SkillEffects/MenuSkills/*.c`
- Rescue/Pair Up: `Kernel/Wizardry/PairUp/Source/PairUp.c`, `Kernel/Wizardry/SkillEffects/MenuSkills/Refuge.c`
- Target rules: `Kernel/Wizardry/UnitTarget/UnitTarget/Source/UnitTarget.c`
- Rescue/trade/drop primitives: `Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c`
- Skill action table: `Kernel/Data/SkillSys/Data/SkillActionInfo.c`
- Skill menu table: `Kernel/Data/SkillSys/Data/SkillMenuInfo.c`
- General skill metadata: `Data/SkillSys/Source/SkillInfo.c`

Use `rg` to locate the exact function and table names. Do not assume vanilla behavior survived the project’s replacement hooks.

## Implementation workflow

1. Choose the smallest existing architecture that fits the contract.
2. Add the SID in the appropriate section of `include/constants/skills-equip.enum.txt`.
3. Add source text, capacity, price, and metadata using the project’s existing naming convention.
4. Add the source implementation with small helpers for usability, target creation, target validation, selection, and execution.
5. Add declarations to `include/kernel/skill-system.h` when required.
6. Register menu and action handlers in the corresponding data tables.
7. Include the generated `.lyn.event` in the relevant feature event file.
8. Rebuild generated artifacts through `make`; never hand-edit generated `.lyn.event` output.

## Menu-skill template

For an adjacent-target menu skill, follow this shape:

```c
static bool Skill_IsValidTarget(struct Unit *actor, struct Unit *target);
static void MakeTargetListForSkill(struct Unit *unit);
u8 Skill_Usability(const struct MenuItemDef *def, int number);
u8 Skill_OnSelected(struct MenuProc *menu, struct MenuItemProc *item);
bool Action_Skill(ProcPtr parent);
```

Target selection should:

- initialize the target list and range map,
- set `gSubjectUnit` before map iteration,
- store `target->uid`, coordinates, SID, and `unitActionType`,
- clear selection graphics before returning,
- revalidate actor and target in the execution callback.

Use `gActionData.subjectIndex` when an action needs a stable actor identity; use `gActiveUnit` only when the surrounding animation/action framework requires the current active unit.

## Target validation checklist

For every target, explicitly decide each item rather than inheriting it accidentally:

- valid unit and on-map availability,
- allied, same-allegiance, enemy, or any faction,
- boss and phantom exclusion,
- dead, hidden, rescued, rescuing, ballista, and unavailable states,
- berserk and other status restrictions,
- skill-based refusal such as `SID_AidRefusal`,
- Aid/Constitution or other vanilla eligibility via the project helper,
- terrain, adjacency, range, and duplicate-target behavior.

Validate again during execution because the target list can become stale.

## Rescue and Pair Up guardrails

The project’s Pair Up implementation uses vanilla rescue state bits. Calling `UnitRescue` sets the correct carry/drop relationship, but it does not by itself guarantee regular-rescue behavior.

When a skill rescues a unit without Pair Up:

- Do not dispatch through Pair Up’s `ActionRescue`.
- Use the regular rescue state transition and hide the rescued unit.
- Ensure `PairUp_IsRescuer`, `PairUp_IsRescued`, and `PairUp_GetRescuePartner` reject the non-Pair-Up relationship.
- Ensure `PairUp_RescueStatScale` does not grant partner stat bonuses.
- Ensure Pair Up transfer and switch menus cannot use the relationship.
- Audit custom rescue-support sprite rendering; hidden rescued enemies must not be drawn as Pair Up supports.
- Preserve normal `UnitDrop`/release behavior and map/sprite refreshes.

For FE8-style rescue penalties, a non-Pair-Up rescuer has Skill and Speed reduced to half while carrying. Apply this in the same status-getter path used by the project, and do not apply the Pair Up bonus on that path.

If the skill forbids trading with a rescued unit, audit every interaction path, not only the visible Trade menu. Search for `TryAddUnitToTradeTargetList`, `MakeTradeTargetList`, rescue transfer/take/give handlers, and Pair Up transfer handlers.

## Actions and state cleanup

After committing a movement or rescue action:

- set both units’ reciprocal `rescue` indices,
- set or clear `US_RESCUING`, `US_RESCUED`, and `US_HIDDEN` consistently,
- update `gActionData.xMove`/`yMove` and subject/target indices as needed,
- refresh entity maps and unit sprites,
- preserve normal drop, death, phase, and suspend behavior,
- do not leave temporary markers, target lists, or animation procs active.

Do not add a new RAM marker until an existing unit state, action-data field, or project helper has been ruled out. If persistent state is required, check save/suspend capacity first.

## Verification loop

Compile the smallest touched targets first:

```sh
make path/to/touched.lyn.event
```

Then run the full build:

```sh
make
```

If the feature changes memory allocation, run `make ramcheck`; if it changes suspend/save data, run `make savecheck`.

The IDE linter may report missing project headers such as `common-chax.h` when it invokes standalone clang. Treat the repository’s `make` build as the authoritative compile check, while still fixing any errors emitted by the actual build.

## Behavioral test matrix

Before handing off, test the skill in-game or with the project’s available test harness:

- menu hidden, disabled, and enabled states,
- every valid and invalid target category,
- stale-target revalidation,
- action animation and active-unit identity,
- movement/canto and `US_HAS_MOVED`,
- resulting Skill and Speed values,
- no unintended Pair Up stat preview, support sprite, switch, or transfer,
- trade/take/give restrictions,
- drop/release and terrain validation,
- rescued-unit death and phase cleanup,
- suspend/resume if relevant.

Report both the build result and any behavior that could not be tested directly.
