# Help Text That Fits

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Skill and item R-text often needs more than three description lines, especially when Tellius-style **Capacity** is shown above the body text. Vanilla FE8 either truncates or (with older Vesly extended boxes) grows the help window taller and fights VRAM on the status screen.

`vesly_extended_help_boxes` is now a **three-mode** overflow setting so designers can pick truncate, expand, or paginate.

---

## 🛠️ Plan

| Mode | Value | Box height | Overflow behavior |
|------|-------|------------|-------------------|
| Vanilla | `0` | 3 lines (`0x30`) | Truncates long descriptions |
| Extended | `1` | Up to 5 lines (`0x50`) | Classic Vesly taller help box |
| Paginated | `2` | Always 3 lines | Extra description lines become pages |

Default in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) is **mode 2**.

### Paginated behavior (mode 2)

| Input / rule | Effect |
|--------------|--------|
| **A** | Advance page (wraps to page 1) |
| **B / R** | Close help |
| **D-pad relocate** | Reset to page 1 for the new R-text target |
| **Gold `n/m`** | Top-right of the help frame, same Y as the **Help** badge |
| **Capacity** | Drawn only on page 1 when Tellius capacity is enabled |

**Description lines per page** (keeps the vanilla 3-line interior):

| Pretext | Desc lines / page |
|---------|-------------------|
| None (normal R-text) | 3 |
| Capacity (1 line) | 2 on page 1; later pages use the non-capacity budget |
| Weapon labels (2) | 1 |
| Staff / other | `3 - pretext` (minimum 1) |

Page count is computed from the real description string (newline walk), not only from `GetStringTextBox` height, so A-paging and the gold indicator stay in sync with what the scroll proc draws.

### Gold page indicator

Digits reuse the status-screen page-number OBJ sheet (`0x289`, same tiles as the `3/4` tab indicator). They are drawn with a **dedicated OBJ palette** (slot 10) so the white tab `3/4` is unchanged. The fill color matches Capacity gold (`0x47DF`).

On the status screen, help-box sprite text uses `0x06012000` so the skill-capacity circle at `0x06013760` does not stomp help tiles.

---

## 🗂️ Code Locations

Gated by `gpKernelDesignerConfig->vesly_extended_help_boxes` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c).

| Feature | Location | Description |
|--------|----------|-------------|
| Mode helpers / page state | [`help-box.h`](../../include/kernel/help-box.h), `sHelpBoxPageState` in [`config-memmap.s`](../../include/link/config-memmap.s) | `HELP_BOX_MODE_*`, `HelpBoxModePaged()`, 8-byte page RAM |
| Config docs on field | [`kernel-lib.h`](../../include/kernel/kernel-lib.h) | Comment block for modes 0/1/2 |
| Page math, A input, gold `n/m`, scroll slice | [`HelpBoxHack.c`](../../Kernel/Wizardry/Core/CombatArt/HelpBoxFix/Source/HelpBoxHack.c) | `HelpBoxFinalizePageState`, `HbMoveCtrl_OnIdle`, `HelpBoxPutPageIndicatorSprites`, `HelpBoxTextScroll_OnLoop`, capacity page-0-only |
| Safe VRAM + text intro | [`HelpBox.c`](../../Kernel/Wizardry/Core/StatScreen/DrawPages/HelpBox.c) | `LoadHelpBoxGfx`, `HelpBoxIntroDrawTexts` (string count + skip) |
| Lyn body placement | [`custom_wizardry.event`](../../Kernel/Wizardry/custom_wizardry.event) | Includes `HelpBoxHack.lyn.event` in FreeSpaceDEMO (kernel text is stubs only) |

---

## 📝 TODO

- [ ] Optional screenshot of gold `1/2` next to Capacity on a multi-page skill
- [ ] Mention mode 2 in FEBuilder designer-config patch text if the patch still describes a bool

---

## 🐛 Limitations & Bugs

- Page indicator digit tiles exist on the status screen OBJ sheet; outside that context the `0x289` sheet may be missing unless something else loaded it.
- Weapon / staff pretext still occupies lines on later pages (only Capacity is stripped after page 1).
- Very long UTF-8 descriptions still depend on `[N]` / control-code newlines for paging; soft-wrap-only strings are not reflowed into pages.

Please report issues in the repository’s **Issues** tab.

---
