# Unlimited Prep Menu Options

The Preparations menu is a scrolling list built from a ROM table instead of the vanilla fixed-size list. Extension rows from add-on patches can be added without recompiling this patch. Integrated documentation: [Documentation/Features/UnlimitedPrepMenuOptions.md](../../Documentation/Features/UnlimitedPrepMenuOptions.md).

## Behaviour

Rows, in the order the vanilla game draws them (taken from the clean ROM's `InitPrepScreenMainMenu` at `0x08095524`):

| Row | Vanilla key | Title | Shown | Grey (A plays the error sound) |
|-----|-------------|-------|-------|--------------------------------|
| Pick Units | 0 | `0x574` | always | never |
| Items | 1 | `0x576` | always | never |
| Support | 4 | `0x577` | when the vanilla support check passes (`gPlaySt+0x14` bit 7 clear) | when the vanilla support check fails |
| Check Map | 7 | `0x578` | always | when `CanPrepScreenCheckMap()` is false (chapter `0x7D`) |
| Save | 2 | `0x579` | always | when `CanPrepScreenSave()` is false |
| Extension rows | `0x40 + slot` | from the row | from `isVisible` | from `color == TEXT_COLOR_SYSTEM_GRAY` |

Link arena: Pick Units, Items and the vanilla arena row (key 3, title `0x75C`, `sub_8095284`). Extension rows are added there too.

- The list shows 5 rows and scrolls. The scrollbar and the cursor work as in the integrated feature.
- Check Map opens the vanilla four-row submenu (View Map `0x590`, Formation `0x591`, Options `0x592`, Save `0x579`). R shows its help text (`0x5BB` to `0x5BE`).
- The description under the list comes from the vanilla description lookup (`sub_8095024`). For extension rows it is the row's `help` string.
- Pressing A on a row plays the window select sound once. The vanilla callbacks do not play a sound, so the row functions play it.
- Items returns with the cursor on Items. Check Map returns to the top row, the same as the clean ROM (checked in an emulator).
- An extension row's submenu returns to the menu the way the integrated kernel's Hex row does: the submenu's end sets `proc->cur_cmd = 0x40 + slot` and `proc->state = 1`. The AtMenu script then goes to label 9 (`AtMenu_Reinitialize`, fade-in, then the menu loop). `PMX_InitPrepScreenMainMenu` puts the cursor back on the row. See "Writing an extension row" below.

## Writing an extension row

This is the contract for an add-on row (the full text is in `Source/PrepMenuExt.h`). The test row in `test/TestRow.c` follows it.

```c
#include "PrepMenuExt.h"   /* the patch's Source/ folder */
#include "proc.h"
#include "soundwrapper.h"
#include "constants/songs.h"

#define MYROW_SLOT 1       /* the slot the add-on writes (0 to 7) */

struct ProcMyRow { PROC_HEADER; };

/* The screen's end callback hands control back to the menu. */
static void MyRow_OnEnd(ProcPtr proc)
{
    struct ProcAtMenu *pproc = (struct ProcAtMenu *)((struct ProcMyRow *)proc)->proc_parent;
    pproc->cur_cmd = PREP_MENU_EXT_KEY_BASE + MYROW_SLOT; /* cursor goes back to this row */
    pproc->state = 1;                                     /* label 9: redraw, fade in, menu */
}

const struct ProcCmd ProcScr_MyRow[] = {
    PROC_NAME("MyRow"),
    PROC_SET_END_CB(MyRow_OnEnd),
    PROC_SLEEP(60),        /* your screen here (fade, draw, wait for input, ...) */
    PROC_END,
};

static void MyRow_OnSelect(struct ProcAtMenu *proc)
{
    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
    proc->state = PREP_MENU_EXT_STATE_BASE + MYROW_SLOT;  /* 0x40 + slot */
    Proc_Goto(proc, 0xA);                                 /* the menu fades out, then calls startSubmenu */
}

static void MyRow_StartSubmenu(struct ProcAtMenu *proc)
{
    Proc_StartBlocking(ProcScr_MyRow, proc);              /* proc is the parent */
}

const struct PrepMenuExtRow MyRowDef = {
    "My row", "Help text", 0,
    MyRow_OnSelect, MyRow_StartSubmenu,
    TEXT_COLOR_SYSTEM_WHITE, { 0, 0, 0 },
};
```

Rules:

- **Return with `state = 1`.** Any of 1, 2, 4 or 5 goes to label 9. Do not use 3 (label 7, the Save path).
- **Do not return with `Proc_Goto` on the AtMenu proc.** Label 0xD runs the old `AtMenu_CtrlLoop`, which takes A itself and opens the Support fortune popup (it sets `state = 4`). Labels 1 and 0xC leave the screen blank. The earlier test row did this, and it is the cause of the "Rank / Augury" popup that was open before this fix.
- **A row with no screen** must still hand back. Set `pproc->cur_cmd` and `pproc->state = 1` in `startSubmenu` itself. This is not tested in the emulator.
- **`startSubmenu` must not be NULL** for a row that does anything. With NULL, nothing sets the state back, and the screen stays faded out.
- The child screen should not call `EndPrepScreenMenu` or the other prep-menu functions. `AtMenu_StartSubmenu` already ends the menu's own proc (`sub_8095C2C`).
- The display is off after the fade-out. A child that draws must turn its layers on (the test row uses `SetDispEnable(0, 0, 0, 0, 0)` and a backdrop colour, and the Hex screen calls `SetDispEnable(1, 1, 0, 1, 1)`).

## Target ROM

- **FE8U (USA)**, clean ROM. The header reads `FIREEMBLEM2EBE8E`.
- Not for the integrated C Skill System kernel. Do not install over `PrepSkill`.

## Build

No build step is needed to install. `Source/PrepMenuUnlimited.lyn.event` is checked in.

To rebuild after editing `Source/PrepMenuUnlimited.c` or `Source/RamAlloc.s`:

```bash
make -C Standalone/unlimited_prep_menu_options
```

This needs devkitARM, [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) and the repo's Event Assembler tools (see [Documentation/Setup.md](../../Documentation/Setup.md)). The makefile also reads `configs/configs.h` from the repo `include/` folder, as other standalones do.

## Installation

The folder is self-contained. It does not need `EAstdlib.event` or `Hack Installation.txt` (FEBuilder's Event Assembler provides `jumpToHack`).

### FEBuilderGBA

1. Open a clean FE8U ROM.
2. Go to **Advanced Editors → Insert EA**.
3. Select `Installer.event` and load it.

### Event Assembler (command line)

From a copy of clean `fe8.gba`, run in this folder:

```bash
EA_DIR=<repo>/Tools/EventAssembler
{
  echo '#include "Extensions/Hack Installation.txt"'
  echo '#include "Installer.event"'
} | "$EA_DIR/ColorzCore" A FE8 -I:"$EA_DIR" -output:fe8-unlimited-prep-menu.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | Hook addresses, the extension header, and the body include |
| `Source/PrepMenuUnlimited.c` | Menu code (hooks and row logic) |
| `Source/PrepMenuUnlimited.lyn.event` | Checked-in lyn output, included by the installer |
| `Source/PrepMenuExt.h` | Add-on header: `struct PrepMenuExtRow`, slot layout, magic, state numbers |
| `Source/RamAlloc.s` | EWRAM addresses for the visible list and two flag bytes |
| `makefile` | Builds `Source/PrepMenuUnlimited.lyn.event` from the C source |
| `test/` | Test-only helpers. Do not install them (see `test/README.md`) |

## Hooks and free space

Each hook is `jumpToHack` at a vanilla entry point, which overwrites 8 bytes (`ldr r3,[pc,#0]; bx r3` plus a 4-byte pointer). Every entry point is 4-byte aligned.

| Vanilla function | Hook (`ORG`) | Overwritten | Replacement |
|------------------|--------------|-------------|-------------|
| `PrepScreenMenu_OnCheckMap` | `$952C0` | 8 bytes | `PMX_PrepScreenMenu_OnCheckMap` |
| `PrepMenu_OnInit` | `$96CCC` | 8 bytes | `PMX_PrepMenu_OnInit` |
| `PrepMenu_CtrlLoop` | `$96D10` | 8 bytes | `PMX_PrepMenu_CtrlLoop` |
| `AtMenu_Reinitialize` | `$95C84` | 8 bytes | `PMX_AtMenu_Reinitialize` |
| `sub_8095C00` (description) | `$95C00` | 8 bytes | `PMX_UpdatePrepMenuDescription` |
| `SetPrepScreenMenuPosition` | `$97200` | 8 bytes | `PMX_SetPrepScreenMenuPosition` |
| `SetPrepScreenMenuItem` | `$97024` | 8 bytes | `PMX_SetPrepScreenMenuItem` |
| `InitPrepScreenMainMenu` | `$95524` | 8 bytes | `PMX_InitPrepScreenMainMenu` |
| `AtMenu_StartSubmenu` | `$9631C` | 8 bytes | `PMX_AtMenu_StartSubmenu` |
| `DrawPrepScreenMenuFrameAt` | `$97154` | 8 bytes | `PMX_DrawPrepScreenMenuFrameAt` |
| `GetActivePrepMenuItemIndex` | `$9710C` | 8 bytes | `PMX_GetActivePrepMenuItemIndex` |
| `SetPrepScreenMenuSelectedItem` | `$970CC` | 8 bytes | `PMX_SetPrepScreenMenuSelectedItem` |

**Free space:** `$1000000` to `$1001060` (4192 bytes). The first 36 bytes are the extension header (`PMX1` magic at `$1000000`, then 8 slots at `$1000004`). The body follows at `$1000024`.

`Installer.event` protects the magic (`$1000000` to `$1000004`) and the body (`$1000024` to the end), but **not** the 8 slot words at `$1000004` to `$1000023`. Add-ons must be able to write their slot. If the slots were protected, an add-on that writes one in the same EA run would stop with "Trying to write data to area protected".

## RAM

Three EWRAM symbols are set in `Source/RamAlloc.s`: 20 bytes in all, `0x0203C000` to `0x0203C013` inclusive:

| Symbol | Address | Size | Use |
|--------|---------|------|-----|
| `gPrepMenuVisibleTableSlots` | `0x0203C000` | 16 bytes | Visible rows: key, plus `0x80` when grey |
| `gPrepMenuTopVisibleIndex` | `0x0203C010` | 1 byte (2 reserved) | Scroll position |
| `gPrepMenuBuildingList` | `0x0203C012` | 1 byte (2 reserved) | Set while the main rows are built |

The sizes keep to even numbers. The 16-byte list holds up to 5 vanilla rows (5 in the main menu, 3 in the link arena) plus 8 extension rows.

## Add-on interface (extension rows)

An add-on adds a row without recompiling this patch. It writes a pointer into one of the 8 slots at `$1000004`, in the **same EA run** as this installer (FEBuilder's Insert EA of `Installer.event`, or one `ColorzCore` command that includes both):

```
PUSH
ORG $1000004 + 4*N        ; N = slot, 0 to 7
POIN MyPrepRowDef          ; a struct PrepMenuExtRow in the add-on
POP
```

- The add-on's own code and data must not be placed inside this patch's body (`$1000024` up to the end of the installer). Put them after the body in the same run (`ORG CURRENTOFFSET` after the installer), or at the add-on's own free space address.
- A separate Insert EA run does not remember this patch's `PROTECT` ranges. Each run starts with no protection, so a second run can write anywhere, including this patch's body. Do not rely on it. Install the patch and its add-ons in one run when they write into `$1000000` or the body.

```c
struct PrepMenuExtRow {
    const char *title;                             /* FE8 text, drawn in the row */
    const char *help;                              /* FE8 text, may be NULL; '\n' or 0x01 = new line */
    bool (*isVisible)(void);                       /* NULL = always shown */
    void (*onSelect)(struct ProcAtMenu *proc);     /* A pressed: play the sound, set proc->state, Proc_Goto(proc, 0xA) */
    void (*startSubmenu)(struct ProcAtMenu *proc); /* called from AtMenu_StartSubmenu; must not be NULL for a real row */
    u8 color;                                      /* TEXT_COLOR_SYSTEM_*; GRAY = not selectable */
    u8 pad[3];
};
```

- **Slot layout:** `$1000000` holds the magic `PMX1` (`0x50 0x4D 0x58 0x31`). `$1000004 + 4*k` holds slot `k` as a `POIN`; 0 means empty. Slot `k` appears after the vanilla rows, in slot order.
- **States:** a row's key and state are `0x40 + slot` (`0x40` to `0x47`). Vanilla states stay 1 to 4 (Pick Units, Items, Save, Support).
- **Selecting:** `onSelect(proc)` runs on A. It must play the sound, set `proc->state = 0x40 + slot` and call `Proc_Goto(proc, 0xA)`. `AtMenu_StartSubmenu` then calls `startSubmenu(proc)`.
- **Returning:** the screen's end sets `proc->cur_cmd = 0x40 + slot` and `proc->state = 1`. See "Writing an extension row" above.
- **Test row (not installed):** `test/TestRow.c` follows the contract with a 60-frame child screen.

Full documentation with the C header is in `Source/PrepMenuExt.h`.

## Conflicts

- Any patch that hooks one of the 12 functions above, including the integrated `Kernel/Wizardry/SkillSys/PrepSkill` feature and other prep-menu patches.
- Free space `$1000000` is shared with other standalone bodies. Install one body there, or move this `ORG`.
- EWRAM `0x0203C000` to `0x0203C013` should not be used by another patch.
- Patches that change the vanilla Check Map submenu (`PrepScreenProc_StartMapMenu`) need to be checked against this one.

## Verification status

- `make` in this folder: passes (`-Wall -Wextra -Werror`).
- ColorzCore assembles `Installer.event` on a copy of the clean ROM with no errors. The 12 hook sites hold `00 4B 18 47` followed by a pointer into `$1000000`.
- ColorzCore assembles `Installer.event` plus an add-on that writes slot 0 (`test/TestRow.event`) in one run with no errors. Slot 0 then holds a pointer to the add-on's struct, placed after the patch body (`$100115C` in this build), not inside it.
- Checked in an emulator (mGBA via `gbarun`, test route in `test/README.md`): the Preparations screen, the Items and Check Map submenus and their return, the scrollbar, and START to leave the menu for the chapter map. The screen with this patch matches the clean ROM pixel for pixel.
- Checked in an emulator with the test row (`test/`): it scrolls into view, its help text shows, its child screen opens, and after the child ends the menu returns with the cursor on the Test row, its help text, and no popup. UP moves to Save with its vanilla description. The real Support row still opens the vanilla unit list, the same as the clean ROM. Items returns to Items with B. See `test/README.md`.
- The earlier "Rank / Augury" popup after the test row's return was the `AtMenu_CtrlLoop` path (`state = 4`, label 8). The fix is in the add-on contract above. No menu code changed (`PrepMenuUnlimited.c` is the same as before).

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester), the `Kernel/Wizardry/SkillSys/PrepSkill` feature (`Source/AtMenu.c`). The kernel-only rows (Infuse, Augury, Bonus EXP, Skills, Synthesize, Base conversations, Event replay, Hex) and their designer-config checks are removed.
