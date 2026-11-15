# Chapter Intro Styles

Alternatives to vanilla's chapter intro (the light burst and title). The player picks one on the Options screen.

## Styles

| Style | Look |
|---|---|
| Vanilla | Vanilla's intro, unchanged |
| Slide | The title slides in from the right while it brightens, holds, then slides out to the left while it darkens. |
| Curtain | The title fades in and out. A revealed map opens from the centre line. |
| Mosaic | The title resolves out of a 16x16 mosaic and dissolves back. A revealed map resolves the same way. |
| Letterbox | The title is wiped in from the left and out to the right. A revealed map fades in between black bars that then pull back. |

Each title holds for 100 frames. A, B or START skips the title.

## Backgrounds

| Background | Behind the title |
|---|---|
| Black | A black screen. |
| Vanilla | Vanilla's scrolling runes (BG1, drawn by vanilla's `0x08020010` with palettes 14-15, scroll proc `0x0859B108`). |
| Last Scene | The previous chapter's closing background, from the recap's record (`gRecap.data.endBg`, else `lastBg`). With no record, e.g. in the prologue or in a save from before the recap, it is vanilla's runes. |

With a CG, the title uses palette 2 (a 224-colour CG leaves palettes 2 and 3 free) and tiles 0x80-0xBF. The CG's
8bpp tiles start at `0x06004000` and cover vanilla's title tiles at 0x280.

## The map

Vanilla shows the map at the end of its intro only when the chapter's `fadeToBlack` (ROM chapter data `+0x87`) is 0
and the map is not a skirmish (`ChapterIntro_LoopFadeToMap`, `0x08020CA4`). Otherwise BG3 and OBJ stay off, and the
chapter's opening event fades the map in. The styles do the same, so their map reveals show only in those chapters.
Of chapters 0-21, only Chapter 12 has `fadeToBlack` 0.

Like vanilla, they start the chapter's map music (`mapBgmIds[9]`, unless it is 0xFFFF) and the flames gradient for
weather 5.

## Options and defaults

Two Options rows, "Chapter Intro" and "Intro Backdrop" (`GAME_OPTION_CHAPTER_INTRO` = 39,
`GAME_OPTION_CHAPTER_INTRO_BG` = 40, after Map Colors). Like UI Style, one value is shown at a time.

Both live in `gPlaySt.config.chapter_intro` (4 bits, the former `custom_option_29`..`_32`) as
`style + 5 * background`.

A new game takes the designer config's `chapter_intro_style` and `chapter_intro_bg`
(`Data/DesignerConfig/designer-config.c`; defaults 0 for vanilla and 1 for vanilla runes). A save made before this
feature has 0 there, which is the vanilla intro. No save layout changes.

The row titles are `MSG_MENU_OPTION_CHAPTER_INTRO_TITLE` and `MSG_MENU_OPTION_CHAPTER_INTRO_BG_TITLE` (end of
`Districts.txt`). The values and help lines are C strings in `ChapterIntroStyles.c`.

## How it hooks in

`ChapterIntroStyles_Installer.event` points the first step of vanilla's `gProcScr_ChapterIntro` (`0x0859B1B0`,
`PROC_CALL(BMapDispSuspend)`, pointer at `0x0859B1B4`) to `ChapterIntroStyles_Gate`. The gate calls
`BMapDispSuspend`. With a style other than Vanilla, it starts `ProcScr_ChapterIntroStyle`, blocking the vanilla
proc's parent, and ends the vanilla proc.

The new proc runs these steps:

- `CIS_Init`: `SetupBackgrounds`, the background, the title, and the intro jingle through
  `ChapterIntro_Init_PlaySound316`. That call writes proc `+0x4C`-`+0x68`, which is why the proc is 0x6C bytes.
- `CIS_TitleLoop`: runs the title, and ends with the screen black.
- `CIS_PrepareMap`: vanilla's `ChapterIntro_8020B20`, `BMapDispResume`, `ChapterIntro_InitMapDisplay`.
- `CIS_MapLoop`: the map reveal, or a 30-frame wait when the map is not revealed.
- `CIS_End`: `ChapterIntro_80210C8`, then the blend vanilla's `InitMapDisplay` sets up.

`BmMain_StartIntroFx` in `MiscFunctions.c` (skip_intro, the base-chapter skip flag) still decides whether any intro
plays.

## Adding a style

1. Add an enum value before `CHAPTER_INTRO_STYLE_COUNT`.
2. Write a title function and a map function, both `bool f(proc, t)`. Each returns true when done. A title function
   must end with `CIS_Darken(16)`.
3. Add their rows to `sTitleFrames` and `sMapFrames`, and a `sStyleChoices` entry.

The 4-bit option holds `style + 5 * background`, so 5 styles x 3 backgrounds fill it (15 of 16 values). Another
style or background needs the encoding changed (and the factor 5 in `ChapterIntro_Get`/`Set`). No spare option bits
remain.

## Testing

`Tools/cloud/smoketest.py` has no test for this yet. Check it by hand in the emulator: the prologue's intro after a
new game, and setting the high nibble of `gPlaySt+0x47` (`0x0202BD37`) to pick the option.
