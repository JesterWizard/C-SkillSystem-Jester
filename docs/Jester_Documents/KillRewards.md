# Kill Rewards

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

`gpKernelDesignerConfig->killRewards`

This feature is designed to **award items** to a unit if they happen to defeat a certain unit.

The intented purpose is to offer secrets or to tie gameplay into the author's story.

---

## 🛠️ Plan

In vanilla FE8, items are usually awarded when defeating units if they have the **drop item** flag enabled.

This system compliments that feature, rather than replacing it, so you can have both running at the same time.

When a combat has ended, we hook into the ``BATTLE_HandleItemDrop`` function and cycle through an array.
Each item in the array will consist of a pair of units and the item to be awarded. If both units are present in the
fight, them the item is awarded to the surviving unit (player only).

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Array construction** | `gKillRewards` — [`KillRewards.c`](../../Kernel/Common/KillRewards/KillRewards.c) | Determines what awards are obtained from specific battles |
| **Hook point** | `BATTLE_HandleItemDrop` — [`BattleHit.c`](../../Kernel/Core/BattleSys/Source/BattleHit.c) | The hooked function we're checking in |

---

## 📝 TODO

- Add a chapter specific check. Not implemented for now due to speed concerns
- Add flags for specific battles between units if they'll fight several times, so the reward is only given once

---

## 🐛 Limitations & Bugs

Please report issues via the repository’s **Issues** tab.

---