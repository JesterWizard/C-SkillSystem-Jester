#!/usr/bin/env python3
"""Emulator smoke tests for the built ROM (fe8-kernel-dev.gba), run with gbarun (Tools/cloud/README.md).

    python3 Tools/cloud/smoketest.py [--quick] [--jobs N] [--out DIR] [--only NAME ...]

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
  worldmap    (Serafew) the world map, reached through the district's gate: the visit keeps the story chapter;
              Enter District loads Serafew from a plain node and from the story chapter's node; locked, the row is
              gone; an unlocked district's node has the green flag (map and minimap); travelling to the story node starts Chapter 6.
  stubs       (Serafew) the stub story chapters: from Chapter 10 the lord
              travels to each story node, each stub's battle loads its enemies and is won (the enemies taken off and the
              defeat-all flag set, then Eirika waits), and the story moves on, through Chapter 20, then 21, 22 and the
              ending. A monster on Chapter 10's node, not marked next, does not stop it.
  wmshops     (Serafew) the node menu's shop rows: Serafew's Armory and Shop rows open its armory and vendor with
              the node's stock, and B leaves them; a node with no shops has none; the road to the story node is open.
  postgame    (Serafew) from Chapter 20's world map through the ending, which goes on into the postgame: the
              dungeons are shown, Black Temple starts nothing, Rausten's secret shop opens, the Lagdou Ruins and the
              Tower of Valni start, Enter District works on Port Kiris. The end save is titled Creature Campaign,
              and Restart Chapter on the save opens the postgame.
  districtresume (Serafew) Resume Chapter in a suspended district comes back in free movement.
  savetitle   (Serafew) the save made on leaving a district is titled with the story chapter, not "Skirmish Map".
  wmsave      (Serafew) the world map's Save, a soft reset and Restart Chapter: the story chapter, the lord's node,
              the unlocked districts and the nodes' states come back, and Enter District still loads the district.
  life        (Serafew) district life: the map is darker at night (story Chapter 8); a Tower Stack best of 100 beats
              Marlo's first record and pays 10 tokens once; a test ROM puts the townsfolk on their spots.
  snek        (Port Kiris, not in --quick) the Pied Piper's Snek: Yes, 80 points, B: 10 tokens and the high score.
  savelayout  the game save's and the suspend save's chunks fit their SRAM blocks (read from the ROM's chunk tables).
  suspend     Eirika waits (the game writes its suspend save), a soft reset and Resume Chapter bring back her HP and
              position and every district save chunk (guild, River Crossing, Tower Stack, fishing, Hex, Snek).
  saveguard   a game save's block checksum carries the layout signature; set back to a pre-guard checksum, the save
              is refused (no Restart Chapter).
  gamesave    the debugger's "Save & restart ch." writes a game save; a soft reset and Restart Chapter bring back the
              same district chunks.
  gallery     the prologue's opening unlocks two CGs; Extras > Gallery shows its rows, opens an unlocked CG full
              size, goes on to the next with right, back to the grid with B, scrolls a row on screen, and back to
              Extras with B.
  bestiary    the prologue's first phase marks Eirika, Seth and O'Neill and their classes; a game save keeps them;
              after a reset Extras > Bestiary lists them as met (the others locked), A shows growths, R the classes,
              and B goes back to Extras.
  skillcodex  the prologue's first phase marks the skills of the units on the map; a game save keeps them; after a
              reset Extras > Skill Codex lists every skill, those met; R goes to the next met, B back to Extras.
  newgameplus Extras > New Game+ shows once a save has been to the postgame; it starts a new game in which Eirika
              has the skills she had equipped in that save and its skill list.
  battle      Eirika attacks the soldier next to her: the forecast's attack rises by the skill's bonus with Str Bonus
              in a free skill slot, the fight hurts the soldier, and with 99 EXP she levels up. Her level-up quote's
              portrait is the expression its tier asks for ([Sad] poor, [Happy] great) when the table has one.
  battlequote O'Neill moved next to Eirika: his battle quote's [LoadFace_Eirika][Surprised] shows her expression
              (a test ROM maps her sad portrait to SURPRISED). With the Mother 3 font, her box has 12-pixel lines
              and is shorter (the font's line height preset).
  lvupanim    with battle animations on, Eirika levels up: after the stats, her quote is printed in a box above the
              level-up window, the portrait shows its tier's expression, and the window slides off and the battle
              ends on the map with her new level.
  biography   the map menu's Biography row opens the list; it scrolls to its last biography, which A does not open (her
              character has not joined); A on Eirika plays her first page.
  debugflag   the debugger opens with the base-chapter intro skip flag (0xEC) set, and not with its own off flag
              (0xEF) set.
  stealer     Eirika holds the Skill Stealer and defeats O'Neill (a boss, 1 HP) with Str Bonus in his first skill
              slot: she gets Str Bonus's scroll.
  frames      the battlequote scene with Eirika's frame set pointed at [Surprised] (a test ROM): her base portrait
              stays, the set is on her face's slot and its open eyes are in the face's graphics.
  awards      the prologue's other two enemies removed, Eirika fights O'Neill until he falls and the chapter ends: off
              by default, no awards are paid; with chapter_awards on (a test ROM), the chapter awards screen (all three
              for Eirika) comes before the save menu and pays 10 + 5 + 5 tokens.
  bigunit     2x2 units: a test ROM lists O'Neill, who loads as four corners drawn once at double size; Eirika breaks
              a corner (no defeat yet). A fighter made a 2x2 block in RAM fights once from the corner that reaches,
              Seth breaks that corner, and the block stops where O'Neill blocks it; its shown range is every tile
              the block can cover.
  deeproots   Deep Roots (a menu skill): Eirika, given it in RAM, walks into the prologue's forest at (9, 6) and hops
              to another forest; she lands there unmoved, hops again, and Wait then ends her turn. A screenshot shows
              her menu on plains, with no Skills row.
  expbar      EXP bar styles (Kernel/Wizardry/ExpBarStyles): with the Options value set to the second and the last
              style, the map EXP bar and the battle-animation one load that style's palette and window tiles; the
              first value (Vanilla) keeps vanilla's palette in battle.
  mapcolors   Map Colors (Kernel/Wizardry/MapColors): Options set to Vivid, then Deep; leaving Options redraws the map,
              and all 160 map palette colours match the transform's model of vanilla's palette. A test ROM gives the
              prologue Chapter 18's palette animation: it runs after a normal boot, and Deep recolours its frames.
  ch1end      Chapter 1's end: the debugger starts Chapter 1, Eirika seizes the gate (the boss taken off it); the
              world map takes the ending's 0x38 as the Intermission (the story chapter), not a side map's return, and
              Chapter 2 follows, not the prologue.

--quick runs every test but the other three districts. --jobs N runs N tests side by side (default: the CPU count),
in separate emulator processes; the report keeps the same order. --only runs the named tests (boot, statscreen,
a district's name, tokens, claim, prizes, unlocks, featured, life, worldmap, stubs, wmshops, wmsave, postgame, savetitle, districtresume, snek, savelayout, suspend, gamesave, saveguard, battle, battlequote,
lvupanim, frames, stealer, debugflag, biography, awards, bigunit, expbar, deeproots, mapcolors, ch1end). Screenshots of every step and a contact sheet (sheet.png) go to --out (default: a new temp folder),
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
import threading
from concurrent.futures import ThreadPoolExecutor, wait
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
BIOGRAPHIES = 7  # gCharacterBiographies (Data/MapMenus/CharacterBiographies.c); Amelia is the last
GM_DATA = 0x03005280
WM_NODE_DATA = 0x082060B0  # gWMNodeData: 0x20 bytes a node, x and y at +0x18
# The stub story chapters, Chapter 10 to 20: (chapter, its node (gWmStoryChapterNodes), the node the lord starts the
# world map on before it (Stubs/events/worldmap.c)); the last chapter is 0x16
STUB_ROUTE = [(0x0B, 10, 9), (0x3D, 11, 10), (0x0C, 11, 11), (0x0D, 12, 11), (0x0E, 13, 12), (0x0F, 13, 13),
              (0x10, 20, 19), (0x11, 21, 19), (0x12, 22, 21), (0x13, 23, 22), (0x14, 25, 23)]
STUB_FINAL = 0x16
PLAY_FLAG_COMPLETE = 1 << 5
CLASS_REVENANT = 0x52  # gGMData (the world map's units, nodes and cursor)
CHAPTER_02, CHAPTER_05, CHAPTER_06 = 0x02, 0x06, 0x07  # include/jester_headers/maps.h
WM_NODE_SERAFEW, WM_NODE_ADLAS_PLAINS = 5, 6  # FE-CLib constants/worldmap.h
DISTRICT_SERAFEW_FLAG = 0x90  # GLOBAL_FLAG_DISTRICT_SERAFEW (include/jester_headers/custom-structs.h)
DISTRICT_PORT_KIRIS_FLAG = 0x91  # GLOBAL_FLAG_DISTRICT_PORT_KIRIS
WM_NODES = 0x1D  # the world map's nodes (NODE_MAX)
WM_PATH_DATA = 0x08206674  # gWMPathData: 12 bytes a path, its two nodes at +4
CHAPTER_07 = 0x08
PROC_SIZE, PROCS = 0x6C, 64  # sProcArray: 64 procs of 0x6C bytes
SRAM = 0x0E000000
CHAPTER_20 = 0x14
WM_NODE_PORT_KIRIS, WM_NODE_RAUSTEN, WM_NODE_BLACK_TEMPLE = 9, 23, 25
WM_NODE_VALNI, WM_NODE_LAGDOU, WM_NODE_MELKAEN = 26, 27, 28
POSTGAME_NODES = [WM_NODE_BLACK_TEMPLE, WM_NODE_VALNI, WM_NODE_LAGDOU, WM_NODE_MELKAEN]
CHAPTER_VALNI_1, CHAPTER_LAGDOU_1 = 0x24, 0x2E  # the nodes' chapters (gWMNodeData)
OPEN_ROADS = 31  # roads 0-30, the map as the story leaves it (road 9 is the Tower of Valni's)
PLAY_FLAG_POSTGAME = 1 << 2
OAM = 0x07000000
DISTRICT_FLAG_PAL, MINIMAP_FLAG_CHR = 8, 0xF7  # Kernel/Wizardry/EnterTown/DistrictMarker.c
UNIT_RED, RED_UNITS = 0x0202CFBC, 50  # the enemy units (0x81 on), 0x48 bytes each
STUB_ENEMY_READS = 8  # a stub loads at most 7 enemies
CHAPTER_FLAG_DEFEAT_ALL = 0x60  # gChapterFlagBits: EVFLAG_DEFEAT_ALL (6), whichever of bits 5 and 6 it is
SHOP_ITEMS = 20  # gProcScr_Shop's proc: its stock at +0x30, 20 u16 (item | uses << 8)
PERMANENT_FLAGS = 0x03005250  # gPermanentFlagBits: flag f (0x65 and up) is bit (f - 0x65)
CHAPTER_FLAGS = 0x03005270  # gChapterFlagBits: flag f (1 to 0x64) is bit (f - 1)
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
CH18_PAL_ANIM = 84  # Chapter 18's map palette animation id (gChapterDataTable, +10)
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
    "HexSave_Save": "gHexSave", "Snek_SaveHighScore": "gSnekHighScore", "BigUnit_Save": "gBigUnitGroups",
    "SaveUnitLearnedSkillLists": "gLearnedSkills",
}
# values written before saving, so a chunk that is not saved or loaded shows up: (RAM symbol, offset, bits, value)
SENTINELS = [("gGuildSave", 2, 16, 1234), ("gCrossingRecords", 2, 16, 777), ("gTowerRecords", 2, 16, 345),
             ("gSnekHighScore", 0, 16, 42),
             # the last 2x2 group's top-left tile: its corners are empty, so no unit moves
             ("gBigUnitGroups", 7 * 6 + 4, 16, 0x0605)]

# the Vesly debugger (B on an empty tile): page 2, "Save & restart ch." (as in boot-to-prep.txt), then the save slot
DEBUG_SAVE = ["press B", "wait 60", "press RIGHT", "wait 40", "press DOWN", "wait 10", "press DOWN", "wait 10",
              "press A", "wait 60"] + ["press DOWN", "wait 10"] * 7 + ["press START", "wait 200", "press A",
                                                                         "wait 120", "press A", "wait 600"]
# From a district's start (Serafew's start.ss): the gate is below; Yes; the save menu (A, A); the world map's
# beginning scene
LEAVE_DISTRICT = ["hold DOWN 8", "wait 60", "hold A 8", "wait 60", "hold LEFT 8", "wait 40", "hold A 8", "wait 400",
                  "hold A 8", "wait 120", "hold A 8", "wait 900"]
# The idle world map: the cursor off every node, A opens the general menu (Unit, Status, Options, Save, Home); Save, the
# first slot, Save
WM_SAVE = ["w32 %#x %#x" % (0x03005280 + 8, 40 << 8), "w32 %#x %#x" % (0x03005280 + 0xC, 40 << 8), "wait 10",
           "press A", "wait 40"] + ["press DOWN", "wait 10"] * 3 + ["press A", "wait 120", "press A", "wait 60",
                                                                    "press A", "wait 300"]
# A+B+START+SELECT, then the language screen and the title
SOFT_RESET = ["hold A+B+START+SELECT 10", "wait 500", "press A", "wait 200", "press START", "wait 200"]
# Eirika is under the cursor at the start of the prologue: A selects her, A stays, and the action menu opens
EIRIKA_MENU = ["wait 30", "press A", "wait 30", "press A", "wait 40"]


class Smoke:
    def __init__(self, out):
        self.out = out
        self.results = []
        self.shots = []
        self._tls = threading.local()
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

    def _results(self):
        """The running task's result list (Runner), or the whole run's."""
        buf = getattr(self._tls, "results", None)
        return self.results if buf is None else buf

    def check(self, name, ok, detail=""):
        self._results().append((name, bool(ok), detail))
        print("%-4s %s%s" % ("PASS" if ok else "FAIL", name, "  (" + detail + ")" if detail else ""), flush=True)
        return ok

    def shot(self, path, label):
        if os.path.exists(path):
            buf = getattr(self._tls, "shots", None)
            (self.shots if buf is None else buf).append((path, label))

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

    def turn2_state(self, boot_ss):
        """The prologue's turn 2, for the tests that fight: boot.ss ends turn 1 (the map menu from the empty tile left of
        Eirika, its last row, End), and the enemy phase brings two soldiers next to her, at (5, 5) and (6, 5). Turn 2
        starts with the cursor on her. (With real-time battle on, the soldiers came during turn 1.)"""
        d = os.path.join(self.out, "boot")
        out = os.path.join(d, "turn2.ss")
        self.run(ROM, ["load " + boot_ss, "press LEFT", "wait 10", "press A", "wait 40", "press UP", "wait 10",
                       "press A", "wait 1200", "shot turn2.png", "save " + out], d)
        self.check("boot: turn 2, with soldiers next to Eirika", os.path.exists(out))
        return out if os.path.exists(out) else None

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
        twice (a wandering NPC, a tile free movement refuses; once can be input timing) marks that tile blocked, and
        the path is planned again. When the
        blocked tiles leave no path, it waits two seconds for the wandering NPCs to move on and forgets them (4 times at
        most)."""
        w, h, tiles = layout["w"], layout["h"], layout["tiles"]
        ground = rules.WALK - {0x26, 0x1E}
        blocked = set()
        missed = {}

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
            # A step that does not register leaves the next ones to run from an earlier tile: stop the run before a
            # step that, from any tile so far, would land on a tile to avoid (the gate's prompt would take the keys)
            run_steps, p, seen = [], pos, [pos]
            for key in steps[:4]:
                dx, dy = {"RIGHT": (1, 0), "LEFT": (-1, 0), "DOWN": (0, 1), "UP": (0, -1)}[key]
                if run_steps and any((q[0] + dx, q[1] + dy) in avoid for q in seen):
                    break
                run_steps.append(key)
                p = (p[0] + dx, p[1] + dy)
                seen.append(p)
            steps = run_steps
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
                    # a step can fail to register once (the frame the run starts on): block the tile on the second
                    tile = (x + dx, y + dy)
                    missed[tile] = missed.get(tile, 0) + 1
                    if missed[tile] >= 2:
                        blocked.add(tile)
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
            # a player would find; so a press that opens nothing is tried again after another wait. The spot's state is
            # saved after the wait, so the tests that load it and press A at once press when it works.
            for wait in (10, 60, 0):
                vals = self.run(rom, ["load " + at, "wait %d" % wait, "save " + spot_ss, "shot %s_0.png" % tag,
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

    def worldmap_idle(self, rom, d):
        """Leaves Serafew (its start.ss) for the world map, with the story on Chapter 6 (Adlas Plains), the lord on the
        Serafew node, nodes 0-5 cleared and Serafew unlocked; saves the idle world map as d/idle.ss. Returns its path and
        the story chapter read there."""
        story = self.sym["gWmStoryChapter"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        flag, bit = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3), 1 << ((DISTRICT_SERAFEW_FLAG - 0x65) & 7)
        # gGMData: the lord's node at +0x11, the nodes' states at +0x30 (4 bytes each; 1 shown and cleared)
        setup = ["w8 %#x %#x" % (story, CHAPTER_06), "w8 %#x %d" % (GM_DATA + 0x11, WM_NODE_SERAFEW),
                 "w8 %#x %#x" % (flag, bit)] + ["w8 %#x 1" % (GM_DATA + 0x30 + 4 * n) for n in range(6)]
        idle = os.path.join(d, "idle.ss")
        vals = self.run(rom, ["load " + start, "wait 30"] + setup + LEAVE_DISTRICT +
                        ["save " + idle, "shot idle.png", "r8 %#x" % story], d)
        return idle, vals

    def test_worldmap(self, states):
        """The world map, reached by leaving Serafew through its gate (a test ROM's district). Before leaving: the story
        on Chapter 6 (Adlas Plains), the lord on the Serafew node, nodes 0-5 cleared and Serafew unlocked. The district
        visit keeps the story chapter; Enter District loads Serafew, from a plain node and from the story chapter's
        node (#672); with Serafew locked the row is not there; travelling to the story node starts Chapter 6."""
        d = os.path.join(self.out, "WorldMap")
        os.makedirs(d, exist_ok=True)
        need = ("gWmStoryChapter", "sWmChapterIdOverride")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("worldmap: the world map", False, "no district state or symbols")
        rom, story, override = states["rom"], self.sym["gWmStoryChapter"], self.sym["sWmChapterIdOverride"]
        flag, bit = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3), 1 << ((DISTRICT_SERAFEW_FLAG - 0x65) & 7)
        idle, vals = self.worldmap_idle(rom, d)
        self.shot(os.path.join(d, "idle.png"), "world map after a district")
        if not self.check("worldmap: leaving a district keeps the story chapter", vals == [CHAPTER_06],
                          "story chapter %s" % vals):
            return

        def enter(name, writes):
            lines = ["load " + idle] + writes + ["wait 10", "press A", "wait 40", "shot %s_menu.png" % name, "press A",
                                                 "wait 330", "shot %s.png" % name, "r8 %#x" % CHAPTER,
                                                 "r8 %#x" % override, "r8 %#x" % story]
            return self.run(rom, lines, d)
        vals = enter("enter", [])
        self.shot(os.path.join(d, "enter_menu.png"), "node menu")
        self.check("worldmap: Enter District loads the district", vals == [DISTRICTS["Serafew"][0], 0, CHAPTER_06],
                   "chapter, override, story %s" % vals)
        vals = enter("story", ["w8 %#x %#x" % (story, CHAPTER_05), "w8 %#x 1" % (GM_DATA + 0x30 + 4 * WM_NODE_SERAFEW)])
        self.shot(os.path.join(d, "story.png"), "Enter District on the story node")
        self.check("worldmap: Enter District on the story chapter's node loads the district",
                   vals == [DISTRICTS["Serafew"][0], 0, CHAPTER_05], "chapter, override, story %s" % vals)
        vals = enter("locked", ["w8 %#x 0" % flag])
        self.shot(os.path.join(d, "locked_menu.png"), "node menu, Serafew locked")
        self.check("worldmap: a locked district has no Enter District row (the first row is another)",
                   vals[:1] == [CHAPTER_06], "chapter %s" % vals)
        # The district flags (#710, DistrictMarker.c): the next chapter's flag in green (OBJ palette DISTRICT_FLAG_PAL),
        # over Serafew's node on the world map and on the minimap (its flag object, OBJ chr MINIMAP_FLAG_CHR), while
        # Serafew is unlocked, and gone once it is locked
        def flags(writes):
            vals = self.run(rom, ["load " + idle] + writes + ["wait 20"] +
                            ["r16 %#x" % (OAM + 8 * i + k) for i in range(128) for k in (0, 4)], d)
            ours = [(a0, a2) for a0, a2 in zip(vals[0::2], vals[1::2])
                    if (a0 >> 8) & 3 != 2 and a2 >> 12 == DISTRICT_FLAG_PAL]
            return (sum(1 for _, a2 in ours if a2 & 0x3FF != MINIMAP_FLAG_CHR),
                    sum(1 for _, a2 in ours if a2 & 0x3FF == MINIMAP_FLAG_CHR))
        shown, locked = flags([]), flags(["w8 %#x 0" % flag])
        self.check("worldmap: an unlocked district's node has the green flag, on the map and the minimap; a locked one "
                   "has none", shown[0] >= 1 and shown[1] == 1 and locked == (0, 0),
                   "map and minimap flags: %s unlocked, %s locked" % (shown, locked))
        # L takes the cursor to the next node (the story node), A sends the lord; START and A through the scenes
        lines = ["load " + idle, "wait 10", "press L", "wait 40", "press A", "wait 30"]
        lines += ["press START", "wait 90", "press A", "wait 60"] * 6 + ["shot travel.png", "r8 %#x" % CHAPTER,
                                                                          "r8 %#x" % (GM_DATA + 0x11)]
        vals = self.run(rom, lines, d)
        self.shot(os.path.join(d, "travel.png"), "the story chapter from its node")
        self.check("worldmap: travelling to the story node starts the story chapter",
                   vals == [CHAPTER_06, WM_NODE_ADLAS_PLAINS], "chapter, lord's node %s" % vals)

    def stub_battle(self, reads=False):
        """A stub chapter's battle (#700), from the trip on: START and A skip a world-map travel scene (Chapter 10's is
        long); the title card, then the opening's line, which waits (START skips it); the enemies and the preparations
        screen, which waits too (START: fight); then (reads: the first enemy slots) every enemy taken off the map and the
        defeat-all flag set, Eirika (under the cursor) waits, and the misc events play the ending; the save menu (the
        first slot, Save)."""
        chapter_flags = self.sym.get("gChapterFlagBits", 0x03005270)
        win = ["w32 %#x 0" % (UNIT_RED + 0x48 * i) for i in range(RED_UNITS)] + \
            ["w8 %#x %#x" % (chapter_flags, CHAPTER_FLAG_DEFEAT_ALL)]
        enemies = ["r32 %#x" % (UNIT_RED + 0x48 * i) for i in range(STUB_ENEMY_READS)] if reads else []
        return ["press START", "wait 60", "press A", "wait 60", "wait 800", "press START", "wait 300", "press START",
                "wait 200"] + enemies + win + ["press A", "wait 30", "press A", "wait 40", "press UP", "wait 10",
                                               "press A", "wait 120", "press A", "wait 60", "press A", "wait 300"]

    def test_stubs(self, states):
        """The stub story chapters (CustomCampaign/Chapters/Stubs), on the world map reached as in test_worldmap: from
        Chapter 10 the lord travels to each story chapter's node (the cursor put on it, A), the stub plays and the
        story moves to the next chapter and node, through Chapter 20; then 21 and 22 follow without the world map and
        the game ends."""
        d = os.path.join(self.out, "Stubs")
        os.makedirs(d, exist_ok=True)
        if "rom" not in states or "gWmStoryChapter" not in self.sym:
            return self.check("stubs: the stub chapters", False, "no district state or symbols")
        rom, story = states["rom"], self.sym["gWmStoryChapter"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        data = open(rom, "rb").read()
        node_xy = lambda n: struct.unpack_from("<hh", data, WM_NODE_DATA - 0x08000000 + n * 0x20 + 0x18)
        # world-map monsters off (one on a story node turns the trip into a skirmish)
        mons = ["w32 %#x 0" % (GM_DATA + 0x10 + 4 * i) for i in range(4, 8)]
        nodes = ["w8 %#x %d" % (GM_DATA + 0x30 + 4 * n, 1 if n < 10 else 0) for n in range(0x1D)]
        setup = ["w8 %#x %#x" % (story, STUB_ROUTE[0][0]), "w8 %#x %d" % (GM_DATA + 0x11, STUB_ROUTE[0][2])] + nodes + mons
        lines = ["load " + start, "wait 30"] + setup + LEAVE_DISTRICT + mons + ["wait 10", "r8 %#x" % story]
        battle = self.stub_battle(reads=True)
        got = []
        for chapter, node, _ in STUB_ROUTE:
            x, y = node_xy(node)
            if chapter == STUB_ROUTE[0][0]:
                # Chapter 10's node with a monster on it and not marked "next" (as a SET_NODE script without
                # WM_SETDESTINATION leaves it): the trip must still start the chapter, not a skirmish (#690)
                lines += ["w32 %#x %#x" % (GM_DATA + 0x20, (CLASS_REVENANT << 16) | (node << 8) | 1),
                          "w8 %#x 1" % (GM_DATA + 0x30 + 4 * node)]
            lines += ["w32 %#x %#x" % (GM_DATA + 8, x << 8), "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10",
                      "press A", "wait 90"] + battle
            if chapter == STUB_ROUTE[-1][0]:
                # Chapters 21 and 22 come straight after, then the ending (MNC4)
                lines += ["wait 200"] + battle + ["wait 200"] + battle + ["wait 900", "shot ending.png",
                                                                          "r8 %#x" % CHAPTER, "r8 %#x" % (PLAYST + 0x14)]
            else:
                lines += ["wait 900"] + mons + ["wait 10", "r8 %#x" % story, "r8 %#x" % (GM_DATA + 0x11)]
        vals = self.run(rom, lines, d)
        self.shot(os.path.join(d, "ending.png"), "after the last stub")
        if not vals or vals[0] != STUB_ROUTE[0][0]:
            return self.check("stubs: the world map on Chapter 10", False, "story %s" % vals[:1])
        # Each battle's enemy slots, then (story, node) after each world-map leg; the last leg has three battles
        n, legs, loaded, at = STUB_ENEMY_READS, [], [], 1
        for i in range(len(STUB_ROUTE)):
            for _ in range(3 if i == len(STUB_ROUTE) - 1 else 1):
                loaded.append(sum(1 for v in vals[at:at + n] if v))
                at += n
            if i < len(STUB_ROUTE) - 1:
                legs.append(tuple(vals[at:at + 2]))
                at += 2
        self.check("stubs: every stub chapter's battle loads its enemies (Chapters 10-22)",
                   len(loaded) == len(STUB_ROUTE) + 2 and all(loaded), "enemies per chapter %s" % loaded)
        want = [(STUB_ROUTE[i + 1][0], STUB_ROUTE[i + 1][2]) for i in range(len(STUB_ROUTE) - 1)]
        bad = ["%#x" % STUB_ROUTE[i][0] for i, (g, w) in enumerate(zip(legs, want)) if g != w]
        self.check("stubs: a monster on the story node, not marked next, does not stop the chapter (Chapter 10)",
                   legs[:1] == want[:1], "story, lord's node %s" % (legs[:1],))
        self.check("stubs: each stub chapter moves the story and the lord on (Chapters 10-19)", not bad,
                   "wrong after %s: %s" % (bad, legs) if bad else "%d chapters" % len(legs))
        end = vals[at:]
        self.check("stubs: Chapter 20 leads through 21 and 22 to the ending", len(end) == 2 and
                   end[0] == STUB_FINAL and end[1] & PLAY_FLAG_COMPLETE, "chapter, state bits %s" % end)

    def test_wmsave(self, states):
        """A game save made from the world map (test_worldmap's state, with Port Kiris unlocked too): the general
        menu's Save, a soft reset and Restart Chapter bring back the story chapter, the lord's node, the unlocked
        districts and the nodes' states; Enter District then loads Serafew. The game boots first, so that SRAM is set up
        (a save state does not hold it, and a reset finds an unset SRAM empty)."""
        d = os.path.join(self.out, "WorldMapSave")
        os.makedirs(d, exist_ok=True)
        need = ("gWmStoryChapter", "sWmChapterIdOverride")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("wmsave: the world map", False, "no district state or symbols")
        rom, story, override = states["rom"], self.sym["gWmStoryChapter"], self.sym["sWmChapterIdOverride"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        flags = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3)
        bits = (1 << ((DISTRICT_SERAFEW_FLAG - 0x65) & 7)) | (1 << ((DISTRICT_PORT_KIRIS_FLAG - 0x65) & 7))
        assert (DISTRICT_PORT_KIRIS_FLAG - 0x65) >> 3 == flags - PERMANENT_FLAGS
        node = lambda n: GM_DATA + 0x30 + 4 * n
        setup = ["w8 %#x %#x" % (story, CHAPTER_06), "w8 %#x %d" % (GM_DATA + 0x11, WM_NODE_SERAFEW),
                 "w8 %#x %#x" % (flags, bits)] + ["w8 %#x 1" % node(n) for n in range(6)]
        reads = ["r8 %#x" % CHAPTER, "r8 %#x" % story, "r8 %#x" % (GM_DATA + 0x11), "r8 %#x" % flags] + \
            ["r8 %#x" % node(n) for n in range(WM_NODES)]
        wipe = ["w8 %#x 0" % a for a in (story, GM_DATA + 0x11, flags)] + ["w8 %#x 0" % node(n) for n in range(WM_NODES)]
        x, y = struct.unpack_from("<hh", open(rom, "rb").read(), WM_NODE_DATA - 0x08000000 + WM_NODE_SERAFEW * 0x20 + 0x18)
        # Restart Chapter, the first slot; then the cursor on Serafew, A (the node menu), A (Enter District)
        lines = ["wait 600", "load " + start, "wait 30"] + setup + LEAVE_DISTRICT + reads + WM_SAVE + ["shot saved.png"]
        lines += wipe + SOFT_RESET + ["press A", "wait 200", "shot slots.png", "press A", "wait 600", "shot loaded.png"]
        lines += reads + ["w32 %#x %#x" % (GM_DATA + 8, x << 8), "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10",
                          "press A", "wait 40", "press A", "wait 330", "shot enter.png", "r8 %#x" % CHAPTER,
                          "r8 %#x" % override]
        vals = self.run(rom, lines, d)
        for name, label in (("slots", "save slots after a world-map save"), ("loaded", "world map loaded"),
                            ("enter", "Enter District after the load")):
            self.shot(os.path.join(d, name + ".png"), label)
        n = len(reads)
        if len(vals) != 2 * n + 2:
            return self.check("wmsave: the world map saves and loads", False, "values %s" % vals)
        pre, post, enter = vals[:n], vals[n:2 * n], vals[2 * n:]
        if not self.check("wmsave: Restart Chapter loads the world map's save", post[0] == pre[0] == CHAPTER_06,
                          "chapter %#x, saved %#x" % (post[0], pre[0])):
            return
        self.check("wmsave: the story chapter comes back", post[1] == pre[1] == CHAPTER_06, "story %#x" % post[1])
        self.check("wmsave: the lord's node comes back", post[2] == pre[2] == WM_NODE_SERAFEW, "node %d" % post[2])
        self.check("wmsave: the unlocked districts come back (Serafew, Port Kiris)", post[3] & bits == bits,
                   "flags %#x" % post[3])
        bad = [i for i in range(WM_NODES) if post[4 + i] != pre[4 + i]]
        self.check("wmsave: the nodes' states come back (%d nodes)" % WM_NODES, not bad,
                   "different: %s" % [(i, pre[4 + i], post[4 + i]) for i in bad] if bad else
                   "shown: %s" % [i for i in range(WM_NODES) if pre[4 + i] & 1])
        self.check("wmsave: Enter District after the load loads the district",
                   enter == [DISTRICTS["Serafew"][0], 0], "chapter, override %s" % enter)

    def procs(self, path):
        """The live procs in a dump of sProcArray (PROCS bytes): [(script address, the proc's bytes)]."""
        data = open(path, "rb").read() if os.path.exists(path) else b""
        return [(struct.unpack_from("<I", data, i)[0], data[i:i + PROC_SIZE]) for i in range(0, len(data), PROC_SIZE)
                if struct.unpack_from("<I", data, i)[0]]

    def test_wmshops(self, states):
        """The node menu's shop rows on the world map (test_worldmap's idle state, Serafew locked so its first rows are
        the shops'). Serafew's node has an armory and a vendor: the Armory row opens the armory's base menu, A opens the
        shop with the node's stock, B leaves to the world map; the Shop row (one down) opens the vendor with its stock.
        On Adlas Plains (no shops), the first row is not a shop's. The road to the story node is in the open paths
        (WM_REVEAL_DEST). Before #694 the rows checked the wrong node's state byte (GMNode was 1 byte in C)."""
        d = os.path.join(self.out, "WorldMapShops")
        os.makedirs(d, exist_ok=True)
        need = ("gWmStoryChapter", "ProcScr_GmapBaseMenu", "gProcScr_Shop", "ProcScr_WorldMapMain", "sProcArray")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("wmshops: the world map", False, "no district state or symbols")
        rom = states["rom"]
        idle, vals = self.worldmap_idle(rom, d)
        if not self.check("wmshops: the world map after a district", vals == [CHAPTER_06], "story %s" % vals):
            return
        data = open(rom, "rb").read()
        node = lambda n, k: WM_NODE_DATA - 0x08000000 + n * 0x20 + k

        def stock(n, k):
            a, out = struct.unpack_from("<I", data, node(n, k))[0] - 0x08000000, []
            while struct.unpack_from("<H", data, a)[0]:
                out.append(struct.unpack_from("<H", data, a)[0] & 0xFF)
                a += 2
            return out
        flag = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3)
        procs = self.sym["sProcArray"]
        base, shop, main = (self.sym[n] for n in ("ProcScr_GmapBaseMenu", "gProcScr_Shop", "ProcScr_WorldMapMain"))
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)

        def visit(name, rows):
            """The node menu, rows DOWNs, A: the base menu; A: the shop; B out to the world map."""
            lines = ["load " + idle, "w8 %#x 0" % flag, "wait 10", "press A", "wait 40"]
            lines += ["press DOWN", "wait 10"] * rows + ["press A", "wait 220", dump(name + "_base.bin"), "press A",
                                                         "wait 200", "shot %s.png" % name, dump(name + "_shop.bin")]
            lines += ["press B", "wait 90"] * 6 + ["shot %s_back.png" % name, dump(name + "_back.bin")]
            self.run(rom, lines, d)
            self.shot(os.path.join(d, name + ".png"), "world-map " + name)
            kind = [p[0x29] for a, p in self.procs(os.path.join(d, name + "_base.bin")) if a == base]
            items = [list(p[0x30:0x30 + 2 * SHOP_ITEMS:2]) for a, p in self.procs(os.path.join(d, name + "_shop.bin"))
                     if a == shop]
            back = [a for a, _ in self.procs(os.path.join(d, name + "_back.bin"))]
            return kind, items, main in back and base not in back and shop not in back

        for name, rows, k, kind_want in (("armory", 0, 0xC, 0), ("vendor", 1, 0x10, 1)):
            want = stock(WM_NODE_SERAFEW, k)
            kind, items, back = visit(name, rows)
            self.check("wmshops: Serafew's %s row opens the %s" % (name, name), kind == [kind_want],
                       "base menu kind %s" % kind)
            # The whole list, and nothing after its end (it used to read on into the next list in ROM). With Three
            # Houses magic on, StartShopScreen drops tomes and staves; the vendors' lists have none (#704,
            # Data/WorldMap/VendorStock.c), so Serafew's vendor sells all of its 5 items, not just a Vulnerary
            got = items[0] if len(items) == 1 else []
            sold = got[:got.index(0)] if 0 in got else got
            self.check("wmshops: the %s sells all of Serafew's stock (%d items), and nothing after it" % (name, len(want)),
                       len(want) >= 5 and sold == want and not any(got[len(sold):]),
                       "shop %s, node %s" % (got, want))
            self.check("wmshops: B leaves the %s for the world map" % name, back, "")
        # Adlas Plains, cleared, the story on Chapter 7 (Renvall): no shop row comes first
        x, y = struct.unpack_from("<hh", data, node(WM_NODE_ADLAS_PLAINS, 0x18))
        lines = ["load " + idle, "w8 %#x %#x" % (self.sym["gWmStoryChapter"], CHAPTER_07),
                 "w8 %#x %d" % (GM_DATA + 0x11, WM_NODE_ADLAS_PLAINS), "w8 %#x 1" % (GM_DATA + 0x30 + 4 * WM_NODE_ADLAS_PLAINS),
                 "w32 %#x %#x" % (GM_DATA + 8, x << 8), "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10", "press A",
                 "wait 40", "shot plain_menu.png", "press A", "wait 220", dump("plain.bin")]
        self.run(rom, lines, d)
        self.shot(os.path.join(d, "plain_menu.png"), "node menu on Adlas Plains")
        got = [a for a, _ in self.procs(os.path.join(d, "plain.bin"))]
        self.check("wmshops: a node with no shops has no shop rows (Adlas Plains)", got and base not in got,
                   "base menu open" if base in got else "")
        # The road WM_REVEAL_DEST drew from Serafew to Adlas Plains
        vals = self.run(rom, ["load " + idle, "r8 %#x" % (GM_DATA + 0xC4)] +
                        ["r8 %#x" % (GM_DATA + 0xA4 + i) for i in range(0x20)], d)
        paths = [p for p in vals[1:1 + vals[0]]] if vals else []
        ends = lambda p: tuple(sorted(data[WM_PATH_DATA - 0x08000000 + 12 * p + 4:WM_PATH_DATA - 0x08000000 + 12 * p + 6]))
        self.check("wmshops: the road to the story node is open (Serafew to Adlas Plains)",
                   any(ends(p) == (WM_NODE_SERAFEW, WM_NODE_ADLAS_PLAINS) for p in paths),
                   "open paths %s" % [(p, ends(p)) for p in paths])

    def test_postgame(self, states):
        """The postgame. From Serafew (start.ss) the world map opens on Chapter 20's (the story on Chapter 20, the lord on
        Rausten Court, the map as the story leaves it: nodes 0-24 and the Tower of Valni shown, roads 0-30 open, Serafew
        and Port Kiris unlocked). The trip to Black Temple plays Chapters 20, 21 and 22, the save menu and the ending;
        after "Fin" the game goes on to the postgame world map. Then: the postgame dungeons are shown, Black Temple no
        longer starts Chapter 20, Rausten Court's node menu has the secret shop, and trips start the Lagdou Ruins and the
        Tower of Valni, and reach Port Kiris, whose Enter District loads it. After a reset the end save is titled
        "Creature Campaign", and Restart Chapter on it opens the postgame world map. The game boots first, so the end save
        is written to set-up SRAM."""
        d = os.path.join(self.out, "Postgame")
        os.makedirs(d, exist_ok=True)
        need = ("gWmStoryChapter", "ProcScr_GmapBaseMenu", "sProcArray")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("postgame: the postgame", False, "no district state or symbols")
        rom, story = states["rom"], self.sym["gWmStoryChapter"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        data = open(rom, "rb").read()
        xy = lambda n: struct.unpack_from("<hh", data, WM_NODE_DATA - 0x08000000 + n * 0x20 + 0x18)
        node = lambda n: GM_DATA + 0x30 + 4 * n
        mons = ["w32 %#x 0" % (GM_DATA + 0x10 + 4 * i) for i in range(4, 8)]
        flags = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3)
        bits = (1 << ((DISTRICT_SERAFEW_FLAG - 0x65) & 7)) | (1 << ((DISTRICT_PORT_KIRIS_FLAG - 0x65) & 7))
        setup = ["w8 %#x %#x" % (story, CHAPTER_20), "w8 %#x %d" % (GM_DATA + 0x11, WM_NODE_RAUSTEN),
                 "w8 %#x %#x" % (flags, bits)] + mons
        setup += ["w8 %#x %d" % (node(n), 1 if n < WM_NODE_BLACK_TEMPLE or n == WM_NODE_VALNI else 0)
                  for n in range(WM_NODES)]
        setup += ["w8 %#x %d" % (GM_DATA + 0xA4 + i, i) for i in range(OPEN_ROADS)] + \
            ["w8 %#x %d" % (GM_DATA + 0xC4, OPEN_ROADS)]
        x, y = xy(WM_NODE_BLACK_TEMPLE)
        post = os.path.join(d, "post.ss")
        lines = ["wait 600", "load " + start, "wait 30"] + setup + LEAVE_DISTRICT + mons + ["wait 10", "shot ch20.png"]
        lines += ["r8 %#x" % node(n) for n in POSTGAME_NODES]
        lines += ["w32 %#x %#x" % (GM_DATA + 8, x << 8), "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10", "press A",
                  "wait 90"] + self.stub_battle() + ["wait 200"] + self.stub_battle() + ["wait 200"] + \
            self.stub_battle()
        # the ending and "Fin", which waits: A, and the game goes on in the end save, on the world map
        lines += ["wait 600", "shot fin.png", "press A", "wait 900", "shot postgame.png", "save " + post,
                  "r8 %#x" % (PLAYST + 0x14), "r8 %#x" % (GM_DATA + 0x11)] + ["r8 %#x" % node(n) for n in POSTGAME_NODES]
        # a reset: the main menu (its proc: the Extras rows, the slots' titles); Restart Chapter, the end save
        procs, menu_scr = self.sym["sProcArray"], self.sym.get("ProcScr_SaveMenu", 0)
        lines += SOFT_RESET + ["dump %#x %#x menu.bin" % (procs, PROC_SIZE * PROCS), "press A", "wait 200",
                               "shot slot.png", "press A", "wait 900", "shot restart.png", "r8 %#x" % (PLAYST + 0x14),
                               "r8 %#x" % (GM_DATA + 0x11)]
        vals = self.run(rom, lines, d)
        for name, label in (("ch20", "world map before Chapter 20"), ("fin", "the ending's Fin"),
                            ("postgame", "after the ending: the postgame"),
                            ("slot", "the end save: Creature Campaign"), ("restart", "Restart Chapter on it")):
            self.shot(os.path.join(d, name + ".png"), label)
        n = len(POSTGAME_NODES)
        menu = [p for a, p in self.procs(os.path.join(d, "menu.bin")) if a == menu_scr]
        extras, titles = (menu[0][0x32], list(menu[0][0x37:0x3A])) if menu else (None, [])
        # (Extras' bit 4, vanilla's Map row, is the Gallery's now: test_gallery)
        self.check("postgame: the end save is titled Creature Campaign",
                   extras is not None and 0x57 in titles,
                   "Extras rows %s, slot titles %s" % ("%#x" % extras if extras is not None else "?",
                                                       ["%#x" % t for t in titles]))
        restart = vals[2 * n + 2:]
        self.check("postgame: Restart Chapter on the end save opens the postgame world map",
                   len(restart) == 2 and restart[0] & PLAY_FLAG_POSTGAME and restart[1] == WM_NODE_BLACK_TEMPLE,
                   "state bits, lord's node %s" % restart)
        vals = vals[:2 * n + 2]
        if len(vals) != 2 * n + 2:
            return self.check("postgame: the ending leads to the postgame", False, "values %s" % vals)
        pre, state, lord, after = vals[:n], vals[n], vals[n + 1], vals[n + 2:]
        self.check("postgame: Chapter 20's world map shows the Lagdou Ruins and Melkaen Coast",
                   all(pre[POSTGAME_NODES.index(k)] & 1 for k in (WM_NODE_LAGDOU, WM_NODE_MELKAEN)),
                   "states %s" % dict(zip(POSTGAME_NODES, pre)))
        if not self.check("postgame: after the ending the game goes on in the postgame",
                          state & PLAY_FLAG_POSTGAME and state & PLAY_FLAG_COMPLETE and lord == WM_NODE_BLACK_TEMPLE,
                          "state bits %#x, lord on %d" % (state, lord)):
            return
        self.check("postgame: the Tower of Valni, the Lagdou Ruins and Melkaen Coast are shown, Black Temple is not next",
                   all(a & 1 for a in after) and not after[POSTGAME_NODES.index(WM_NODE_BLACK_TEMPLE)] & 2,
                   "states %s" % dict(zip(POSTGAME_NODES, after)))

        def trip(name, to, then=()):
            """From the postgame state, the cursor on node to, A, and the lord walks there (monsters kept off)."""
            x, y = xy(to)
            lines = ["load " + post] + mons + ["w32 %#x %#x" % (GM_DATA + 8, x << 8),
                                               "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10", "press A", "wait 30"]
            lines += (mons + ["wait 120"]) * 10 + ["shot %s_there.png" % name] + list(then)
            return self.run(rom, lines + ["r8 %#x" % CHAPTER, "r8 %#x" % (GM_DATA + 0x11)], d)
        # Black Temple and back: the story's last node starts nothing now
        vals = trip("temple", WM_NODE_RAUSTEN)
        x, y = xy(WM_NODE_BLACK_TEMPLE)
        lines = ["load " + post] + mons + ["w32 %#x %#x" % (GM_DATA + 8, xy(WM_NODE_RAUSTEN)[0] << 8),
                                           "w32 %#x %#x" % (GM_DATA + 0xC, xy(WM_NODE_RAUSTEN)[1] << 8), "wait 10",
                                           "press A", "wait 30"] + (mons + ["wait 120"]) * 6
        lines += ["w32 %#x %#x" % (GM_DATA + 8, x << 8), "w32 %#x %#x" % (GM_DATA + 0xC, y << 8), "wait 10", "press A",
                  "wait 30"] + (mons + ["wait 120"]) * 6 + ["shot temple.png", "r8 %#x" % CHAPTER,
                                                            "r8 %#x" % (GM_DATA + 0x11)]
        vals = self.run(rom, lines, d)
        self.shot(os.path.join(d, "temple.png"), "postgame: back on Black Temple")
        self.check("postgame: a trip back to Black Temple does not start Chapter 20",
                   vals == [STUB_FINAL, WM_NODE_BLACK_TEMPLE], "chapter, lord's node %s" % vals)
        # Rausten Court's node menu: Armory, Shop, then the secret shop (the menu reopens on its row unk_cd: 0)
        procs, base = self.sym["sProcArray"], self.sym["ProcScr_GmapBaseMenu"]
        vals = trip("secret", WM_NODE_RAUSTEN, ["w8 %#x 0" % (GM_DATA + 0xCD), "press A", "wait 40",
                                               "shot secret_menu.png", "press DOWN", "wait 10", "press DOWN", "wait 10",
                                               "press A", "wait 220", "shot secret.png",
                                               "dump %#x %#x secret.bin" % (procs, PROC_SIZE * PROCS)])
        self.shot(os.path.join(d, "secret_menu.png"), "postgame: Rausten Court's node menu")
        kind = [p[0x29] for a, p in self.procs(os.path.join(d, "secret.bin")) if a == base]
        self.check("postgame: Rausten Court's third row opens the secret shop", kind == [2],
                   "base menu kind %s, lord on %s" % (kind, vals[1:]))
        # The dungeons: arriving starts their first floor
        for name, to, chapter in (("lagdou", WM_NODE_LAGDOU, CHAPTER_LAGDOU_1), ("valni", WM_NODE_VALNI, CHAPTER_VALNI_1)):
            vals = trip(name, to, ["press A", "wait 60", "press A", "wait 600", "shot %s.png" % name])
            self.shot(os.path.join(d, name + ".png"), "postgame: " + name)
            self.check("postgame: a trip to the %s starts its first floor" % ("Lagdou Ruins" if name == "lagdou" else
                                                                               "Tower of Valni"),
                       vals[:1] == [chapter], "chapter, lord's node %s" % vals)
        # Port Kiris, through Melkaen Coast: Enter District
        vals = trip("kiris", WM_NODE_PORT_KIRIS, ["w8 %#x 0" % (GM_DATA + 0xCD), "press A", "wait 40",
                                                  "shot kiris_menu.png", "press A", "wait 400", "shot kiris.png"])
        self.shot(os.path.join(d, "kiris.png"), "postgame: Enter District on Port Kiris")
        self.check("postgame: Enter District on Port Kiris loads the district", vals == [DISTRICTS["PortKiris"][0],
                                                                                         WM_NODE_PORT_KIRIS],
                   "chapter, lord's node %s" % vals)

    def test_savetitle(self, states):
        """The save made on leaving a district (chapter 0x38, the side-map return) is titled with the story chapter
        (#705): the game boots (SRAM set up), Serafew is left with the story on Chapter 6 (the save menu's first slot,
        Save); after a soft reset, Restart Chapter's slot list holds the slot's title index (ProcScr_SaveMenu's
        chapter_idx at +0x37), which must be Chapter 6's row's chapTitleId, not a skirmish title (0x46-0x53)."""
        d = os.path.join(self.out, "SaveTitle")
        os.makedirs(d, exist_ok=True)
        need = ("gWmStoryChapter", "ProcScr_SaveMenu", "sProcArray")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("savetitle: the save title", False, "no district state or symbols")
        rom, story = states["rom"], self.sym["gWmStoryChapter"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        flag, bit = PERMANENT_FLAGS + ((DISTRICT_SERAFEW_FLAG - 0x65) >> 3), 1 << ((DISTRICT_SERAFEW_FLAG - 0x65) & 7)
        setup = ["w8 %#x %#x" % (story, CHAPTER_06), "w8 %#x %d" % (GM_DATA + 0x11, WM_NODE_SERAFEW),
                 "w8 %#x %#x" % (flag, bit)] + ["w8 %#x 1" % (GM_DATA + 0x30 + 4 * n) for n in range(6)]
        procs = self.sym["sProcArray"]
        lines = ["wait 600", "load " + start, "wait 30"] + setup + LEAVE_DISTRICT + SOFT_RESET + \
            ["press A", "wait 200", "shot slots.png", "dump %#x %#x slots.bin" % (procs, PROC_SIZE * PROCS)]
        self.run(rom, lines, d)
        self.shot(os.path.join(d, "slots.png"), "the save made on leaving Serafew")
        got = [p[0x37] for a, p in self.procs(os.path.join(d, "slots.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        want = open(rom, "rb").read()[CHAPTER_TABLE + CHAPTER_06 * 0x94 + 0x0E]
        self.check("savetitle: the save made on leaving a district is titled with the story chapter (Chapter 6)",
                   got == [want], "title index %s, Chapter 6's %#x" % (["%#x" % g for g in got], want))

    def test_districtresume(self, states):
        """A suspended district resumes in free movement (#708): the game boots (SRAM set up), Serafew starts (its
        start writes the suspend save), a soft reset and Resume Chapter bring it back, and the free-movement control
        proc runs (the district's opening, which turns it on, does not play again on a resume)."""
        d = os.path.join(self.out, "DistrictResume")
        os.makedirs(d, exist_ok=True)
        need = ("FreeMovementControlProc", "sProcArray")
        if "rom" not in states or not all(n in self.sym for n in need):
            return self.check("districtresume: the district", False, "no district state or symbols")
        rom, procs = states["rom"], self.sym["sProcArray"]
        start = os.path.join(self.out, "Serafew", "start.ss")
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)
        # 600 frames: the suspend save is written when the player phase starts, after its banner
        lines = ["wait 600", "load " + start, "wait 600", dump("before.bin")] + SOFT_RESET + \
            ["press A", "wait 200", "press A", "wait 300", "wait 300", "shot resumed.png", dump("after.bin"),
             "r8 %#x" % CHAPTER]
        vals = self.run(rom, lines, d)
        self.shot(os.path.join(d, "resumed.png"), "Serafew resumed")
        fmu = self.sym["FreeMovementControlProc"]
        before = fmu in [a for a, _ in self.procs(os.path.join(d, "before.bin"))]
        after = fmu in [a for a, _ in self.procs(os.path.join(d, "after.bin"))]
        self.check("districtresume: Resume Chapter in a district comes back in free movement",
                   before and after and vals == [DISTRICTS["Serafew"][0]],
                   "free movement before %s, after %s, chapter %s" % (before, after, vals))

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

    def layout_signature(self):
        """SaveLayoutGuard.c's signature, from the ROM's chunk tables: FNV-1a over each chunk's size and id (sav, then
        sus, each ending 0xFFFFFFFF), the two block sizes and the save block offsets."""
        rom = open(ROM, "rb").read()

        def mix(h, v):
            for i in range(4):
                h = ((h ^ ((v >> (8 * i)) & 0xFF)) * 16777619) & 0xFFFFFFFF
            return h
        h = 2166136261
        for table in ("gEmsSavChunks", "gEmsSusChunks"):
            a = self.sym[table] - 0x08000000
            while struct.unpack_from("<H", rom, a)[0] != 0xFFFF:
                h = mix(mix(h, struct.unpack_from("<H", rom, a + 2)[0]), struct.unpack_from("<I", rom, a + 12)[0])
                a += 16
            h = mix(h, 0xFFFFFFFF)
        for v in struct.unpack_from("<2H", rom, self.sym["gEmsSizes"] - 0x08000000):
            h = mix(h, v)
        for v in struct.unpack_from("<7H", rom, self.sym["gEmsOffsets"] - 0x08000000):
            h = mix(h, v)
        return h or 1

    def test_newgameplus(self):
        """New Game+ (Documentation/Features/NewGamePlus.md). A cleared save (the debugger's "Save & restart ch." with
        the cleared bit) has no New Game+ row in Extras; with the postgame bit it does. Then Extras > New Game+, the
        save, Normal and the empty file start a new game: in its prologue Eirika has the skills she had equipped in
        that save (Str Bonus and Vantage, written before saving) and its skill list (Deep Roots, a skill id above 0xFF,
        written as her only learned skill). The game boots first, so the saves go to set-up SRAM."""
        d = os.path.join(self.out, "newgameplus")
        os.makedirs(d, exist_ok=True)
        need = ("gNewGamePlus", "gLearnedSkills", "ProcScr_SaveMenu", "sProcArray")
        if not all(n in self.sym for n in need):
            return self.check("newgameplus: the New Game+ symbols", False, "no symbols")
        carry, lists, procs = self.sym["gNewGamePlus"], self.sym["gLearnedSkills"], self.sym["sProcArray"]
        str_bonus, vantage = self.skill_id("SID_StrBonus"), self.skill_id("SID_Vantage")
        packed = str_bonus | (vantage << 10)
        skills = [(packed >> (8 * k)) & 0xFF for k in range(7)]
        # the learned skills (struct LearnedSkills): only Eirika's Deep Roots, (pid 1 << 10) | sid
        deep_roots = (1 << 10) | self.skill_id("SID_DeepRoots")
        learned = ["w16 %#x 1" % lists, "w16 %#x %#x" % (lists + 2, deep_roots)] + \
            ["w32 %#x 0" % (lists + 4 + 4 * k) for k in range(64)]
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)
        extras = ["press DOWN", "wait 20"] * 4 + ["press A", "wait 200"]
        # a cleared save only
        self.run(ROM, self.boot_lines() + ["w8 %#x %#x" % (PLAYST + 0x14, PLAY_FLAG_COMPLETE)] + DEBUG_SAVE +
                 SOFT_RESET + extras + [dump("cleared.bin")], d)
        lines = self.boot_lines() + ["w8 %#x %#x" % (PLAYST + 0x14, PLAY_FLAG_COMPLETE | PLAY_FLAG_POSTGAME)]
        lines += ["w8 %#x %#x" % (UNIT1 + 0x32 + k, v) for k, v in enumerate(skills)] + learned
        lines += DEBUG_SAVE + SOFT_RESET + extras + [dump("postgame.bin"), "shot extras.png"]
        # New Game+ is the third row (Sound Room, Support Conversation, New Game+, Map); the first slot; Normal; the
        # empty file; then the opening, skipped
        lines += ["press DOWN", "wait 20", "press DOWN", "wait 20", "press A", "wait 200", "shot source.png", "press A",
                  "wait 230", "press DOWN", "wait 20", "press A", "wait 200", "shot file.png", "press A", "wait 60"]
        lines += ["press START", "wait 90", "press A", "wait 90"] * 12 + ["shot prologue.png"]
        lines += ["r8 %#x" % CHAPTER, "r32 %#x" % carry, "r32 %#x" % (carry + 8)]
        lines += ["r8 %#x" % (UNIT1 + 0x32 + k) for k in range(7)] + ["dump %#x 0x7AC learned.bin" % lists]
        vals = self.run(ROM, lines, d)
        for name, label in (("extras", "Extras with New Game+"), ("source", "New Game+: the save"),
                            ("file", "New Game+: the empty file"), ("prologue", "New Game+: the prologue")):
            self.shot(os.path.join(d, name + ".png"), label)
        menu = lambda name: [p[0x32] for a, p in self.procs(os.path.join(d, name)) if a == self.sym["ProcScr_SaveMenu"]]
        row = 1 << 3  # EXTRA_MENU_OPTION_NEW_GAME_PLUS
        cleared, postgame = menu("cleared.bin"), menu("postgame.bin")
        self.check("newgameplus: Extras has New Game+ only once a save has been to the postgame",
                   cleared and postgame and not cleared[0] & row and postgame[0] & row,
                   "Extras rows %s, then %s" % (["%#x" % v for v in cleared], ["%#x" % v for v in postgame]))
        if not self.check("newgameplus: New Game+ starts a new game", len(vals) == 10 and vals[0] == 0 and
                          vals[1] == 0x2B50474E, "values %s" % vals):
            return
        self.check("newgameplus: Eirika and Seth are carried", vals[2] & 0x6 == 0x6, "carried %#x" % vals[2])
        self.check("newgameplus: Eirika has the skills she had equipped (Str Bonus, Vantage)", vals[3:10] == skills,
                   "skill bytes %s, saved %s" % (["%#x" % v for v in vals[3:10]], ["%#x" % v for v in skills]))
        path = os.path.join(d, "learned.bin")
        data = open(path, "rb").read() if os.path.exists(path) else b"\0\0"
        count = struct.unpack_from("<H", data)[0]
        entries = struct.unpack_from("<%dH" % count, data, 2) if 2 + 2 * count <= len(data) else ()
        self.check("newgameplus: Eirika's skill list comes along (Deep Roots, id above 0xFF)", deep_roots in entries,
                   "%d learned skills: %s" % (count, ["%#x" % e for e in entries[:12]]))
        self.check("newgameplus: the learned skills stay sorted and each character's own",
                   list(entries) == sorted(set(entries)) and all(e >> 10 for e in entries),
                   "%d learned skills" % count)

    def test_gallery(self):
        """The Gallery (Documentation/Features/Gallery.md). The prologue's opening shows Mountain Forest (background
        0x1D), Flight from Renais (0xDB) and Seth Attacked (0x3A): all three unlock, in the SRAM entry of the trial
        map's save block (magic "GLRY"). After a reset, Extras has the Gallery row; it shows its first three rows. A on
        a locked CG does nothing; on Mountain Forest it shows the CG full size on BG3; right goes to the next unlocked
        one (Flight from Renais, row 13), right again to Seth Attacked (around the end); B goes back to the grid on it. Down three rows scrolls the grid a row, on screen (no fade), and B goes back
        to Extras."""
        d = os.path.join(self.out, "gallery")
        os.makedirs(d, exist_ok=True)
        need = ("gGalleryScreen", "gLCDControlBuffer", "ProcScr_SaveMenu", "sProcArray")
        if not all(n in self.sym for n in need):
            return self.check("gallery: the Gallery symbols", False, "no symbols")
        gs, lcd, procs = self.sym["gGalleryScreen"], self.sym["gLCDControlBuffer"], self.sym["sProcArray"]
        unlocks = SRAM + 0x64 + 6 * 0x10  # gSram->saveBlockInfo[SAVE_ID_XMAP]
        # struct GalleryScreen: count, cursor, top, view; sprites (+8); the first unlock byte (+0xC)
        state = ["r8 %#x" % (gs + k) for k in (0, 1, 2, 3, 8, 0xC)]
        lines = self.boot_lines() + ["r8 %#x" % (unlocks + k) for k in range(6)]
        # the main menu: Extras is the last row; in Extras the Gallery is third from the end (then the Bestiary and
        # the Skill Codex): UP from the top goes around to the Skill Codex, UP twice more to the Gallery
        lines += SOFT_RESET + ["press UP", "wait 20", "press A", "wait 200",
                               "dump %#x %#x menu.bin" % (procs, PROC_SIZE * PROCS)]
        lines += ["press UP", "wait 20"] * 3 + ["press A", "wait 150", "shot grid.png"] + state
        # the cursor on the first CG (Myrrh & Morva, locked): A; then Mountain Forest (one down): A, the full view
        lines += ["press A", "wait 60"] + state + ["press DOWN", "wait 20", "press A", "wait 60", "shot full.png",
                                                    "r16 %#x" % lcd] + state
        lines += ["press RIGHT", "wait 60", "shot next.png"] + state + ["press RIGHT", "wait 60"] + state
        lines += ["press B", "wait 60", "shot back.png"] + state
        # Seth Attacked (row 0): down three rows, past the third on screen; the grid scrolls a row
        lines += ["press DOWN", "wait 20", "press DOWN", "wait 20", "press DOWN", "wait 3", "shot scrolling.png",
                  "wait 30", "shot scrolled.png"] + state
        lines += ["press B", "wait 150", "shot extras.png", "dump %#x %#x after.bin" % (procs, PROC_SIZE * PROCS)]
        vals = self.run(ROM, lines, d)
        for name, label in (("grid", "the Gallery's first rows"), ("full", "the full view: Mountain Forest"),
                            ("next", "right: the next unlocked CG"), ("back", "B: back on the grid"),
                            ("scrolling", "down past the third row: scrolling"), ("scrolled", "a row down"),
                            ("extras", "B: back to Extras")):
            self.shot(os.path.join(d, name + ".png"), label)
        if len(vals) != 6 + 7 * 6 + 1:
            return self.check("gallery: the Gallery runs", False, "values %s" % vals)
        sram, dispcnt = vals[:6], vals[18]
        grid, locked, full, nxt, around, back, scrolled = (vals[6:12], vals[12:18], vals[19:25], vals[25:31],
                                                           vals[31:37], vals[37:43], vals[43:49])
        magic = sram[0] | sram[1] << 8 | sram[2] << 16 | sram[3] << 24
        self.check("gallery: the prologue's opening unlocks Mountain Forest, Seth Attacked and Flight from Renais",
                   magic == 0x59524C47 and sram[4] & 0x0C == 0x0C and sram[5] & 0x20,
                   "magic %#x, bits %#x %#x" % (magic, sram[4], sram[5]))
        menu = [p[0x32] for a, p in self.procs(os.path.join(d, "menu.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("gallery: Extras has the Gallery row", menu and menu[0] & (1 << 4),
                   "Extras rows %s" % ["%#x" % m for m in menu])
        self.check("gallery: the grid opens on its first rows", grid[0] == 24 and grid[2] == 0 and grid[4] == 1 and
                   grid[5] & 0x0C == 0x0C, "count, cursor, top, view, sprites, bits %s" % grid)
        self.check("gallery: A on a locked CG does nothing", locked[4] == 1 and locked[1] == 0,
                   "cursor %d, grid shown %d" % (locked[1], locked[4]))
        # the other BGs' maps are cleared and no sprite is put (the fade turns every layer on)
        self.check("gallery: A on Mountain Forest shows it full size (BG3, no thumbnails)",
                   full[3] == 3 and full[4] == 0 and dispcnt & 0x0800, "view %d, DISPCNT %#x" % (full[3], dispcnt))
        self.check("gallery: right goes to the next unlocked CG (Flight from Renais), right again around the end "
                   "(Seth Attacked)", nxt[3] == 13 and around[3] == 2, "views %d, %d" % (nxt[3], around[3]))
        self.check("gallery: B goes back to the grid on that CG", back[1] == 2 and back[4] == 1,
                   "cursor %d, grid shown %d" % (back[1], back[4]))
        self.check("gallery: down past the third row scrolls the grid a row, on screen",
                   scrolled[1] == 11 and scrolled[2] == 1 and scrolled[4] == 1 and
                   self.diff(os.path.join(d, "back.png"), os.path.join(d, "scrolling.png")) > 0,
                   "cursor %d, top %d, grid shown %d" % (scrolled[1], scrolled[2], scrolled[4]))
        after = [p for a, p in self.procs(os.path.join(d, "after.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("gallery: B again goes back to the save menu", bool(after))

    def test_bestiary(self):
        """The Bestiary (Documentation/Features/Bestiary.md). The prologue's first phase marks every unit on the map in
        gBestiarySeen: Eirika (1), Seth (2) and O'Neill (0x68); the Lord (2), Paladin (7) and Fighter (0x3F) classes.
        The debugger's "Save & restart ch." writes them to the game save's tail. After a reset, Extras > Bestiary (above
        the last row, the Skill Codex) opens on Characters with those three met; Ephraim (the second row) is locked; A on
        Eirika shows her growths; R changes to Classes (Lord, Paladin, Fighter met); B goes back to Extras."""
        d = os.path.join(self.out, "bestiary")
        os.makedirs(d, exist_ok=True)
        need = ("gBestiarySeen", "gBestiaryScreen", "ProcScr_SaveMenu", "sProcArray")
        if not all(n in self.sym for n in need):
            return self.check("bestiary: the Bestiary symbols", False, "no symbols")
        seen, bs, procs = self.sym["gBestiarySeen"], self.sym["gBestiaryScreen"], self.sym["sProcArray"]
        # struct BestiaryScreen: tab, cursor, top, page, count, met (+0..5); the rows (+0x4C) and which are met (+0xCC)
        state = lambda name: "dump %#x 0xE0 %s" % (bs, name)
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)
        lines = self.boot_lines() + ["dump %#x 0x44 seen.bin" % seen] + DEBUG_SAVE + SOFT_RESET
        # Extras: the Bestiary is above the last row (the Skill Codex)
        lines += ["press UP", "wait 20", "press A", "wait 200", dump("menu.bin"), "press UP", "wait 20", "press UP",
                  "wait 20", "press A", "wait 150", "shot chars.png", state("chars.bin")]
        lines += ["hold DOWN 4", "wait 20", "shot locked.png", state("locked.bin"), "hold UP 4", "wait 20", "press A",
                  "wait 20", "shot growths.png", state("growths.bin"), "press R", "wait 30", "shot classes.png",
                  state("classes.bin"), "press B", "wait 150", "shot extras.png", dump("after.bin")]
        self.run(ROM, lines, d)
        for name, label in (("chars", "Bestiary: Characters"), ("locked", "a character not met"),
                            ("growths", "A: Eirika's growths"), ("classes", "R: Classes"), ("extras", "B: Extras")):
            self.shot(os.path.join(d, name + ".png"), label)
        try:
            ram = open(os.path.join(d, "seen.bin"), "rb").read()
            st = {n: open(os.path.join(d, n + ".bin"), "rb").read() for n in ("chars", "locked", "growths", "classes")}
        except OSError as e:
            return self.check("bestiary: the Bestiary runs", False, str(e))
        bit = lambda bits, i: (bits[i >> 3] >> (i & 7)) & 1
        chars, classes = ram[4:36], ram[36:68]
        self.check("bestiary: the first phase marks Eirika, Seth and O'Neill, and the Lord, Paladin and Fighter",
                   ram[:4] == b"BSTY" and all(bit(chars, p) for p in (1, 2, 0x68)) and
                   all(bit(classes, j) for j in (2, 7, 0x3F)), "magic %r, chars %s, classes %s" %
                   (ram[:4], chars.hex(), classes.hex()))
        menu = [p[0x32] for a, p in self.procs(os.path.join(d, "menu.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("bestiary: Extras has the Bestiary row", menu and menu[0] & (1 << 6),
                   "Extras rows %s" % ["%#x" % m for m in menu])
        c = st["chars"]
        met = lambda b, row: (b[0xCC + (row >> 3)] >> (row & 7)) & 1
        self.check("bestiary: after a reset the save's three characters are met, Eirika first",
                   c[0] == 0 and c[5] == 3 and c[0x4C] == 1 and met(c, 0), "tab %d, met %d, first row %#x" %
                   (c[0], c[5], c[0x4C]))
        self.check("bestiary: the second row (Ephraim) is not met", st["locked"][1] == 1 and
                   st["locked"][0x4D] == 0x0F and not met(st["locked"], 1), "cursor %d, row %#x" %
                   (st["locked"][1], st["locked"][0x4D]))
        self.check("bestiary: A shows growths", st["growths"][1] == 0 and st["growths"][3] == 1,
                   "cursor %d, page %d" % (st["growths"][1], st["growths"][3]))
        k = st["classes"]
        self.check("bestiary: R shows the classes, three met", k[0] == 1 and k[5] == 3,
                   "tab %d, met %d" % (k[0], k[5]))
        after = [p for a, p in self.procs(os.path.join(d, "after.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("bestiary: B goes back to the save menu", bool(after))

    def test_skillcodex(self):
        """The Skill Codex (Documentation/Features/SkillCodex.md). The prologue's first phase marks the skills of every
        unit on the map in gSkillCodexSeen (Eirika's, Seth's, O'Neill's and the soldiers'). The debugger's "Save &
        restart ch." writes them to the game save's tail. After a reset, Extras > Skill Codex (the last row) lists every
        skill with a description, as many met as were marked; the cursor starts on the first; R goes to the first skill
        met, then the next; DOWN moves a row; B goes back to the save menu."""
        d = os.path.join(self.out, "skillcodex")
        os.makedirs(d, exist_ok=True)
        need = ("gSkillCodexSeen", "gBestiaryScreen", "ProcScr_SaveMenu", "sProcArray")
        if not all(n in self.sym for n in need):
            return self.check("skillcodex: the Skill Codex symbols", False, "no symbols")
        seen, cs, procs = self.sym["gSkillCodexSeen"], self.sym["gBestiaryScreen"], self.sym["sProcArray"]
        # struct SkillCodexScreen (in gBestiaryScreen): cursor, top, count, met (u16 each)
        state = lambda name: "dump %#x 8 %s" % (cs, name)
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)
        lines = self.boot_lines() + ["dump %#x 0x84 seen.bin" % seen] + DEBUG_SAVE + SOFT_RESET
        lines += ["press UP", "wait 20", "press A", "wait 200", dump("menu.bin"), "press UP", "wait 20", "press A",
                  "wait 150", "shot codex.png", state("open.bin")]
        lines += ["press R", "wait 30", "shot met.png", state("met.bin"), "press R", "wait 30", state("next.bin"),
                  "hold DOWN 4", "wait 20", "shot down.png", state("down.bin"), "press B", "wait 150",
                  "shot extras.png", dump("after.bin")]
        self.run(ROM, lines, d)
        for name, label in (("codex", "Skill Codex: the first rows"), ("met", "R: the first skill met"),
                            ("down", "DOWN: a row on"), ("extras", "B: Extras")):
            self.shot(os.path.join(d, name + ".png"), label)
        try:
            ram = open(os.path.join(d, "seen.bin"), "rb").read()
            st = {n: struct.unpack("<4H", open(os.path.join(d, n + ".bin"), "rb").read()[:8])
                  for n in ("open", "met", "next", "down")}
        except (OSError, struct.error) as e:
            return self.check("skillcodex: the Skill Codex runs", False, str(e))
        marked = [i for i in range(1, 0x3FF) if (ram[4 + (i >> 3)] >> (i & 7)) & 1]
        self.check("skillcodex: the first phase marks the units' skills", ram[:4] == b"SCDX" and len(marked) > 0,
                   "magic %r, %d skills: %s" % (ram[:4], len(marked), ["%#x" % m for m in marked[:12]]))
        menu = [p[0x32] for a, p in self.procs(os.path.join(d, "menu.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("skillcodex: Extras has the Skill Codex row", menu and menu[0] & (1 << 7),
                   "Extras rows %s" % ["%#x" % m for m in menu])
        cur, top, count, met = st["open"]
        self.check("skillcodex: after a reset it lists the skills, the marked ones met, the cursor on the first",
                   count > 100 and 0 < met <= len(marked) and cur == top, "cursor %#x, top %#x, %d of %d met" %
                   (cur, top, met, count))
        listed = [m for m in marked if m >= cur]
        self.check("skillcodex: R goes to the first skill met, then the next", len(listed) >= 2 and
                   st["met"][0] == listed[0] and st["next"][0] == listed[1],
                   "R %#x, R %#x, marked %s" % (st["met"][0], st["next"][0], ["%#x" % m for m in listed[:3]]))
        self.check("skillcodex: DOWN moves a row on", st["down"][0] > st["next"][0],
                   "cursor %#x after %#x" % (st["down"][0], st["next"][0]))
        after = [p for a, p in self.procs(os.path.join(d, "after.bin")) if a == self.sym["ProcScr_SaveMenu"]]
        self.check("skillcodex: B goes back to the save menu", bool(after))

    def test_saveguard(self):
        """The save layout guard (#711): the debugger's "Save & restart ch." writes a game save whose block checksum is
        SramChecksum32 (0x2A) xor the layout signature; with that checksum put back to a bare 0x2A (a save from before
        the guard, or a build whose signature differs), the title's save menu offers no Restart Chapter."""
        d = os.path.join(self.out, "saveguard")
        os.makedirs(d, exist_ok=True)
        need = ("gEmsSavChunks", "gEmsSusChunks", "gEmsSizes", "gEmsOffsets", "ProcScr_SaveMenu", "sProcArray")
        if not all(n in self.sym for n in need):
            return self.check("saveguard: the chunk tables", False, "no symbols")
        procs = self.sym["sProcArray"]
        info = SRAM + 0x64  # SaveBlockInfo[SAVE_ID_GAME0]: checksum32 at +0x0C (SRAM reads a byte at a time)
        dump = lambda name: "dump %#x %#x %s" % (procs, PROC_SIZE * PROCS, name)
        lines = self.boot_lines() + DEBUG_SAVE + ["r8 %#x" % (info + 0xC + i) for i in range(4)]
        lines += SOFT_RESET + [dump("valid.bin")] + ["w8 %#x %d" % (info + 0xC + i, (0x2A >> (8 * i)) & 0xFF)
                                                     for i in range(4)]
        lines += SOFT_RESET + [dump("old.bin"), "shot old.png"]
        vals = self.run(ROM, lines, d)
        self.shot(os.path.join(d, "old.png"), "a save from another layout")
        sig = self.layout_signature()
        got = sum(v << (8 * i) for i, v in enumerate(vals[:4])) if len(vals) == 4 else None
        self.check("saveguard: a game save's checksum carries the layout signature", got == 0x2A ^ sig,
                   "checksum %s, signature %#x" % ("%#x" % got if got is not None else "?", sig))
        menu = lambda name: [p[0x30] for a, p in self.procs(os.path.join(d, name)) if a == self.sym["ProcScr_SaveMenu"]]
        valid, old = menu("valid.bin"), menu("old.bin")
        restart = 1 << 1  # MAIN_MENU_OPTION_RESTART
        self.check("saveguard: a save from another layout is refused (no Restart Chapter)",
                   valid and old and valid[0] & restart and not old[0] & restart,
                   "main menu options %s, then %s" % (["%#x" % v for v in valid], ["%#x" % v for v in old]))

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

    def test_lvupanim(self, boot_ss):
        """The level-up quote with battle animations on (EkrLvup_Quote, Kernel/Wizardry/Lvupfx/Lvupfx/MapLvup.c): Eirika
        attacks with 99 EXP and animations on (option byte 0x42). After the stats, the talk prints her quote into BG0
        rows below the screen and it is copied to BG2's rows 2-3 (the box's text row); the box is on BG1 rows 1-5.
        Then the window slides off and the battle ends on the map (before, the quote's jump overwrote the next step
        and the game froze there)."""
        d = os.path.join(self.out, "lvupanim")
        os.makedirs(d, exist_ok=True)
        need = ("gBG1TilemapBuffer", "gBG2TilemapBuffer", "InBattleMainRoutine")
        if not all(n in self.sym for n in need):
            return self.check("lvupanim: symbols", False, "missing %s" % [n for n in need if n not in self.sym])
        lvl = UNIT1 + 0x08
        attack = EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press A"]
        bg1, bg2 = os.path.join(d, "bg1.bin"), os.path.join(d, "bg2.bin")
        vals = self.run(ROM, ["load " + boot_ss, "w8 %#x 0" % (PLAYST + 0x42), "w8 %#x 99" % (UNIT1 + 0x09),
                              "r8 %#x" % lvl] + attack +
                        ["wait 1080", "shot quote.png", "dump %#x 0x180 %s" % (self.sym["gBG1TilemapBuffer"], bg1),
                         "dump %#x 0x100 %s" % (self.sym["gBG2TilemapBuffer"], bg2), "r32 %#x" % GFACES,
                         "r8 %#x" % (EVENT_SLOTS + 9 * 4), "save quote.ss", "wait 600", "shot after.png",
                         "r32 0x02024CB8", "r8 %#x" % lvl], d)
        self.shot(os.path.join(d, "quote.png"), "level-up quote (animations)")
        self.shot(os.path.join(d, "after.png"), "after the animated level-up")
        if len(vals) != 5:
            return self.check("lvupanim: the fight runs", False, "values %s" % vals)
        lv0, face, gains, main, lv1 = vals
        tm = lambda f, y: struct.unpack_from("<32H", open(f, "rb").read(), 64 * y) if os.path.exists(f) else ()
        text = sum(1 for y in (2, 3) for e in tm(bg2, y) if e)
        box = sum(1 for y in range(1, 6) for e in tm(bg1, y) if e)
        self.check("lvupanim: the quote is printed in a box above the window", text >= 8 and box >= 20,
                   "%d text cells, %d box cells" % (text, box))
        self.check_quote_face(face, gains, os.path.join(d, "quote.ss"), d, "lvupanim")
        self.check("lvupanim: the window goes and the battle ends on the map",
                   main != self.sym["InBattleMainRoutine"] + 1 and lv1 == lv0 + 1,
                   "main loop %#x, level %d to %d" % (main, lv0, lv1))

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
        # The line height preset of the font in configs.h (TEXT_ENGINE_LINE_HEIGHT in TextEngineRework.c): Mother 3's
        # lines are 12 pixels apart, so the box is 4 rows a line shorter (BG1 stepped by that below the box's inside)
        if "sTextEngineLineSpacing" in self.sym:
            configs = open(os.path.join(ROOT, "include", "configs", "configs.h")).read()
            height = 12 if re.search(r"^#define CONFIG_FONT_MOTHER_3\b", configs, re.M) else 16
            sp = self.sym["sTextEngineLineSpacing"]
            vals = self.run(rom, ["load " + os.path.join(d, "battlequote.ss"), "r32 0x0859133C"], d)
            vals = self.run(rom, ["load " + os.path.join(d, "battlequote.ss"), "r8 %#x" % (vals[0] + 0x0A),
                                  "r8 %#x" % (sp + 0xA4), "r8 %#x" % (sp + 0xA3)], d) if vals else []
            if height == 16:
                self.check("battlequote: 16-pixel lines (the font's preset): no line spacing",
                           len(vals) == 3 and vals[1] == 0, "values %s" % vals)
            else:
                self.check("battlequote: %d-pixel lines (the font's preset): the box is shorter" % height,
                           len(vals) == 3 and vals[1] == 1 and vals[2] == vals[0] * (16 - height),
                           "lines %s, active %s, box %s rows shorter" % tuple(vals + ["?"] * (3 - len(vals))))

    def test_deeproots(self, boot_ss):
        """Deep Roots (Kernel/Wizardry/SkillEffects/MenuSkills/DeepRoots.c). The prologue's forests are (9, 6), (10, 7),
        (13, 7), (14, 8) and (2, 0); Eirika starts on plains at (4, 5) with Mov 5. Given the skill in RAM and put on
        plains at (8, 5), she walks into the forest at (9, 6): her menu is Skills, Item, Wait. Skills, Deep Roots and the
        first target hop her to another forest within 5 tiles, and she is unmoved there (state 0). Without moving she
        hops again (Skills is the second row: O'Neill is next to (10, 7)), then Wait ends her turn (state 0x42)."""
        d = os.path.join(self.out, "deeproots")
        os.makedirs(d, exist_ok=True)
        sid = self.skill_id("SID_DeepRoots")
        if sid is None:
            return self.check("deeproots: SID_DeepRoots exists", False)
        forests = {(9, 6), (10, 7), (13, 7), (14, 8)}
        skills = UNIT1 + 0x32
        vals = self.run(ROM, ["load " + boot_ss, "r32 %#x" % BM_MAP_UNIT] + ["r8 %#x" % (skills + k) for k in range(5)], d)
        rows, packed = vals[0], sum(v << (8 * k) for k, v in enumerate(vals[1:]))
        slot = next((k for k in range(4) if (packed >> (10 * k)) & 0x3FF == 0), None)
        if slot is None:
            return self.check("deeproots: Eirika has a free skill slot", False)
        packed |= sid << (10 * slot)
        row5 = self.run(ROM, ["load " + boot_ss, "r32 %#x" % (rows + 4 * 5)], d)[0]
        give = ["w8 %#x %#x" % (skills + k, (packed >> (8 * k)) & 0xFF) for k in range(5)]
        pos = ["r8 %#x" % (UNIT1 + 0x10), "r8 %#x" % (UNIT1 + 0x11), "r32 %#x" % (UNIT1 + 0x0C)]

        # On plains: no Skills row (the menu starts Attack, Pair Up, ...; a Skills row would come first)
        self.run(ROM, ["load " + boot_ss] + give + ["press A", "wait 30", "press A", "wait 30", "shot plains.png"], d)
        self.shot(os.path.join(d, "plains.png"), "Deep Roots: on plains, no Skills row")

        lines = ["load " + boot_ss] + give + ["w8 %#x 8" % (UNIT1 + 0x10), "w8 %#x 5" % (UNIT1 + 0x11),
                                              "w8 %#x 0" % (row5 + 4), "w8 %#x 1" % (row5 + 8)]
        lines += ["press RIGHT", "wait 8"] * 4 + ["wait 20", "press A", "wait 30", "press RIGHT", "wait 8", "press DOWN",
                                                  "wait 28", "press A", "wait 40", "shot menu.png"]
        lines += ["press A", "wait 30", "press A", "wait 40", "shot target.png", "press A", "wait 260",
                  "shot hop1.png"] + pos
        lines += ["press A", "wait 30", "press A", "wait 30", "press DOWN", "wait 10", "press A", "wait 30", "press A",
                  "wait 40", "press DOWN", "wait 20", "press A", "wait 260", "shot hop2.png"] + pos
        lines += ["press A", "wait 30", "press A", "wait 30", "press UP", "wait 10", "press A", "wait 60",
                  "shot wait.png", "r32 %#x" % (UNIT1 + 0x0C)]
        got = self.run(ROM, lines, d)
        for f, label in (("menu.png", "her menu in the forest"), ("target.png", "choosing a forest"),
                         ("hop1.png", "after the first hop"), ("hop2.png", "after the second hop"),
                         ("wait.png", "after Wait")):
            self.shot(os.path.join(d, f), "Deep Roots: " + label)
        if len(got) != 7:
            return self.check("deeproots: the hops run", False, "values %s" % got)
        x1, y1, st1, x2, y2, st2, st3 = got
        self.check("deeproots: the first hop lands on another forest within Mov, unmoved",
                   (x1, y1) in forests and (x1, y1) != (9, 6) and abs(x1 - 9) + abs(y1 - 6) <= 5 and not st1 & 0x42,
                   "at (%d, %d), state %#x" % (x1, y1, st1))
        self.check("deeproots: a second hop from there, still unmoved",
                   (x2, y2) in forests and (x2, y2) != (x1, y1) and not st2 & 0x42,
                   "at (%d, %d), state %#x" % (x2, y2, st2))
        self.check("deeproots: Wait then ends her turn", st3 & 0x42 == 0x42, "state %#x" % st3)

    def test_ch1end(self, boot_ss):
        """Chapter 1's end: the debugger restarts at Chapter 1 (as boot-to-prep.txt does Chapter 4) and skips its
        opening; the boss off the gate, Eirika on it seizes. The ending names 0x38 (the Intermission at Castle Frelia,
        NEXT_CHAPTER_WITH_MAP), which is also the side-map return: the world map must take it as the story chapter, so
        the Intermission plays and the story goes on to Chapter 2, not back to the prologue."""
        d = os.path.join(self.out, "ch1end")
        os.makedirs(d, exist_ok=True)
        story = self.sym.get("gWmStoryChapter")
        if story is None:
            return self.check("ch1end: gWmStoryChapter's address", False)
        debug = ["w8 %#x 1" % CHAPTER, "press B", "wait 60", "press RIGHT", "wait 40"] + ["press DOWN", "wait 10"] * 2 + \
            ["press A", "wait 60"] + ["press DOWN", "wait 10"] * 7 + ["press START", "wait 200", "press A", "wait 120",
                                                                      "press A", "wait 1000"]
        lines = ["load " + boot_ss] + debug + ["press START", "wait 200"] * 8 + ["wait 300", "shot ch1.png",
                                                                                 "r32 %#x" % BM_MAP_UNIT, "save ch1.ss"]
        vals = self.run(ROM, lines, d)
        self.shot(os.path.join(d, "ch1.png"), "Chapter 1's first turn")
        if not vals:
            return self.check("ch1end: Chapter 1 starts", False)
        rows = self.run(ROM, ["load " + os.path.join(d, "ch1.ss")] + ["r32 %#x" % (vals[0] + 4 * y) for y in (2, 9)] +
                        ["r8 %#x" % (UNIT_RED + 0x32 + k) for k in range(7)], d)
        if len(rows) != 9:
            return self.check("ch1end: Chapter 1 starts", False)
        # The boss, Breguet (lance rank C), holds Lance Prowess Lv2 only: the rank skills walked again from E (the
        # load, then the battle and prep syncs) must not give Lv1 back next to it (WRankSkill.c)
        packed = sum(v << (8 * k) for k, v in enumerate(rows[2:]))
        held = [(packed >> (10 * k)) & 0x3FF for k in range(5)]
        lv1, lv2 = self.skill_id("SID_LanceProwess1"), self.skill_id("SID_LanceProwess2")
        self.check("ch1end: the boss has Lance Prowess Lv2 and not Lv1 beside it",
                   lv1 is not None and lv2 in held and lv1 not in held, "skills %s" % ["%#x" % h for h in held])
        rows = rows[:2]
        # The boss (0x81) stands on the gate (2, 2); Eirika starts at (12, 9) under the cursor
        lines = ["load " + os.path.join(d, "ch1.ss"), "w32 %#x 0" % UNIT_RED, "w8 %#x 1" % (rows[0] + 2),
                 "w8 %#x 0" % (rows[1] + 12), "w8 %#x 2" % UNIT_X, "w8 %#x 2" % UNIT_Y]
        lines += ["press LEFT", "wait 8"] * 10 + ["press UP", "wait 8"] * 7 + ["wait 20", "press A", "wait 30", "press A",
                                                                              "wait 40", "shot seize.png", "press A",
                                                                              "wait 60"]
        for i in range(24):
            lines += ["r8 %#x" % CHAPTER, "r8 %#x" % story, "press START", "wait 50", "press A", "wait 50"]
        lines += ["shot after.png"]
        vals = self.run(ROM, lines, d)
        self.shot(os.path.join(d, "seize.png"), "Chapter 1: Eirika on the gate")
        self.shot(os.path.join(d, "after.png"), "after Chapter 1")
        seen = list(zip(vals[0::2], vals[1::2]))
        self.check("ch1end: the Intermission (0x38) becomes the story chapter, then Chapter 2 starts",
                   (0x38, 0x38) in seen and seen[-1:] == [(CHAPTER_02, CHAPTER_02)] and all(c != 0 for c, _ in seen),
                   "(chapter, story) %s" % sorted(set(seen), key=seen.index))

    def test_mapcolors(self, boot_ss):
        """Map Colors (Kernel/Wizardry/MapColors/Source/MapColors.c): the Options row sets gPlaySt.config.map_colors
        (bits 2-3 of PLAYST + 0x47; 0 Vanilla, 1 Vivid, 2 Deep). Leaving Options redraws the map and recolours the map
        palettes (BG banks 6-15, 160 colours) by the transform in the source: Vivid is saturation 22 and contrast 17,
        Deep is saturation 26, contrast 19 and a curve. Each colour is checked against that model of vanilla's palette.
        Then a test ROM with Chapter 18's palette animation on the prologue: the animated colours cycle through the
        frames in its ROM data, and with Deep set they are those frames recoloured."""
        d = os.path.join(self.out, "mapcolors")
        os.makedirs(d, exist_ok=True)
        base = PALETTE_BUFFER + 0xC0
        vanilla = self.run(ROM, ["load " + boot_ss] + ["r16 %#x" % (base + 2 * k) for k in range(160)], d)
        if len(vanilla) != 160:
            return self.check("mapcolors: the map palette", False, "read %d colours" % len(vanilla))
        curve = [0, 0, 1, 2, 2, 3, 4, 5, 6, 7, 8, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26,
                 27, 29, 30, 31]

        def model(c, mode):
            r, g, b = c & 31, (c >> 5) & 31, (c >> 10) & 31
            luma = (r * 77 + g * 150 + b * 29) >> 8
            sat, con = (22, 17) if mode == 1 else (26, 19)
            out = []
            for ch in (r, g, b):
                ch = luma + (((ch - luma) * sat) >> 4)
                ch = 16 + (((ch - 16) * con) >> 4)
                ch = min(31, max(0, ch))
                if mode == 2:
                    ch = curve[ch]
                out.append(ch)
            return out[0] | (out[1] << 5) | (out[2] << 10)

        route = ["press RIGHT", "wait 10", "press RIGHT", "wait 10", "press UP", "wait 10", "press A", "wait 30"]
        route += ["press DOWN", "wait 8"] * 4 + ["press A", "wait 90"]
        for mode in (1, 2):
            name = "Vivid" if mode == 1 else "Deep"
            lines = ["load " + boot_ss] + route + ["press DOWN", "wait 10"] * 14
            lines += ["press RIGHT", "wait 20"] * mode
            lines += ["shot row%d.png" % mode, "press B", "wait 120", "shot map%d.png" % mode,
                      "r8 %#x" % (PLAYST + 0x47)] + ["r16 %#x" % (base + 2 * k) for k in range(160)]
            vals = self.run(ROM, lines, d)
            self.shot(os.path.join(d, "row%d.png" % mode), "Map Colors: %s (Options row)" % name)
            self.shot(os.path.join(d, "map%d.png" % mode), "Map Colors: %s (map)" % name)
            if len(vals) != 161:
                self.check("mapcolors: the Options row sets %s" % name, False, "read %d values" % len(vals))
                continue
            value = (vals[0] >> 2) & 3
            self.check("mapcolors: the Options row sets %s" % name, value == mode, "map_colors %d" % value)
            pal = vals[1:]
            bad = sum(1 for k in range(160) if pal[k] != model(vanilla[k], mode))
            changed = sum(1 for k in range(160) if pal[k] != vanilla[k])
            self.check("mapcolors: the map is redrawn in %s after Options" % name,
                       len(pal) == 160 and bad == 0 and changed > 0,
                       "%d colours changed, %d off the model" % (changed, bad))

        # Palette animations (BMapVSync_UpdateMapPalAnimations, rewritten in MapColors.c): a test ROM gives the prologue
        # Chapter 18's (gChapterDataTable 0x088B0890, 0x94 bytes a chapter, palette animation id at +10), which cycles
        # palette entries 0x9C-0x9F through frames in its ROM data. A normal boot starts it; Vanilla shows the frames,
        # Deep (set in RAM: the next frame of the animation applies it) their recoloured versions.
        data = bytearray(open(ROM, "rb").read())
        data[0x8B0890 + 10] = CH18_PAL_ANIM
        rom = os.path.join(d, "mapanim.gba")
        open(rom, "wb").write(data)
        frames = set()
        if "SplitedChapterData_TileAnim" not in self.sym:
            return self.check("mapcolors: the map animation table", False, "no symbols")
        table = self.sym["SplitedChapterData_TileAnim"] - 0x08000000  # CustomCampaign/Event/PListSplit.event
        anim = struct.unpack_from("<I", data, table + 4 * CH18_PAL_ANIM)[0] - 0x08000000
        while data[anim + 4]:
            src, count, index = struct.unpack_from("<I", data, anim)[0] - 0x08000000, data[anim + 5], data[anim + 6]
            frames |= {(index + k, struct.unpack_from("<H", data, src + 2 * k)[0]) for k in range(count)}
            anim += 8
        entries = sorted({e for e, _ in frames})
        reads = [("r16 %#x" % (base + 2 * e)) for e in entries]
        ss = os.path.join(d, "mapanim.ss")
        got = self.run(rom, self.boot_lines() + ["save " + ss] + (["wait 7"] + reads) * 12, d)
        seen = {(entries[i % len(entries)], v) for i, v in enumerate(got)}
        moving = len({v for _, v in seen}) > len(entries)
        self.check("mapcolors: a map palette animation runs (Vanilla)", len(got) == 12 * len(entries) and moving and
                   seen <= frames, "%d samples, %d values" % (len(got), len({v for _, v in seen})))
        lines = ["load " + ss, "w8 %#x 8" % (PLAYST + 0x47), "wait 30"] + (["wait 7"] + reads) * 12
        got = self.run(rom, lines, d)
        deep = {(e, model(c, 2)) for e, c in frames}
        seen = {(entries[i % len(entries)], v) for i, v in enumerate(got)}
        self.check("mapcolors: the animated colours are recoloured (Deep)", len(got) == 12 * len(entries) and
                   seen <= deep and len({v for _, v in seen}) > 1, "%d of %d samples off the model" %
                   (len(seen - deep), len(seen)))

    def test_expbar(self, boot_ss):
        """EXP bar styles: the Options value (gPlaySt.config.exp_bar_style, 4 bits from bit 22 of the second option
        word) set in RAM, then a battle that ends in the EXP bar, on the map and with battle animations. The map bar
        loads the style's palette into bank 5, the battle one into bank 1 with its window tiles at VRAM 0x6002000.
        Vanilla (0) keeps vanilla's palette in battle."""
        d = os.path.join(self.out, "expbar")
        os.makedirs(d, exist_ok=True)
        gfx = os.path.join(ROOT, "Kernel", "Wizardry", "ExpBarStyles", "Gfx")
        order = []
        src = open(os.path.join(ROOT, "Kernel", "Wizardry", "ExpBarStyles", "Source", "ExpBarStyles.c")).read()
        for m in re.finditer(r"^\tEXP_BAR_STYLE\((\w+),", src, re.M):
            order.append(m.group(1))
        if not order:
            return self.check("expbar: the style table", False, "no EXP_BAR_STYLE rows")
        b36, b37 = self.run(ROM, ["load " + boot_ss, "r8 %#x" % (PLAYST + 0x46), "r8 %#x" % (PLAYST + 0x47)], d)
        attack = EIRIKA_MENU + ["press A", "wait 40", "press A", "wait 40", "press A"]
        vanilla_pal = list(struct.unpack_from("<16H", open(os.path.join(ROOT, "fe8.gba"), "rb").read(), 0x803590))

        def style(v):
            return ["w8 %#x %#x" % (PLAYST + 0x46, (b36 & 0x3F) | ((v & 3) << 6)),
                    "w8 %#x %#x" % (PLAYST + 0x47, (b37 & 0xFC) | (v >> 2))]

        for v in (1, len(order), 0):
            name = order[v - 1] if v else "Vanilla"
            pal = vanilla_pal if not v else list(struct.unpack("<16H", open(os.path.join(gfx, name + ".gbapal"), "rb").read()))
            # map: 99 EXP, so the bar shows; battle: animations on (option byte 0x42, animationType 0)
            got = self.run(ROM, ["load " + boot_ss, "w8 %#x 99" % (UNIT1 + 0x09)] + style(v) + attack +
                           ["wait 228", "shot map_%s.png" % name] +
                           ["r16 %#x" % (PALETTE_BUFFER + 0x20 * 5 + 2 * k) for k in range(16)], d)
            self.shot(os.path.join(d, "map_%s.png" % name), "EXP bar: %s (map)" % name)
            if v:
                self.check("expbar: the map EXP bar has %s's palette" % name, got[1:] == pal[1:],
                           "bank 5 %s" % " ".join("%04x" % c for c in got[1:4]))
            dump = os.path.join(d, "chr_%s.bin" % name)
            got = self.run(ROM, ["load " + boot_ss, "w8 %#x 0" % (PLAYST + 0x42), "w8 %#x 60" % (UNIT1 + 0x09)] +
                           style(v) + attack + ["wait 510", "shot battle_%s.png" % name, "dump 0x6002000 416 " + dump] +
                           ["r16 %#x" % (PALETTE_BUFFER + 0x20 + 2 * k) for k in range(16)], d)
            self.shot(os.path.join(d, "battle_%s.png" % name), "EXP bar: %s (battle)" % name)
            self.check("expbar: the battle EXP bar has %s's palette" % name, got[1:] == pal[1:],
                       "bank 1 %s" % " ".join("%04x" % c for c in got[1:4]))
            if v:
                want = open(os.path.join(gfx, name + ".4bpp"), "rb").read()[:416]
                have = open(dump, "rb").read() if os.path.exists(dump) else b""
                self.check("expbar: the battle EXP bar has %s's window tiles" % name, have == want,
                           "%d of 13 tiles" % sum(have[i:i + 32] == want[i:i + 32] for i in range(0, 416, 32)))

    def test_bigunit(self, boot_ss):
        """2x2 units (Documentation/Features/BigUnits.md). A test ROM lists O'Neill: he loads as four corners on a 2x2
        block, drawn once at twice the size (an affine double-size OBJ). Seth breaks his top-left corner: it leaves the
        group, and his defeat (flag 2) waits for the last corner. On the plain ROM, the fighter next to Eirika made a
        2x2 block in RAM: at (2, 3) only its bottom-right corner reaches Seth, and the block fights once from there
        (only that corner hurt); at (12, 7) it moves a tile left (O'Neill blocks the next) and its
        shown range is every tile the block can cover. Seth breaks the fighter's corner: the other three stand."""
        d = os.path.join(self.out, "bigunit")
        os.makedirs(d, exist_ok=True)
        if "gBigUnitCharacters" not in self.sym or "gBigUnitGroups" not in self.sym:
            return self.check("bigunit: the 2x2 unit tables exist", False)
        groups = self.sym["gBigUnitGroups"]
        unit = lambda uid: 0x0202CFBC + 0x48 * (uid - 0x81) if uid & 0x80 else UNIT1 + 0x48 * (uid - 1)

        # A test ROM with O'Neill (0x68) in the character list
        data = bytearray(open(ROM, "rb").read())
        data[self.sym["gBigUnitCharacters"] - 0x08000000] = 0x68
        rom = os.path.join(d, "bigunit.gba")
        open(rom, "wb").write(data)
        ss = os.path.join(d, "oneill.ss")
        vals = self.run(rom, self.boot_lines() + ["save " + ss, "shot oneill.png"] +
                        ["r8 %#x" % (groups + k) for k in range(6)] +
                        ["r16 %#x" % (OAM + 8 * i + k) for i in range(128) for k in (0, 2)], d)
        self.shot(os.path.join(d, "oneill.png"), "2x2 O'Neill")
        if len(vals) != 6 + 256:
            return self.check("bigunit: the test ROM boots", False, "values %d" % len(vals))
        uids, (gx, gy) = vals[:4], vals[4:6]
        reads = []
        for uid in uids:
            reads += ["r32 %#x" % unit(uid), "r8 %#x" % (unit(uid) + 0x10), "r8 %#x" % (unit(uid) + 0x11)]
        got = self.run(rom, ["load " + ss] + reads, d) if all(uids) else []
        where = [(got[3 * k + 1], got[3 * k + 2]) for k in range(4)] if len(got) == 12 else []
        self.check("bigunit: O'Neill loads as four corners on a 2x2 block",
                   all(uids) and len(set(uids)) == 4 and (gx, gy) == (10, 8) and len(set(got[0::3])) == 1 and
                   where == [(10, 8), (11, 8), (10, 9), (11, 9)],
                   "corners %s at %s, block (%d, %d)" % (["%#x" % u for u in uids], where, gx, gy))
        oam = vals[6:]
        doubled = [i for i in range(128) if oam[2 * i] & 0x300 == 0x300 and (oam[2 * i + 1] >> 9) & 0x1F in (26, 27)]
        self.check("bigunit: the block is one double-size affine sprite", len(doubled) == 1,
                   "OAM entries %s" % doubled)

        # Eirika, moved next to the top-left corner (1 HP), breaks it (A through his quote): the corner leaves the
        # group, and his defeat (flag 2, which a 1x1 O'Neill's defeat sets) waits for the last corner
        if not all(uids):
            return
        rows = self.run(rom, ["load " + ss, "r32 %#x" % BM_MAP_UNIT], d)[0]
        row = lambda y: self.run(rom, ["load " + ss, "r32 %#x" % (rows + 4 * y)], d)[0]
        lines = ["load " + ss, "w8 %#x 0" % (row(5) + 4), "w8 %#x 1" % (row(8) + 9), "w8 %#x 9" % (UNIT1 + 0x10),
                 "w8 %#x 8" % (UNIT1 + 0x11), "w8 %#x 1" % (unit(uids[0]) + 0x13)]
        lines += ["press RIGHT", "wait 8"] * 5 + ["press DOWN", "wait 8"] * 3
        # Eirika: stay, Attack, the first weapon, the only target; A through his quote
        lines += ["press A", "wait 30", "press A", "wait 40", "press A", "wait 40", "press A", "wait 40", "press A",
                  "wait 60"] + ["wait 50", "press A", "wait 40"] * 5 + ["press A", "wait 60"] * 16
        lines += ["wait 300", "shot break.png", "r8 %#x" % CHAPTER_FLAGS] + ["r8 %#x" % (groups + k) for k in range(4)]
        got = self.run(rom, lines, d)
        self.shot(os.path.join(d, "break.png"), "2x2 O'Neill, a corner broken")
        self.check("bigunit: a broken corner leaves the block; no defeat while three stand",
                   len(got) == 5 and not got[0] & 2 and got[1] == 0 and all(got[2:]),
                   "chapter flags %s, corners %s" % (got[:1], got[1:]))

        # The plain ROM: the fighter next to Eirika (0x83) copied to 0x84-0x86, as a block at (x, y)
        src = self.run(ROM, ["load " + boot_ss] + ["r8 %#x" % (unit(0x83) + k) for k in range(0x48)], d)
        if len(src) != 0x48:
            return self.check("bigunit: the fighter's unit reads", False)

        def block(x, y):
            out = ["load " + boot_ss]
            for k, uid in enumerate((0x83, 0x84, 0x85, 0x86)):
                b = list(src)
                b[0x0B], b[0x10], b[0x11] = uid, x + (k & 1), y + (k >> 1)
                b[0x3C:0x40] = [0, 0, 0, 0]
                out += ["w8 %#x %#x" % (unit(uid) + i, v) for i, v in enumerate(b)]
            return out + ["w8 %#x %#x" % (groups + i, v) for i, v in enumerate((0x83, 0x84, 0x85, 0x86, x, y))]

        # Eirika and Seth wait; the enemy phase
        end_turn = EIRIKA_MENU + ["press UP", "wait 10", "press A", "wait 60", "press UP", "wait 10", "press A",
                                  "wait 30", "press A", "wait 40", "press UP", "wait 10", "press A", "wait 1600"]
        fight_ss = os.path.join(d, "fight.ss")
        got = self.run(ROM, block(2, 3) + end_turn + ["shot fight.png", "save " + fight_ss] +
                       ["r8 %#x" % (unit(u) + 0x13) for u in (0x83, 0x84, 0x85, 0x86)] +
                       ["r8 %#x" % (unit(u) + 0x0C) for u in (0x83, 0x84, 0x85, 0x86)] +
                       ["r8 %#x" % (groups + 4), "r8 %#x" % (groups + 5)], d)
        self.shot(os.path.join(d, "fight.png"), "2x2 block after its attack")
        # (The corners' grayed state is not checked: the next player phase has cleared it by now.)
        self.check("bigunit: the AI fights once, from the corner that reaches (bottom-right)",
                   len(got) == 10 and got[0] == got[1] == got[2] and got[3] < got[0] and got[8:] == [2, 3],
                   "HP %s, states %s, block %s" % (got[:4], ["%#x" % v for v in got[4:8]], got[8:]))

        # Seth (above Eirika) breaks the bottom-right corner (1 HP; his Skill and Luck raised so he cannot miss): the
        # other three stand, with their HP
        seth = UNIT1 + 0x48
        got = self.run(ROM, ["load " + fight_ss, "w8 %#x 1" % (unit(0x86) + 0x13), "w8 %#x 40" % (seth + 0x15),
                             "w8 %#x 40" % (seth + 0x19), "press UP", "wait 20", "press A",
                             "wait 30", "press A", "wait 40", "press A", "wait 40", "press A", "wait 40", "press A",
                             "wait 700", "shot fighter_break.png"] + ["r8 %#x" % (groups + k) for k in range(4)], d)
        self.shot(os.path.join(d, "fighter_break.png"), "2x2 fighter, a corner broken")
        self.check("bigunit: a corner at 0 HP breaks and the other three fight on",
                   got == [0x83, 0x84, 0x85, 0], "corners %s" % ["%#x" % v for v in got])

        mov = self.run(ROM, ["load " + boot_ss, "r32 0x0202E4E0"], d)[0]
        lines = block(12, 7) + end_turn + ["r8 %#x" % (groups + 4), "r8 %#x" % (groups + 5)]
        lines += ["press RIGHT", "wait 8"] * 7 + ["press DOWN", "wait 8"] * 2 + ["press A", "wait 40", "shot range.png"]
        rows = ["r32 %#x" % (mov + 4 * y) for y in range(10)]
        got = self.run(ROM, lines + rows, d)
        self.shot(os.path.join(d, "range.png"), "2x2 block's range")
        if len(got) != 12:
            return self.check("bigunit: the block's range shows", False, "values %s" % got)
        cells = self.run(ROM, lines + ["r8 %#x" % (got[2 + y] + x) for y in range(10) for x in range(15)], d)[2:]
        reach = sorted((i % 15, i // 15) for i, v in enumerate(cells) if v <= 0x77)
        want = sorted((x, y) for x in range(11, 15) for y in range(7, 10))
        self.check("bigunit: blocked by O'Neill, the block stops a tile left; its range is every tile it can cover",
                   got[:2] == [11, 7] and reach == want, "block %s, range %s" % (got[:2], reach))

    def designer_offset(self, field):
        """A field's offset in struct KernelDesigerConfig (include/kernel/kernel-lib.h): its u8 and u16 fields, aligned."""
        src = open(os.path.join(ROOT, "include", "kernel", "kernel-lib.h")).read()
        body = src[src.index("struct KernelDesigerConfig {"):]
        body = body[:body.index("\n};")]
        off = 0
        for m in re.finditer(r"^\s*(u8|u16|s8|s16|u32|s32)\s+(\w+)\s*;", body, re.M):
            size = {"u8": 1, "s8": 1, "u16": 2, "s16": 2, "u32": 4, "s32": 4}[m.group(1)]
            off = (off + size - 1) // size * size
            if m.group(2) == field:
                return off
            off += size
        return None

    def designer_rom(self, d, name, field, value):
        """A test ROM with a designer config byte set (gpKernelDesignerConfig points at the struct)"""
        off = self.designer_offset(field)
        if off is None or "gpKernelDesignerConfig" not in self.sym:
            return None
        data = bytearray(open(ROM, "rb").read())
        cfg = struct.unpack_from("<I", data, self.sym["gpKernelDesignerConfig"] - 0x08000000)[0] - 0x08000000
        data[cfg + off] = value
        rom = os.path.join(d, name)
        open(rom, "wb").write(data)
        return rom

    def test_awards(self, boot_ss):
        """The prologue with only O'Neill left (the other two enemies removed), moved next to Eirika: she attacks him,
        he falls to her counter in the enemy phase and the chapter ends. The awards are off by default (chapter_awards in
        designer-config.c): the chapter ends with no awards paid. A test ROM turns them on: the awards screen comes
        before the save menu, names Eirika for all three awards (the only unit that fought: one foe defeated, no damage
        taken) and pays 10 + 5 + 5 tokens."""
        d = os.path.join(self.out, "awards")
        os.makedirs(d, exist_ok=True)
        rom_on = self.designer_rom(d, "awards.gba", "chapter_awards", 1)
        if not rom_on:
            return self.check("awards: a test ROM with chapter_awards on", False)
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
        off = self.run(ROM, [ln for ln in lines if not ln.startswith("shot")], d)
        self.check("awards: off by default, the chapter ends with no awards paid",
                   len(off) == 5 and off[3] == 0, "tokens %s" % (off[3:4] or "?"))
        vals = self.run(rom_on, lines, d)
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

    def test_biography(self, boot_ss):
        """The map menu (A on an empty tile) has Biography (its fourth row): the list opens, DOWN scrolls it to its last
        biography (Amelia). She has not joined the army in the prologue, so A on her does nothing; UP back to Eirika
        and A plays her first page."""
        d = os.path.join(self.out, "boot")
        os.makedirs(d, exist_ok=True)
        need = ("gTopVisibleListIndex", "gCharacterBiographyListNumber", "gCharacterBiographyPage")
        if not all(n in self.sym for n in need):
            return self.check("biography: the biography menu", False, "no symbols")
        top, number, page = (self.sym[n] for n in need)
        count = BIOGRAPHIES
        lines = ["load " + boot_ss, "wait 20", "shot bio_map.png", "press RIGHT", "wait 10", "press RIGHT", "wait 10",
                 "press UP", "wait 10", "press A", "wait 40"] + ["press DOWN", "wait 10"] * 3
        lines += ["shot bio_menu.png", "press A", "wait 120", "shot bio_list.png"] + ["press DOWN", "wait 10"] * (count - 1)
        lines += ["wait 10", "shot bio_last.png", "r8 %#x" % top, "press A", "wait 60", "shot bio_locked.png",
                  "r8 %#x" % number]
        lines += ["press UP", "wait 10"] * (count - 1)
        lines += ["wait 10", "press A", "wait 240", "shot bio_page.png", "r8 %#x" % number, "r8 %#x" % page]
        vals = self.run(ROM, lines, d)
        m, lst, last, locked, pg = (os.path.join(d, "bio_%s.png" % n) for n in ("map", "list", "last", "locked", "page"))
        self.shot(lst, "biography list")
        self.shot(pg, "biography page")
        if len(vals) != 4:
            return self.check("biography: the biography menu", False, "values %s" % vals)
        locked_number = vals.pop(1)
        self.check("biography: the map menu opens the biography list", self.diff(m, lst) > 25,
                   "change %.0f" % self.diff(m, lst))
        self.check("biography: DOWN scrolls the list to its last biography",
                   vals[0] == count - 5 and self.diff(lst, last) > 1, "top row %d" % vals[0])
        # the mural behind the list moves: a fade to the biography would be a far bigger change
        self.check("biography: A on a character not in the army does nothing",
                   locked_number != count - 1 and self.diff(last, locked) < 25,
                   "biography %d, change %.1f" % (locked_number, self.diff(last, locked)))
        self.check("biography: A on Eirika plays her first page",
                   vals[1] == 0 and vals[2] >= 1 and self.diff(lst, pg) > 25,
                   "biography %d, page %d, change %.0f" % (vals[1], vals[2], self.diff(lst, pg)))

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

    def check_quote_face(self, face, gains, quote_ss, d, test="battle"):
        """The level-up quotes open with [Sad] for a poor level-up (0-2 stat points, or 9+) and [Happy] for a great one
        (6-8): the level-up portrait shows that expression of Eirika's portrait if the table has one."""
        if not face:
            return self.check("%s: the level-up quote's portrait" % test, False, "no face")
        got = self.run(ROM, ["load " + quote_ss, "r16 %#x" % (face + 0x3E)], d)
        tier = 1 if 3 <= gains <= 5 else 2 if 6 <= gains <= 8 else 0
        base = EIRIKA_PORTRAIT
        want = {0: self.expression(base, EXPRESSION_SAD), 2: self.expression(base, EXPRESSION_HAPPY)}.get(tier) or base
        self.check("%s: the level-up quote shows its expression" % test, got == [want],
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


class Runner:
    """Runs tests in parallel (--jobs): each in a worker thread (gbarun runs in a subprocess), with its checks and
    screenshots kept apart and merged by key, so the report and the contact sheet keep the sequential order. A task may
    submit the tests that depend on it; tests that share a folder (_smoke.txt, state files) run in one task, in turn."""

    def __init__(self, smoke, jobs):
        self.s = smoke
        self.pool = ThreadPoolExecutor(max_workers=max(1, jobs))
        self.lock = threading.Lock()
        self.futures = []
        self.done = []

    def submit(self, key, fn, *args):
        f = self.pool.submit(self._run, key, fn, args)
        with self.lock:
            self.futures.append(f)
        return f

    def _run(self, key, fn, args):
        self.s._tls.results, self.s._tls.shots = [], []
        ret = None
        try:
            ret = fn(*args)
        except Exception as e:  # a crashed test fails, the others go on
            self.s.check("%s: ran" % getattr(fn, "__name__", "test"), False, repr(e))
        with self.lock:
            self.done.append((key, self.s._tls.results, self.s._tls.shots))
        self.s._tls.results = self.s._tls.shots = None
        return ret

    def finish(self):
        while True:
            with self.lock:
                pending = [f for f in self.futures if not f.done()]
            if not pending:
                break
            wait(pending)
        self.pool.shutdown()
        for _, results, shots in sorted(self.done, key=lambda d: d[0]):
            self.s.results += results
            self.s.shots += shots


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--quick", action="store_true", help="every test but the districts other than Serafew")
    ap.add_argument("--out", help="folder for screenshots and the report (default: a new temp folder)")
    ap.add_argument("--only", nargs="+", help="run only these tests")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 1,
                    help="tests run side by side (default: the CPU count; 1 runs them one after another)")
    a = ap.parse_args()
    if not os.path.exists(ROM):
        sys.exit("%s is missing: build first (Tools/cloud/build.sh)" % ROM)
    out = os.path.abspath(a.out) if a.out else tempfile.mkdtemp(prefix="smoke-")
    os.makedirs(out, exist_ok=True)
    s = Smoke(out)
    r = Runner(s, a.jobs)
    names = ["Serafew"] if a.quick else list(DISTRICTS)
    want = set(a.only) if a.only else None
    if want and "snek" in want and "PortKiris" not in names:
        names.append("PortKiris")

    def on(n):
        return want is None or n in want

    def seq(*tests):
        """Tests that share a folder, in turn, in one task."""
        def run():
            for t in tests:
                t()
        return run

    boot_tests = ("statscreen", "battle", "battlequote", "awards", "frames", "stealer", "debugflag", "biography",
                  "bigunit", "expbar", "deeproots", "mapcolors", "ch1end", "lvupanim")
    if on("boot") or any(on(t) for t in boot_tests):
        def boot_chain():
            boot_ss = s.test_boot()
            if not boot_ss:
                return
            fights = ("battle", "bigunit", "expbar", "deeproots", "lvupanim")
            turn2_ss = s.turn2_state(boot_ss) if any(on(t) for t in fights) else None
            # boot/ and battle/ are shared folders
            r.submit((0, 1), seq(*[lambda f=f: f(boot_ss) for t, f in (
                ("statscreen", s.test_statscreen), ("biography", s.test_biography), ("debugflag", s.test_debugflag))
                if on(t)]))
            r.submit((0, 2), seq(*[lambda f=f, ss=ss: f(ss) for t, f, ss in (
                ("battle", s.test_battle, turn2_ss), ("battlequote", s.test_battlequote, boot_ss),
                ("stealer", s.test_stealer, boot_ss), ("frames", s.test_frames, boot_ss)) if on(t) and ss]))
            if on("awards"):
                r.submit((0, 3), s.test_awards, boot_ss)
            if on("bigunit") and turn2_ss:
                r.submit((0, 4), s.test_bigunit, turn2_ss)
            if on("expbar") and turn2_ss:
                r.submit((0, 5), s.test_expbar, turn2_ss)
            if on("lvupanim") and turn2_ss:
                r.submit((0, 9), s.test_lvupanim, turn2_ss)
            if on("deeproots") and turn2_ss:
                r.submit((0, 6), s.test_deeproots, turn2_ss)
            if on("mapcolors"):
                r.submit((0, 7), s.test_mapcolors, boot_ss)
            if on("ch1end"):
                r.submit((0, 8), s.test_ch1end, boot_ss)
        r.submit((0, 0), boot_chain)
    for i, (t, f) in enumerate((("savelayout", s.test_savelayout), ("suspend", s.test_suspend),
                                ("gamesave", s.test_gamesave), ("saveguard", s.test_saveguard),
                                ("newgameplus", s.test_newgameplus), ("gallery", s.test_gallery),
                                ("bestiary", s.test_bestiary), ("skillcodex", s.test_skillcodex))):
        if on(t):
            r.submit((1, i), f)
    serafew_dir = ("tokens", "claim", "prizes", "unlocks", "featured", "life")
    serafew_own = ("worldmap", "stubs", "wmsave", "wmshops", "postgame", "savetitle", "districtresume")
    for k, n in enumerate(names):
        if not (on(n) or (n == "Serafew" and any(on(t) for t in serafew_dir + serafew_own)) or
                (n == "PortKiris" and on("snek"))):
            continue

        def district_chain(n=n, k=k):
            states = s.test_district(n)
            if n == "Serafew":
                # Serafew/ is a shared folder; the others have their own
                r.submit((2 + k, 1), seq(*[lambda t=t: getattr(s, "test_" + t)(states)
                                           for t in serafew_dir if on(t)]))
                for j, t in enumerate(serafew_own):
                    if on(t):
                        r.submit((2 + k, 2 + j), getattr(s, "test_" + t), states)
            if n == "PortKiris" and on("snek"):
                r.submit((2 + k, 1), s.test_snek, states)
        r.submit((2 + k, 0), district_chain)
    r.finish()
    sys.exit(s.report())


if __name__ == "__main__":
    main()
