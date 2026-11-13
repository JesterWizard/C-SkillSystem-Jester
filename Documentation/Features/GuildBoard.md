# Guild Board

<p align="center">
  <img src="../Gifs/Guild_Board.gif" alt="The Serafew guild board: a met quest is claimed for tokens, a stat booster is bought on the Prizes page, then the Records page" width="480"/>
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [Tokens](#tokens)
- [Quests](#quests)
- [Prizes](#prizes)
- [Records](#records)
- [The Rival](#the-rival)
- [Chapter Awards](#chapter-awards)
- [Controls](#controls)
- [The Board on the Map](#the-board-on-the-map)
- [Adding Quests and Prizes](#adding-quests-and-prizes)
- [Save Data](#save-data)
- [Enabling and Disabling](#enabling-and-disabling)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

Each [district](./Districts.md) has a **guild board**, a small wooden notice board on its map next to the Hex shop.
Stand on it and press A: the board opens with three pages.

- **Quests**: the district's requests. Score so much in a minigame, catch so many kinds of fish, win battles, bring an
  item. A met quest is claimed here for tokens, and sometimes gold and an item.
- **Prizes**: items bought with tokens, among them tonics for the next battle and skill scrolls.
- **Records**: every minigame's bests on one page, and (DOWN) the records of Marlo, the guild's rival.

**Tokens** are the minigames' shared currency: every minigame pays some at its end, and so do quests. They tie the
district games to each other and to the main campaign.

---

## 🪙 Tokens

| Earned from | Tokens |
|---|---|
| [River Crossing](./Minigames/Crossing.md) | 1 per 100 points, at the end of a game (or when you leave with B) |
| [Tower Stack](./Minigames/Tower.md) | 1 per 20 points, the same way |
| [Fishing](./Minigames/Fishing.md) | 1 a catch; 3 for a new species or a record size; 10 for a legend |
| [Hex](./Minigames/Hex.md) | 5 for a win, 1 for a draw |
| [Snek](./Minigames/Snek.md) | 1 per 8 points (two coins) |
| A claimed quest | its reward |
| [Chapter awards](#chapter-awards) | 10 for the MVP, 5 for the most foes defeated, 5 for the least damage taken |

- Each game shows what it paid: River Crossing's and Tower Stack's final line (`+13 tokens` after the gold), Fishing's
  catch card, Hex's Victory and Draw banners, and the Pied Piper's line after a Snek game ("That's 10 tokens for the
  guild board.").
- You can hold up to 9999 tokens. The board also counts the tokens earned in all (up to 65535): a quest can ask for it.
- The title line shows the tokens you hold.

---

## 📜 Quests

A board shows three quests at a time: the first three of its district's quests (and of the quests on every board) that
are not yet claimed, in the order of the table. Claim one and the next takes its place.

Each quest is two lines: the request, then its progress (`640 / 1500`) or **Done!**, and its reward. A met quest's
request is green.

A quest's progress is read from what the game already saves, so nothing has to be accepted or tracked:

| Kind | Progress | Parameter |
|---|---|---|
| `GUILD_QUEST_CROSSING_SCORE` | River Crossing's best score on a course | the course: 0 Serafew, 1 Port Kiris, 2 Caer Pelyn, 3 Jehanna Hall |
| `GUILD_QUEST_CROSSING_MEDAL` | that course's medal: 1 bronze, 2 silver, 3 gold | the course |
| `GUILD_QUEST_TOWER_SCORE` | Tower Stack's best score | |
| `GUILD_QUEST_SNEK_SCORE` | Snek's high score | |
| `GUILD_QUEST_FISH_SPECIES` | species in the fish journal (of 20) | |
| `GUILD_QUEST_FISH_LEGENDS` | legends caught (of 4) | |
| `GUILD_QUEST_FISH_RANK` | the angler's rank (0 Novice .. 4 Master) | |
| `GUILD_QUEST_HEX_CARDS` | cards in the Hex binder | |
| `GUILD_QUEST_WINS` | battles won (the battle records) | a character, or 0 for the whole army |
| `GUILD_QUEST_DELIVER` | copies of an item held by the army's units and in the convoy | the item; they are handed over when the quest is claimed |
| `GUILD_QUEST_TOKENS_EARNED` | tokens earned in all | |

A record you already hold counts: a quest posted after you reached it is met at once.

### The quests

| Board | Quest | Reward |
|---|---|---|
| Serafew | Score 800 in Serafew's crossing | 15 tokens |
| Serafew | Bring 2 Vulneraries | 10 tokens, 300 gold |
| Serafew | Catch 5 kinds of fish | 15 tokens |
| Serafew | Win 20 battles | 20 tokens, an Elixir |
| Serafew | Score 100 at Tower Stack | 15 tokens |
| Port Kiris | Earn a medal in Port Kiris's crossing | 15 tokens |
| Port Kiris | Reach the Angler rank | 20 tokens |
| Port Kiris | Hold 10 Hex cards | 15 tokens |
| Port Kiris | Bring a Pure Water | 15 tokens, 500 gold |
| Port Kiris | Score 40 at Snek | 15 tokens |
| Caer Pelyn | Score 1500 in Caer Pelyn's crossing | 25 tokens |
| Caer Pelyn | Land a legendary fish | 30 tokens, a Speedwing |
| Caer Pelyn | Win 60 battles | 30 tokens |
| Caer Pelyn | Earn 200 tokens in all | 20 tokens, 1000 gold |
| Jehanna Hall | Win gold in Jehanna's crossing | 50 tokens, a Master Seal |
| Jehanna Hall | Fill the fish journal | 50 tokens |
| Jehanna Hall | Hold 30 Hex cards | 30 tokens |
| Jehanna Hall | Win 150 battles | 50 tokens, an Energy Ring |
| Every board | Score 300 at Tower Stack | 30 tokens |
| Every board | Score 100 at Snek | 30 tokens |
| Serafew | Eirika: win 25 battles | 10 tokens, the last page of her biography |
| Serafew | Seth: win 25 battles | 10 tokens, the last page of his biography |
| Port Kiris | Franz: win 20 battles | 10 tokens, the last page of his biography |
| Port Kiris | Gilliam: win 20 battles | 10 tokens, the last page of his biography |
| Caer Pelyn | Tana: win 15 battles | 10 tokens, the last page of her biography |
| Serafew | Earn 60 tokens in all: Eirika and Seth | 5 tokens, 40 support points between them |
| Port Kiris | Earn 120 tokens in all: Franz and Gilliam | 5 tokens, 40 support points between them |
| Serafew | Artur: win 15 battles | 10 tokens, the last page of his biography |
| Jehanna Hall | Amelia: win 15 battles | 10 tokens, the last page of her biography |

### Story and support quests

A quest can unlock more than its reward. `gGuildUnlocks` (`Data/Guild/Guild.c`) lists each quest's unlocks, naming the
quest by its message, so the quest table's order does not matter:

- `UNLOCK_BIOGRAPHY(quest, character, page)`: that page of the character's
  [biography](./CharacterBiographies.md) (0 to 3), and the pages after it, stay locked until the quest is claimed. The
  biography stops before the first locked page.
- `UNLOCK_SUPPORT(quest, a, b, points)`: claiming the quest gives the two characters `points` support points each way, up
  to the next rank, so its support conversation can then happen as usual. The points go into the battle records, so the
  units do not need to be on the map.

The board shows "+story" or "+support" after such a quest's reward.

### Featured quests

Each stretch of the story features one quest on some boards. While the world map leads to that story chapter
(`gWmStoryChapter`), the quest tops the board, its line is gold until it is met, its reward shows the bonus and
"Featured!", and claiming it on that board pays the bonus tokens on top of its reward. Claimed, it is no longer featured.

| Story chapter | Board | Quest | Bonus |
|---|---|---|---|
| 6 | Serafew | Score 800 in Serafew's crossing | 15 |
| 7 | Serafew | Catch 5 kinds of fish | 15 |
| 8 | Serafew | Score 100 at Tower Stack | 15 |
| 10 | Port Kiris | Score 40 at Snek | 15 |
| 10 | Serafew | Score 300 at Tower Stack | 20 |
| 13 | Caer Pelyn | Score 1500 in Caer Pelyn's crossing | 20 |
| 14 | Caer Pelyn | Land a legendary fish | 25 |
| 16 | Jehanna Hall | Hold 30 Hex cards | 20 |
| 17 | Jehanna Hall | Gold in Jehanna Hall's crossing | 30 |

The rows are `gGuildFeatured` in `Data/Guild/Guild.c`:

```c
FEATURED(CHAPTER_08, SERAFEW, MSG_GUILD_Q_SERAFEW_TOWER, 15),
/*       story       board    the quest's msg             bonus */
```

A board with no row for the current story chapter features nothing. Any quest the board shows can be featured, the
every-board ones too.

---

## 🎁 Prizes

| Prize | Tokens | |
|---|---|---|
| Vulnerary, Antitoxin, Torch | 5 | |
| Door Key | 10 | |
| Pure Water | 15 | |
| Elixir | 25 | |
| Master Seal | 60 | |
| Rare Candy | 100 | +100 EXP; nothing else in the campaign gives it |
| HP, Str, Mag, Skl, Spd, Lck, Def, Res Tonic | 8 | +2 to the stat until the chapter ends ([Tonics](./Tonics.md)) |
| Omni Tonic | 30 | +2 to every stat until the chapter ends |
| Each stat booster (Angelic Robe, Energy Ring, Secret Book, Speedwing, Goddess Icon, Dragonshield, Talisman) | 80 | once each |
| Vantage, Renewal scroll | 120 | once each; a skill scroll teaches its skill |
| Adept, Pavise, Luna scroll | 150 | once each |
| Galeforce scroll | 250 | once |

- A prize you can't afford is grey. A one-time prize you bought shows **Sold out**.
- A, then A again, buys. The item goes to the unit, or to the convoy when its items are full.
- The page shows six prizes and scrolls.

---

## 📊 Records

| Line | What |
|---|---|
| Quests, Earned | quests claimed (of all), tokens earned in all |
| Crossing | the best score of each district's River Crossing course (S, P, C, J) |
| Tower, Snek | Tower Stack's best score, Snek's high score |
| Fishing, Journal | the angler's rank, species in the journal |
| Legends, Hex cards | legends caught, cards in the Hex binder |
| Battles | battles won by the army |

---

## 🥇 The Rival

Marlo, the guild's rival, holds records in three minigames. On the Records page, DOWN shows them (UP goes back):
your best, his record, and its tokens, or "Beaten!".

| From story chapter | River Crossing (Serafew) | Tower Stack | Snek | Tokens each |
|---|---|---|---|---|
| the start | 600 | 60 | 30 | 10 |
| 8 | 1200 | 150 | 60 | 20 |
| 10 | 2000 | 250 | 90 | 30 |

- He raises his records as the story goes on: the page shows his latest record in each game.
- Beating a record pays its tokens once, the next time a board opens ("You beat Marlo's record! Tokens paid."). Every
  record that counts is paid, an earlier one too.
- The rows are `gGuildRival` in `Data/Guild/Guild.c`: the game as a quest kind (`GUILD_QUEST_CROSSING_SCORE` with its
  course, `GUILD_QUEST_TOWER_SCORE`, `GUILD_QUEST_SNEK_SCORE`), the story chapter it counts from, the score and the
  tokens. Each has a beaten bit in the save (16 at most), by its place: add new rows at the end.

---

## 🏆 Chapter Awards

**Off by default**: turn on `chapter_awards` in `Data/DesignerConfig/designer-config.c`. When it is on and a story
chapter is cleared, a screen before the save menu hands out three awards and pays their tokens:

| Award | Goes to | Tokens |
|---|---|---|
| MVP | the most damage dealt, plus 10 per foe defeated | 10 |
| Most Foes | the most foes defeated | 5 |
| Unscathed | the least damage taken, among the units that fought (more battles wins a tie) | 5 |

Each award shows the unit's chibi portrait, its name and what won it ("37 damage, 1 defeated"). They appear one at a
time, then the tokens earned in all. A (or START) shows them all at once, and A again goes on to the save menu. The
screen looks like the Extras screens: the title centred on the Bestiary's banner (`ExtrasUi_LoadBanner`) over the
scrolling mural.

- Every battle a player unit fights against an enemy counts, from either side: the damage it dealt, the damage it took
  and whether the foe fell. Staff use and battles with other units do not count.
- A unit needs a claim to win: some damage for the MVP, a foe for Most Foes, a battle for Unscathed. An award nobody
  earns reads "No one this time." and pays nothing. Units that died in the chapter win nothing.
- The tokens are paid when the screen opens, so leaving it early loses nothing. Districts give no awards, and a chapter
  that ends without being cleared (flag 3) shows none.
- The counts start over when a chapter starts, and are kept in the suspend save. Up to 12 units are counted.

The awards' names and tokens are `gChapterAwardTypes` in `Data/Guild/Guild.c`:

```c
const struct ChapterAwardType gChapterAwardTypes[CHAPTER_AWARD_COUNT] = {
	[CHAPTER_AWARD_MVP] = { MSG_AWARDS_MVP, 10 },
	...
```

The screen replaces the vanilla `GameControl_CallPostChapterSaveMenu` (`0x08009DE0`): it starts the awards screen,
which starts the save menu when it ends. Chapter 0x38 still goes without a save menu, as in vanilla.

---

## 🎮 Controls

| Button | |
|---|---|
| L / R, LEFT / RIGHT | Turn the page |
| UP / DOWN | Move the hand. Records: DOWN shows the rival's records, UP the bests again |
| A | Quests: claim a met quest. Prizes: ask, then A again to buy |
| B | Leave |

The hint line at the bottom shows the page's buttons, and for a moment what just happened ("Quest complete! Reward
received.", "Not enough tokens.").

---

## 📍 The Board on the Map

| District | Board (x, y) |
|---|---|
| Serafew | (7, 18), beside the Hex shop |
| Port Kiris | (14, 14), east of the Hex shop |
| Caer Pelyn | (10, 9), by the Hex shop door |
| Jehanna Hall | (14, 6), west of the Hex shop |

- A board is `GUILD_BOARD(x, y)` in the district's `events/traps.c` (`TRAP_GUILD_BOARD`, 32, `bmtrick.h`).
- Rebuild the chapter's `events.lyn.event` after adding one: `traps.c` is included by `events.c`. In a cloud build run
  `make CustomCampaign/Chapters/<Name>/events/events.lyn.event` first; `Tools/cloud/build.sh` alone does not see the change.
- The marker is an 8x8 wooden board with white papers (two tiles at `MAPANIMFX_CHR_R + 0x22`, item icon palette 0),
  drawn by `DistrictNpc.c` with the other district markers. While a quest on that board is met and waiting, a red
  notice blinks on it.
- A on the board runs an event whose ASMC fades the map out, runs the board's screen and brings the map back, as the
  minigame spots do.

---

## ✍️ Adding Quests and Prizes

Both tables are in `Data/Guild/Guild.c`.

```c
QUEST(SERAFEW, GUILD_QUEST_CROSSING_SCORE, 0, 800, 15, 0, 0, MSG_GUILD_Q_SERAFEW_CROSSING),
/*    board    kind                        param target tokens gold item  its line            */
```

- `district` is the district chapter (`0x3B` Serafew, `0x3C` Port Kiris, `0x41` Caer Pelyn, `0x42` Jehanna Hall), or 0
  for every board. A board shows quests in table order, so put a district's easy ones first.
- At most 32 quests (29 used): each has a claimed bit in the save, by its place in the table. Add new quests at the end, so saved
  bits keep meaning the same quests.
- The quest's line is a message in `Contents/Texts/Source/texts/Districts.txt` (add it at the end of the file). Keep it
  under about 34 characters.
- Prizes: one macro per row.

  | Macro | Gives |
  |---|---|
  | `PRIZE(ITEM_X, cost)` | the item, with full uses |
  | `PRIZE_ONCE(ITEM_X, cost)` | the same, once |
  | `TONIC(kind, cost)` | the tonic of that kind (0 HP, 1 Str, 2 Mag, 3 Skl, 4 Spd, 5 Lck, 6 Def, 7 Res, 8 Omni) |
  | `SCROLL(SID_X, cost)` | the skill's scroll, once. Wrap the row in `#if defined(SID_X) && (COMMON_SKILL_VALID(SID_X))` |

  Under them is `struct GuildPrize`: `.kind` is `GUILD_PRIZE_ITEM` (an item index), `GUILD_PRIZE_WORD` (an item word
  given as-is) or `GUILD_PRIZE_SKILL` (a skill id, turned into its scroll by `MakeSkillScrollItem` when bought).
- One-time prizes have a bit each (16 at most, 13 used), by their rank among the one-time prizes: add new ones after the
  last one-time prize, never between. Rows that are not one-time can go anywhere.
- A new kind of quest: a `GUILD_QUEST_*` in `include/kernel/guild.h` and a case in `Guild_QuestProgress`
  (`GuildCore.c`) that reads the progress from saved data.

---

## 💾 Save Data

`gGuildSave` (`struct GuildSave`, 0x10 bytes, `include/kernel/guild.h`): a magic word, the tokens, the tokens earned
in all, the one-time prizes bought, the quests claimed and the rival's records beaten (a bit each). It is a chunk in both save lists
(`Kernel/Wizardry/SaveData/data.event`), after Tower Stack's, and `Guild_NewGame` clears it on a new game. A save made
before the chunk existed loads an empty board.

The chapter awards' counts are `gChapterAwards` (`struct ChapterAwardsData`, 0x64 bytes,
`include/kernel/chapter-awards.h`): a chunk in the suspend save only, after the guild's, cleared when a chapter starts
(`ChapterInit_ResetChapterAwards`). With both, the suspend save has 1271 bytes free and the game save 1149.

The screens' states are `gGuildScreen` (0x80 bytes) and `gChapterAwardsScreen` (0x60 bytes), RAM only.

---

## ⚙️ Enabling and Disabling

The switch is `CONFIG_DISTRICT_GUILD` in `include/configs/configs.h` (on by default). Comment it out to remove the
board, its trap entry, its map markers, its save chunk, the minigames' token payouts and the chapter awards. The boards stay in the trap
data but do nothing.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Installer | [`Guild_Installer.event`](../../Kernel/Wizardry/Guild/Guild_Installer.event) | Included from `Kernel/Wizardry/wizardry.event` (`WIZARDRY_CUSTOM`) |
| Rules | [`GuildCore.c`](../../Kernel/Wizardry/Guild/Source/GuildCore.c) | Tokens, quest progress and claiming, prizes, items, the save chunk |
| Screen | [`GuildScreen.c`](../../Kernel/Wizardry/Guild/Source/GuildScreen.c) | `ProcScr_GuildScreen`: the three pages |
| Map | [`GuildDistrict.c`](../../Kernel/Wizardry/Guild/Source/GuildDistrict.c) | `Guild_OnPressA`, `StartDistrictGuildASMC`, the marker |
| Chapter awards | [`ChapterAwards.c`](../../Kernel/Wizardry/Guild/Source/ChapterAwards.c) | The counts, `ProcScr_ChapterAwards`, the `GameControl_CallPostChapterSaveMenu` replacement ([`LynJump.event`](../../Kernel/Wizardry/Guild/Source/LynJump.event)) |
| Award hooks | [`UnitHooks/data.event`](../../Kernel/Wizardry/UnitHooks/data.event), [`ChapterInitHook/data.event`](../../Kernel/Wizardry/ChapterInitHook/data.event) | `ChapterAwards_OnBattleToUnit`, `ChapterInit_ResetChapterAwards` |
| Tables | [`Guild.c`](../../Data/Guild/Guild.c) | `gGuildQuests`, `gGuildPrizes`, `gGuildUnlocks`, `gGuildFeatured`, `gGuildRival`, `gChapterAwardTypes` |
| Header | [`guild.h`](../../include/kernel/guild.h), [`chapter-awards.h`](../../include/kernel/chapter-awards.h) | Quest kinds, structs, the API |
| Token payouts | `Guild_AddTokens` in `CrossingScreen.c`, `TowerScreen.c`, `FishingScreen.c`, `HexScreen.c`, `Snek.c` | Each game's end |
| Trap type | [`bmtrick.h`](../../Tools/FE-CLib-Mokha/include/bmtrick.h) | `TRAP_GUILD_BOARD` (32) and `GUILD_BOARD(x, y)` |
| A-press hook | [`HookList.event`](../../Kernel/Wizardry/FreeMovement/cFreeMU/HookList.event) | `HookListFMU_TrapList_OnPressA` entry |
| Map markers | [`DistrictNpc.c`](../../Kernel/Wizardry/EnterTown/DistrictNpc.c) | `GUILD_CHR_MAP_BOARD` |
| Texts | [`Districts.txt`](../../Contents/Texts/Source/texts/Districts.txt) | `MSG_GUILD_*`, `MSG_AWARDS_*` |
| RAM | [`config-memmap.s`](../../include/link/config-memmap.s) | `gGuildSave`, `gGuildScreen`, `gChapterAwards`, `gChapterAwardsScreen` |
| Save chunk | [`data.event`](../../Kernel/Wizardry/SaveData/data.event) | `Guild_Save` / `Guild_Load` in both save lists, `Guild_NewGame`; `ChapterAwards_Save` / `ChapterAwards_Load` in the suspend list |
| Switch | [`configs.h`](../../include/configs/configs.h) | `CONFIG_DISTRICT_GUILD` |

---

## ⚠️ Limitations

- Progress counts what you have done in all, not since a quest was posted. A record set earlier meets a quest at once.
- A delivery counts every copy the army holds, including ones a unit has equipped; claiming takes them from the convoy
  first, then from the units.
- Snek is played with the Pied Piper in Port Kiris, next to its guild board (see [Snek](./Minigames/Snek.md)).
