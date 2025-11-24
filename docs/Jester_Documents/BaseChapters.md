# Base Chapters

<p align="center">
  <img src="../Gifs/Base_Chapters.gif" alt="Base Chapters Demo" width="600"/>
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

`CONFIG_BASE_CHAPTERS`

This feature is a C-based rewrite of **Myi64’s** `Enter Town` ASM hack (not to be confused with Huicheelar’s Base Chapters system).

The goal is to provide **dedicated base maps** separate from the main campaign, similar to the **Monastery** from *Fire Emblem: Three Houses*.

### What happens in a Base Chapter?

- The **first playable unit** (in deployment order) automatically enters **Free Movement mode** (via Mokha’s hack)  
- Movement becomes subject to **real-time D-pad control** instead of turn-based tiles  
- The player can:
  - Talk to NPC units  
  - Trigger events  
  - Visit houses and shops  
  - Use arenas normally  
- The player **cannot fight enemies** on the base map outside of arenas

### Why rewrite the ASM hack?

- The original ASM version was **limited in scope**
- The other base chapter hack has a tendency to **overwrite existing chapters** unless heavily customized
- This rewrite gives **full control** over:
  - Which world map nodes count as bases  
  - Which maps are used as bases  
  - How events behave or appear based on flags, conditions, and story milestones  

---

## 🛠️ Plan

In vanilla FE8, there are **0x4F (80)** usable map slots.  
Large portions of these are duplicated between Eirika and Ephraim routes, meaning many maps are effectively **free real estate** for base map usage — even more if creating a custom story.

### How Bases Work

- You define **which world map nodes** receive base chapters  
- After finishing the chapter associated with that node, the base becomes available  
- Bases can be **entered and exited freely** afterward  
- Global flags can be checked to:
  - Spawn new units  
  - Change dialogue  
  - Update events dynamically based on story progress  

This allows bases to evolve narratively over time.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **Base chapter node assignment** | `EnterTownNodes` — [`EnterTown.c`](../../Kernel/Wizardry/Misc/EnterTown/EnterTown.c) | Controls which world map nodes become base chapters and which map each base uses |

---

## 📝 TODO

- Fix the **black screen flashing** when entering a base map  
- Improve camera behavior to **smoothly follow the player**, similar to Vesly’s *PokéEmblem*  
- Add an **escape tile** that triggers an exit event (instead of using the Retreat command)  

---

## 🐛 Limitations & Bugs

Please report issues via the repository’s **Issues** tab.

- This hack **does not extend the map list**, so you remain limited to FE8’s **0x4F maps**. This may change in the future.

---