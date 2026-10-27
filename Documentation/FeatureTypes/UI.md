# UI screens and windows

> Code: `Kernel/Wizardry/StatScreen/`, `Kernel/Wizardry/PopupRework/`, `Kernel/Wizardry/PopupIconShine/`, `Kernel/Wizardry/LanguageSelect/`, `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c`  ·  Entry points: `gStatScreenDrawPages` (`Kernel/Wizardry/StatScreen/data.event:10`), `StartStatScreenHelp` / `StartMovingHelpBox` (`Kernel/Wizardry/StatScreen/DrawPages/HelpBox.c:65`, `Tools/FE-CLib-Mokha/include/statscreen.h:190`), `gPopupComponents` / `NewPopup_Simple` (`Kernel/Wizardry/PopupRework/Source/PopupR.c:155`, `Tools/FE-CLib-Mokha/include/popup.h:113`), `ProcScr_LanguageSelect` (`Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:353`)  ·  Existing docs: `Documentation/Features/StatScreenChapterLocks.md`, `Documentation/Features/StatScreenPromotions.md`, `Documentation/Features/PaginatedHelpBoxes.md`, `Documentation/Features/PopupItemIconShine.md`, `Documentation/Features/LanguageSelect.md`, `Documentation/Features/VeslyUIStyles.md`

## What it covers

Screens and windows the player sees that are not selectable menus: stat screen pages (the Promotions page, `Kernel/Wizardry/StatScreen/DrawPages/DrawPage7.c`), R-text help boxes (the Forge menu's help, `ForgeMenu.c`), popups (`PopupR.c`, PopupIconShine), and a proc-driven window with its own sprites, palettes and text (`LanguageSelect.c`, `ForgeMenu.c`). Selectable rows are in [Menus.md](Menus.md). Asset files (LZ77, palettes) are covered in [Graphics.md](Graphics.md).

## How the engine calls it

### Stat screen pages

1. `DisplayPage(pageid)` (`Kernel/Wizardry/StatScreen/DrawPages/HelpBox.c:127-152`) clamps the visible page number, maps it through `TranslateStatPageId` (`HelpBox.c:104`), clears `gUiTmScratchA` and `gUiTmScratchC`, and calls `gStatScreenDrawPages[realPageId]()` (`HelpBox.c:152`). It is hooked at ROM `$878CC` (`Kernel/Wizardry/StatScreen/DrawPages/LynJump.event:8`).
2. `gStatScreenDrawPages` is a `POIN` list in `Kernel/Wizardry/StatScreen/data.event:10-19`, included from `Kernel/Wizardry/wizardry.event:30`. Its index is the page id: the `PAGE_*` enum is `include/kernel/stat-screen.h:8-13` (`PAGE_GAIDEN_MAGIC` = 4 through `PAGE_SKILL_TREE` = 7).
3. The visible page count is `GetStatPageCount` (`Kernel/Wizardry/StatScreen/DrawMorePage/Source/DrawMorePage.c:199`). Each optional page counts only if `IsStatScreenPageAvailable` (`HelpBox.c:50`) is true. That needs its designer flag (`IsStatScreenPageEnabledByConfig`, `HelpBox.c:21`; `stat_page_*` in `include/kernel/kernel-lib.h:130-134`) and, if it has one, its chapter lock (`sStatScreenPageChapterLocks`, `HelpBox.c:14`).
4. A page draws into the stat screen's text array `gStatScreen.text[]` with `PutDrawText` and into the tilemap `gUiTmScratchA`. `DrawPage7Rework` (`DrawPage7.c:29`) is the example.

### Help boxes (R-text)

1. A menu's `onRPress` (or a stat page's help) starts a moving help box with `StartMovingHelpBox(info, parent)` (`Tools/FE-CLib-Mokha/include/statscreen.h:190`). `info` is a `struct HelpBoxInfo` (`statscreen.h:148`): four neighbours (`adjUp`, `adjDown`, `adjLeft`, `adjRight`), a display position (`xDisplay`, `yDisplay`), a text id `mid`, and optional `redirect` and `populate` callbacks.
2. `populate` picks the text at display time by writing `proc->mid` (`ForgeMenu.c:85`, `HbPopulate_ForgeItem`). `redirect` moves the box to another node when the target is empty (`ForgeMenu.c:90`, `HbRedirect_ForgeItem`).
3. The stat screen's R text goes through `StartStatScreenHelp` (`HelpBox.c:65`), which switches on the translated page id to one of the `RTextPage*` tables (`include/kernel/stat-screen.h:26-29`).
4. Box graphics come from `LoadHelpBoxGfx` (`HelpBox.c:155`, declared `Tools/FE-CLib-Mokha/include/helpbox.h:112`). In extended or paged modes it moves the box tiles off the skill-capacity circle (`HelpBox.c:160-174`). Text layout is the `text_box_extension_layout` mode (`include/kernel/help-box.h:14-16`; field `include/kernel/kernel-lib.h:150`). See [PaginatedHelpBoxes.md](../Features/PaginatedHelpBoxes.md).

### Popups

1. A popup is a list of instructions, `struct PopupInstruction { u8 opcode; u32 data; }`, ended by `POPUP_END`. The `POPUP_*` macros are at `Tools/FE-CLib-Mokha/include/popup.h:27-39`.
2. The engine starts one with `NewPopup_Simple(inst, duration, winStyle, parent)` (`popup.h:113`). Helpers wrap common cases: `NewPopup_VerySimple(msg, sound, parent)` (`popup.h:118`), `NewPopup_ItemGot(parent, unit, item)` (`popup.h:121`). `SetPopupItem` and `SetPopupUnit` set the item or unit that the instruction names.
3. `ParsePopupInstAndGetLen` (`Kernel/Wizardry/PopupRework/Source/PopupR.c:181`, ROM `$10F38`) sums each opcode's width through `gPopupComponents[op].get_len`. `GeneratePopupText` (`PopupR.c:198`, ROM `$1106C`) draws each opcode through `.display`. Both are hooked from `Kernel/Wizardry/PopupRework/Source/LynJump.event`.
4. The opcode table is `gPopupComponents[CHAX_POPUP_OP_ALLOC_MAX]` (`PopupR.c:155`; `include/kernel/popup-reowrk.h:30`). Its opcodes are `enum chax_popup_opcode_index` (`popup-reowrk.h:5-17`). `CHAX_POPUP_OP_ALLOC_MAX` is `0x30`, so there are spare slots after `CHAX_POPUP_OP_MAX`.
5. Battle popups run as a proc: `BmPopR_Loop` (`Kernel/Wizardry/PopupRework/Source/BmPopup.c:13`) walks `gpBattlePopupConfigs`, calls each config's `setup()`, and starts `NewPopup_Simple` when it returns true (`BmPopup.c:22`). The proc script is `ProcScr_BmPopR` (`BmPopup.c:28`).

### Proc-driven windows

1. A window is a proc script, `const struct ProcCmd ProcScr_X[]` (`LanguageSelect.c:353`). The macros are `PROC_NAME`, `PROC_CALL` (runs once), `PROC_REPEAT` (runs every frame until `Proc_Break`, which moves to the next command), `PROC_LABEL` / `PROC_GOTO`, `PROC_END`, `PROC_YIELD` (`Tools/FE-CLib-Mokha/include/proc.h:16-42`).
2. Start it with `Proc_Start` or `Proc_StartBlocking(script, parent)` (`proc.h:136-137`). A blocking child stops its parent's input. The Forge menu's skill cursor does this (`ForgeMenu.c:185`, `Proc_StartBlocking(ProcScr_ForgeSkillFocus, menu)`). A hook can also insert the proc into a vanilla list. `LanguageSelect_Installer.event:10` writes `ORG 0x591710 POIN ProcScr_LanguageSelect`.
3. Background: write the tilemap buffers `gBG0TilemapBuffer`, `gBG1TilemapBuffer`, or `BG_GetMapBuffer(bg)` (`Tools/FE-CLib-Mokha/include/hardware.h:392`). Address cells with `TILEMAP_INDEX` and `TILEMAP_LOCATED` (`hardware.h:205-208`). Push them to VRAM with `BG_EnableSyncByMask(BG0_SYNC_BIT | ...)` (`hardware.h:412`, `:250`).
4. Text: `InitText(text, tileWidth)` (`Tools/FE-CLib-Mokha/include/fontgrp.h:172`); `PutDrawText(text, dest, colorId, x, tileWidth, str)` for a string drawn in one call (`fontgrp.h:203`); `Text_SetParams` plus `Text_DrawString` plus `PutText` to redraw a string each frame (`fontgrp.h:183`, `:191`, `:184`); `ClearText` (`fontgrp.h:175`). `ResetText` (`fontgrp.h:167`) clears the shared state before a new screen. Fonts: `InitSystemTextFont` and `SetTextFont` (`ForgeMenu.c:449-450`).
5. Frame: `DrawUiFrame(tilemap, x, y, width, height, tilebase, style)` (`Tools/FE-CLib-Mokha/include/uiutils.h:10`). Load its graphics once with `LoadUiFrameGraphics()` (`uiutils.h:27`). VeslyUI replaces `DrawUiFrame` (`Kernel/Wizardry/VeslyUI/C_Code.c:670`), so a window that calls it follows the active style. `style` is the window colour 0-3 (see [VeslyUIStyles.md](../Features/VeslyUIStyles.md)).
6. Icons: `ResetIconGraphics_()`, `LoadIconPalettes(n)` (`Tools/FE-CLib-Mokha/include/icon.h:9`, `:11`), and `DrawIcon(dest, iconId, oamPalBase)` (`icon.h:17`). The Forge row passes `GetItemIconId(item)` and `0x4000` (`ForgeMenu.c:406`).
7. Sprites and palettes: `Decompress(src, OBJ_CHR_ADDR(n))` loads an LZ77 sheet into OBJ VRAM (`LanguageSelect.c:255`; `hardware.h:217`, `Tools/FE-CLib-Mokha/include/bmlib.h:78`). Sprites go through `PutSpriteExt(layer, oam1, oam0, gfx, oam2)` (`Tools/FE-CLib-Mokha/include/ctc.h:87`). Palette data is written to `gPaletteBuffer` (`hardware.h:177`), then `EnablePaletteSync()` (`hardware.h:410`; `LanguageSelect.c:165`). `ApplyPalette(src, palId)` (`hardware.h:257`) is a macro over `CopyToPaletteBuffer`.

## Where to look

| File | Role |
|---|---|
| `Kernel/Wizardry/StatScreen/data.event` | `gStatScreenDrawPages` list (`:10-19`); index = page id |
| `Kernel/Wizardry/StatScreen/DrawPages/HelpBox.c` | `IsStatScreenPageAvailable` (`:50`), `TranslateStatPageId` (`:104`), `StartStatScreenHelp` (`:65`), `DisplayPage` (`:127`), `LoadHelpBoxGfx` (`:155`) |
| `Kernel/Wizardry/StatScreen/DrawPages/DrawPage7.c` | Promotions page, a small page draw (`:29`) |
| `Kernel/Wizardry/StatScreen/DrawMorePage/Source/DrawMorePage.c` | `GetStatPageCount` (`:199`) |
| `include/kernel/stat-screen.h` | `PAGE_*` ids (`:8-13`), `RTextPage*` (`:26-29`) |
| `include/kernel/kernel-lib.h`, `Data/DesignerConfig/designer-config.c` | `stat_page_*` flags (`kernel-lib.h:130-134`; `designer-config.c:145-149`) |
| `Tools/FE-CLib-Mokha/include/statscreen.h` | `struct HelpBoxInfo` (`:148`), `StartHelpBox` (`:183`), `StartMovingHelpBox` (`:190`) |
| `include/kernel/help-box.h` | `HELP_BOX_MODE_*` (`:14-16`), `HelpBoxNeedsSafeVram` |
| `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c` | Help box table (`:109-131`), menu and frame (`:188`, `:199-236`), text and icons (`:378-406`) |
| `Kernel/Wizardry/PopupRework/Source/PopupR.c` | Opcode components (`:9-155`), `gPopupComponents` (`:155`), `ParsePopupInstAndGetLen` (`:181`), `GeneratePopupText` (`:198`) |
| `Kernel/Wizardry/PopupRework/Source/BmPopup.c` | Battle popup proc (`:13`, `:28`) |
| `include/kernel/popup-reowrk.h` | Opcode enum (`:5-17`), `struct PopupComponent` (`:25`) |
| `Tools/FE-CLib-Mokha/include/popup.h` | `POPUP_*` macros (`:27-39`), `NewPopup_Simple` (`:113`), `NewPopup_VerySimple` (`:118`) |
| `Kernel/Wizardry/PopupIconShine/Source/PopupIconShine.c` | Popup icon proc that replaces the icon loop (a non-menu example) |
| `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c` | Proc screen: init, per-frame loop, `ProcScr_LanguageSelect` (`:242`, `:295`, `:353`), palettes (`:165`) and OBJ sheet (`:255`) |
| `Kernel/Wizardry/LanguageSelect/LanguageSelect_Installer.event` | Hook that inserts the screen (`ORG 0x591710`, `:10`) and the LZ77 include |
| `Kernel/Wizardry/VeslyUI/C_Code.c` | `DrawUiFrame` replacement (`:670`) |
| `Tools/FE-CLib-Mokha/include/fontgrp.h`, `uiutils.h`, `icon.h`, `hardware.h`, `ctc.h`, `proc.h` | Text, frame, icon, BG/palette, sprite and proc APIs |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the kind: stat page, help box, popup, or proc window. Each kind has its own registration (Template below).
3. Put new code in the `WIZARDRY_CUSTOM` block (`Kernel/Wizardry/wizardry.event:129-191`), not in kernel text. Kernel text is nearly full (CLAUDE.md). `LanguageSelect` and `ForgeMenu` sit there.
4. Stat page: add a `PAGE_*` id and a draw function, add the function to `gStatScreenDrawPages` at the matching index, and give the page a designer flag, a chapter lock if wanted, and an R-text table. A new id past `PAGE_SKILL_TREE` also needs the bounds in Gotchas.
5. Help box: write a `struct HelpBoxInfo` table, optional `populate` and `redirect` callbacks, and a menu `onRPress` that calls `LoadHelpBoxGfx(NULL, -1)` and `StartMovingHelpBox`.
6. Popup: build the `POPUP_*` instruction array and call `NewPopup_Simple`. For a new opcode, add the enum value (`popup-reowrk.h`) and a `gPopupComponents` entry (`PopupR.c`).
7. Proc window: write the `ProcCmd` script, start it with `Proc_Start` or `Proc_StartBlocking` from a menu or ASMC, and clear the screen (`ClearBg0Bg1`, `BG_EnableSyncByMask`) when it ends.
8. Gate it with a designer flag if it is optional (see [Menus.md](Menus.md) step 5).
9. Build and test in game. Not covered here.

## Template

Names with `Example` are placeholders. Message ids need entries in `Contents/Texts/Source/texts/`.

### Stat screen page

Adapted from `Kernel/Wizardry/StatScreen/DrawPages/DrawPage7.c` (`DrawPage7Rework`, `:29-36`).

A new file `DrawPageExample.c` in the page folder `Kernel/Wizardry/StatScreen/DrawPages/`:

```c
#include "common-chax.h"
#include "hooks.h"
#include "stat-screen.h"

void DrawPageExample(void)
{
	for (int i = 14; i < 21; i++)
		ClearText(&gStatScreen.text[i]);

	PutDrawText(&gStatScreen.text[14],
		gUiTmScratchA + TILEMAP_INDEX(5, 8),
		TEXT_COLOR_SYSTEM_GOLD, 0, 0,
		"Example");
}
```

Registration, in `Kernel/Wizardry/StatScreen/data.event`, as the entry at the same index as the page id:

```
    POIN DrawPageExample
```

Also add `PAGE_EXAMPLE = 8,` to the enum in `include/kernel/stat-screen.h` (see Gotchas before using a value past 7), and a case in `StartStatScreenHelp` (`HelpBox.c:65`) that points at an R-text table.

### R-text help box

Adapted from the Forge menu help (`ForgeMenu.c:109-131`, `:188-197`).

```c
#include "common-chax.h"
#include "statscreen.h"
#include "helpbox.h"
#include "bmmenu.h"

static void HbPopulate_Example(struct HelpBoxProc *proc)
{
	proc->mid = MSG_Example_Help;
}

const struct HelpBoxInfo sExampleHbInfo = {
	.adjUp = NULL, .adjDown = NULL, .adjLeft = NULL, .adjRight = NULL,
	.xDisplay = 14 * 8, .yDisplay = 2 * 8,
	.mid = MSG_Example_Help,
	.redirect = NULL,
	.populate = HbPopulate_Example,
};

/* Set as the menu's onRPress in its MenuDef */
static u8 ExampleMenuHelp(struct MenuProc *menu)
{
	LoadHelpBoxGfx(NULL, -1);
	StartMovingHelpBox(&sExampleHbInfo, (struct Proc *)menu);
	return 0;
}
```

### Popup (instruction and custom opcode)

Adapted from `PoprGetLen_CombArtIcon` and `PoprDisp_CombArtIcon` (`Kernel/Wizardry/PopupRework/Source/PopupR.c:142-152`) and the `NewPopup_VerySimple` call in `ForgeMenu.c:27-29`.

`include/kernel/popup-reowrk.h`, in `enum chax_popup_opcode_index` before `CHAX_POPUP_OP_MAX`:

```c
	CHAX_POPUP_OP_EXAMPLE_ICON,
```

`Kernel/Wizardry/PopupRework/Source/PopupR.c`:

```c
STATIC_DECLAR int PoprGetLen_ExampleIcon(struct PopupProc *proc, const struct PopupInstruction *inst)
{
	proc->iconX = proc->xGfxSize;
	proc->iconId = 0; /* your icon id source */
	LoadIconPalette(0, proc->iconPalId);
	return 0x10;
}

STATIC_DECLAR void PoprDisp_ExampleIcon(struct Text *text, const struct PopupInstruction *inst)
{
	Text_Skip(text, 0x10);
}
```

Add to the `gPopupComponents` initialiser (`PopupR.c:155`):

```c
	[CHAX_POPUP_OP_EXAMPLE_ICON] = { PoprGetLen_ExampleIcon, PoprDisp_ExampleIcon },
```

Use it:

```c
static const struct PopupInstruction sExamplePopupInst[] = {
	{ CHAX_POPUP_OP_EXAMPLE_ICON, 0 },   /* same shape as CHAX_POPUP_SKILL_ICON in popup-reowrk.h:20 */
	POPUP_SPACE(4),
	POPUP_MSG(MSG_Example_Text),
	POPUP_END,
};

void StartExamplePopup(ProcPtr parent)
{
	NewPopup_Simple(sExamplePopupInst, 0x5A, 0, parent); /* 0x5A = duration in frames */
}
```

### Proc-driven window with text, frame and palette

Adapted from `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c` (the proc script `:353`, init `:242`, loop `:295`) and the text and frame setup in `ForgeMenu.c:199-236`.

A new file `Example.c` in the feature's own folder, `Kernel/Wizardry/<Feature>/Source/`:

```c
#include "common-chax.h"
#include "proc.h"
#include "fontgrp.h"
#include "uiutils.h"
#include "hardware.h"

struct ExampleProc {
	PROC_HEADER;
	struct Text title; /* text storage in the proc: layout not checked (unverified) */
	u8 cursor;
	s16 clock;
};

static void Example_Init(struct ExampleProc *proc)
{
	ClearBg0Bg1();
	ResetText();
	DrawUiFrame(gBG1TilemapBuffer, 1, 1, 14, 6, 0, 0); /* last arg: window colour 0-3 */
	InitText(&proc->title, 12);
	PutDrawText(&proc->title, TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 2),
		TEXT_COLOR_SYSTEM_WHITE, 0, 0, "Example");
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Example_Idle(struct ExampleProc *proc)
{
	proc->clock++;

	if (gKeyStatusPtr->newKeys & B_BUTTON)
		Proc_Break(proc); /* goes to the next command */
}

static void Example_End(struct ExampleProc *proc)
{
	(void)proc;
	ClearBg0Bg1();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

const struct ProcCmd ProcScr_Example[] = {
	PROC_NAME("EXAMPLE"),
	PROC_CALL(Example_Init),
	PROC_REPEAT(Example_Idle),
	PROC_CALL(Example_End),
	PROC_END,
};
```

Start it from the menu callback that opens the window: `Proc_StartBlocking(ProcScr_Example, (ProcPtr)menu);`. A sprite or palette step needs the calls in "How the engine calls it" (sections 7 and 8).

## Worked examples

- `Kernel/Wizardry/StatScreen/DrawPages/DrawPage7.c`: the Promotions page. Clears its text slots, draws class names and skill icons into `gUiTmScratchA`, and sets an event slot for the R text.
- `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c`: a menu with a help box (`:109-131`), text and icons per row (`:378-406`), a frame and text setup (`:199-236`), and a blocking sub-proc (`:163-185`).
- `Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c`: a whole-screen proc with OBJ sprites, an affine transform, a palette rewrite each frame (`:165`), a blend fade, and a hook into the game-control proc list (`LanguageSelect_Installer.event:10`).
- `Kernel/Wizardry/PopupIconShine/Source/PopupIconShine.c`: a popup icon proc that replaces the vanilla icon loop, with a designer toggle (see [PopupItemIconShine.md](../Features/PopupItemIconShine.md)).

## Gotchas

- **Stat page ids past 7 need more than the list.** `DisplayPage` rejects `realPageId > PAGE_SKILL_TREE` (`HelpBox.c:149`). `TranslateStatPageId` handles only ids 4 to 7 (`HelpBox.c:104-125`). `gStatScreenDrawPages` ends at index 7 (`data.event:17-18`). Extending all three is needed, and nothing in the repo does it yet (unverified beyond 7).
- **Stat page chapter locks are hard-coded.** The lock table is in `HelpBox.c:14`. Changing a lock needs a rebuild (see [StatScreenChapterLocks.md](../Features/StatScreenChapterLocks.md)).
- **Clear the text slots you use.** `DrawPage7Rework` clears `gStatScreen.text[14..20]` and `[29..34]` before drawing (`DrawPage7.c:34-36`). Do the same in a new page.
- **Shared text pool.** `gPrepItemTexts[32]` (`Tools/FE-CLib-Mokha/include/prepscreen.h:128`) is reused by the Forge menu (`ForgeMenu.c:219-236`, slots 0-12) and by the Crafting menu (`Kernel/Wizardry/Crafting/Crafting.c`). Two screens drawing into it at once would clash. Which screens are open together is unverified.
- **Help box VRAM.** The stat screen and shop put help-box tiles at `0x06012000` in safe modes (`HelpBox.c:160-174`), because the skill-capacity circle is at `0x06013760` (`HelpBox.c:160` comment). Do not move them back to `0x06013000` on those screens.
- **OBJ palette slots.** The map rewrites OBJ palettes 8-10 every frame, so a window that uses them loses its colours on the map. The Forge menu uses palette 4 for this reason (`ForgeMenu.c:210`, comment).
- **Signed `%`.** Do not use `%` on signed values. The kernel links `__aeabi_idivmod` to vanilla `__modsi3`, which drops the remainder (`LanguageSelect.c:309`). Use `DivRem` or a loop. See CLAUDE.md.
- **Popup component table is sparse.** `gPopupComponents` is designated-initialised, so an opcode with no entry is NULL. `ParsePopupInstAndGetLen` skips NULL `get_len` entries (`PopupR.c:181`), so a missing entry draws nothing and does not error.
- **`Documentation/Features/PopupR.md` is a stub.** It is three lines about the skill popup stack. Use this doc.
- **Window style follows VeslyUI.** `DrawUiFrame` is replaced (`VeslyUI/C_Code.c:670`), so the last argument is a window colour, not a free tile choice. Generated styles mix frame tiles by position (see [VeslyUIStyles.md](../Features/VeslyUIStyles.md)).
- **Installer hook sites.** `LanguageSelect` replaces the first child of the game-control proc list at `$591710` (`LanguageSelect_Installer.event:10`). Two features that replace the same vanilla address will conflict. Whether the build detects this is unverified.
- **Blocking children stop input.** `Proc_StartBlocking` stops the parent. `ForgeMenu.c:185` uses it for the skill cursor, and the proc must yield back (`PROC_YIELD`) before the menu acts on the same key (`ForgeMenu.c:163-175`).
- **Proc struct size.** The `struct Text` inside `ExampleProc` in the template is an unverified choice. Check the proc allocation size before adding large fields.
- **Put UI code in the custom block.** `PopupRework` (`wizardry.event:63`) and `StatScreen` (`:108`) sit in the kernel section. New features go in the `WIZARDRY_CUSTOM` block (`wizardry.event:129-191`).
