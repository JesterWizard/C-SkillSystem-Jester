# Start Map Effects

---

## Index
- [Introduction](#introduction)
- [Implementation Plan](#implementation-plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

## Introduction

The Start Map Effects feature adds a pre-phase map menu that lets the player pick a single map-wide effect before the chapter begins.

Player-facing goals:
- Keep the menu readable by showing only the effect name in the list.
- Show a short R text description for the highlighted effect.
- Apply the chosen effect to the matching faction for a fixed number of turns.
- Let the menu stay lightweight enough that new effects can be added by editing one table.

The current menu entries are intentionally split into two text fields:
- `label`: the short name shown in the list.
- `rText`: the description shown underneath the menu.

That separation keeps the visible menu clean while still giving each effect a descriptive line of text.

## Implementation Plan

The feature follows a simple table-driven flow.

| Step | Behavior | Result |
|------|----------|--------|
| 1 | The pre-phase hook opens the prompt if no choice has been made yet. | The player sees the map effect menu before the map proceeds. |
| 2 | The prompt draws the list, the selected R text, and the cursor hand. | The active effect is easy to read at a glance. |
| 3 | The player confirms an entry or cancels with No Effect. | The selected effect is stored in suspend state. |
| 4 | The prompt closes and restores the unit sprites and BG state. | The map returns to normal presentation. |
| 5 | The stored effect is checked each pre-phase tick. | The effect duration counts down until it expires. |

The effect table now carries all of the data needed to add a new entry:
- list label
- R text description
- stat/move behavior kind
- numeric value
- duration in turns
- target faction

That means adding a new effect should usually only require editing the table in `StartMapEffects.c`.

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Effect table | [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Stores each effect's label, R text, behavior kind, numeric value, duration, and target faction. |
| Menu setup | `StartMapEffectsPrompt_OnInit` in [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Initializes text, UI graphics, cursor hand, and the hidden-unit state while the prompt is open. |
| List rendering | `StartMapEffectsPrompt_Draw` in [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Draws the visible list, the scroll bar, the cursor hand, and the selected R text. |
| Menu teardown | `StartMapEffectsPrompt_Finish` in [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Restores the sprite visibility, clears the menu area, and ends the prompt cleanly. |
| Effect application | `StartMapEffects_ApplyStatEffect` and `StartMapEffects_ApplyMovEffect` in [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Applies the selected effect to matching units during stat and movement reads. |
| Pre-phase hook | `StartMapEffects_PrePhaseHook` in [`StartMapEffects.c`](../../Kernel/Wizardry/Common/StartMapEffects/Source/StartMapEffects.c) | Starts the prompt on the first entry, then counts down the effect duration on later turns. |
| Documentation reference | [`StartMapEffects.md`](StartMapEffects.md) | Contributor-facing summary of the feature and extension points. |

## TODO

- Add localized text IDs for the effect names and R text if this feature needs translation support.
- Consider splitting the menu descriptions into two lines if a future effect needs a longer explanation.
- Add a small preview note or icon if the list grows beyond a handful of effects.
- Review whether the No Effect entry should be visually distinct from real effects.

## Limitations & Bugs

- The effect descriptions are currently short inline strings in code, so they are not yet data-driven from the text system.
- The menu supports one active effect at a time per chapter.
- The description line is intentionally compact; longer text will need a layout change.
- The feature hides map unit sprites while the prompt is open and restores them afterward, which is correct for the menu but still worth testing on chapters with unusual scripted unit visibility.
