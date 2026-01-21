# Timer

<p align="center">
  <img src="../Gifs/Timer.gif" alt="Timer Demo" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

``gpKernelDesignerConfig->goal_timer``

This features adds a new goal type in the form of a real-time ticking clock. E.g. you have to complete a map within 15 or you lose.

In the words of Huichelaar - "Timed stages in FE? That's so evil lmao"

---

## 🛠️ Plan

- Add a configurable timer that is set in an ASMC of the chapter setup events
- Have that timer countdown when not in battle animations
- Produce a game over screen when it hits 0
- Suspend/Resume current time
- Add/remove time based on whatever parameters you wish in the variable ``gChapterTimerSeconds``
- Play events when certain times are reached (time currently doesn't pause of them, will need to locate the event engine proc)

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Global variables** | `gChapterTimerSeconds` and `gChapterTimerSeconds_Initial` in [`Timer.c`](../../include/jester_headers/custom-structs.h) | Holds the current and initial time respectively |
| **Initialize timer** | `StartChapterTimer` in [`Timer.c`](../../Kernel/Wizardry/Misc/Timer/Timer.c) | Takes care of setting the global variables and starting the timer proc |
| **New goal type** | `GOAL_TYPE_TIMER` in `GoalDisplay_Init` [`Timer.c`](../../Kernel/Wizardry/Misc/Timer/Timer.c) | Handles the display initialization of the new goal |
| **Draw countdown** | `DrawTimeHMS` in [`Timer.c`](../../Kernel/Wizardry/Misc/Timer/Timer.c) | Handles the calculations to update the digits |
| **Update timer** | Hook into `GoalDisplay_Loop_Display` in [`Timer.c`](../../Kernel/Wizardry/Misc/Timer/Timer.c) | Call `DrawTimeHMS` here to display the new time every 60 frames |

---

## 📝 TODO

- Pause time on events

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

- The timer restarts when reaching 0 and then exiting/resuming
- Some graphics on resume have been known to slightly glitch intermittently but it's quickly resolved when the map is viewed again

---
