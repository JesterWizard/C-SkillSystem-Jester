# Bonus EXP

<p align="center">
  <img src="../Gifs/Base_Conversations.gif" alt="Bonus EXP" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [How-To-Use](#How-To-Use)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

``gpKernelDesignerConfig->prep_menu_base_conversations``

This feature is a reimplementation of Snek's prep menu base conversations.

The aim is to ensure:

- It works with C Skill System
- No custom structs are required
- No hijacking of the support structs are required
- The user can determine what background, music and items are made available per conversation
- Played conversations can be replayed, but awarded items are not awarded again
- An unlimited number of base conversations can be handled per chapter, courtesy of my list scroller

---

## 🛠️ How To Use

- Inside [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) set the `.prep_menu_base_conversations` option to true.
- Set the maximum number of conversations you want to have per chapter in the ``ChapterConversations`` struct.
- Set the conversation titleid, textid, background, music, item to award and flag index in ``gBaseConversationTable``

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **RAM - Allocation 1** | `gBaseConversations_Total` in [`config-memmap.s`](../../include/link/config-memmap.s) | Holds RAM allocations for various global variables and arrays |
| **RAM - Allocation 2** | `gBaseConversations_Flags` in [`config-memmap.s`](../../include/link/config-memmap.s) | Holds RAM allocations for various global variables and arrays |
| **Base Conversation table** | `gBaseConversationTable` in [`BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) | Handles all elements of the convos |
| **Get number of conos per chapter** | `NumberOfChapterBaseConversations` in [`BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) |
| **Redraw UI elements** | `DrawBaseConversations` in [`BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) | Redraw various UI elements when required |
| **Initialize UI elements** | `PrepInitGfx_BASE` in [`BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) | Set up backgrounds and UI elements |
| **Frame loop** | `PrepLoop_MainKeyHandler_BASE` in [BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) | The loop that runs every frame check for button states etc |
| **Popup proc for awarding items** | `BasePopup` in [BaseConversations.c`](../../Kernel/Wizardry/Core/SkillSys/PrepSkill/Source/CustomMenuOptions/BaseConversations.c) | The proc that handles the notifying of item awards |
| **Text strings for convos** | [BaseConversations.txt`](../../Data/FE8_Rewritten_Terper/Text/BaseConversations/BaseConversations.txt) | The text file that handles the storing of base convo titles and text conversations |

---

## 📝 TODO

- Turn off base maps or fade it out as a prep screen option if there are no base conversations to have for a chapter

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

---