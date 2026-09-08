@echo off
setlocal

REM --- Tool Paths ---
set SOX="C:\Program Files (x86)\sox-14-4-2\sox.exe"
set WAV2AGB="C:\Users\Owner\Downloads\WAV2EA\wav2agb.exe"

REM --- Process all MP3s into WAVS, compress then and process to ASM files ---
for %%F in (*.mp3) do (
    echo Processing: %%F

    REM Step 1: SoX preprocessing
    %SOX% "%%F" -r 13379 -b 8 -c 1 "%%~nF_Compressed.wav" silence 1 0.1 1% -1 0.1 1% vol 2.0 gain -n

    REM Step 2: DPCM compression via wav2agb
    %WAV2AGB% -c -l 3 "%%~nF_Compressed.wav"

    REM Step 3: Delete the WAV file now as we already have the .s file that we'll be converting to DMP
    del "%%~nF_Compressed.wav"
)

echo.
echo All WAVs processed to 13379 Hz, 8-bit mono with silence removal and DPCM applied!
echo Now assemble the .s file to see the REAL compression in binary form.
pause

REM Audio formats
REM 8000Hz, 11025Hz, 13379Hz (GBA default), 16000Hz, 22050Hz, 24000Hz, 42048Hz (highest quality)