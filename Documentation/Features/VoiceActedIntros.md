# Voice Acted Intros - How to Install WAVs into Your Game

> [!Important]  
> This guide will assume you're using Windows or WSL, as some key EXEs are required for this process. I'm unsure if equivalent Linux versions exist.

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Tools](#tools)
- [How to Modify](#how-to-modify)
- [Limitations and Bugs](#limitations-and-bugs)

---

## Introduction

``Designer Config - voice_acted_dialogue``

A lot of guides for music/sound on FEUniverse focus on **MIDI installation**. That makes sense: MIDI files are tiny compared to WAVs, which are relatively large.

The GBA isn’t widely known for advanced audio, but it **can** play voiced audio (see old GBA Video cartridges). For voiced dialogue and short high-quality clips, **WAVs** are a good choice.

This guide focuses specifically on preparing and installing WAVs into a buildfile-based ROM project. Campaign-local steps (including auto song IDs) are also in [Data/CustomCampaign/README.md](../../Data/CustomCampaign/README.md).

As an example, following the standard audio output of the GBA (13379Hz) audio will eat up around ~430KB/minute. Less if you reduce the quality between 8000Hz and 11000Hz.

---

## Plan

This guide explains how to:

- Convert mp3 files to WAV
- Compress them with SoX
- Convert them to insertable .s files
- Insert them into an installer event file as DMPs which will be generated during compilation
- Call the audio in game

---

## Tools

You will need:

- **SoX** - For mp3 conversion and compression
- **WAV2AGB** — converts a `.WAV` file into an insertable `.s` file and applies DPCM compression (vital)
- A copy of the [compress_mp3_to_s](../../Data/CustomCampaign/Music/compress_mp3_to_s.bat) ``.bat`` file
  - The filepaths of the EXEs may need to be adjusted depending on where you have them installed.
  - Also be sure to move thebat file to your C drive to be able to run it if you're using WSL
- A good ear for judging audio quality

Links:

- Sox (Source) - https://github.com/chirlu/sox
- Sox (EXE) - https://sourceforge.net/projects/sox/files/sox/14.4.2/sox-14.4.2-win32.exe/download
- WAV2AGB (EXE) - https://github.com/ipatix/wav2agb/releases/tag/v1.0.0 (Download wav2agb-windows-clang64.zip)

---

## How to Modify

1. **Grab your MP3** from whatever source you like.

2. Drop it into a folder with a copy of the ``compress_mp3_to_s.bat`` file

3. Double click the bat file to run it.
  - It will create a WAV file, compress it, produce a .s file and then delete the WAV file
  - Inside the bat file you will be able to see the settings used. I only recommend changing the audio quality, various presets are in the comments

4) Take the ``.s`` file and grab a template copy of the [Audio_Insert_Event.event](../../Data/CustomCampaign/Music/Audio_Insert_Event.event)
file and change every instance of ``[AUDIO_FILENAME]`` to the file name of your ``.s`` file

5) Leave ``SongTable(AUTO, ...)`` in the insert event. Do not pick a raw ID in FEBuilder.

6) Rename ``Audio_Insert_Event.event`` to whatever you like, put it in that chapter's ``music/`` folder, and add it to [music/installer.event](../../Data/CustomCampaign/Chapters/00/music/installer.event)
like so ``#include "Line_0001_Compressed.event"``. [Music_Installer.event](../../Data/CustomCampaign/Music/Music_Installer.event) already includes each chapter's music installer.

7) Run ``make assign_voice_songs`` (or ``python3 Data/CustomCampaign/Music/assign_voice_song_ids.py``). That dumps unused vanilla table slots from ``fe8.gba``, skips IDs already used by other hacks, writes a named ID into the event, and updates [voice-songs.h](../../include/jester_headers/voice-songs.h). Use ``--next`` / ``--list-free`` to inspect the pool without rewriting files.

8) Call the audio with the generated name, e.g. ``SOUN(SONG_VOICE_CH01_LINE_0001)`` or ``MUSC(SONG_VOICE_CH01_LINE_0001)``.

9) Run ``make -j`` to compile your ``.s`` file to a ``.dmp`` and see it inserted into the game.

---

## Limitations and Bugs

- The GBA cannot decode MP3s or modern compressed formats in real time, so WAV (with DPCM compression) is the practical option.  
- WAVs are large even after compression; storage is limited.  
- Better compression solutions (e.g., **8ad**) require replacing the sound engine, which is outside the scope of this guide and significantly more complex.
- Vanilla `gSongTable` is 1000 entries (`0x000`-`0x3E7`), not a clean `0x400` block. Unused slots are scattered; `assign_voice_song_ids.py` walks that list and skips IDs already taken by other hacks (unit-select quotes, dragon vein SFX, etc.). Expanding past `0x3E7` would overwrite sample data unless the table is relocated.