# Skill Icon Palette Banks

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

## Introduction
Skill icons now carry a per-skill palette selector in `SkillInfo`, and the draw paths read that selector when they load or render an icon. The current data has been reset so every skill uses palette bank `0` for now (which is by default the weapon icon palette), but the engine still understands a second bank for alternate art in palette bank 1.

The important constraint is that this is a bank selector, not a free-form color expansion. Each bank is still a normal 4bpp GBA icon palette with 16 usable slots.

## Plan
The runtime keeps the skill icon graphics the same and switches only the palette bank. That gives each skill a simple `iconPal` value instead of a bespoke per-icon palette build.

| Bank | Usable color slots | Notes |
|------|--------------------|-------|
| 0 | 16 slots, indices `0-15` | Default skill icon palette bank. Slot `0` is the transparent/background entry. |
| 1 | 16 slots, indices `0-15` | Alternate skill icon palette bank. It uses the same slot rules as bank `0`. |

The actual RGB colors come from the palette art loaded for the skill icon sheet. `SkillInfo` only chooses which bank to use; it does not define the RGB values itself.

## Code Locations
| Feature | Location | Description |
|---------|----------|-------------|
| Skill palette field | [Data/SkillSys/Source/SkillInfo.c](Data/SkillSys/Source/SkillInfo.c) | Authors `iconPal` for each skill entry. |
| Generated mirror | [Data/SkillSys/Source/SkillInfo.lyn.event](Data/SkillSys/Source/SkillInfo.lyn.event) | Serialized mirror of the skill table, including the palette byte. |
| Palette lookup | [Kernel/Wizardry/Core/SkillSys/kernel/Infos.c](Kernel/Wizardry/Core/SkillSys/kernel/Infos.c) | Implements `GetSkillIconPal` and the shared `GetIconPal` routing for skill icon sheets. |
| Popup rendering | [Kernel/Wizardry/Core/SkillSys/kernel/SkillPopup.c](Kernel/Wizardry/Core/SkillSys/kernel/SkillPopup.c) | Loads the selected palette bank for skill popups. |
| Map/stat/menu rendering | [Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage7.c](Kernel/Wizardry/Core/StatScreen/DrawPages/DrawPage7.c), [Kernel/Wizardry/Core/StatScreen/DrawSkillPage/Source/MokhaPlanA/disp.c](Kernel/Wizardry/Core/StatScreen/DrawSkillPage/Source/MokhaPlanA/disp.c), [Kernel/Wizardry/Core/StatScreen/DrawSkillPage/Source/MokhaPlanB/disp.c](Kernel/Wizardry/Core/StatScreen/DrawSkillPage/Source/MokhaPlanB/disp.c) | Uses the palette selector when drawing skill icons in the stat screen. |
| Skill menus and debug views | [Kernel/Wizardry/Core/SkillSys/SkillScroll/Source/SkillScroll.c](Kernel/Wizardry/Core/SkillSys/SkillScroll/Source/SkillScroll.c), [Kernel/Wizardry/Core/SkillSys/SkillScroll/Source/RemoveSkillMenu.c](Kernel/Wizardry/Core/SkillSys/SkillScroll/Source/RemoveSkillMenu.c), [Kernel/Wizardry/Misc/SkillEffects/MenuSkills/SkillSwap.c](Kernel/Wizardry/Misc/SkillEffects/MenuSkills/SkillSwap.c), [Kernel/Wizardry/Misc/SkillEffects/MenuSkills/SkillSwapPlus.c](Kernel/Wizardry/Misc/SkillEffects/MenuSkills/SkillSwapPlus.c), [Kernel/Wizardry/Misc/VeslyDebugger/C_Code.c](Kernel/Wizardry/Misc/VeslyDebugger/C_Code.c) | Draws skill icons with the selected palette bank in menu/debug contexts. |

## TODO
- Expand the bank selector if the skill icon art pipeline ever needs more than two banks.
- Replace any hard-coded palette-bank limits with a named constant if the count grows.
- Keep `SkillInfo.c` and `SkillInfo.lyn.event` synchronized whenever palette assignments change.

## Limitations & Bugs
- This does not create extra art palettes automatically. The alternate bank still needs valid palette data in the icon pipeline.
- The current runtime only consumes bank `0` or `1`.
- Changing the bank selector does not change icon colors by itself; the palette art source under `Contents/Gfx/Sources/SkillIcon` controls the actual RGB values.