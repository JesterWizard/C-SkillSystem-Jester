# Hex Card Minigame

<p align="center">
  <img src="../../Images/Hex_Preview.png" alt="Hex minigame" width="600" />
</p>

---

## 📑 Index
- [Introduction](#introduction)
- [How to Play](#how-to-play)
- [Controls](#controls)
- [Card Binder](#card-binder)
- [Stakes](#stakes)
- [Opponents](#opponents)
- [Hex Shop](#hex-shop)
- [Card List](#card-list)
- [Enabling and Disabling](#enabling-and-disabling)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

**Hex** is a card game for two players, played on a board of hexagon cells. You play it with your own army: each card is a
character or a monster. The cards you win are kept in a **card binder** that is saved with the game.

Hex is started from the **Hex** row of the preparations menu. The row's help text is "Play Hex with your army and view your
cards." The Hex players of the four districts also play you, each with a fixed deck (see [Districts](../Districts.md)). The game is
optional: it is controlled by the compile-time switch `CONFIG_HEX_CARDS` (see [Enabling and Disabling](#enabling-and-disabling)).

## 🎲 How to Play

- The board has 14 hexagon cells, in 5 rows of 2, 3, 4, 3 and 2 cells.
- Each player has 7 cards in hand. The player is blue, the opponent is red.
- The players take turns, and each turn places one card from the hand on an empty cell. The game ends when the board is full
  (14 placements).
- Each card has an **attack**, an **HP** (maximum hit points) and a **reach**: the directions in which it attacks. The reach
  directions are up-left, up-right, right, down-right, down-left and left.
- **Placement** works in three steps:
  1. **Attack:** every enemy card in a neighbour cell in the placed card's reach directions loses HP equal to the placed card's attack.
  2. **Counterattack:** the placed card loses the attack of every enemy neighbour whose reach points back at it.
  3. **Flip:** every card with 0 HP or less changes owner. A flipped card gets its full HP back; a card that was only damaged keeps its HP.
- **Scoring:** a player's score is the number of cards that player owns on the board, plus the cards still left in hand.
- **Winner:** when the board is full, the player with the higher score wins. Equal scores are a draw.

## 🎮 Controls

The Hex screen has several states. The controls for each are below. **A** confirms, **B** cancels or goes back.

### Front menu

| Input | Action |
|------|--------|
| UP / DOWN | Choose `Play` or `Cards` |
| A | Open the highlighted row: `Play` opens the opponent list, `Cards` opens the collection |
| B | Leave for the preparations menu |

### Opponent list (`Choose an opponent`)

| Input | Action |
|------|--------|
| UP / DOWN | Move the cursor over your living units (held keys repeat) |
| A | Start the hand picker against the highlighted unit |
| B | Back to the front menu |

### Collection (`Cards  N/44`)

| Input | Action |
|------|--------|
| UP / DOWN | Scroll the list of the 44 card types |
| B | Back to the front menu |

### Hand picker (`Your cards`, `Your hand`)

| Input | Action |
|------|--------|
| UP / DOWN | Move the list cursor (held keys repeat) |
| A | Add a copy of the highlighted card to your hand |
| B | Remove the last card from your hand. With no card in hand, back to the opponent list (no match, no penalty) |

When your hand has 7 cards, the list shows `Accept` and `Change`:

| Input | Action |
|------|--------|
| UP / DOWN | Choose `Accept` or `Change` |
| A | Confirm the choice |
| B | Same as `Change`: remove the last card and go back to the hand picker |

`Accept` shows the opponent's hand (`Your opponent's hand`) and then starts the match. A skips the wait.

### Match: choosing a card (`Blue begins`, your turn)

| Input | Action |
|------|--------|
| UP / DOWN | Step over the cards in your hand that are not placed yet |
| A | Choose the highlighted card and go to cell choice |
| B | Nothing: a match is played to its end |

### Match: choosing a cell

| Input | Action |
|------|--------|
| D-pad | Move to a neighbouring cell. UP goes to up-left or up-right, DOWN to down-left or down-right, LEFT and RIGHT to left or right. An empty neighbour is preferred. When there is none, the cursor moves to the cell before or after in reading order |
| A | Place the card. A full or illegal cell plays the error sound |
| B | Back to card choice |

The opponent's turn is announced with `<Name> is thinking`. After a placement the board stays as it was for 30 frames, then the
attack shows with a shake and a hit sound (if anything changed).

### Match end and stakes

| Input | Action |
|------|--------|
| A (at the result banner) | Decide the stakes, see [Stakes](#stakes) |
| UP / DOWN (take screen) | Choose one of the opponent's 7 starting cards |
| A (take screen) | Take the chosen card into your card binder |
| A (reward banner) | Skip the banner and go back to the front menu |

## 🃏 Card Binder

- Your cards are kept in a **card binder**, part of the save data (`gHexSave`, struct `HexSaveData`, 0x54 bytes). It holds the
  copies of each card type and the district claim counters. It is saved with the game and with a suspend.
- A card type is one of the 44 types in the [Card List](#card-list). The binder holds up to 255 copies of each type.
- The collection (`Cards`) shows the binder.
- **Hex Card item:** the item `CONFIG_ITEM_INDEX_HEX_CARD` (`0xEC` in the default config) still exists. It is the way a card
  reaches the binder. Its type is stored in the uses byte, and the item word is `(type << 8) | CONFIG_ITEM_INDEX_HEX_CARD`.
- The item shows the name of the card. Its icon is the hexagonal card picture in item icon slot `ITEM_ICON_HEX_CARD` (`0x8E`,
  `Data/CustomItems/HexCard/`), the same for every card type; the row's `iconId` is that slot.
- A Hex Card item cannot be equipped.
- **Migration:** when the Hex screen opens, every Hex Card item in the convoy and in the inventory of a living player unit moves
  into the binder. An item stays where it is only when its type is already at 255 copies.
- **Starter gift:** when the Hex screen opens and the binder is empty, 7 starter cards go into the binder: 2 Ross, 2 Amelia,
  1 Ewan, 1 Franz and 1 Neimi.
- **Old saves:** a save made before the binder existed loads an empty binder. Its Hex Card items still move into the binder
  when the Hex screen opens.

## 🏆 Stakes

Stakes are settled when the match ends (you press A at the result banner).

| Result | What happens |
|------|-----|
| Win | If you hold fewer than 255 copies of at least one card type in the opponent's starting hand, you take one of those cards (`TAKE`). You choose the card with UP / DOWN and take it into the binder with A. Otherwise you gain no card (`No card gained.`). |
| Loss | If you own more than 7 cards, the opponent takes your best card from the hand you just played: the highest tier, then the highest attack + max HP, with the first card in hand order winning a tie. Against a district NPC, the NPC takes a random card from the hand you played instead. If you own 7 or fewer cards, nothing changes (`Your cards are safe this time.`). |
| Draw | Nothing changes hands (`Draw. No cards change hands.`). |

Other rules:

- You can hold at most 255 copies of one card type. A take of a type you already hold 255 of is refused (`You can't hold more`).
  You then choose another card.
- Gains and losses are shown in the banner: `Gained <card>!` or `Lost <card>...`.

## 👥 Opponents

In the preparations menu, the opponents are the living units of your army. Each unit gives you one opponent, with a deck and an AI
level, based on the unit's **effective level**: its level, plus 20 if the unit is promoted.

- **AI level:** 1 if the effective level is below 10, otherwise 2. An army opponent never plays random moves.
- **Tier cap:** 1 if the effective level is below 10, 2 if below 20, otherwise 3.
- **Deck:** the first card is the card of the unit's own character (when there is one, even above the tier cap). The rest of the
  7-card deck is drawn from the character card types (0 to 31) of tier at or below the cap, with at most 2 copies of one type in
  the deck. Monster cards are never drawn for a unit.
- The deck is always the same for the same unit at the same level and promotion. A unit that gains a level, or is promoted, may get a
  different deck and AI level the next time you pick it.
- **Opponent's moves:** the opponent picks the move with the best score difference (level 1: right after the move; level 2: after
  your best reply). On a tie it prefers the move that does more damage to your cards (the HP your cards lose, a card that changes
  owner counting its full HP), then the move next to more of your cards, then a seeded pick.

### District opponents

Each of the four districts (Serafew, Port Kiris, Caer Pelyn and Jehanna Hall) has two Hex players. They are NPCs, not units of
your army. Each one has a fixed 7-card deck and an AI level of 1 or 2. The players and their decks are listed in
[Districts](../Districts.md#hex-players).

- **Opening a match:** the Hex screen opens at the hand picker against that NPC's deck. The front menu and the opponent list
  are not shown. B with no card in hand leaves the screen. The screen also closes after the stakes.
- **Rare cards:** a rare card in an NPC's deck has a claim counter. Each win that takes it adds one. When the counter reaches
  its cap, the NPC plays a fallback card in its place.
- **Stakes:** a win takes one of the NPC's 7 cards. A loss takes a random card from the hand you played, if you own more than
  7 cards. A draw changes nothing.
- **Monster cards:** types 32 to 43 are only in the decks of Caer Pelyn and Jehanna Hall.

## 🛒 Hex Shop

Each district has a Hex shop, run by a witch. It is the vanilla shop screen in Hex mode. The building, the door and the full
price list are in [Districts](../Districts.md#hex-shop).

- **Prices:** a ware costs copies of one card type. For example, Serafew's Vulnerary costs 2 Ross. The prices are in
  [`Data/Hex/HexShops.c`](../../../Data/Hex/HexShops.c).
- **Greyed wares:** a ware you can't afford is grey. If you press A on it, the witch says you haven't the cards.
- **Buying:** A on an affordable ware, then Yes. The cards leave the binder and the item goes to Eirika.
- **No gold:** the shop takes cards only. No gold is spent and no gold box shows.
- **Leaving:** B leaves, with the witch's goodbye.

## 🂠 Card List

All 44 card types. The "Starter copies" column is the number of copies in the starter gift (7 cards in total). Reach
directions: UL up-left, UR up-right, R right, DR down-right, DL down-left, L left.

| # | Character | Attack | HP | Reach | Tier | Starter copies |
|---:|---|---:|---:|---|---:|---:|
| 0 | Ross | 1 | 2 | UR R DL | 1 | 2 |
| 1 | Amelia | 1 | 3 | UL UR | 1 | 2 |
| 2 | Ewan | 3 | 1 | R L | 1 | 1 |
| 3 | Franz | 2 | 2 | DR DL | 1 | 1 |
| 4 | Colm | 2 | 1 | UL UR DR | 1 | 0 |
| 5 | Neimi | 1 | 1 | UL UR DR DL | 1 | 1 |
| 6 | Lute | 4 | 1 | R | 1 | 0 |
| 7 | Garcia | 2 | 3 | L | 1 | 0 |
| 8 | Natasha | 1 | 2 | UR R DR | 1 | 0 |
| 9 | Moulder | 2 | 3 | UR | 1 | 0 |
| 10 | Gilliam | 3 | 2 | DR | 1 | 0 |
| 11 | Vanessa | 2 | 1 | UL R L | 1 | 0 |
| 12 | Kyle | 3 | 2 | UL UR R | 2 | 0 |
| 13 | Forde | 3 | 3 | R L | 2 | 0 |
| 14 | Dozla | 1 | 5 | DR DL | 2 | 0 |
| 15 | Joshua | 4 | 2 | UR DL | 2 | 0 |
| 16 | Tethys | 2 | 2 | UL UR DR DL | 2 | 0 |
| 17 | Gerik | 2 | 4 | UL UR | 2 | 0 |
| 18 | Marisa | 4 | 3 | R | 2 | 0 |
| 19 | L'Arachel | 2 | 2 | UL R DR L | 2 | 0 |
| 20 | Innes | 3 | 2 | UR DR L | 2 | 0 |
| 21 | Cormag | 3 | 4 | UR | 2 | 0 |
| 22 | Saleh | 4 | 1 | UR R DR | 2 | 0 |
| 23 | Tana | 2 | 4 | UL DL | 2 | 0 |
| 24 | Seth | 4 | 4 | UL UR | 3 | 0 |
| 25 | Myrrh | 2 | 4 | UL UR R L | 3 | 0 |
| 26 | Duessel | 2 | 5 | UL UR R | 3 | 0 |
| 27 | Ephraim | 4 | 4 | R L | 3 | 0 |
| 28 | Glen | 2 | 5 | UL R DL | 3 | 0 |
| 29 | Valter | 2 | 5 | UL DR L | 3 | 0 |
| 30 | Lyon | 4 | 2 | UL UR DR DL | 3 | 0 |
| 31 | Eirika | 3 | 5 | UL UR | 3 | 0 |
| 32 | Revenant | 1 | 3 | R DR | 1 | 0 |
| 33 | Bonewalker | 2 | 2 | UL L | 1 | 0 |
| 34 | Mogall | 3 | 1 | UR DL | 1 | 0 |
| 35 | Bael | 2 | 2 | UL R DR | 1 | 0 |
| 36 | Gwyllgi | 3 | 1 | UL DR DL | 1 | 0 |
| 37 | Wight | 3 | 3 | UL L | 2 | 0 |
| 38 | Tarvos | 3 | 3 | R DR | 2 | 0 |
| 39 | Arch Mogall | 4 | 2 | UL R DL | 2 | 0 |
| 40 | Cyclops | 4 | 3 | UL | 2 | 0 |
| 41 | Gargoyle | 2 | 3 | UR R DL L | 2 | 0 |
| 42 | Deathgoyle | 3 | 4 | UL R DR L | 3 | 0 |
| 43 | Draco Zombie | 4 | 5 | UL L | 3 | 0 |

Monster cards (32 to 43) are named after their monster class, and their portraits are 32x32 minimugs from `gHexMonsterArt`,
cropped from the FE8 class cards by the [monster mug script](../../../Tools/hex/gen_monster_mugs.py).

The collection screen (`Cards`) shows the same 44 types. A type you have never owned is shown as `???`.

## ⚙️ Enabling and Disabling

The switch is in [`include/configs/configs.h`](../../../include/configs/configs.h):

```c
/* Hex cards: the Hex Card item (one card per type, the type is the uses byte) and the Hex minigame code.
 * Comment out the next line to remove the Hex minigame and the card item from the build. */
#define CONFIG_HEX_CARDS
```

To remove the minigame, comment out `#define CONFIG_HEX_CARDS`. To turn it back on, restore the line.

The compile-time switch removes the code and the card item. To keep the card item and only hide the menu row, use the runtime
switch `prep_menu_hex` in [`designer-config.c`](../../../Data/DesignerConfig/designer-config.c). It defaults to `true`. Set it to
`false` to hide the Hex row in the preparations menu (the other prep rows are switched the same way). It has no effect when
`CONFIG_HEX_CARDS` is off, because the row is not compiled then.

**Cloud sessions** (`Tools/cloud/build.sh`) do not regenerate tracked `.lyn.event` files whose `.c` file is unchanged in git. After
you turn the switch on or off, delete the guarded outputs before you build, or the build fails (for example with
`Undefined identifier: HexCard_NameId` when the switch is off):

```
rm -f Data/ItemSys/Source/Items.lyn.event \
  Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/DuraItem.lyn.event \
  Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/ItemInfos.lyn.event \
  Kernel/Wizardry/SkillEffects/MenuSkills/Reinforce.lyn.event \
  Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.lyn.event \
  Data/Hex/HexCardTypes.lyn.event \
  Data/Hex/HexNpcDecks.lyn.event \
  Data/Hex/HexShops.lyn.event \
  Data/Hex/HexMonsterArt.lyn.event
Tools/cloud/build.sh
```

`build.sh` regenerates the deleted files. Afterwards, `git diff` on those files should show only the changes for this feature
(`AtMenu.lyn.event` in particular).

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Switch | [`include/configs/configs.h`](../../../include/configs/configs.h) | `CONFIG_HEX_CARDS` and `CONFIG_ITEM_INDEX_HEX_CARD` |
| Preparations menu row | [`Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c`](../../../Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c) | The `Hex` row and its help text |
| Entry point | [`Kernel/Wizardry/Minigames/Hex/Source/HexPrep.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexPrep.c) | `StartHexScreen_FromPrep` starts the screen from the menu |
| Screen | [`Kernel/Wizardry/Minigames/Hex/Source/HexScreen.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexScreen.c) | Menus, opponent list, hand picker, match, stakes |
| Rules and AI | [`Kernel/Wizardry/Minigames/Hex/Source/HexCore.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexCore.c) | Placement, scoring, winner, computer opponent |
| Rules header | [`include/kernel/hex-core.h`](../../../include/kernel/hex-core.h) | Board layout, directions, tile rules |
| Cards | [`Kernel/Wizardry/Minigames/Hex/Source/HexCards.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexCards.c) | Card item, binder helpers, item migration, starter gift |
| Binder save data | [`Kernel/Wizardry/Minigames/Hex/Source/HexSave.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexSave.c) | `gHexSave`: the binder and the claim counters, the save chunk |
| Card header | [`include/kernel/hex.h`](../../../include/kernel/hex.h) | Card type struct, binder and district structs |
| Card table | [`Data/Hex/HexCardTypes.c`](../../../Data/Hex/HexCardTypes.c) | The 44 card types |
| District matches | [`Kernel/Wizardry/Minigames/Hex/Source/HexDistrict.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexDistrict.c) | Talk-event entry points, district NPC decks, the shop's rows and payment |
| Hex shop | [`Kernel/Wizardry/Minigames/Hex/Source/HexShopVanilla.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexShopVanilla.c) | The shop screen in Hex mode: the witch, the prices, the buy |
| Shop hooks | [`Kernel/Wizardry/LimitedShopStock/src/LimitedShopStock.c`](../../../Kernel/Wizardry/LimitedShopStock/src/LimitedShopStock.c) | Hex branches of the vanilla shop |
| District decks | [`Data/Hex/HexNpcDecks.c`](../../../Data/Hex/HexNpcDecks.c) | The fixed deck of each district Hex player |
| Shop rows | [`Data/Hex/HexShops.c`](../../../Data/Hex/HexShops.c) | The wares of each district shop and their card prices |
| Monster art | [`Data/Hex/HexMonsterArt.c`](../../../Data/Hex/HexMonsterArt.c) | Generated: the 12 monster minimugs. Do not edit by hand |
| Monster art script | [`Tools/hex/gen_monster_mugs.py`](../../../Tools/hex/gen_monster_mugs.py) | Crops the minimugs from the FE8 class cards (`--check`, `--preview`) |
| Card art | [`Kernel/Wizardry/Minigames/Hex/Source/HexArt.c`](../../../Kernel/Wizardry/Minigames/Hex/Source/HexArt.c) | Tiles, digits, cursors, frame and palettes |
| Item name | [`Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/DuraItem.c`](../../../Kernel/Wizardry/ItemSys/ItemInfoRemap/Source/DuraItem.c) | Shows the character's name for a card |
| Text | [`Contents/Texts/Source/texts/Hex.txt`](../../../Contents/Texts/Source/texts/Hex.txt) | Item name and description, preparations row and help text |
| Shop text | [`Contents/Texts/Source/texts/Districts.txt`](../../../Contents/Texts/Source/texts/Districts.txt) | The witch's lines (`MSG_HEXSHOP_*`) |
| Developer notes | [`Kernel/Wizardry/Minigames/Hex/README.md`](../../../Kernel/Wizardry/Minigames/Hex/README.md) | Detailed notes for developers |

## ⚠️ Limitations

- **Uses number on the card.** The uses byte holds the card type, not a durability. The item code reports a uses value of 1 for
  Hex Cards (`GetItemUses` in `ItemInfos.c`), so a menu that shows an item's uses can show `1` on a card.
- **Flipped cells.** The developer notes list the flashing of flipped cells as not done.
- **Starter gift.** It is handed out whenever the screen opens with an empty binder, so a player who loses every card gets the starter set again on the next open.
- **Living units only.** Opponents use living units of your army. A Hex Card item in a dead unit's inventory is not moved into the binder.
- **No quitting a match.** Once a match starts, B does nothing until it ends.
- **Monster art.** The monster minimugs are generated by a script. Some of them (Revenant, Tarvos) are rough. They can be touched up by editing the crop boxes in `Tools/hex/gen_monster_mugs.py`.
- **Hex shop.** The shop is the vanilla shop screen. Its Hex parts are hooks in `LimitedShopStock.c` and `HexShopVanilla.c`, so a change to the vanilla shop code can change the Hex shop too.
