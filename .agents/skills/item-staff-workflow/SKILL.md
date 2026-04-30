---
name: item-staff-workflow
description: "Use when creating a new custom item staff in this repo. Follow the repeatable workflow for picking a safe item slot, wiring item data, adding revamp handlers, building any menu/UI flow, and validating the result. Avoid 0xCA because skill scrolls use it; the next usable item slot is 0xDB."
---

# Item Staff Workflow

Use this skill when adding a new custom staff item that needs item data, item-effect revamp wiring, text entries, and possibly a custom menu or target-selection flow.

## Use This Skill When

- The user asks for a new staff item.
- The staff needs a custom usability/effect/action pipeline.
- The staff needs a custom menu, popup, or selection list.
- The item must coexist with existing item aliases and shared effect slots.
- You need a repeatable checklist for adding staff items without colliding with reserved indices.

## Core Guardrails

- Do not use item index `0xCA`; it is reserved by skill scrolls.
- Start from the next usable item index, currently `0xDB`, unless the repository state changes.
- Prefer reusing existing state, tables, and helpers before reserving new RAM.
- Keep new item behavior consistent with the existing item-effect revamp architecture.
- If a new menu or list is needed, keep the UI simple, deterministic, and close to existing menu patterns.

## Workflow

1. Confirm the staff’s player-facing behavior.
2. Check whether an existing item slot, effect slot, or shared data buffer can be reused.
3. Pick a safe item index at or above `0xDB` and add the item constant.
4. Add the item entry in the item database with name, description, rank, icon, uses, and effect ID.
5. Add or reuse the revamp enum slot in the item-sys headers.
6. Register the staff handlers in the revamp table.
7. Add the source text entries for the item name and all related descriptions.
8. Implement the staff logic, including any usability checks, action flow, and post-use effects.
9. If the staff needs a menu, build the smallest menu that matches the existing engine patterns.
10. Compile the touched objects and regenerate text artifacts if the item text changed.

## Implementation Pattern

### 1. Item Slot

- Add the item constant in `Tools/FE-CLib-Mokha/include/constants/items.h`.
- Use a slot that does not collide with skill scrolls or existing aliases.
- If the item is an alias or placeholder, make that explicit in the enum and the item table.

### 2. Item Data

- Add the item row in `Data/ItemSys/Source/Items.c`.
- Set the staff type, uses, rank, range, icon, and `useEffectId`.
- Keep the item entry visually and mechanically aligned with the staff’s role.
- Add text IDs for name, description, use description, and subtitle if needed.

### 3. Revamp Wiring

- Add the effect enum in `include/kernel/item-sys.h`.
- Declare the usability/effect/action functions in the same header.
- Register the handlers in `Kernel/Data/ItemSys/Source/IERevampTable.c`.
- Keep the effect slot and function names consistent across all files.

### 4. Staff Logic

- Put shared helpers near the top of the new source file.
- Separate responsibility into small helpers for:
  - availability checks,
  - target enumeration,
  - placement validation,
  - UI drawing,
  - item execution.
- If the staff depends on a list of targets, keep the list ordering stable and define how duplicates and stale entries are handled.
- If the staff needs adjacent placement, validate all candidate tiles before enabling the item.

### 5. Menu / Popup Flow

- Reuse the standard menu framework already used in the repo.
- Keep visible lists short when possible; use scroll bars only when the list must exceed the visible window.
- Draw map sprites or icons in the menu only if they help the user choose correctly.
- Put cleanup in the menu end handler so cursors, sprites, and worker procs are always released.
- Avoid using `EndTargetSelection` as the handoff point for a blocking menu proc unless the surrounding proc ownership is already correct; starting the menu from the wrong parent can leave the active unit sprite softlocked even when the menu appears.

### 6. Execution Flow

- Store the chosen target and any placement coordinates in the action data before exiting the menu.
- Perform the actual revive/use effect in the action handler.
- Refresh maps, sprites, and movement state after moving a unit onto the map.
- Trigger the expected popup or animation after the action is committed.

### 7. Text Pipeline

- Add the source strings in `Contents/Texts/Source/texts/Items.txt`.
- Rebuild the text data so the generated message header contains the new IDs.
- Verify the generated IDs are the ones referenced by the item table and menu text.

## Validation Checklist

- The new item index does not overlap reserved content.
- The item table compiles and references valid text IDs.
- The revamp table points at the new handlers.
- The new source file compiles cleanly on its own.
- Any menu or popup code closes cleanly and clears temporary state.
- The text build regenerates the new message IDs.
- If the staff uses runtime storage, that storage is already reserved and declared.

## Common Failure Modes

- Picking an item index that is already reserved by another system.
- Adding item data without adding the matching text IDs.
- Wiring the item table but forgetting the revamp table.
- Creating menu state without clearing the worker proc or cursor state.
- Using shared globals without confirming the correct header or declaration path.
- Assuming text IDs exist before regenerating the text build.

## Good Default Output

When asked to create a staff item, prefer a short implementation plan that covers:

- item slot selection,
- item database entry,
- effect revamp registration,
- source text additions,
- staff-specific logic,
- validation build steps.

If the staff needs a custom list UI, include the list behavior and the visible count in the plan before coding.
