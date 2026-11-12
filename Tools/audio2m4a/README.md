# audio2m4a

Turns an audio file (mp3, wav, ...) into a sequenced FE8 song: note data for the GBA's m4a ("Sappy") engine, played on
instrument samples that are already in the vanilla FE8U ROM. No sample data is added, so a song costs kilobytes, not
megabytes.

| Lord of the Dance (4:47) | Size |
| --- | --- |
| The mp3 | 4,562,546 bytes |
| As a sampled song (13379 Hz, 8 bit, how the voice lines are stored) | about 3,850,000 bytes |
| Sequenced by this tool | 21,088 bytes |

## What comes out

The tool transcribes the recording, it does not copy it. The result is a new arrangement of the same tune on FE8's
instruments: it follows the original's melody, bass, chords, tempo and loudness, and it does not sound like the
recording. Automatic transcription of a full mix makes mistakes (wrong or missing notes, most of all in busy parts), and
the drums are
the hits found in the recording's drum part, played on the nearest piece of vanilla's kit. For a song that matters, open the `.mid` it
writes, fix it by ear, and convert that MIDI with a MIDI tool (mid2agb, Sappy) instead.

Size follows the number of notes, not the length: about 3.5 bytes a note (Lord of the Dance has about 5,900).

## Steps

1. Split the recording into drums, bass and the rest with [Demucs](https://github.com/facebookresearch/demucs)
   (`htdemucs`). Without Demucs (or with `--no-stems`) it falls back to a harmonic / percussive split, which is much
   rougher: the first version of Lord of the Dance was made that way and had four times the drum hits, most of them
   noise.
2. Transcribe the bass and the rest with [basic-pitch](https://github.com/spotify/basic-pitch) (its ONNX model).
3. Track the beat and put every note on a grid of a sixth of a beat. The song's tempo follows the recording beat by beat.
4. Make six tracks:
   - lead: one line tracked through the notes from E4 up (flute);
   - fiddle: the lead again on strings where the recording is loud;
   - strings: the other notes that last a beat or more;
   - harp: the other notes that are shorter, at most four at a time;
   - bass: one line tracked through the bass part;
   - drums: each hit of the drum part, as kick, tom, rim or hi-hat by where its energy is.
5. Write the song with its own small voice bank (`VOICES` in the script: sample addresses in the vanilla ROM, and
   vanilla's drum kit at `0x082228F0`).

The instruments are fixed in `VOICES` and the `tracks` list. Change them there for a song that wants other ones.

## Noise

- The GBA's sound output is 8 bit, and its noise is the same whatever plays. A quiet song hisses more. So the tool
  mixes louder than vanilla's BGM (`--gain`, default 1.5; 1.0 is vanilla's level) and flattens the recording's
  dynamics. Check a new song's recording for clipping before raising the gain.
- A sample played far below its own pitch is noisy. Vanilla's bass sample (`0x08227988`) sounds an octave below its
  key, so the bass track is shifted up an octave (`keysh=12`); vanilla's songs play it on keys 41 to 57.
- Cymbals and snares are the hissiest part of a kit. A hit is only a hi-hat if most of its energy is above 3 kHz.

## Use

```bash
pip install numpy scipy librosa soundfile pretty_midi mir_eval resampy onnxruntime torch torchaudio demucs
```

```bash
pip install --no-deps basic-pitch
```

```bash
python3 Tools/audio2m4a/audio2m4a.py song.mp3 /tmp/song --name MySong
```

It needs `ffmpeg`. Demucs downloads its model from `dl.fbaipublicfiles.com` on the first run (a cloud session's
environment must allow that host). It writes `/tmp/song/MySong.event` and `/tmp/song/MySong.mid`, and prints the size and the length in
frames. The stems and the transcription are kept in `/tmp/song/work`; a second run only arranges. `--only lead,bass`
keeps some tracks, to listen to them alone.

## Adding the song to the game

1. Copy `MySong.event` to `CustomCampaign/Music/Songs/`.
2. In `CustomCampaign/Music/Songs/installer.event`, add `SongTable(0xNN, MySong, 0)` and the `#include`. Take a free id
   from `python3 CustomCampaign/Music/assign_voice_song_ids.py --next`; that script then skips it. A Sound Room song
   needs an id below `0x100`.
3. Add the id and the length in frames to `include/jester_headers/custom-songs.h`.
4. For the Sound Room: add a name at the end of `Contents/Texts/Source/texts/Districts.txt` and a row before the end
   row of `Data/SoundRoom/SoundRoomTable.c`. With `SoundRoom_AlwaysShown` the song is in the list from the start;
   with no function it shows once the game has played it (`StartBgm`).
5. Play it in a chapter like any song (`MUSC 0xNN`, a chapter's music fields).

## Listening without a rebuild

`gbarun` records sound (`wav FILE`, see `Tools/cloud/README.md`). Main menu > Extras > Sound Room, then the song:

```
hold A+B+START+SELECT 10
wait 500
press A
wait 200
press START
wait 200
press UP
wait 20
press A
wait 200
press A
wait 200
```

The Sound Room's grid has four songs a row; song 70 is 17 rows down and one right. Then `wav out.wav`, `press A 4` and
a `wait` of the song's length.
