#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "prepscreen.h"
#include "hex.h"
#include "kernel/hex-core.h"

#ifdef CONFIG_HEX_CARDS

/*
 * The Hex board screen (Preparations > Hex): a match of the player's hand (blue, left) against the red hand (right). The
 * board (14 cells, 2-3-4-3-2), both hands, both scores and the name bar. The state machine is in the "match" section below
 * (BEGIN, PICK_TILE, PICK_CELL, RESOLVE, AI_TURN, END); README.md lists the keys.
 *
 * Layers: BG1 wood (priority 3, charblock 2 tiles 0..3, screenblock 20; tile 4 is the solid name-bar box), BG0 text (priority
 * 0, the name bar). OBJ 1D mapping. OBJ palette banks 0 (red frame) and 1 (blue frame), 2..13 the portraits of the cards on
 * screen. Every frame draws the sprites again (ClearSprites, then PutSprite): a later PutSprite is drawn in front of an
 * earlier one, so each sprite is pushed back to front. Board layout and card art: docs/custom.md "Hex" (the Minish Cap mod).
 */

#define PL_HEX_EXIT 2

#define HEX_SPRITE_LAYER 1
#define HEX_OBJ_VRAM 0x06010000
#define HEX_OBJ_TILE(t) ((void *)(HEX_OBJ_VRAM + (t) * 0x20))

/* OBJ tiles (1D mapping, 32 bytes each, tile numbers from HEX_OBJ_VRAM). */
#define HEX_OBJ_OUTLINE 0          /* 16 tiles: gHexEmptyTiles */
#define HEX_OBJ_DIGIT_WHITE 16     /* 11 tiles: gHexDigitTiles, digit d = 16 + d, blank = 26 */
#define HEX_OBJ_DIGIT_GREEN 27     /* 11 tiles: gHexDigitGreenTiles, digit d = 27 + d */
#define HEX_OBJ_CURSOR 38          /* 8 tiles: gHexCursorTiles, left arrow 38, down arrow 42 (4 tiles each, 16x16) */
#define HEX_OBJ_CURSOR_DOWN 4      /* offset of the down arrow from HEX_OBJ_CURSOR */
#define HEX_OBJ_SCORE 512          /* 40 tiles: gHexScoreTiles, digit d = 512 + 4 d (16x16) */
#define HEX_OBJ_TYPE_BASE 48       /* per card type 32 tiles: frame at base, portrait at base + 16 */
#define HEX_OBJ_TYPE_TILES 32
#define HEX_OBJ_PORTRAIT_OFFSET 16
#define HEX_OBJ_BANK_FIRST 2       /* OBJ palette banks 2..15: the portraits */
#define HEX_OBJ_BANK_LAST 15
#define HEX_BANK_RED 0             /* OBJ palette bank of the red frame (and of the red digits, scores, cursors) */
#define HEX_BANK_BLUE 1
#define HEX_NO_BANK 0xFF
#define HEX_DIGIT_BLANK 10

/* Digit spots: top-left of the 5x7 footprint, in box coordinates (include/custom/hexArt.h in the Minish Cap mod). */
#define HEX_DIGIT_ATTACK_DX 7
#define HEX_DIGIT_ATTACK_DY 9
#define HEX_DIGIT_MAX_DX 7
#define HEX_DIGIT_MAX_DY 16
#define HEX_DIGIT_HP_DX 20
#define HEX_DIGIT_HP_DY 16
#define HEX_BOX 32
#define HEX_BOARD_TOP 10

/* Hands and scores (blue on the left, red on the right). */
#define HEX_HAND_BLUE_X 4
#define HEX_HAND_RED_X 204
#define HEX_HAND_Y 4
#define HEX_HAND_STEP 20
#define HEX_SCORE_Y 128
#define HEX_SCORE_BLUE_X 44
#define HEX_SCORE_RED_X 168
#define HEX_CURSOR_GAP 2

/* BG: wood and the name bar in charblock 2 (0x06008000), screenblock 20 (0x0600A000). */
#define HEX_BG_CHARBLOCK 2
#define HEX_BG_SCREENBLOCK 20
#define HEX_BG_PRIORITY 3
#define HEX_BG_WOOD_PAL 1
#define HEX_BG_BAR_TILE 4
#define HEX_BG_BAR_PAL 2
#define HEX_BG_BAR_COLOUR 0x10EB   /* dark brown (#5A3A22), palette index 1 of the bar bank */
#define HEX_NAME_ROW 18            /* name bar rows 18..19 (y 144..159) */
#define HEX_NAME_COLS 30

/* Match states (HexScreenState.state). */
#define HEX_STATE_BEGIN 0      /* "Blue begins" / "Red begins" (60 frames, or A) */
#define HEX_STATE_PICK_TILE 1  /* blue chooses a hand card: UP/DOWN, A = next, B = leave (forfeit) */
#define HEX_STATE_PICK_CELL 2  /* blue chooses a cell: D-pad, A = place, B = back to PICK_TILE */
#define HEX_STATE_RESOLVE 3    /* the placement is shown (40 frames), then the next turn or END */
#define HEX_STATE_AI_TURN 4    /* red: "Red is thinking" for one frame, then the AI places */
#define HEX_STATE_END 5        /* the result banner; A = leave */

#define HEX_BEGIN_FRAMES 60
#define HEX_RESOLVE_FRAMES 40
#define HEX_AI_LEVEL 2

/* Name bar codes: 0..HEX_CARD_TYPES-1 is the name of that card type; the rest are banners. */
#define HEX_BAR_NONE 0xFF
#define HEX_BAR_BLUE_BEGINS 0xE0
#define HEX_BAR_RED_BEGINS 0xE1
#define HEX_BAR_RED_THINKING 0xE2
#define HEX_BAR_VICTORY 0xE3
#define HEX_BAR_DEFEAT 0xE4
#define HEX_BAR_DRAW 0xE5

/* Sounds (FE8 SE ids, the same ones the prep menu uses: AtMenu.c). */
#define HEX_SE_SELECT SONG_SE_SYS_WINDOW_SELECT1
#define HEX_SE_CANCEL SONG_SE_SYS_WINDOW_CANSEL1
#define HEX_SE_CURSOR_UD SONG_SE_SYS_CURSOR_UD1
#define HEX_SE_CURSOR_LR SONG_SE_SYS_CURSOR_LR1
#define HEX_SE_ERROR SONG_6C

static const s16 sHexBoxX[HEX_CELLS] = { 90, 118, 76, 104, 132, 62, 90, 118, 146, 76, 104, 132, 90, 118 };
static const s16 sHexBoxY[HEX_CELLS] = { 10, 10, 34, 34, 34, 58, 58, 58, 58, 82, 82, 82, 106, 106 };

/* The red hand: Colm, Lute, Garcia, Kyle, Forde, Dozla, Joshua. */
static const u8 sHexRedHand[HEX_HAND] = { 4, 6, 7, 12, 13, 14, 15 };

static const u16 sHexBarPal[16] = { 0x0000, HEX_BG_BAR_COLOUR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const u32 sHexBarTile[8] = { 0x11111111, 0x11111111, 0x11111111, 0x11111111,
				    0x11111111, 0x11111111, 0x11111111, 0x11111111 };

/*
 * The match state lives in gHexScreenState (include/link/config-memmap.s, 0x180 bytes), not in the proc: a proc is only
 * 0x6C bytes (struct Proc in FE-CLib proc.h), so the proc keeps the small fields and the text, and the rest goes here.
 */
#define HEX_STATE_SIZE 0x180
#define HEX_ST ((struct HexScreenState *)gHexScreenState)

struct HexScreenState {
	struct HexGame game;
	u8 blueHand[HEX_HAND];
	u8 blueCount;
	u8 typeBank[HEX_CARD_TYPES];
	struct HexTileType types[HEX_CARD_TYPES];
	u8 state;
	u8 cell;        /* PICK_CELL: the cell under the down arrow */
	u8 timer;       /* frames in the current state */
	u8 aiHand;      /* the AI's last choice (hand slot, cell) */
	u8 aiCell;
	u8 placedType;  /* card type of the last placement (its name shows during RESOLVE) */
	u8 bar;         /* the code drawn in the name bar (HEX_BAR_NONE: nothing yet) */
};

/* Compile-time check: a constant division by zero (an error) if the state outgrows gHexScreenState. */
enum { HEX_STATE_FITS = 1 / (sizeof(struct HexScreenState) <= HEX_STATE_SIZE) };

struct ProcHexScreen {
	PROC_HEADER;
	struct Text nameText;
	struct DispCnt dispBak;
	struct BgCnt bg1Bak;
	u8 sel;
};

/* The blue hand from the collection: the first cards, in type order (up to HEX_HAND). Returns the count. */
static int HexScreen_BlueHand(u8 *hand)
{
	int n = 0;
	int type, k;

	for (type = 0; type < HEX_CARD_TYPES && n < HEX_HAND; type++) {
		int owned = HexCol_Count(type);

		for (k = 0; k < owned && n < HEX_HAND; k++)
			hand[n++] = (u8)type;
	}

	return n;
}

/* The starter set (the copies HexCol_GiveStarters hands out), used when the collection is empty. Returns the count. */
static int HexScreen_StarterHand(u8 *hand)
{
	int n = 0;
	int type, k;

	for (type = 0; type < HEX_CARD_TYPES && n < HEX_HAND; type++) {
		int copies = gpHexCardTypes[type].starter;

		for (k = 0; k < copies && n < HEX_HAND; k++)
			hand[n++] = (u8)type;
	}

	return n;
}

/* Fills the slots from n up by repeating the first n cards. With n == 0 the hand is type 0 (cannot happen with the starters). */
static void HexScreen_FillHand(u8 *hand, int n)
{
	int i;
	int k = 0;

	if (n == 0) {
		for (i = 0; i < HEX_HAND; i++)
			hand[i] = 0;
		return;
	}
	for (i = n; i < HEX_HAND; i++) {
		hand[i] = hand[k++];
		if (k >= n)
			k = 0;
	}
}

/* The blue hand of the match: the collection, else the starters; always HEX_HAND cards. */
static void HexScreen_MakeBlueHand(void)
{
	int n = HexScreen_BlueHand(HEX_ST->blueHand);

	if (n == 0)
		n = HexScreen_StarterHand(HEX_ST->blueHand);
	HexScreen_FillHand(HEX_ST->blueHand, n);
	HEX_ST->blueCount = HEX_HAND;
}

/* The engine's tile table (attack, max HP, reach) from the card table: index = card type. */
static void HexScreen_MakeTypes(void)
{
	int t;

	for (t = 0; t < HEX_CARD_TYPES; t++) {
		HEX_ST->types[t].attack = gpHexCardTypes[t].attack;
		HEX_ST->types[t].maxHp = gpHexCardTypes[t].maxHp;
		HEX_ST->types[t].reach = gpHexCardTypes[t].reach;
	}
}

/* One banked card type: its frame (reach mask, owner-neutral) and its portrait (the chibi, clipped to the hexagon). */
static void HexScreen_LoadType(int type, int bank)
{
	const struct HexCardType *info = &gpHexCardTypes[type];
	u32 base = HEX_OBJ_TYPE_BASE + (u32)(bank - HEX_OBJ_BANK_FIRST) * HEX_OBJ_TYPE_TILES;
	const struct FaceData *face = GetPortraitData(GetCharacterData(info->pid)->portraitId);
	const u32 *src = (const u32 *)gGenericBuffer;
	u32 keep[128];
	int i;

	CpuFastCopy(gHexFrameTiles[info->reach & 0x3F], HEX_OBJ_TILE(base), 512);

	Decompress(face->imgChibi, gGenericBuffer);
	for (i = 0; i < 128; i++)
		keep[i] = src[i] & gHexPortraitKeep[i];
	CpuFastCopy(keep, HEX_OBJ_TILE(base + HEX_OBJ_PORTRAIT_OFFSET), 512);

	ApplyPalette(face->pal, 0x10 + bank);
}

/* Banks the types of both hands (blue, then red), then loads their art. */
static void HexScreen_LoadArt(void)
{
	int type, i;
	int next = HEX_OBJ_BANK_FIRST;

	for (type = 0; type < HEX_CARD_TYPES; type++)
		HEX_ST->typeBank[type] = HEX_NO_BANK;

	for (i = 0; i < HEX_ST->blueCount; i++) {
		type = HEX_ST->blueHand[i];
		if (HEX_ST->typeBank[type] == HEX_NO_BANK && next <= HEX_OBJ_BANK_LAST)
			HEX_ST->typeBank[type] = (u8)next++;
	}
	for (i = 0; i < HEX_HAND; i++) {
		type = sHexRedHand[i];
		if (HEX_ST->typeBank[type] == HEX_NO_BANK && next <= HEX_OBJ_BANK_LAST)
			HEX_ST->typeBank[type] = (u8)next++;
	}

	for (type = 0; type < HEX_CARD_TYPES; type++) {
		if (HEX_ST->typeBank[type] != HEX_NO_BANK)
			HexScreen_LoadType(type, HEX_ST->typeBank[type]);
	}
}

/* OBJ tiles and palettes that do not depend on the cards: the cell outline, digits, score digits, cursors, frames. */
static void HexScreen_LoadSprites(void)
{
	CpuFastCopy(gHexEmptyTiles, HEX_OBJ_TILE(HEX_OBJ_OUTLINE), 512);
	CpuFastCopy(gHexDigitTiles, HEX_OBJ_TILE(HEX_OBJ_DIGIT_WHITE), 352);
	CpuFastCopy(gHexDigitGreenTiles, HEX_OBJ_TILE(HEX_OBJ_DIGIT_GREEN), 352);
	CpuFastCopy(gHexCursorTiles, HEX_OBJ_TILE(HEX_OBJ_CURSOR), 256);
	CpuFastCopy(gHexScoreTiles, HEX_OBJ_TILE(HEX_OBJ_SCORE), 1280);

	ApplyPalette(gHexFramePalRed, 0x10 + HEX_BANK_RED);
	ApplyPalette(gHexFramePalBlue, 0x10 + HEX_BANK_BLUE);
}

/* The wood (BG1) and the name-bar tile and palette. */
static void HexScreen_LoadBackground(void)
{
	int x, y;

	CpuFastCopy(gHexWoodBgTiles, (void *)0x06008000, 128);
	CpuFastCopy(sHexBarTile, (void *)(0x06008000 + HEX_BG_BAR_TILE * 0x20), 32);
	ApplyPalette(gHexWoodBgPal, HEX_BG_WOOD_PAL);
	ApplyPalette(sHexBarPal, HEX_BG_BAR_PAL);

	for (y = 0; y < 20; y++) {
		for (x = 0; x < 30; x++) {
			int tile = (x & 1) | ((y & 1) << 1);
			int bank = HEX_BG_WOOD_PAL;

			if (y >= HEX_NAME_ROW && x >= 1 && x < 1 + 28) {
				tile = HEX_BG_BAR_TILE;
				bank = HEX_BG_BAR_PAL;
			}
			gBG1TilemapBuffer[TILEMAP_INDEX(x, y)] = (u16)(tile | (bank << 12));
		}
	}
	/* Written to the screenblock directly as well: the BG1 map sync does not reach it on this screen. */
	CpuFastCopy(gBG1TilemapBuffer, (void *)(0x06000000 + HEX_BG_SCREENBLOCK * 0x800), 0x800);
}

/* The first unused slot of owner's hand, 0 if none is left. */
static u8 HexScreen_FirstUnused(const struct HexGame *g, u8 owner)
{
	u8 i;

	for (i = 0; i < HEX_HAND; i++) {
		if (g->hand[owner][i] != HEX_USED)
			return i;
	}
	return 0;
}

/* The first empty cell, 0 if the board is full. */
static u8 HexScreen_FirstEmpty(const struct HexGame *g)
{
	u8 c;

	for (c = 0; c < HEX_CELLS; c++) {
		if (g->cell[c].owner == HEX_EMPTY)
			return c;
	}
	return 0;
}

/* A new match: an empty board, both hands, the first player from the game clock. */
static void HexScreen_NewMatch(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	u8 first = (GetGameClock() & 1) ? HEX_RED : HEX_BLUE;

	Hex_Init(&h->game, h->blueHand, sHexRedHand, first);
	h->state = HEX_STATE_BEGIN;
	h->timer = 0;
	h->aiHand = HEX_USED;
	h->aiCell = HEX_USED;
	h->placedType = 0;
	h->bar = HEX_BAR_NONE;
	h->cell = HexScreen_FirstEmpty(&h->game);
	proc->sel = HexScreen_FirstUnused(&h->game, HEX_BLUE);
}

static void HexScreen_Push(int x, int y, const u16 *object, u32 tile, u32 bank)
{
	PutSprite(HEX_SPRITE_LAYER, x, y, object, OAM2_CHR(tile) + OAM2_PAL(bank) + OAM2_LAYER(0));
}

/* One 8x8 digit: a blank for anything outside 0..9. current selects the green set (current HP). */
static void HexScreen_Digit(int x, int y, int value, bool32 current, u32 bank)
{
	u32 tile;

	if (value < 0 || value > 9)
		value = HEX_DIGIT_BLANK;
	tile = (current ? HEX_OBJ_DIGIT_GREEN : HEX_OBJ_DIGIT_WHITE) + (u32)value;
	HexScreen_Push(x, y, gObject_8x8, tile, bank);
}

/* The attack and max HP digits of a card whose box is at (x, y); placed cards also show the current HP. */
static void HexScreen_CardDigits(int x, int y, int type, u32 bank, bool32 placed, int hp)
{
	const struct HexCardType *info = &gpHexCardTypes[type];

	HexScreen_Digit(x + HEX_DIGIT_ATTACK_DX, y + HEX_DIGIT_ATTACK_DY, info->attack, FALSE, bank);
	HexScreen_Digit(x + HEX_DIGIT_MAX_DX, y + HEX_DIGIT_MAX_DY, info->maxHp, FALSE, bank);
	if (placed)
		HexScreen_Digit(x + HEX_DIGIT_HP_DX, y + HEX_DIGIT_HP_DY, hp, TRUE, bank);
}

/*
 * The frame and the portrait of a card (the portrait on top of the frame); the frame palette is the owner's. PutSprite draws
 * a later call in front of an earlier one, so every sprite is pushed back to front.
 */
static void HexScreen_CardBody(int x, int y, int type, u32 owner)
{
	u32 base = HEX_OBJ_TYPE_BASE + (u32)(HEX_ST->typeBank[type] - HEX_OBJ_BANK_FIRST) * HEX_OBJ_TYPE_TILES;
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;

	HexScreen_Push(x, y, gObject_32x32, base, frameBank);
	HexScreen_Push(x, y, gObject_32x32, base + HEX_OBJ_PORTRAIT_OFFSET, HEX_ST->typeBank[type]);
}

/* The hexagon board: an empty cell is the outline, a placed card its digits, portrait and frame. */
static void HexScreen_Cells(void)
{
	const struct HexGame *g = &HEX_ST->game;
	int c;

	for (c = 0; c < HEX_CELLS; c++) {
		int x = sHexBoxX[c];
		int y = sHexBoxY[c];
		u8 owner = g->cell[c].owner;
		u32 frameBank;

		if (owner == HEX_EMPTY) {
			HexScreen_Push(x, y, gObject_32x32, HEX_OBJ_OUTLINE, HEX_BANK_RED);
			continue;
		}
		frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;
		HexScreen_CardBody(x, y, g->cell[c].type, owner);
		HexScreen_CardDigits(x, y, g->cell[c].type, frameBank, TRUE, g->cell[c].hp);
	}
}

/*
 * A hand strip (types = the game's hand of owner: HEX_USED slots are gone): the bodies, the lowest slot in front (each card
 * overlaps the one above it), then the digits of every slot on top of every frame.
 */
static void HexScreen_Hand(int x, const u8 *types, int n, u8 owner)
{
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;
	int i;

	for (i = 0; i < n; i++) {
		if (types[i] != HEX_USED)
			HexScreen_CardBody(x, HEX_HAND_Y + HEX_HAND_STEP * i, types[i], owner);
	}
	for (i = 0; i < n; i++) {
		if (types[i] != HEX_USED)
			HexScreen_CardDigits(x, HEX_HAND_Y + HEX_HAND_STEP * i, types[i], frameBank, FALSE, 0);
	}
}

/* A player's score: two 16x16 digits (tens, then ones; no tens digit under 10). */
static void HexScreen_Score(const struct HexGame *g, u8 owner, int x)
{
	int score = (int)Hex_Score(g, owner);
	int tens = score / 10;
	u32 bank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;

	if (tens > 0)
		HexScreen_Push(x, HEX_SCORE_Y, gObject_16x16, HEX_OBJ_SCORE + 4 * (u32)tens, bank);
	HexScreen_Push(x + 16, HEX_SCORE_Y, gObject_16x16, HEX_OBJ_SCORE + 4 * (u32)(score - tens * 10), bank);
}

/*
 * The down arrow above a cell: 2 px above its box, or, for the top row (no room above it on screen), its top at y 0.
 */
static int HexScreen_CellCursorY(int cell)
{
	int y = sHexBoxY[cell] - 16 - HEX_CURSOR_GAP;

	return y < 0 ? 0 : y;
}

static void HexScreen_Draw(struct ProcHexScreen *proc)
{
	const struct HexScreenState *h = HEX_ST;
	bool32 picking = h->state == HEX_STATE_PICK_TILE || h->state == HEX_STATE_PICK_CELL;

	/* The fade and the prep menu's own code turn BG2 and BG3 back on: keep the screen's layers (BG0 text, BG1 wood, OBJ). */
	SetDispEnable(1, 1, 0, 0, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	ClearSprites();

	HexScreen_Score(&h->game, HEX_BLUE, HEX_SCORE_BLUE_X);
	HexScreen_Score(&h->game, HEX_RED, HEX_SCORE_RED_X);
	HexScreen_Cells();
	HexScreen_Hand(HEX_HAND_BLUE_X, h->game.hand[HEX_BLUE], HEX_HAND, HEX_BLUE);
	HexScreen_Hand(HEX_HAND_RED_X, h->game.hand[HEX_RED], HEX_HAND, HEX_RED);

	/*
	 * The cursors last, in front of the board: a left arrow just right of the blue card (kept while a cell is picked),
	 * a down arrow above the cell.
	 */
	if (picking)
		HexScreen_Push(HEX_HAND_BLUE_X + HEX_BOX + HEX_CURSOR_GAP, HEX_HAND_Y + HEX_HAND_STEP * proc->sel + 4, gObject_16x16,
			       HEX_OBJ_CURSOR + 0, HEX_BANK_RED);
	if (h->state == HEX_STATE_PICK_CELL)
		HexScreen_Push(sHexBoxX[h->cell] + 8, HexScreen_CellCursorY(h->cell), gObject_16x16,
			       HEX_OBJ_CURSOR + HEX_OBJ_CURSOR_DOWN, HEX_BANK_RED);
}

/* The name bar: text centred on row 18 (BG0 text over the BG1 bar). */
static void HexScreen_BarText(struct ProcHexScreen *proc, const char *text)
{
	int tiles;
	int col;
	int x;

	for (x = 0; x < HEX_NAME_COLS; x++) {
		gBG0TilemapBuffer[TILEMAP_INDEX(x, HEX_NAME_ROW)] = 0;
		gBG0TilemapBuffer[TILEMAP_INDEX(x, HEX_NAME_ROW + 1)] = 0;
	}
	ClearText(&proc->nameText);

	tiles = (GetStringTextLen(text) + 7) / 8;
	col = (HEX_NAME_COLS - tiles) / 2;

	Text_SetParams(&proc->nameText, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&proc->nameText, text);
	PutText(&proc->nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, col, HEX_NAME_ROW));

	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* Draws a name bar code (HEX_BAR_*, or a card type: the card's name) if it is not the one shown. */
static void HexScreen_SetBar(struct ProcHexScreen *proc, u8 code)
{
	const struct CharacterData *chara;

	if (HEX_ST->bar == code)
		return;
	HEX_ST->bar = code;

	switch (code) {
	case HEX_BAR_BLUE_BEGINS:
		HexScreen_BarText(proc, "Blue begins");
		break;
	case HEX_BAR_RED_BEGINS:
		HexScreen_BarText(proc, "Red begins");
		break;
	case HEX_BAR_RED_THINKING:
		HexScreen_BarText(proc, "Red is thinking");
		break;
	case HEX_BAR_VICTORY:
		HexScreen_BarText(proc, "Victory!");
		break;
	case HEX_BAR_DEFEAT:
		HexScreen_BarText(proc, "Defeat...");
		break;
	case HEX_BAR_DRAW:
		HexScreen_BarText(proc, "Draw");
		break;
	default:
		chara = GetCharacterData(gpHexCardTypes[code].pid);
		HexScreen_BarText(proc, GetStringFromIndex(chara->nameTextId));
		break;
	}
}

/* The bar code of the current state (what the bar should say now): the highlighted blue card, the card just placed, or a banner. */
static u8 HexScreen_BarFor(const struct ProcHexScreen *proc)
{
	const struct HexScreenState *h = HEX_ST;
	u8 w;

	switch (h->state) {
	case HEX_STATE_BEGIN:
		return h->game.turn == HEX_BLUE ? HEX_BAR_BLUE_BEGINS : HEX_BAR_RED_BEGINS;
	case HEX_STATE_PICK_TILE:
	case HEX_STATE_PICK_CELL:
		return h->blueHand[proc->sel];
	case HEX_STATE_RESOLVE:
		return h->placedType;
	case HEX_STATE_AI_TURN:
		return HEX_BAR_RED_THINKING;
	default:
		w = Hex_Winner(&h->game);
		if (w == 0)
			return HEX_BAR_DRAW;
		return w == HEX_BLUE ? HEX_BAR_VICTORY : HEX_BAR_DEFEAT;
	}
}

static void HexScreen_Init(struct ProcHexScreen *proc)
{
	/* First open with no cards: hand out the starter set. */
	if (HexCol_Total() == 0)
		HexCol_GiveStarters();

	proc->dispBak = gLCDControlBuffer.dispcnt;
	proc->bg1Bak = *BG_GetControlBuffer(1);

	SetupBackgrounds(NULL);
	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);

	/* BG1 (wood and name bar) behind BG0 (text); OBJ 1D mapping, BG0, BG1 and OBJ on. */
	BG_GetControlBuffer(1)->charBaseBlock = HEX_BG_CHARBLOCK;
	BG_GetControlBuffer(1)->screenBaseBlock = HEX_BG_SCREENBLOCK;
	BG_GetControlBuffer(1)->priority = HEX_BG_PRIORITY;
	BG_GetControlBuffer(1)->colorMode = 0;
	SetDispEnable(1, 1, 0, 0, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	HexScreen_LoadBackground();
	HexScreen_LoadSprites();

	HexScreen_MakeBlueHand();
	HexScreen_MakeTypes();
	HexScreen_NewMatch(proc);
	HexScreen_LoadArt();

	InitText(&proc->nameText, 16);
	BG_EnableSyncByMask(BG1_SYNC_BIT);
	SetDefaultColorEffects();
}

/* ---- The match (docs: README.md "Match") ---- */

/* Starts the player's turn (PICK_TILE): the first unused slot, the first empty cell. */
static void HexScreen_BeginPlayerTurn(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;

	h->state = HEX_STATE_PICK_TILE;
	h->timer = 0;
	proc->sel = HexScreen_FirstUnused(&h->game, HEX_BLUE);
	h->cell = HexScreen_FirstEmpty(&h->game);
}

/* Up / Down in PICK_TILE: the next unused slot in that direction (no wrap). Returns TRUE if the cursor moved. */
static bool32 HexScreen_StepSlot(struct ProcHexScreen *proc, bool32 down)
{
	const struct HexGame *g = &HEX_ST->game;
	int s;

	for (s = proc->sel + (down ? 1 : -1); s >= 0 && s < HEX_HAND; s += (down ? 1 : -1)) {
		if (g->hand[HEX_BLUE][s] != HEX_USED) {
			proc->sel = (u8)s;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * The D-pad in PICK_CELL. UP / DOWN: the neighbour above (UL, UR) / below (DL, DR), an empty one if there is one; LEFT /
 * RIGHT: the neighbour L / R. Without a neighbour in that direction, the cell before / after in reading order (no wrap).
 * Returns TRUE if the cursor moved.
 */
static bool32 HexScreen_MoveCell(u16 keys)
{
	struct HexScreenState *h = HEX_ST;
	const struct HexGame *g = &h->game;
	u8 dirs[2] = { 0, 0 };
	int n;
	int i;
	int target = -1;
	s8 nb;

	if (keys & DPAD_UP) {
		dirs[0] = 0;
		dirs[1] = 1;
		n = 2;
	} else if (keys & DPAD_DOWN) {
		dirs[0] = 4;
		dirs[1] = 3;
		n = 2;
	} else if (keys & DPAD_LEFT) {
		dirs[0] = 5;
		n = 1;
	} else if (keys & DPAD_RIGHT) {
		dirs[0] = 2;
		n = 1;
	} else {
		return FALSE;
	}

	for (i = 0; i < n && target < 0; i++) {
		nb = Hex_Neighbor(h->cell, dirs[i]);
		if (nb >= 0 && g->cell[nb].owner == HEX_EMPTY)
			target = nb;
	}
	for (i = 0; i < n && target < 0; i++) {
		nb = Hex_Neighbor(h->cell, dirs[i]);
		if (nb >= 0)
			target = nb;
	}
	if (target < 0) {
		if (keys & (DPAD_UP | DPAD_LEFT)) {
			if (h->cell > 0)
				target = h->cell - 1;
		} else if (h->cell + 1 < HEX_CELLS) {
			target = h->cell + 1;
		}
	}
	if (target < 0)
		return FALSE;
	h->cell = (u8)target;
	return TRUE;
}

/* Plays hand slot hand of the player whose turn it is on cell, then RESOLVE. */
static void HexScreen_Place(u8 hand, u8 cell)
{
	struct HexScreenState *h = HEX_ST;

	h->placedType = h->game.hand[h->game.turn][hand];
	Hex_Place(&h->game, h->types, hand, cell);
	PlaySoundEffect(HEX_SE_SELECT);
	h->state = HEX_STATE_RESOLVE;
	h->timer = 0;
}

/* After RESOLVE: END when the board is full, red's AI_TURN, else the next blue pick. */
static void HexScreen_NextTurn(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;

	if (Hex_IsOver(&h->game)) {
		h->state = HEX_STATE_END;
		h->timer = 0;
		return;
	}
	if (h->game.turn == HEX_RED) {
		h->state = HEX_STATE_AI_TURN;
		h->timer = 0;
		return;
	}
	HexScreen_BeginPlayerTurn(proc);
}

static void HexScreen_UpdateBegin(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;

	h->timer++;
	if (h->timer < HEX_BEGIN_FRAMES && (gKeyStatusPtr->newKeys & A_BUTTON) == 0)
		return;
	if (h->game.turn == HEX_BLUE) {
		HexScreen_BeginPlayerTurn(proc);
	} else {
		h->state = HEX_STATE_AI_TURN;
		h->timer = 0;
	}
}

/* Returns TRUE to leave the screen (B: forfeit, no penalty yet). */
static bool32 HexScreen_UpdatePickTile(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	u16 keys = gKeyStatusPtr->newKeys;

	if (keys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		return TRUE;
	}
	if (keys & (DPAD_UP | DPAD_DOWN)) {
		if (HexScreen_StepSlot(proc, (keys & DPAD_DOWN) != 0))
			PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if (keys & A_BUTTON) {
		h->state = HEX_STATE_PICK_CELL;
		h->cell = HexScreen_FirstEmpty(&h->game);
		PlaySoundEffect(HEX_SE_SELECT);
	}
	return FALSE;
}

static void HexScreen_UpdatePickCell(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	u16 keys = gKeyStatusPtr->newKeys;

	if (keys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		h->state = HEX_STATE_PICK_TILE;
		h->timer = 0;
		return;
	}
	if (HexScreen_MoveCell(keys))
		PlaySoundEffect((keys & (DPAD_UP | DPAD_DOWN)) ? HEX_SE_CURSOR_UD : HEX_SE_CURSOR_LR);
	if (keys & A_BUTTON) {
		if (Hex_IsLegal(&h->game, proc->sel, h->cell))
			HexScreen_Place(proc->sel, h->cell);
		else
			PlaySoundEffect(HEX_SE_ERROR);
	}
}

static void HexScreen_UpdateResolve(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;

	h->timer++;
	if (h->timer >= HEX_RESOLVE_FRAMES)
		HexScreen_NextTurn(proc);
}

/* AI_TURN: the first frame only shows "Red is thinking"; the next one chooses and places. */
static void HexScreen_UpdateAi(void)
{
	struct HexScreenState *h = HEX_ST;
	u32 seed;

	if (h->timer == 0) {
		h->timer = 1;
		return;
	}
	seed = GetGameClock() * 0x9E37 + h->game.placed;
	Hex_AiChoose(&h->game, h->types, HEX_AI_LEVEL, seed, &h->aiHand, &h->aiCell);
	if (h->aiHand >= HEX_HAND || h->aiCell >= HEX_CELLS) {
		h->state = HEX_STATE_END;
		h->timer = 0;
		return;
	}
	HexScreen_Place(h->aiHand, h->aiCell);
}

/* END: A leaves the screen. */
static bool32 HexScreen_UpdateEnd(void)
{
	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		PlaySoundEffect(HEX_SE_SELECT);
		return TRUE;
	}
	return FALSE;
}

static void HexScreen_Loop(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	bool32 leave = FALSE;

	switch (h->state) {
	case HEX_STATE_BEGIN:
		HexScreen_UpdateBegin(proc);
		break;
	case HEX_STATE_PICK_TILE:
		leave = HexScreen_UpdatePickTile(proc);
		break;
	case HEX_STATE_PICK_CELL:
		HexScreen_UpdatePickCell(proc);
		break;
	case HEX_STATE_RESOLVE:
		HexScreen_UpdateResolve(proc);
		break;
	case HEX_STATE_AI_TURN:
		HexScreen_UpdateAi();
		break;
	default:
		leave = HexScreen_UpdateEnd();
		break;
	}

	if (leave) {
		Proc_Goto(proc, PL_HEX_EXIT);
		return;
	}
	HexScreen_SetBar(proc, HexScreen_BarFor(proc));
	HexScreen_Draw(proc);
}

static void HexScreen_OnEnd(struct ProcHexScreen *proc)
{
	struct ProcAtMenu *pproc = proc->proc_parent;

	ClearSprites();
	gLCDControlBuffer.dispcnt = proc->dispBak;
	*BG_GetControlBuffer(1) = proc->bg1Bak;

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	SetDefaultColorEffects();

	/* Same hand-off as the other prep sub-screens: the prep menu redraws itself. cur_cmd puts the cursor back on the Hex row. */
	pproc->cur_cmd = PREP_MAINMENU_HEX;
	pproc->state = 1;
}

const struct ProcCmd ProcScr_HexScreen[] = {
	PROC_NAME("HexScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(HexScreen_OnEnd),

	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(HexScreen_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_REPEAT(HexScreen_Loop),

	PROC_LABEL(PL_HEX_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_HEX_CARDS */
