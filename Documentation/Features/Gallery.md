# Gallery

<p align="center">
  <img src="../Images/Gallery.png" alt="The Gallery: the banner with the selected CG, the grid scrolling a row, and a CG full size" width="720"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Adding a CG](#adding-a-cg)
- [Unlocking](#unlocking)
- [How it works](#how-it-works)
- [Testing](#testing)
- [Code Locations](#code-locations)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

**Extras > Gallery**, next to the Sound Room, shows the game's CGs:

- **The banner** on top (the Sound Room's song banner) shows the name of the CG under the cursor, or "???" for one
  not seen yet.
- **The panel** on the left, in a window: which CG of how many at the top, and at its foot the buttons, "View" and
  "Back".
- **The grid** on the right, in a window (the player's window colour), over the scrolling mural: three CGs a row,
  three rows on screen. Each is a small version (48x32, a fifth of the CG) in a frame; the frame of the one under the
  cursor is gold and glows. A CG not seen yet is dimmed, with a padlock.
- **Moving.** The D-pad moves the cursor; moving off the rows on screen scrolls the grid a row (no fade). L and R
  move three rows.
- **Full size.** A on an unlocked CG shows it full screen. Left and right go to the previous and next unlocked CG, around
  the ends; B goes back to the grid, on that CG.
- B on the grid goes back to Extras.

---

## ➕ Adding a CG

The Gallery shows the backgrounds listed in `gGalleryEntries` (`Data/Gallery/Gallery.c`), in that order:

```c
const struct GalleryEntry gGalleryEntries[] = {
	{ 0x38, MSG_GALLERY_CG_MYRRH_MORVA },
	...
	{ GALLERY_END, 0 },
};
```

1. Put the image in the background table first (`Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event`; see
   [Max Color Backgrounds](./MaxColorBackgrounds.md)). 16-, 224- and 256-colour backgrounds all work: a vanilla one is
   read from the clean ROM, a new one from the `.dmp` its `BG.event` label `#incbin`s.
2. Add a row: its background index and a name message. Add the message at the end of
   `Contents/Texts/Source/texts/Districts.txt` (`## MSG_GALLERY_CG_...`, then the name and `[X]`).
3. Show it in an event (`TEXT_BG`, `TEXT_BG_HIDE_MAP`, `BACG` and the background fades), and it unlocks there.

The build makes the thumbnails (see [How it works](#how-it-works)) again whenever the list, `BG.event` or a `.dmp`
changes.

At most 96 rows (`GALLERY_MAX`). The list starts with the three custom CGs (0x38-0x3A) and the ten vanilla backgrounds
the story shows so far. Its order is the unlock bits' order: insert a row in the middle and the unlocks already made
move to the rows after it, so add new CGs at the end.

---

## 🔓 Unlocking

A CG unlocks the first time an event shows its background, in any save file, and stays unlocked for all of them, as
the Sound Room's songs do. Event command `0x21` (`gEventLoCmdTableRe`) is `Gallery_Event21`: for `TEXT_BG` / `BACG`,
the background transition and the fade in, it unlocks the background, then runs vanilla's `Event21_TextBg`. A skipped
scene counts. The fade's "fade out" form passes background 0, so it unlocks nothing.

The unlocks are 96 bits behind a magic word ("GLRY") in the SRAM entry vanilla keeps for the e-Reader trial map's save
block (`gSram->saveBlockInfo[SAVE_ID_XMAP]`, 16 bytes at `0x0E0000C4`). This hack has no trial maps, and that entry
is only read by the trial-map check, which sees no trial map. It is outside the game and suspend saves, so the save
layout and its signature do not change, and erasing a file keeps them. Only wiping all of SRAM (vanilla's first boot)
clears them.

---

## 🛠️ How it works

- **The Extras row.** Vanilla's Map row (bit 4), which the [Creature Campaign](./WorldMapStoryChapters.md#the-postgame)
  no longer needs. Its label is drawn in the Extras label sheet (`Kernel/Wizardry/NewGamePlus/Gfx/Img_NgpMenuObjs.png`)
  where "Map" was. `InitSaveMenuChoice` adds it; the Extras list (`sub_80A9B90`) opens it like the Sound Room;
  `SaveMenuStartExtraMiscScreen` and `SaveMenuPostExtraMiscScreen` are rewritten with a case for it.
- **The thumbnails** are build output. `Tools/gallery/thumbs.py` (a makefile rule, `Data/Gallery/GalleryThumbs.bin`,
  gitignored) decodes each listed background: a vanilla one from `fe8.gba` (LZ77 4bpp tiles, its TSA, 8 palettes), a
  224- or 256-colour one from its `.dmp` files. It shrinks the picture to 48x32 (the average of each 5x5 block), picks
  15 colours for it (median cut) and writes 768 bytes of 4bpp tiles in the sprites' order and a 16-colour palette:
  800 bytes a row, `gGalleryThumbs` (`Data/Gallery/Gallery.event`). Made from the ROM at build time, so the repo holds
  none of vanilla's art.
- **The screen.** BG0 the text (the banner's name, centred on it; the panel); BG1 the windows round the panel and
  the grid (`DrawUiFrame`, the player's window style, BG palette 1); BG2 the banner and the buttons; BG3 the mural
  (`StartMuralBackground`, palettes 14 and 15). Vanilla's sprite sheets are unpacked into spare OBJ VRAM
  (`0x06014000`) and the tiles wanted copied to BG VRAM: the banner from `Img_PlayStatusSprites` (the Sound Room's song
  banner, 24x4 tiles, palette 11), the A button from `Img_SoundRoomUiElements` (the Sound Room's round A, palette 10).
  Vanilla has no round B: the B is a copy of the A with its letter redrawn (`sLetterB`, in the A's ink, its soft shade
  and its gold), edited in RAM and copied over (VRAM takes no byte writes). Sprites: the thumbnails (a 32x32 and a
  16x32 each, 1D mapping), their frames (`Gfx/Img_GalleryCursor.png`, a 64x64 sprite each: grey, OBJ palette 13, or
  the cursor's gold, OBJ palette 12, which glows) and the padlocks (`Gfx/Img_GalleryLock.png`, OBJ palette 15).
- **The grid** fills the window's inside (x 72-231, y 40-151): a CG and its frame is 52x36, 2 pixels apart. Each
  place on the grid has a slot: its thumbnail's tiles in OBJ VRAM and its palette in OBJ palettes 0-11 (dimmed when
  locked), loaded again whenever the rows are drawn.
- **Scrolling.** Moving the cursor off the three rows on screen draws four rows (the next one below, or above) and
  slides the sprites 38 pixels, 8 a frame; then the three rows on screen are drawn again, still. Window 0 is the
  window's inside: outside it, over the window's edge, the banner and the panel, the sprites do not show.
- **The full view.** The game's own loader (`EventShowTextBgDirect`, which Max Color Backgrounds hooks for 224- and
  256-colour CGs) on BG3 alone, the mural stopped; it starts again on the way back to the grid.

---

## 🧪 Testing

`python3 Tools/cloud/smoketest.py --only gallery`: the prologue's opening unlocks Mountain Forest (0x1D) and Seth
Attacked (0x3A) in SRAM; after a reset Extras has the Gallery row; the grid opens on its first rows; A on a locked CG
does nothing; A on Mountain Forest shows it full size; right goes to Seth Attacked (around the end); B goes back to the
grid on it; down three rows scrolls the grid a row, on screen; B again goes back to Extras.

---

## 🗂️ Code Locations

| What | Where |
|---|---|
| The CG list | `gGalleryEntries` in `Data/Gallery/Gallery.c` |
| Unlocks, the event hook, the Extras screens | `Kernel/Wizardry/Gallery/Source/Gallery.c` |
| The screen | `Kernel/Wizardry/Gallery/Source/GalleryScreen.c` |
| LynJump, art, the event-table patch | `Kernel/Wizardry/Gallery/Source/LynJump.event`, `Gallery_Installer.event` |
| Frame and padlock art | `Kernel/Wizardry/Gallery/Gfx/` (the buttons and the banner are vanilla's) |
| The thumbnails | `Tools/gallery/thumbs.py`, the `GALLERY_THUMBS` rule in `makefile`, `Data/Gallery/Gallery.event` |
| The Extras row | `InitSaveMenuChoice` (`Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c`); `sub_80A9B90` (`Kernel/Wizardry/NewGamePlus/Source/NgpSaveMenu.c`) |
| Struct, constants | `include/kernel/gallery.h` |
| RAM | `gGalleryScreen` in `include/link/config-memmap.s` |
| Messages | `MSG_GALLERY_*` at the end of `Contents/Texts/Source/texts/Districts.txt` |
| Smoke test | `test_gallery` in `Tools/cloud/smoketest.py` |

---

## 🐛 Limitations & Bugs

- A thumbnail has 15 colours: fine detail and smooth gradients are rough at that size.
- The build needs Python 3 for the thumbnails (no other packages).
- Only backgrounds shown through event command `0x21` unlock. A CG drawn some other way (an ASMC, vanilla's CG
  table) needs a `0x21` command in its event too, or `Gallery_UnlockBackground(bg)`.
- The unlocks live outside the saves: copying a save to another cartridge does not carry them.
