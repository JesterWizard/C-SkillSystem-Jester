# VeslyUI Styles

<p align="center">
  <img src="../Images/VeslyUI_Wood.png" alt="Wood UI style" width="600"/>
</p>
<p align="center">
  <img src="../Images/VeslyUI_Tech.png" alt="Tech UI style" width="600"/>
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
- The status screen's chapter title banner and play-time box can follow the style (wood and tech have their own; the other styles keep vanilla's).

**Wood** and **Tech** are the newest styles, both built by the generator described below.

**Wood** Its windows are grained boards framed by dark, black-outlined wooden planks. The edges are chipped and cracked, and at each corner the top and bottom planks run past the window and end in a broken, splintered tip. Vanilla's gold bars, scroll curls and gem crests are gone, including the crest on the battle name plates and the scrollwork on the HP panel. Four woods replace vanilla's window colours.

| Window colour | Vanilla | Wood | Faction it tints |
|---------------|---------|------|------------------|
| 0 | Blue | Oak | Player |
| 1 | Red | Mahogany | Enemy |
| 2 | Green | Moss-stained | Allied (green) |
| 3 | Sepia | Weathered grey | Other |

**Tech** has a techno-futuristic look. Its windows are dark glass panels with scanlines. Each edge is one bright neon line, the same thickness on all four sides, with a dim glow just outside it and no black outline. Inside the line is a 2-pixel band of panel: invisible on dark panels, an even inset line on light ones. Corners are rounded, the line cutting across them in two white-hot steps. The edges are otherwise plain and symmetrical. Light panels such as title plates and help boxes become pale glass. The chapter name on the status screen is white and cyan.

| Window colour | Vanilla | Tech | Faction it tints |
|---------------|---------|------|------------------|
| 0 | Blue | Cyan | Player |
| 1 | Red | Magenta | Enemy |
| 2 | Green | Green | Allied (green) |
| 3 | Sepia | Violet | Other |

---

## 🛠️ Plan

### Choosing a style

A style is picked by a permanent event flag, checked in this order:

| Order | Style | Flag | ID |
|-------|-------|------|----|
| 1 | Wood | `WoodStyleFlag` (`0xAB`) | 4 |
| 2 | Tech | `TechStyleFlag` (`0xAD`) | 5 |
| 3 | Stephano menus / Sokaballa battle | `StephanoStyleFlag` (`0xAA`) | 1 |
| 4 | Gamma | `GammaStyleFlag` (`0xB4`, the guide flag) | 2 |
| 5 | Pikmin menus / Fenreir battle | `PikminStyleFlag` (`0xAC`) | 3 |
| — | Vanilla | none set | 0 |

Set the flag from an event (for example `ENUT 0xAB` for wood or `ENUT 0xAD` for tech, `ENUF` to turn it off), or from C with `SetFlag`. The change shows the next time a screen loads its UI graphics, such as after opening the stat screen or entering battle. `gpKernelDesignerConfig->vesly_custom_ui` must be on.

Wood and Tech are checked first because the campaign sets the guide flag early, which would otherwise always select Gamma.

### What a style supplies

Each getter in `C_Code.c` indexes a table by the style ID (one entry per style). A style that has no art for an entry reuses vanilla's. Tech's files follow the same names with a `Tech_` prefix in `png/Tech`.

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
| Status screen banner and play-time box | `Wood_PlayStatus.png`, `Wood_PlayStatusPal.pal` | `Img_PlayStatusSprites`, `Pal_PlayStatusSprites` |

### Building the wood and tech art

`png/style_tools.py` holds the shared redraw. Each style's script (`png/Wood/generate_wood.py`, `png/Tech/generate_tech.py`) sets the look and calls `build_style`, which builds every file for that style from the vanilla images in `png/Vanilla`. It keeps each image's tile layout and window outline, so the game's tilemaps still line up, and redraws what is inside:

- Gold trim bars are redrawn by the style's `beam`, from each pixel's position across the bar: wooden beams for wood; for tech, the neon edge line and its band, with the rest of the bar left transparent (a neon line on a dark rail between two panels).
- Every edge that meets the transparent backdrop gets the style's `OUTLINE`: black for wood, the neon line for tech (which also adds a glow outside it).
- Ornament tiles (curls, crests, the battle gem) are replaced with the plain tile of the same role (`PLAIN_COPIES`, `BATTLE_COPIES`).
- Corner tiles come from the style's `CORNER_ART`: broken plank ends for wood, rounded neon corners for tech (`CORNERS`, `BATTLE_CORNERS`).
- The status screen's chapter title banner and play-time box are rebuilt from the finished frame tiles, so they match the windows. The play-time box keeps vanilla's letters on a small plate (`play_status`). It is 96 pixels wide instead of 80 so the clock fits, so `StatusScreenSpriteDraw_Loop` in `C_Code.c` draws it with a wider sprite for these styles.
- Beam and edge tiles get the style's details (`edge_details`): chips, cracks and grain streaks for wood (`EDGE_CHIPS`, plus an occasional small dent on other tiles); for tech, the side lines move one pixel out to leave the band, and every lit edge gets its glow.

For these styles, `DrawUiFrame` (replaced in `C_Code.c`) picks each top beam, bottom beam and left edge tile from a set of interchangeable ones by position. So wood's chips along a window don't repeat every 8 pixels. Tech's beam tiles all look the same, so the mixing makes no difference there. The tile sets in `C_Code.c` (`sFrameTopBeams` and friends) must match `TOP_BEAMS` and friends in `style_tools.py`.

Last, it swaps in the style's palettes and adds its fill texture: wood grain, or tech scanlines. Both repeat every 8×8 tile, so repeated tiles join up. The installer converts the PNGs with `Png2Dmp --lz77` at build time.

To retune the look, edit the colour lists at the top of the script and rerun it. `Png2Dmp` maps pixels by colour, so each PNG palette must keep 16 distinct colours (the script checks this).

### Adding another style

1. Add art matching the vanilla tile counts, plus palettes. The quickest way is a new `png/<Name>/generate_<name>.py` modelled on the tech or wood script; `style_tools.py` lists the names a style script defines.
2. Add labels for it in `VeslyUI_Installer.event` and a flag.
3. Add an entry to every table in `C_Code.c` and a flag check in `GetUI_id`.
4. Add a case to `GetMMBWindowPaletteRow` in `ModularMinimugBox.c`.
5. For a generated style, add its ID to `IsGeneratedStyle` in `C_Code.c` (frame tile mixing and the wide play-time box) and an entry to `sChapterTitlePal`.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Style selection | `GetUI_id`, `GetUIPalID` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Reads the style flags in priority order |
| Window frames | `GetUiFrame`, `UnpackUiFrameImage`, `UnpackUiFrameBuffered`, `UnpackUiFramePalette` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Frame tiles and the window-colour palette |
| Prep screens | `PrepInitGfxHook`, `Decompress_Img_PrepItemUseScreen`, `DrawFundsSprite_Init`, `PutImg_PrepPopupWindow`, `PrepUiPalHook_2`, `PrepUiPalHook_3` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Prep art, funds/items sprite and palettes |
| Shop | `StartUiGoldBox`, `StartUiGoldBox_New` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Gold box sprite and palette |
| Battle screen | `EfxPrepareScreenFx`, `ekrDispUPMain`, `GetBPal_8802558` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Battle frame, name/item boxes, faction palettes |
| Status screen banners | `ChapterStatus_ShowAllLayers`, `StatusScreenSpriteDraw_Init`, `StatusScreenSpriteDraw_Loop` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Chapter title banner, play-time box and chapter name colours |
| Faction palettes | `GetFactionBattleForecastFramePalette`, `StatusScreenUIPal_Hook`, `sub_80BE5B4` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Forecast, status screen and world map tints |
| Minimug box | `GetMMBWindowPaletteRow` in [`ModularMinimugBox.c`](../../Kernel/Wizardry/ModularMinimugBox/ModularMinimugBox.c) | Map HUD unit window palette per style |
| Assets | [`VeslyUI_Installer.event`](../../Kernel/Wizardry/VeslyUI/VeslyUI_Installer.event), [`png/`](../../Kernel/Wizardry/VeslyUI/png/) | Style flags, art and palettes |
| Style generators | [`png/style_tools.py`](../../Kernel/Wizardry/VeslyUI/png/style_tools.py), [`png/Wood/generate_wood.py`](../../Kernel/Wizardry/VeslyUI/png/Wood/generate_wood.py), [`png/Tech/generate_tech.py`](../../Kernel/Wizardry/VeslyUI/png/Tech/generate_tech.py) | Build the wood and tech art from `png/Vanilla` |
| Frame tile mixing | `DrawUiFrame`, `PickFrameTile`, `IsGeneratedStyle` in [`C_Code.c`](../../Kernel/Wizardry/VeslyUI/C_Code.c) | Mixes interchangeable beam tiles for the generated styles |

---

## 📝 TODO

- Bring the screens below under the style system so every style covers them.
- A player-facing way to pick a style (an Options entry) instead of event flags.
- More styles built the same way.

---

## 🐛 Limitations & Bugs

- Styles only reach the screens listed above. Still vanilla in every style: the unit stat screen, the map battle HP windows and EXP bar (shown when battle animations are off), and the phase banners. The status screen's chapter title banner and play-time box only change for wood and tech.
- A style change does not redraw windows already on screen; it shows when a screen next loads its UI graphics.
- Wood's and tech's prep screen, prep popup and shop gold box art follow vanilla's tile layout but have not been checked in-game yet.
- Each style adds roughly the size of its compressed art to the ROM.

Please report issues in the repository’s **Issues** tab.
