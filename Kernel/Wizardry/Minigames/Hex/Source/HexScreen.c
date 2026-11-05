#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "prepscreen.h"
#include "hex.h"
#include "kernel/hex-core.h"

#ifdef CONFIG_HEX_CARDS

/*
 * The Hex board screen (Preparations > Hex). A fixed demo position: the board (14 cells, 2-3-4-3-2), the blue hand on the left
 * (the first 7 cards the player owns, from the collection), the red demo hand on the right, two blue and two red cards placed
 * on the board, both scores and the name bar (the highlighted blue card's name). Nothing is played yet.
 *
 * Layers: BG1 wood (priority 3, charblock 2 tiles 0..3, screenblock 20; tile 4 is the solid name-bar box), BG0 text (priority
 * 0, the name). OBJ 1D mapping. OBJ palette banks 0 (red frame) and 1 (blue frame), 2..13 the portraits of the cards on screen.
 * Every frame draws the sprites again (ClearSprites, then PutSprite), front-most first: a cell is the digits, the portrait, the
 * frame. Board layout and card art: docs/custom.md "Hex" (the Minish Cap mod, HexScreen.c there).
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

static const s16 sHexBoxX[HEX_CELLS] = { 90, 118, 76, 104, 132, 62, 90, 118, 146, 76, 104, 132, 90, 118 };
static const s16 sHexBoxY[HEX_CELLS] = { 10, 10, 34, 34, 34, 58, 58, 58, 58, 82, 82, 82, 106, 106 };

/* The red demo hand, and the demo position on the board: cell, owner, card type, current HP. */
static const u8 sHexRedHand[HEX_HAND] = { 4, 6, 7, 12, 13, 14, 15 }; /* Colm, Lute, Garcia, Kyle, Forde, Dozla, Joshua */

struct HexDemoCard {
	u8 cell;
	u8 owner;
	u8 type;
	u8 hp;
};

static const struct HexDemoCard sHexDemoBoard[] = {
	{ 2, HEX_BLUE, 0, 2 },  /* Ross */
	{ 7, HEX_BLUE, 1, 1 },  /* Amelia, damaged */
	{ 9, HEX_RED, 4, 1 },   /* Colm */
	{ 12, HEX_RED, 14, 3 }, /* Dozla, damaged */
};

static const u16 sHexBarPal[16] = { 0x0000, HEX_BG_BAR_COLOUR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const u32 sHexBarTile[8] = { 0x11111111, 0x11111111, 0x11111111, 0x11111111,
				    0x11111111, 0x11111111, 0x11111111, 0x11111111 };

/*
 * The board state lives in gGenericBuffer, past the portrait decompression buffer. A proc is only 0x6C bytes (struct Proc in
 * FE-CLib proc.h), so the proc keeps the small fields and the text, and the rest goes here.
 */
#define HEX_STATE_OFFSET 0x1000
#define HEX_ST ((struct HexScreenState *)(gGenericBuffer + HEX_STATE_OFFSET))

struct HexScreenState {
	struct HexGame game;
	u8 blueHand[HEX_HAND];
	u8 blueCount;
	u8 typeBank[HEX_CARD_TYPES];
};

struct ProcHexScreen {
	PROC_HEADER;
	struct Text nameText;
	struct DispCnt dispBak;
	struct BgCnt bg1Bak;
	u8 sel;
};
/* The blue hand: the first cards of the collection, in type order (up to HEX_HAND). Returns the count. */
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
static void HexScreen_LoadArt(struct ProcHexScreen *proc)
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

/* The demo position: the blue hand (collection), the red hand, the placed cards, the scores (Hex_Score: placed + in hand). */
static void HexScreen_SetupGame(struct ProcHexScreen *proc)
{
	struct HexGame *g = &HEX_ST->game;
	int i;

	for (i = 0; i < HEX_CELLS; i++) {
		g->cell[i].type = 0;
		g->cell[i].owner = HEX_EMPTY;
		g->cell[i].hp = 0;
	}
	for (i = 0; i < HEX_HAND; i++) {
		g->hand[HEX_BLUE][i] = i < HEX_ST->blueCount ? HEX_ST->blueHand[i] : HEX_USED;
		g->hand[HEX_RED][i] = sHexRedHand[i];
	}
	for (i = 0; i < (int)ARRAY_COUNT(sHexDemoBoard); i++) {
		const struct HexDemoCard *c = &sHexDemoBoard[i];

		g->cell[c->cell].type = c->type;
		g->cell[c->cell].owner = c->owner;
		g->cell[c->cell].hp = (s8)c->hp;
	}
	g->turn = HEX_BLUE;
	g->placed = (u8)ARRAY_COUNT(sHexDemoBoard);
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
static void HexScreen_CardBody(struct ProcHexScreen *proc, int x, int y, int type, u32 owner)
{
	u32 base = HEX_OBJ_TYPE_BASE + (u32)(HEX_ST->typeBank[type] - HEX_OBJ_BANK_FIRST) * HEX_OBJ_TYPE_TILES;
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;

	HexScreen_Push(x, y, gObject_32x32, base, frameBank);
	HexScreen_Push(x, y, gObject_32x32, base + HEX_OBJ_PORTRAIT_OFFSET, HEX_ST->typeBank[type]);
}

/* The hexagon board: an empty cell is the outline, a placed card its digits, portrait and frame. */
static void HexScreen_Cells(struct ProcHexScreen *proc)
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
		HexScreen_CardBody(proc, x, y, g->cell[c].type, owner);
		HexScreen_CardDigits(x, y, g->cell[c].type, frameBank, TRUE, g->cell[c].hp);
	}
}

/*
 * A hand strip: the bodies, the lowest slot in front (each card overlaps the one above it), then the digits of every slot
 * on top of every frame.
 */
static void HexScreen_Hand(struct ProcHexScreen *proc, int x, const u8 *types, int n, u8 owner)
{
	u32 frameBank = owner == HEX_BLUE ? HEX_BANK_BLUE : HEX_BANK_RED;
	int i;

	for (i = 0; i < n; i++) {
		if (types[i] != HEX_USED)
			HexScreen_CardBody(proc, x, HEX_HAND_Y + HEX_HAND_STEP * i, types[i], owner);
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

static void HexScreen_Draw(struct ProcHexScreen *proc)
{
	/* The fade and the prep menu's own code turn BG2 and BG3 back on: keep the screen's layers (BG0 text, BG1 wood, OBJ). */
	SetDispEnable(1, 1, 0, 0, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 1;

	ClearSprites();

	/* The cursor on the highlighted blue card: a left arrow just right of its slot. */
	if (HEX_ST->blueCount > 0)
		HexScreen_Push(HEX_HAND_BLUE_X + HEX_BOX + HEX_CURSOR_GAP, HEX_HAND_Y + HEX_HAND_STEP * proc->sel + 4, gObject_16x16,
		       HEX_OBJ_CURSOR + 0, HEX_BANK_RED);

	HexScreen_Score(&HEX_ST->game, HEX_BLUE, HEX_SCORE_BLUE_X);
	HexScreen_Score(&HEX_ST->game, HEX_RED, HEX_SCORE_RED_X);
	HexScreen_Cells(proc);
	HexScreen_Hand(proc, HEX_HAND_BLUE_X, HEX_ST->blueHand, HEX_ST->blueCount, HEX_BLUE);
	HexScreen_Hand(proc, HEX_HAND_RED_X, sHexRedHand, HEX_HAND, HEX_RED);
}

/* The name bar: the name of the highlighted blue card, centred on row 18 (BG0 text over the BG1 bar). */
static void HexScreen_Name(struct ProcHexScreen *proc)
{
	const struct CharacterData *chara;
	const char *name;
	int tiles;
	int col;
	int x;

	for (x = 0; x < HEX_NAME_COLS; x++) {
		gBG0TilemapBuffer[TILEMAP_INDEX(x, HEX_NAME_ROW)] = 0;
		gBG0TilemapBuffer[TILEMAP_INDEX(x, HEX_NAME_ROW + 1)] = 0;
	}
	ClearText(&proc->nameText);

	/* No card owned (the convoy was full when the starters were handed out): the bar stays empty. */
	if (HEX_ST->blueCount == 0) {
		BG_EnableSyncByMask(BG0_SYNC_BIT);
		return;
	}

	chara = GetCharacterData(gpHexCardTypes[HEX_ST->blueHand[proc->sel]].pid);
	name = GetStringFromIndex(chara->nameTextId);
	tiles = (GetStringTextLen(name) + 7) / 8;
	col = (HEX_NAME_COLS - tiles) / 2;

	Text_SetParams(&proc->nameText, 0, TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&proc->nameText, name);
	PutText(&proc->nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, col, HEX_NAME_ROW));

	BG_EnableSyncByMask(BG0_SYNC_BIT);
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

	HEX_ST->blueCount = (u8)HexScreen_BlueHand(HEX_ST->blueHand);
	proc->sel = 0;
	HexScreen_SetupGame(proc);
	HexScreen_LoadArt(proc);

	InitText(&proc->nameText, 16);
	HexScreen_Name(proc);
	BG_EnableSyncByMask(BG1_SYNC_BIT);
	SetDefaultColorEffects();
}

static void HexScreen_Loop(struct ProcHexScreen *proc)
{
	u16 keys = gKeyStatusPtr->newKeys;

	if (keys & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		Proc_Goto(proc, PL_HEX_EXIT);
		return;
	}

	if ((keys & DPAD_UP) && proc->sel > 0) {
		proc->sel--;
		HexScreen_Name(proc);
	}
	if ((keys & DPAD_DOWN) && proc->sel + 1 < HEX_ST->blueCount) {
		proc->sel++;
		HexScreen_Name(proc);
	}

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
