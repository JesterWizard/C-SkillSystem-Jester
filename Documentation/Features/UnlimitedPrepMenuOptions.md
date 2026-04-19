# Unlimited Prep Menu Options

<p align="center">
  <img src="../Gifs/Unlimited_Prep_Menu_Options.gif" alt="Unlimited Prep Menu" width="600"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [How-To-Use](#how-to-use)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

``gPrepMenuTable``  
``PrepMenu_CtrlLoop``  
``SetPrepScreenMenuItem``

This feature replaces the vanilla prep menu’s **nested 8-entry struct limitation** with a scrollable, ROM-backed menu system.

The aim is to ensure:

- An unlimited number of prep menu options
- No hard 8-entry struct limit
- No 5-visible-entry restriction
- Scrollbar-based navigation
- Reduced RAM usage
- No pre-stored msg/description buffers
- Lazy-loading of title and description text only when needed
- Direct indexing into a ROM table

Instead of storing menu data in RAM structs, this system:

- Uses a constant ROM table (`gPrepMenuTable`)
- Uses `cur_index` directly as a table index
- Increments `max_index` dynamically
- Only fetches `msg` and `msg_desc` when drawing

---

## 🛠️ How To Use

### 1️⃣ Add a Menu Entry

Add a new entry inside:

```
gPrepMenuTable[]
```

Format:

```
{
    PREP_MAINMENU_* index,
    callback_function,
    TEXT_COLOR_SYSTEM_*,
    title_msg_id,
    description_msg_id,
    r_button_msg_id
}
```

The order in this table determines the display order.

---

### 2️⃣ Enable/Disable via Designer Config

Entries are conditionally enabled in:

```
InitPrepScreenMainMenu
```

Example:

```
if (gpKernelDesignerConfig->prep_menu_augury == true)
    SetPrepScreenMenuItem(PREP_MAINMENU_AUGURY, NULL, TEXT_COLOR_SYSTEM_WHITE, 0, 0);
```

`SetPrepScreenMenuItem` is stubbed and now only:

- Increments `proc->max_index`
- Does NOT store data

All data lives in ROM.

---

### 3️⃣ Submenu Handling

When A is pressed, the callback stored in `gPrepMenuTable` is executed:

```
entry->effect((struct ProcAtMenu *)proc->proc_parent);
```

Submenus are dispatched via:

```
AtMenu_StartSubmenu
```

---

## 🗂️ Code Locations

| Feature | Location | Description |
|----------|----------|-------------|
| **Main Menu Table** | `gPrepMenuTable` | ROM-backed table storing all menu data |
| **Check Map Table** | `gCheckMapMenuTable` | Separate ROM table for Check Map submenu |
| **Menu Initialization** | `PrepMenu_OnInit` | Resets cursor, initializes scrollbar |
| **Main Control Loop** | `PrepMenu_CtrlLoop` | Handles input, scrolling, help text |
| **Submenu Dispatcher** | `AtMenu_StartSubmenu` | Launches selected submenu |
| **Description Updater** | `sub_8095C00` | Updates description dynamically |
| **Menu Position Drawer** | `SetPrepScreenMenuPosition` | Draws visible entries only |
| **Frame Drawer** | `DrawPrepScreenMenuFrameAt` | Draws UI frame and check map entries |
| **Max Index Stub** | `SetPrepScreenMenuItem` | Only increments `max_index` |
| **Active Index Getter** | `GetActivePrepMenuItemIndex` | Returns PREP_MAINMENU_* constant |

---

## 🧠 System Overview

### 📜 ROM-Backed Menu Table

```
static const struct PrepMenuItem gPrepMenuTable[]
```

Each entry contains:

```
{
    index,
    effect,
    color,
    msg,
    msg_desc,
    msg_rtext
}
```

This replaces:

- Nested menu structs
- Pre-allocated RAM buffers
- Fixed 8-entry limitations

---

### 🧮 Visible Count vs Total Count

```
#define PREP_MENU_VISIBLE_COUNT 5
```

Only 5 entries are drawn at once.

However:

```
proc->max_index
```

Can grow dynamically with no structural limit.

Scrolling is handled via:

```
gTopVisibleListIndex
```

---

### 🎮 Input Handling

Handled entirely in:

```
PrepMenu_CtrlLoop
```

Supports:

- A Button → execute callback
- B Button → exit
- START → custom handler
- R Button → dynamic help box
- DPAD Up/Down → scroll with wrap support

Cursor position updates dynamically:

```
ShowSysHandCursor(...)
```

Scrollbar updates via:

```
UpdateMenuScrollBarConfig(...)
```

---

### 🧾 Dynamic Description Loading

Descriptions are NOT pre-stored.

Instead:

```
ParsePrepMenuDescTexts(gPrepMenuTable[slot].msg_desc);
```

And on cursor movement:

```
sub_8095C00(...)
```

This reduces:

- Persistent RAM usage
- Static struct duplication
- Redundant buffers

---

### 🧩 Stubbed Functions

The following vanilla functions are intentionally neutralized:

```
SetPrepScreenMenuItem
SetPrepScreenMenuSelectedItem
```

They now:

- Do not store data
- Only increment `max_index`
- Treat `cur_index` as a direct ROM index

---

### 🔄 Scrollbar Behavior

Scrollbar activates when:

```
proc->max_index > 4
```

Positioning logic:

```
if (proc->cur_index < gTopVisibleListIndex)
    gTopVisibleListIndex = proc->cur_index;

if (proc->cur_index >= gTopVisibleListIndex + PREP_MENU_VISIBLE_COUNT - 1)
    gTopVisibleListIndex = proc->cur_index - PREP_MENU_VISIBLE_COUNT + 1;
```

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

---