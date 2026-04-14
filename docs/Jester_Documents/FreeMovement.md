# Free Movement

---

## 📑 Index
- [Introduction](#introduction)
- [How To Use](#how-to-use)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

This feature enables **free movement**, allowing units to move around the map in a style similar to traditional **JRPG overworld movement**, rather than being restricted to their standard Fire Emblem movement range.

This fork includes the **ASM Sme** implementation of free movement by default.

---

## 🚀 How To Use

Free movement can be enabled from any `.c` or `.h` file that includes:

```c
#include "jester_headers/custom-functions.h"
```

To enable it in a chapter, add this function as an ASMC to your chapter start event:

```event
ASMC(EnableFreeMovementASMC);
```

Once called, the player gains control of free movement for the active unit.

## 🐛 Limitations & Bugs

Please be aware of the following limitations:

- Free movement disables itself after saving and suspending a chapter

- Only the first deployed unit is supported — do not attempt to activate free movement for multiple units simultaneously

- You cannot interact with enemies to initiate combat, nor can you use standard commands like Trade, Rescue, etc. while in free movement mode

Please report additional issues in the repository’s Issues tab.