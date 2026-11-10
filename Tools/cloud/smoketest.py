#!/usr/bin/env python3
"""Emulator smoke tests for the built ROM (fe8-kernel-dev.gba), run with gbarun (Tools/cloud/README.md).

    python3 Tools/cloud/smoketest.py [--quick] [--out DIR] [--only NAME ...]

What it checks:
  boot        a new game reaches the prologue map: chapter 0, Eirika on the map, the screen drawn.
  statscreen  R on Eirika opens the stat screen; B closes it and the map comes back.
  <district>  for Serafew, Port Kiris, Caer Pelyn and Jehanna Hall: a test ROM starts a new game in the district; Eirika
              walks to every spot in its events/traps.c (fishing, River Crossing, Tower Stack, guild board); A opens
              the spot's screen and B brings the map back. River Crossing and Tower Stack also check their state in RAM.
  tokens      (Serafew) a River Crossing game over pays the guild board's tokens (1 per 100 points).
  claim       (Serafew) a met quest is claimed on the guild board for its tokens.

--quick runs boot, statscreen and Serafew only. --only runs the named tests (boot, statscreen, a district's name,
tokens, claim). Screenshots of every step and a contact sheet (sheet.png) go to --out (default: a new temp folder),
with report.txt. The exit status is the number of failed checks (0: all passed).

Build first (Tools/cloud/build.sh). RAM addresses come from fe8-kernel-dev.sym and include/link/config-memmap.o, so
they follow the build. Needs Pillow.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from collections import deque

from PIL import Image, ImageChops, ImageStat

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ROM = os.path.join(ROOT, "fe8-kernel-dev.gba")
BOOT = os.path.join(ROOT, "Tools", "cloud", "boot-to-prologue.txt")
sys.path.insert(0, os.path.join(ROOT, "Tools", "district", "tilegen"))
import rules  # noqa: E402  (the district terrain rules: which terrains are ground)

# name: (chapter id, tile config copy in CustomCampaign/Chapters/_hexshop, River Crossing course)
DISTRICTS = {
    "Serafew": (0x3B, "town_cfg.bin", 0),
    "PortKiris": (0x3C, "town_cfg.bin", 1),
    "CaerPelyn": (0x41, "caer_cfg.bin", 2),
    "Jehanna": (0x42, "jeh_cfg.bin", 3),
}
SPOTS = {"FISHING_SPOT": "fishing", "CROSSING_SPOT": "crossing", "TOWER_SPOT": "tower", "GUILD_BOARD": "guild"}

# FE8U RAM that does not move
PLAYST = 0x0202BCF0
UNIT1 = 0x0202BE4C
CHAPTER = PLAYST + 0x0E
UNIT_X, UNIT_Y = UNIT1 + 0x10, UNIT1 + 0x11
CHAPTER_TABLE = 0x8B0890
CHAPTER_ROW = 0x94


class Smoke:
    def __init__(self, out):
        self.out = out
        self.results = []
        self.shots = []
        self.sym = self._symbols()

    # ---- symbols ----

    def _symbols(self):
        sym = {}
        with open(os.path.join(ROOT, "fe8-kernel-dev.sym")) as f:
            for line in f:
                p = line.split()
                if len(p) >= 2:
                    try:
                        sym.setdefault(p[1], int(p[0], 16))
                    except ValueError:
                        pass
        nm = os.path.join(os.environ.get("DEVKITARM", ""), "bin", "arm-none-eabi-nm")
        obj = os.path.join(ROOT, "include", "link", "config-memmap.o")
        if os.path.exists(nm) and os.path.exists(obj):
            for line in subprocess.run([nm, obj], capture_output=True, text=True).stdout.splitlines():
                p = line.split()
                if len(p) == 3:
                    sym[p[2]] = int(p[0], 16)
        return sym

    # ---- running ----

    def run(self, rom, lines, cwd):
        """Runs a gbarun script; returns the values its r8/r16/r32 lines print, in order."""
        path = os.path.join(cwd, "_smoke.txt")
        with open(path, "w") as f:
            f.write("\n".join(lines) + "\n")
        res = subprocess.run(["gbarun", rom, path], cwd=cwd, capture_output=True, text=True, timeout=900)
        vals = []
        for line in res.stdout.splitlines():
            m = re.match(r"\s*0x[0-9A-Fa-f]+ = (0x[0-9A-Fa-f]+)", line)
            if m:
                vals.append(int(m.group(1), 16))
        return vals

    def check(self, name, ok, detail=""):
        self.results.append((name, bool(ok), detail))
        print("%-4s %s%s" % ("PASS" if ok else "FAIL", name, "  (" + detail + ")" if detail else ""), flush=True)
        return ok

    def shot(self, path, label):
        if os.path.exists(path):
            self.shots.append((path, label))

    # ---- images ----

    @staticmethod
    def diff(a, b):
        """Mean difference of two screenshots, 0..255."""
        ia, ib = Image.open(a).convert("RGB"), Image.open(b).convert("RGB")
        return sum(ImageStat.Stat(ImageChops.difference(ia, ib)).mean) / 3

    @staticmethod
    def hist_dist(a, b):
        """How different two screenshots' colours are, 0 (the same mix) .. 2: a scrolled map stays close."""
        ha = Image.open(a).convert("RGB").quantize(64, method=Image.Quantize.MEDIANCUT)
        pal = ha.getpalette()[:192]
        hb = Image.open(b).convert("RGB").quantize(palette=ha, dither=Image.Dither.NONE)
        ca, cb = ha.histogram()[:64], hb.histogram()[:64]
        na, nb = float(sum(ca)) or 1.0, float(sum(cb)) or 1.0
        return sum(abs(x / na - y / nb) for x, y in zip(ca, cb))

    def same_screen(self, a, b):
        """The same screen, or the same map scrolled (the camera can move when a screen closes)."""
        return self.diff(a, b) < 15 or self.hist_dist(a, b) < 0.25

    @staticmethod
    def brightness(a):
        return sum(ImageStat.Stat(Image.open(a).convert("RGB")).mean) / 3

    # ---- tests ----

    def test_boot(self):
        d = os.path.join(self.out, "boot")
        os.makedirs(d, exist_ok=True)
        with open(BOOT) as f:
            lines = [ln.rstrip("\n") for ln in f]
        vals = self.run(ROM, lines + ["r8 %#x" % CHAPTER, "r32 %#x" % UNIT1, "r8 %#x" % UNIT_X], d)
        png = os.path.join(d, "boot.png")
        self.shot(png, "boot")
        ok = os.path.exists(os.path.join(d, "boot.ss"))
        self.check("boot: a save state of the prologue map", ok)
        if not ok:
            return None
        self.check("boot: chapter 0 (prologue)", vals[0] == 0, "chapter %#x" % vals[0])
        self.check("boot: Eirika is on the map", vals[1] != 0, "unit 1 character %#x" % vals[1])
        self.check("boot: the screen is drawn", self.brightness(png) > 20, "brightness %.0f" % self.brightness(png))
        return os.path.join(d, "boot.ss")

    def test_statscreen(self, boot_ss):
        d = os.path.join(self.out, "boot")
        self.run(ROM, ["load " + boot_ss, "wait 30", "shot st0.png", "press R", "wait 60", "shot st1.png",
                       "press B", "wait 60", "shot st2.png"], d)
        a, b, c = (os.path.join(d, "st%d.png" % i) for i in range(3))
        self.shot(b, "stat screen")
        self.check("statscreen: R opens it", self.diff(a, b) > 25, "change %.0f" % self.diff(a, b))
        self.check("statscreen: B returns to the map", self.same_screen(a, c), "change %.0f" % self.diff(a, c))

    def district_rom(self, name, d):
        """A test ROM whose new game starts in the district (its map, tileset and events in chapter row 0), with its
        wandering NPCs pointed at chapter 0."""
        ch = DISTRICTS[name][0]
        data = bytearray(open(ROM, "rb").read())
        base = CHAPTER_TABLE
        for off in list(range(4, 0x0C)) + [0x74]:
            data[base + off] = data[base + ch * CHAPTER_ROW + off]
        npcs = self.sym.get("gDistrictNpcs")
        if npcs:
            i = npcs - 0x08000000
            while data[i] != 0xFF:
                if data[i] == ch:
                    data[i] = 0
                i += 4
        rom = os.path.join(d, "t.gba")
        open(rom, "wb").write(data)
        return rom

    def walk(self, rom, state, target, layout, cfg, avoid, d, max_runs=40):
        """Walks Eirika to target from a save state. Each run takes up to 4 path steps; a step that does not move her
        (a wandering NPC, a tile free movement refuses) marks that tile blocked, and the path is planned again."""
        w, h, tiles = layout["w"], layout["h"], layout["tiles"]
        ground = rules.WALK - {0x26, 0x1E}
        blocked = set()

        def ok(x, y):
            if not (0 <= x < w and 0 <= y < h) or (x, y) in blocked:
                return False
            if (x, y) == target:
                return True
            terr = cfg[0x2000 + tiles[y * w + x]]
            return (x, y) not in avoid and terr in ground and terr not in (3, 5, 6, 7)

        def path(s):
            prev = {s: None}
            q = deque([s])
            while q:
                c = q.popleft()
                if c == target:
                    break
                for key, (dx, dy) in (("RIGHT", (1, 0)), ("LEFT", (-1, 0)), ("DOWN", (0, 1)), ("UP", (0, -1))):
                    n = (c[0] + dx, c[1] + dy)
                    if n not in prev and ok(*n):
                        prev[n] = (c, key)
                        q.append(n)
            if target not in prev:
                return None
            steps, c = [], target
            while prev[c]:
                c, key = prev[c]
                steps.append(key)
            return steps[::-1]

        cur = state
        out = os.path.join(d, "walk.ss")
        pos = tuple(self.run(rom, ["load " + cur, "r8 %#x" % UNIT_X, "r8 %#x" % UNIT_Y], d))
        for _ in range(max_runs):
            if pos == target:
                return cur
            steps = path(pos)
            if not steps:
                return None
            lines = ["load " + cur]
            for key in steps[:4]:
                # free movement: the first short press of a direction may only turn the unit, the second steps
                lines += ["hold %s 4" % key, "wait 20", "hold %s 4" % key, "wait 24", "r8 %#x" % UNIT_X,
                          "r8 %#x" % UNIT_Y]
            lines.append("save " + out)
            vals = self.run(rom, lines, d)
            x, y = pos
            for k, key in enumerate(steps[:4]):
                dx, dy = {"RIGHT": (1, 0), "LEFT": (-1, 0), "DOWN": (0, 1), "UP": (0, -1)}[key]
                got = (vals[2 * k], vals[2 * k + 1])
                if got == (x, y):
                    blocked.add((x + dx, y + dy))
                    break
                x, y = got
            pos = (vals[-2], vals[-1])
            cur = out
        return cur if pos == target else None

    def test_district(self, name):
        ch, cfgname, course = DISTRICTS[name]
        d = os.path.join(self.out, name)
        os.makedirs(d, exist_ok=True)
        rom = self.district_rom(name, d)
        with open(BOOT) as f:
            lines = [ln.rstrip("\n") for ln in f]
        self.run(rom, lines, d)
        start = os.path.join(d, "start.ss")
        self.run(rom, ["load " + os.path.join(d, "boot.ss"), "w8 %#x %#x" % (CHAPTER, ch), "wait 30",
                       "save " + start], d)
        if not self.check("%s: the district map loads" % name, os.path.exists(start)):
            return {}
        layout = json.load(open(os.path.join(ROOT, "CustomCampaign", "Chapters", name, "map", name + "_layout.json")))
        cfg = open(os.path.join(ROOT, "CustomCampaign", "Chapters", "_hexshop", cfgname), "rb").read()
        traps = open(os.path.join(ROOT, "CustomCampaign", "Chapters", name, "events", "traps.c")).read()
        spots = [(m.group(1), int(m.group(2)), int(m.group(3))) for m in re.finditer(r"(\w+)\((\d+),\s*(\d+)", traps)]
        avoid = {(x, y) for _, x, y in spots}
        states = {}
        for kind, x, y in spots:
            if kind not in SPOTS:
                continue
            label = "%s: %s at (%d, %d)" % (name, SPOTS[kind], x, y)
            at = self.walk(rom, start, (x, y), layout, cfg, avoid - {(x, y)}, d)
            if not self.check(label + ": reachable", at):
                continue
            tag = "%s_%d_%d" % (SPOTS[kind], x, y)
            spot_ss = os.path.join(d, tag + ".ss")
            open_ss = os.path.join(d, tag + "_open.ss")
            reads = []
            if SPOTS[kind] == "crossing" and "gCrossingState" in self.sym:
                reads = ["r8 %#x" % (self.sym["gCrossingState"] + 0x31), "r8 %#x" % (self.sym["gCrossingState"] + 0x37)]
            if SPOTS[kind] == "tower" and "gTowerState" in self.sym:
                reads = ["r8 %#x" % (self.sym["gTowerState"] + 0x4A)]
            p0, p1, p2 = (os.path.join(d, "%s_%d.png" % (tag, i)) for i in range(3))
            # An A press as a wandering NPC starts a step is ignored (they move 30 frames after the player stops), as
            # a player would find; so a press that opens nothing is tried again after another wait.
            for wait in (10, 60, 0):
                vals = self.run(rom, ["load " + at, "save " + spot_ss, "wait %d" % wait, "shot %s_0.png" % tag,
                                      "hold A 6", "wait 150", "shot %s_1.png" % tag, "save " + open_ss] + reads +
                                ["press B", "wait 150", "shot %s_2.png" % tag], d)
                if self.diff(p0, p1) > 25:
                    break
            self.shot(p1, "%s %s" % (name, SPOTS[kind]))
            self.check(label + ": A opens it", self.diff(p0, p1) > 25, "change %.0f" % self.diff(p0, p1))
            self.check(label + ": B returns to the map", self.same_screen(p0, p2),
                       "change %.0f, colours %.2f" % (self.diff(p0, p2), self.hist_dist(p0, p2)))
            if SPOTS[kind] == "crossing" and len(vals) == 2:
                self.check(label + ": 3 lives, course %d" % course, vals == [3, course], "lives %d, course %d" % tuple(vals))
            if SPOTS[kind] == "tower" and len(vals) == 1:
                self.check(label + ": lives set", vals[0] > 0, "lives %d" % vals[0])
            states.setdefault(SPOTS[kind], (spot_ss, open_ss))
        states["rom"] = rom
        return states

    def test_tokens(self, states):
        d = os.path.join(self.out, "Serafew")
        if "crossing" not in states or "gGuildSave" not in self.sym or "gCrossingState" not in self.sym:
            return self.check("tokens: River Crossing pays tokens", False, "no crossing spot or symbols")
        g, cs = self.sym["gGuildSave"], self.sym["gCrossingState"]
        vals = self.run(states["rom"], [
            "load " + states["crossing"][1], "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2), "press DOWN", "wait 20",
            "w32 %#x 1300" % (cs + 0x1C), "w8 %#x 1" % (cs + 0x31), "w16 %#x 2" % (cs + 0x22), "wait 260",
            "shot tokens.png", "r16 %#x" % (g + 2)], d)
        self.shot(os.path.join(d, "tokens.png"), "crossing game over")
        self.check("tokens: a 1300-point River Crossing game pays 13 tokens", vals == [13], "tokens %s" % vals)

    def test_claim(self, states):
        d = os.path.join(self.out, "Serafew")
        if "guild" not in states or "gGuildSave" not in self.sym or "gCrossingRecords" not in self.sym:
            return self.check("claim: a met quest pays", False, "no guild board or symbols")
        g, cr = self.sym["gGuildSave"], self.sym["gCrossingRecords"]
        vals = self.run(states["rom"], [
            "load " + states["guild"][0], "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2), "w32 %#x 0" % (g + 8),
            "w16 %#x 0x5243" % cr, "w16 %#x 900" % (cr + 2), "hold A 6", "wait 150", "press A", "wait 30",
            "shot claim.png", "r16 %#x" % (g + 2), "r32 %#x" % (g + 8)], d)
        self.shot(os.path.join(d, "claim.png"), "quest claimed")
        self.check("claim: Serafew's first quest pays 15 tokens and is marked claimed", vals == [15, 1],
                   "tokens, claimed bits %s" % vals)

    # ---- report ----

    def report(self):
        n = len(self.shots)
        if n:
            cols = 4
            sheet = Image.new("RGB", (240 * cols, 172 * ((n + cols - 1) // cols)), (24, 24, 24))
            from PIL import ImageDraw
            draw = ImageDraw.Draw(sheet)
            for i, (p, label) in enumerate(self.shots):
                x, y = (i % cols) * 240, (i // cols) * 172
                sheet.paste(Image.open(p).convert("RGB"), (x, y))
                draw.text((x + 2, y + 160), label, fill=(255, 255, 255))
            sheet.save(os.path.join(self.out, "sheet.png"))
        failed = [r for r in self.results if not r[1]]
        with open(os.path.join(self.out, "report.txt"), "w") as f:
            for name, ok, detail in self.results:
                f.write("%-4s %s%s\n" % ("PASS" if ok else "FAIL", name, "  (" + detail + ")" if detail else ""))
        print("\n%d checks, %d failed. Screens: %s" % (len(self.results), len(failed), os.path.join(self.out, "sheet.png")))
        return len(failed)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--quick", action="store_true", help="boot, statscreen and Serafew only")
    ap.add_argument("--out", help="folder for screenshots and the report (default: a new temp folder)")
    ap.add_argument("--only", nargs="+", help="run only these tests")
    a = ap.parse_args()
    if not os.path.exists(ROM):
        sys.exit("%s is missing: build first (Tools/cloud/build.sh)" % ROM)
    out = os.path.abspath(a.out) if a.out else tempfile.mkdtemp(prefix="smoke-")
    os.makedirs(out, exist_ok=True)
    s = Smoke(out)
    names = ["Serafew"] if a.quick else list(DISTRICTS)
    want = set(a.only) if a.only else None

    def on(n):
        return want is None or n in want

    if on("boot") or on("statscreen"):
        boot_ss = s.test_boot()
        if boot_ss and on("statscreen"):
            s.test_statscreen(boot_ss)
    for n in names:
        if on(n) or (n == "Serafew" and (on("tokens") or on("claim"))):
            states = s.test_district(n)
            if n == "Serafew" and on("tokens"):
                s.test_tokens(states)
            if n == "Serafew" and on("claim"):
                s.test_claim(states)
    sys.exit(s.report())


if __name__ == "__main__":
    main()
