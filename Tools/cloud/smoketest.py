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
  prizes      (Serafew) the guild board sells the Omni Tonic and a skill scroll: the tokens drop by their costs and
              both reach Eirika.
  unlocks     (Serafew) the board's story quests show "+story"/"+support"; the Eirika and Seth one adds 40 support
              points each way.
  featured    (Serafew) with the story on Chapter 8, the Tower Stack quest tops the board and pays 15 + 15 tokens.
  life        (Serafew) district life: the map is darker at night (story Chapter 8); a Tower Stack best of 100 beats
              Marlo's first record and pays 10 tokens once; a test ROM puts the townsfolk on their spots.
  snek        (Port Kiris, not in --quick) the Pied Piper's Snek: Yes, 80 points, B: 10 tokens and the high score.
  savelayout  the game save's and the suspend save's chunks fit their SRAM blocks (read from the ROM's chunk tables).
  suspend     Eirika waits (the game writes its suspend save), a soft reset and Resume Chapter bring back her HP and
              position and every district save chunk (guild, River Crossing, Tower Stack, fishing, Hex, Snek).
  gamesave    the debugger's "Save & restart ch." writes a game save; a soft reset and Restart Chapter bring back the
              same district chunks.
  battle      Eirika attacks the soldier next to her: the forecast's attack rises by the skill's bonus with Str Bonus
              in a free skill slot, the fight hurts the soldier, and with 99 EXP she levels up. Her level-up quote's
              portrait is the expression its tier asks for ([Sad] poor, [Happy] great) when the table has one.
  battlequote O'Neill moved next to Eirika: his battle quote's [LoadFace_Eirika][Surprised] shows her expression
              (a test ROM maps her sad portrait to SURPRISED).
  debugflag   the debugger opens with the base-chapter intro skip flag (0xEC) set, and not with its own off flag
              (0xEF) set.
  stealer     Eirika holds the Skill Stealer and defeats O'Neill (a boss, 1 HP) with Str Bonus in his first skill
              slot: she gets Str Bonus's scroll.
  frames      the battlequote scene with Eirika's frame set pointed at [Surprised] (a test ROM): her base portrait
              stays, the set is on her face's slot and its open eyes are in the face's graphics.
  awards      the prologue's other two enemies removed, Eirika fights O'Neill until he falls and the chapter ends: the
              chapter awards screen (all three for Eirika) comes before the save menu and pays 10 + 5 + 5 tokens.

--quick runs every test but the other three districts. --only runs the named tests (boot, statscreen,
a district's name, tokens, claim, prizes, unlocks, featured, life, snek, savelayout, suspend, gamesave, battle, battlequote,
frames, stealer, debugflag, awards). Screenshots of every step and a contact sheet (sheet.png) go to --out (default: a new temp folder),
with report.txt. The exit status is the number of failed checks (0: all passed).

Build first (Tools/cloud/build.sh). RAM addresses come from fe8-kernel-dev.sym and include/link/config-memmap.o, so
they follow the build. Needs Pillow.
"""
import argparse
import json
import os
import re
import struct
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
UNIT_GREEN = 0x0202DDCC
PERMANENT_FLAGS = 0x03005250  # gPermanentFlagBits: flag f (0x65 and up) is bit (f - 0x65)
ITEM_SKILL_STEALER = 0xBE  # CONFIG_ITEM_INDEX_SKILL_STEALER (include/configs/configs.h)
PALETTE_BUFFER = 0x020228A8
CHAPTER = PLAYST + 0x0E
UNIT_X, UNIT_Y = UNIT1 + 0x10, UNIT1 + 0x11
CHAPTER_TABLE = 0x8B0890
CHAPTER_ROW = 0x94
BATTLE_ACTOR = 0x0203A4EC
BATTLE_TARGET = 0x0203A56C
GFACES = 0x03004980
EVENT_SLOTS = 0x030004B8
EIRIKA_PORTRAIT = 0x02
EXPRESSION_SAD, EXPRESSION_HAPPY, EXPRESSION_SURPRISED, EXPRESSION_DETERMINED = 3, 4, 5, 6  # include/kernel/portrait-expressions.h
BM_MAP_UNIT = 0x0202E4D8
ITEM_TONIC = 0xDF
SKILL_SCROLL_ITEMS = (0x0A, 0xBD, 0xC9, 0xCA)  # CONFIG_ITEM_INDEX_SKILL_SCROLL_1..4 (include/configs/configs.h)
GUILD_PRIZE_WORD, GUILD_PRIZE_SKILL = 1, 2  # include/kernel/guild.h
GUILD_QUEST_WINS, GUILD_QUEST_TOKENS_EARNED = 9, 11  # include/kernel/guild.h
BWL_SIZE = 15  # struct NewBwl (include/kernel/bwl.h), packed
SNEK_PIPER_TILE, SNEK_TALK_TILE = (17, 14), (16, 14)  # CustomCampaign/Chapters/PortKiris/events/units.c

# save chunk save function: the RAM it saves (its size comes from the chunk table; Snek saves a u16 and a magic)
SAVE_RAM = {
    "Guild_Save": "gGuildSave", "CrossingRecords_Save": "gCrossingRecords",
    "CrossingProgress_Save": "gCrossingProgress", "TowerRecords_Save": "gTowerRecords",
    "FishingRecords_Save": "gFishingRecords", "FishingProgress_Save": "gFishingProgress",
    "HexSave_Save": "gHexSave", "Snek_SaveHighScore": "gSnekHighScore",
}
# values written before saving, so a chunk that is not saved or loaded shows up: (RAM symbol, offset, bits, value)
SENTINELS = [("gGuildSave", 2, 16, 1234), ("gCrossingRecords", 2, 16, 777), ("gTowerRecords", 2, 16, 345),
             ("gSnekHighScore", 0, 16, 42)]

# the Vesly debugger (B on an empty tile): page 2, "Save & restart ch." (as in boot-to-prep.txt), then the save slot
DEBUG_SAVE = ["press B", "wait 60", "press RIGHT", "wait 40", "press DOWN", "wait 10", "press DOWN", "wait 10",
              "press A", "wait 60"] + ["press DOWN", "wait 10"] * 7 + ["press START", "wait 200", "press A",
                                                                         "wait 120", "press A", "wait 600"]
# A+B+START+SELECT, then the language screen and the title
SOFT_RESET = ["hold A+B+START+SELECT 10", "wait 500", "press A", "wait 200", "press START", "wait 200"]
# Eirika is under the cursor at the start of the prologue: A selects her, A stays, and the action menu opens
EIRIKA_MENU = ["wait 30", "press A", "wait 30", "press A", "wait 40"]


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
        (a wandering NPC, a tile free movement refuses) marks that tile blocked, and the path is planned again. When the
        blocked tiles leave no path, it waits two seconds for the wandering NPCs to move on and forgets them (4 times at
        most)."""
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
        waits = 0
        for _ in range(max_runs):
            if pos == target:
                return cur
            steps = path(pos)
            if not steps and blocked and waits < 4:
                # wandering NPCs can close a narrow road for a while: let them move on and plan again
                waits += 1
                blocked.clear()
                self.run(rom, ["load " + cur, "wait 120", "save " + out], d)
                cur = out
                continue
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
            "w8 %#x 0" % self.sym["gWmStoryChapter"], "w16 %#x 0xFFFF" % (g + 0x0C), "w16 %#x 0x5243" % cr,
            "w16 %#x 900" % (cr + 2), "hold A 6", "wait 150", "press A", "wait 30",
            "shot claim.png", "r16 %#x" % (g + 2), "r32 %#x" % (g + 8)], d)
        self.shot(os.path.join(d, "claim.png"), "quest claimed")
        self.check("claim: Serafew's first quest pays 15 tokens and is marked claimed", vals == [15, 1],
                   "tokens, claimed bits %s" % vals)

    def test_featured(self, states):
        """Serafew's board while the story is on Chapter 8: the Tower Stack quest (met, best 100) tops the board in gold
        with "Featured!", and claiming it pays its 15 tokens and the 15-token bonus."""
        d = os.path.join(self.out, "Serafew")
        need = ("gGuildSave", "gTowerRecords", "gWmStoryChapter")
        if "guild" not in states or not all(n in self.sym for n in need):
            return self.check("featured: the featured quest", False, "no guild board or symbols")
        g, tr = self.sym["gGuildSave"], self.sym["gTowerRecords"]
        vals = self.run(states["rom"], [
            "load " + states["guild"][0], "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2), "w32 %#x 0" % (g + 8),
            "w8 %#x 0x09" % self.sym["gWmStoryChapter"], "w16 %#x 0xFFFF" % (g + 0x0C), "w16 %#x 0x5453" % tr,
            "w16 %#x 100" % (tr + 2),
            "hold A 6", "wait 150", "shot featured.png", "press A", "wait 30", "r16 %#x" % (g + 2)], d)
        self.shot(os.path.join(d, "featured.png"), "featured quest")
        self.check("featured: Chapter 8's Serafew quest tops the board and pays 15 + 15 tokens", vals == [30],
                   "tokens %s" % vals)

    def test_life(self, states):
        """District life on Serafew's map. Time of day: with the story on Chapter 8 (night), the map's palettes come
        back from the guild board darker than with the story at its start (day). Rival: with Tower Stack's best at
        100, opening the board pays Marlo's first Tower Stack record (60) once, 10 tokens, and DOWN on the Records page
        shows his records. Townsfolk: a test ROM whose spots count from the start, with the NPCs standing still,
        puts three of them on their spots."""
        d = os.path.join(self.out, "Serafew")
        need = ("gGuildSave", "gWmStoryChapter", "gTowerRecords", "gCrossingRecords", "gSnekHighScore")
        if "guild" not in states or not all(n in self.sym for n in need):
            return self.check("life: district life", False, "no guild board or symbols")
        g, story = self.sym["gGuildSave"], self.sym["gWmStoryChapter"]
        pal = [ "r16 %#x" % (PALETTE_BUFFER + 0x20 * 6 + 2 * k) for k in range(16 * 10)]
        tint = {}
        for name, chapter in (("day", 0), ("night", 0x09)):
            vals = self.run(states["rom"], ["load " + states["guild"][0], "w8 %#x %#x" % (story, chapter), "hold A 6",
                                            "wait 150", "press B", "wait 150", "shot %s.png" % name] + pal, d)
            tint[name] = sum((v & 31) + ((v >> 5) & 31) + ((v >> 10) & 31) for v in vals) if len(vals) == 160 else 0
            self.shot(os.path.join(d, name + ".png"), "Serafew by %s" % name)
        self.check("life: the map is darker at night", 0 < tint["night"] < tint["day"] * 0.8,
                   "colour sum day %d, night %d" % (tint["day"], tint["night"]))

        cr, tr = self.sym["gCrossingRecords"], self.sym["gTowerRecords"]
        lines = ["load " + states["guild"][0], "w8 %#x 0" % story, "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2),
                 "w16 %#x 0" % (g + 0x0C), "w16 %#x 0x5243" % cr, "w16 %#x 0" % (cr + 2), "w16 %#x 0x5453" % tr,
                 "w16 %#x 100" % (tr + 2), "w16 %#x 0" % self.sym["gSnekHighScore"], "hold A 6", "wait 150",
                 "r16 %#x" % (g + 2), "r16 %#x" % (g + 0x0C), "press R", "wait 20", "press R", "wait 20",
                 "press DOWN", "wait 30", "shot rival.png", "press B", "wait 150", "hold A 6", "wait 150",
                 "r16 %#x" % (g + 2)]
        vals = self.run(states["rom"], lines, d)
        self.shot(os.path.join(d, "rival.png"), "rival records")
        self.check("life: beating Marlo's Tower Stack record pays 10 tokens once", vals == [10, 2, 10],
                   "tokens, beaten bits, tokens after reopening %s" % vals)
        self.test_townsfolk()

    def test_townsfolk(self):
        name, d = "Serafew", os.path.join(self.out, "Townsfolk")
        os.makedirs(d, exist_ok=True)
        if "gDistrictNpcSpots" not in self.sym or "gDistrictNpcs" not in self.sym:
            return self.check("life: townsfolk's places", False, "no symbols")
        ch = DISTRICTS[name][0]
        rom = self.district_rom(name, d)
        data = bytearray(open(rom, "rb").read())
        want = {}
        a = self.sym["gDistrictNpcSpots"] - 0x08000000
        while data[a] != 0xFF:
            if data[a] == ch:
                data[a], data[a + 2] = 0, 0
                want[data[a + 1]] = (data[a + 3], data[a + 4])
            a += 6
        a = self.sym["gDistrictNpcs"] - 0x08000000
        while data[a] != 0xFF:
            if data[a] == 0:
                data[a + 2] = 0
            a += 4
        open(rom, "wb").write(data)
        with open(BOOT) as f:
            self.run(rom, [ln.rstrip("\n") for ln in f], d)
        vals = self.run(rom, ["load " + os.path.join(d, "boot.ss"), "wait 60", "shot townsfolk.png"] +
                        ["r32 %#x" % (UNIT_GREEN + i * 0x48) for i in range(8)] +
                        ["r8 %#x" % (UNIT_GREEN + i * 0x48 + k) for i in range(8) for k in (0x10, 0x11)], d)
        self.shot(os.path.join(d, "townsfolk.png"), "townsfolk on their spots")
        if len(vals) != 24:
            return self.check("life: townsfolk's places", False, "values %s" % vals)
        rom_data = open(ROM, "rb").read()
        got = {}
        for i in range(8):
            ptr = vals[i]
            if ptr >= 0x08000000:
                got[rom_data[ptr - 0x08000000 + 4]] = (vals[8 + 2 * i], vals[9 + 2 * i])
        bad = {"%#x" % p: (got.get(p), xy) for p, xy in want.items() if got.get(p) != xy}
        self.check("life: %d townsfolk stand on their spots" % len(want), want and not bad,
                   "wrong: %s" % bad if bad else ", ".join("%#x at %s" % (p, xy) for p, xy in want.items()))

    def guild_quests(self):
        """gGuildQuests from the ROM: (district, kind, param, msg) per row."""
        rom = open(ROM, "rb").read()
        a, out = self.sym["gGuildQuests"] - 0x08000000, []
        while rom[a + 1]:
            district, kind, param, _item, _target, _tok, _gold, msg = struct.unpack_from("<BBBBHHHH", rom, a)
            out.append((district, kind, param, msg))
            a += 12
        return out

    def test_unlocks(self, states):
        """Serafew's board: with its earlier quests claimed, the story quests show "+story" and "+support"; claiming the
        Eirika and Seth support quest (60 tokens earned) adds 40 support points both ways (their battle records)."""
        d = os.path.join(self.out, "Serafew")
        need = ("gGuildSave", "gGuildQuests", "gPidStatsData")
        if "guild" not in states or not all(n in self.sym for n in need):
            return self.check("unlocks: support and story quests", False, "no guild board or symbols")
        quests = self.guild_quests()
        support = next((i for i, q in enumerate(quests) if q[0] == 0x3B and q[1] == GUILD_QUEST_TOKENS_EARNED), None)
        if support is None:
            return self.check("unlocks: Serafew's support quest", False, "not in gGuildQuests")
        # everything on Serafew's board before the story quests is claimed, so they and the support quest show
        first = next(i for i, q in enumerate(quests) if q[1] == GUILD_QUEST_WINS and q[2] != 0)
        earlier = [i for i, q in enumerate(quests[:first]) if q[0] in (0, 0x3B)]
        shown = [i for i, q in enumerate(quests) if q[0] in (0, 0x3B) and i not in earlier][:3]
        claimed = sum(1 << i for i in earlier)
        g, bwl = self.sym["gGuildSave"], self.sym["gPidStatsData"]
        sup = lambda pid: [bwl + (pid - 1) * BWL_SIZE + 4 + k for k in range(7)]
        reads = ["r8 %#x" % a for a in sup(1) + sup(2)]
        lines = ["load " + states["guild"][0], "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2), "w16 %#x 60" % (g + 4),
                 "w32 %#x %#x" % (g + 8, claimed), "w8 %#x 0" % self.sym["gWmStoryChapter"]] + reads
        lines += ["hold A 6", "wait 150", "shot unlocks.png"]
        lines += ["hold DOWN 4", "wait 12"] * shown.index(support) + ["press A", "wait 30"] + reads
        vals = self.run(states["rom"], lines, d)
        self.shot(os.path.join(d, "unlocks.png"), "story quests")
        if len(vals) != 28:
            return self.check("unlocks: the support quest", False, "values %s" % vals)
        gain = [b - a for a, b in zip(vals[:14], vals[14:])]
        self.check("unlocks: the support quest adds 40 points each way", sorted(gain) == [0] * 12 + [40, 40],
                   "Eirika %s, Seth %s" % (gain[:7], gain[7:]))

    def test_snek(self, states):
        """Port Kiris: Eirika talks to the Pied Piper (0x7E, next to the guild board), says Yes, scores 80 and quits
        with B: the game pays 10 tokens, sets the high score and plays the new-record conversation."""
        d = os.path.join(self.out, "PortKiris")
        need = ("gGuildSave", "gSnekCurrentScore", "gSnekHighScore")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("snek: the Pied Piper's game", False, "no Port Kiris map or symbols")
        layout = json.load(open(os.path.join(ROOT, "CustomCampaign", "Chapters", "PortKiris", "map",
                                             "PortKiris_layout.json")))
        cfg = open(os.path.join(ROOT, "CustomCampaign", "Chapters", "_hexshop", "town_cfg.bin"), "rb").read()
        traps = open(os.path.join(ROOT, "CustomCampaign", "Chapters", "PortKiris", "events", "traps.c")).read()
        avoid = {(int(m.group(2)), int(m.group(3))) for m in re.finditer(r"(\w+)\((\d+),\s*(\d+)", traps)}
        at = self.walk(states["rom"], os.path.join(d, "start.ss"), SNEK_TALK_TILE, layout, cfg,
                       avoid | {SNEK_PIPER_TILE}, d)
        if not self.check("snek: Eirika reaches the Pied Piper", at):
            return
        g, sc, hi = self.sym["gGuildSave"], self.sym["gSnekCurrentScore"], self.sym["gSnekHighScore"]
        # RIGHT faces him, A talks; A through his lines and Yes; in the game the score is set to 80, then B quits
        lines = ["load " + at, "wait 40", "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2), "w16 %#x 0" % hi,
                 "hold RIGHT 4", "wait 20", "hold A 6", "wait 90"]
        lines += ["hold A 6", "wait 60"] * 4 + ["hold A 6", "wait 90", "hold A 6", "wait 200", "wait 100",
                                                "w16 %#x 80" % sc, "wait 30", "hold B 6", "wait 240"]
        lines += ["hold A 6", "wait 100"] * 2 + ["shot snek.png", "r16 %#x" % (g + 2), "r16 %#x" % hi]
        vals = self.run(states["rom"], lines, d)
        self.shot(os.path.join(d, "snek.png"), "snek result")
        self.check("snek: 80 points pay 10 tokens and set the high score", vals == [10, 80],
                   "tokens, high score %s" % vals)

    def prizes(self):
        """gGuildPrizes from the ROM: (item, cost, kind, once) per row."""
        rom = open(ROM, "rb").read()
        a, out = self.sym["gGuildPrizes"] - 0x08000000, []
        while struct.unpack_from("<H", rom, a)[0]:
            item, cost, kind, once = struct.unpack_from("<HHBB", rom, a)
            out.append((item, cost, kind, once))
            a += 8
        return out

    def test_prizes(self, states):
        """Buys the Omni Tonic and the first skill scroll on Serafew's board: the tokens drop by their costs and both
        items reach Eirika."""
        d = os.path.join(self.out, "Serafew")
        if "guild" not in states or "gGuildPrizes" not in self.sym or "gGuildSave" not in self.sym:
            return self.check("prizes: a tonic and a skill scroll", False, "no guild board or symbols")
        rows = self.prizes()
        tonic = next((i for i, r in enumerate(rows) if r[2] == GUILD_PRIZE_WORD and r[0] == (8 << 8) | ITEM_TONIC), None)
        scroll = next((i for i, r in enumerate(rows) if r[2] == GUILD_PRIZE_SKILL), None)
        if tonic is None or scroll is None:
            return self.check("prizes: a tonic and a skill scroll", False, "Omni Tonic %s, scroll %s" % (tonic, scroll))
        g, items = self.sym["gGuildSave"], UNIT1 + 0x1E
        # A opens the board, R the Prizes page; the hand starts on the first prize
        lines = ["load " + states["guild"][0], "w16 %#x 0x4447" % g, "w16 %#x 500" % (g + 2), "w16 %#x 0" % (g + 6),
                 "hold A 6", "wait 150", "press R", "wait 40"]
        lines += ["hold DOWN 4", "wait 12"] * tonic + ["press A", "wait 20", "press A", "wait 40"]
        lines += ["hold DOWN 4", "wait 12"] * (scroll - tonic) + ["press A", "wait 20", "press A", "wait 40",
                                                                  "shot prizes.png", "r16 %#x" % (g + 2)]
        lines += ["r16 %#x" % (items + 2 * k) for k in range(5)]
        vals = self.run(states["rom"], lines, d)
        self.shot(os.path.join(d, "prizes.png"), "prizes bought")
        if len(vals) != 6:
            return self.check("prizes: a tonic and a skill scroll", False, "values %s" % vals)
        cost = rows[tonic][1] + rows[scroll][1]
        self.check("prizes: the Omni Tonic and a skill scroll cost %d tokens" % cost, vals[0] == 500 - cost,
                   "tokens 500 to %d" % vals[0])
        got = vals[1:]
        has_tonic = (8 << 8) | ITEM_TONIC in got
        has_scroll = any(w & 0xFF in SKILL_SCROLL_ITEMS and w >> 8 for w in got)
        self.check("prizes: both reach Eirika's items", has_tonic and has_scroll,
                   "items " + " ".join("%04x" % w for w in got))

    # ---- saves ----

    def boot_lines(self):
        """The boot script without its screenshot and save state: SRAM is not in a save state, so a test that resets
        the game boots in the same run."""
        with open(BOOT) as f:
            return [ln.rstrip("\n") for ln in f if not ln.startswith(("shot", "save"))]

    def chunks(self, table):
        """(size, save function address) of each chunk in a ROM chunk table."""
        rom = open(ROM, "rb").read()
        a, out = self.sym[table] - 0x08000000, []
        while struct.unpack_from("<H", rom, a)[0] != 0xFFFF:
            out.append((struct.unpack_from("<H", rom, a + 2)[0], struct.unpack_from("<I", rom, a + 4)[0]))
            a += 16
        return out

    def test_savelayout(self):
        need = ("gEmsSavChunks", "gEmsSusChunks", "gEmsSizes", "gEmsOffsets")
        if not all(n in self.sym for n in need):
            return self.check("savelayout: the chunk tables are in the ROM", False, "no symbols")
        rom = open(ROM, "rb").read()
        sav_size, sus_size = struct.unpack_from("<HH", rom, self.sym["gEmsSizes"] - 0x08000000)
        offs = struct.unpack_from("<5H", rom, self.sym["gEmsOffsets"] - 0x08000000)
        for name, table, size in (("game save", "gEmsSavChunks", sav_size), ("suspend save", "gEmsSusChunks", sus_size)):
            used = sum(c[0] for c in self.chunks(table))
            self.check("savelayout: the %s's chunks fit its block" % name, used <= size,
                       "%d of %d bytes, %d free" % (used, size, size - used))
        # the suspend block, then the three game slots, end to end below the magic word and the end of SRAM
        blocks = [(offs[3], sus_size), (offs[0], sav_size), (offs[1], sav_size), (offs[2], sav_size)]
        ok = all(a + n <= b for (a, n), (b, _) in zip(blocks, blocks[1:]))
        end = blocks[-1][0] + sav_size
        magic = struct.unpack_from("<H", rom, self.sym["gSkillSysMagicOffset"] - 0x08000000)[0] \
            if "gSkillSysMagicOffset" in self.sym else end
        self.check("savelayout: the blocks do not overlap", ok and end <= magic <= 0x8000,
                   "slots end at %#x, magic at %#x" % (end, magic))

    def save_regions(self, table):
        """(RAM symbol, address, size) of each district chunk in a chunk table."""
        names = {self.sym[f] & ~1: f for f in SAVE_RAM if f in self.sym}
        out = []
        for size, save in self.chunks(table):
            f = names.get(save & ~1)
            if f and SAVE_RAM[f] in self.sym:
                ram = SAVE_RAM[f]
                out.append((ram, self.sym[ram], 2 if ram == "gSnekHighScore" else size))
        return out

    def round_trip(self, name, table, save_lines, restore_lines, extra_reads=(), extra_writes=()):
        """Boots, writes the sentinels, dumps the district chunks, saves, wipes them, resets, restores and dumps them
        again. Returns (the chunks, those that came back different, the values of extra_reads after the restore)."""
        d = os.path.join(self.out, name)
        os.makedirs(d, exist_ok=True)
        regions = self.save_regions(table)
        lines = self.boot_lines()
        for ram, off, bits, val in SENTINELS:
            if ram in self.sym:
                lines.append("w%d %#x %d" % (bits, self.sym[ram] + off, val))
        lines += list(extra_writes)
        lines += ["dump %#x %d pre_%s.bin" % (a, n, ram) for ram, a, n in regions]
        lines += save_lines
        lines += ["w8 %#x 0" % (a + k) for _, a, n in regions for k in range(n)]
        lines += SOFT_RESET + restore_lines + ["shot restored.png"]
        lines += ["dump %#x %d post_%s.bin" % (a, n, ram) for ram, a, n in regions]
        vals = self.run(ROM, lines + list(extra_reads), d)
        self.shot(os.path.join(d, "restored.png"), name + " restored")
        bad = []
        for ram, a, n in regions:
            pre, post = (os.path.join(d, "%s_%s.bin" % (k, ram)) for k in ("pre", "post"))
            if not (os.path.exists(pre) and os.path.exists(post)) or open(pre, "rb").read() != open(post, "rb").read():
                bad.append(ram)
        return regions, bad, vals

    def test_suspend(self):
        # Eirika's HP drops to 9 before she waits; the wait writes the suspend save
        hp = UNIT1 + 0x13
        regions, bad, vals = self.round_trip(
            "suspend", "gEmsSusChunks",
            EIRIKA_MENU + ["press UP", "wait 10", "press A", "wait 90", "w8 %#x 1" % hp],
            ["press A", "wait 200", "press A", "wait 200"],
            ["r8 %#x" % hp, "r8 %#x" % UNIT_X, "r8 %#x" % UNIT_Y, "r8 %#x" % CHAPTER], ["w8 %#x 9" % hp])
        if not self.check("suspend: Resume Chapter reloads the map", len(vals) == 4 and vals[3] == 0,
                          "chapter %s" % (vals[3:] or "?")):
            return
        self.check("suspend: Eirika's HP and position come back", vals[:3] == [9, 4, 5],
                   "HP %d at (%d, %d)" % tuple(vals[:3]))
        self.check("suspend: the district chunks come back (%d)" % len(regions), regions and not bad,
                   "different: " + ", ".join(bad) if bad else ", ".join(r[0] for r in regions))

    def test_gamesave(self):
        # the chunks are wiped before the reset, so only a loaded save brings them back
        regions, bad, _ = self.round_trip("gamesave", "gEmsSavChunks", DEBUG_SAVE,
                                          ["press A", "wait 200", "press A", "wait 300"])
        self.check("gamesave: the district chunks come back (%d)" % len(regions), regions and not bad,
                   "different: " + ", ".join(bad) if bad else ", ".join(r[0] for r in regions))

    # ---- battle ----

    def skill_id(self, name):
        m = re.search(r"#define %s (0x[0-9A-Fa-f]+|\d+)" % name,
                      open(os.path.join(ROOT, "include", "constants", "skills.h")).read())
        return int(m.group(1), 0) if m else None

    def test_battle(self, boot_ss):
        d = os.path.join(self.out, "battle")
        os.makedirs(d, exist_ok=True)
        skills = UNIT1 + 0x32
        # Attack, the first weapon: the forecast of the first target
        forecast = EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "shot forecast.png"]
        vals = self.run(ROM, ["load " + boot_ss] + ["r8 %#x" % (skills + k) for k in range(5)] + forecast +
                        ["r16 %#x" % (BATTLE_ACTOR + 0x5A)], d)
        self.shot(os.path.join(d, "forecast.png"), "battle forecast")
        if not self.check("battle: the forecast opens", len(vals) == 6 and vals[5] > 0,
                          "attack %s" % (vals[5:] or "?")):
            return
        base = vals[5]
        # the five skill slots are 10 bits each in unit+0x32..0x36: Str Bonus goes in the first empty one
        packed = sum(v << (8 * k) for k, v in enumerate(vals[:5]))
        sid = self.skill_id("SID_StrBonus")
        slot = next((k for k in range(4) if (packed >> (10 * k)) & 0x3FF == 0), None)
        writes = []
        if sid is not None and slot is not None:
            packed |= sid << (10 * slot)
            writes = ["w8 %#x %#x" % (skills + k, (packed >> (8 * k)) & 0xFF) for k in range(5)]
        lvl = UNIT1 + 0x08
        quote_ss = os.path.join(d, "quote.ss")
        vals = self.run(ROM, ["load " + boot_ss, "r8 %#x" % lvl, "w8 %#x 99" % (UNIT1 + 0x09)] + writes + forecast +
                        ["r16 %#x" % (BATTLE_ACTOR + 0x5A), "r32 %#x" % BATTLE_TARGET, "press A", "wait 460",
                         "shot levelup.png", "wait 150", "shot quote.png", "save " + quote_ss, "r32 %#x" % GFACES,
                         "r8 %#x" % (EVENT_SLOTS + 9 * 4), "wait 300", "shot after.png",
                         "r8 %#x" % (BATTLE_TARGET + 0x13), "r8 %#x" % (BATTLE_TARGET + 0x72), "r8 %#x" % lvl], d)
        self.shot(os.path.join(d, "levelup.png"), "level-up")
        self.shot(os.path.join(d, "quote.png"), "level-up quote")
        if len(vals) != 8:
            return self.check("battle: the fight runs", False, "values %s" % vals)
        lv0, atk, target, face, gains, hp, hp0, lv1 = vals
        if writes:
            self.check("battle: Str Bonus in a skill slot raises the attack", atk > base,
                       "attack %d without, %d with" % (base, atk))
        else:
            self.check("battle: a free skill slot for Str Bonus", False, "SID %s, slot %s" % (sid, slot))
        self.check("battle: the attack hurts an enemy", target != 0 and hp < hp0, "HP %d to %d" % (hp0, hp))
        self.check("battle: 99 EXP levels Eirika up", lv1 == lv0 + 1, "level %d to %d" % (lv0, lv1))
        self.check_quote_face(face, gains, quote_ss, d)

    def test_battlequote(self, boot_ss):
        """Moves O'Neill (the first enemy) next to Eirika and attacks him: his battle quote has Eirika answer with
        [LoadFace_Eirika][Surprised]. Eirika has no surprised portrait, so a test ROM maps her sad row to SURPRISED;
        the tag on a face that has just loaded must show it."""
        d = os.path.join(self.out, "battle")
        os.makedirs(d, exist_ok=True)
        if "gPortraitExpressions" not in self.sym or not self.expression(EIRIKA_PORTRAIT, EXPRESSION_SAD):
            return self.check("battlequote: Eirika has a sad portrait for the test", False)
        data = bytearray(open(ROM, "rb").read())
        a = self.sym["gPortraitExpressions"] - 0x08000000
        while data[a]:
            if data[a] == EIRIKA_PORTRAIT and data[a + 1] == EXPRESSION_SAD:
                data[a + 1], want = EXPRESSION_SURPRISED, data[a + 2]
            a += 4
        rom = os.path.join(d, "quote.gba")
        open(rom, "wb").write(data)
        enemy = 0x0202CFBC
        vals = self.run(rom, ["load " + boot_ss, "r32 %#x" % BM_MAP_UNIT, "r8 %#x" % (enemy + 0x10),
                              "r8 %#x" % (enemy + 0x11)], d)
        rows, ex, ey = vals
        row = lambda y: self.run(rom, ["load " + boot_ss, "r32 %#x" % (rows + 4 * y)], d)[0]
        # the tile below Eirika, (4, 6); the unit map row pointers come from gBmMapUnit
        moves = ["w8 %#x 0" % (row(ey) + ex), "w8 %#x 0x81" % (row(6) + 4), "w8 %#x 4" % (enemy + 0x10),
                 "w8 %#x 6" % (enemy + 0x11)]
        # Attack, the first weapon, DOWN to the second target (O'Neill), A; A through his two lines to Eirika's
        lines = ["load " + boot_ss] + moves + EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press DOWN",
                                                           "wait 30", "press A", "wait 60"]
        lines += ["wait 50", "press A", "wait 40"] * 5 + ["shot battlequote.png", "save battlequote.ss",
                                                          "r32 %#x" % GFACES]
        face = self.run(rom, lines, d)
        self.shot(os.path.join(d, "battlequote.png"), "battle quote")
        got = self.run(rom, ["load " + os.path.join(d, "battlequote.ss"), "r16 %#x" % (face[0] + 0x3E)], d) \
            if face and face[0] else []
        self.check("battlequote: [Surprised] after Eirika's face loads shows it", got == [want],
                   "portrait %s, want %#x" % (", ".join("%#x" % v for v in got) or "none", want))

    def test_awards(self, boot_ss):
        """The prologue with only O'Neill left (the other two enemies removed), moved next to Eirika: she attacks him,
        he falls to her counter in the enemy phase and the chapter ends. The awards screen comes before the save menu, names Eirika for all three
        awards (the only unit that fought: one foe defeated, no damage taken) and pays 10 + 5 + 5 tokens."""
        d = os.path.join(self.out, "awards")
        os.makedirs(d, exist_ok=True)
        need = ("gGuildSave", "gChapterAwards")
        if not all(n in self.sym for n in need):
            return self.check("awards: the chapter awards", False, "no symbols")
        g, ca, enemy = self.sym["gGuildSave"], self.sym["gChapterAwards"], 0x0202CFBC
        vals = self.run(ROM, ["load " + boot_ss, "r32 %#x" % BM_MAP_UNIT] +
                        ["r8 %#x" % (enemy + i * 0x48 + k) for i in range(3) for k in (0x10, 0x11)], d)
        if len(vals) != 7:
            return self.check("awards: the prologue's enemies", False, "values %s" % vals)
        rows, pos = vals[0], [(vals[1 + 2 * i], vals[2 + 2 * i]) for i in range(3)]
        tile = lambda x, y: self.run(ROM, ["load " + boot_ss, "r32 %#x" % (rows + 4 * y)], d)[0] + x
        # O'Neill (the first enemy) to the tile below Eirika, (4, 6), with no Spd, Def or Lck, and Eirika with 30 Str and
        # Skl; the others go
        lines = ["load " + boot_ss, "w8 %#x 0" % tile(*pos[0]), "w8 %#x 0x81" % tile(4, 6), "w8 %#x 4" % (enemy + 0x10),
                 "w8 %#x 6" % (enemy + 0x11), "w8 %#x 0" % (enemy + 0x16), "w8 %#x 0" % (enemy + 0x17),
                 "w8 %#x 0" % (enemy + 0x19), "w8 %#x 30" % (UNIT1 + 0x14), "w8 %#x 30" % (UNIT1 + 0x15),
                 "w16 %#x 0x4447" % g, "w16 %#x 0" % (g + 2)]
        for i in (1, 2):
            lines += ["w32 %#x 0" % (enemy + i * 0x48), "w8 %#x 0" % tile(*pos[i])]
        # Attack, the first weapon, the first target; A through his quote; START and A skip the ending scenes
        lines += EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press A", "wait 60"]
        lines += ["wait 50", "press A", "wait 40"] * 5 + ["wait 300"]
        # he survives her attack and falls to her counter in the enemy phase
        skip = ["press START", "wait 50", "press A", "wait 50"]
        lines += skip * 3 + ["r8 %#x" % (ca + 3), "r8 %#x" % (ca + 5), "r8 %#x" % (ca + 6)]
        lines += skip * 3 + ["wait 200", "shot awards.png", "r16 %#x" % (g + 2), "r16 %#x" % ca]
        lines += ["press A", "wait 150", "shot savemenu.png"]
        vals = self.run(ROM, lines, d)
        self.shot(os.path.join(d, "awards.png"), "chapter awards")
        self.shot(os.path.join(d, "savemenu.png"), "save menu after the awards")
        if len(vals) != 5:
            return self.check("awards: the chapter ends", False, "values %s" % vals)
        count, kills, battles, tokens, magic = vals
        self.check("awards: Eirika's battles are counted (1 unit, 1 foe defeated)",
                   count == 1 and kills == 1 and battles >= 1, "units, foes, battles %s" % vals[:3])
        self.check("awards: the chapter end pays 10 + 5 + 5 tokens", tokens == 20 and magic == 0,
                   "tokens %d, counts %s" % (tokens, "cleared" if magic == 0 else "left"))

    def test_frames(self, boot_ss):
        """The battlequote scene with Eirika's [Surprised] drawn by a frame set: a test ROM points her Determined
        frame set (gPortraitFrames) at SURPRISED (she has no surprised portrait). The face keeps her base portrait,
        the set is on its slot, and the face's graphics have the set's open eyes where the face image shows them."""
        d = os.path.join(self.out, "battle")
        os.makedirs(d, exist_ok=True)
        need = ("gPortraitFrames", "gPortraitFrameSlots")
        if not all(n in self.sym for n in need):
            return self.check("frames: a frame set", False, "no symbols")
        data = bytearray(open(ROM, "rb").read())
        a = self.sym["gPortraitFrames"] - 0x08000000
        while data[a] and not (data[a] == EIRIKA_PORTRAIT and data[a + 1] == EXPRESSION_DETERMINED):
            a += 8
        if not data[a]:
            return self.check("frames: Eirika has a frame set", False)
        data[a + 1] = EXPRESSION_SURPRISED
        frames = struct.unpack_from("<I", data, a + 4)[0] - 0x08000000
        rom = os.path.join(d, "frames.gba")
        open(rom, "wb").write(data)
        enemy = 0x0202CFBC
        rows, ex, ey = self.run(rom, ["load " + boot_ss, "r32 %#x" % BM_MAP_UNIT, "r8 %#x" % (enemy + 0x10),
                                      "r8 %#x" % (enemy + 0x11)], d)
        tile_row = lambda y: self.run(rom, ["load " + boot_ss, "r32 %#x" % (rows + 4 * y)], d)[0]
        moves = ["w8 %#x 0" % (tile_row(ey) + ex), "w8 %#x 0x81" % (tile_row(6) + 4), "w8 %#x 4" % (enemy + 0x10),
                 "w8 %#x 6" % (enemy + 0x11)]
        lines = ["load " + boot_ss] + moves + EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press DOWN",
                                                           "wait 30", "press A", "wait 60"]
        lines += ["wait 50", "press A", "wait 40"] * 5 + ["shot frames.png", "save frames.ss", "r32 %#x" % GFACES]
        face = self.run(rom, lines, d)
        self.shot(os.path.join(d, "frames.png"), "Eirika's frame set")
        if not face or not face[0]:
            return self.check("frames: Eirika's face is up", False)
        slots = self.sym["gPortraitFrameSlots"]
        vals = self.run(rom, ["load " + os.path.join(d, "frames.ss"), "r16 %#x" % (face[0] + 0x3E),
                              "r16 %#x" % (face[0] + 0x3C), "r32 %#x" % slots, "r8 %#x" % (slots + 0x10)], d)
        if len(vals) != 4:
            return self.check("frames: the face's state", False, "values %s" % vals)
        fid, oam2, set_ptr, applied = vals
        self.check("frames: [Surprised] keeps the base portrait and puts the set on the slot",
                   fid == EIRIKA_PORTRAIT and set_ptr == a + 0x08000000 and applied == 1,
                   "portrait %#x, set %#x (want %#x), applied %d" % (fid, set_ptr, a + 0x08000000, applied))
        # Eirika's eyes are at tile (3, 4): sheet tile (3, 4) is face image tile 9 (row 0, column 8 + 1)
        vram = ((oam2 + 9) & 0x3FF) * 0x20 + 0x06010000
        got = self.run(rom, ["load " + os.path.join(d, "frames.ss")] + ["r32 %#x" % (vram + 4 * k) for k in range(8)], d)
        want = list(struct.unpack_from("<8I", data, frames))
        self.check("frames: the face's graphics show the set's open eyes", got == want,
                   "first eye tile %s" % ("matches" if got == want else "differs"))

    def test_stealer(self, boot_ss):
        """Eirika holds the Skill Stealer; O'Neill (a boss) has Str Bonus in his first skill slot and 1 HP, and she
        defeats him: she gets Str Bonus's scroll, not the next skill's."""
        d = os.path.join(self.out, "battle")
        os.makedirs(d, exist_ok=True)
        sid = self.skill_id("SID_StrBonus")
        if sid is None:
            return self.check("stealer: Str Bonus exists", False)
        enemy, skills = 0x0202CFBC, 0x0202CFBC + 0x32
        vals = self.run(ROM, ["load " + boot_ss, "r32 %#x" % BM_MAP_UNIT, "r8 %#x" % (enemy + 0x10),
                              "r8 %#x" % (enemy + 0x11)] + ["r8 %#x" % (skills + k) for k in range(5)] +
                        ["r16 %#x" % (UNIT1 + 0x1E + 2 * k) for k in range(5)], d)
        rows, ex, ey = vals[:3]
        packed = sum(v << (8 * k) for k, v in enumerate(vals[3:8]))
        packed = (packed & ~0x3FF) | sid
        free = next((k for k in range(5) if vals[8 + k] == 0), None)
        if free is None:
            return self.check("stealer: Eirika has a free item slot", False)
        tile_row = lambda y: self.run(ROM, ["load " + boot_ss, "r32 %#x" % (rows + 4 * y)], d)[0]
        lines = ["load " + boot_ss, "w8 %#x 0" % (tile_row(ey) + ex), "w8 %#x 0x81" % (tile_row(6) + 4),
                 "w8 %#x 4" % (enemy + 0x10), "w8 %#x 6" % (enemy + 0x11), "w8 %#x 1" % (enemy + 0x13),
                 "w8 %#x 0" % (enemy + 0x16), "w8 %#x 0" % (enemy + 0x19), "w8 %#x 30" % (UNIT1 + 0x15),
                 "w16 %#x %#x" % (UNIT1 + 0x1E + 2 * free, ITEM_SKILL_STEALER)]
        lines += ["w8 %#x %#x" % (skills + k, (packed >> (8 * k)) & 0xFF) for k in range(5)]
        # Attack, the first weapon, O'Neill (the second target); A through his quote and the scroll's popup
        lines += EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press DOWN", "wait 30", "press A",
                                "wait 60"]
        lines += ["wait 50", "press A", "wait 40"] * 5 + ["press A", "wait 60"] * 16 + ["wait 60", "shot stealer.png"]
        lines += ["press A", "wait 60"] * 4 + ["r16 %#x" % (UNIT1 + 0x1E + 2 * k) for k in range(5)]
        got = self.run(ROM, lines, d)
        self.shot(os.path.join(d, "stealer.png"), "Skill Stealer")
        want = self.scroll_item(sid)
        self.check("stealer: defeating a boss gives his first skill's scroll", want in got,
                   "items %s, want %#x (Str Bonus %#x)" % (", ".join("%#x" % v for v in got), want, sid))

    @staticmethod
    def scroll_item(sid):
        """MakeSkillScrollItem (SkillScroll.c) with CONFIG_TURN_ON_ALL_SKILLS"""
        if sid >= 0x2FF:
            return ((sid - 0x2FF) << 8) | SKILL_SCROLL_ITEMS[3]
        if sid >= 0x1FF:
            return ((sid - 0x1FF) << 8) | SKILL_SCROLL_ITEMS[2]
        if sid > 0xFF:
            return ((sid - 0xFF) << 8) | SKILL_SCROLL_ITEMS[1]
        return (sid << 8) | SKILL_SCROLL_ITEMS[0]

    def test_debugflag(self, boot_ss):
        """The debugger's off switch is flag 0xEF, apart from the base-chapter intro skip (0xEC, set by Enter District):
        with 0xEC set the debugger still opens; with 0xEF set it does not."""
        d = os.path.join(self.out, "boot")
        os.makedirs(d, exist_ok=True)

        def flag(f):
            return PERMANENT_FLAGS + ((f - 0x65) >> 3), 1 << ((f - 0x65) & 7)
        opens = DEBUG_SAVE[:DEBUG_SAVE.index("press A") + 2]
        shots = {}
        for name, f in (("none", None), ("intro_skip", 0xEC), ("debugger_off", 0xEF)):
            writes = ["w8 %#x %#x" % flag(f)] if f else []
            self.run(ROM, ["load " + boot_ss, "wait 10", "shot map.png"] + writes + opens + ["shot dbg_%s.png" % name], d)
            shots[name] = os.path.join(d, "dbg_%s.png" % name)
        self.shot(shots["intro_skip"], "debugger with the intro skip flag")
        m = os.path.join(d, "map.png")
        self.check("debugflag: the debugger opens with the intro skip flag (0xEC) set",
                   self.diff(m, shots["intro_skip"]) > 25 and self.same_screen(shots["none"], shots["intro_skip"]),
                   "change %.0f" % self.diff(m, shots["intro_skip"]))
        self.check("debugflag: flag 0xEF turns the debugger off",
                   not self.same_screen(shots["none"], shots["debugger_off"]),
                   "change from the debugger %.0f" % self.diff(shots["none"], shots["debugger_off"]))

    def expression(self, base, expression):
        """The portrait gPortraitExpressions gives a base portrait for an expression, or None."""
        if "gPortraitExpressions" not in self.sym:
            return None
        rom = open(ROM, "rb").read()
        a = self.sym["gPortraitExpressions"] - 0x08000000
        while rom[a]:
            if rom[a] == base and rom[a + 1] == expression:
                return rom[a + 2]
            a += 4
        return None

    def check_quote_face(self, face, gains, quote_ss, d):
        """The level-up quotes open with [Sad] for a poor level-up (0-2 stat points, or 9+) and [Happy] for a great one
        (6-8): the level-up portrait shows that expression of Eirika's portrait if the table has one."""
        if not face:
            return self.check("battle: the level-up quote's portrait", False, "no face")
        got = self.run(ROM, ["load " + quote_ss, "r16 %#x" % (face + 0x3E)], d)
        tier = 1 if 3 <= gains <= 5 else 2 if 6 <= gains <= 8 else 0
        base = EIRIKA_PORTRAIT
        want = {0: self.expression(base, EXPRESSION_SAD), 2: self.expression(base, EXPRESSION_HAPPY)}.get(tier) or base
        self.check("battle: the level-up quote shows its expression", got == [want],
                   "%d stat points: portrait %s, want %#x" % (gains, ", ".join("%#x" % v for v in got), want))

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
    ap.add_argument("--quick", action="store_true", help="every test but the districts other than Serafew")
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
    if want and "snek" in want and "PortKiris" not in names:
        names.append("PortKiris")

    def on(n):
        return want is None or n in want

    if on("boot") or on("statscreen") or on("battle") or on("battlequote") or on("awards") or on("frames") or on("stealer") or \
            on("debugflag"):
        boot_ss = s.test_boot()
        if boot_ss and on("statscreen"):
            s.test_statscreen(boot_ss)
        if boot_ss and on("battle"):
            s.test_battle(boot_ss)
        if boot_ss and on("battlequote"):
            s.test_battlequote(boot_ss)
        if boot_ss and on("debugflag"):
            s.test_debugflag(boot_ss)
        if boot_ss and on("stealer"):
            s.test_stealer(boot_ss)
        if boot_ss and on("frames"):
            s.test_frames(boot_ss)
        if boot_ss and on("awards"):
            s.test_awards(boot_ss)
    if on("savelayout"):
        s.test_savelayout()
    if on("suspend"):
        s.test_suspend()
    if on("gamesave"):
        s.test_gamesave()
    for n in names:
        if on(n) or (n == "Serafew" and (on("tokens") or on("claim") or on("prizes") or on("unlocks") or on("featured") or on("life"))) or \
                (n == "PortKiris" and on("snek")):
            states = s.test_district(n)
            if n == "Serafew" and on("tokens"):
                s.test_tokens(states)
            if n == "Serafew" and on("claim"):
                s.test_claim(states)
            if n == "Serafew" and on("prizes"):
                s.test_prizes(states)
            if n == "Serafew" and on("unlocks"):
                s.test_unlocks(states)
            if n == "Serafew" and on("featured"):
                s.test_featured(states)
            if n == "Serafew" and on("life"):
                s.test_life(states)
            if n == "PortKiris" and on("snek"):
                s.test_snek(states)
    sys.exit(s.report())


if __name__ == "__main__":
    main()
