# Stat Screen – Chapter Locks

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

The expanded stat screen supports several optional pages, but not all of them are meant to appear from chapter 1 onward.
This system uses a small chapter-lock table so later pages unlock gradually as the campaign progresses.

At the moment, the lock logic applies to the three optional late pages:

| Page | Page Constant | Unlock Chapter |
|------|---------------|----------------|
| Gaiden Magic | `PAGE_GAIDEN_MAGIC` | `CHAPTER_06` |
| Personal Data | `PAGE_PERSONAL_DATA` | `CHAPTER_08` |
| Promotions | `PAGE_PROMOTIONS` | `CHAPTER_10` |

The first four stat screen pages are always available unless some separate feature-specific code changes their behavior.

## Plan

The chapter-lock flow is intentionally simple:

1. A page must first be enabled in `gpKernelDesignerConfig`.
2. If that page has an entry in the chapter-lock table, the current `gPlaySt.chapterIndex` must be at or past the unlock chapter.
3. Only pages that pass both checks are counted into the visible stat screen page total.
4. Because locked pages disappear from the visible page list, visible page ids are translated back into their real internal ids before drawing or opening help text.

This means page visibility is not just cosmetic.
The same availability check affects page count, page-number display, draw dispatch, and help-box routing.

### Current Lock Table

`sStatScreenPageChapterLocks` in `HelpBox.c` is the authoritative source for chapter unlocks:

```c
static const struct StatScreenPageChapterLock sStatScreenPageChapterLocks[] = {
    { PAGE_GAIDEN_MAGIC, CHAPTER_06 },
    { PAGE_PERSONAL_DATA, CHAPTER_08 },
    { PAGE_PROMOTIONS, CHAPTER_10 },
};
```

### Behavior Notes

- `GetStatScreenPageUnlockChapter(page)` returns `-1` when a page has no chapter lock entry.
- `IsStatScreenPageAvailable(page)` combines config gating and chapter gating in one place.
- `GetStatPageCount()` starts from a base count of `4`, then adds each unlocked optional page.
- `TranslateStatPageId()` skips over locked pages so visible page slots stay contiguous.

### Practical Example

If `PAGE_PROMOTIONS` is enabled in config but the player is still before `CHAPTER_10`, the promotions page is treated as unavailable.
It will not appear in the page count, page name display, draw routing, or help-box selection.

Once the campaign reaches `CHAPTER_10`, the same page becomes visible without needing separate per-page code changes.

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Page constants and API | `stat-screen.h` in [`include/kernel/stat-screen.h`](../../include/kernel/stat-screen.h) | Defines `PAGE_GAIDEN_MAGIC`, `PAGE_PERSONAL_DATA`, `PAGE_PROMOTIONS`, and the chapter-lock helper declarations |
| Lock table and availability checks | `sStatScreenPageChapterLocks`, `GetStatScreenPageUnlockChapter`, `IsStatScreenPageAvailable`, and `TranslateStatPageId` in [`HelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBox.c) | Central source of truth for chapter locks and page-id remapping |
| Page drawing and help routing | `DisplayPage` and `StartStatScreenHelp` in [`HelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBox.c) | Uses translated page ids so hidden pages do not break draw/help dispatch |
| Visible page count | `GetStatPageCount` in [`DrawMorePage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawMorePage/Source/DrawMorePage.c) | Builds the final page total shown to the player |
| Promotion-page help-box cursor safeguards | `HbRedirect_SSItem` in [`DrawItemPage.c`](../../Kernel/Wizardry/Core/StatScreen/DrawItemPage/Source/DrawItemPage.c) | Contains extra promotions-page handling that assumes the page has already passed availability checks |

## TODO

- Add a general stat screen feature index so this document and `StatScreenPromotions.md` are grouped together.
- Decide whether unlock chapters should stay hardcoded in C or move into a config/event-facing table.
- Document any future locks if more optional pages are added beyond Gaiden Magic, Personal Data, and Promotions.

## Limitations & Bugs

The current lock system is hardcoded.
Anyone changing unlock timing must edit and rebuild `HelpBox.c`; there is no data-driven event hook for chapter lock values yet.

Because page compression is handled by `TranslateStatPageId()`, any code that directly assumes a fixed visible page number can still break if optional pages are disabled or locked.
Contributors should prefer the page constants plus `IsStatScreenPageAvailable()` instead of hardcoding visible indices.
