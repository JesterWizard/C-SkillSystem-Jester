# Tutorials

<p align="center">
  <img src="../Images/Tutorials.png" alt="The Extras list with Tutorials, and the Tutorials screen on Skill Capacity" width="720"/>
</p>

*Extras > Tutorials, the last Extras row, opens Select Mode's screen: the tutorials on its banners in the menu labels'
letters, the chosen one's text in its box.*

---

## 📑 Index
- [Introduction](#introduction)
- [The tutorials](#the-tutorials)
- [Adding a tutorial](#adding-a-tutorial)
- [How it works](#how-it-works)
- [Testing](#testing)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

**Extras > Tutorials**, the last Extras row (under the Skill Codex), explains the hack's own features:

- **The banners** on the left are Select Mode's, three on screen in its green, teal and red; the cursor's banner is
  lit, with Select Mode's hand beside it. Each shows a tutorial's title in the letters of the menus' labels (the
  Extras list's, the header's "Tutorials").
- **Scrolling.** Up and down choose a tutorial. Past the third banner the list scrolls under them; a gold arrow above
  or below the banners shows there are more that way. Down on the last tutorial goes around to the first, and up on
  the first to the last (on a press, not while the key is held).
- **The box** on the right is Select Mode's description box. It shows the chosen tutorial's text in its five lines.
  Every tutorial fits the box; a longer text would show four lines a page, with the page and the page count in gold
  on the last line ("2/3"), and A or right would show the next page, left the one before.
- **The header** is Select Mode's, with "Tutorials" in the Extras labels' letters, centred in it.
- B goes back to Extras.

---

## 📚 The tutorials

| Title | About |
|---|---|
| Tutorials | How this screen works |
| Skills | Where skills come from; five more can be equipped |
| Equipping Skills | The Skills row of Preparations |
| Skill Capacity | The cost of each rank; 50 capacity, 75 promoted |
| Skill Scrolls | Scrolls; the Skill Codex |
| Magic | Spells learned by level, not items |
| Casting | B.Mag and W.Mag |
| Spell Uses | Uses per chapter |
| Combat Arts | Choosing an art in the forecast |
| Using Arts | What arts do; most stop follow-ups |
| Weapons | Weapons never break; the Stack command |
| Districts | What they are; the green flag |
| District Openings | The chapter each opens at |
| Visiting | Enter District, talking, leaving |
| Fishing | The fishing spot, bait, casting |
| Reeling In | Hooking and reeling |
| Catches | Pay-out, the golden fish, tokens |
| River Crossing | The frog spot, hopping, docks |
| Crossing Rules | Lives, time, pay-out |
| Tower Stack | The tower spot, dropping blocks |
| Tower Rules | Perfects, lives, Fever |
| Hex Cards | Hex players; the prize |
| Hex Rules | Placing cards, attacking, switching sides |
| Guild Board | The board, pages, the red ! |
| Tokens | Claiming quests; prizes |
| Enemy Promotions | Enemies promote when they defeat a unit |
| Promote On Kill | The Options row |
| Stat Screen | R on a unit, turning pages |
| Stat Pages | Growths, Gaiden Magic, Promotions |
| Threat Arrows | The Options row and what the arrows show |

Each is one box: a topic that needs more is split over several tutorials, one after another.

Left out on purpose: Pair Up (nothing in the story unlocks it yet), and anything off by default (Weapon Traits,
Biorhythm, Gambits, the Skill Tree and the Skill Shop).

---

## ➕ Adding a tutorial

1. Add two messages at the end of `Contents/Texts/Source/texts/Districts.txt` (a message anywhere else moves every later
   message's id, CLAUDE.md):

   ```
   ## MSG_TUT_EXAMPLE_TITLE
   Example[X]
   ## MSG_TUT_EXAMPLE
   The whole text, as one paragraph. The screen wraps it to the box.[X]
   ```

   - The title must fit a banner: 128 pixels. It may use any letter `Tools/tutorial-titles/titles.py` has (the
     script stops on one it lacks: draw it in `DRAWN` there).
   - The text is wrapped at 88 pixels (`TEXT_PX`). Keep it to the box's five lines, about 90 characters: split a longer
     topic over two tutorials. The smoke test fails on a tutorial that does not fit. (The screen still shows a longer
     one four lines a page, up to 48 lines, `TUT_MAX_LINES`.) Control codes such as `[LF]` count as spaces.
2. Run `python3 Tools/tutorial-titles/titles.py fe8.gba` (a second argument writes a preview PNG of every title). It
   draws each title into `Tutorials/Source/TutorialTitles.c` and `.h` (generated: don't edit them).
3. Add `{ MSG_TUT_EXAMPLE_TITLE, MSG_TUT_EXAMPLE, Img_TutTitle_EXAMPLE },` to `gTutorials` (`Kernel/Wizardry/Tutorials/Source/TutorialsData.c`),
   where it should show in the list, before the `{ 0 }` that ends it.

---

## 🛠️ How it works

- **The Extras row.** Bit 0 of the Extras list's byte (`EXTRA_MENU_OPTION_TUTORIALS`), vanilla's Link Arena row, which
  this hack never sets (`RemoveLinkArena.event`). `InitSaveMenuChoice` (`ExtrasMenu/Source/SupportMenu.c`) always adds
  it; the Extras list (`sub_80A9B90`), `SaveMenuStartExtraMiscScreen` and `SaveMenuPostExtraMiscScreen` have a case for it.
- **Its place, last.** Vanilla lists a menu's rows in bit order, so bit 0 would be first. `SaveMenuGetBitfile` (a row's
  bit) and `SaveMenuGetBitfileByMask` (a bit's row) are replaced (`Tutorials/Source/TutorialsExtras.c`): when bit 7 is
  set they walk bits 1-7, then 0. Bit 7 is the Skill Codex's, always in the Extras list; the main menu never sets bit 7
  (`MAIN_MENU_7`), so its order (Resume Chapter, bit 0, first) is vanilla's. Every routine that draws or picks the
  Extras rows goes through these two.
- **Eight rows.** With New Game+ and the Bonus Claim both showing, the Extras list has 8 rows; vanilla's layout
  (`sub_80AAF6C`) holds 7 (from y 2, 21 pixels apart; fewer are centred, 25 apart), and the last two were drawn off the
  screen. Its replacement (`TutorialsExtras.c`) is vanilla's with a case for 8: from y 2, 19 pixels apart. The hand
  follows the same spacing.
- **The row's label.** "Tutorials" is made of vanilla's own letters by `Tools/extras-labels/labels.py`: the "T" of
  Transfer Data, the "u" of Resume Chapter, the "t" and "a" of Battle History, the "o" of Copy Data, the "r" of Restart,
  the "i" of Link Arena, the "l" of Gallery and the "s" of Easy. It is in the Extras label sheet
  (`NewGamePlus/Gfx/Img_NgpMenuObjs.png`) in three pieces: label row 0, tiles 26-29 and 30-31, and row 1, tiles 12-13.
  Row 0's tiles 26-27 were the empty fourth piece of New Game+'s sprite, which now has three; row 1's 12-13 lie between
  the Bestiary's last piece and Support Conversation's sprite (row 0's 12-13 are inside the Bonus Claim's "Transfer
  Data" sprite). The "u" is Resume's, columns 34-38 of the save sheet (both its stems). The header's sprite is drawn
  8 pixels down from Select Mode's header, as vanilla's "Select Mode" is, which centres it. Its sprite is `Sprite_TutorialsExtrasLabel` in `gExtrasLabelSprites` (`NgpSaveMenu.c`), in place of
  Link Arena's.
- **The screen** (`Tutorials/Source/TutorialsScreen.c`) is vanilla's Select Mode screen rebuilt
  (`InitDifficultySelectScreen` and `DrawDifficultySprites_Loop`): the same palettes, `Img_DifficultyMenuObjs` at OBJ
  tile 0x40, the box's tilemap (`gUnknown_08A29558`) on BG1 at (17, 6), the banners (`gSprite_08A20AC2`) at x 8, y 48,
  80 and 112 on OBJ palettes 5-10 (lit and unlit for each), their glow (`sub_80AC4F8`), the header (`gSprite_DifficultyMenuSelectModeBg`)
  and the hand (`gSprite_08A20A98`), which slides to its banner in 4 frames and bobs as vanilla's. The save menu's
  background stays behind it. The box's lines are BG0 text at (18, 7), two rows apart.
- **The titles** are images, not text: `Tools/tutorial-titles/titles.py` draws each title message as
  `labels.py` draws the Extras labels, from vanilla's own letters (the main menu's, the save menu's and Select Mode's
  words), and centres it in 128x16 (`Img_TutTitle_<KEY>`, in `TutorialTitles.c`). Vanilla's words have no U, W, O, V,
  F, I, P, K, b, w, q or g: those are drawn in the script (`DRAWN`) in the same strokes. The screen copies the three
  shown titles into OBJ tiles 0x2C0-0x33F (two 16-tile rows each, under the Extras label sheet) whenever the list
  moves, and draws each as four 32x16 pieces 8 pixels down its banner, on the label palette (OBJ palette 11). The scroll arrows are `DisplayUiVArrow`, their
  tiles (`UnpackUiVArrowGfx`) at OBJ tile 0x180 on palette 12. The header's "Tutorials" is the
  Extras label sheet loaded at OBJ tile 0x200, as the Extras list has it, on OBJ palette 11 with the label palette
  (`Pal_SaveScreenSprits`' third).
- **Wrapping.** The text is decoded into `gGenericBuffer`, and the line starts are worked out word by word with
  `GetStringTextLen` (`Tutorials_Wrap`); a page draws its four lines from them.
- **RAM.** The screen's state lives in the Bestiary screen's (`gBestiaryScreen`): the Extras screens never run together.

---

## 🧪 Testing

`python3 Tools/cloud/smoketest.py --only tutorials` boots, saves and resets, then opens Extras and presses UP once from
the top row: going around to the last row opens Tutorials. It checks that the Extras byte has bit 0, that every
tutorial of `TutorialsData.c` is listed with the first on its first page, that A turns the page, that DOWN chooses the
next tutorial on its first page, that DOWN past the third banner scrolls the list a row, that every tutorial's text
fits its box (one page, five lines at most), that DOWN on the last tutorial goes around to the first and UP back to
the last, and that B goes back to the save menu. The Gallery, Bestiary and Skill Codex tests count their rows from the bottom of Extras, one more now.

---

## 🗂️ Code Locations

| Part | File | What |
|---|---|---|
| Row order, 8 rows | [`TutorialsExtras.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsExtras.c) | `SaveMenuGetBitfile`, `SaveMenuGetBitfileByMask`, `sub_80AAF6C` |
| Screen | [`TutorialsScreen.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsScreen.c) | Set-up, sprites, wrapping, keys |
| List | [`TutorialsData.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsData.c) | `gTutorials` |
| Titles | [`titles.py`](../../Tools/tutorial-titles/titles.py), `TutorialTitles.c` (generated) | The titles' images and the letters vanilla lacks |
| Text | [`Districts.txt`](../../Contents/Texts/Source/texts/Districts.txt) | `MSG_TUT_*`, at the end |
| Header | [`tutorials.h`](../../include/kernel/tutorials.h) | `EXTRA_MENU_OPTION_TUTORIALS`, `struct Tutorial` |
| Extras row | [`SupportMenu.c`](../../Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c), [`NgpSaveMenu.c`](../../Kernel/Wizardry/NewGamePlus/Source/NgpSaveMenu.c), [`Gallery.c`](../../Kernel/Wizardry/Gallery/Source/Gallery.c) | Adding the row, its label sprite, starting the screen and coming back |
| Label | [`labels.py`](../../Tools/extras-labels/labels.py) | The word and its tiles |

---

## 🐛 Limitations

- The page count is one digit: a tutorial of 10 or more pages would show the wrong count (none has more than one).
- The list jumps a row at a time; it does not scroll smoothly.
- The tutorials are English only, like the rest of `Districts.txt`.
