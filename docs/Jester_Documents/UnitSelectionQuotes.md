# Unit Selection Quotes

<p align="center">
  <video src="../Videos/UnitSelectionQuotes.mp4" alt="Unit Selection Quotes" width="600" controls/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

``CONFIG_UNIT_SELECTION_QUOTES``

This is a rewrite of Leonarth's voice clip selection feature [`FEUniverse Link`](https://feuniverse.us/t/fe8-and-maybe-fe7-leonarths-asm-thingies/2693)

When you select a unit, the BGM will momentarily dip out and a voice clip will play.

The original implementation only allowed for a single voice clip per unit and was a bit messy to edit manually.
This rewrite allows for multiple quotes to be player per unit on a random basis whilst also extending support for additional criteria such as HP.
Furthermore, the creation of the installer is now entirely automated via Python, only requiring that you supply the necessary ``.s`` files in the ``./dmp`` folder

---

## 🛠️ Plan

The system is designed as follows:

- Select a unit
- A random number between 0-2 is rolled in ``UnitBeginAction``
- The music ID corresponding to that number for the active unit is checked in the ``character_voice_ids`` array
- The corresponding quote is played (if it exists)

With DCPM compressed enabled, I'm averaging about 7KB per ``.dmp`` at a quality of ``13379Mhz``. I have about 100 dmps which altogether use 700KB.

To update the installation script, simply run ``make generate_unit_sfx`` from the root of the repo

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **dmp folder** | All folders in [`./dmp`](../../Kernel/Wizardry/Misc/UnitSelectionSFX/dmp/) | Folder containing all the various .s files and their associated dmps for installation |
| **Python script** | [`generate_sfx_event.py`](../../Kernel/Wizardry/Misc/UnitSelectionSFX/generate_sfx_event.py) | This automates the installation process using music ids in the range ``0x26D-0x2BB, 0x385-0x3AB, 0x3D7-0x3E4``  |
| **Installer event file** | [`UnitSelectionSFX_Installer.event`](../../Kernel/Wizardry//Misc//UnitSelectionSFX/UnitSelectionSFX_Installer.event) | The output of the python file which adds the unit selection dmps to the song table |
| **UnitBeginAction loop** | ``CONFIG_UNIT_SELECTION_QUOTES`` inside ``UnitBeginAction`` in [`BeginActionHook.c`](../../Kernel/Wizardry/Common/BeginActionHook/Source/BeginActionHook.c) | Handles the selection of the voiced quote and the control of the BGM volume |

---

## 📝 TODO

- Add ability to play different tracks based on HP amount or other parameters
- Readd the ability to disable the tracks for the map after playing them once (will need a lot of flags)

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

- I've currently removed the ability to disable the voices after playing them once as I didn't like the implementation
- The BGM volume being lowered currently has a fixed delay, ideally I'd want it to vary based on the length of the track but it's tricky to implement. 45 frames seems to cover most tracks well enough.

---
