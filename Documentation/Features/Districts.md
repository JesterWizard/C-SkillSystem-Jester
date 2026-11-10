# Districts

---

## 📑 Index
- [Introduction](#introduction)
- [The Four Districts](#the-four-districts)
- [Entering and Leaving](#entering-and-leaving)
- [NPCs](#npcs)
- [Gifts](#gifts)
- [Hex Players](#hex-players)
- [Hex Shop](#hex-shop)
- [Fishing](#fishing)
- [River Crossing](#river-crossing)
- [Tower Stack](#tower-stack)
- [Guild Board](#guild-board)
- [Adding a District](#adding-a-district)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

A **district** is a town map that you enter from the world map. Districts are base chapters, built on the
[Base Chapters](./BaseChapters.md) system. Each district is a free-movement map with Eirika and five townsfolk. There are no
enemies on these maps.

Each district:

- Opens once its story chapter has moved on (see [The Four Districts](#the-four-districts)).
- Stays open after that. You can enter it again whenever the party is on its node.
- Has five green NPCs. Two play [Hex](./Minigames/Hex.md). The others are a gold giver, an item giver and a hint giver.
- Has a Hex shop, a building on the map. Its door opens the shop (see [Hex Shop](#hex-shop)).
- Has a gate tile. Stepping on it asks if you want to leave.
- Has a pool of water with a fishing spot (see [Fishing](#fishing)).
- Has a River Crossing spot, marked with a flag (see [River Crossing](#river-crossing)).
- Has a Tower Stack spot, marked with a small tower (see [Tower Stack](#tower-stack)).
- Has a guild board, a small notice board by the Hex shop, with quests, prizes for minigame tokens and every minigame's
  records (see [Guild Board](#guild-board)).

---

## 🗺️ The Four Districts

| District | World-map node | Unlocked after | Chapter id | Hex players | Gate tile |
|---|---|---|---|---|---|
| Serafew | `NODE_SERAFEW` | Chapter 5 (story id `0x06`). Opens when the next world-map story chapter, Chapter 6, starts. | `0x3B` | Orrin, Della | (8, 20) |
| Port Kiris | `NODE_PORT_KIRIS` | Chapter 9 (story id `0x0A`). Opens when Chapter 10 starts. | `0x3C` | Bram, Corra | (6, 21) |
| Caer Pelyn | `NODE_CAER_PELYN` | Chapter 12 (story id `0x0C`). Opens when Chapter 13 starts, once those chapters are in the story table. | `0x41` | Tamsin, Ewlyn | (5, 20) |
| Jehanna Hall | `NODE_JEHANNA_HALL_13` | Chapter 15 (story id `0x0F`). Opens when Chapter 16 starts, once those chapters are in the story table. | `0x42` | Rashid, Samira | (11, 14) |

The gate tile is where stepping leaves the district. The table lists it as (x, y) in tiles. All four gates are on the
bottom edge.

Chapters `0x3F` and `0x40` are not districts. They are cutscene maps (`OUTSIDE_GRADO_CASTLE` and `OUTSIDE_RENAIS`). The
prologue and chapters 03 and 09 load them, so a district must not use them. Earlier districts used those slots and made a
new game hang in the prologue. Caer Pelyn and Jehanna Hall moved to `0x41` and `0x42`.

### Maps

| District | Map id | Event id | Size (w x h) | Tileset (obj, palette, tileconfig) | Eirika starts at |
|---|---|---|---|---|---|
| Serafew | `0xCA` | `0xCB` | 15 x 21 | `0xEB`, `0xEE`, `0x8B` | (8, 19) |
| Port Kiris | `0xCC` | `0xCD` | 26 x 22 | `0xEB`, `0xEE`, `0x8B` | (6, 20) |
| Caer Pelyn | `0xF0` | `0xCE` | 20 x 21 | `0xEC`, `0xEF`, `0x8C` | (5, 19) |
| Jehanna Hall | `0xF1` | `0xCF` | 24 x 15 | `0xED`, `0xF0`, `0x8D` | (11, 13) |

The map id is the `mainLayerId` of the chapter. The event id is its `mapEventDataId`. Each district map is cut from the
hand-made vanilla map of the same town with `Tools/district/compose.py`, using the spec in `Tools/district/compose/<Name>.json`
(base map, crop and edited cells). Serafew uses vanilla map `0x1F`, Port Kiris `0x2B`, Caer Pelyn `0x31` and Jehanna Hall
`0x45`. The tileset is the district's copy with the Hex shop building (see [Hex Shop](#hex-shop)).

### Unlocking

Each district has one unlock flag, `0x90` to `0x93`. The flag is set when the story moves on from the district's story
chapter. It is never cleared.

- The story chapter is the one the district lists in its `storyChapter` field.
- The story moves on when the next chapter in the world-map story table starts. The table is `gWmStoryChapterNodes`
  in `CustomCampaign/Event/World_Map_Events.c`.
- A district whose story chapter has no row in that table never opens (see [Limitations](#limitations)).

---

## 🚪 Entering and Leaving

### Entering

- On the world map, open the menu of a node that has a district. The first row is **Enter District**.
- The row shows only when all of these are true:
  - `base_chapters` is on in the designer config (`Data/DesignerConfig/designer-config.c`, on by default).
  - The party's world-map unit (`gGMData.units[0]`) is on that node.
  - The district is unlocked.
- Choosing the row loads the district map. The row sets the intro-skip flag, so the chapter intro card does not show (`CONFIG_BASE_CHAPTERS`).
- The row's help text is "Choose to enter this world map node to see different events."

### Leaving

- **Gate:** step on the gate tile. The gate is an automatic free-movement trap (`TRAP_DISTRICT_GATE`, 28).
  It asks **Leave the district?** with Yes and No.
  - Yes leaves through `StartRetreatProcessing`, the same call as the vanilla Retreat.
  - No keeps you in the district.
- **Retreat row:** L opens the free-movement menu. Its Retreat row shows the vanilla "Retreat?" prompt, and Yes also leaves the district.

### Exit markers

- Each gate tile shows two small gold chevrons. They point off the nearest map edge and slide toward it.
- They are OBJ sprites, drawn every frame by `DistrictNpc_DrawGates` (`Kernel/Wizardry/EnterTown/DistrictNpc.c`).
- They show only while free movement is on and no event runs. They reload after an event, like the Hex Card icon.
- They are not added to the standing-map-sprite trap table. That table ends at id `0x6A` in vanilla, and ids above it overwrite the data after it.

---

## 👥 NPCs

Each district has five green NPCs. They are listed in `District_<Name>_Npcs` in each district's `units.c`, and their wander settings are in
`gDistrictNpcs` (`Kernel/Wizardry/EnterTown/DistrictNpc.c`).

| Role | Character | Wander radius | What they do |
|---|---|---|---|
| Hex player A | `CHARACTER_CITIZEN_YOUNG_MAN` (`0xFA`) | 2 | Starts a Hex match. Has a Hex Card icon. |
| Hex player B | `CHARACTER_CITIZEN_TEEN_GIRL` (`0xFB`) | 2 | Starts a Hex match. Has a Hex Card icon. |
| Gold giver | `CHARACTER_CITIZEN` (`0xFC`) | 2 | Gives gold once. |
| Item giver | `CHARACTER_CITIZEN_YOUNG_GIRL` (`0xF9`) | 2 | Gives an item once. |
| Hint giver | generic `0x7F` | 3 | Gives a hint. The hint repeats. |

A radius of 0 means the NPC stands still.

### Wandering

- NPCs take turns. Each turn one NPC may take one step in a random direction (up, down, left or right).
- Between turns the wait is 20 to 49 frames. About one turn in three is an idle turn with no step.
- An NPC stays inside a box around its start tile. The box is its start tile plus or minus its radius.
- An NPC does not step onto:
  - a trap tile, including the gate,
  - a door tile (village, house, armory, vendor or arena terrain), or
  - a tile where free movement does not allow the unit.
- NPCs do not move while:
  - a D-pad key is held,
  - the player's unit is moving (and for 30 frames after it stops),
  - another unit is moving,
  - an event is running, or
  - free movement is off.
- The code tracks up to eight NPCs per district. It only looks at unit ids `0x41` to `0x7F` (the green units).

### Hex Card icon

- Each Hex player has a Hex Card icon above the head. It moves 1 pixel up and down every 16 frames.
- The icon is only compiled when `CONFIG_HEX_CARDS` is on.

---

## 🎁 Gifts

Gold and item gifts are each given once. Each gift is tied to its own flag. After the gift, the giver says a short "after" line
and gives nothing more.

| District | Gold (flag) | Item to Eirika (flag) |
|---|---|---|
| Serafew | 1000 gold (`0x94`) | Elixir, `ITEM_ELIXIR` (`0x95`) |
| Port Kiris | 1500 gold (`0x96`) | Booster SPD, `ITEM_BOOSTER_SPD` (`0x97`) |
| Caer Pelyn | 2000 gold (`0x98`) | Booster RES, `ITEM_BOOSTER_RES` (`0x99`) |
| Jehanna Hall | 3000 gold (`0x9A`) | Booster POW, `ITEM_BOOSTER_POW` (`0x9B`) |

The gold giver is `CHARACTER_CITIZEN`. The item giver is `CHARACTER_CITIZEN_YOUNG_GIRL`. The flags are in
`include/jester_headers/custom-structs.h`.

---

## 🃏 Hex Players

The Hex rules are in [Hex](./Minigames/Hex.md). This section only covers how Hex works in a district.

### Hex players

Each district has two Hex players. Each one has a fixed 7-card deck in `Data/Hex/HexNpcDecks.c`, and an AI level of 1 or 2.

| District | Player | AI level | Rare cards (wins that can claim each) |
|---|---|---|---|
| Serafew | Orrin | 1 | Kyle (2), Forde (2) |
| Serafew | Della | 1 | Tethys (3) |
| Port Kiris | Bram | 1 | Joshua (2), Marisa (2) |
| Port Kiris | Corra | 2 | L'Arachel (2), Dozla (2), Gerik (1) |
| Caer Pelyn | Tamsin | 2 | Saleh (2), Innes (2), Tana (2), Myrrh (1) |
| Caer Pelyn | Ewlyn | 2 | Cormag (2), Duessel (1), Gerik (2), Wight (2), Gargoyle (2) |
| Jehanna Hall | Rashid | 2 | Seth (1), Ephraim (1), Glen (1), Tarvos (2), Cyclops (1), Arch Mogall (2) |
| Jehanna Hall | Samira | 2 | Valter (1), Lyon (1), Eirika (1), Duessel (1), Myrrh (1), Deathgoyle (1), Draco Zombie (1) |

The other cards in each deck are common cards. A common card can be won any number of times.

### The Pied Piper (Port Kiris)

An old man next to Port Kiris's guild board, at (17, 14) (character `0x7E`), offers a game of
[Snek](./Minigames/Snek.md). Its scores count toward the guild board's Snek quests, and each game pays tokens.

- **Opening a match:** the Hex screen opens at the hand picker. The front menu and the opponent list are not shown.
  B with no card in hand leaves the screen.
- **Win:** you can take one of the opponent's 7 starting cards. A card type you already hold 255 copies of is refused.
- **Rare cards:** each rare card has a counter. Each win that takes it adds one. When the counter reaches its cap, that
  NPC plays a fallback card in its place.
- **Loss:** if you own more than 7 cards, the NPC takes a random card from the hand you played. With 7 or fewer cards,
  nothing is taken.
- **Draw:** nothing changes hands.
- **Where cards go:** cards go to your card binder, which is part of your save. They do not go to the convoy.
- **Starter cards:** when the Hex screen opens with no cards at all, 7 starter cards are added to the binder. These are
  two Ross, two Amelia, one Ewan, one Franz and one Neimi.
- **Old Hex Card items:** Hex Card items in the convoy or in unit inventories move into the binder when the Hex screen opens.

#### Monster cards

Caer Pelyn and Jehanna Hall play the monster cards, types 32 to 43. Together, the Hex players of those two districts use
all 12 monster cards.

---

## 🛒 Hex Shop

Each district has a Hex shop. The shop is a building on the map. Its door is a location event: Eirika stands on the door and
presses A, and the shop opens. The shop is the vanilla FE8 shop screen in Hex mode, run by a witch.

### The building

- `Tools/district/hexshop_building.py` pastes a drawn shop building from `Tools/district/hexshop_art/` onto the map. It
  does not recolour a house.
  - Every district's building is the same market stall, a 3x2-metatile block (`stall.txt`, 48x32 pixels). It is a
    supplied illustration read pixel for pixel (33x25), with one wall row added above the windows and two rows added
    through the door (33x28): a blue hipped roof, a cream stone wall with two green windows, a red-and-white striped
    awning and a wooden door.
  - The stall stands on the ground under its block: the town yard (Serafew, Port Kiris), the Caer Pelyn plaza floor or
    the Jehanna sand. In Caer Pelyn and Jehanna the six block cells are first set to plain ground in the compose spec
    (`Tools/district/compose/<Name>.json`), and the cells other than the door get terrain `0x2E` (building).
  - Each art character maps to a colour (`STALL_COLORS` in the script). `.` is the ground.
  - The cell below each door is walkable, so Eirika can step onto the door: the stall's sides and back are building.
- Each district has a copy of its tileset with the new building. The copies are in `CustomCampaign/Chapters/_hexshop/`.
  Serafew and Port Kiris share one copy. Each copy also has its own palette: its vanilla palette with bank 5 redefined to
  the stall's 12 colours and 3 ground colours, because the stall's colours are not in the vanilla palettes. They are
  `town_pal.bin` (palette 0xEE, from 0x0F), `caer_pal.bin` (0xEF, from 0x34) and `jeh_pal.bin` (0xF0, from 0x43).
  Every 8x8 tile that holds stall pixels uses bank 5.
- The copies are registered in `CustomCampaign/Chapters/_hexshop/HexShopTiles.event`.
- The door tile has terrain `0x03` (village). No NPC steps on it.
- The door has a location event, `HOUSE(0, EventScr_District_<Name>_HexShop, x, y)`. It runs
  `ASMC(StartDistrictHexShopASMC)`.

| District | Door (x, y) | Tileset copy (obj, palette, tileconfig) |
|---|---|---|
| Serafew | (5, 19) | `0xEB`, `0xEE`, `0x8B` |
| Port Kiris | (11, 15) | `0xEB`, `0xEE`, `0x8B` |
| Caer Pelyn | (11, 8) | `0xEC`, `0xEF`, `0x8C` |
| Jehanna Hall | (16, 6) | `0xED`, `0xF0`, `0x8D` |

### Using the shop

- Stand on the door and press A to open the shop.
- The shop screen shows the shopkeeper portrait `0xAF` (`FID_HEX_WITCH_SHOP`, `include/kernel/hex.h`), the only Hex witch
  portrait. `Tools/hex/witch_shop_from_art.py` makes it from the supplied pixel-art illustration
  `Tools/hex/art/hex_witch_shop_source.webp` (a silver-haired witch in a purple hat with a gold ornament, green eyes and gold
  hoop earrings): it reads the art's 64x64 pixel grid exactly, crops a 48x48 window into the shopkeeper box, maps it to 15
  fitted colours, adds three mouth frames, and stores the sheet mirrored because the shop draws the box flipped.
- Her lines are the `MSG_HEXSHOP_*` texts in `Contents/Texts/Source/texts/Districts.txt`.
- The shop goes straight to the buy list. There is no Buy or Sell question.
- A buys the ware under the cursor. B leaves, with her goodbye.
- There is no gold box, and no gold is spent.
- Each ware costs copies of ONE card type from your binder. The price shows as `Ross x2`.
- A ware you can't afford is grey, its name and its price. If you press A on it, the witch says you haven't the cards.
- A on an affordable ware asks for the cost: "That'll be N of your cards. A fair trade?" Yes buys it.
- Buying takes the cards from the binder. The item goes to Eirika. If she is full, the vanilla "Send it to storage?"
  prompt appears.
- The shop has no stock limit. A ware can be bought again while you hold the cards.

### Prices

The prices are in `Data/Hex/HexShops.c`. Each ware has one price: copies of one card type. The name in brackets is the
name used in `HexShops.c`.

| District | Ware | Price |
|---|---|---|
| Serafew | Vulnerary | Ross x2 |
| Serafew | Elixir | Amelia x3 |
| Serafew | HP Tonic | Franz x3 |
| Serafew | STR Tonic | Colm x3 |
| Serafew | Angelic Robe (Booster HP) | Kyle x1 |
| Serafew | Goddess Icon (Booster LCK) | Tethys x1 |
| Port Kiris | Pure Water | Lute x2 |
| Port Kiris | DEF Tonic | Garcia x3 |
| Port Kiris | SPD Tonic | Vanessa x3 |
| Port Kiris | Speedwing (Booster SPD) | Joshua x1 |
| Port Kiris | Dragonshield (Booster DEF) | Dozla x1 |
| Port Kiris | Secret Book (Booster SKL) | Marisa x1 |
| Caer Pelyn | Elixir | Revenant x2 |
| Caer Pelyn | RES Tonic | Bael x3 |
| Caer Pelyn | MAG Tonic | Mogall x3 |
| Caer Pelyn | Talisman (Booster RES) | Tana x1 |
| Caer Pelyn | Body Ring (Booster CON) | Gerik x1 |
| Caer Pelyn | Energy Ring (Booster POW) | Cormag x1 |
| Jehanna Hall | OMNI Tonic | Bael x4 |
| Jehanna Hall | Energy Ring (Booster POW) | Seth x1 |
| Jehanna Hall | Swiftsole (Booster MOV) | Ephraim x1 |
| Jehanna Hall | Goddess Icon (Booster LCK) | Eirika x1 |
| Jehanna Hall | Secret Book (Booster SKL) | Lyon x1 |
| Jehanna Hall | Angelic Robe (Booster HP) | Draco Zombie x1 |

---

## 🎣 Fishing

Each district has a pool of water with a fishing spot next to it. A red and white bobber floats on the water beside each
spot. Stand on the spot and press A to open the fishing screen: buy a bait, cast, hook a fish and keep the line's tension
in the green to land it for gold. Each district's water has its own five species (20 in all, a golden one in each), every
catch gets a size and a catch card, and the fish journal records them. Find all five of a water's
species and its legend appears; catches raise an angler rank, townsfolk post requests, and a catch can be kept as food. The rules, the fish, the baits and the rewards are
in [District Fishing](./Minigames/Fishing.md).

| District | Spot (x, y) | Pool |
|---|---|---|
| Serafew | (12, 2) | A pool added to the north-east corner, cells (11..14, 0..1) |
| Port Kiris | (4, 16) and (13, 12) | The harbour: the west shore and the pier |
| Caer Pelyn | (3, 13) | A pond added on the west grass, cells (2..4, 11..12) |
| Jehanna Hall | (9, 5) | The oasis |

- The spots are free-movement traps (`TRAP_FISHING_SPOT`) in each district's `events/traps.c`.
- Serafew and Caer Pelyn had no water. Their pools are cells in the compose specs (see [Maps](#maps)).
- The Caer Pelyn item giver now starts at (9, 13), off the pond.
- The bobbers are OBJ sprites drawn with the gate arrows by `DistrictNpc.c`.

---

## 🐸 River Crossing

Each district has a River Crossing spot, marked on the map with a small red flag. Stand on it and press A to play a
lane-crossing minigame: hop across a road of enemy riders and a river of drifting rafts to fill five docks. Each district
has its own course, from Serafew (the gentlest) to Jehanna Hall (the fastest). The rules are in
[River Crossing](./Minigames/Crossing.md).

| District | Spot (x, y) | Course |
|---|---|---|
| Serafew | (3, 15) | 0 |
| Port Kiris | (18, 20) | 1 |
| Caer Pelyn | (15, 13) | 2 |
| Jehanna Hall | (16, 11) | 3 |

---

## 🧱 Tower Stack

Each district has a Tower Stack spot, marked on the map with a small white tower. Stand on it and press A to play a
block-stacking minigame: drop sliding blocks onto a tower, chain perfect landings into a fever, and turn the score into
gold. The rules are in [Tower Stack](./Minigames/Tower.md).

| District | Spot (x, y) |
|---|---|
| Serafew | (6, 16) |
| Port Kiris | (16, 15) |
| Caer Pelyn | (11, 16) |
| Jehanna Hall | (8, 12) |

---

## 📋 Guild Board

Each district has a guild board, a small wooden notice board by the Hex shop. Stand on it and press A: its pages are the
district's quests, prizes bought with tokens (every minigame pays tokens), and every minigame's records. A red notice
blinks on the board while a quest there is met and waiting to be claimed. The rules are in [Guild Board](./GuildBoard.md).

| District | Board (x, y) |
|---|---|
| Serafew | (7, 18) |
| Port Kiris | (14, 14) |
| Caer Pelyn | (10, 9) |
| Jehanna Hall | (14, 6) |

---

## 🛠️ Adding a District

1. **Pick the ids.** Choose these before you start:
   - a world-map node (`NODE_*` in `Tools/FE-CLib-Mokha/include/constants/worldmap.h`),
   - a chapter id. The installer row for a chapter id is at `0x8B0890 + chapter × 0x94`. The four existing districts use ids
     `0x3B`, `0x3C`, `0x41` and `0x42`. Do not use `0x3F` or `0x40`: they are the cutscene maps `OUTSIDE_GRADO_CASTLE` and
     `OUTSIDE_RENAIS` (see [Maps](#maps)). The chapter number is a literal in `District.c`, `DistrictNpc.c`,
     `Data/Hex/HexNpcDecks.c` and `Data/Hex/HexShops.c`, so the new district needs rows in all four (steps 5 to 8),
   - a map id and an event id that are not already used. The four districts use the ids in [Maps](#maps). This page does not check the other ids in use,
   - two gift flags and one unlock flag. Add them to `include/jester_headers/custom-structs.h`. Flags `0x90` to `0x9B`
     are in use. The comment in that file lists the free range.

2. **Make the map.** The map is cut from a hand-made vanilla map of the same town, not generated. The pipeline is:
   1. Write a spec, `Tools/district/compose/<Name>.json`, with the base map, the crop and any edited cells. The four
      existing specs are examples. To add a pool for fishing, `Tools/district/pond.py` picks the water metatiles
      (see [Tools/district/README.md](../../Tools/district/README.md)). The installed maps come from `Tools/district/compose.py`.
   2. Run `compose.py <spec> --write` for each district. This writes the district's layout.
   3. Add the Hex shop building to all the layouts together:
      `python3 Tools/district/hexshop_building.py --chapters CustomCampaign/Chapters --out-dir CustomCampaign/Chapters/_hexshop --write-layouts`.
      A new district needs its own entry in `SETS` in `hexshop_building.py`, and its ids in `HexShopTiles.event`.
      See [Tools/district/README.md](../../Tools/district/README.md).
   4. Compress the six `_hexshop` `.bin` files with `Tools/EventAssembler/Tools/compress` to `.lz`.
   5. Set the layout `tileset` to the Hex shop ids, `[obj, palette, tileconfig]`. The palette stays the vanilla one.
   6. Build the map with `district_map.py`, for each district:

   ```
   python3 Tools/district/district_map.py CustomCampaign/Chapters/<Name>/map/<Name>_layout.json \
     CustomCampaign/Chapters/<Name>/map <Name>_map \
     --chapter 0x.. --map-id 0x.. --label MapData_<Name> --check
   ```

   It writes `<Name>_map.tmx`, `<Name>_map_data.dmp` and `<Name>_map.event` into the same folder. Do not edit those files
   by hand. Change the spec or the layout and run the steps again. See
   [Tools/district/README.md](../../Tools/district/README.md) for the formats.

   The [tilegen](../../Tools/district/tilegen/README.md) generator is kept as an experiment. It is not part of this pipeline.

3. **Copy a district folder.** Copy `CustomCampaign/Chapters/Serafew/` to `CustomCampaign/Chapters/<Name>/` and rename the
   files. Then update:
   - `events/chapter.c`: the chapter data. Set the tileset, `mainLayerId` (map id), `mapEventDataId` (event id), title and start position.
   - `events/asmc.h`: the `SetGameOptions_*` function. Its name must match the one in `events.c`.
   - `events/traps.c`: the gate, as `DISTRICT_GATE(x, y)`, the fishing spot, as `FISHING_SPOT(x, y, dir)`
     (see [Fishing](./Minigames/Fishing.md#adding-a-fishing-spot)), and the River Crossing spot, as `CROSSING_SPOT(x, y)`.
     A new district also needs a course: add its chapter to `Crossing_LevelOfChapter` and a row to `sCourses`
     ([River Crossing](./Minigames/Crossing.md#the-courses)). Add the Tower Stack spot as `TOWER_SPOT(x, y)` and the guild board as `GUILD_BOARD(x, y)` (its quests are listed by district chapter in `Data/Guild/Guild.c`).
   - `events/units.c`: Eirika's start tile and the five NPCs.
   - `events/events.c`: the opening event, the talk events, the Hex shop event, the gift flags and the event group.
   - `text/npcs.txt`: the `District_<Name>_*` texts. This file is already included by `CustomCampaign/Text/Text.txt` for the
     four existing districts, so add your new file there too.

4. **Install the chapter and events.**
   - In `CustomCampaign/Chapters/Chapter_Installer.event`, add an `ORG` row with the `chapter.lyn.event` include.
   - In `CustomCampaign/Event/Event_Installer.event`, add `NewEventPointerTable_Event(<event id>, <Name>Event)` and include
     the district's `events.lyn.event` and map `.event` files.

5. **Add the node row.** In `Kernel/Wizardry/EnterTown/District.c`, add a `gDistrictNodes` row above the end row:
   `{ node, chapter, storyChapter, 0, unlock flag }`.
   - `storyChapter` is the story chapter whose move-on unlocks the district.
   - That chapter must also have a row in `gWmStoryChapterNodes` (`CustomCampaign/Event/World_Map_Events.c`). Without it the
     flag is never set.

6. **Add the NPC rows.** In `Kernel/Wizardry/EnterTown/DistrictNpc.c`, add five `gDistrictNpcs` rows
   `{ chapter, pid, radius, flags }`. Use `DISTRICT_NPC_HEX` on the two Hex players. Keep the `{ 0xFF, 0, 0, 0 }` end row last.

7. **Add the Hex deck.** In `Data/Hex/HexNpcDecks.c`, add one row per Hex player with 7 cards:
   - common card: `HEX_NPC_C(type)`,
   - rare card: `HEX_NPC_R(type, claimSlot, cap, fallback)`.

   Add the player's name as a `MSG_HEXNPC_*` text in `Contents/Texts/Source/texts/Districts.txt`, and use it in the row.
   Keep the `HEX_NPC_END` row last. Each rare card needs a claim slot that no other row uses. Slots 18 and 19 are free.

8. **Add the shop rows.** In `Data/Hex/HexShops.c`, add one row per ware, with the district's rows together. Keep
   `HEX_SHOP_END` last.

9. **More rare cards (only if you run out of claim slots).** Claim counters are `gHexSave.claims[HEX_CLAIM_SLOTS]` in
   `include/kernel/hex.h`. To add more, grow `claims[]`. Then update every size that depends on it:
   - the `0x54` size check in `Kernel/Wizardry/Minigames/Hex/Source/HexSave.c`,
   - both Hex `EmsChunk($0054, ...)` lines in `Kernel/Wizardry/SaveData/data.event` (the save list and the suspend list).

   Saves made before the change keep the old chunk size. Test loading an old save.

10. **Build and test.** Build the ROM (`make`, or `Tools/cloud/build.sh` in cloud sessions). Check the regenerated `.lyn.event`
    files in `git diff`. Then test the unlock, the entry, the gate, each talk event, the Hex matches and the Hex shop.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| District table | [`District.c`](../../Kernel/Wizardry/EnterTown/District.c) | `gDistrictNodes`, unlock, leave prompt |
| District header | [`district.h`](../../include/kernel/district.h) | `struct DistrictNode`, `struct DistrictNpc` |
| District NPCs | [`DistrictNpc.c`](../../Kernel/Wizardry/EnterTown/DistrictNpc.c) | `gDistrictNpcs`, wandering, Hex Card icon |
| Node menu row | [`EnterTown.c`](../../Kernel/Wizardry/EnterTown/EnterTown.c) | `Enter District` row and its entry |
| Unlock hook | [`WmStoryChapter.c`](../../Kernel/Wizardry/WorldMap/WmStoryChapter.c) | `WmStoryChapter_Begin` calls `District_MarkStoryChapterCleared` |
| Story table | [`World_Map_Events.c`](../../CustomCampaign/Event/World_Map_Events.c) | `gWmStoryChapterNodes` |
| Gate hook | [`HookList.event`](../../Kernel/Wizardry/FreeMovement/cFreeMU/HookList.event) | Trap 28 runs `DistrictGate_OnStep` |
| Gate macro | [`bmtrick.h`](../../Tools/FE-CLib-Mokha/include/bmtrick.h) | `TRAP_DISTRICT_GATE` and `DISTRICT_GATE(x, y)` |
| Intro skip | [`MiscFunctions.c`](../../Kernel/Wizardry/MiscFunctions/Source/MiscFunctions.c) | Skips the intro card when the flag is set |
| Switches | [`configs.h`](../../include/configs/configs.h) | `CONFIG_BASE_CHAPTERS`, `CONFIG_HEX_CARDS` |
| Runtime switch | [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | `base_chapters` |
| Flags | [`custom-structs.h`](../../include/jester_headers/custom-structs.h) | `GLOBAL_FLAG_DISTRICT_*` |
| District folders | [`CustomCampaign/Chapters/`](../../CustomCampaign/Chapters/) | `Serafew`, `PortKiris`, `CaerPelyn`, `Jehanna` |
| Chapter install | [`Chapter_Installer.event`](../../CustomCampaign/Chapters/Chapter_Installer.event) | Chapter rows |
| Event install | [`Event_Installer.event`](../../CustomCampaign/Event/Event_Installer.event) | Event and map rows |
| Map tool | [`district_map.py`](../../Tools/district/district_map.py) | Layout to `.tmx`, `.dmp` and `.event` |
| Map tool notes | [`README.md`](../../Tools/district/README.md) | Usage and layout format |
| Hex decks | [`HexNpcDecks.c`](../../Data/Hex/HexNpcDecks.c) | Deck of each Hex player |
| Hex shop rows | [`HexShops.c`](../../Data/Hex/HexShops.c) | The wares of each district shop and their card prices |
| Hex shop | [`HexShopVanilla.c`](../../Kernel/Wizardry/Minigames/Hex/Source/HexShopVanilla.c) | The shop screen in Hex mode: the witch, the prices, the buy |
| Shop hooks | [`LimitedShopStock.c`](../../Kernel/Wizardry/LimitedShopStock/src/LimitedShopStock.c) | Hex branches of the vanilla shop: price line, buy, B, `Shop_Init` |
| Shop building | [`hexshop_building.py`](../../Tools/district/hexshop_building.py) | Pastes the market stall into each tileset copy |
| Building art | [`hexshop_art/`](../../Tools/district/hexshop_art/) | `stall.txt`, the market stall used by every tileset copy |
| Shop tilesets | [`_hexshop/`](../../CustomCampaign/Chapters/_hexshop/) | Tileset copies with the shop building, and `HexShopTiles.event` |
| Fishing | [`Minigames/Fishing/`](../../Kernel/Wizardry/Minigames/Fishing/) | The fishing screen, the spot's A-press handler, the map bobbers ([doc](./Minigames/Fishing.md)) |
| Pond tool | [`pond.py`](../../Tools/district/pond.py) | Picks the water metatiles of a small pool |
| River Crossing | [`Minigames/Crossing/`](../../Kernel/Wizardry/Minigames/Crossing/) | The lane-crossing minigame, its spots' A-press handler, the map flags ([doc](./Minigames/Crossing.md)) |
| Tower Stack | [`Minigames/Tower/`](../../Kernel/Wizardry/Minigames/Tower/) | The block-stacking minigame, its spots' A-press handler, the map towers ([doc](./Minigames/Tower.md)) |
| Guild board | [`Guild/`](../../Kernel/Wizardry/Guild/) | Quests, prizes, records, tokens, the boards' A-press handler and markers ([doc](./GuildBoard.md)) |
| Shop portrait | [`witch_shop_from_art.py`](../../Tools/hex/witch_shop_from_art.py) | Makes `hex_witch_shop.png` (portrait `0xAF`, `FID_HEX_WITCH_SHOP`) from the supplied illustration |
| Exit markers | [`DistrictNpc.c`](../../Kernel/Wizardry/EnterTown/DistrictNpc.c) | `DistrictNpc_DrawGates`, the gold chevrons on each gate |
| Map composer | [`compose.py`](../../Tools/district/compose.py) | Cuts each district map from a vanilla map, from `compose/<Name>.json` |
| Compose specs | [`compose/`](../../Tools/district/compose/) | Base map, crop and edited cells of each district |
| Map generator (experimental) | [`tilegen/`](../../Tools/district/tilegen/) | Not used for the installed maps. See its README |
| Hex district code | [`HexDistrict.c`](../../Kernel/Wizardry/Minigames/Hex/Source/HexDistrict.c) | Talk-event entry points, shop rows and payment |
| Hex save | [`HexSave.c`](../../Kernel/Wizardry/Minigames/Hex/Source/HexSave.c) | Binder and claim counters |
| Hex save data | [`hex.h`](../../include/kernel/hex.h) | `struct HexSaveData`, `HEX_CLAIM_SLOTS` |
| Save chunk | [`data.event`](../../Kernel/Wizardry/SaveData/data.event) | Hex save chunk in both lists |
| Texts | [`Districts.txt`](../../Contents/Texts/Source/texts/Districts.txt) | Hex player names, the leave prompt and the witch's lines |
| District texts | [`CustomCampaign/Chapters/`](../../CustomCampaign/Chapters/) | `<District>/text/npcs.txt` |

---

## ⚠️ Limitations

- **Caer Pelyn and Jehanna Hall wait on campaign content.** Their story chapters, 12 and 15, have no row in `gWmStoryChapterNodes`
  yet, because the table ends at Chapter 10. They open when Chapter 13 (or 16) starts, once Chapter 12 (or 15) and that chapter
  are in the table. Until then, the flags `0x92` and `0x93` are never set.
- **Opening is tied to the next story chapter.** A district opens when the next world-map story chapter starts, not when its
  own chapter is cleared.
- **Free movement only.** The gate, the NPC walks, and the Hex Card icon work only in free movement.
- **One player unit.** Each district places Eirika alone (`units.c`). There are no enemies.
- **NPC limit.** The code tracks at most eight NPCs per district, and only unit ids `0x41` to `0x7F`.
- **Flags.** The districts use flags `0x90` to `0x9B`.
- **Claim slots.** Two of the 32 claim slots are free (18 and 19). See step 9 of [Adding a District](#adding-a-district) to add more.
- **Starter cards.** The starter cards are added whenever the Hex screen opens with no cards, including inside a district.
- **Hand-cut maps.** The installed district maps are cut from vanilla maps with `compose.py`, using the specs in
  `compose/`. The [tilegen](../../Tools/district/tilegen/README.md) generator is experimental.
  Its specs and its seam counts no longer match the installed maps.
- **Hex shop copies.** `hexshop_building.py` only knows the four districts in `SETS`. A new district needs its own entry
  there, and its copy's ids in `HexShopTiles.event`.
