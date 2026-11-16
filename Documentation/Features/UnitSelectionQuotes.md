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

Runtime flag: ``KernelDesigerConfig::unit_selection_quotes`` (FEBuilder B163). Off skips playback.

This is a rewrite of Leonarth's voice clip selection feature [`FEUniverse Link`](https://feuniverse.us/t/fe8-and-maybe-fe7-leonarths-asm-thingies/2693)

When you select a unit, the BGM will momentarily dip out and a voice clip will play.

The original implementation only allowed for a single voice clip per unit and was a bit messy to edit manually.
This rewrite allows for multiple quotes to be player per unit on a random basis whilst also extending support for additional criteria such as HP.
Furthermore, the creation of the installer is now entirely automated via Python, only requiring that you supply the WAVs in the ``./wav`` folder

---

## 🛠️ Plan

The system is designed as follows:

- Select a unit
- A random number between 0-2 is rolled in ``UnitBeginAction``
- The music ID corresponding to that number for the active unit is checked in the ``character_voice_ids`` array
- The corresponding quote is played (if it exists). Quotes are VC2 stream songs, decoded while they play ([VoiceStream.md](VoiceStream.md))

The 114 quotes take 342 KB: about 3 KB a quote, half what they took as DPCM and a 26th of their source WAVs. Every quote's sizes are in [VoiceStreamSizes.md](VoiceStreamSizes.md).

### Adding a quote

1. Put the quote's WAV in ``Kernel/Wizardry/UnitSelectionSFX/wav/<Character>/``, e.g. ``wav/Eirika/Eirika_Lets_Go.wav``.
   - Any sample rate and bit depth works: ``vc2.py`` resamples to 13379 Hz, trims the silence and normalises.
   - Names ending in ``_Compressed`` are skipped: those are leftovers of the old SoX step.
2. From the root of the repo, run ``make rebuild_unit_sfx`` (or ``python3 Tools/voice/vc2.py gen-unit --keep``):
   - Quotes already installed keep their song IDs. A new WAV gets the next free ID in ``0x26D-0x2BB, 0x385-0x3AB, 0x3D7-0x3E4``.
   - It encodes the new WAVs to ``vc2/<Character>/<name>.vc2`` and rewrites ``UnitSelectionSFX_Installer.event``.
   - It prints the ``character_voice_ids`` rows with each character's first three IDs.
3. Copy the new character's row into ``character_voice_ids`` in [BeginActionHook.c](../../Kernel/Wizardry/BeginActionHook/Source/BeginActionHook.c). A character may need a different constant name than the folder's (``CHARACTER_GLEN_CC``, ``CHARACTER_CAELLACH_CC``).
4. Build. Run ``python3 Tools/voice/vc2.py report`` to update the size table.

To change a quote, replace its WAV, delete its ``.vc2`` and run step 2 again. To redo them all (for example at another ``--ratio``), delete the ``vc2`` folder and run ``vc2.py gen-unit``.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| **wav folder** | [`./wav`](../../Kernel/Wizardry/UnitSelectionSFX/wav/), one folder per character | The quotes' source WAVs |
| **vc2 folder** | [`./vc2`](../../Kernel/Wizardry/UnitSelectionSFX/vc2/) | The encoded quotes the ROM holds (generated) |
| **Encoder** | [`vc2.py`](../../Tools/voice/vc2.py) ``gen-unit`` | Encodes the WAVs and writes the installer, keeping each quote's song ID |
| **Installer event file** | [`UnitSelectionSFX_Installer.event`](../../Kernel/Wizardry/UnitSelectionSFX/UnitSelectionSFX_Installer.event) | Generated: each quote's stream song and song table entry |
| **UnitBeginAction loop** | ``unit_selection_quotes`` inside ``UnitBeginAction`` in [`BeginActionHook.c`](../../Kernel/Wizardry/BeginActionHook/Source/BeginActionHook.c) | Handles the selection of the voiced quote and the control of the BGM volume |
| **Player** | [`VoiceStream.c`](../../Kernel/Wizardry/VoiceStream/Source/VoiceStream.c) | Decodes the quote while it plays |

---

## 📝 TODO

- Add ability to play different tracks based on HP amount or other parameters
- Readd the ability to disable the tracks for the map after playing them once (will need a lot of flags)

---

## 🐛 Limitations & Bugs

Please report issues in the repository’s **Issues** tab.

- I've currently removed the ability to disable the voices after playing them once as I didn't like the implementation
- A quote starts about 40 ms after the unit is selected, and selecting a unit costs about one extra lag frame while its decoding gets ahead ([VoiceStream.md](VoiceStream.md#cost)).
- The BGM volume being lowered currently has a fixed delay, ideally I'd want it to vary based on the length of the track but it's tricky to implement. 45 frames seems to cover most tracks well enough.

---
