# Grow Along a Skill Tree

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

## Introduction

The skill tree gives a unit a dedicated progression page on the stat screen. Eirika can select a starting branch, spend skill points on connected skills, and inspect the skills that become unavailable after a path is chosen.

The proof of concept uses the existing learned-skill bitfield and BWL skill-point counter. It does not add a new save field: the selected path is derived from the first learned path-specific node.

## Plan

The tree is stored as a small ROM definition. Each node identifies its skill, path, parent, screen position, and directional neighbors.

```text
            [LifeAndDeath]
           /      |       \
   [FaireSword] [Patience] [Avoid]
       /  \                  /  \
[Flashing] [Luna]    [Chivalry] [QuickDraw]
                          |
                   [RightfulKing]
```

| State | Condition | Player-facing result |
|-------|-----------|----------------------|
| Learned | The node's learned bit is set | Green marker; the skill remains part of the unit's learned pool |
| Available | Parent is learned, path is valid, and SP covers the dedicated cost | The node can be selected and purchased |
| Path locked | A different path-specific node was learned | The node remains visible but cannot be purchased |
| Prerequisite locked | The parent node is not learned | The node remains visible but cannot be purchased |
| No tree | The unit has no ROM definition | The page displays `No skill tree` and has no interactive cursor |

On an available node, `A` opens an inline confirmation prompt. Confirming deducts the node's dedicated cost and calls `AddSkill`; canceling leaves both the unit and SP unchanged. On the tree page, D-pad moves the cursor only; `L` / `Select` change pages and `R` opens the selected skill's description help. The page draws icons and status markers without skill names.

The cursor and help-box state live in a single even-sized EWRAM reservation in `config-memmap.s`. This keeps the state screen-local without consuming normal save or suspend-save bytes.

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Skill-tree data model | [`skill-tree.h`](../../include/kernel/skill-tree.h) | Defines node, unit-tree, cost-table, and state-query structures |
| SP cost table | [`SkillTree.c`](../../Data/SkillTree/Source/SkillTree.c) | Maps each PoC skill ID to its tree cost |
| Eirika tree | [`SkillTree.c`](../../Data/SkillTree/Source/SkillTree.c) | Defines the nine-node Eirika layout and its directional navigation graph |
| Path locking | `GetSkillTreeChosenPath`, `IsSkillTreeNodeAvailable`, and `IsSkillTreeNodeLocked` in [`SkillTree.c`](../../Data/SkillTree/Source/SkillTree.c) | Derives the chosen path from learned path-specific skills and validates prerequisites |
| Stat-screen page | `DrawPageSkillTree` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Draws connectors, icons, SP, node state markers, and selected-node information |
| Tree input | `SkillTree_HandleStatScreenInput` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Moves the node cursor, opens confirmation, spends SP, and routes help |
| Stat-screen replacement | `StatScreen_OnIdle` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Preserves vanilla stat-screen controls while dispatching tree-page input |
| Page registration | [`HelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBox.c), [`DrawMorePage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawMorePage/Source/DrawMorePage.c), and [`data.event`](../../Kernel/Wizardry/Core/StatScreen/data.event) | Adds the optional page, visible-page translation, help routing, and draw function |
| Configuration | [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables the tree system and its stat-screen page for the PoC |
| SP rewards | [`BattleHit.c`](../../Kernel/Wizardry/Core/BattleSys/Source/BattleHit.c) | Grants the existing kill SP reward when either the skill shop or tree system is enabled |
| Screen state storage | [`config-memmap.s`](../../include/link/config-memmap.s) | Reserves cursor, confirmation, message, and help-box state in EWRAM |

## TODO

- Add ROM definitions for additional units.
- Replace the temporary page-name art with a dedicated `Skill Tree` label.
- Add localized confirmation and state messages.
- Add a respec policy before exposing trees to the full roster.

## Limitations & Bugs

- Only Eirika has a tree definition; all other units show an empty page.
- The path choice is intentionally permanent because it is inferred from learned skills.
- The tree uses the existing seven/five equipped-skill slot rules. A full skill list prevents a purchase even though learned skills can otherwise remain in the learned pool.
- Branch links are implied by layout (I / II / III columns); dedicated connector art is still TODO.
- The page currently reuses the Skills page-name graphic until dedicated art is added.
- Stat-screen pages no longer wrap from last→first (or first→last); navigation stops at the ends.
