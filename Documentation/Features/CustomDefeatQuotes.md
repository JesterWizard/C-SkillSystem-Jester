# Dual-Character Defeat Quotes

<p align="center">
  <img src="https://media.giphy.com/media/3o7aD2saalBwwftBIY/giphy.gif" />
</p>

---

## Overview

This hack expands the vanilla defeat quote system to allow:

- **Defeat quotes based on BOTH combat participants**
- Multiple conditional boss quotes
- Chapter-specific defeat dialogue
- Route-aware behavior
- Optional fallback to vanilla logic

Think of it as **battle quotes, but for death scenes**.

---

## Core Feature

Vanilla defeat quotes only check:

➡️ Who died

This system checks:

➡️ Who died  
➡️ Who killed them  

Allowing for character-specific kill reactions.

Example:

| Boss | Killer | Result |
|------|--------|--------|
| Oneill | Eirika | Unique quote |
| Oneill | Seth | Different quote |
| Oneill | Anyone else | Default quote |

---

## Defeat Quote Table

``const struct DefeatTalkEntNew gNewDefeatTalkList[]``

Each entry supports:

| Field | Purpose |
|------|--------|
| `pidA` | Defeated unit |
| `pidB` | Killer (or wildcard) |
| `route` | Route restriction |
| `chapter` | Chapter restriction |
| `flag` | Event flag set on death |
| `msg` | Quote text |

---

## Wildcard Support

``.pidB = 0xFFFF``

The quote will trigger regardless of killer.

This preserves compatibility with traditional boss quotes.

---

## Lookup Logic

``GetDefeatTalkEntry_NEW()``

Checks:

1. Chapter match  
2. Flag unused  
3. Defeated unit match  
4. Killer match (if specified)

A match occurs when:

```
Actor == pidB
OR
Target == pidB
```

Meaning either side of battle can trigger the quote.

---

## Hooked Engine Functions

| Hook | Purpose |
|------|--------|
| `CheckBattleDefeatTalk` | Determines if quote should play |
| `DisplayDefeatTalkForPid` | Displays quote |

These now branch between:

- Vanilla system
- New dual-character system

Based on:

``gpKernelDesignerConfig->custom_defeat_quotes``

---

## Music Handling

Special music behavior remains supported.

Examples:

| Condition | Music |
|-----------|-------|
| Player death | Sad theme |
| Game Over | Game Over theme |
| Boss defeat | Default handling |

---

## Quote Display

Supports:

| Type | Behavior |
|------|---------|
| Text (`msg`) | Standard defeat quote |
| Event (`event`) | Runs custom event |
| None | Only sets defeat flag |

---

## Example Entry

```
{
.pidA = CHARACTER_ONEILL,
.pidB = CHARACTER_EIRIKA,
.route = CHAPTER_MODE_ANY,
.chapter = PROLOGUE,
.flag = EVFLAG_DEFEAT_BOSS,
.msg = MSG_DEFEAT_QUOTE_EIRIKA_ONEILL,
}
```

Triggers ONLY if Eirika defeats Oneill

---

## Limitations

- One quote per match condition

---

## Result

Enables:

✔️ Rival-specific defeat dialogue  
✔️ Story-reactive boss deaths  
✔️ Character-driven storytelling  

Without breaking vanilla defeat logic.

---