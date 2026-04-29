# Stat Screen – Promotions

<p align="center">
  <img src="../Gifs/Promotion_Stat_Page.gif" alt="Promotion Stat Page" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [How To Modify](#how-to-modify)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

In most Fire Emblem titles, determining **promotion paths** requires digging through internal data, reaching specific items or events, or searching external documentation.  
The game itself historically obscures this information, leaving players guessing about what classes promote into and what skills they gain.

This feature solves that problem by providing a **dedicated stat screen page** that clearly displays:

- All available promotion options for the active unit  
- Up to **three promotions per unit**  
- Up to **three skills per promotion**  
- Full **R-text integration** for skill explanations

---

## 🛠️ Plan

The system aims to give players full visibility into promotion choices before they commit.

Features include:

- Display of up to **3 promotion options**
- Display of up to **3 skills per promotion**
- Modular, per-unit customization
- Standing map sprites (SMS) and decorative platforms for each option
- R-text popups for all shown skills
- Fully integrated into the stat screen as a new page

---

## 🗂️ Code Locations

| Functionality | Location | Description |
|--------------|----------|-------------|
| **R-text redirection** | `HbRedirect_SSItem` inside `gpKernelDesignerConfig->stat_page_promotions` in [`DrawItemPage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawItemPage.c) | Routes the help box to promotion data |
| **SMS + platforms** | `PageNumCtrl_DisplayMuPlatform` in [`DrawMorePage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawMorePage/Source/DrawMorePage.c) | Handles sprite/platform display |
| **Promotion data & skill icons** | `unit_promotions` and `DrawPage7Rework` in [`DrawPage7.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage7.c) | Core promotion logic and drawing |
| **R-text logic** | [`HelpBoxPage7.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBoxPage7.c) | Determines which promotion entry is being referenced |
| **Page graphics** | `Gfx_StatScreenObj_9Pages.png` and related logic in `DisplayPageNameSprite` and `PageNumCtrl_UpdatePageNum` in [`DrawMorePage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawMorePage/Source/DrawMorePage.c) | Handles page titles and page number sprites |

---

## 🧭 How To Modify

To edit promotion options:

1. Open  
   **[`DrawPage7.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage7.c)**  

2. Find the `unit_promotions` array. Entries use the `UNIT_PROMOS` macro with `PROMO()` entries for each promotion option.

3. Example entry:
```c
UNIT_PROMOS(CHAR_MY_UNIT,
    PROMO(CLASS_PALADIN, SKILL_LANCE_MASTER, SKILL_DEFENSE_UP, SKILL_NONE),
    PROMO(CLASS_DRUID,    SKILL_MAGIC_BOOST, SKILL_NONE,       SKILL_NONE),
    PROMO(CLASS_WYVERN,  SKILL_FLYING_STR,   SKILL_NONE,       SKILL_NONE)
),
/* other UNIT_PROMOS(...) entries ... */
{0} /* terminator - DO NOT REMOVE */
```

---

### Notes / Rules

- Each UNIT_PROMOS entry corresponds to a specific unit character ID.

- A unit may have 1 to 3 PROMO() entries.

- Each PROMO(class_id, skillA, skillB, skillC) can list 0–3 skills; use a sentinel (SKILL_NONE or 0) for unused slots.

- Terminate the unit_promotions list with a final {0} — this is required by the parser and must not be removed.

---

## 📝 TODO

Consider redesigning layout to accommodate a 4th promotion option if needed.

---

## 🐛 Limitations & Bugs

R-text / Help box disabling is now handled by ``IsPromoPageCursorOnEmptySlot`` inside
``HbRedirect_SSItem`` in ``DrawItemPage.c``.  The cursor hand is redirected away from
any promotion-name or skill-icon slot that has no corresponding class or skill assigned,
so navigating with the D-Pad no longer allows focus transitions into empty slots.

Page index expectations
Some code assumes this promotions page is located at a specific index (page 7 / index 6) in ``gStatScreen.page``. If MP or personal-info pages are disabled or reordered, certain behaviors may break. Consider ensuring required pages are present or refactoring any hardcoded page-index logic.

Report further issues in the repo’s Issues tab:
https://github.com/JesterWizard/C-SkillSystem-Mokha/issues