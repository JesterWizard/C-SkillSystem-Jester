# Fancy Teleport Animation (FE7 Assassin Spawn FX Port)

<p align="center">
  <img src="../Gifs/FE7FancyTeleport.gif" alt="Fancy Teleport Animation" width="600"/>
</p>

---

## Index
- [Introduction](#introduction)
- [How-To-Use](#how-to-use)
- [Macro Usage](#macro-usage)
- [Class Support](#class-support)
- [Customization](#customization)
- [Code Flow](#code-flow)
- [Limitations](#limitations)

---

## Introduction

This feature ports the **FE7 Assassin teleport-style spawn animation** into FE8.

It allows:

- Stylish magical spawn effects
- Class-specific animations
- Event-driven deployment visuals

Instead of units simply appearing on the map, they now **materialize with flair**.

---

## How To Use

Call this macro inside any event:

```
FANCY_TELEPORT_ANIMATION_AT_COORDS(x, y, classId)
```

This will:

1. Spawn the teleport animation  
2. Match it to the unit’s class  
3. Play at the specified coordinates  
4. Pause briefly to allow animation completion  

---

## Macro Usage

```
#define FANCY_TELEPORT_ANIMATION_AT_COORDS(x, y, classId) \
    SVAL(EVT_SLOT_3, x) \
    SVAL(EVT_SLOT_4, y) \
    SVAL(EVT_SLOT_5, classId) \
    ASMC(CallEvent_SpawnAssassinfx) \
    STAL(30)
```

### Parameters

| Parameter | Purpose |
|----------|--------|
| `x` | Map X coordinate |
| `y` | Map Y coordinate |
| `classId` | Class to determine animation |

---

## Class Support

The animation dynamically changes based on class.

Supported classes:

| Class | Animation |
|-------|-----------|
| Assassin | FE7 teleport effect |
| Rogue | Rogue variant |
| Thief | Thief variant |

Handled via:

```
SpawnAnimByClass[]
```

Which maps:

```
CLASS_ASSASSIN
CLASS_ROGUE
CLASS_THIEF
```

to their respective animation configurations.

---

## Customization

Each class uses its own:

- Sprite animation
- Palette
- AP configuration

Example:

```
static const struct EventSpriteAnimConf EventSpriteAnimConf_SpawnAssassin
```

Palette bank:

```
pal_bank = 0xA → Blue
pal_bank = 0xB → Red
```

Changing this allows faction-based effects.

---

## Code Flow

Event → Macro → ASMC → Animation Proc

### Step Breakdown

1. Event sets slots:

```
EVT_SLOT_3 → X  
EVT_SLOT_4 → Y  
EVT_SLOT_5 → Class
```

2. Calls:

```
CallEvent_SpawnAssassinfx
```

3. Converts map → screen coordinates

4. Calls:

```
StartSpawnClassFx
```

5. Launches:

```
ProcScr_EventSpriteAnim
```

---

## Animation Assets

Each class uses:

| Class | Sprite |
|-------|--------|
| Assassin | Img_EventSpriteAnim_SpawnAssassin_Blue |
| Rogue | Img_EventSpriteAnim_SpawnRogue_Blue |
| Thief | Img_EventSpriteAnim_SpawnThief_Blue |

All share:

```
ApConf_EventSpriteAnim_Spawn
```

---

## Limitations

- Only works for configured classes  
- Requires valid classId  
- Animation must complete before unit appears (handled via STAL)   

---