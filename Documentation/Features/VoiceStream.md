# Voice Stream (VC2)

Voice lines coded with a transform codec at about half the size of the m4a mixer's DPCM, decoded while they play.
It covers the world map narration and the unit selection quotes.

Every voice line in the ROM is VC2; the DPCM versions are gone. Adding lines:
[VoiceActedIntros.md](VoiceActedIntros.md) (world map) and [UnitSelectionQuotes.md](UnitSelectionQuotes.md) (unit
selection).

## The codec

`Tools/voice/vc2.py` is the encoder, a reference decoder and the event generator. It needs Python 3 and numpy.

- 13379 Hz, MDCT with 128-sample frames and a sine window, 14 bands.
- Each band has a 2 dB scale factor. A band gets 3 to 63 quantiser levels from its scale factor and a per-line
  offset. Coefficients are Rice-coded. Bands with no levels and zero coefficients are noise-filled, so there are no
  "underwater" holes.
- The per-line offset is searched so the line is about `--ratio` (default 0.5) of its DPCM size.

Commands:

```bash
python3 Tools/voice/vc2.py encode IN.wav OUT.vc2
```

```bash
python3 Tools/voice/vc2.py decode IN.vc2 OUT.wav
```

```bash
python3 Tools/voice/vc2.py gen-unit
```

```bash
python3 Tools/voice/vc2.py gen-wm
```

```bash
python3 Tools/voice/vc2.py report
```

- `encode` takes any WAV: it resamples to 13379 Hz, trims silence and normalises, like the old SoX step.
- `decode` writes what the game plays, to within about 1 LSB of the game's 8-bit output.
- `gen-unit` encodes `Kernel/Wizardry/UnitSelectionSFX/wav/<Character>/*.wav` to `vc2/` and writes
  `UnitSelectionSFX_Installer.event`. Quotes it already has keep their song IDs; new WAVs get free ones.
- `gen-wm` turns each line event a chapter's `music/installer.event` includes into the line's stream song, encoded from
  `music/wav/<name>.wav`. Lines from before VC2 only exist as DPCM (`music/audio/<name>.dmp`, named by the event's
  `// Source:` line), so those are transcoded.
- `report` writes [VoiceStreamSizes.md](VoiceStreamSizes.md): every line's size as WAV, 8-bit PCM, DPCM and VC2.
- `--keep` (on `gen-unit` and `gen-wm`) skips lines whose `.vc2` already exists. `--ratio` sets the size as a share of
  DPCM (default 0.5).

### How the songs work

A stream song is an ordinary one-track song:

- Its instrument, `VoiceStream_Tone`, plays `gVoiceStream.wave`. That is a looping 8-bit wave whose data is a ring
  buffer.
- Its track, `VoiceStream_Track`, holds a C3 note at velocity 64, the same as the DPCM songs.
- After the track pointer it carries the magic `VSC2` and the VC2 data.

`gen-unit` and `gen-wm` write these headers and their song table entries.

## Playback

`Kernel/Wizardry/VoiceStream/Source/VoiceStream.c`:

1. `m4aSongNumStart` is replaced (vanilla `0x080D01FC`). A song whose instrument is `VoiceStream_Tone` sets up the
   decoder and starts `ProcScr_VoiceStream` on tree 3. Every other song starts as before.
2. Each main-loop frame the proc does three things:
   - It reads where the mixer is in the ring (the channel's read pointer, `CHN_POSITION_ABS` +0x28).
   - It decodes 128-sample frames to stay up to 1024 samples (77 ms) ahead.
   - It stops the song once the mixer is past the end of the line.

   A frame decodes at most what the mixer read since the last one plus one frame (3, or 4 when the lead is short). So
   a scene running at 30 fps gets twice the decoding.
3. The line starts 512 samples (38 ms) ahead of the mixer. Until then the mixer plays the zeroed ring, which builds up
   a lead through the lag frames scenes have when a line starts.
4. If the mixer catches up anyway (`gVoiceStream.underruns` counts it), the decoder carries on 512 samples ahead of it.
   Only the stale ring slots replay, and the line is not cut.

### The IWRAM overlay

The decoder is split in two:

- The bit stream parser runs in Thumb from ROM.
- The transform (DCT-IV through a 64-point radix-4 FFT, window, overlap-add) is ARM:
  `VoiceSynthArm.s` (988 bytes).

IWRAM has no free room for the ARM part. These looked free but are used:

| Range | Looked free because | Actually |
|---|---|---|
| `0x03002C60`-`0x03003060` | zero in every dump | the HQ mixer's mix buffer (`hq_buffer_ptr`). It cleans it after each mix. |
| `0x030069D8`-`0x03006CB0` | zero in every dump | tracks 8-16 of the BGM2 player (`gMPlayTable`: 16 tracks from `0x030067B0`). Used by any BGM with 8 or more tracks. |
| `0x03002B8C`-`0x03002C60` | newlib stdio | about 200 bytes, too small |

So each decode burst borrows `0x030049B0`-`0x03004E50` (1184 bytes):

- That range holds `gMovMapFillStPool1`, `gWorkingTerrainMoveCosts`, `gStatGainSimUnit` and `gMovMapFillStPool2`.
- The two fill pools are scratch. They hold data only while a flood fill runs, and the bursts run between proc calls
  in the main loop, never inside one. No interrupt handler touches them.
- The 160 bytes in the middle are saved to `gVoiceStream.save` and put back after the burst.
- The code is copied from ROM each burst, about 1 KB.

Tested: after a line, the two variables match a DPCM run byte for byte, and the movement range is drawn right.

The battle animation sheet decoder (`Kernel/Wizardry/BanimCodec`, [CustomBattleAnimations.md](CustomBattleAnimations.md#compression))
borrows `gMovMapFillStPool2` (0x03004C50-0x03004E50) the same way. Each copies its code in before it runs, so
neither depends on the other's copy.

`VoiceSynthArm.s` is generated. Edit `Tools/voice/vsynth.c` and run `Tools/voice/gen_synth_asm.sh`. The script checks
that the code calls nothing outside itself.

The decoder's tables come from `vc2.py gen-tables`: `VoiceStreamTables.h` and `Tools/voice/vsynth_tables.h`.

Lyn note: GCC turned the copy loop into "source minus `0x030049B0`" off one register. That relocation came out of
lyn and EA `0x08000000` short. An empty `asm` on the source pointer stops it.

### Memory

- `gVoiceStream` (3652 bytes) is in the demo pool (`_kernel_malloc_demo`, `0x0203F1A0`). It holds the ring (2304
  samples), the decoder's state, the IMDCT buffer and the save area.
- Nothing else writes the demo pool:
  - no code points into `0x0203F150`-`0x02040000`;
  - newlib's heap (`_sbrk` from `end`, `0x0203EFB8`) is reached only by float formatting, which nothing uses;
  - the event palette backup at `0x0203EFBC` stops at `0x0203F07C`;
  - a playthrough with a marker pattern there left it untouched.
- Stack: the FFT's 512 bytes are on the stack. While a quote plays, the lowest SP measured was `0x0300793C` (DPCM run:
  `0x03007B54`). That leaves 564 bytes above the mixer's code.

## Cost

Measured in the emulator, on the prologue's world map narration:

- About 53k cycles per 128-sample frame: 26k for the parser in ROM, 26k for the transform in the IWRAM overlay,
  plus the copy.
- About 1.75 frames are decoded per game frame, which is 33% of the CPU while a line plays.
- Unit selection: one lag frame in the first 20 frames after the press (16 main-loop frames against 17 for DPCM),
  then none.

## Size

| Set | Lines | DPCM | VC2 | Saved |
|---|---|---|---|---|
| unit selection | 111 | 652 KB | 336 KB | 48.5% |
| world map | 75 | 2282 KB | 1145 KB | 49.8% |
| **all** | 186 | 2934 KB | 1481 KB | 49.5% |

The ROM is 1.50 MB smaller (24.45 MB with DPCM, 22.95 MB with VC2). The unit selection quotes' source WAVs total
8.5 MB, so the ROM keeps them in 3.8% of that. Every line: [VoiceStreamSizes.md](VoiceStreamSizes.md).

## Testing

`gbarun` (`Tools/cloud/README.md`) records sound with `audio FILE` / `audio end`, and counts cycles in an address range
with `cyc`. The checks below were run by hand:

- **Ring:** the ring's samples match `vc2.py decode` (93-95% exactly, the rest by 1 or 2 LSB).
- **Sound:** the recorded voice minus a run with the stream track's volume set to 0 matches the reference at 21-23 dB
  for world map lines 1-7. The DPCM line against its own data scores the same; that is the 8-bit output's limit, not
  the codec's. The gain matches the DPCM lines (0.163).
- **Underruns:** none during the prologue narration and the unit selections.
- **Stress:** seven quick select/cancel cycles, then a move. The range, the move and the menu were all right.
- **Smoke tests:** `smoketest.py`, 207 checks, all passed.
