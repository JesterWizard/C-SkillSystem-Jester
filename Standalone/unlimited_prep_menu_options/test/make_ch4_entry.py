#!/usr/bin/env python3
"""TEST ONLY. Writes test/Ch4Entry.bin: chapter data entry 4 (0x94 bytes) from your own clean FE8U ROM.

Usage (from anywhere):

    python3 test/make_ch4_entry.py /path/to/clean/fe8.gba

The output is next to this script (test/Ch4Entry.bin). ToChapter4.event includes it, and it is
git-ignored, so the ROM data is never checked in. The ROM is only read, never written.

The entry is gChapterDataTable (file offset 0x8B0890, ROM address 0x088B0890) + 4 * 0x94.
"""
import os
import sys

ENTRY_SIZE = 0x94
TABLE_OFFSET = 0x8B0890          # gChapterDataTable, file offset
CHAPTER_ID = 4
HEADER_TITLE = b"FIREEMBLEM2E"   # ROM header title, offset 0xA0
HEADER_CODE = b"BE8E"            # game code, offset 0xAC (FE8 USA)


def main(argv):
    if len(argv) != 2:
        sys.stderr.write(__doc__.split("\n\n", 1)[0] + "\n")
        sys.stderr.write("usage: python3 test/make_ch4_entry.py <clean fe8.gba>\n")
        return 2

    try:
        with open(argv[1], "rb") as f:
            rom = f.read()
    except OSError as err:
        sys.stderr.write("error: cannot read %s: %s\n" % (argv[1], err.strerror))
        return 1

    if rom[0xA0:0xAC] != HEADER_TITLE or rom[0xAC:0xB0] != HEADER_CODE:
        sys.stderr.write("error: not an FE8U (USA) ROM (header title or game code differs)\n")
        return 1

    start = TABLE_OFFSET + CHAPTER_ID * ENTRY_SIZE
    entry = rom[start:start + ENTRY_SIZE]
    if len(entry) != ENTRY_SIZE:
        sys.stderr.write("error: ROM too short for chapter %d's entry\n" % CHAPTER_ID)
        return 1

    out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Ch4Entry.bin")
    with open(out_path, "wb") as f:
        f.write(entry)

    print("wrote %s: %d bytes from file offset 0x%X" % (out_path, len(entry), start))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
