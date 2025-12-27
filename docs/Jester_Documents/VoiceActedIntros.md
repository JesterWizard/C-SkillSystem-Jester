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

``CONFIG_VOICE_ACTED_DIALOGUE``

A lot of guides for music/sound on FEUniverse focus on **MIDI installation**. That makes sense: MIDI files are tiny compared to WAVs, which are relatively large.

The GBA isn’t widely known for advanced audio, but it **can** play voiced audio (see old GBA Video cartridges). For voiced dialogue and short high-quality clips, **WAVs** are a good choice.

This guide focuses specifically on preparing and installing WAVs into a buildfile-based ROM project.

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
- A copy of the [compress_mp3_to_s](../../Data/FE8_Rewritten_Terper/Music/compress_mp3_to_s.bat) ``.bat`` file
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

4) Take the ``.s`` file and grab a template copy of the [Audio_Insert_Event.event](../../Data/FE8_Rewritten_Terper/Music/Audio_Insert_Event.event)
file and change every instance of ``[AUDIO_FILENAME]`` to the file name of your ``.s`` file

5) Reference FEBuilder's audio track list to see which song slots are free if you don't want to override the default.

6) Then in your ``Audio_Insert_Event.event`` file change the first parameter of ``SongTable`` to be the song ID you want to use, e.g. ``0xC0``

7) Rename ``Audio_Insert_Event.event`` to whatever you like and include it in [Music_Installer.event](../../Data/FE8_Rewritten_Terper/Music/Music_Installer.event)
like so ``#include "Prologue/Audio_Insert_Event.event"``

8) Now you call the audio by using the MUSC command, e.g. ``MUSC(0xC0)``

9) Run ``make -j`` to compile your ``.s`` file to a ``.dmp`` and see it inserted into the game.

---

## Limitations and Bugs

- The GBA cannot decode MP3s or modern compressed formats in real time, so WAV (with DPCM compression) is the practical option.  
- WAVs are large even after compression; storage is limited.  
- Better compression solutions (e.g., **8ad**) require replacing the sound engine, which is outside the scope of this guide and significantly more complex.