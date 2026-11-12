# audio2m4a

Turns an audio file (mp3, wav, ...) into a sequenced FE8 song: note data for the GBA's m4a ("Sappy") engine, played on
instrument samples that are already in the vanilla FE8U ROM. No sample data is added, so a song costs kilobytes, not
megabytes.

| Lord of the Dance (4:47) | Size |
| --- | --- |
| The mp3 | 4,562,546 bytes |
| As a sampled song (13379 Hz, 8 bit, how the voice lines are stored) | about 3,850,000 bytes |
| Sequenced by this tool | 19,648 bytes |

## What comes out

The tool transcribes the recording, it does not copy it. The result is a new arrangement of the same tune on FE8's
instruments: it follows the original's melody, bass, chords, tempo and loudness, and it does not sound like the
recording. Automatic transcription of a full mix makes mistakes (wrong or missing notes, most of all in busy parts), and
the drums are a guess from the recording's kick, snare and cymbal bands. For a song that matters, open the `.mid` it
writes, fix it by ear, and convert that MIDI with a MIDI tool (mid2agb, Sappy) instead.

Size follows the number of notes, not the length: about 3.5 bytes a note. A 2 minute piece with a plain melody comes
out near 5 KB; a dense one near 20 KB.

## Steps

1. Split the audio into harmonic and percussive parts (librosa HPSS).
2. Transcribe the harmonic part with [basic-pitch](https://github.com/spotify/basic-pitch) (its ONNX model).
3. Track the beat and put every note on a grid of a sixth of a beat. The song's tempo follows the recording beat by beat.
4. Make five tracks:
   - lead: one line tracked through the notes from E4 up (flute);
   - bass: one line tracked from D1 to D3 (bass);
   - strings: the other notes that last a beat or more;
   - harp: the other notes that are shorter;
   - drums: kick, snare and closed hi-hat from the percussive part, only where the recording has percussion.
5. Write the song with its own small voice bank (`VOICES` in the script: sample addresses in the vanilla ROM, and
   vanilla's drum kit at `0x082228F0`).

The instruments are fixed in `VOICES` and the `tracks` list. Change them there for a song that wants other ones;
`Tools/audio2m4a` has no table of every vanilla sample.

## Use

```bash
pip install numpy scipy librosa soundfile pretty_midi mir_eval resampy onnxruntime
```

```bash
pip install --no-deps basic-pitch
```

```bash
python3 Tools/audio2m4a/audio2m4a.py song.mp3 /tmp/song --name MySong
```

It needs `ffmpeg`. It writes `/tmp/song/MySong.event` and `/tmp/song/MySong.mid`, and prints the size and the length in
frames. The transcription is kept in `/tmp/song/work`; a second run only arranges.

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
