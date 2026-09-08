import os
import sys
from pathlib import Path

# Get the folder where the script itself resides
SCRIPT_DIR = Path(__file__).parent.resolve()

# The dmp folder is inside the script folder
DMP_DIR = SCRIPT_DIR / "dmp"

if not DMP_DIR.exists() or not DMP_DIR.is_dir():
    print("No 'dmp' folder found.")
    exit(1)

# Free Song ID ranges
FREE_RANGES = [
    range(0x26D, 0x2BC),  # 0x26D-0x2BB
    range(0x385, 0x3AC),  # 0x385-0x3AB
    range(0x3D7, 0x3E5),  # 0x3D7-0x3E4
]

def get_free_ids():
    ids = []
    for r in FREE_RANGES:
        ids.extend(list(r))
    return ids

def main():
    folders = [f for f in DMP_DIR.iterdir() if f.is_dir()]
    free_ids = get_free_ids()
    used_ids = {}

    lines = []

    # Header for EA event
    lines.append('#include "EAstdlib.event"')
    lines.append('#include "Extensions/Hack Installation.txt"')
    lines.append('#ifndef SongTableOffset')
    lines.append('#define SongTableOffset  0x224470')
    lines.append('#define SongTable(index,SongPointer,Group) "PUSH; ORG SongTableOffset+(8*index); POIN SongPointer; SHORT Group Group; POP"')
    lines.append('#endif\n')
    lines.append('// ======================================================')
    lines.append('// Shared Sound Data (ONE TIME)')
    lines.append('// ======================================================')
    lines.append('ALIGN 4')
    lines.append('VoiceSound_data:')
    lines.append('    BYTE $BE $7F $BC $00 $BB $05 $BD $00 $E7 $3C $40 $98 $B1 $79 $83 $83\n')

    for folder in folders:
        folder_path = DMP_DIR / folder
        # Look for .s files
        s_files = sorted([f for f in folder_path.iterdir() if f.suffix.lower() == ".s"])
        if not s_files:
            continue

        used_ids[folder.name] = []

        lines.append(f'// ======================================================')
        lines.append(f'// {folder.name.upper()}')
        lines.append(f'// ======================================================')

        for idx, sfile in enumerate(s_files, start=1):
            if not free_ids:
                pct = len(used_ids[folder.name]) / len(s_files) * 100
                print(f"ERROR: Ran out of free SongTable IDs in folder '{folder.name}' after assigning {pct:.2f}% of files.")
                sys.exit(1)
            song_id = free_ids.pop(0)
            used_ids[folder.name].append(song_id)

            wav_label = f"{folder.name}_{idx}_wav"
            map_label = f"{folder.name}_{idx}_map"
            sound_label = f"{folder.name}_{idx}_Sound"

            # Use .dmp in incbin line
            dmp_filename = sfile.stem + ".dmp"

            lines.append(f'ALIGN 4')
            lines.append(f'{wav_label}:')
            lines.append(f'    #incbin "dmp/{folder.name}/{dmp_filename}"\n')

            lines.append(f'ALIGN 4')
            lines.append(f'{map_label}:')
            lines.append(f'    WORD $3C00')
            lines.append(f'    POIN {wav_label}')
            lines.append(f'    WORD $FF00FF\n')

            lines.append(f'ALIGN 4')
            lines.append(f'{sound_label}:')
            lines.append(f'    WORD 1')
            lines.append(f'    POIN {map_label}')
            lines.append(f'    POIN VoiceSound_data\n')

            lines.append(f'SongTable(0x{song_id:X}, {sound_label}, 7)\n')

    # Write output event file
    installer_path = SCRIPT_DIR / "UnitSelectionSFX_Installer.event"
    with installer_path.open("w") as f:
        f.write("\n".join(lines))

    # Print C table
    print("\nReplace the array in BeginActionHook.c with the one below to have the voices play:\n")
    print("static const VoiceIds character_voice_ids[] =")
    print("{")
    print("    {0},")  # Default empty entry
    for folder_name, ids in used_ids.items():
        hex_ids = ", ".join(f"0x{id:X}" for id in ids)
        print(f"    {{ CHARACTER_{folder_name.upper()},    {{{hex_ids}}} }},")
    print("};\n")

if __name__ == "__main__":
    main()