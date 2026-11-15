# Voice Acted Intros - How to Install WAVs into Your Game

> [!Important]  
> Everything here runs on Windows (WSL), Linux or macOS. You need Python 3 with numpy. No EXEs are required.

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Tools](#tools)
- [How to Modify](#how-to-modify)
- [Changing or Re-encoding Lines](#changing-or-re-encoding-lines)
- [Limitations and Bugs](#limitations-and-bugs)

---

## Introduction

``Designer Config - voice_acted_dialogue``

A lot of guides for music/sound on FEUniverse focus on **MIDI installation**. That makes sense: MIDI files are tiny compared to WAVs, which are relatively large.

The GBA isn't widely known for advanced audio, but it **can** play voiced audio (see old GBA Video cartridges). This guide is about voiced dialogue on the world map.

The voice lines are stored as **VC2**, a transform codec made for this buildfile ([VoiceStream.md](VoiceStream.md)). The game decodes a line while it plays. Sizes, at the GBA's standard 13379 Hz:

| Format | Per minute | Against VC2 |
|---|---|---|
| 8-bit PCM (an uncompressed GBA sample) | about 784 KB | about 4x the size |
| DPCM (what the lines used before) | about 405 KB | about 2x the size |
| **VC2** | **about 205 KB** | |

Every line's sizes are in [VoiceStreamSizes.md](VoiceStreamSizes.md).

---

## Plan

This guide explains how to:

- Turn an mp3 (or anything else) into a WAV
- Put the WAV in the chapter's `music/wav/` folder
- Add the line's event from the template and give it a song ID
- Encode it to VC2 with `Tools/voice/vc2.py`
- Call the audio in game

---

## Tools

You will need:

- **Python 3 with numpy** (`pip install numpy`): runs [`Tools/voice/vc2.py`](../../Tools/voice/vc2.py), which does the resampling, trimming, normalising and encoding.
- **Something that writes WAV files**, if your audio is in another format: SoX, ffmpeg or Audacity.
- A good ear for judging audio quality.

`vc2.py` takes a WAV at any sample rate (8-, 16-, 24- or 32-bit, mono or stereo). It converts it to 13379 Hz mono, trims leading and trailing silence, and normalises the volume, as the old SoX step did. There is no need to prepare the WAV.

Links:

- SoX - https://sourceforge.net/projects/sox/files/sox/14.4.2/
- ffmpeg - https://ffmpeg.org/download.html

---

## How to Modify

1. **Get the line as a WAV.** From an mp3, for example:

   ```bash
   ffmpeg -i Line.mp3 Chapter_01_Line_0006.wav
   ```

   or `sox Line.mp3 Chapter_01_Line_0006.wav`. Any rate and bit depth works.

2. **Put the WAV in the chapter's `music/wav/` folder**, e.g. `CustomCampaign/Chapters/01/music/wav/Chapter_01_Line_0006.wav`. Create the folder if the chapter has none. The file name (without `.wav`) is the line's name from here on.

3. **Make the line's event.** Copy the template [`Audio_Insert_Event.event`](../../CustomCampaign/Music/Audio_Insert_Event.event) into the chapter's `music/` folder as `<name>.event` (e.g. `music/Chapter_01_Line_0006.event`). Change every ``[AUDIO_FILENAME]`` to the line's name. Leave ``SongTable(AUTO, ...)`` alone: do not pick a raw ID in FEBuilder.

4. **Include the event** from the chapter's [music/installer.event](../../CustomCampaign/Chapters/00/music/installer.event), like so: ``#include "Chapter_01_Line_0006.event"``. [Music_Installer.event](../../CustomCampaign/Music/Music_Installer.event) already includes each chapter's music installer. A new chapter needs a line there too.

5. **Give it a song ID.** Run ``make assign_voice_songs`` (or ``python3 CustomCampaign/Music/assign_voice_song_ids.py``). It needs `fe8.gba` at the repo root.
   - It dumps unused vanilla table slots from ``fe8.gba``, skips IDs already used by other hacks, writes a named ID into the event, and updates [voice-songs.h](../../include/jester_headers/voice-songs.h).
   - The name comes from the event's file name, e.g. ``SONG_VOICE_CH01_CHAPTER_01_LINE_0006``.
   - Use ``--next`` / ``--list-free`` to inspect the pool without rewriting files.

6. **Encode it:**

   ```bash
   python3 Tools/voice/vc2.py gen-wm --keep
   ```

   It encodes every line that has no `music/vc2/<name>.vc2` yet, and turns each line's event into the stream song that plays it. `--keep` leaves the lines that are already encoded alone. It prints each line's size and its share of the old DPCM size (about 50%).

7. **Call the audio** with the generated name. In world map text, use ``WM_VOICE_LINE(SONG_VOICE_CH01_CHAPTER_01_LINE_0006)``. That plays it and waits for the text (`CustomCampaign/Chapters/_shared/worldmap-macros.h`). Elsewhere, use ``SOUN(...)``.

8. **Build** (``make``, or ``Tools/cloud/build.sh`` in a cloud session) and listen in game.

To hear a line as the game will play it without building, decode it:

```bash
python3 Tools/voice/vc2.py decode CustomCampaign/Chapters/01/music/vc2/Chapter_01_Line_0006.vc2 check.wav
```

---

## Changing or Re-encoding Lines

- **New take of a line:** replace its WAV in `music/wav/`, delete its `music/vc2/<name>.vc2`, and run `vc2.py gen-wm --keep` again.
- **Smaller or better:** `--ratio` sets the size as a share of DPCM. The default is 0.5; lower is smaller and rougher. To redo every line, run `vc2.py gen-wm --ratio 0.45` without `--keep`.
- **Lines from before VC2** (chapters 00-09) only exist as DPCM files in `music/audio/<name>.dmp`. Each event's `// Source:` line names its source, and `gen-wm` encodes from that. A WAV with the line's name in `music/wav/` replaces the DPCM source the next time it encodes.
- **Size table:** after changing lines, run `python3 Tools/voice/vc2.py report` to update [VoiceStreamSizes.md](VoiceStreamSizes.md).

---

## Limitations and Bugs

- Decoding takes about a third of the CPU while a line plays. Scenes that already lag may lag a little more while a voice plays (the sound keeps going: it gets decoding time first). See [VoiceStream.md](VoiceStream.md#cost).
- A line starts about 40 ms after it is called, so that the decoder can get ahead of the mixer.
- One voice line plays at a time. Starting another stops the first, as before.
- Vanilla `gSongTable` is 1000 entries (`0x000`-`0x3E7`), not a clean `0x400` block. Unused slots are scattered; `assign_voice_song_ids.py` walks that list and skips IDs already taken by other hacks (unit-select quotes, dragon vein SFX, etc.). Expanding past `0x3E7` would overwrite sample data unless the table is relocated.
