# Previously on Your Quest

When a game save or a suspended chapter is loaded, a "Previously on your quest..." recap shows the key scenes of the
last story chapter played, in greyscale, before the game goes on (like FireRed's). A steps to the next scene, START
or B skips the rest.

## What it shows

Up to four scenes, in this order:

| Scene | Picture | Summary |
|---|---|---|
| The opening | the chapter's first background (a CG shown during the opening wins), with the opening's first two speakers | the chapter's opening line, or "A new chapter began." |
| A character who died | their portrait, eyes closed, over the background shown last before | "<Name> fell in battle." |
| A character who joined | their portrait over the background shown last before | "<Name> joined the army." |
| The ending | the last background shown once the chapter was won (once a CG is shown, only another CG replaces it), with the ending's first two speakers | the chapter's ending line, or "The chapter came to an end." |

At most two characters (died or joined, in the order it happened). The header reads "Previously on your quest...
n/N". Two speakers face each other, left and right; one stands in the middle; a CG shows no speakers. Portraits sit on the summary box: its
frame covers their lower edge, as in a conversation (sprite layer 2, behind BG1 and BG0, in front of the background).
A chapter with no background in its opening or ending shows fewer scenes; a save with no recap shows none.

## Recording

- A chapter's recap starts with its opening: `CallBeginningEvents` (FE8U `0x080153D4`) is replaced in
  `Recap.c` (`Source/LynJump.event`) to start it first. Not `ChapterInitHooks`, which run after the opening, and not
  `gPlaySt.chapterIndex` changing: an opening's cutscene map loads change it (the prologue loads Chapter 16's map and
  Outside Renais). Saving never starts one: on the world map the next chapter is already set.
- Backgrounds: event command 0x21 (`TEXT_BG`, `SET_BACKGROUND`, `TEXT_CG`), through `Gallery_Event21`
  (`Kernel/Wizardry/Gallery/Source/Gallery.c`), which calls `Recap_NoteBackground`. The ending is the last background
  shown while `EVFLAG_WIN` is set.
- CGs win: a CG (a 224- or 256-colour background) shown during the opening replaces an ordinary opening background,
  and once the ending has a CG, only another CG replaces it.
- Speakers: the first two different portraits loaded by an event's conversation (`TextEngine_LoadFace`,
  `Kernel/Wizardry/TextEngineRework`; not a battle's quote), during the opening (until the first phase,
  `Recap_PrePhase` in `gPrePhaseFuncs`) and once `EVFLAG_WIN` is set. A scene skipped with START loads no portraits.
- Deaths: `gKillUnitHooks` (`Kernel/Wizardry/UnitHooks/data.event`), blue characters 1-0x3F.
- Joins: the army (blue characters 1-0x3F, alive) at each background, against the army when the recap started. Not in
  the prologue, where the lords arrive.
- A district visit is not a story chapter: it neither starts nor adds to the recap.

## Showing

`Recap_PostSaveMenu` replaces the save menu's last step (`ProcScr_SaveMenu`, `PROC_CALL(PostSaveMenuHandler)` at
`0x08A20390`, `Recap_Installer.event`). It runs `PostSaveMenuHandler`, which loads the save; the save tail's loader
sets `gRecap.pending`; the recap then plays as a blocking child while the screen is still black, and the game starts
after it.

- Backgrounds are drawn with vanilla's `EventShowTextBgDirect`, so the 224-colour CGs of `Backgrounds_224_256_Colors`
  work. A 224-colour CG leaves BG palettes 2 and 3, so the text and the frames move there (`Recap_MoveUiPalettes`).
- Every palette is turned grey after a scene is drawn (`Recap_Greyscale`).

## Data and save

`struct RecapData` (0x18 bytes, magic `RCP2`, `include/kernel/recap.h`) is kept in RAM (`gRecap`, `include/link/config-memmap.s`,
with the army snapshot and the pending flag) and in the tail of every game save and of the suspend save
(`Kernel/Wizardry/SaveData/Source/SaveTail.c`, after the Skill Codex's share). The tail is not a save chunk, so the save
layout and its signature do not change; an older save has no recap until it is saved again.

## Story summaries

`gRecapChapterTexts` (`Kernel/Wizardry/Recap/Source/Recap.c`) maps a chapter id to its opening and ending messages,
`MSG_RECAP_<CHAPTER>_OPEN` and `_END`, at the end of `Contents/Texts/Source/texts/Districts.txt`. A chapter without a
row (or with 0) uses the generic lines. To add one: append the two messages at the end of `Districts.txt` (anywhere
else shifts every later message id), then add a row before the `0xFF` end.

## Code

| What | Where |
|---|---|
| Recording, save tail, screen, summaries | `Kernel/Wizardry/Recap/Source/Recap.c` |
| Structs, prototypes | `include/kernel/recap.h` |
| Save-menu hook | `Kernel/Wizardry/Recap/Recap_Installer.event` |
| Chapter start | `CallBeginningEvents` in `Recap.c`, `Kernel/Wizardry/Recap/Source/LynJump.event` |
| Deaths, the opening's end | `Kernel/Wizardry/UnitHooks/data.event`, `Kernel/Wizardry/PrePhaseHook/data.event` |
| Speakers | `TextEngine_LoadFace`, `Kernel/Wizardry/TextEngineRework/Source/TextEngineRework.c` |
| Backgrounds | `Gallery_Event21`, `Kernel/Wizardry/Gallery/Source/Gallery.c` |
| Smoke test | `test_recap`, `Tools/cloud/smoketest.py` (`--only recap`) |
