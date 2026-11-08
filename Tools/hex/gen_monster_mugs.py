#!/usr/bin/env python3
"""Generate the 12 monster minimugs of the Hex screen (Data/Hex/HexMonsterArt.c) from the FE8 monster class cards.

Run from anywhere:

    python3 Tools/hex/gen_monster_mugs.py [--rom fe8.gba] [--out Data/Hex/HexMonsterArt.c]
                                          [--preview PATH.png] [--check]

Each monster has a class card (struct FaceData in the portrait table at ROM 0x8ACBC4, 0x1C bytes per entry): the palette
(16 GBA colours, offset 0x08) and the imgCard (LZ77 type 0x10, offset 0x10). The card is 90 4bpp 8x8 tiles, 10 per row, so
80x72 px. The mug is a square crop of the card, resized to 32x32 with Image.BOX and mapped to the nearest palette entry.

Output per monster:

- sChibi_<Name>[]: the 32x32 4bpp image (16 tiles of 8x8, row-major, 4 per row, left pixel = low nibble), LZ77 type 0x10
  compressed with Tools/EventAssembler/Tools/compress. Palette index 0 is transparent (the card background).
- sPal_<Name>[16]: the card's palette.

gHexMonsterArt[] lists them in the order of MONSTERS (HEX_MONSTER_COUNT rows, declared in include/kernel/hex.h).

--preview writes a PNG: the 12 mugs at 4x scale in a row with their names, transparent pixels on a mid-blue background.
--check decompresses each generated stream (read back from the written .c file) and compares it with the raw tiles.
"""

import argparse
import os
import re
import struct
import subprocess
import sys
import tempfile

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DEFAULT_ROM = os.path.join(ROOT, "fe8.gba")
DEFAULT_OUT = os.path.join(ROOT, "Data", "Hex", "HexMonsterArt.c")
COMPRESS = os.path.join(ROOT, "Tools", "EventAssembler", "Tools", "compress")

ROM_BASE = 0x08000000
FACE_TABLE = 0x8ACBC4	# struct FaceData[], one entry per portrait id
FACE_SIZE = 0x1C
FACE_OFF_PAL = 0x08	# u16 colours[16]
FACE_OFF_CARD = 0x10	# imgCard, LZ77 type 0x10

CARD_W, CARD_H = 80, 72
CARD_TILES_PER_ROW = 10
CARD_TILE_COUNT = 90
MUG = 32

# (name, portrait id, crop x0, crop y0, crop size) on the 80x72 card
MONSTERS = [
	("Revenant", 0x9A, 16, 2, 32),
	("Bonewalker", 0x9B, 20, 4, 40),
	("Mogall", 0xA5, 16, 8, 48),
	("Bael", 0x9E, 16, 8, 48),
	("Gwyllgi", 0xA0, 46, 20, 32),
	("Wight", 0x9C, 20, 4, 40),
	("Tarvos", 0xA2, 22, 4, 32),
	("Arch Mogall", 0xA4, 16, 8, 48),
	("Cyclops", 0x9F, 20, 0, 32),
	("Gargoyle", 0xA8, 24, 4, 40),
	("Deathgoyle", 0xA9, 8, 22, 32),
	("Draco Zombie", 0xAA, 28, 6, 32),
]

PREVIEW_BG = (64, 110, 170)
PREVIEW_SCALE = 4


def ident(name):
	return name.replace(" ", "")


def rom_ptr(rom, off):
	value = struct.unpack_from("<I", rom, off)[0]
	if not (ROM_BASE <= value < ROM_BASE + len(rom)):
		raise ValueError("pointer 0x%08X at 0x%X is not a ROM address" % (value, off))
	return value - ROM_BASE


def lz77_decode(data):
	"""Decode GBA LZ77 type 0x10 (the format written by the compress tool)."""
	if data[0] != 0x10:
		raise ValueError("not LZ77 type 0x10")
	size = data[1] | (data[2] << 8) | (data[3] << 16)
	out = bytearray()
	pos = 4
	while len(out) < size:
		flags = data[pos]
		pos += 1
		for bit in range(8):
			if len(out) >= size:
				break
			if flags & (0x80 >> bit):
				b0 = data[pos]
				b1 = data[pos + 1]
				pos += 2
				length = (b0 >> 4) + 3
				disp = (((b0 & 0x0F) << 8) | b1) + 1
				start = len(out) - disp
				if start < 0:
					raise ValueError("back-reference before start of output")
				for i in range(length):
					out.append(out[start + i])
			else:
				out.append(data[pos])
				pos += 1
	return bytes(out[:size])


def gba_rgb(c):
	return ((c & 31) * 8, ((c >> 5) & 31) * 8, ((c >> 10) & 31) * 8)


def load_face(rom, pid):
	entry = FACE_TABLE + pid * FACE_SIZE
	pal_off = rom_ptr(rom, entry + FACE_OFF_PAL)
	card_off = rom_ptr(rom, entry + FACE_OFF_CARD)
	pal = list(struct.unpack_from("<16H", rom, pal_off))
	card = lz77_decode(rom[card_off:])
	if len(card) != CARD_TILE_COUNT * 32:
		raise ValueError("portrait 0x%02X: card is %d bytes, expected %d" % (pid, len(card), CARD_TILE_COUNT * 32))
	return pal, card


def card_image(pal, card):
	"""The 80x72 RGB card: tile t at column t % 10, row t // 10; 8x8 4bpp, left pixel = low nibble."""
	rgb = [gba_rgb(c) for c in pal]
	pixels = [None] * (CARD_W * CARD_H)
	for t in range(CARD_TILE_COUNT):
		tx = t % CARD_TILES_PER_ROW
		ty = t // CARD_TILES_PER_ROW
		for y in range(8):
			for x in range(8):
				byte = card[t * 32 + y * 4 + x // 2]
				idx = (byte >> (4 * (x & 1))) & 0x0F
				pixels[(ty * 8 + y) * CARD_W + tx * 8 + x] = rgb[idx]
	img = Image.new("RGB", (CARD_W, CARD_H))
	img.putdata(pixels)
	return img


def mug_indices(pal, card, x0, y0, size):
	"""32x32 palette indices of the crop: BOX resize, then the nearest palette entry 0..15 (squared RGB distance)."""
	img = card_image(pal, card)
	crop = img.crop((x0, y0, x0 + size, y0 + size)).resize((MUG, MUG), Image.BOX)
	pal_rgb = [gba_rgb(c) for c in pal]
	indices = []
	raw = crop.tobytes()
	for p in range(0, len(raw), 3):
		r, g, b = raw[p], raw[p + 1], raw[p + 2]
		best = min(range(16), key=lambda i: (r - pal_rgb[i][0]) ** 2 + (g - pal_rgb[i][1]) ** 2 + (b - pal_rgb[i][2]) ** 2)
		indices.append(best)
	return indices


def pack_4bpp(indices):
	"""512 bytes: 16 tiles of 8x8 4bpp, row-major (4 per row), left pixel = low nibble."""
	out = bytearray()
	for ty in range(MUG // 8):
		for tx in range(MUG // 8):
			for y in range(8):
				row = (ty * 8 + y) * MUG
				for x in range(0, 8, 2):
					lo = indices[row + tx * 8 + x]
					hi = indices[row + tx * 8 + x + 1]
					out.append(lo | (hi << 4))
	assert len(out) == 512
	return bytes(out)


def compress(raw):
	if not os.path.isfile(COMPRESS):
		raise FileNotFoundError("compress tool not found: %s" % COMPRESS)
	with tempfile.TemporaryDirectory() as tmp:
		in_path = os.path.join(tmp, "raw.bin")
		out_path = os.path.join(tmp, "packed.bin")
		with open(in_path, "wb") as f:
			f.write(raw)
		subprocess.run([COMPRESS, in_path, out_path], check=True)
		with open(out_path, "rb") as f:
			data = f.read()
	if not data or data[0] != 0x10:
		raise RuntimeError("compressed output does not start with 0x10")
	return data


def build_entries(rom):
	entries = []
	for name, pid, x0, y0, size in MONSTERS:
		pal, card = load_face(rom, pid)
		indices = mug_indices(pal, card, x0, y0, size)
		raw = pack_4bpp(indices)
		entries.append({
			"name": name,
			"pid": pid,
			"pal": pal,
			"indices": indices,
			"raw": raw,
			"packed": compress(raw),
		})
	return entries


def write_c(path, entries):
	lines = [
		"/* Generated by Tools/hex/gen_monster_mugs.py from the FE8 monster class cards. Do not edit. */",
		"#include \"common-chax.h\"",
		"#include \"kernel/hex.h\"",
		"",
		"#ifdef CONFIG_HEX_CARDS",
		"",
	]
	for e in entries:
		n = ident(e["name"])
		lines.append("static const u8 sChibi_%s[] __attribute__((aligned(4))) = {" % n)
		packed = e["packed"]
		for i in range(0, len(packed), 16):
			chunk = ", ".join("0x%02X" % b for b in packed[i:i + 16])
			lines.append("\t%s," % chunk)
		lines.append("};\t/* LZ77, 32x32 4bpp */")
		lines.append("static const u16 sPal_%s[16] = { %s };\t\t/* the card's palette */" % (
			n, ", ".join("0x%04X" % c for c in e["pal"])))
		lines.append("")
	lines.append("const struct HexMonsterArt gHexMonsterArt[HEX_MONSTER_COUNT] = {")
	for e in entries:
		n = ident(e["name"])
		lines.append("\t{ sChibi_%s, sPal_%s },\t/* %s */" % (n, n, e["name"]))
	lines.append("};")
	lines.append("")
	lines.append("#endif /* CONFIG_HEX_CARDS */")
	lines.append("")
	os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
	with open(path, "w", encoding="utf-8", newline="\n") as f:
		f.write("\n".join(lines))


def check_c(path, entries):
	"""Read the arrays back from the written .c file, decompress each stream and compare with the raw tiles."""
	with open(path, "r", encoding="utf-8") as f:
		text = f.read()
	ok = True
	for e in entries:
		n = ident(e["name"])
		m = re.search(r"static const u8 sChibi_%s\[\] __attribute__\(\(aligned\(4\)\)\) = \{([^}]*)\};" % n, text)
		p = re.search(r"static const u16 sPal_%s\[16\] = \{([^}]*)\};" % n, text)
		if not m or not p:
			print("FAIL %s: arrays not found in %s" % (e["name"], path))
			ok = False
			continue
		stream = bytes(int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]+", m.group(1)))
		pal = [int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]+", p.group(1))]
		if stream != e["packed"]:
			print("FAIL %s: written stream differs from the generated one" % e["name"])
			ok = False
			continue
		if pal != e["pal"]:
			print("FAIL %s: written palette differs" % e["name"])
			ok = False
			continue
		if lz77_decode(stream) != e["raw"]:
			print("FAIL %s: decompressed stream differs from the raw tiles" % e["name"])
			ok = False
			continue
		print("OK %s (portrait 0x%02X, %d bytes compressed)" % (e["name"], e["pid"], len(stream)))
	return ok


def font(size):
	try:
		return ImageFont.load_default(size=size)
	except TypeError:
		return ImageFont.load_default()


def write_preview(path, entries):
	cell_w = MUG * PREVIEW_SCALE + 8
	label_h = 22
	cell_h = MUG * PREVIEW_SCALE + label_h
	canvas = Image.new("RGB", (cell_w * len(entries) + 8, cell_h + 8), PREVIEW_BG)
	draw = ImageDraw.Draw(canvas)
	fnt = font(12)
	for i, e in enumerate(entries):
		mug = Image.new("RGB", (MUG, MUG), PREVIEW_BG)
		rgb = [gba_rgb(c) for c in e["pal"]]
		mug.putdata([PREVIEW_BG if idx == 0 else rgb[idx] for idx in e["indices"]])
		mug = mug.resize((MUG * PREVIEW_SCALE, MUG * PREVIEW_SCALE), Image.NEAREST)
		x = 4 + i * cell_w + 4
		canvas.paste(mug, (x, 4))
		tw = draw.textlength(e["name"], font=fnt)
		draw.text((x + (MUG * PREVIEW_SCALE - tw) / 2, 4 + MUG * PREVIEW_SCALE + 4), e["name"], fill=(255, 255, 255), font=fnt)
	os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
	canvas.save(path)


def main(argv=None):
	ap = argparse.ArgumentParser(description="FE8 monster class cards -> Hex minimugs (Data/Hex/HexMonsterArt.c)")
	ap.add_argument("--rom", default=DEFAULT_ROM, help="clean FE8 ROM (default: fe8.gba at the repo root)")
	ap.add_argument("--out", default=DEFAULT_OUT, help="C file to write (default: Data/Hex/HexMonsterArt.c)")
	ap.add_argument("--preview", metavar="PATH.png", help="write a 4x preview PNG of the 12 mugs")
	ap.add_argument("--check", action="store_true", help="read the written .c back and compare each stream with the raw tiles")
	args = ap.parse_args(argv)

	with open(args.rom, "rb") as f:
		rom = f.read()

	entries = build_entries(rom)
	write_c(args.out, entries)
	print("wrote %s" % args.out)

	if args.preview:
		write_preview(args.preview, entries)
		print("wrote %s" % args.preview)

	if args.check:
		if not check_c(args.out, entries):
			print("FAIL: check failed", file=sys.stderr)
			return 1
	return 0


if __name__ == "__main__":
	sys.exit(main())
