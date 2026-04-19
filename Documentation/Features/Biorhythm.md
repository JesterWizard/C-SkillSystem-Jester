# Biorhythm

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

This system recreates the **Biorhythm** mechanic from **Fire Emblem 9**, a feature meant to simulate characters having *good days* and *bad days*.  

Depending on the current biorhythm state, a unit may experience increases or decreases to **hit** and **avoid** on each turn.

---

## 🛠️ Plan

This implementation provides:

- The ability to configure the **number of biorhythm states**
- The ability to set **per-character values** for each state
- The ability to set a **per-character starting position** within the state cycle

Units transition through their biorhythm states **one per turn**, looping back to the beginning once the cycle ends.

### Example Cycle

|  Turn  |   Hit    |    Avo    |
|--------|----------|-----------|
| 1      |   -15    |    -15    |
| 2      |    -5    |     -5    |
| 3      |     0    |      0    |
| 4      |     5    |      5    |
| 5      |    10    |     10    |
| 6      |    15    |     15    |

...etc etc


---

## 🗂️ Code Locations

### Header / Configuration
Declared in [`custom-arrays.h`](../../include/jester_headers/custom-arrays.h):

- `MAX_BIORHYTHM_STATES`  
  - Determines the size of the biorhythm array  
  - Increase for more states
- `BiorhythmPInfoConfig` struct  
  - Stores biorhythm-related properties  
  - You may add new variables (e.g., `BiorhythmSpeed`) to control cycle rate

### Implementation
Defined in [`Biorhythm.c`](../../Kernel/Wizardry/Misc/Biorhythm/Biorhythm.c):

- Supports **unique values for up to 255 units**
- You may:
  - Increase `MAX_BIORHYTHM_STATES`
  - Adjust per-unit HIT/AVO modifiers for each state

---

## 📝 TODO

_(None listed yet — add items as needed.)_

---

## 🐛 Limitations & Bugs

None currently known.

Please report new issues in the repository’s **Issues** tab.

---
