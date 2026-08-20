#!/usr/bin/env python3
"""Assign unused song-table IDs to chapter voice lines.

Vanilla FE8's table at 0x224470 has 1000 entries (0x000-0x3E7). Unused slots
are scattered. This script reads empty slots from fe8.gba, subtracts IDs
already claimed by other SongTable() inserts, and assigns the rest to
Chapters/*/music/*.event files.

Usage:
  python3 assign_voice_song_ids.py           # assign AUTO, write headers, update events
  python3 assign_voice_song_ids.py --next    # print the next free ID and remaining count
  python3 assign_voice_song_ids.py --list-free
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SONG_TABLE_OFFSET = 0x224470
SONG_TABLE_END = 0x2263B0
DUMMY_SONG = 0x082263B0
ENTRY_SIZE = 8
ENTRY_COUNT = (SONG_TABLE_END - SONG_TABLE_OFFSET) // ENTRY_SIZE

SONGTABLE_RE = re.compile(
    r"SongTable\s*\(\s*(AUTO|0x[0-9A-Fa-f]+|\d+|[A-Za-z_][A-Za-z0-9_]*)",
    re.IGNORECASE,
)
DEFINE_RE = re.compile(
    r"#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(0x[0-9A-Fa-f]+|\d+)"
)
SOUN_RE = re.compile(r"\bSOUN\(\s*(0x[0-9A-Fa-f]+)\s*\)")

SCRIPT_DIR = Path(__file__).resolve().parent
TERPER_DIR = SCRIPT_DIR.parent
REPO_ROOT = TERPER_DIR.parent.parent
CHAPTERS_DIR = TERPER_DIR / "Chapters"
FE8_GBA = REPO_ROOT / "fe8.gba"
EVENT_OUT = SCRIPT_DIR / "voice-songs.event"
HEADER_OUT = REPO_ROOT / "include" / "jester_headers" / "voice-songs.h"

SKIP_OCCUPIED_PARTS = {
    "Audio_Insert_Event.event",
    "voice-songs.event",
    "assign_voice_song_ids.py",
}


def parse_int(token: str) -> int:
    token = token.strip()
    if token.lower().startswith("0x"):
        return int(token, 16)
    return int(token, 10)


def vanilla_empty_ids(rom: bytes) -> list[int]:
    empty = []
    for i in range(ENTRY_COUNT):
        off = SONG_TABLE_OFFSET + i * ENTRY_SIZE
        ptr = int.from_bytes(rom[off : off + 4], "little")
        if ptr in (0, DUMMY_SONG):
            empty.append(i)
    return empty


def is_voice_music_event(path: Path) -> bool:
    try:
        rel = path.relative_to(CHAPTERS_DIR)
    except ValueError:
        return False
    parts = rel.parts
    return (
        len(parts) == 3
        and parts[1] == "music"
        and path.name != "installer.event"
        and path.suffix == ".event"
    )


def is_legacy_terper_music(path: Path) -> bool:
    try:
        rel = path.relative_to(SCRIPT_DIR)
    except ValueError:
        return False
    return rel.parts[0] in {"Prologue"} or rel.parts[0].startswith("Chapter_")


def voice_symbol(path: Path) -> str:
    rel = path.relative_to(CHAPTERS_DIR)
    chapter = rel.parts[0]
    stem = path.stem
    if stem.endswith("_Compressed"):
        stem = stem[: -len("_Compressed")]
    ident = re.sub(r"[^0-9A-Za-z]+", "_", stem).upper().strip("_")
    return f"SONG_VOICE_CH{chapter}_{ident}"


def collect_defines(text: str) -> dict[str, int]:
    return {name: parse_int(value) for name, value in DEFINE_RE.findall(text)}


def songtable_ids_in_text(text: str) -> list[int]:
    defines = collect_defines(text)
    ids = []
    for token in SONGTABLE_RE.findall(text):
        if token.upper() == "AUTO":
            continue
        if token.lower().startswith("0x") or token.isdigit():
            ids.append(parse_int(token))
            continue
        if token in defines:
            ids.append(defines[token])
    return ids


def iter_event_files() -> list[Path]:
    files = []
    for root in (REPO_ROOT / "Kernel", TERPER_DIR):
        if not root.exists():
            continue
        files.extend(root.rglob("*.event"))
    return files


def occupied_ids_from_repo() -> set[int]:
    occupied = set()
    for path in iter_event_files():
        if path.name in SKIP_OCCUPIED_PARTS:
            continue
        if is_voice_music_event(path) or is_legacy_terper_music(path):
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        occupied.update(songtable_ids_in_text(text))
    return occupied


def discover_voice_events() -> list[Path]:
    files = [
        p
        for p in CHAPTERS_DIR.rglob("*.event")
        if is_voice_music_event(p)
    ]
    return sorted(files, key=lambda p: (p.relative_to(CHAPTERS_DIR).as_posix()))


def parse_voice_arg(text: str) -> str | None:
    match = SONGTABLE_RE.search(text)
    return match.group(1) if match else None


def load_existing_header_ids() -> dict[str, int]:
    if not HEADER_OUT.exists():
        return {}
    text = HEADER_OUT.read_text(encoding="utf-8")
    ids = {}
    for name, value in DEFINE_RE.findall(text):
        if name.startswith("SONG_VOICE_"):
            ids[name] = parse_int(value)
    return ids


def assign_ids(
    voice_files: list[Path], occupied: set[int], free_pool: list[int]
) -> dict[Path, tuple[str, int]]:
    existing_names = load_existing_header_ids()
    used = set(occupied)
    assigned: dict[Path, tuple[str, int]] = {}
    pending: list[Path] = []

    for path in voice_files:
        symbol = voice_symbol(path)
        text = path.read_text(encoding="utf-8")
        arg = parse_voice_arg(text)
        if arg is None:
            raise SystemExit(f"No SongTable(...) in {path}")

        song_id = None
        if arg.upper() == "AUTO":
            song_id = existing_names.get(symbol)
        elif arg.lower().startswith("0x") or arg.isdigit():
            song_id = parse_int(arg)
        elif arg.startswith("SONG_VOICE_"):
            song_id = existing_names.get(arg, existing_names.get(symbol))
            if song_id is None:
                raise SystemExit(f"{path} uses {arg} but that name has no ID yet")
        else:
            raise SystemExit(f"{path} has unsupported SongTable index '{arg}'")

        if song_id is None:
            pending.append(path)
            continue
        if song_id in used:
            raise SystemExit(
                f"{path} wants song ID 0x{song_id:X}, but that slot is already taken"
            )
        used.add(song_id)
        assigned[path] = (symbol, song_id)

    pool = [i for i in free_pool if i not in used]
    for path in pending:
        if not pool:
            raise SystemExit(
                f"Ran out of unused song-table IDs while assigning {path}"
            )
        song_id = pool.pop(0)
        used.add(song_id)
        assigned[path] = (voice_symbol(path), song_id)

    by_id: dict[int, Path] = {}
    for path, (_, song_id) in assigned.items():
        if song_id in by_id:
            raise SystemExit(
                f"Duplicate song ID 0x{song_id:X}: {by_id[song_id]} and {path}"
            )
        by_id[song_id] = path
    return assigned


def write_outputs(assigned: dict[Path, tuple[str, int]], remaining: list[int]) -> None:
    rows = sorted(assigned.items(), key=lambda item: item[1][1])
    next_id = remaining[0] if remaining else None
    next_line = f"0x{next_id:X}" if next_id is not None else "none"

    event_lines = [
        "// Auto generated by assign_voice_song_ids.py — do not edit by hand.",
        f"// Next free song ID: {next_line} ({len(remaining)} remaining in vanilla empty slots)",
        "",
    ]
    header_lines = [
        "#pragma once",
        "",
        "// Auto generated by assign_voice_song_ids.py — do not edit by hand.",
        f"// Next free song ID: {next_line} ({len(remaining)} remaining in vanilla empty slots)",
        "",
    ]
    for path, (symbol, song_id) in rows:
        rel = path.relative_to(REPO_ROOT).as_posix()
        hex_id = f"0x{song_id:X}"
        event_lines.append(f"#define {symbol} {hex_id} // {rel}")
        header_lines.append(f"#define {symbol} {hex_id} // {rel}")

    EVENT_OUT.write_text("\n".join(event_lines) + "\n", encoding="utf-8")
    HEADER_OUT.parent.mkdir(parents=True, exist_ok=True)
    HEADER_OUT.write_text("\n".join(header_lines) + "\n", encoding="utf-8")


def rewrite_voice_events(assigned: dict[Path, tuple[str, int]]) -> None:
    for path, (symbol, _) in assigned.items():
        text = path.read_text(encoding="utf-8")
        new_text, count = SONGTABLE_RE.subn(rf"SongTable({symbol}", text, count=1)
        if count != 1:
            raise SystemExit(f"Failed to rewrite SongTable in {path}")
        if new_text != text:
            path.write_text(new_text, encoding="utf-8")


def rewrite_soun_calls(assigned: dict[Path, tuple[str, int]]) -> None:
    id_to_symbol = {song_id: symbol for _, (symbol, song_id) in assigned.items()}

    def repl(match: re.Match[str]) -> str:
        song_id = parse_int(match.group(1))
        symbol = id_to_symbol.get(song_id)
        if symbol is None:
            return match.group(0)
        return f"SOUN({symbol})"

    for path in CHAPTERS_DIR.rglob("*"):
        if path.suffix not in {".c", ".h"}:
            continue
        text = path.read_text(encoding="utf-8")
        new_text = SOUN_RE.sub(repl, text)
        if new_text != text:
            path.write_text(new_text, encoding="utf-8")


def remaining_free(free_pool: list[int], assigned: dict[Path, tuple[str, int]], occupied: set[int]) -> list[int]:
    used = set(occupied)
    used.update(song_id for _, song_id in assigned.values())
    return [i for i in free_pool if i not in used]


def format_ranges(ids: list[int]) -> str:
    if not ids:
        return "(none)"
    ranges = []
    start = prev = ids[0]
    for value in ids[1:]:
        if value == prev + 1:
            prev = value
            continue
        ranges.append((start, prev))
        start = prev = value
    ranges.append((start, prev))
    parts = []
    for a, b in ranges:
        if a == b:
            parts.append(f"0x{a:X}")
        else:
            parts.append(f"0x{a:X}-0x{b:X} ({b - a + 1})")
    return ", ".join(parts)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--next", action="store_true", help="Print the next free song ID")
    parser.add_argument("--list-free", action="store_true", help="List remaining unused IDs")
    args = parser.parse_args()

    if not FE8_GBA.exists():
        raise SystemExit(f"Missing {FE8_GBA} (needed to dump unused song-table slots)")

    rom = FE8_GBA.read_bytes()
    empty = vanilla_empty_ids(rom)
    free_pool = [i for i in empty if i != 0]
    occupied = occupied_ids_from_repo()
    voice_files = discover_voice_events()
    assigned = assign_ids(voice_files, occupied, free_pool)
    remaining = remaining_free(free_pool, assigned, occupied)

    if args.next:
        if not remaining:
            print("No unused vanilla song IDs remain", file=sys.stderr)
            return 1
        print(f"0x{remaining[0]:X}")
        print(f"{len(remaining)} remaining", file=sys.stderr)
        return 0

    if args.list_free:
        print(format_ranges(remaining))
        print(f"{len(remaining)} remaining", file=sys.stderr)
        return 0

    write_outputs(assigned, remaining)
    rewrite_voice_events(assigned)
    rewrite_soun_calls(assigned)
    print(f"Assigned {len(assigned)} voice songs")
    print(f"Wrote {EVENT_OUT.relative_to(REPO_ROOT)}")
    print(f"Wrote {HEADER_OUT.relative_to(REPO_ROOT)}")
    if remaining:
        print(f"Next free ID: 0x{remaining[0]:X} ({len(remaining)} remaining)")
    else:
        print("No unused vanilla song IDs remain")
    return 0


if __name__ == "__main__":
    sys.exit(main())
