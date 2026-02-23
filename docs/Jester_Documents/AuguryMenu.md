# Augury

<p align="center">
  <img src="../Gifs/Augury_Menu.gif" alt="Augury" width="600"/>
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

`gpKernelDesignerConfig->prep_menu_augury`

This feature adds an **Augury** option to the prep menu.

The aim is to ensure:

- Chapter-specific hints can be displayed to the player
- Each chapter can define its own text entry
- Optional custom background music per chapter
- Random music fallback when no song is specified
- Clean integration with the existing prep screen system
- Automatic restoration of the prep screen music after exit

The Augury system functions as a lightweight, chapter-indexed talk event.

Each chapter entry consists of:

- A body text ID
- A song ID (optional)

---

## 🛠️ How To Use

- Add entries inside the ``gAuguryTable``:

  ```
  { bodyTextId, songId }
  ```

- The table is indexed using:

  ```
  gPlaySt.chapterIndex
  ```

- For each chapter:
  - Set `bodyTextId` to a valid `MSG_*` entry
  - Set `songId` to:
    - A valid BGM ID  
    - Or `0` to use a random track

Example:

```
{ MSG_AUGURY_CHAPTER_04, SONG_LAUGHTER }
```

If `songId == 0`, the system automatically selects a random BGM:

```
StartBgm(NextRN_N(0x46), 0);
```

The screen is launched from prep using:

```
StartAuguryScreen_FromPrep
```

---

## 🗂️ Code Locations

| Feature | Location | Description |
|----------|----------|-------------|
| **Augury Table** | `gAuguryTable` | Stores chapter-indexed hint text and music |
| **Screen Initialization** | `PrepInitGfx_AUGURY` | Clears backgrounds and prepares mural layer |
| **Event Caller** | `CallAuguryEvent` | Starts talk event and handles BGM selection |
| **Talk Wait Handler** | `WaitForBaseConversation` | Waits until talk event finishes |
| **Prep Exit Callback** | `PrepItemList_OnEnd_AUGURY` | Restores prep music and UI state |
| **Proc Definition** | `ProcScr_PrepItemListScreen_AUGURY` | Controls full Augury screen lifecycle |
| **Prep Entry Hook** | `StartAuguryScreen_FromPrep` | Starts Augury from prep menu |

---

## 🧠 System Overview

### 📜 Augury Data Structure

Each entry in:

```
gAuguryTable[]
```

Contains:

```
{
    bodyTextId,
    songId
}
```

Where:

- `bodyTextId` → The hint text displayed in the talk window
- `songId` → Background music override (optional)

The table is indexed directly by:

```
gPlaySt.chapterIndex
```

This ensures:

- Each chapter automatically loads its corresponding hint
- No additional condition checking is required
- Simple and clean scaling across chapters

---

### 🎵 Music Handling

Inside `CallAuguryEvent`:

- If `songId == 0`
  - A random song is selected
- Otherwise
  - The specified BGM plays

When exiting the Augury screen:

```
StartBgm(SONG_COMBAT_PREPARATION, 0);
```

This guarantees the prep screen music is restored.

---

### 🗨️ Talk System Integration

The hint is displayed using:

```
StartTalkExt(...)
```

Additional setup:

- `SetInitTalkTextFont()`
- `SetTalkPrintColor(1)`
- `ApplyPalette(Pal_TalkBubble, 3)`

The proc waits until:

```
IsTalkActive()
```

returns false before fading out.

---

### 🔄 Proc Flow

```
ProcScr_PrepItemListScreen_AUGURY
```

Flow:

1. Initialize graphics
2. Fade in
3. Call Augury talk event
4. Wait for talk to complete
5. Fade out
6. Restore prep state

---

## 📝 TODO

- Add multi-page auguries per chapter
- Allow conditional hints (e.g. based on flags)
- Support portrait display during augury
- Add preview icon in prep menu
- Add locked/hidden augury support

---

## 🐛 Limitations & Bugs

- Only one hint entry per chapter.
- No built-in conditional branching.
- No scrolling support for long text.
- Song selection limited to one override per chapter.

Please report issues in the repository’s **Issues** tab.

---