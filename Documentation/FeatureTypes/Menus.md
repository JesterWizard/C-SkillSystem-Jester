# Menus

> Code: `Data/UnitMenu/`, `Data/MapMenus/`, `Kernel/Wizardry/SkillSys/PrepSkill/Source/`, `Kernel/Wizardry/ExtrasMenu/`, `Kernel/Wizardry/MenuOptions/`, `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c`  ·  Entry points: `gUnitActionMenuItemsRework` (`Data/UnitMenu/Source/UnitMenu.c:138`), `gMapMenuItems_NEW` / `gMapMenuDef_NEW` (`Data/MapMenus/Usability.c:31`, `:45`), `gPrepMenuTable` (`Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c:92`), `InitSaveMenuChoice` (`Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c:63`), `gGameOptions_NEW` (`Kernel/Wizardry/MenuOptions/MenuOptions.c:154`), `StartMenu` / `StartOrphanMenu` (`Tools/FE-CLib-Mokha/include/uimenu.h:159`, `:161`)  ·  Existing docs: `Documentation/Features/UnlimitedPrepMenuOptions.md`, `Documentation/Features/Prestige.md`, `Documentation/Features/SystemConfig.md`, `Documentation/SkillTypes/MenuSkills.md`

## What it covers

Adding a selectable command (not a skill) to a menu: a row in the unit action menu, the map menu, the prep screen, the title/save "Extras" list, the Options screen, or a standalone menu started from C or an event. Examples: `PrestigeCommand` (unit action row, config-gated), `MapMenu_AchievementsCommand` (map menu row that opens a proc), `Augury` (prep screen row), `SHOW_TRUE_2RN` (Options row with a designer gate), `ForgeMenu` (standalone menu). Skill submenus are in [`../SkillTypes/MenuSkills.md`](../SkillTypes/MenuSkills.md). Here "standalone" means a menu opened from code or an event; the FEBuilder `Standalone/` patch folders are a different thing (see [Standalone.md](Standalone.md)).

## How the engine calls it

A menu is one `struct MenuDef` (position, style, a row array, init/end, and three menu-level callbacks) plus a `struct MenuItemDef` row array (`Tools/FE-CLib-Mokha/include/uimenu.h:27`, `:45`). Each row has a name, two text ids, a colour, an `overrideId` byte, and these callbacks: `isAvailable(def, number)` returns `MENU_ENABLED` / `MENU_DISABLED` / `MENU_NOTSHOWN`; `onDraw(menu, item)`; `onSelected(menu, item)` returns a `MENU_ACT_*` bit mask; `onIdle`; `onSwitchIn` / `onSwitchOut` (cursor on or off the row). The menu-level callbacks are `onInit`, `onEnd`, `onBPress` (returns flags), `onRPress(menu)` and `onHelpBox(menu, item)` (`uimenu.h:45-56`). The order in which the engine calls these is not traced here (unverified).

Per kind:

1. **Unit action menu.** Vanilla `gUnitActionMenuDef` is at ROM `0x59D1F0` (`Kernel/Wizardry/UnitMenu/UnitMenu.event:4`). Its `menuItems` pointer (`+0x8`, `0x59D1F8`) is repointed to `gUnitActionMenuItemsRework` (`Data/UnitMenu/UnitMenu.event:11`). The CHAX branch of the table is `Data/UnitMenu/Source/UnitMenu.c:162-263`. Vanilla opens the menu (path not traced).
2. **Map menu.** `PlayerPhase_MainIdle` (`Data/MapMenus/Usability.c:57`, ROM hook `Data/MapMenus/LynJump.event:2` at `$1C940`) opens `gMapMenuDef_NEW` with `StartOrphanMenuAdjusted` on an A press over an empty tile (`Usability.c:118`). `Data/Data.event:40` includes the generated `Usability.lyn.event`.
3. **Prep screen.** Each time the prep main menu opens, `InitPrepScreenMainMenu` (`AtMenu.c:493`) calls `SetPrepScreenMenuItem(index, ...)` (`AtMenu.c:472`) in a fixed order. Each call appends the matching `gPrepMenuTable` row to the visible list. A row's callback sets `proc->state = index + 1` and jumps to label `0xA`; `AtMenu_StartSubmenu` (`AtMenu.c:129`) switches on `state`. The index enum is `prep_atmenu_item_index` (`Tools/FE-CLib-Mokha/include/prepscreen.h:45`).
4. **Extras (title / save menu).** `InitSaveMenuChoice` (`ExtrasMenu/Source/SupportMenu.c:63`, ROM `$AB89C` via `ExtrasMenu/Source/LynJump.event`) adds rows with `AddExtraMenuOption` (`Tools/FE-CLib-Mokha/include/savemenu.h:349`) using the `EXTRA_MENU_OPTION_*` bits (`savemenu.h:64-69`). Each row is gated by an `IsExtra*Enabled` predicate (`Tools/FE-CLib-Mokha/include/bmsave.h:465-471`). The dispatch of a chosen extra is vanilla (not traced).
5. **Options.** Rows are `gGameOptions_NEW[GAME_OPTION_*]` (`MenuOptions.c:154`; enum in `Tools/FE-CLib-Mokha/include/uiconfig.h:49-88`). Display order is `gGameOptionsUiOrder_NEW` (`MenuOptions.c:10`). `IsGameOptionUiVisible` (`MenuOptions.c:48`) hides rows whose designer gate is off. Values live in the `gPlaySt.config` bitfield (`Tools/FE-CLib-Mokha/include/types.h:140`), read by `GetGameOption` (`MenuOptions.c:738`) and written by `SetGameOption` (`MenuOptions.c:674`). The map menu reaches it through the vanilla `MapMenu_OptionsCommand` row (`Data/MapMenus/Usability.c:38`).
6. **Standalone.** Code calls `StartMenu(&def, parent)` (`uimenu.h:159`) or `StartOrphanMenu(&def)` (`uimenu.h:161`). Events reach it through an `ASMC` wrapper with the signature `CallForgeMenuASMC(struct EventEngineProc *)` (`ForgeMenu.c:446-456`). The only live call in this repo is from C at `Kernel/Wizardry/VeslyDebugger/C_Code.c:5135`; the event form is commented out in `CustomCampaign/Chapters/00/events/events.c:209`. Event-side ASMC calls look like `ASMC(GrantBEXP)` (`CustomCampaign/Chapters/01/events/events.c:56`) and `ASMC(SetGameOptions)` (`CustomCampaign/Chapters/00/events/events.c:103`).

## Where to look

| File | Role |
|---|---|
| `Tools/FE-CLib-Mokha/include/uimenu.h` | `MenuItemDef`, `MenuDef`, `MENU_ENABLED`/`MENU_DISABLED`/`MENU_NOTSHOWN` (`:99-101`), `MENU_ACT_*` (`:122-129`), `StartMenu` / `StartOrphanMenu`, stock callbacks (`MenuAlwaysEnabled`, `MenuCancelSelect`, `MenuAutoHelpBoxSelect`, `MenuStdHelpBox`) |
| `Data/UnitMenu/Source/UnitMenu.c` | Unit action table `gUnitActionMenuItemsRework` (`:138`, ends `:335`); forward declarations of each command (`:24-45`) |
| `Data/UnitMenu/Source/PrestigeCommand.c` | Unit action row example: usability (`:10`) and effect (`:80`) |
| `Data/UnitMenu/UnitMenu.event` | Repoints the table (`:11`); `#include`s every `Source/*.lyn.event` |
| `Kernel/Wizardry/UnitMenu/UnitMenu.event` | Vanilla `gUnitActionMenuDef` address (`:4`) |
| `Data/MapMenus/Usability.c` | Map menu rows (`:31-43`), def (`:45`), A-press opener (`:118`), config-gated usability (`:15`, `:23`) |
| `Data/MapMenus/Usability.lyn.event` | Build output from `Usability.c`; edit the `.c` |
| `Kernel/Wizardry/VeslyAchievements/AchievementsMenu.c` | Map command that starts a proc and ends the menu (`:135-140`) |
| `Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c` | Prep table (`:92-105`), table size (`:107`), `AtMenu_StartSubmenu` (`:129`), `SetPrepScreenMenuItem` (`:472`), `InitPrepScreenMainMenu` (`:493`) |
| `Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/` | Prep submenu screens; `TEMPLATE.c` is the repo's own starting point |
| `include/jester_headers/custom-structs.h` (`:156`), `include/link/config-memmap.s` (`:115`) | `gPrepMenuVisibleTableSlots[12]`, the 12-slot array |
| `Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c` | `InitSaveMenuChoice` (`:63`), the extras gating |
| `Tools/FE-CLib-Mokha/include/savemenu.h` | `EXTRA_MENU_*` (`:56-61`), `EXTRA_MENU_OPTION_*` (`:64-69`), `AddExtraMenuOption` (`:349`) |
| `Kernel/Wizardry/MenuOptions/MenuOptions.c` | Option rows, order `[33]` (`:10`), visibility (`:48`), get/set (`:674`, `:738`), defaults in `InitPlayConfig` (`:793`) |
| `Tools/FE-CLib-Mokha/include/uiconfig.h` | `GAME_OPTION_*` enum, `struct GameOption` (`:13`) |
| `Tools/FE-CLib-Mokha/include/types.h` | `struct PlaySt_OptionBits` (`:140`), spare bits `custom_option_23`..`_32` |
| `Contents/Texts/Source/texts/Menu_Options.txt` | Option title, description, On/Off strings (`## MSG_...` then the text) |
| `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c` | Standalone menu example (`gForgeMenuItems` `:32`, `gForgeMenuDef` `:188`, `CallForgeMenuASMC` `:446`) |
| `Kernel/Wizardry/Crafting/Crafting.c`, `Crafting_Installer.event` | Menu defined as Event Assembler data; not installed (`Kernel/Wizardry/wizardry.event:143` is commented out) |
| `Data/DesignerConfig/designer-config.c`, `include/kernel/kernel-lib.h` | Runtime gate fields (`KernelDesigerConfig`) |
| `include/common-chax.h` | `#define CHAX 1` (`:12`), which selects the CHAX branch of the unit table |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the kind from "How the engine calls it". The kinds have different registration points and are not interchangeable.
3. Write the callbacks. `isAvailable` returns one of `MENU_ENABLED`, `MENU_DISABLED`, `MENU_NOTSHOWN`. `onSelected` returns a `|` of `MENU_ACT_*` bits. Use `MENU_ACT_END` to close the menu, `MENU_ACT_CLEAR` when the command hands the screen to another proc, and `MENU_ACT_SND6A` / `MENU_ACT_SND6B` for the select / deny sounds (the sound meanings are inferred from `PrestigeCommandEffect` and `ForgeMenuOnSelect`, not from the engine).
4. Register the row in its table (see the Template subsections). For the unit action menu, forward-declare the callbacks in `UnitMenu.c`.
5. If the row is optional, add a `u8` field to `KernelDesigerConfig` (`include/kernel/kernel-lib.h`), a `.field = ..., // comment` line in `Data/DesignerConfig/designer-config.c`, and a row in `Documentation/Features/SystemConfig.md`. Read it as `gpKernelDesignerConfig->field`.
6. Build and test in game. Not covered here.

## Template

Each kind has its own registration. Names with `Example` are placeholders. Message ids (`MSG_Example_*`) must be added to a text file in `Contents/Texts/Source/texts/`.

### Unit action menu row

Adapted from `Data/UnitMenu/Source/PrestigeCommand.c` (`:10` usability, `:80` effect).

A new file `Example.c` in the unit menu source folder `Data/UnitMenu/Source/`:

```c
#include "common-chax.h"
#include "bmmenu.h"
#include "kernel-lib.h"
#include "constants/texts.h"

u8 ExampleCommandUsability(const struct MenuItemDef *def, int number)
{
	(void)def;
	(void)number;

	/* optional: if (!gpKernelDesignerConfig->example_menu) return MENU_NOTSHOWN; */
	if (!UNIT_IS_VALID(gActiveUnit))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 ExampleCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	(void)menu;
	(void)menuItem;

	gActionData.unitActionType = UNIT_ACTION_WAIT; /* PrestigeCommandEffect does this */
	/* start the proc or anim for the command here */
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}
```

Row, in `gUnitActionMenuItemsRework` (`Data/UnitMenu/Source/UnitMenu.c`, before `MenuItemsEnd` at `:335`), and forward declarations near `:44`:

```c
	{" Example", MSG_Example_Name, MSG_Example_Desc, TEXT_COLOR_SYSTEM_WHITE, 0x7D, ExampleCommandUsability, 0, ExampleCommandEffect, 0, 0, 0},
```

Include, in `Data/UnitMenu/UnitMenu.event` next to the `PrestigeCommand` line:

```
#include "Source/Example.lyn.event"
```

### Map menu row

Adapted from `MapMenu_IsAchievementsCommandAvailable_Config` (`Data/MapMenus/Usability.c:15`) and `MapMenu_AchievementsCommand` (`Kernel/Wizardry/VeslyAchievements/AchievementsMenu.c:135`).

In `Data/MapMenus/Usability.c` (the callbacks are in the same file, so there is no include step):

```c
u8 MapMenu_ExampleCommandAvailable(const struct MenuItemDef *def, int number)
{
	(void)def;
	(void)number;

	if (!gpKernelDesignerConfig->example_menu)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 MapMenu_ExampleCommand(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	(void)menu;
	(void)menuItem;

	Proc_Start(ProcScr_Example, PROC_TREE_3); /* a proc you write (see UI.md) */
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}
```

Row, in `gMapMenuItems_NEW` (`Usability.c:31`), before `MenuItemsEnd`:

```c
    {"Example", MSG_Example_Name, MSG_Example_Desc, 0, 0x76, MapMenu_ExampleCommandAvailable, 0, MapMenu_ExampleCommand, 0, 0, 0},
```

`0x76` must be a value no other row uses (check the table; uniqueness is unverified).

### Prep screen row

Adapted from the Augury row path: `PrepScreenMenu_OnAugury` (`AtMenu.c:49`), `gPrepMenuTable` (`AtMenu.c:92`), `AtMenu_StartSubmenu` (`AtMenu.c:129`), `InitPrepScreenMainMenu` (`AtMenu.c:493`).

`Tools/FE-CLib-Mokha/include/prepscreen.h`, in `prep_atmenu_item_index` (`:45`): add `PREP_MAINMENU_EXAMPLE = 12,` (see Gotchas: 12 is also named in a commented-out line there).

`Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c`, above `gPrepMenuTable` (callbacks must come first; the table has no forward declarations):

```c
void PrepScreenMenu_OnExample(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = PREP_MAINMENU_EXAMPLE + 1;
    Proc_Goto(proc, 0xA);
}
```

Row, in `gPrepMenuTable`:

```c
    { PREP_MAINMENU_EXAMPLE, PrepScreenMenu_OnExample, TEXT_COLOR_SYSTEM_WHITE, MSG_Example_Title, MSG_Example_Desc, 0 },
```

Case, in the `switch` in `AtMenu_StartSubmenu`:

```c
        case PREP_MAINMENU_EXAMPLE + 1: StartExampleScreen_FromPrep(proc); break;
```

Gate, in `InitPrepScreenMainMenu`, at the position where the row should appear (display order follows the call order):

```c
        if (gpKernelDesignerConfig->prep_menu_example == true)
            SetPrepScreenMenuItem(PREP_MAINMENU_EXAMPLE, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
```

The table is already 12 rows, the size of `gPrepMenuVisibleTableSlots`. A 13th row needs that array (and its `_kernel_malloc` size) enlarged. See Gotchas.

### Extras (title / save menu) row

Adapted from `InitSaveMenuChoice` (`Kernel/Wizardry/ExtrasMenu/Source/SupportMenu.c:63`). Only gating an existing row is verified:

```c
    if (IsExtraFreeMapEnabled() != 0)
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_MAP);
```

Adding a new `EXTRA_MENU_*` value also needs a vanilla dispatch for it. That dispatch is not traced (unverified).

### Options row

Adapted from the `GAME_OPTION_ACHIEVEMENTS` and `GAME_OPTION_SHOW_TRUE_2RN` rows (`Kernel/Wizardry/MenuOptions/MenuOptions.c:645`, `:659`). Each step below is one edit site.

1. `Tools/FE-CLib-Mokha/include/uiconfig.h`: add `GAME_OPTION_EXAMPLE = 37,` after `GAME_OPTION_SHOW_TRUE_2RN = 36`.
2. `Tools/FE-CLib-Mokha/include/types.h`: rename a spare bit in `struct PlaySt_OptionBits` (`custom_option_23`..`custom_option_32`), e.g. `u32 example: 1;`.
3. `Kernel/Wizardry/MenuOptions/MenuOptions.c`: append `GAME_OPTION_EXAMPLE,` to `gGameOptionsUiOrder_NEW` (`:10`) and change its size from `[33]` to `[34]`.
4. Same file, in `gGameOptions_NEW`:

```c
    [GAME_OPTION_EXAMPLE] =
    {
        .msgId = MSG_MENU_OPTION_EXAMPLE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_EXAMPLE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_EXAMPLE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000, MSG_000, 190, 0 },
            { MSG_000, MSG_000, 189, 0 },
        },
        .icon = 0x44, /* index into the icon sheet used by DrawGameOptionIcon */
        .func = GenericOptionChangeHandler,
    },
```

5. Same file: `case GAME_OPTION_EXAMPLE: gPlaySt.config.example = newValue; break;` in `SetGameOption` (`:674`), and `case GAME_OPTION_EXAMPLE: return gPlaySt.config.example;` in `GetGameOption` (`:738`).
6. Same file: `gPlaySt.config.example = 0;` in `InitPlayConfig` (`:793`; compare the `show_true_2rn` default at `:840`).
7. `Contents/Texts/Source/texts/Menu_Options.txt`:

```
## MSG_MENU_OPTION_EXAMPLE_TITLE
Example[X]
## MSG_MENU_OPTION_EXAMPLE_DESC
Example description[X]
```

8. If the row is designer-gated, add a `return false` case in `IsGameOptionUiVisible` (`:48`), as the `show_true_2rn` case does (`:54-56`).

### Standalone menu (from C or an event)

Adapted from `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c` (`gForgeMenuItems` `:32`, `gForgeMenuDef` `:188`, `DrawForgeMenuItem` `:378`, `CallForgeMenuASMC` `:446`). The `MenuDef` shape matches `Data/MapMenus/Usability.c:45`.

A new file `Example.c` in the feature's own folder, `Kernel/Wizardry/<Feature>/Source/`:

```c
#include "common-chax.h"
#include "bmmenu.h"
#include "uiutils.h"
#include "fontgrp.h"
#include "hardware.h"
#include "icon.h"

static int ExampleMenuDraw(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	(void)menu;

	PutDrawText(&menuItem->text,
		TILEMAP_LOCATED(gBG0TilemapBuffer, menuItem->xTile, menuItem->yTile),
		TEXT_COLOR_SYSTEM_WHITE, 0, 0,
		GetStringFromIndex(menuItem->def->nameMsgId));

	BG_EnableSyncByMask(BG0_SYNC_BIT);
	return 0;
}

static u8 ExampleMenuSelect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	(void)menu;
	(void)menuItem;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

const struct MenuItemDef gExampleMenuItems[] = {
	{"", MSG_Example_Title, MSG_Example_Desc, TEXT_COLOR_SYSTEM_WHITE, 0, MenuAlwaysEnabled, ExampleMenuDraw, ExampleMenuSelect, 0, 0, 0},
	MenuItemsEnd
};

const struct MenuDef gExampleMenuDef = {
	{1, 1, 12, 0},            /* x, y, w, h in tiles (h = 0 as in ForgeMenu and the map menu) */
	0,                        /* style */
	gExampleMenuItems,
	0, 0, 0,                  /* onInit, onEnd, _u14 */
	MenuCancelSelect,         /* onBPress */
	MenuAutoHelpBoxSelect,    /* onRPress */
	MenuStdHelpBox            /* onHelpBox */
};

/* Called from an event as ASMC(CallExampleMenuASMC), or from C with a parent proc */
void CallExampleMenuASMC(struct EventEngineProc *proc)
{
	ClearBg0Bg1();
	SetDispEnable(1, 1, 1, 1, 1);
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();
	ResetIconGraphics_();

	StartMenu(&gExampleMenuDef, (ProcPtr)proc);
}
```

The item text uses `MenuItemProc::text`, which is per row and survives across frames (`uimenu.h:89`).

## Worked examples

- `Data/UnitMenu/Source/PrestigeCommand.c`: a unit action row with a designer gate (`:10-35`) whose effect starts a proc through `NewMuSkillAnimOnActiveUnitWithDeamon` (`:99`) and returns `MENU_ACT_*` flags.
- `Data/MapMenus/Usability.c` with `Kernel/Wizardry/VeslyAchievements/AchievementsMenu.c:135-140`: a map menu row that calls `Proc_Start` and returns `MENU_ACT_END | MENU_ACT_CLEAR`, plus a custom draw (`MapMenu_AchievementsCommandDraw`, `Usability.c:8`).
- `Kernel/Wizardry/SkillSys/PrepSkill/Source/CustomMenuOptions/Augury.c` with `AtMenu.c:129-143`: a prep row, including the `state = index + 1` hand-off.
- `Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ForgeMenu.c`: a standalone menu started from an ASMC, with a help box and a sub-proc (`ProcScr_ForgeSkillFocus`, `ForgeMenu.c:163`).

## Gotchas

- **Prep screen is capped at 12 rows.** `PREP_MENU_TABLE_SIZE` is 12 (`AtMenu.c:107`) and `gPrepMenuVisibleTableSlots` is 12 bytes (`custom-structs.h:156`, `config-memmap.s:115`). `InitPrepScreenMainMenu` zeroes `PREP_MENU_TABLE_SIZE` entries (`AtMenu.c:499-500`). A 13th row therefore writes past the array unless the array is enlarged. `Documentation/Features/UnlimitedPrepMenuOptions.md` says "unlimited"; the code does not do that.
- **Prep display order is call order, not table order.** `SetPrepScreenMenuItem` appends in the order it is called (`AtMenu.c:478-483`), and `InitPrepScreenMainMenu` calls it in a fixed sequence (`:503-533`). `UnlimitedPrepMenuOptions.md` says the table order decides; that is stale.
- **Prep index 12 may be spoken for.** `prepscreen.h:58-59` has commented-out `PREP_MAINMENU_FORMATION = 12` and `PREP_MAINMENU_OPTIONS = 12`. No live row uses 12 (`AtMenu.c:92-105`). Whether a vanilla screen still reads index 12 is unverified.
- **Unit menu table is larger than `MENU_ITEM_MAX`.** The header says `MENU_ITEM_MAX = 11` and `MenuProc::menuItems[MENU_ITEM_MAX]` (`uimenu.h:10`, `:66`), yet `gUnitActionMenuItemsRework` has far more rows (`UnitMenu.c:138-335`). How the engine copes with that is unverified. Do not rely on the header number.
- **`overrideId` has two uses.** Feature code reads it as a payload: Crafting uses it as a recipe index (`Kernel/Wizardry/Crafting/Crafting.c:48`, `:83`, `:95`, `:131`). The vanilla rows store values such as `0x6E`, `0x7A`, `0x7C`, which look like command ids; `struct MenuItemOverride` (`uimenu.h`) keys on a `cmdid`. Whether the engine looks up `overrideId` for vanilla rows, and whether values must be unique, is unverified.
- **Disabled rows still get `onSelected`.** `OnRecipeSelect` (`Crafting.c:189`) and `ForgeMenuOnSelect` (`ForgeMenu.c:409`) check `availability == MENU_ENABLED` themselves and show a frozen help box otherwise. This suggests `MENU_DISABLED` rows still receive the press, but the engine path was not traced (unverified).
- **Callbacks need a forward declaration in `UnitMenu.c`.** The table lists functions before they are defined, so each one needs a prototype (`UnitMenu.c:24-45`). Map menu callbacks can be defined above the table in `Usability.c`.
- **CHAX branch only.** `Data/UnitMenu/Source/UnitMenu.c:162` is `#if CHAX`, and `CHAX` is 1 (`include/common-chax.h:12`). The `#else` vanilla rows (`:263-270`) are not built. Edit the CHAX branch.
- **`Usability.lyn.event` is build output.** Edit `Data/MapMenus/Usability.c`; the build regenerates `Usability.lyn.event` (`makefile:211`). CLAUDE.md warns that some `.lyn.event` files have hand edits, so check before regenerating any of them.
- **Options array size is fixed.** `gGameOptionsUiOrder_NEW` is declared `[33]` and lists 33 entries (`MenuOptions.c:10-46`). Adding a row without bumping the size is a silent error, and the order array is what the screen walks.
- **`SetGameOption` has no default branch.** An option with no `case` is silently not stored (`MenuOptions.c:674-737`). Add the `case` in both `SetGameOption` and `GetGameOption`.
- **Options bits are the save layout.** `gPlaySt.config` is a bitfield (`types.h:140`) with spare bits named `custom_option_*`. Whether it is written to the save file is unverified; check `Documentation/Memory.md` before adding bits that must persist.
- **Extras enum is fixed.** `EXTRA_MENU_*` runs 0-6 (`savemenu.h:56-61`), and `EXTRA_MENU_6` is declared but not used by any row in this repo. A new value needs a vanilla dispatch (unverified).
- **Standalone menus need the setup calls.** `CallForgeMenuASMC` sets display, fonts, UI frame graphics and icon graphics before `StartMenu` (`ForgeMenu.c:446-456`). Skipping them leaves the menu without its graphics. The sample `Example.c` above copies the same calls.
- **Crafting menu is not installed.** `Kernel/Wizardry/Crafting/Crafting_Installer.event` is included only by a commented-out line (`Kernel/Wizardry/wizardry.event:143`). Its `CallSynthesisMenuASMC` (`Crafting.c:32`) is reachable only if that line is enabled.
- **`PopupRework` names are not `Menus`.** Popups are in `Documentation/FeatureTypes/UI.md`, not here.
