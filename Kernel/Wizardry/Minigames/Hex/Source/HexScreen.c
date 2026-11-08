#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "prepscreen.h"
#include "hex.h"
#include "kernel/hex-core.h"

#ifdef CONFIG_HEX_CARDS

/*
 * The Hex screen (Preparations > Hex): the front menu (MENU: Play or Cards), the opponent (OPPONENT: one of the player's living
 * units, its deck in the red column), the player picks a hand of 7 cards from the collection (HAND_PICK, PICK_DONE), the
 * opponent's hand is revealed (REVEAL), one match is played (BEGIN, PICK_TILE, PICK_CELL, RESOLVE, AI_TURN, END), then the
 * stakes (TAKE, REWARD), and the screen returns to MENU. CARDS is the collection viewer.
 * The blue hand (right) and the red hand
 * (left); the board, both scores and the name bar. README.md lists the keys and the rules.
 *
 * Layers: BG3 the rune mural (RestartMuralBackground, priority 3, the same as the other prep sub-screens), BG1 the UI windows
 * (DrawUiFrame2, priority 2: the list over the board, the name bar across the bottom), BG0 text (priority 0: the list, the name
 * bar). OBJ 1D mapping. OBJ palette banks 0 (red frame) and 1 (blue frame), 2..15 the
 * portraits of the cards on screen (HexScreen_BankFor). Every frame draws the sprites again (ClearSprites, then PutSprite): a
 * later PutSprite is drawn in front of an earlier one, so each sprite is pushed back to front. Board layout and card art:
 * docs/custom.md "Hex" (the Minish Cap mod).
 */

#define PL_HEX_EXIT 2

#define HEX_SPRITE_LAYER 1
#define HEX_OBJ_VRAM 0x06010000
#define HEX_OBJ_TILE(t) ((void *)(HEX_OBJ_VRAM + (t) * 0x20))

/* OBJ tiles (1D mapping, 32 bytes each, tile numbers from HEX_OBJ_VRAM). */
#define HEX_OBJ_OUTLINE 0          /* 16 tiles: gHexEmptyTiles */
#define HEX_OBJ_DIGIT_WHITE 16     /* 11 tiles: gHexDigitTiles, digit d = 16 + d, blank = 26 */
#define HEX_OBJ_DIGIT_GREEN 27     /* 11 tiles: gHexDigitGreenTiles, digit d = 27 + d */
#define HEX_OBJ_CURSOR 38          /* 8 tiles: gHexCursorTiles, left arrow 38, down arrow 42 (4 tiles each, 16x16; the right arrow is the left one flipped) */
#define HEX_OBJ_CURSOR_DOWN 4      /* offset of the down arrow from HEX_OBJ_CURSOR */
#define HEX_OBJ_SCORE 512          /* 40 tiles: gHexScoreTiles, digit d = 512 + 4 d (16x16) */
#define HEX_OBJ_CARD_BACK 552      /* 16 tiles: gHexCardBackTiles, the owner fill behind every portrait (512..551 is taken) */
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
#define HEX_BOARD_TOP 0           /* the top row of cells; the board spans y 0..127, above the name bar (y 128) */

/* Hands and scores (blue on the right, red on the left). */
#define HEX_HAND_BLUE_X 204
#define HEX_HAND_RED_X 4
#define HEX_HAND_Y 4
#define HEX_HAND_STEP 20
#define HEX_SCORE_Y 112
#define HEX_SCORE_BLUE_X 168
#define HEX_SCORE_RED_X 44
#define HEX_CURSOR_GAP 2
#define HEX_ATTACK_DELAY 30        /* RESOLVE: the frames that show the board from before the placement; then the attack and the shake */
#define HEX_SHAKE_FRAMES 16        /* RESOLVE: the frames from HEX_ATTACK_DELAY in which the changed cells shake (sHexShake) */
#define HEX_RESOLVE_CALM_FRAMES 20 /* RESOLVE: the frames after the shake, the settled board */

/* The hand picker's list (BG0 text rows 1..13 in the list window rows 0..15, the window is BG1): the title on row 1, then the owned card
 * types, two rows each (16 px), from HEX_PICK_ROW0 (six rows). Names at column HEX_PICK_NAME_COL, the copies still available right-aligned to column
 * HEX_PICK_COUNT_RIGHT (exclusive). The cursor (right arrow, pointing at the names) is at HEX_PICK_CURSOR_X, the preview card at HEX_PREVIEW_X/Y.
 */
#define HEX_PICK_VISIBLE 6
#define HEX_PICK_TITLE_ROW 1
#define HEX_PICK_ROW0 3
#define HEX_PICK_TOP (HEX_PICK_ROW0 * 8)
#define HEX_PICK_NAME_COL 7
#define HEX_PICK_NAME_TILES 10
#define HEX_PICK_COUNT_TILES 4
#define HEX_PICK_TITLE_TILES 16
#define HEX_PICK_COUNT_RIGHT 19
#define HEX_PICK_CURSOR_X 40
#define HEX_PICK_MAX_COUNT 99
#define HEX_SCREEN_COLS 30
#define HEX_PREVIEW_X 160
#define HEX_PREVIEW_Y 40

/*
 * The windows (DrawUiFrame2, style 0, tile units, the frame included): the list over the board area (MENU, OPPONENT, CARDS,
 * HAND_PICK, PICK_DONE; rows 0..15), and the name bar across the bottom (rows 16..19, text on row 17). The hands sit at the
 * sides (x 4..36 and 204..236), so the bar is narrower than the screen: its columns are 5..24.
 */
#define HEX_LIST_X 5
#define HEX_LIST_Y 0
#define HEX_LIST_W 20
#define HEX_LIST_H 16
#define HEX_BAR_X 5
#define HEX_BAR_Y 16
#define HEX_BAR_W 20
#define HEX_BAR_H 4
#define HEX_NAME_ROW 17            /* the name bar's text row (the bar is rows 16..19, the glyphs inside the frame) */
#define HEX_NAME_TEXT_TILES 24

/* Match states (HexScreenState.state). */
#define HEX_STATE_BEGIN 0      /* "Blue begins" / "Red begins" (60 frames, or A) */
#define HEX_STATE_PICK_TILE 1  /* blue chooses a hand card: UP/DOWN, A = next, B does nothing (a match is finished) */
#define HEX_STATE_PICK_CELL 2  /* blue chooses a cell: D-pad, A = place, B = back to PICK_TILE */
#define HEX_STATE_RESOLVE 3    /* the placement (HEX_RESOLVE_FRAMES): the board before it, then the attack, the shake, the settled board */
#define HEX_STATE_AI_TURN 4    /* red: "Red is thinking" for one frame, then the AI places */
#define HEX_STATE_END 5        /* the result banner; A = the stakes (TAKE or REWARD) */
#define HEX_STATE_HAND_PICK 6  /* the list of owned cards: UP/DOWN, A adds a copy, B removes the last one (B with none goes to OPPONENT) */
#define HEX_STATE_PICK_DONE 7  /* seven cards: Accept (REVEAL) or Change (HAND_PICK, the last card removed) */
#define HEX_STATE_REVEAL 8     /* both hands face up: "Your opponent's hand" for 60 frames (A skips), then BEGIN */
#define HEX_STATE_TAKE 9       /* a win: UP/DOWN the opponent's starting cards, A takes one into the binder */
#define HEX_STATE_REWARD 10    /* the stake banner: 120 frames, then MENU (or A) */
#define HEX_STATE_MENU 11      /* the front menu: Play (OPPONENT) or Cards (CARDS); B leaves for the prep menu */
#define HEX_STATE_OPPONENT 12  /* the player's living units: UP/DOWN, the red column shows the highlighted deck; A = HAND_PICK, B = MENU */
#define HEX_STATE_CARDS 13     /* the collection: UP/DOWN, the preview of an owned card; B = MENU */

#define HEX_BEGIN_FRAMES 60
#define HEX_RESOLVE_FRAMES (HEX_ATTACK_DELAY + HEX_SHAKE_FRAMES + HEX_RESOLVE_CALM_FRAMES)
#define HEX_REVEAL_FRAMES 60
#define HEX_NOTICE_FRAMES 60
#define HEX_REWARD_FRAMES 120

#define HEX_RESULT_DRAW 0
#define HEX_RESULT_WIN 1
#define HEX_RESULT_LOSS 2
#define HEX_NO_TYPE 0xFF

/* Name bar codes: 0..HEX_CARD_TYPES-1 is the name of that card type; the rest are banners. */
#define HEX_BAR_NONE 0xFF
#define HEX_BAR_BLUE_BEGINS 0xE0
#define HEX_BAR_RED_BEGINS 0xE1
#define HEX_BAR_RED_THINKING 0xE2
#define HEX_BAR_VICTORY 0xE3
#define HEX_BAR_DEFEAT 0xE4
#define HEX_BAR_DRAW 0xE5
#define HEX_BAR_OPPONENT_HAND 0xE6
#define HEX_BAR_ACCEPT 0xE7
#define HEX_BAR_CANT_HOLD 0xE8
#define HEX_BAR_GAINED 0xEA    /* "Gained <card>!" */
#define HEX_BAR_LOST 0xEB      /* "Lost <card>..." */
#define HEX_BAR_SAFE 0xEC
#define HEX_BAR_NO_GAIN 0xED
#define HEX_BAR_DRAW_NO_CHANGE 0xEE
#define HEX_BAR_TIER1 0xF0     /* CARDS: "Tier 1" .. "Tier 3" */
#define HEX_BAR_TIER2 0xF1
#define HEX_BAR_TIER3 0xF2
#define HEX_BAR_DECK 0xF3      /* OPPONENT: "<Name>'s deck" */
#define HEX_BAR_STALE 0xFE     /* not a bar: the next SetBar draws the bar it is given */

/* Sounds (FE8 SE ids, the same ones the prep menu uses: AtMenu.c). */
#define HEX_SE_SELECT SONG_SE_SYS_WINDOW_SELECT1
#define HEX_SE_CANCEL SONG_SE_SYS_WINDOW_CANSEL1
#define HEX_SE_CURSOR_UD SONG_SE_SYS_CURSOR_UD1
#define HEX_SE_CURSOR_LR SONG_SE_SYS_CURSOR_LR1
#define HEX_SE_ERROR SONG_6C
#define HEX_SE_HIT 0xD4            /* the battle hit sound (EfxPlayHittedSFX's first id, base ROM 0x08072450) */

static const s16 sHexBoxX[HEX_CELLS] = { 90, 118, 76, 104, 132, 62, 90, 118, 146, 76, 104, 132, 90, 118 };
static const s16 sHexBoxY[HEX_CELLS] = { 0, 0, 24, 24, 24, 48, 48, 48, 48, 72, 72, 72, 96, 96 }; /* + HEX_BOARD_TOP */

/* RESOLVE: the x offset of a card that was hit, flipped or counterattacked, by frame (HEX_SHAKE_FRAMES frames). */
static const s8 sHexShake[HEX_SHAKE_FRAMES] = { 2, -2, 2, -2, 1, -1, 1, -1, 1, -1, 0, 0, 1, -1, 0, 0 };

/* The arrow of the list and of the hand: gObject_16x16 with the OAM horizontal flip (a right-pointing arrow). */
static const u16 sHexObject16x16HFlip[] = { 1, OAM0_SHAPE_16x16, OAM1_SIZE_16x16 + OAM1_HFLIP, 0 };

/*
 * The state lives in gHexScreenState (include/link/config-memmap.s, 0x280 bytes), not in the proc: a proc is only 0x6C bytes
 * (struct Proc in FE-CLib proc.h), so the proc keeps the name bar text and the saved display registers.
 */
#define HEX_STATE_SIZE 0x280
#define HEX_ST ((struct HexScreenState *)gHexScreenState)

struct HexScreenState {
	struct HexGame game;
	u8 redHand[HEX_HAND];	/* the opponent's starting hand: the deck of the unit highlighted in OPPONENT (HexScreen_MakeDeck) */
	u8 aiLevel;		/* Hex_AiChoose level for red (HexScreen_SetOpponent) */
	u8 oppPid;		/* CHARACTER_* of the opponent (its unit's pid) */
	u16 oppNameId;		/* text id of the opponent's name (the name bar and the banners) */
	u8 npcDeck;		/* district mode: the gHexNpcDecks row of the NPC (HEX_NO_DECK: the prep menu, the opponent is a unit) */
	u8 menuSel;		/* MENU: 0 = Play, 1 = Cards */
	u8 oppCursor;		/* OPPONENT: the highlighted row, an index into the living units */
	u8 oppScroll;		/* OPPONENT: the first row on screen */
	u8 cardCursor;		/* CARDS: the highlighted card type */
	u8 cardScroll;		/* CARDS: the first row on screen */
	u8 pickHand[HEX_HAND];	/* the player's hand: the cards picked (HAND_PICK), then the blue hand of the match */
	u8 pickCount;
	u8 pickCursor;		/* HAND_PICK: the highlighted row, an index into owned[] */
	u8 pickScroll;		/* HAND_PICK: the first row on screen */
	u8 pickChoice;		/* PICK_DONE: 0 = Accept, 1 = Change */
	u8 ownedN;		/* the owned card types (copies > 0), counted when the screen goes to MENU (HexScreen_ToMenu) */
	u8 owned[HEX_CARD_TYPES];
	u8 held[HEX_CARD_TYPES];	/* copies owned per type, capped at HEX_PICK_MAX_COUNT */
	u8 typeBank[HEX_CARD_TYPES];
	struct HexTileType types[HEX_CARD_TYPES];
	u8 state;
	u8 cell;        /* PICK_CELL: the cell under the down arrow */
	u8 timer;       /* frames in the current state */
	u8 aiHand;      /* the AI's last choice (hand slot, cell) */
	u8 aiCell;
	u8 placedType;  /* card type of the last placement (its name shows during RESOLVE) */
	struct HexCell shownCell[HEX_CELLS];	/* RESOLVE, the first HEX_ATTACK_DELAY frames: the board before the placement, the placed card on it at full HP */
	u8 shownScore[3];	/* the same: each player's score before the placement (indexed by owner) */
	u8 bar;         /* the code drawn in the name bar (HEX_BAR_NONE: nothing yet) */
	u8 listKey;     /* 1 while the list and its plate are drawn (0: the list is drawn again) */
	u8 takeSel;     /* TAKE: the opponent's card under the cursor */
	u8 notice;      /* TAKE: frames left of the refusal message */
	u8 result;      /* HEX_RESULT_* */
	u8 gotType;     /* the card taken from the opponent (HEX_NO_TYPE: none) */
	u8 lostType;    /* the card the opponent took from the player (HEX_NO_TYPE: none) */
	struct Text rows[HEX_PICK_VISIBLE];
	struct Text counts[HEX_PICK_VISIBLE];
	struct Text title;
};

/* Compile-time check: a constant division by zero (an error) if the state outgrows gHexScreenState. */
enum { HEX_STATE_FITS = 1 / (sizeof(struct HexScreenState) <= HEX_STATE_SIZE) };

struct ProcHexScreen {
	PROC_HEADER;
	struct Text nameText;
	struct DispCnt dispBak;
	u8 sel;
};

/* ---- The windows ---- */

/* Rows y0..y0+rows-1 of BG0 and BG1 back to empty (the window's cells are drawn again by DrawUiFrame2). */
static void HexScreen_ClearRows(int y0, int rows)
{
	int x, y;

	for (y = y0; y < y0 + rows; y++) {
		for (x = 0; x < HEX_SCREEN_COLS; x++) {
			gBG0TilemapBuffer[TILEMAP_INDEX(x, y)] = 0;
			gBG1TilemapBuffer[TILEMAP_INDEX(x, y)] = 0;
		}
	}
}

/* The engine's tile table (attack, max HP, reach) from the card table: index = card type. */
static void HexScreen_MakeTypes(struct HexScreenState *h)
{
	int t;

	for (t = 0; t < HEX_CARD_TYPES; t++) {
		h->types[t].attack = gpHexCardTypes[t].attack;
		h->types[t].maxHp = gpHexCardTypes[t].maxHp;
		h->types[t].reach = gpHexCardTypes[t].reach;
	}
}

/* One banked card type: its frame (reach mask, owner-neutral) and its portrait (the chibi, clipped to the hexagon). */
static void HexScreen_LoadType(int type, int bank)
{
	const struct HexCardType *info = &gpHexCardTypes[type];
	u32 base = HEX_OBJ_TYPE_BASE + (u32)(bank - HEX_OBJ_BANK_FIRST) * HEX_OBJ_TYPE_TILES;
	const u8 *chibi;
	const u16 *pal;
	const u32 *src = (const u32 *)gGenericBuffer;
	u32 keep[128];
	int i;

	CpuFastCopy(gHexFrameTiles[info->reach & 0x3F], HEX_OBJ_TILE(base), 512);

	if (type >= HEX_MONSTER_FIRST) {
		chibi = gHexMonsterArt[type - HEX_MONSTER_FIRST].chibi;
		pal = gHexMonsterArt[type - HEX_MONSTER_FIRST].pal;
	} else {
		const struct FaceData *face = GetPortraitData(GetCharacterData(info->pid)->portraitId);

		chibi = face->imgChibi;
		pal = face->pal;
	}

	Decompress(chibi, gGenericBuffer);
	for (i = 0; i < 128; i++)
		keep[i] = src[i] & gHexPortraitKeep[i];
	CpuFastCopy(keep, HEX_OBJ_TILE(base + HEX_OBJ_PORTRAIT_OFFSET), 512);

	ApplyPalette(pal, 0x10 + bank);
}

/* TRUE if type t is in the player's hand (the pick, or the blue hand of the match) or in the opponent's starting hand. */
static bool32 HexScreen_Kept(const struct HexScreenState *h, int t)
{
	int i;

	for (i = 0; i < h->pickCount; i++) {
		if (h->pickHand[i] == t)
			return TRUE;
	}
	for (i = 0; i < HEX_HAND; i++) {
		if (h->redHand[i] == t)
			return TRUE;
	}
	return FALSE;
}

/*
 * The OBJ bank of type t (its art loaded if it was not). A free bank is used when there is one; otherwise a type that is not
 * kept (HexScreen_Kept) gives up its bank. Returns HEX_NO_BANK when no bank can be had (the caller leaves the card out).
 */
static u8 HexScreen_BankFor(struct HexScreenState *h, int t)
{
	int b;
	int i;
	bool32 used;
	int victim = -1;

	if (t < 0 || t >= HEX_CARD_TYPES)
		return HEX_NO_BANK;
	if (h->typeBank[t] != HEX_NO_BANK)
		return h->typeBank[t];

	for (b = HEX_OBJ_BANK_FIRST; b <= HEX_OBJ_BANK_LAST; b++) {
		used = FALSE;
		for (i = 0; i < HEX_CARD_TYPES; i++) {
			if (h->typeBank[i] == b) {
				used = TRUE;
				break;
			}
		}
		if (!used)
			break;
	}
	if (b > HEX_OBJ_BANK_LAST) {
		for (i = 0; i < HEX_CARD_TYPES && victim < 0; i++) {
			if (h->typeBank[i] != HEX_NO_BANK && !HexScreen_Kept(h, i))
				victim = i;
		}
		if (victim < 0)
			return HEX_NO_BANK;
		b = h->typeBank[victim];
		h->typeBank[victim] = HEX_NO_BANK;
	}
	h->typeBank[t] = (u8)b;
	HexScreen_LoadType(t, b);
	return (u8)b;
}

/* The opponent's deck is loaded when it is chosen (HexScreen_SetOpponent): it stays on screen through the picks and the match. */
static void HexScreen_LoadRedArt(struct HexScreenState *h)
{
	int i;

	for (i = 0; i < HEX_HAND; i++)
		HexScreen_BankFor(h, h->redHand[i]);
}

/* OBJ tiles and palettes that do not depend on the cards: the cell outline, digits, score digits, cursors, frames. */
static void HexScreen_LoadSprites(void)
{
	CpuFastCopy(gHexEmptyTiles, HEX_OBJ_TILE(HEX_OBJ_OUTLINE), 512);
	CpuFastCopy(gHexDigitTiles, HEX_OBJ_TILE(HEX_OBJ_DIGIT_WHITE), 352);
	CpuFastCopy(gHexDigitGreenTiles, HEX_OBJ_TILE(HEX_OBJ_DIGIT_GREEN), 352);
	CpuFastCopy(gHexCursorTiles, HEX_OBJ_TILE(HEX_OBJ_CURSOR), 256);
	CpuFastCopy(gHexScoreTiles, HEX_OBJ_TILE(HEX_OBJ_SCORE), 1280);
	CpuFastCopy(gHexCardBackTiles, HEX_OBJ_TILE(HEX_OBJ_CARD_BACK), 512);

	ApplyPalette(gHexFramePalRed, 0x10 + HEX_BANK_RED);
	ApplyPalette(gHexFramePalBlue, 0x10 + HEX_BANK_BLUE);
}

/* The owned card types (in type order) and their copies, read from the collection once. */
static void HexScreen_Scan(struct HexScreenState *h)
{
	int t;
	int n;

	h->ownedN = 0;
	for (t = 0; t < HEX_CARD_TYPES; t++) {
		n = HexCol_Count(t);
		h->held[t] = (u8)(n > HEX_PICK_MAX_COUNT ? HEX_PICK_MAX_COUNT : n);
		if (n > 0)
			h->owned[h->ownedN++] = (u8)t;
	}
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
 * The body of a card, back to front: the card back (the owner fill, in the frame bank), the portrait (its own bank), then the
 * frame ring and reach dots (the owner's frame bank). PutSprite draws a later call in front of an earlier one, so every
 * sprite is pushed back to front. Three 32x32 sprites per card.
 */
static void HexScreen_CardBody(const struct HexScreenState *h, int x, int y, int type, u32 owner)
{
	u32 base = HEX_OBJ_TYPE_BASE + (u32)(h->typeBank[type] - HEX_OBJ_BANK_FIRST) * HEX_OBJ_TYPE_TILES;
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;

	HexScreen_Push(x, y, gObject_32x32, HEX_OBJ_CARD_BACK, frameBank);
	HexScreen_Push(x, y, gObject_32x32, base + HEX_OBJ_PORTRAIT_OFFSET, h->typeBank[type]);
	HexScreen_Push(x, y, gObject_32x32, base, frameBank);
}

/*
 * RESOLVE shakes the cards that were hit (lastHit), that changed owner (lastFlip), and the placed card when it was
 * counterattacked (lastCounter): an x offset from sHexShake by the frame, for HEX_SHAKE_FRAMES frames. Zero otherwise.
 */
static int HexScreen_ShakeX(const struct HexScreenState *h, int c)
{
	const struct HexGame *g = &h->game;
	u16 bit = (u16)(1u << c);
	int k = (int)h->timer - HEX_ATTACK_DELAY;

	if (h->state != HEX_STATE_RESOLVE || k < 0 || k >= HEX_SHAKE_FRAMES)
		return 0;
	if ((g->lastHit & bit) != 0 || (g->lastFlip & bit) != 0 || (g->lastCounter != 0 && c == g->lastCell))
		return sHexShake[k];
	return 0;
}

/* RESOLVE's first HEX_ATTACK_DELAY frames show the board and the scores from before the placement (HexScreen_Place). */
static bool32 HexScreen_Before(const struct HexScreenState *h)
{
	return h->state == HEX_STATE_RESOLVE && h->timer < HEX_ATTACK_DELAY;
}

/* The hexagon board: an empty cell is the outline, a placed card its digits, portrait and frame. */
static void HexScreen_Cells(const struct HexScreenState *h)
{
	const struct HexCell *cells = HexScreen_Before(h) ? h->shownCell : h->game.cell;
	int c;

	for (c = 0; c < HEX_CELLS; c++) {
		int x = sHexBoxX[c];
		int y = HEX_BOARD_TOP + sHexBoxY[c];
		u8 owner = cells[c].owner;
		u32 frameBank;

		if (owner == HEX_EMPTY) {
			HexScreen_Push(x, y, gObject_32x32, HEX_OBJ_OUTLINE, HEX_BANK_RED);
			continue;
		}
		x += HexScreen_ShakeX(h, c);
		frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;
		HexScreen_CardBody(h, x, y, cells[c].type, owner);
		HexScreen_CardDigits(x, y, cells[c].type, frameBank, TRUE, cells[c].hp);
	}
}

/*
 * A hand strip (types: n slots, HEX_USED slots are gone): the bodies, the lowest slot in front (each card overlaps the one
 * above it), then the digits of every slot on top of every frame.
 */
static void HexScreen_Hand(const struct HexScreenState *h, int x, const u8 *types, int n, u8 owner)
{
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;
	int i;

	for (i = 0; i < n; i++) {
		if (types[i] != HEX_USED)
			HexScreen_CardBody(h, x, HEX_HAND_Y + HEX_HAND_STEP * i, types[i], owner);
	}
	for (i = 0; i < n; i++) {
		if (types[i] != HEX_USED)
			HexScreen_CardDigits(x, HEX_HAND_Y + HEX_HAND_STEP * i, types[i], frameBank, FALSE, 0);
	}
}

/* A player's score: two 16x16 digits (tens, then ones; no tens digit under 10). */
static void HexScreen_Score(int score, u8 owner, int x)
{
	int tens = 0;
	int ones = score;
	u32 bank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;

	while (ones >= 10) {
		ones -= 10;
		tens++;
	}
	if (tens > 0)
		HexScreen_Push(x, HEX_SCORE_Y, gObject_16x16, HEX_OBJ_SCORE + 4 * (u32)tens, bank);
	HexScreen_Push(x + 16, HEX_SCORE_Y, gObject_16x16, HEX_OBJ_SCORE + 4 * (u32)ones, bank);
}

/*
 * The down arrow at a cell: its top at the cell's top edge (the same place in every cell, as on the top row), x = the cell's
 * x + 8 (centred on the 32 px box). It is drawn over the cell.
 */
static int HexScreen_CellCursorY(int cell)
{
	return HEX_BOARD_TOP + sHexBoxY[cell];
}

/* The preview of the highlighted card at the right of the list (only if the card has art). */
static void HexScreen_Preview(const struct HexScreenState *h, int type)
{
	if (h->typeBank[type] == HEX_NO_BANK)
		return;
	HexScreen_CardBody(h, HEX_PREVIEW_X, HEX_PREVIEW_Y, type, HEX_BLUE);
	HexScreen_CardDigits(HEX_PREVIEW_X, HEX_PREVIEW_Y, type, HEX_BANK_BLUE, FALSE, 0);
}

/* HAND_PICK / PICK_DONE: no board. The blue column is the hand being picked, the red column the opponent's hand; the arrow
 * by the list row (or by Accept / Change) and the preview of the highlighted card.
 */
static void HexScreen_DrawPick(const struct HexScreenState *h)
{
	HexScreen_Hand(h, HEX_HAND_BLUE_X, h->pickHand, h->pickCount, HEX_BLUE);
	HexScreen_Hand(h, HEX_HAND_RED_X, h->redHand, HEX_HAND, HEX_RED);

	if (h->state == HEX_STATE_PICK_DONE) {
		HexScreen_Push(HEX_PICK_CURSOR_X, HEX_PICK_TOP + 16 * h->pickChoice, sHexObject16x16HFlip, HEX_OBJ_CURSOR + 0,
			       HEX_BANK_RED);
		return;
	}
	if (h->ownedN == 0)
		return;
	HexScreen_Preview(h, h->owned[h->pickCursor]);
	HexScreen_Push(HEX_PICK_CURSOR_X, HEX_PICK_TOP + 16 * (h->pickCursor - h->pickScroll), sHexObject16x16HFlip,
		       HEX_OBJ_CURSOR + 0, HEX_BANK_RED);
}

/*
 * Every frame: the sprites are cleared and drawn again. The picking states draw no board; the match and the stakes draw the
 * board, both scores and both hands (in TAKE the red column is the opponent's starting hand, the cursor on the card to take).
 */
static int HexScreen_LivingCount(void);

/*
 * MENU, OPPONENT and CARDS: no board and no blue hand. OPPONENT shows the highlighted unit's deck in the red column; CARDS
 * previews the highlighted card if it is owned. The arrow is by the highlighted row of the list.
 */
static void HexScreen_DrawSide(const struct HexScreenState *h)
{
	int row;

	switch (h->state) {
	case HEX_STATE_OPPONENT:
		if (HexScreen_LivingCount() == 0)
			return;
		HexScreen_Hand(h, HEX_HAND_RED_X, h->redHand, HEX_HAND, HEX_RED);
		row = h->oppCursor - h->oppScroll;
		break;
	case HEX_STATE_CARDS:
		if (h->held[h->cardCursor] > 0)
			HexScreen_Preview(h, h->cardCursor);
		row = h->cardCursor - h->cardScroll;
		break;
	default:
		row = h->menuSel;
		break;
	}
	HexScreen_Push(HEX_PICK_CURSOR_X, HEX_PICK_TOP + 16 * row, sHexObject16x16HFlip, HEX_OBJ_CURSOR + 0, HEX_BANK_RED);
}

static void HexScreen_Draw(struct ProcHexScreen *proc)
{
	const struct HexScreenState *h = HEX_ST;

	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	ClearSprites();

	if (h->state == HEX_STATE_MENU || h->state == HEX_STATE_OPPONENT || h->state == HEX_STATE_CARDS) {
		HexScreen_DrawSide(h);
		return;
	}
	if (h->state == HEX_STATE_HAND_PICK || h->state == HEX_STATE_PICK_DONE) {
		HexScreen_DrawPick(h);
		return;
	}

	if (HexScreen_Before(h)) {
		HexScreen_Score(h->shownScore[HEX_BLUE], HEX_BLUE, HEX_SCORE_BLUE_X);
		HexScreen_Score(h->shownScore[HEX_RED], HEX_RED, HEX_SCORE_RED_X);
	} else {
		HexScreen_Score((int)Hex_Score(&h->game, HEX_BLUE), HEX_BLUE, HEX_SCORE_BLUE_X);
		HexScreen_Score((int)Hex_Score(&h->game, HEX_RED), HEX_RED, HEX_SCORE_RED_X);
	}
	HexScreen_Cells(h);
	HexScreen_Hand(h, HEX_HAND_BLUE_X, h->game.hand[HEX_BLUE], HEX_HAND, HEX_BLUE);
	if (h->state == HEX_STATE_TAKE)
		HexScreen_Hand(h, HEX_HAND_RED_X, h->redHand, HEX_HAND, HEX_RED);
	else
		HexScreen_Hand(h, HEX_HAND_RED_X, h->game.hand[HEX_RED], HEX_HAND, HEX_RED);

	/*
	 * The cursors last, in front of the board: a right arrow just left of the blue card (kept while a cell is picked),
	 * a down arrow over the top edge of the cell; in TAKE a left arrow just right of the red card.
	 */
	if (h->state == HEX_STATE_PICK_TILE || h->state == HEX_STATE_PICK_CELL)
		HexScreen_Push(HEX_HAND_BLUE_X - 16 - HEX_CURSOR_GAP, HEX_HAND_Y + HEX_HAND_STEP * proc->sel + 4, sHexObject16x16HFlip,
			       HEX_OBJ_CURSOR + 0, HEX_BANK_RED);
	if (h->state == HEX_STATE_PICK_CELL)
		HexScreen_Push(sHexBoxX[h->cell] + 8, HexScreen_CellCursorY(h->cell), gObject_16x16,
			       HEX_OBJ_CURSOR + HEX_OBJ_CURSOR_DOWN, HEX_BANK_RED);
	if (h->state == HEX_STATE_TAKE)
		HexScreen_Push(HEX_HAND_RED_X + HEX_BOX + HEX_CURSOR_GAP, HEX_HAND_Y + HEX_HAND_STEP * h->takeSel + 4, gObject_16x16,
			       HEX_OBJ_CURSOR + 0, HEX_BANK_RED);
}

/* ---- The list (HAND_PICK, PICK_DONE) ---- */

/* The copies of type t still to be picked: held minus the copies in the hand being built. */
static int HexScreen_Avail(const struct HexScreenState *h, int t)
{
	int i;
	int picked = 0;
	int held = h->held[t];

	for (i = 0; i < h->pickCount; i++) {
		if (h->pickHand[i] == t)
			picked++;
	}
	return held > picked ? held - picked : 0;
}

/* Appends src to dst (at index at, at most size - 1 characters in all). Returns the new length. */
static int HexScreen_Append(char *dst, int at, const char *src, int size)
{
	while (*src != '\0' && at < size - 1)
		dst[at++] = *src++;
	dst[at] = '\0';
	return at;
}

/* Appends a number of 0..99 (in decimal, at most size - 1 characters in all). Returns the new length. */
static int HexScreen_AppendNum(char *dst, int at, int n, int size)
{
	int tens = 0;

	if (n < 0)
		n = 0;
	if (n > 99)
		n = 99;
	while (n >= 10) {
		n -= 10;
		tens++;
	}
	if (tens > 0 && at < size - 1)
		dst[at++] = (char)('0' + tens);
	if (at < size - 1)
		dst[at++] = (char)('0' + n);
	dst[at] = '\0';
	return at;
}

/* One list text: cleared, then str drawn at (col, row) on BG0. */
static void HexScreen_Put(struct Text *text, const char *str, int col, int row)
{
	ClearText(text);
	Text_SetParams(text, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(text, str);
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, col, row));
}

/* One list text right-aligned on the count column: str is cleared, drawn, then moved so its right edge is HEX_PICK_COUNT_RIGHT. */
static void HexScreen_PutRight(struct Text *text, const char *str, int row)
{
	int tiles = (GetStringTextLen(str) + 7) / 8;

	ClearText(text);
	Text_SetParams(text, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(text, str);
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, HEX_PICK_COUNT_RIGHT - tiles, row));
}

/* A copy count (a number, right-aligned on the count column). */
static void HexScreen_PutCount(struct Text *text, int n, int row)
{
	Text_SetParams(text, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawNumber(text, n);
	PutText(text, TILEMAP_LOCATED(gBG0TilemapBuffer, n >= 10 ? HEX_PICK_COUNT_RIGHT - 2 : HEX_PICK_COUNT_RIGHT - 1, row));
}

/* The name of card type t (the character it is named after). */
static const char *HexScreen_TypeName(int t)
{
	return GetStringFromIndex(HexCardType_NameId(t));
}

/* The k-th living unit of the player (gUnitArrayBlue, the same test as HexCards.c), NULL past the last one. */
static struct Unit *HexScreen_Living(int k)
{
	int i;

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;
		if (k == 0)
			return unit;
		k--;
	}
	return NULL;
}

static int HexScreen_LivingCount(void)
{
	int i;
	int n = 0;

	for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
		struct Unit *unit = &gUnitArrayBlue[i];

		if (UNIT_IS_VALID(unit) && !(unit->state & US_DEAD))
			n++;
	}
	return n;
}

/*
 * The deck of an opponent: pid is the unit's character, eff its effective level (promoted units count 20 more). The first card
 * is the card of the character (when there is one, even above the tier cap). The other slots are drawn with a u32 LCG seeded
 * from pid: among the types of tier <= maxTier with at most 2 copies in the deck. Monsters (HEX_MONSTER_FIRST and up) are never
 * drawn for a unit. Deterministic: the same unit, the same deck.
 */
static void HexScreen_MakeDeck(u8 *deck, u32 pid, int eff)
{
	int maxTier = eff < 10 ? 1 : (eff < 20 ? 2 : 3);
	u8 copies[HEX_CARD_TYPES];
	u8 elig[HEX_CARD_TYPES];
	u32 s = pid * 2654435761u + 12345u;
	u32 r;
	int filled = 0;
	int t;
	int n;
	int i;

	for (t = 0; t < HEX_CARD_TYPES; t++)
		copies[t] = 0;
	for (t = 0; t < HEX_MONSTER_FIRST; t++) {
		if (gpHexCardTypes[t].pid == pid) {
			deck[0] = (u8)t;
			copies[t] = 1;
			filled = 1;
			break;
		}
	}

	while (filled < HEX_HAND) {
		n = 0;
		for (t = 0; t < HEX_CARD_TYPES; t++) {
			if (t < HEX_MONSTER_FIRST && gpHexCardTypes[t].tier <= maxTier && copies[t] < 2)
				elig[n++] = (u8)t;
		}
		if (n == 0)
			break;	/* not reached: eleven tier 1 types with two copies each */
		s = s * 1103515245u + 12345u;
		r = s >> 16;
		/*
		 * r mod n by subtraction: GCC turns r - (r / n) * n into __aeabi_uidivmod, which is the game's __umodsi3 here and
		 * returns the remainder in the wrong register (TextEngineRework.c has the same note).
		 */
		while (r >= (u32)n)
			r -= (u32)n;
		i = (int)r;
		t = elig[i];
		copies[t]++;
		deck[filled++] = (u8)t;
	}
	while (filled < HEX_HAND)
		deck[filled++] = 0;
}

/* The opponent is the unit: its deck (redHand), its AI level (0..2) and its name, and the red art is loaded. */
static void HexScreen_SetOpponent(struct HexScreenState *h, const struct Unit *unit)
{
	int eff = unit->level + ((UNIT_CATTRIBUTES(unit) & CA_PROMOTED) ? 20 : 0);

	h->oppPid = unit->pCharacterData->number;
	h->oppNameId = unit->pCharacterData->nameTextId;
	h->aiLevel = eff < 10 ? 1 : 2;
	HexScreen_MakeDeck(h->redHand, h->oppPid, eff);
	HexScreen_LoadRedArt(h);
}

/* A district NPC is the opponent: its deck (HexNpc_CardType, so a rare card the NPC has run out of is its fallback), AI and name. */
static void HexScreen_SetNpcOpponent(struct HexScreenState *h, const struct HexNpcDeck *deck)
{
	int i;

	for (i = 0; i < HEX_HAND; i++)
		h->redHand[i] = (u8)HexNpc_CardType(&deck->cards[i]);
	h->aiLevel = deck->aiLevel;
	h->oppPid = deck->pid;
	h->oppNameId = deck->nameId;
	HexScreen_LoadRedArt(h);
}

/* Called before the Hex proc starts (HexPrep.c: HEX_NO_DECK; HexDistrict.c: the NPC's row). The proc's state is kept across it. */
void HexScreen_SetDistrictDeck(int deckIndex)
{
	HEX_ST->npcDeck = (u8)deckIndex;
}

/* The highlighted opponent (OPPONENT's cursor) is set as the opponent; nothing if there is no living unit. */
static void HexScreen_CursorOpponent(struct HexScreenState *h)
{
	struct Unit *unit = HexScreen_Living(h->oppCursor);

	if (unit != NULL)
		HexScreen_SetOpponent(h, unit);
}

/* Up / Down (held keys repeat) move a list cursor; the window follows it. count is the number of rows. Returns TRUE if it moved. */
static bool32 HexScreen_Step(u8 *cursor, u8 *scroll, int count, bool32 down)
{
	int cur = *cursor;

	if (down) {
		if (cur + 1 >= count)
			return FALSE;
		cur++;
	} else {
		if (cur == 0)
			return FALSE;
		cur--;
	}
	*cursor = (u8)cur;
	if (cur < *scroll)
		*scroll = (u8)cur;
	if (cur >= *scroll + HEX_PICK_VISIBLE)
		*scroll = (u8)(cur + 1 - HEX_PICK_VISIBLE);
	return TRUE;
}

/* The states with a list on the plate (the front menu, the opponents, the collection, the hand picker). */
static bool32 HexScreen_IsList(u8 state)
{
	return state == HEX_STATE_MENU || state == HEX_STATE_OPPONENT || state == HEX_STATE_CARDS || state == HEX_STATE_HAND_PICK ||
	       state == HEX_STATE_PICK_DONE;
}

/* The list window's rows back to empty. */
static void HexScreen_ClearList(void)
{
	HexScreen_ClearRows(HEX_LIST_Y, HEX_LIST_H);
}

/* MENU: the title and the two rows. */
static void HexScreen_DrawMenuList(struct HexScreenState *h)
{
	HexScreen_Put(&h->title, "Hex", HEX_PICK_NAME_COL, HEX_PICK_TITLE_ROW);
	HexScreen_Put(&h->rows[0], "Play", HEX_PICK_NAME_COL, HEX_PICK_ROW0);
	HexScreen_Put(&h->rows[1], "Cards", HEX_PICK_NAME_COL, HEX_PICK_ROW0 + 2);
}

/* OPPONENT: the living units from the scroll position, each with its name and "Lv N" (right-aligned). */
static void HexScreen_DrawOpponentList(struct HexScreenState *h)
{
	char text[16];
	int count = HexScreen_LivingCount();
	int i;
	int k;
	int n;
	int row;

	HexScreen_Put(&h->title, "Choose an opponent", HEX_PICK_NAME_COL, HEX_PICK_TITLE_ROW);
	for (i = 0; i < HEX_PICK_VISIBLE; i++) {
		struct Unit *unit;

		k = h->oppScroll + i;
		if (k >= count)
			break;
		unit = HexScreen_Living(k);
		row = HEX_PICK_ROW0 + 2 * i;
		HexScreen_Put(&h->rows[i], GetStringFromIndex(unit->pCharacterData->nameTextId), HEX_PICK_NAME_COL, row);

		n = HexScreen_Append(text, 0, "Lv ", sizeof(text));
		HexScreen_AppendNum(text, n, unit->level, sizeof(text));
		HexScreen_PutRight(&h->counts[i], text, row);
	}
}

/* CARDS: "Cards  N/32" and every card type in table order: the name and the copies owned, or "???" for a type never owned. */
static void HexScreen_DrawCardList(struct HexScreenState *h)
{
	char text[24];
	int i;
	int k;
	int n;
	int row;

	n = HexScreen_Append(text, 0, "Cards  ", sizeof(text));
	n = HexScreen_AppendNum(text, n, h->ownedN, sizeof(text));
	n = HexScreen_Append(text, n, "/", sizeof(text));
	HexScreen_AppendNum(text, n, HEX_CARD_TYPES, sizeof(text));
	HexScreen_Put(&h->title, text, HEX_PICK_NAME_COL, HEX_PICK_TITLE_ROW);

	for (i = 0; i < HEX_PICK_VISIBLE; i++) {
		k = h->cardScroll + i;
		if (k >= HEX_CARD_TYPES)
			break;
		row = HEX_PICK_ROW0 + 2 * i;
		if (h->held[k] == 0) {
			HexScreen_Put(&h->rows[i], "???", HEX_PICK_NAME_COL, row);
			continue;
		}
		HexScreen_Put(&h->rows[i], HexScreen_TypeName(k), HEX_PICK_NAME_COL, row);
		HexScreen_PutCount(&h->counts[i], h->held[k], row);
	}
}

/*
 * HAND_PICK and PICK_DONE: the title, then the owned types from the scroll position, each with its name and its copies still
 * available (right-aligned). PICK_DONE shows Accept and Change on the first two rows.
 */
static void HexScreen_DrawPickList(struct HexScreenState *h)
{
	int i;
	int k;
	int t;
	int n;
	int row;

	if (h->state == HEX_STATE_PICK_DONE) {
		HexScreen_Put(&h->title, "Your hand", HEX_PICK_NAME_COL, HEX_PICK_TITLE_ROW);
		HexScreen_Put(&h->rows[0], "Accept", HEX_PICK_NAME_COL, HEX_PICK_ROW0);
		HexScreen_Put(&h->rows[1], "Change", HEX_PICK_NAME_COL, HEX_PICK_ROW0 + 2);
		return;
	}

	HexScreen_Put(&h->title, "Your cards", HEX_PICK_NAME_COL, HEX_PICK_TITLE_ROW);
	for (i = 0; i < HEX_PICK_VISIBLE; i++) {
		k = h->pickScroll + i;
		if (k >= h->ownedN)
			break;
		t = h->owned[k];
		row = HEX_PICK_ROW0 + 2 * i;
		HexScreen_Put(&h->rows[i], HexScreen_TypeName(t), HEX_PICK_NAME_COL, row);

		n = HexScreen_Avail(h, t);
		HexScreen_PutCount(&h->counts[i], n, row);
	}
}

/* The list texts of the current state (the window is drawn by HexScreen_ListUpdate). */
static void HexScreen_DrawList(struct HexScreenState *h)
{
	int i;

	ClearText(&h->title);
	for (i = 0; i < HEX_PICK_VISIBLE; i++) {
		ClearText(&h->rows[i]);
		ClearText(&h->counts[i]);
	}

	switch (h->state) {
	case HEX_STATE_MENU:
		HexScreen_DrawMenuList(h);
		break;
	case HEX_STATE_OPPONENT:
		HexScreen_DrawOpponentList(h);
		break;
	case HEX_STATE_CARDS:
		HexScreen_DrawCardList(h);
		break;
	default:
		HexScreen_DrawPickList(h);
		break;
	}
}

/* The list window and its texts are drawn while a list state lasts (again on every change of list state), and cleared once they are over. */
static void HexScreen_ListUpdate(struct HexScreenState *h)
{
	if (HexScreen_IsList(h->state)) {
		if (h->listKey == 0) {
			HexScreen_ClearList();
			DrawUiFrame2(HEX_LIST_X, HEX_LIST_Y, HEX_LIST_W, HEX_LIST_H, 0);
			HexScreen_DrawList(h);
			h->listKey = 1;
			BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
		}
	} else if (h->listKey != 0) {
		HexScreen_ClearList();
		h->listKey = 0;
		BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	}
}

/* ---- The name bar ---- */

/* A name bar piece: the text centred on row 17 (BG0 text over the BG1 frame). */
static void HexScreen_BarText(struct ProcHexScreen *proc, const char *text)
{
	int tiles;
	int col;

	HexScreen_ClearRows(HEX_BAR_Y, HEX_BAR_H);
	DrawUiFrame2(HEX_BAR_X, HEX_BAR_Y, HEX_BAR_W, HEX_BAR_H, 0);
	ClearText(&proc->nameText);

	tiles = (GetStringTextLen(text) + 7) / 8;
	col = HEX_BAR_X + (HEX_BAR_W - tiles) / 2;

	Text_SetParams(&proc->nameText, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&proc->nameText, text);
	PutText(&proc->nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, col, HEX_NAME_ROW));

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

/* The name bar with a name between head and tail ("" + name + "'s deck"). */
static void HexScreen_BarName(struct ProcHexScreen *proc, const char *head, const char *name, const char *tail)
{
	char text[48];
	int n;

	n = HexScreen_Append(text, 0, head, sizeof(text));
	n = HexScreen_Append(text, n, name, sizeof(text));
	HexScreen_Append(text, n, tail, sizeof(text));
	HexScreen_BarText(proc, text);
}

/* The name bar with a card's name between head and tail ("Gained " + name + "!"). */
static void HexScreen_BarCard(struct ProcHexScreen *proc, const char *head, int type, const char *tail)
{
	HexScreen_BarName(proc, head, HexScreen_TypeName(type), tail);
}

/* Draws a name bar code (HEX_BAR_*, or a card type: the card's name) if it is not the one shown. */
static void HexScreen_SetBar(struct ProcHexScreen *proc, u8 code)
{
	struct HexScreenState *h = HEX_ST;

	if (h->bar == code)
		return;
	h->bar = code;

	if (code < HEX_CARD_TYPES) {
		HexScreen_BarText(proc, HexScreen_TypeName(code));
		return;
	}
	switch (code) {
	case HEX_BAR_BLUE_BEGINS:
		HexScreen_BarText(proc, "Blue begins");
		break;
	case HEX_BAR_RED_BEGINS:
		HexScreen_BarName(proc, "", GetStringFromIndex(h->oppNameId), " begins");
		break;
	case HEX_BAR_RED_THINKING:
		HexScreen_BarName(proc, "", GetStringFromIndex(h->oppNameId), " is thinking");
		break;
	case HEX_BAR_DECK:
		HexScreen_BarName(proc, "", GetStringFromIndex(h->oppNameId), "'s deck");
		break;
	case HEX_BAR_TIER1:
		HexScreen_BarText(proc, "Tier 1");
		break;
	case HEX_BAR_TIER2:
		HexScreen_BarText(proc, "Tier 2");
		break;
	case HEX_BAR_TIER3:
		HexScreen_BarText(proc, "Tier 3");
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
	case HEX_BAR_OPPONENT_HAND:
		HexScreen_BarText(proc, "Your opponent's hand");
		break;
	case HEX_BAR_ACCEPT:
		HexScreen_BarText(proc, "Accept this hand?");
		break;
	case HEX_BAR_CANT_HOLD:
		HexScreen_BarText(proc, "You can't hold more");
		break;
	case HEX_BAR_GAINED:
		HexScreen_BarCard(proc, "Gained ", h->gotType, "!");
		break;
	case HEX_BAR_LOST:
		HexScreen_BarCard(proc, "Lost ", h->lostType, "...");
		break;
	case HEX_BAR_SAFE:
		HexScreen_BarText(proc, "Your cards are safe this time.");
		break;
	case HEX_BAR_NO_GAIN:
		HexScreen_BarText(proc, "No card gained.");
		break;
	case HEX_BAR_DRAW_NO_CHANGE:
		HexScreen_BarText(proc, "Draw. No cards change hands.");
		break;
	default:
		HexScreen_BarText(proc, "");
		break;
	}
}

/* The bar code of the current state (what the bar should say now): the highlighted card, the card just placed, or a banner. */
static u8 HexScreen_BarFor(const struct ProcHexScreen *proc)
{
	const struct HexScreenState *h = HEX_ST;
	u8 w;

	switch (h->state) {
	case HEX_STATE_MENU:
		return HEX_BAR_NONE;
	case HEX_STATE_OPPONENT:
		return HexScreen_LivingCount() > 0 ? HEX_BAR_DECK : HEX_BAR_NONE;
	case HEX_STATE_CARDS:
		if (h->held[h->cardCursor] == 0)
			return HEX_BAR_NONE;
		return (u8)(HEX_BAR_TIER1 + gpHexCardTypes[h->cardCursor].tier - 1);
	case HEX_STATE_BEGIN:
		return h->game.turn == HEX_BLUE ? HEX_BAR_BLUE_BEGINS : HEX_BAR_RED_BEGINS;
	case HEX_STATE_HAND_PICK:
		return h->ownedN > 0 ? h->owned[h->pickCursor] : HEX_BAR_NONE;
	case HEX_STATE_PICK_DONE:
		return HEX_BAR_ACCEPT;
	case HEX_STATE_REVEAL:
		return HEX_BAR_OPPONENT_HAND;
	case HEX_STATE_PICK_TILE:
	case HEX_STATE_PICK_CELL:
		return h->pickHand[proc->sel];
	case HEX_STATE_RESOLVE:
		return h->placedType;
	case HEX_STATE_AI_TURN:
		return HEX_BAR_RED_THINKING;
	case HEX_STATE_TAKE:
		if (h->notice > 0)
			return HEX_BAR_CANT_HOLD;
		return h->redHand[h->takeSel];
	case HEX_STATE_REWARD:
		if (h->result == HEX_RESULT_DRAW)
			return HEX_BAR_DRAW_NO_CHANGE;
		if (h->result == HEX_RESULT_WIN)
			return h->gotType == HEX_NO_TYPE ? HEX_BAR_NO_GAIN : HEX_BAR_GAINED;
		return h->lostType == HEX_NO_TYPE ? HEX_BAR_SAFE : HEX_BAR_LOST;
	default:
		w = Hex_Winner(&h->game);
		if (w == 0)
			return HEX_BAR_DRAW;
		return w == HEX_BLUE ? HEX_BAR_VICTORY : HEX_BAR_DEFEAT;
	}
}

/*
 * MENU (the opening state, and the state after a match's stakes): the collection is read again (the stakes change it), the
 * match's picks and stakes are cleared, and the name bar is blanked.
 */
static void HexScreen_ToMenu(struct HexScreenState *h)
{
	HexScreen_Scan(h);
	h->state = HEX_STATE_MENU;
	h->timer = 0;
	h->pickCount = 0;
	h->pickCursor = 0;
	h->pickScroll = 0;
	h->pickChoice = 0;
	h->bar = HEX_BAR_STALE;
	h->takeSel = 0;
	h->notice = 0;
	h->gotType = HEX_NO_TYPE;
	h->lostType = HEX_NO_TYPE;
}

static void HexScreen_Init(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	int i;

	/* Hex Card items (convoy and units) move into the binder; then the first open with no cards hands out the starter set. */
	HexCol_MigrateItems();
	if (HexCol_Total() == 0)
		HexCol_GiveStarters();

	proc->dispBak = gLCDControlBuffer.dispcnt;

	/* The standard prep sub-screen layout (as PrepItemList_InitGfx_INFUSE): the mural on BG3, the windows on BG1, the text on BG0. */
	SetupBackgrounds(NULL);
	SetDispEnable(0, 0, 0, 0, 0);
	BG_Fill(BG_GetMapBuffer(0), 0);
	BG_Fill(BG_GetMapBuffer(1), 0);
	BG_Fill(BG_GetMapBuffer(2), 0);
	BG_Fill(BG_GetMapBuffer(3), 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();
	HexScreen_LoadSprites();

	RestartMuralBackground();
	gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
	EnablePaletteSync();

	/* BG0 text, BG1 windows, BG3 mural, OBJ 1D mapping. */
	SetDispEnable(1, 1, 0, 1, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	HexScreen_MakeTypes(h);
	for (i = 0; i < HEX_CARD_TYPES; i++)
		h->typeBank[i] = HEX_NO_BANK;

	/* No opponent yet: the red column stays empty until Play picks one (HexScreen_SetOpponent sets the deck and the AI). */
	for (i = 0; i < HEX_HAND; i++)
		h->redHand[i] = 0;
	h->aiLevel = 0;
	h->oppPid = 0;
	h->oppNameId = 0;
	h->menuSel = 0;
	h->oppCursor = 0;
	h->oppScroll = 0;
	h->cardCursor = 0;
	h->cardScroll = 0;
	h->listKey = 0;
	HexScreen_ToMenu(h);

	InitText(&proc->nameText, HEX_NAME_TEXT_TILES);
	InitText(&h->title, HEX_PICK_TITLE_TILES);
	for (i = 0; i < HEX_PICK_VISIBLE; i++) {
		InitText(&h->rows[i], HEX_PICK_NAME_TILES);
		InitText(&h->counts[i], HEX_PICK_COUNT_TILES);
	}

	if (h->npcDeck != HEX_NO_DECK) {
		/* District mode (HexScreen_SetDistrictDeck): the NPC is the opponent and the hand picker opens at once (the UpdateOpponent A branch). */
		HexScreen_SetNpcOpponent(h, &gHexNpcDecks[h->npcDeck]);
		h->pickCount = 0;
		h->pickCursor = 0;
		h->pickScroll = 0;
		h->pickChoice = 0;
		h->state = HEX_STATE_HAND_PICK;
	}
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG3_SYNC_BIT);
	SetDefaultColorEffects();
}

/* ---- The hand picker (HAND_PICK, PICK_DONE, REVEAL) ---- */

/* Back to HAND_PICK with the last card removed (PICK_DONE's Change, or B). */
static void HexScreen_Change(struct HexScreenState *h)
{
	h->pickCount--;
	h->pickChoice = 0;
	h->state = HEX_STATE_HAND_PICK;
	h->listKey = 0;
	PlaySoundEffect(HEX_SE_CANCEL);
}

/* The 7 cards are chosen: the opponent's hand is revealed and the match starts (BEGIN) after REVEAL. */
static void HexScreen_StartReveal(struct HexScreenState *h)
{
	u8 first = (GetGameClock() & 1) ? HEX_RED : HEX_BLUE;
	int i;

	for (i = 0; i < HEX_HAND; i++)
		HexScreen_BankFor(h, h->pickHand[i]);

	Hex_Init(&h->game, h->pickHand, h->redHand, first);
	h->state = HEX_STATE_REVEAL;
	h->timer = 0;
	h->aiHand = HEX_USED;
	h->aiCell = HEX_USED;
	h->placedType = 0;
	h->cell = HexScreen_FirstEmpty(&h->game);
	PlaySoundEffect(HEX_SE_SELECT);
}

/* HAND_PICK: Up / Down move the list cursor, A adds a copy of the highlighted type (error sound if none is left or the art is
 * out of banks), B removes the last card (with none in hand it goes back to OPPONENT). The 7th card goes to PICK_DONE.
 */
static void HexScreen_UpdatePick(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;
	u16 repeated = gKeyStatusPtr->repeatedKeys;
	int t;

	if (newKeys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		if (h->pickCount == 0) {
			h->state = HEX_STATE_OPPONENT;
			return;
		}
		h->pickCount--;
		h->listKey = 0;
		return;
	}
	if (h->ownedN == 0)
		return;

	if (repeated & (DPAD_UP | DPAD_DOWN)) {
		if (HexScreen_Step(&h->pickCursor, &h->pickScroll, h->ownedN, (repeated & DPAD_DOWN) != 0)) {
			h->listKey = 0;
			PlaySoundEffect(HEX_SE_CURSOR_UD);
		}
	}

	t = h->owned[h->pickCursor];
	HexScreen_BankFor(h, t);
	if (newKeys & A_BUTTON) {
		if (h->pickCount < HEX_HAND && HexScreen_Avail(h, t) > 0 && h->typeBank[t] != HEX_NO_BANK) {
			h->pickHand[h->pickCount] = (u8)t;
			h->pickCount++;
			h->listKey = 0;
			PlaySoundEffect(HEX_SE_SELECT);
			if (h->pickCount == HEX_HAND) {
				h->state = HEX_STATE_PICK_DONE;
				h->pickChoice = 0;
			}
		} else {
			PlaySoundEffect(HEX_SE_ERROR);
		}
	}
}

/* PICK_DONE: Up / Down choose Accept (0) or Change (1). A: Accept starts the reveal, Change goes back to HAND_PICK. B is Change. */
static void HexScreen_UpdatePickDone(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;

	if (newKeys & (DPAD_UP | DPAD_DOWN)) {
		h->pickChoice ^= 1;
		PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if (newKeys & B_BUTTON) {
		HexScreen_Change(h);
		return;
	}
	if (newKeys & A_BUTTON) {
		if (h->pickChoice == 0)
			HexScreen_StartReveal(h);
		else
			HexScreen_Change(h);
	}
}

/* REVEAL: both hands for HEX_REVEAL_FRAMES (A skips), then BEGIN. */
static void HexScreen_UpdateReveal(struct HexScreenState *h)
{
	h->timer++;
	if (h->timer < HEX_REVEAL_FRAMES && (gKeyStatusPtr->newKeys & A_BUTTON) == 0)
		return;
	h->state = HEX_STATE_BEGIN;
	h->timer = 0;
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

/*
 * Plays hand slot hand of the player whose turn it is on cell, then RESOLVE. Before the placement the display snapshot is taken
 * (the board, both scores); after it, the placed card is written into the snapshot at its full HP, so RESOLVE's first frames
 * show the card on the board and nothing else changed yet (HexScreen_Before).
 */
static void HexScreen_Place(u8 hand, u8 cell)
{
	struct HexScreenState *h = HEX_ST;
	struct HexGame *g = &h->game;
	u8 mover = g->turn;
	int c;

	for (c = 0; c < HEX_CELLS; c++)
		h->shownCell[c] = g->cell[c];
	h->shownScore[HEX_BLUE] = (u8)Hex_Score(g, HEX_BLUE);
	h->shownScore[HEX_RED] = (u8)Hex_Score(g, HEX_RED);

	h->placedType = g->hand[mover][hand];
	Hex_Place(g, h->types, hand, cell);
	h->shownCell[g->lastCell].type = h->placedType;
	h->shownCell[g->lastCell].owner = mover;
	h->shownCell[g->lastCell].hp = (s8)h->types[h->placedType].maxHp;

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

/* PICK_TILE: A goes to PICK_CELL. B does nothing: a match is played to its end. */
static void HexScreen_UpdatePickTile(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	u16 keys = gKeyStatusPtr->newKeys;

	if (keys & (DPAD_UP | DPAD_DOWN)) {
		if (HexScreen_StepSlot(proc, (keys & DPAD_DOWN) != 0))
			PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if (keys & A_BUTTON) {
		h->state = HEX_STATE_PICK_CELL;
		h->cell = HexScreen_FirstEmpty(&h->game);
		PlaySoundEffect(HEX_SE_SELECT);
	}
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

/*
 * RESOLVE: the board from before for HEX_ATTACK_DELAY frames; then the attack (the shake, and a hit sound if anything changed),
 * then the settled board until HEX_RESOLVE_FRAMES.
 */
static void HexScreen_UpdateResolve(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	const struct HexGame *g = &h->game;

	h->timer++;
	if (h->timer == HEX_ATTACK_DELAY && (g->lastHit != 0 || g->lastFlip != 0 || g->lastCounter != 0))
		PlaySoundEffect(HEX_SE_HIT);
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
	Hex_AiChoose(&h->game, h->types, h->aiLevel, seed, &h->aiHand, &h->aiCell);
	if (h->aiHand >= HEX_HAND || h->aiCell >= HEX_CELLS) {
		h->state = HEX_STATE_END;
		h->timer = 0;
		return;
	}
	HexScreen_Place(h->aiHand, h->aiCell);
}

/* The opponent's starting hand has a type the player holds fewer than HEX_BINDER_MAX copies of. */
static bool32 HexScreen_Takeable(const struct HexScreenState *h)
{
	int i;

	for (i = 0; i < HEX_HAND; i++) {
		if (HexCol_Count(h->redHand[i]) < HEX_BINDER_MAX)
			return TRUE;
	}
	return FALSE;
}

/* The card of the player's hand the opponent takes: the highest tier, then the highest attack + max HP (the first on a tie). */
static u8 HexScreen_HighestPick(const struct HexScreenState *h)
{
	int i;
	int t;
	int tier;
	int sum;
	int bestTier = -1;
	int bestSum = -1;
	u8 best = h->pickHand[0];

	for (i = 0; i < HEX_HAND; i++) {
		t = h->pickHand[i];
		tier = gpHexCardTypes[t].tier;
		sum = gpHexCardTypes[t].attack + gpHexCardTypes[t].maxHp;
		if (tier > bestTier || (tier == bestTier && sum > bestSum)) {
			best = (u8)t;
			bestTier = tier;
			bestSum = sum;
		}
	}
	return best;
}

/* END: A decides the stakes. A win goes to TAKE (or REWARD when nothing can be taken); a loss takes one card when the player
 * holds more than a hand: a district NPC takes a random card of the played hand, the prep menu takes the best one. A draw
 * changes nothing.
 */
static void HexScreen_UpdateEnd(struct HexScreenState *h)
{
	u8 w;

	if ((gKeyStatusPtr->newKeys & A_BUTTON) == 0)
		return;
	PlaySoundEffect(HEX_SE_SELECT);

	w = Hex_Winner(&h->game);
	h->gotType = HEX_NO_TYPE;
	h->lostType = HEX_NO_TYPE;
	h->takeSel = 0;
	h->notice = 0;
	h->timer = 0;
	if (w == 0) {
		h->result = HEX_RESULT_DRAW;
		h->state = HEX_STATE_REWARD;
	} else if (w == HEX_BLUE) {
		h->result = HEX_RESULT_WIN;
		h->state = HexScreen_Takeable(h) ? HEX_STATE_TAKE : HEX_STATE_REWARD;
	} else {
		h->result = HEX_RESULT_LOSS;
		if (HexCol_Total() > HEX_HAND) {
			u8 lost;

			if (h->npcDeck != HEX_NO_DECK)
				lost = h->pickHand[NextRN_N(HEX_HAND)];
			else
				lost = HexScreen_HighestPick(h);

			if (HexCol_TakeOne(lost))
				h->lostType = lost;
		}
		h->state = HEX_STATE_REWARD;
	}
}

/* TAKE: UP / DOWN choose one of the opponent's starting cards; A gives it to the binder. A type already at HEX_BINDER_MAX copies
 * is refused (error sound and a message for HEX_NOTICE_FRAMES).
 */
static void HexScreen_UpdateTake(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;
	int t;

	if (h->notice > 0)
		h->notice--;
	if ((newKeys & DPAD_UP) && h->takeSel > 0) {
		h->takeSel--;
		PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if ((newKeys & DPAD_DOWN) && h->takeSel + 1 < HEX_HAND) {
		h->takeSel++;
		PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if (newKeys & A_BUTTON) {
		t = h->redHand[h->takeSel];
		if (HexCol_Give(t)) {
			const struct HexNpcCard *card;

			/* A rare card of a district NPC counts toward its claim cap. Checked before the claim is added, so the win that
			 * reaches the cap still counts. A fallback (the cap is reached) is not rare-live and counts for nothing.
			 */
			if (h->npcDeck != HEX_NO_DECK) {
				card = &gHexNpcDecks[h->npcDeck].cards[h->takeSel];
				if (card->claimSlot != HEX_NO_CLAIM && HexNpc_IsRareLive(card))
					HexClaim_Add(card->claimSlot);
			}
			h->gotType = (u8)t;
			h->state = HEX_STATE_REWARD;
			h->timer = 0;
			PlaySoundEffect(HEX_SE_SELECT);
		} else {
			PlaySoundEffect(HEX_SE_ERROR);
			h->notice = HEX_NOTICE_FRAMES;
		}
	}
}

/* REWARD: the stake banner for HEX_REWARD_FRAMES, or until A. Returns TRUE when the match is over (back to MENU). */
static bool32 HexScreen_UpdateReward(struct HexScreenState *h)
{
	h->timer++;
	if (h->timer < HEX_REWARD_FRAMES && (gKeyStatusPtr->newKeys & A_BUTTON) == 0)
		return FALSE;
	PlaySoundEffect(HEX_SE_SELECT);
	return TRUE;
}

/* ---- The front menu, the opponents and the collection (MENU, OPPONENT, CARDS) ---- */

/* MENU: UP / DOWN choose Play (0) or Cards (1), A opens it. B leaves the screen (the prep menu). Returns TRUE to leave. */
static bool32 HexScreen_UpdateMenu(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;

	if (newKeys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		return TRUE;
	}
	if (newKeys & (DPAD_UP | DPAD_DOWN)) {
		h->menuSel ^= 1;
		PlaySoundEffect(HEX_SE_CURSOR_UD);
	}
	if (newKeys & A_BUTTON) {
		PlaySoundEffect(HEX_SE_SELECT);
		if (h->menuSel == 0) {
			h->state = HEX_STATE_OPPONENT;
			h->oppCursor = 0;
			h->oppScroll = 0;
			h->bar = HEX_BAR_STALE;
			HexScreen_CursorOpponent(h);
		} else {
			h->state = HEX_STATE_CARDS;
		}
	}
	return FALSE;
}

/* OPPONENT: UP / DOWN choose one of the living units (its deck and AI level are set as the cursor moves). A starts the hand
 * picker against that deck, B goes back to MENU.
 */
static void HexScreen_UpdateOpponent(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;
	u16 repeated = gKeyStatusPtr->repeatedKeys;
	int count = HexScreen_LivingCount();

	if (newKeys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		h->state = HEX_STATE_MENU;
		return;
	}
	if (count == 0)
		return;

	if ((repeated & (DPAD_UP | DPAD_DOWN)) &&
	    HexScreen_Step(&h->oppCursor, &h->oppScroll, count, (repeated & DPAD_DOWN) != 0)) {
		PlaySoundEffect(HEX_SE_CURSOR_UD);
		h->listKey = 0;
		h->bar = HEX_BAR_STALE;
		HexScreen_CursorOpponent(h);
	}

	if (newKeys & A_BUTTON) {
		PlaySoundEffect(HEX_SE_SELECT);
		HexScreen_CursorOpponent(h);
		h->pickCount = 0;
		h->pickCursor = 0;
		h->pickScroll = 0;
		h->pickChoice = 0;
		h->state = HEX_STATE_HAND_PICK;
	}
}

/* CARDS: UP / DOWN move over every card type; an owned card is previewed (its art is loaded). B goes back to MENU. */
static void HexScreen_UpdateCards(struct HexScreenState *h)
{
	u16 newKeys = gKeyStatusPtr->newKeys;
	u16 repeated = gKeyStatusPtr->repeatedKeys;

	if (newKeys & B_BUTTON) {
		PlaySoundEffect(HEX_SE_CANCEL);
		h->state = HEX_STATE_MENU;
		return;
	}
	if ((repeated & (DPAD_UP | DPAD_DOWN)) &&
	    HexScreen_Step(&h->cardCursor, &h->cardScroll, HEX_CARD_TYPES, (repeated & DPAD_DOWN) != 0)) {
		PlaySoundEffect(HEX_SE_CURSOR_UD);
		h->listKey = 0;
		h->bar = HEX_BAR_STALE;
	}
	if (h->held[h->cardCursor] > 0)
		HexScreen_BankFor(h, h->cardCursor);
}

static void HexScreen_Loop(struct ProcHexScreen *proc)
{
	struct HexScreenState *h = HEX_ST;
	u8 before = h->state;
	bool32 leave = FALSE;

	switch (h->state) {
	case HEX_STATE_MENU:
		leave = HexScreen_UpdateMenu(h);
		break;
	case HEX_STATE_OPPONENT:
		HexScreen_UpdateOpponent(h);
		break;
	case HEX_STATE_CARDS:
		HexScreen_UpdateCards(h);
		break;
	case HEX_STATE_HAND_PICK:
		HexScreen_UpdatePick(h);
		break;
	case HEX_STATE_PICK_DONE:
		HexScreen_UpdatePickDone(h);
		break;
	case HEX_STATE_REVEAL:
		HexScreen_UpdateReveal(h);
		break;
	case HEX_STATE_BEGIN:
		HexScreen_UpdateBegin(proc);
		break;
	case HEX_STATE_PICK_TILE:
		HexScreen_UpdatePickTile(proc);
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
	case HEX_STATE_TAKE:
		HexScreen_UpdateTake(h);
		break;
	case HEX_STATE_REWARD:
		if (HexScreen_UpdateReward(h))
			HexScreen_ToMenu(h);
		break;
	default:
		HexScreen_UpdateEnd(h);
		break;
	}

	/* District mode: the front menu and the opponent list are not part of it. B with no card, or the end of a match, leaves. */
	if (h->npcDeck != HEX_NO_DECK && (h->state == HEX_STATE_MENU || h->state == HEX_STATE_OPPONENT))
		leave = TRUE;

	if (leave) {
		Proc_Goto(proc, PL_HEX_EXIT);
		return;
	}
	if (h->state != before && HexScreen_IsList(h->state))
		h->listKey = 0;
	HexScreen_ListUpdate(h);
	HexScreen_SetBar(proc, HexScreen_BarFor(proc));
	HexScreen_Draw(proc);
}

static void HexScreen_OnEnd(struct ProcHexScreen *proc)
{
	ClearSprites();
	EndMuralBackground_();
	ClearBg0Bg1();
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();

	/*
	 * Same hand-off as the other prep sub-screens: the prep menu redraws itself. cur_cmd puts the cursor back on the Hex row.
	 * In district mode (a match) the parent is the map's HexDistrict proc, not the prep menu.
	 */
	if (HEX_ST->npcDeck == HEX_NO_DECK) {
		struct ProcAtMenu *pproc = proc->proc_parent;

		pproc->cur_cmd = PREP_MAINMENU_HEX;
		pproc->state = 1;
	}
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
