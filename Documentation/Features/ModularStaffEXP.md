# Modular Staff EXP

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

`CONFIG_MODULAR_STAFF_EXP`

In vanilla **FE8**, staff EXP gains are heavily hardcoded.  
This hack replaces that behavior with a **modular EXP system** that allows full control over how much EXP each staff use grants.

---

## 🛠️ Plan

A custom function, `StaffEXP`, acts as a centralized EXP controller.

It includes:

- A **switch-case table** that defines EXP for all vanilla staves  
- Support for **custom staves** with their own EXP values  
- Room for **conditional rules**, such as:  
  - Staves that don’t grant EXP to certain units  
  - Staves that only grant EXP in specific contexts  
  - Staves that scale EXP based on class, item durability, or map state  

This gives complete creative freedom over staff experience design.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Staff EXP logic** | `StaffEXP` — [`BattleExp.c`](../../Kernel/Wizardry/Core/BattleSys/Source/BattleExp.c) | Central function that returns EXP values for staff usage |

---

## 📝 TODO

_(No planned additions yet — add as needed.)_

---

## 🐛 Limitations & Bugs

Please report bugs or desired enhancements in the repository’s **Issues** tab.

---
