# Tutorials

<p align="center">
  <img src="../Images/Tutorials.png" alt="The Tutorials screen on Combat Arts, and Combat Arts' map explaining the art" width="720"/>
</p>

*Extras > Tutorials, the last Extras row, lists the README's gameplay features on Select Mode's banners. A starts the
chosen one's map: a test map that explains the feature, then lets you try it.*

---

## 📑 Index
- [Introduction](#introduction)
- [The tutorials](#the-tutorials)
- [The maps](#the-maps)
- [Guided steps](#guided-steps)
- [Adding a tutorial](#adding-a-tutorial)
- [How it works](#how-it-works)
- [Testing](#testing)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

**Extras > Tutorials**, the last Extras row (under the Skill Codex), lets the player try the hack's own features, one
per README entry:

- **The banners** on the left are Select Mode's, three on screen, in the header bar's colours (not Select Mode's green,
  teal and red), with Select Mode's hand beside the chosen one. Each shows a tutorial's title in the letters of the menus' labels (the
  Extras list's, the header's "Tutorials").
- **Scrolling.** Up and down choose a tutorial. Past the third banner the list scrolls under them; a gold arrow above
  or below the banners shows there are more that way. Down on the last tutorial goes around to the first, and up on
  the first to the last (on a press, not while the key is held).
- **The box** on the right is Select Mode's description box. It sums the chosen tutorial up in four lines, with
  "A: Try it" in gold on the fifth. Every summary fits the box; a longer one would show four lines a page, with the
  page count on the last line ("2/3"), right showing the next page and left the one before.
- **A** starts the tutorial's map (below). When the map ends, the game comes back to this screen, on that tutorial.
- **The header** is Select Mode's, with "Tutorials" in the Extras labels' letters, centred in it.
- B goes back to Extras.

---

## 📚 The tutorials

The README's features that are on in this build and that a player sees or uses on a map, in the README's order
(menus, graphics, text and system features are left out):

| Section | Tutorial | Its map (only what the tutorial needs) |
|---|---|---|
| Battle & Combat | Combat Arts | Eirika with a Slim Sword (Hexblade), a soldier |
| | Critical Cut-in | Joshua with a Killing Edge and the Crit skill, a fighter |
| | Weapon Slots | Colm, a thief: his weapon ranks on the stat screen |
| | Expanded HP | Gilliam with 120 HP and an Iron Lance, a soldier |
| | Gaiden Magic | Lute (level 8: Fire, Thunder, Elfire), hurt, and Natasha (Heal, Mend), a soldier. Listed only with Three Houses magic (spells used a number of times a chapter) |
| | MP System | Lute, a soldier. Listed only with the MP system and no Three Houses magic: the two never go together |
| | Kill Rewards | Artur (Mag 20, Skl and Spd 15, 30 HP: he defeats her in one fight) against Novala (Artur wins the Divine tome) |
| | Magic Seal | Lute (3, 6) and a fighter with Magic Seal in the far corner (13, 9), so the seal's edge shows between them |
| | Tonics | Eirika with two tonics |
| Staves | Staff EXP | Natasha heals a hurt Eirika |
| Maps, Fog & AI | AI Rescue | Eirika; a paladin next to an unarmed soldier at 2 HP (the paladin loaded first, so it acts first). The enemy phase plays first |
| | Enemy Promotion | Ross (1 HP, no speed or luck) and a level 10 fighter (Skl 20) that can reach him: its first blow defeats him. Eirika waits on Ross's row (2, 5), out of the fighter's reach, so the camera stays put: with Ross alone, his defeat was a game over, which ended the map during the promotion's effect |
| | Enemy Fog Vision, Fog Stages | The map in fog (vision 2): Colm the thief, two soldiers |
| | Fog Vision | The same, with Eirika beside Colm to compare their sight |
| | Fancy Teleport | Eirika; an assassin warps in after the explanation |
| | Free Movement | Eirika, free movement on (`EnableFreeMovementASMC`) |
| | Move Ghost | Eirika: Translucent Unit Sprite on any move |
| | Traps | Eirika, a heal tile and a pair of teleport tiles |
| | Threat Arrows | Eirika, two soldiers, the Options row turned on |
| Growth & Progression | Prestige | Franz, a level 10 cavalier |
| | Skill Capacity | Eirika: her skills page |
| | Trainee Promotion | Ross at level 9 with 99 EXP, next to a soldier |
| Towns & Minigames | Districts | Port Kiris, as it is |
| | Fishing, River Crossing, Tower Stack, Guild Board | Port Kiris with only that spot (and the gate), no townspeople |
| | Hex Cards | Port Kiris with only its Hex players, no spots |
| | Snek | Port Kiris with only the Pied Piper (the old man), no spots |

A map with no enemy has nothing to win: its "defeat every enemy" check runs as soon as the opening ends and goes on
(`TutorialMap_CheckWin`). A map with no Eirika has its own unit as the leader (`GetPlayerLeaderUnitId`, below).

Left out:

- **Off in this build:** Anima Triangle, Biorhythm, Laguz Bars, Mokha Gambits, Real-Time Battle, Shield Item,
  Weapon Rank Skills, the Escape and Timer goals, Menu Skill AI, Start Map Effects, Weapon Traits, Minimum Damage,
  Allocate Stat Points, Arena Roster, Class Growths, Infuse and Skill Synthesis (locked), the Skill Tree, Timing Stat
  Level-Up, the World Map Skill Shop, Pair-Up (nothing in the story unlocks it) and 2x2 Units (no unit is listed).
- **The Arbalest, Aum, Phoenix and Skill staves:** with Three Houses magic no unit keeps a staff item (they are
  removed at every phase start, `PrePhase_RemoveThreeHousesMagicItems`), and no unit learns them as spells, so a
  player can't use them.
- **Bonus EXP:** a preparations menu; the tutorials' map has no preparations.

---

## 🗺️ The maps

- **Starting.** A on a tutorial fades the screen out and starts its map as a game of its own: a new game's state in
  RAM only (as `WriteNewGameSave` sets one up, without writing it), vanilla's tutorial flag (`PLAY_FLAG_TUTORIAL`) set,
  and the tutorial's chapter. Free movement is turned off (`DisableFreeMovementASMC`: it would stay on after the Free
  Movement tutorial); only Free Movement and the districts turn it on. The map's title card names the tutorial:
  on a tutorial's map, the message decoder (`GetStringFromIndexInBuffer`, AntiHuffman.c) gives the tutorial's title
  for the title of the chapter row the card shows (`TutorialMap_TitleMessage`; Port Kiris's card shows Chapter 9's
  row).
- **The tutorials' map** is chapter row 0x43 (`TUTORIAL_MAP`, `CustomCampaign/Chapters/TutorialMap`), a vanilla filler
  row with no title that nothing used, with vanilla Chapter 4's map. Its opening puts the tutorial's units on it and
  runs its set-up (`TutorialMap_Setup`), fades in, explains the feature in a box in the middle of the screen (as the
  in-map tutorials do), then runs the tutorial's after-explanation step, if any (the teleport's effect). Then the
  player tries it. The towns' tutorials start Port Kiris instead: its opening keeps only the tutorial's spot or people
  (`TutorialMap_TownSetup`, before its townspeople start walking), and shows the explanation
  (`EventScr_TutorialMap_DistrictIntro`).
- **No phase banner.** On a tutorial's map the phase banner ("PLAYER PHASE", which dims the map and lights it again)
  never shows: the explanation goes straight on to the guided step. `PhaseIntro_EndIfNoUnits` (FE8U 0x0801F2AC) is
  replaced: on a tutorial's map it starts the map's music (as the banner's end does) and ends the banner.
- **Leaving: SELECT.** On a tutorial's map or district, SELECT asks "Leave the tutorial?" (Yes/No) while the player
  phase, or free movement, waits for input (`TutorialMap_QuitWatch`). Yes ends the map from a proc of its own
  (`TutorialMap_Leave`, as vanilla's game over does once its screen is over), back to Tutorials.
- **Straight to Tutorials.** The way back goes through the save menu (the game control's label 5), which drew its main
  menu at full brightness and faded it out before Tutorials faded in; the map also showed for a frame as it ended.
  Now every colour is black and no layer shows (`TutorialMap_Blackout`) from the end of the map, and again once the
  save menu has drawn its menu: its fade out (`NewFadeOut` shows every layer, `EnableAllGfx`, and fades the palettes
  it finds) stays black, and the Tutorials screen loads its own palettes and fades in. The save menu's slot sprites
  put their palettes back every frame, so a proc in the vblank tree (`ProcScr_TutorialMapHide`, run right before the
  display settings are copied) keeps every layer hidden until the Tutorials screen exists; without it the main menu's
  frame showed for a frame or more on the black screen.
- **The game lock.** The leaving event (SELECT's Yes, a lesson's end) locks the game as a gameplay event does
  (`gBmSt.lock`), and is deleted with the map before it unlocks. The way back clears the lock: left set, every later
  tutorial's map stayed black (found playing two tutorials in a row; `tutorialguides` checks it).
- **No Eirika.** Vanilla's leader (`GetPlayerLeaderUnitId`), whom the camera and the cursor go to at the player phase's
  start, is Eirika on a story map, whether she is there or not; with her missing the cursor went off the map, and A
  there decoded a bad message until it overran `gPlaySt`. The kernel's copy falls back on the first player unit on a
  tutorial's map, as vanilla does in skirmishes. The last player unit falling is a loss as the lord's is
  (`TutorialMap_OnUnitKill` sets `EVFLAG_GAMEOVER`).
- **Ending.** Every map tutorial ends with its lesson (Guided steps, below), straight back to Extras > Tutorials, on the
  tutorial just played, with no box. Losing ("Your units have fallen", no Game Over), the map menu's **Retreat**
  (enabled on a tutorial's map, "Retreat?" then Yes) and a district's own ways out come back there too. The defeat of
  every enemy no longer ends a map: a tutorial with guided steps ends with its lesson (`TutorialMap_CheckWin`). The map's own endings leave as the skirmish retreat does (MNCH -1,
  then `StartRetreatProcessing`): a chapter change (MNC2) or a game over would start the map again in tutorial mode.
- **The save memory is never written.** Vanilla's tutorial flag already stops the suspend saves; the game save between
  chapters is never reached (the way back leaves first). The writes that happen outside a save are skipped on a
  tutorial's map: the Gallery's unlocks (`Gallery_UnlockBackground`), the
  game over's boot type (`EventCallGameOverExt`, in case one happens) and the Sound Room's unlocks
  (`UnlockSoundRoomSong`). The new game's
  hooks run, except `NewGameSaveGenericHook`, whose RAM parts are done by hand (it clears the suspend save and the
  world map in the save memory). Achievements, which write as they unlock, are off in this build
  (`vesly_achievements`); turned on, `CannotUnlockAchievements` would need the same check.

---

## 🧭 Guided steps

Each map tutorial walks the player through its lesson, as vanilla's Easy-mode tutorial does in the prologue: the
cursor goes to the lesson's unit, only the keys of the step work, a box says what to do. Each has an end, and the end
goes straight back to Tutorials. They start at the player phase's start, after the explanation.

| Kind | Steps | Ends | Tutorials |
|---|---|---|---|
| A unit's action | A on the unit (the cursor is on it; only A works). The destination flashes: move there and press A (any other tile: "Not there", and the cursor goes back to it). Its menu has only the lesson's command; B, R and START do nothing until the action is done | When the action is over | Combat Arts (Attack; the box says left or right picks Hexblade in the forecast), Critical Cut-in, Expanded HP, Trainee Promotion (Attack), Gaiden Magic, Kill Rewards (B.Mag), Staff EXP (W.Mag), Tonics (Items), Magic Seal (Wait inside the seal), Traps (Wait on the rune tile), Prestige |
| The same, then the enemy phase | As above, with Wait | After the enemy phase (the fighter defeats Ross and promotes) | Enemy Promotion |
| The stat screen | Only R works on the unit; the stat screen opens on the lesson's page, then every key works | When the stat screen closes | Weapon Slots and Skill Capacity (Weapons & Skills), MP System (Magic List: its MP costs) |
| Selecting a unit | Only A works on the unit; selected, the map is the player's (the lesson is its range) | When the unit's action is over | Move Ghost, Threat Arrows |
| The enemy phase first | No step: the player phase ends at once (`TutGuide_EndTurn`, as the map menu's End does) | After the enemy phase (the paladin rescues the soldier) | AI Rescue |
| A turn to watch | No step | After the enemy phase, at the next player phase's start | the three fog tutorials, Fancy Teleport |
| A walk | No step (free movement's A acts on what is next to the unit, and the map has nothing) | Once the unit has walked 4 tiles from its start (`TutGuide_WalkStep`) | Free Movement |
| None | The districts and their games are walks; they end by their own ways out (the gate, Retreat) | | |

- **SELECT** still leaves at any step: its prompt frees the keys while it asks, and No gives the step its keys back.
- **The end** (`EventScr_TutG_Leave`: fade to black, then `TutorialMap_Leave`) starts only at a quiet point: a player
  phase's start or idle loop, or a phase change. After an action, `EventScr_TutG_Done` queues it for the phase change
  (the turn ends by itself when no unit is left to act) and `TutGuide_LeaveWhenIdle` starts it as soon as the player
  phase waits again. Started at once, from the action's post-action event, it ended the map while the action's
  post-action hooks (`ProcScr_HookCommon`) still ran: after Kill Rewards' item they drew the map, and its icons, over
  the Tutorials screen. A turn's end is two steps: the phase change queues the end for the next player phase's start
  (`EventScr_TutG_Turn`, `EventScr_TutG_TurnEnd`). Enemy Promotion's Ross falling is the lesson, not a loss
  (`TutorialMap_OnUnitKill`).
- **The engine is vanilla's** (`prologue-tutorials.h` in the decomp): a step is an event queued for a point of the
  player phase (`EvtEnqueueConditionalTutCall`: its start, a unit selected, a destination chosen, the unit moved, the
  action done), which vanilla's `EnqueueTutEvent` finds by address in the chapter's tutorial list (TUTORIAL_MAP's
  `tutorialEvents`, `gTutorialGuideScripts`). `TutEventExecType0` and `1` check the unit and the tile and ask again;
  `IGNORE_KEYS` keeps the player on the step. The first step is queued by the opening (`TutorialMap_Setup`).
- **The unit menu** shows only the step's command: vanilla's `DISABLEOPTIONS` knows only its own 15 commands, so
  `OverriddenMenuAvailability` (FE8U 0x0804F7AC) is replaced: on a tutorial's map with a command set
  (`TutGuide_LockMenu`, `gPlaySt.config2.unused_22[2]`), every other row of the unit menu
  (`gUnitActionMenuItemsRework`, told by its effect) is hidden, as long as that command shows. A menu with no row at
  all crashes the game: Expanded HP's Gilliam had a Silver Lance over his rank, so no Attack, and the lock hid
  everything. He has an Iron Lance now.
- **The stat screen step** (`TutGuide_StatStep`): vanilla opens the stat screen on the page kept in gPlaySt's two
  low state bits, too few for this hack's pages. The screen draws its page two frames after it starts
  (`gProcScr_StatScreen`'s `PROC_SLEEP(2)`), so a proc that sees it start sets `gStatScreen.page` (the shown pages'
  index of the lesson's page, `TranslateStatPageId`, kept in `unused_22[5]`) and gives every key back. Once the stat
  screen has closed and the player phase waits again, the lesson is over.
- **Which tutorials are listed** (`Tutorials_Count`, `Tutorials_Index`, `Tutorials_Shown` in `TutorialsData.c`): the
  MP system and Three Houses magic are two ways to pay for a spell and never go together, so MP System's tutorial is
  listed only with the MP system and no Three Houses magic, and Gaiden Magic's (uses a chapter) only with Three Houses
  magic (`IsGaidenMagicUsesSystem`). The screen counts and draws the listed ones; a tutorial's map keeps its
  `gTutorials` index.
- **The tiles a box borrows.** A tutorial box's window (vanilla's box dialogue) takes OBJ tiles 0x2E0-0x2FF: the unit
  window's HP digits (0x2E0) and the move arrow (0x2F0). After every guide box `TutGuide_RestoreBoxTiles` puts both
  back (`PlayerPhaseInterfaceNumbersHook`'s digits, `PathArrowDisp_Init`'s arrow). Without it the arrow showed as a solid
  blue band after "Not there", and the HP digits as garbage.
- **Ending the turn** (`TutGuide_EndTurn`, AI Rescue and Enemy Promotion) ends the side windows' maker before the
  windows (VeslyAchievements repoints it, `0x0808D14C`; RealtimeScheduler.c and TrapData.c do the same): it spawned
  the unit and goal windows again, and they stayed through the enemy phase, whose map battle box and promotion popup
  reuse their sprite tiles.
- **One fade out.** The chapter's goal is `GOAL_TYPE_SPECIAL`, not "defeat all": with no enemy left, vanilla's phase
  change (`RunPhaseSwitchEvents`) ended the chapter (`CallEndEvent`), a fade out and in before the lesson's own way
  back. The "defeat every enemy" event has its own entry flag (`TUTMAP_FLAG_NO_WIN`, set by a guided tutorial's
  opening) so it never starts in a guided tutorial, and `NOFADE` (as does the district intro).
- **A box over the battle forecast** takes the tiles of its numbers (the box's font), which show as squares until the
  forecast is drawn again: Combat Arts says what to do in the forecast before the menu, not in it.
- **Cleared** (`TutGuide_Reset`: no step queued, no key ignored, no command lock) when a tutorial starts, when its
  map is left, and when a lesson ends.

---

## ➕ Adding a tutorial

1. Add three messages at the end of `Contents/Texts/Source/texts/Districts.txt` (a message anywhere else moves every
   later message's id, CLAUDE.md):

   ```
   ## MSG_TUT_EXAMPLE_TITLE
   Example[X]
   ## MSG_TUT_EXAMPLE
   The box's summary, as one paragraph. The screen wraps it to the box.[X]
   ## MSG_TUT_EXAMPLE_MAP
   The explanation on the map, in the middle of the screen.[A][N]About 30 characters a line, [A] every two.[A][X]
   ```

   - The title must fit a banner: 128 pixels. It may use any letter `Tools/tutorial-titles/titles.py` has (the
     script stops on one it lacks: draw it in `DRAWN` there).
   - The summary is wrapped at 88 pixels (`TEXT_PX`). Keep it to four lines, about 70 characters (the fifth is "A: Try
     it"). The smoke test fails on a summary that does not fit. Control codes such as `[LF]` count as spaces.
2. Run `python3 Tools/tutorial-titles/titles.py fe8.gba` (a second argument writes a preview PNG of every title). It
   draws each title into `Tutorials/Source/TutorialTitles.c` and `.h` (generated: don't edit them).
3. In `Kernel/Wizardry/Tutorials/Source/TutorialsData.c`, add its units (`BLUE`, `RED`, `RED_STILL`; the players round
   (4, 5), the enemies round (9-11, 4-6) of the 15x15 map), a set-up function if it needs one (an option, a skill, fog,
   HP...), and its row in `gTutorials`, where it should show in the list, before the `{ 0 }` that ends it:

   ```c
   MAP_TUTORIAL(MSG_TUT_EXAMPLE_TITLE, MSG_TUT_EXAMPLE, Img_TutTitle_EXAMPLE, MSG_TUT_EXAMPLE_MAP,
                TutUnits_Example, Setup_Example, NULL, EventScr_TutG_Example),
   ```

   `TOWN_TUTORIAL(...)` (the four messages only) starts Port Kiris instead. Loading a unit leaves out its staves and
   tomes, and every phase start takes them away again (Three Houses magic): a mage's spells come from its level.
4. Its guided steps (below), unless the lesson is only to watch (the last argument is then `NULL`): in
   `Kernel/Wizardry/Tutorials/Source/TutorialGuides.c`, one line, and its scripts in `gTutorialGuideScripts`:

   ```c
   TUTG_ACTION(Example, CHARACTER_EIRIKA, 4, 5, 8, 5, MSG_TUTG_MOVE, TUTG_CMD_ATTACK, MSG_TUTG_CMD_ATTACK,
               EventScr_TutG_Done, POSTACTION)
   ...
   TUTG_ACTION_LIST(Example)
   ```

   and `extern const EventScr EventScr_TutG_Example[];` in `TutorialsData.c`. A lesson with no step of the player's
   takes one of the end scripts as its guide instead: `EventScr_TutG_Turn` (it ends after the enemy phase),
   `EventScr_TutG_Walk` (after a walk of 4 tiles) or `EventScr_TutG_EnemyFirst` (the enemy phase at once, then the
   end). The unit's tile and its destination
   must be on the map (the unit's own tile: it stays), the destination in its move and the command possible there:
   with the command not in the menu, the menu is shown whole (`TutGuide_CommandShown`), and the lesson goes on
   unguided. `python3 Tools/cloud/smoketest.py --only tutorialguides` checks it.

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
- **The banners' colours** are the home menu's (the save menu's main buttons): the chosen tutorial's in the highlighted
  button's palette (`Pal_SaveScreenSprits`' first, OBJ palette 13), its outline pulsing with `Pal_08A28088` as
  `SaveMenuUpdateCursorPalette` does, the others in the plain buttons' (its fifth, OBJ palette 14). Select Mode's banner
  uses the same colour slots.
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
- **Starting a map** (`Tutorials/Source/TutorialMap.c`, `TutorialMap_Launch`). After the fade, the save menu is sent to
  its last step (`PL_SAVEMENU_EXIT`, `PostSaveMenuHandler`) with `main_sel_bitfile` 0x40, for which it sets no action,
  so the action set here (1) stands; `save_menu_type` 2 makes the game control go straight to the map
  (`GameCtrl_CheckNewGameAndBranch`). The tutorial's index is kept in `gPlaySt.config2.unused_22[1]` (0 in every save,
  written only here).
- **Coming back.** Every end of a map reaches the game control's label 9 (`GameControl_PostChapterSwitch`). Its
  proc-script pointer (`$591900`) is `TutorialMap_PostChapterSwitch`: in tutorial mode it clears the flag, marks
  `unused_22[0]`, starts the title's music and goes to label 5, the save menu. The save menu's first
  `SaveMenu_LoadExtraMenuGraphics` (pointer `$A200EC`, `TutorialMap_LoadExtraMenuGraphics`) sees the mark and jumps
  to Extras > Tutorials, as choosing it in Extras does.

---

## 🧪 Testing

`python3 Tools/cloud/smoketest.py --only tutorialmap` boots, saves and resets, opens Tutorials, and presses A on Combat
Arts: it checks the map is chapter 0x43 in tutorial mode with its units, that SELECT's "Leave the tutorial?" (Yes) comes back to
Tutorials on that tutorial, out of tutorial mode, and that the save memory is byte for byte as it was.

`python3 Tools/cloud/smoketest.py --only tutorials` boots, saves and resets, then opens Extras and presses UP once from
the top row: going around to the last row opens Tutorials. It checks that the Extras byte has bit 0, that every
tutorial of `TutorialsData.c` is listed with the first on its first page, that RIGHT turns the page, that DOWN
chooses the next tutorial on its first page, that DOWN past the third banner scrolls the list a row, that every
tutorial's summary fits its box (one page, four lines at most), that DOWN on the last tutorial goes around to the first
and UP back to the last, and that B goes back to the save menu. The Gallery, Bestiary and Skill Codex tests count their rows from the bottom of Extras, one more now.

`python3 Tools/cloud/smoketest.py --only tutorialguides` starts every guided tutorial from the Tutorials screen (the
guides read from `TutorialGuides.c`, their tutorials from `TutorialsData.c`). It checks that each starts with the
cursor on its unit and only A (R for a stat screen step) working; that the unit's action opens a unit menu with only
the lesson's command, R the stat screen on the lesson's page with every key back, and a select step's A every key
back; that SELECT's prompt frees the keys and No gives the step its keys back. Then three ends: Weapon Slots' stat
screen closed goes back to Tutorials, black all the way until the Tutorials screen fades in (no frame of the map or
of the save menu's main menu); AI Rescue, with no key pressed, plays its enemy phase and goes back; Combat Arts' attack
goes back. It drives the steps with `gbarun`'s `skipto` (A until the player phase waits: through the explanation and
the guide's first box), and lists the tutorials as the screen does (`tutorials_shown`: MP System's or Gaiden
Magic's, by `designer-config.c`).

---

## 🗂️ Code Locations

| Part | File | What |
|---|---|---|
| Row order, 8 rows | [`TutorialsExtras.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsExtras.c) | `SaveMenuGetBitfile`, `SaveMenuGetBitfileByMask`, `sub_80AAF6C` |
| Screen | [`TutorialsScreen.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsScreen.c) | Set-up, sprites, wrapping, keys |
| List | [`TutorialsData.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialsData.c) | `gTutorials` |
| Titles | [`titles.py`](../../Tools/tutorial-titles/titles.py), `TutorialTitles.c` (generated) | The titles' images and the letters vanilla lacks |
| Text | [`Districts.txt`](../../Contents/Texts/Source/texts/Districts.txt) | `MSG_TUT_*` and the guided steps' `MSG_TUTG_*`, at the end |
| Header | [`tutorials.h`](../../include/kernel/tutorials.h) | `EXTRA_MENU_OPTION_TUTORIALS`, `struct Tutorial` |
| Extras row | [`SupportMenu.c`](../../Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c), [`NgpSaveMenu.c`](../../Kernel/Wizardry/NewGamePlus/Source/NgpSaveMenu.c), [`Gallery.c`](../../Kernel/Wizardry/Gallery/Source/Gallery.c) | Adding the row, its label sprite, starting the screen and coming back |
| Label | [`labels.py`](../../Tools/extras-labels/labels.py) | The word and its tiles |
| Maps | [`TutorialMap.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialMap.c) | Starting a map, coming back, the opening's ASMCs, Retreat, the Sound Room's unlocks |
| The map's chapter | [`TutorialMap/events`](../../CustomCampaign/Chapters/TutorialMap/events/events.c) | Row 0x43, its opening and ending, its tutorial list (`gTutorialGuideScripts`); Port Kiris calls `EventScr_TutorialMap_DistrictIntro` |
| Guided steps | [`TutorialGuides.c`](../../Kernel/Wizardry/Tutorials/Source/TutorialGuides.c) | Each tutorial's steps, the unit menu lock (`OverriddenMenuAvailability`), the stat screen step |
| Save memory | [`Gallery.c`](../../Kernel/Wizardry/Gallery/Source/Gallery.c), [`C_Code.c`](../../Kernel/Wizardry/VeslyDebugger/C_Code.c), `TutorialMap.c` (`UnlockSoundRoomSong`) | Their writes skipped on a tutorial's map |

---

## 🐛 Limitations

- The page count is one digit: a tutorial of 10 or more pages would show the wrong count (none has more than one).
- The list jumps a row at a time; it does not scroll smoothly.
- The tutorials are English only, like the rest of `Districts.txt`.
- The guided steps cover one action per lesson: Gaiden Magic guides Lute's B.Mag, not Natasha's W.Mag after it.
- The districts' tutorials (Districts, Fishing, Hex Cards, River Crossing, Snek, Tower Stack, Guild Board) have no
  guided steps and no end of their own: the gate, the map menu's Retreat or SELECT leaves.
- SELECT (or the map menu's Retreat) is the way out of a tutorial's map before it is won; Suspend stays greyed out
  (tutorial mode).
- The units, set-ups and texts were checked on the map (units, items, menus, fog, traps, the teleport, Port Kiris), but
  each feature's own behaviour (an enemy promoting, the AI rescuing, a trainee promoting...) is the feature's: the maps
  only set the scene for it.
