# Grow Along a Skill Tree

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla Sacred Stones has no long-term skill progression track on the unit screen. Skills arrive from class lists, scrolls, or shops, but there is no branching “build” the player commits to over a campaign.

This feature adds an optional **Skill Tree** page on the unit status screen. The player spends BWL skill points (`NewBwl.skillPoints`) to unlock skills along a parent-linked tree. Choosing a path locks the other path-specific branches. The first proof of concept ships for **Eirika only**.

Design goals:

- Reuse existing learned-skill bits and BWL SP — **no new save or suspend fields** for path choice
- Keep the page interactive (cursor, confirm purchase, R-text) without starving the Kernel text section (UI lives under `Data/SkillTree/`)
- Fit a readable layout in the right-hand stat panel (about six icon columns by six rows)

---

## 🛠️ Plan

### Config gates

Both flags are set in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c):

| Flag | Role |
|------|------|
| `skill_tree` | Enables the system (including kill SP rewards shared with the skill shop) |
| `stat_page_skill_tree` | Registers the optional last status-screen page (`PAGE_SKILL_TREE = 7`) |

### Data model

Each unit tree is a ROM table entry (`struct UnitSkillTree`): character ID, node count, and up to `SKILL_TREE_MAX_NODES` (24) nodes.

Each node stores:

| Field | Meaning |
|-------|---------|
| `sid` | Equippable skill ID (`0x01`–`0xFE`) |
| `path` | `0` = shared trunk; `1` / `2` / `3` = mutually exclusive branches |
| `parent` | Index of prerequisite node, or `SKILL_TREE_NODE_NONE` |
| `x`, `y` | Tile position inside the 18×18 page scratch |
| `adj` | Unused by runtime navigation (geometry is used instead) |

SP costs live in a separate SID→cost table (`gSkillTreeSpCostTable`).

### Eirika layout (bottom-up)

The tree grows **upward** from the footer toward the title. Shared trunk first, then three exclusive paths:

```text
 y=3       [Astra]       [Sol]       [RightfulKing]
 y=5   [Flashing] [Luna] [Adept] [Chivalry] [QuickDraw]
 y=7       [Vantage]     [Renewal]     [Desperation]
 y=9   [FaireSword]      [Patience]      [Avoid]
 y=11                 [StrongRiposte]
 y=13                  [LifeAndDeath]
```

| Path | Theme | Head → … → Capstone |
|------|-------|---------------------|
| Shared | Trunk | LifeAndDeath → StrongRiposte |
| 1 | Offense | FaireSword → Vantage → FlashingBlade / LunaAttack → Astra |
| 2 | Sustain | Patience → Renewal → Adept → Sol |
| 3 | Tempo | Avoid → Desperation → Chivalry / QuickDraw → RightfulKing |

Typical SP costs by depth: 5 / 10 / 15 / 20 / 25.

### Node states

| State | Condition | Player-facing result |
|-------|-----------|----------------------|
| Learned | Skill bit is set | Half-bright icon; purchase blocked; “Learned” message on A |
| Available | Parent learned, path open, enough SP | Full-color icon; A opens Learn? Yes/No |
| Path locked | Another path’s skill was learned | Half-bright; “Locked” on A |
| Prerequisite locked | Parent not learned | Half-bright; “Locked” on A |
| No SP | Path/parent OK but SP &lt; cost | Half-bright; “No SP” on A |
| No tree | Unit missing from ROM table | “No tree” placeholder; no cursor |

Path choice is **inferred**: the first learned node with `path != 0` becomes the locked path for the rest of the run. No extra save byte.

### Controls (tree page)

| Input | Behavior |
|-------|----------|
| D-pad | Move to nearest skill in that direction (layout geometry) |
| Left / Right at edge | Change status page (wraps last ↔ first) |
| A | Open confirm if available; otherwise show status message |
| Confirm Yes | Spend dedicated SP cost, `AddSkill`, refresh icons |
| Confirm No / B | Cancel confirm |
| R | Skill description help for the selected node |
| B (no confirm) | Exit status screen (vanilla) |

### Drawing notes

- Icons draw once when the page becomes idle after a slide (`gSkillTreePageDrawn`). Cursor moves only refresh Cost / SP / messages so VRAM icon uploads are not repeated every frame.
- Learned and unselectable icons use half-bright copies of the item-icon palettes on BGPAL 6/7; those banks are restored when leaving the page.
- Page title art temporarily reuses the Skills (“Weapon & Skills”) label until dedicated art exists.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Data model / caps | [`skill-tree.h`](../../include/kernel/skill-tree.h) | Node structs, `SKILL_TREE_MAX_NODES`, query prototypes, EWRAM externs |
| SP costs + Eirika tree | [`SkillTree.c`](../../Data/SkillTree/Source/SkillTree.c) | `gSkillTreeSpCostTable`, `gUnitSkillTreeTable`, path/parent availability |
| Path / buy checks | `GetSkillTreeChosenPath`, `IsSkillTreeNodeAvailable`, `IsSkillTreeNodeLocked` in [`SkillTree.c`](../../Data/SkillTree/Source/SkillTree.c) | Infer locked path; gate purchases |
| Page draw | `DrawPageSkillTree` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Title, icons, Cost/SP, confirm prompt, faded pals |
| Cursor geometry | `SkillTree_FindNeighbor` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Nearest-node D-pad movement from `x`/`y` |
| Input / learn | `SkillTree_HandleStatScreenInput`, `SkillTree_TryLearn` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Idle hook, confirm, SP spend, help |
| Idle replace | `StatScreen_OnIdle` in [`SkillTreePage.c`](../../Data/SkillTree/Source/SkillTreePage.c) | Dispatches tree input; otherwise vanilla unit/page controls |
| Page id / count | `TranslateStatPageId`, `IsStatScreenPageAvailable` in [`HelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBox.c); `GetStatPageCount` in [`DrawMorePage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawMorePage/Source/DrawMorePage.c) | Visible index ↔ physical page 7 |
| Draw table | [`data.event`](../../Kernel/Wizardry/Core/StatScreen/data.event) | `POIN DrawPageSkillTree` at index 7 |
| Installer | [`SkillTree.event`](../../Data/SkillTree/SkillTree.event) via [`Data.event`](../../Data/Data.event) | Lyn events for data + page |
| Designer flags | [`designer-config.c`](../../Data/DesignerConfig/designer-config.c), [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | `skill_tree`, `stat_page_skill_tree` |
| Kill SP | [`BattleHit.c`](../../Kernel/Wizardry/Core/BattleSys/Source/BattleHit.c) | Grants SP when `skill_shop` **or** `skill_tree` is on |
| Screen RAM | [`config-memmap.s`](../../include/link/config-memmap.s) | `gSkillTreeCursor` block (cursor, flags, `gSkillTreePageDrawn`, help box) |
| Temp page name | [`PageNameSprite.c`](../../Data/StatScreen/Source/PageNameSprite.c) | Page-7 sprite reuses Skills art |

---

## 📝 TODO

- Add ROM trees for more playable units.
- Dedicated “Skill Tree” page-name graphic (stop reusing Skills).
- Localized strings instead of hardcoded English (`Learn?`, `Cost`, `SP`, status messages).
- Optional connector art between parent and child icons.
- Respec / path-reset policy before roster-wide rollout.
- Review Tellius capacity interaction when purchasing many tree skills.

---

## 🐛 Limitations & Bugs

- Only Eirika has a tree; other units show “No tree.”
- Path lock is permanent for the PoC because it is inferred from learned skills.
- Purchase still requires a free equip slot when the skill is not already equipped (`AddSkill` / slot rules).
- Parent links are data-only; there are no drawn branch lines yet.
- Half-bright icons borrow BGPAL 6/7 for the duration of the page visit.
- Full icon redraw runs on page enter and after a successful learn — not on every D-pad tick (intentional, to avoid VRAM upload storms).

Please report issues in the repository’s **Issues** tab.

---
