# 32-Color Portraits

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

Vanilla GBA portraits are one 4bpp OBJ sprite and one 16-color OBJ palette. Extra shades have to be quantized away, or the mug has to steal a second face bank the way [halfbodies](Halfbodies.md) do.

This system keeps tiles at 4bpp and the vanilla 128×112 sheet layout. Colors beyond the first 16 live in a second palette (`pal1`) and a second mug-sized overlay. At runtime that overlay is copied into the **paired** face VRAM bank and drawn with the same OAM template as the base mug, so extra pixels sit on the source pixels instead of being packed into spare tiles on the pal0 rows.

The hooks are always installed. `gpKernelDesignerConfig->portrait_32_color` turns the overlay path on or off without rebuilding the module. The default in `designer-config.c` is `false`.

---

## 🛠️ Plan

### How it works

`PortraitFormatter32` splits the indexed PNG into two layers:

- **pal0 / base mug:** first 16 palette colors (index 0 is transparency). Extra-color pixels are punched to 0 so the overlay can show through.
- **pal1 / overlay mug:** transparent + up to 15 extra colors. Pal0 pixels are 0.

`setMugEntry_32Color` points `imgCard` at pal1 so the runtime can tell a 32-color blob from a vanilla mug (`imgCard == pal + 16`). Chibi is at `+0x2650` so halfbody detection (`+0x2644` / `+0x2648`) does not fire.

When the flag is on, talk allocates **even** face slots only (0 and 2), the same pairing halfbodies use. Each 32-color mug occupies two of the four default face banks:

| Face slot | Base CHR / pal | Overlay CHR / pal |
|-----------|----------------|-------------------|
| **0** | `0x6000` / OBJ pal 6 | `0x5000` / OBJ pal 7 |
| **2** | `0x4000` / OBJ pal 8 | `0x7000` / OBJ pal 9 |

`Face_OnIdle` draws the vanilla sprite, then the same sprite again at the overlay CHR and pal1. Transparent overlay pixels leave pal0 visible. Mouth and eye frames still write pal0 CHR only; extra-color pixels in those tiles stay on the overlay.

At most **two** 32-color overlays are active (`OVERLAY_ACTIVE_MAX`). A third mug still draws, pal0-only. `EndFace` restores the paired palette.

### Authoring

1. Paint a vanilla-layout 128×112 indexed PNG (max 32 unique colors; first 16 are pal0, the rest are pal1 extras).
2. Install it with `PortraitFormatter32` and `setMugEntry_32Color`:

```
Portrait_0x02:
   #incext PortraitFormatter32 "Portraits/eirika_32.png"
setMugEntry_32Color(0x02,Portrait_0x02,2,6,3,4)
```

3. Set `.portrait_32_color = true` in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c), or FEBuilder field **B155** in [`PATCH_DesignerConfig.txt`](../../Patches/PATCH_DesignerConfig.txt).

Do not use `setMugEntry_32Color` on a halfbody sheet, and do not enable this flag together with `half_body_portraits` on the same face: both features claim the odd VRAM/pal bank.

---

## 🗂️ Code Locations

Gated by `gpKernelDesignerConfig->portrait_32_color` in `designer-config.c`.

| Feature | Location | Description |
|--------|----------|-------------|
| **Runtime flag** | `portrait_32_color` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Enables pal1 overlay bind/draw; default `false` |
| **FEBuilder patch** | `B155` in [`PATCH_DesignerConfig.txt`](../../Patches/PATCH_DesignerConfig.txt) | Same flag for FEBuilder |
| **Mug formatter** | [`PortraitFormatter32`](../../Tools/EventAssembler/Tools/PortraitFormatter32) | Writes pal0 mug, pal0+pal1 (`0x40`), overlay mug (`0x1000` at `+0x1644`), padded chibi |
| **Portrait table macros** | `setMugEntry_32Color` in [`Tool Helpers.txt`](../../Tools/EventAssembler/Tools/Tool Helpers.txt) | `imgChibi` at `+0x2650`, `pal` at `+0x1604`, `imgCard` at pal1 (`+0x1624`) |
| **Test mug** | `Portrait_0x02` in [`CustomPortraits.event`](../../Data/CustomPortraits/CustomPortraits.event) | Eirika FID `0x02` via `Portraits/eirika_32.png` |
| **Bind / unbind / overlay copy** | `Portrait32_BindFace`, `Portrait32_UnbindFace`, `Portrait32_LoadOverlayGfx` in [`Portrait32Color.c`](../../Kernel/Wizardry/Misc/Portrait32Color/Source/Portrait32Color.c) | Pairs slot `n` with bank `n ^ 1`, copies overlay CHR, restores pal on end |
| **Draw** | `Face_OnIdle` in [`Portrait32Color.c`](../../Kernel/Wizardry/Misc/Portrait32Color/Source/Portrait32Color.c) | Vanilla sprite then overlay sprite at pal1 |
| **Fade-in** | `Portrait32_OnFadeIn` from `StartFaceFadeIn` in [`HalfBodyPortraits.c`](../../Kernel/Wizardry/Misc/HalfBodyPortraits/Source/HalfBodyPortraits.c) | Pal-fades pal1; fade-out does not pal-fade the stolen bank |
| **Slot pairing** | `FindFreeFaceSlot` in [`HalfBodyPortraits.c`](../../Kernel/Wizardry/Misc/HalfBodyPortraits/Source/HalfBodyPortraits.c) | Step 2 when 32-color **or** halfbody is on, so talk only uses slots 0 and 2 |
| **Decompress hook** | `Face_OnInit` and `sub_8006650` in [`HalfBodyPortraits.c`](../../Kernel/Wizardry/Misc/HalfBodyPortraits/Source/HalfBodyPortraits.c) | Decompress pal0 mug, then `Portrait32_LoadOverlayGfx` |
| **Lyn jumps** | [`LynJump.event`](../../Kernel/Wizardry/Misc/Portrait32Color/Source/LynJump.event) | `$55BC` `Face_OnIdle`, `$5738` `EndFace` |
| **Installer** | [`Portrait32Color_Installer.event`](../../Kernel/Wizardry/Misc/Portrait32Color/Portrait32Color_Installer.event) | Included from [`custom_wizardry.event`](../../Kernel/Wizardry/custom_wizardry.event) after HalfBody |
| **RAM** | `sPortrait32State` in [`config-memmap.s`](../../include/link/config-memmap.s) | Four slot records + magic (`0xA4`); not suspend-persisted |

---

## 📝 TODO

- Allow more than two simultaneous 32-color mugs without overflowing hardware OAM or the four face VRAM banks.
- Cover extra-color pixels in mouth/eye frames (those CHR copies are pal0-only today).

---

## 🐛 Limitations & Bugs

- **At most two** 32-color overlays at once. A third `StartFace` is pal0-only (extra pixels are holes).
- **Talk is limited to two faces** while this flag is on, including vanilla 16-color mugs, because slot allocation walks 0 then 2.
- **Incompatible with halfbodies** on the same portrait: both want the odd CHR/pal bank. Bind skips the overlay if `half_body_portraits` is on and the mug is a halfbody blob.
- **Stat-screen BG mugs** (`PutFace80x72`) stay pal0-only; they never take the overlay path.
- **Flag off + 32-color blob:** extra-color pixels in the base mug are transparent. FID `0x02` is currently `eirika_32.png`; turn the flag on, or swap that entry back to `PortraitFormatter` / vanilla Eirika, if those holes are visible.
- GBA palettes are 5-bit. Source 8-bit extra colors are quantized.
- Please report issues in the repository’s **Issues** tab.

---
