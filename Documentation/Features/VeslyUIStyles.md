# VeslyUI Styles

<p align="center">
  <img src="../Images/VeslyUI_Wood.png" alt="Wood UI style" width="600"/>
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

VeslyUI swaps the game's menu and battle UI graphics for alternative styles. Every style keeps vanilla's tile layouts, so the game's own tilemaps still work; only the art and palettes change.

Player-facing impact:

- Window frames (unit, item and map menus, the unit list, guide, options, help and info windows, the map HUD) change look together.
- The battle screen's name plates, weapon boxes and HIT/DMG/CRT panels follow the style, tinted by faction.
- Prep screens, the prep popup, the funds/items sprites and the shop gold box follow the style.
- Faction-coloured frames (battle forecast, status screen counters, world map) use the style's palettes.

**Wood** is the newest style. Its windows are grained boards framed by dark, black-outlined wooden planks. The edges are chipped and cracked, and at each corner the top and bottom planks run past the window and end in a broken, splintered tip. Vanilla's gold bars, scroll curls and gem crests are gone, including the crest on the battle name plates and the scrollwork on the HP panel. Four woods replace vanilla's window colours.

| Window colour | Vanilla | Wood | Faction it tints |
|---------------|---------|------|------------------|
| 0 | Blue | Oak | Player |
| 1 | Red | Mahogany | Enemy |
| 2 | Green | Moss-stained | Allied (green) |
| 3 | Sepia | Weathered grey | Other |

---

## 🛠️ Plan

### Choosing a style

A style is picked by a permanent event flag, checked in this order:

| Order | Style | Flag | ID |
|-------|-------|------|----|
| 1 | Wood | `WoodStyleFlag` (`0xAB`) | 4 |
| 2 | Stephano menus / Sokaballa battle | `StephanoStyleFlag` (`0xAA`) | 1 |
| 3 | Gamma | `GammaStyleFlag` (`0xB4`, the guide flag) | 2 |
| 4 | Pikmin menus / Fenreir battle | `PikminStyleFlag` (`0xAC`) | 3 |
| — | Vanilla | none set | 0 |

Set the flag from an event (for example `ENUT 0xAB` for wood, `ENUF 0xAB` to turn it off). The change shows the next time a screen loads its UI graphics, such as after opening the stat screen or entering battle. `gpKernelDesignerConfig->vesly_custom_ui` must be on.

Wood is checked first because the campaign sets the guide flag early, which would otherwise always select Gamma.

### What a style supplies

Each getter in `C_Code.c` indexes a 5-entry table by the style ID. A style that has no art for an entry reuses vanilla's.

| Area | Wood asset | Vanilla it replaces |
|------|------------|---------------------|
| Window frame tiles | `Wood_Main.png` | `gUiFrameImage` |
| Menu palettes (8 rows) | `Wood_MenuPal.pal` | `gUiFramePaletteA`-row block |
| Prep screen extras | `Wood_Preps.png`, `Wood_PrepsUseGfx.png` | `gUnknown_08A1B730`, `Img_PrepItemUseScreen` |
| Prep funds/items sprites | `Wood_PrepItems.png`, `Wood_PrepItemsPal.pal` | `gUnknown_08A1B1FC`, `gUnknown_08A1B638` |
| Prep popup | `Wood_PrepPopupWindow.png` | `Img_PrepPopupWindow` |
| Shop gold box | `Wood_ShopGoldDisplay.png` | `Img_ShopGoldBox` |
| Battle frame | `Wood_Battle.png`, `Wood_BattlePal.pal` | `Img_08801C14`, `gUnknown_08802558` |
| Battle name/item boxes | `Wood_Left*.png`, `Wood_Right*.png` | `Img_EfxLeft/Right Name/ItemBox` |

### Building the wood art

`png/Wood/generate_wood.py` builds every wood file from the vanilla images in `png/Vanilla`. It keeps each image's tile layout and window outline, so the game's tilemaps still line up, and redraws what is inside:

- Gold trim bars become beams: outlined, with shading set by each pixel's position across the bar.
- Every edge that meets the transparent backdrop gets a black outline.
- Ornament tiles (curls, crests, the battle gem) are replaced with the plain tile of the same role (`PLAIN_COPIES`, `BATTLE_COPIES`).
- Corner tiles become broken plank ends (`CORNERS`, `BATTLE_CORNERS`, `CORNER_ART`).
- Beam and edge tiles get chips, cracks and grain streaks (`EDGE_CHIPS`); other tiles get an occasional small dent.

In wood mode, `DrawUiFrame` (replaced in `C_Code.c`) picks each top beam, bottom beam and left edge tile from a set of interchangeable ones by position. So the chips along a window don't repeat every 8 pixels. The tile sets in `C_Code.c` must match the `EDGE_CHIPS` tiles.

It then swaps in the wood palettes and adds a wood-grain pattern to the fill colours. The pattern repeats every 8×8 tile, so repeated tiles join into continuous grain. The installer converts the PNGs with `Png2Dmp --lz77` at build time.

To retune the look, edit the colour lists at the top of the script and rerun it. `Png2Dmp` maps pixels by colour, so each PNG palette must keep 16 distinct colours (the script checks this).

### Adding another style

1. Add art matching the vanilla tile counts (start from `png/Vanilla`), plus palettes.
2. Add labels for it in `VeslyUI_Installer.event` and a flag.
3. Add an entry to every table in `C_Code.c` and a flag check in `GetUI_id`.
4. Add a case to `GetMMBWindowPaletteRow` in `ModularMinimugBox.c`.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Style selection | `GetUI_id`, `GetUIPalID` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Reads the style flags in priority order |
| Window frames | `GetUiFrame`, `UnpackUiFrameImage`, `UnpackUiFrameBuffered`, `UnpackUiFramePalette` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Frame tiles and the window-colour palette |
| Prep screens | `PrepInitGfxHook`, `Decompress_Img_PrepItemUseScreen`, `DrawFundsSprite_Init`, `PutImg_PrepPopupWindow`, `PrepUiPalHook_2`, `PrepUiPalHook_3` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Prep art, funds/items sprite and palettes |
| Shop | `StartUiGoldBox`, `StartUiGoldBox_New` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Gold box sprite and palette |
| Battle screen | `EfxPrepareScreenFx`, `ekrDispUPMain`, `GetBPal_8802558` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Battle frame, name/item boxes, faction palettes |
| Faction palettes | `GetFactionBattleForecastFramePalette`, `StatusScreenUIPal_Hook`, `sub_80BE5B4` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Forecast, status screen and world map tints |
| Minimug box | `GetMMBWindowPaletteRow` in [`ModularMinimugBox.c`](../../Kernel/Wizardry/ModularMinimugBox/ModularMinimugBox.c) | Map HUD unit window palette per style |
| Assets | [`VeslyUI_Installer.event`](../../Kernel/Wizardry/VeslyUI/VeslyUI_Installer.event), [`png/`](../../Kernel/Wizardry/VeslyUI/png/) | Style flags, art and palettes |
| Wood generator | [`png/Wood/generate_wood.py`](../../Kernel/Wizardry/VeslyUI/png/Wood/generate_wood.py) | Builds the wood art from `png/Vanilla` |
| Wood frame tiles | [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | `DrawUiFrame` replacement that mixes chipped beam tiles in wood mode |

---

## 📝 TODO

- Bring the screens below under the style system so every style covers them.
- A player-facing way to pick a style (an Options entry) instead of event flags.
- More styles built the same way.

---

## 🐛 Limitations & Bugs

- Styles only reach the screens listed above. Still vanilla in every style: the unit stat screen, the map battle HP windows and EXP bar (shown when battle animations are off), the status screen's chapter title banner and play-time box, and the phase banners.
- A style change does not redraw windows already on screen; it shows when a screen next loads its UI graphics.
- Wood's prep screen, prep popup and shop gold box art follow vanilla's tile layout but have not been checked in-game yet.
- Each style adds roughly the size of its compressed art to the ROM.

Please report issues in the repository’s **Issues** tab.
