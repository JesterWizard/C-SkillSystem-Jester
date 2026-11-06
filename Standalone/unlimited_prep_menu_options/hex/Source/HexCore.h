#pragma once

/*
 * Hex card game: rules engine and AI (ported from the Minish Cap mod, src_custom/hexCore.c there).
 *
 * Plain C, no game globals, no malloc, no floats, no / or % in HexCore.c. Uses the
 * game's u8/s8/u16/s16/u32/bool32 types (FE-CLib global.h). The same file builds on the host with -DHEX_HOST and a shim
 * (the host test shim is not part of this add-on).
 *
 * Board: 14 pointy-top hexes in 5 rows of 2, 3, 4, 3, 2 cells, each row centred. Cells are numbered 0..13 row by row, left
 * to right:
 *
 *            00    01
 *        02    03    04
 *     05    06    07    08
 *        09    10    11
 *            12    13
 *
 * Directions: 0 = up-left, 1 = up-right, 2 = right, 3 = down-right, 4 = down-left, 5 = left. Opposite = d + 3 (mod 6).
 * Axial layout used for the neighbour table (row r, column x in half-cell units, centre of the board is x = 0):
 *   00 (0,-1)  01 (0,1)
 *   02 (1,-2)  03 (1,0)  04 (1,2)
 *   05 (2,-3)  06 (2,-1) 07 (2,1)  08 (2,3)
 *   09 (3,-2)  10 (3,0)  11 (3,2)
 *   12 (4,-1)  13 (4,1)
 * A neighbour in direction d is (r + dr[d], x + dx[d]) with (dr, dx) = (-1,-1), (-1,1), (0,2), (1,1), (1,-1), (0,-2).
 * Cells 03, 06, 07 and 10 have 6 neighbours.
 *
 * Tile types: attack (1..9), maxHp (1..9), reach (6-bit mask, bit d = the tile attacks in direction d).
 * A placed tile: type (index into the caller's HexTileType table), owner (HEX_BLUE / HEX_RED, 0 = empty cell), hp (s8).
 * Each player has a hand of HEX_HAND tile type ids; a used slot is HEX_USED (0xFF). The players alternate; 14 placements.
 *
 * Placement (Hex_Place): (1) attack: every enemy tile in a neighbour direction of the reach loses the tile's attack;
 * (2) counterattack: the placed tile loses the attack of every enemy neighbour whose reach points back at the placed cell
 * (also when that neighbour is at 0 HP and not yet flipped); (3) flip: every tile with hp <= 0 changes owner and gets
 * hp = maxHp (damaged-only tiles keep their hp).
 *
 * Results of the last Hex_Place (for the caller's animation): lastCell (the cell placed), lastHit (bit n: the enemy tile in
 * cell n lost HP to the attack), lastCounter (bit n: the enemy tile in cell n counterattacked the placed tile), lastFlip
 * (bit n: the tile in cell n changed owner). The return value of Hex_Place is lastFlip.
 */

#include "global.h"

#define HEX_CELLS 14
#define HEX_HAND 7
#define HEX_EMPTY 0
#define HEX_BLUE 1 /* the player */
#define HEX_RED 2 /* the opponent */
#define HEX_USED 0xFF
#define HEX_DIRS 6

struct HexTileType {
	u8 attack, maxHp, reach;
};

struct HexCell {
	u8 type, owner;
	s8 hp;
};

struct HexGame {
	struct HexCell cell[HEX_CELLS];
	/* [owner][i] = tile type id, HEX_USED = already placed */
	u8 hand[3][HEX_HAND];
	/* HEX_BLUE or HEX_RED: whose placement is next */
	u8 turn;
	/* 0..14 */
	u8 placed;
	/* the cell of the last Hex_Place */
	u8 lastCell;
	/* enemy tiles that lost HP to the attack */
	u16 lastHit;
	/* enemy tiles that counterattacked the placed tile */
	u16 lastCounter;
	/* tiles that changed owner (the return value of Hex_Place) */
	u16 lastFlip;
};

/* Starts a game: blueHand / redHand are HEX_HAND tile type ids each; first is HEX_BLUE or HEX_RED. */
void Hex_Init(struct HexGame *g, const u8 *blueHand, const u8 *redHand, u8 first);

/* The cell in direction dir from cell, or -1 if there is none (or the arguments are out of range). */
s8 Hex_Neighbor(u8 cell, u8 dir);

/* True if the player whose turn it is may place hand slot handIndex on cell. */
bool32 Hex_IsLegal(const struct HexGame *g, u8 handIndex, u8 cell);

/* Plays hand slot handIndex of g->turn on cell. Returns the bitmask of flipped cells (lastFlip). An illegal move changes
 * nothing and returns 0.
 */
u32 Hex_Place(struct HexGame *g, const struct HexTileType *types, u8 handIndex, u8 cell);

/* Tiles on the board owned by owner plus tiles left in its hand. */
u32 Hex_Score(const struct HexGame *g, u8 owner);

bool32 Hex_IsOver(const struct HexGame *g);

/* 0 for a draw, otherwise HEX_BLUE or HEX_RED with the higher score. */
u8 Hex_Winner(const struct HexGame *g);

/*
 * Chooses a legal move for g->turn. level 0 = random legal move, 1 = greedy (own - other score after the move), 2 = greedy
 * with a one-ply look-ahead (own - other after the opponent's greedy reply). Levels above 2 play as 2. Levels 1 and 2 break a tie
 * on the value by aggression: the HP the opponent's cards lose right after the move (a card that changes owner counts its
 * maxHp), then the opponent's cards next to the placed cell, then seed. Level 0 is seed-random only, and is kept for the host
 * tests. When the game is over there is no move: both outputs are HEX_USED (0xFF).
 */
void Hex_AiChoose(const struct HexGame *g, const struct HexTileType *types, u8 level, u32 seed, u8 *outHandIndex,
		  u8 *outCell);

#ifdef HEX_HOST
/* Host tests only: the change of (mover score - other score) if mover places type on the empty cell (HexPlaceFast, and
 * the generic HexPlaceDelta, in HexCore.c).
 */
s32 Hex_PlaceDelta(const struct HexGame *g, const struct HexTileType *types, u8 type, u8 cell, u8 mover);
s32 Hex_PlaceDeltaSlow(const struct HexGame *g, const struct HexTileType *types, u8 type, u8 cell, u8 mover);
#endif
