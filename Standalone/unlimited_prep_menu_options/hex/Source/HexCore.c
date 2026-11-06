/*
 * Hex card game: rules engine and AI. Ported from the Minish Cap mod (src_custom/hexCore.c). See HexCore.h
 * for the board layout and the placement rules.
 *
 * No graphics, no game hooks. Plain C: no malloc, no floats, no / or % (the FE8 link has no signed modulo helper), no game
 * globals. The same file builds on the host with -DHEX_HOST (Tools/hex/run_hex_tests.sh).
 */
#include "global.h"
#include "HexCore.h"

#define HEX_NONE 0xFF

/* Neighbour table: sHexNeighbor[cell][dir], -1 = no cell there. Derived from the axial layout in hex-core.h. */
static const s8 sHexNeighbor[HEX_CELLS][HEX_DIRS] = {
	{ -1, -1, 1, 3, 2, -1 },	/* 00 */
	{ -1, -1, -1, 4, 3, 0 },	/* 01 */
	{ -1, 0, 3, 6, 5, -1 },		/* 02 */
	{ 0, 1, 4, 7, 6, 2 },		/* 03 */
	{ 1, -1, -1, 8, 7, 3 },		/* 04 */
	{ -1, 2, 6, 9, -1, -1 },	/* 05 */
	{ 2, 3, 7, 10, 9, 5 },		/* 06 */
	{ 3, 4, 8, 11, 10, 6 },		/* 07 */
	{ 4, -1, -1, -1, 11, 7 },	/* 08 */
	{ 5, 6, 10, 12, -1, -1 },	/* 09 */
	{ 6, 7, 11, 13, 12, 9 },	/* 10 */
	{ 7, 8, -1, -1, 13, 10 },	/* 11 */
	{ 9, 10, 13, -1, -1, -1 },	/* 12 */
	{ 10, 11, -1, -1, -1, 12 },	/* 13 */
};

/* The opposite direction of d (d + 3 mod 6, without a modulo). */
static u8 HexOpposite(u8 d)
{
	return (u8)(d < 3 ? d + 3 : d - 3);
}

/* Mixes a 32-bit value (multiply and shifts only). The result is the random source for the seed. */
static u32 HexHash(u32 x)
{
	x ^= x >> 16;
	x *= 0x7FEB352Du;
	x ^= x >> 15;
	x *= 0x846CA68Bu;
	x ^= x >> 16;
	return x;
}

/* Tie-break key of a move (hand slot * 16 + cell) for a seed. */
static u32 HexMoveKey(u32 seed, u32 move)
{
	return HexHash(seed + move * 0x9E3779B1u);
}

/* Score difference from the point of view of me (the player who moved). */
static s32 HexEval(const struct HexGame *g, u8 me)
{
	return (s32)Hex_Score(g, me) - (s32)Hex_Score(g, (u8)(3 - me));
}

/*
 * The first hand slot (<= hi) that holds the same tile type as the unused slot hi (hi itself if none). Placing that type
 * on a cell has the same value from either slot (the board and the rest of the hand are the same), so the search computes
 * the value once per type and cell. The tie-break still uses each slot's own move number.
 */
static u8 HexFirstSlot(const struct HexGame *g, u8 mover, u8 hi)
{
	u8 j;

	for (j = 0; j < hi; j++) {
		if (g->hand[mover][j] == g->hand[mover][hi])
			return j;
	}
	return hi;
}

/*
 * How much (mover score - other score) changes if mover places type on the empty cell, without copying g. The same rules
 * as Hex_Place: the placed tile moves from the hand to the board (+0); every tile that flips from the other player to
 * mover is +2, every tile that flips from mover to the other is -2. Only the placed cell and its neighbours change HP.
 */
static s32 HexPlaceDelta(const struct HexGame *g, const struct HexTileType *types, u8 type, u8 cell, u8 mover)
{
	const struct HexTileType *tt = &types[type];
	u8 other = (u8)(3 - mover);
	s8 nCell[HEX_DIRS];
	s8 nHp[HEX_DIRS];
	s8 placedHp = (s8)tt->maxHp;
	u32 nMask = 0;
	s32 delta = 0;
	u8 d;
	u8 i;

	/* 1. Attack: enemy neighbours in the reach directions lose the attack. */
	for (d = 0; d < HEX_DIRS; d++) {
		s8 n = sHexNeighbor[cell][d];

		nCell[d] = n;
		nHp[d] = 0;
		if (n < 0)
			continue;
		nMask |= (u32)1 << n;
		nHp[d] = g->cell[n].hp;
		if (g->cell[n].owner == other && (tt->reach & (1 << d)))
			nHp[d] = (s8)(nHp[d] - tt->attack);
	}
	/* 2. Counterattack: the placed tile loses the attack of every enemy neighbour whose reach points back at it. */
	for (d = 0; d < HEX_DIRS; d++) {
		u8 back = HexOpposite(d);
		s8 n = nCell[d];
		const struct HexTileType *nt;

		if (n < 0 || g->cell[n].owner != other)
			continue;
		nt = &types[g->cell[n].type];
		if (nt->reach & (1 << back))
			placedHp = (s8)(placedHp - nt->attack);
	}
	/* 3. Flip: every owned tile with hp <= 0 changes owner. The placed tile is mover's; a flipped tile is -2 for mover. */
	if (placedHp <= 0)
		delta -= 2;
	for (d = 0; d < HEX_DIRS; d++) {
		s8 n = nCell[d];

		if (n < 0 || g->cell[n].owner == HEX_EMPTY || nHp[d] > 0)
			continue;
		delta += (g->cell[n].owner == other) ? 2 : -2;
	}
	/*
	 * Cells that are not neighbours keep their HP (none is at 0 or below in a position the game reached, but Hex_Place
	 * would flip one, so it is counted the same way).
	 */
	for (i = 0; i < HEX_CELLS; i++) {
		if ((nMask & ((u32)1 << i)) || g->cell[i].owner == HEX_EMPTY || g->cell[i].hp > 0)
			continue;
		delta += (g->cell[i].owner == other) ? 2 : -2;
	}
	return delta;
}

/*
 * The facts of one board that every placement on it needs (built once per board by HexBoardPrepare): for each cell, the
 * directions of its enemy neighbours (the enemy of the mover), the counterattack a tile placed there takes, and the owned
 * tiles at 0 HP or below (none in a position the game reaches; see HexPlaceFast).
 */
struct HexBoard {
	u8 enemyDirs[HEX_CELLS];
	u8 counter[HEX_CELLS];
	u32 deadMask;
};

static void HexBoardPrepare(const struct HexGame *g, const struct HexTileType *types, u8 mover, struct HexBoard *bi)
{
	u8 other = (u8)(3 - mover);
	u8 c;
	u8 d;

	bi->deadMask = 0;
	for (c = 0; c < HEX_CELLS; c++) {
		bi->enemyDirs[c] = 0;
		bi->counter[c] = 0;
		if (g->cell[c].owner != HEX_EMPTY && g->cell[c].hp <= 0)
			bi->deadMask |= (u32)1 << c;
	}
	/*
	 * Each enemy tile e adds itself to its neighbours c: c sees e in direction opposite(d), and e counterattacks c when its
	 * reach points at c (direction d from e).
	 */
	for (c = 0; c < HEX_CELLS; c++) {
		const struct HexTileType *et;

		if (g->cell[c].owner != other)
			continue;
		et = &types[g->cell[c].type];
		for (d = 0; d < HEX_DIRS; d++) {
			s8 n = sHexNeighbor[c][d];

			if (n < 0)
				continue;
			bi->enemyDirs[n] = (u8)(bi->enemyDirs[n] | (1 << HexOpposite(d)));
			if (et->reach & (1 << d))
				bi->counter[n] = (u8)(bi->counter[n] + et->attack);
		}
	}
}

/*
 * HexPlaceDelta for a board prepared by HexBoardPrepare(g, types, mover). The placed tile loses the counterattack total
 * (no per-neighbour work); the hit enemy neighbours flip when the attack takes them to 0 HP or below. A board with an owned
 * tile at 0 HP or below (deadMask) goes to HexPlaceDelta, which flips those too.
 */
static s32 HexPlaceFast(const struct HexGame *g, const struct HexTileType *types, const struct HexBoard *bi, u8 type,
			u8 cell, u8 mover)
{
	const struct HexTileType *tt;
	u8 hits;
	s32 delta = 0;
	u8 d;

	if (bi->deadMask != 0)
		return HexPlaceDelta(g, types, type, cell, mover);
	tt = &types[type];
	if ((s32)tt->maxHp - (s32)bi->counter[cell] <= 0)
		delta -= 2;
	hits = (u8)(bi->enemyDirs[cell] & tt->reach);
	for (d = 0; d < HEX_DIRS; d++) {
		if (hits & (1 << d)) {
			s8 n = sHexNeighbor[cell][d];

			if ((s32)g->cell[n].hp - (s32)tt->attack <= 0)
				delta += 2;
		}
	}
	return delta;
}

#ifdef HEX_HOST
/* Host tests only: the change of (mover - other) for the two implementations (see Tools/hex/test_hex.c). */
s32 Hex_PlaceDelta(const struct HexGame *g, const struct HexTileType *types, u8 type, u8 cell, u8 mover)
{
	struct HexBoard bi;

	HexBoardPrepare(g, types, mover, &bi);
	return HexPlaceFast(g, types, &bi, type, cell, mover);
}

s32 Hex_PlaceDeltaSlow(const struct HexGame *g, const struct HexTileType *types, u8 type, u8 cell, u8 mover)
{
	return HexPlaceDelta(g, types, type, cell, mover);
}
#endif

/*
 * The aggression of a legal move of g->turn, the tie-break after the value (HexChooseBest). damage: the HP the opponent's
 * cards lose right after the move, a card that changes owner counting its maxHp instead (its HP is restored when it flips).
 * pressure: the opponent's cards next to the placed cell (before the move). The move is simulated on a copy of g.
 */
static void HexAggression(const struct HexGame *g, const struct HexTileType *types, u8 hand, u8 cell, s32 *outDamage,
			  s32 *outPressure)
{
	struct HexGame sim = *g;
	u8 other = (u8)(3 - g->turn);
	s32 damage = 0;
	s32 pressure = 0;
	u8 i;
	u8 d;

	Hex_Place(&sim, types, hand, cell);
	for (i = 0; i < HEX_CELLS; i++) {
		if (g->cell[i].owner != other)
			continue;
		if (sim.cell[i].owner == other)
			damage += (s32)g->cell[i].hp - (s32)sim.cell[i].hp;
		else
			damage += (s32)types[g->cell[i].type].maxHp;
	}
	for (d = 0; d < HEX_DIRS; d++) {
		s8 n = sHexNeighbor[cell][d];

		if (n >= 0 && g->cell[n].owner == other)
			pressure++;
	}
	*outDamage = damage;
	*outPressure = pressure;
}

/*
 * The move of g->turn with the best value val (val[first slot of its type][cell], the value of each legal move; HexFirstSlot).
 * Ties on the value go to the larger damage (HexAggression), then the larger pressure, then the larger HexMoveKey. With
 * tieBreak 0 the first move with the best value is taken (the value is all that a caller needs). The aggression is only
 * computed for moves that tie the best value. Returns FALSE if there is no legal move; *outVal is the best value.
 */
static bool32 HexChooseBest(const struct HexGame *g, const struct HexTileType *types, u32 seed, bool32 tieBreak,
			    const s16 val[HEX_HAND][HEX_CELLS], u8 *outHand, u8 *outCell, s32 *outVal)
{
	u8 mover = g->turn;
	bool32 found = 0;
	s32 bestVal = 0;
	s32 bestDamage = 0;
	s32 bestPressure = 0;
	u32 bestKey = 0;
	u8 hi;
	u8 c;

	for (hi = 0; hi < HEX_HAND; hi++) {
		u8 first;

		if (g->hand[mover][hi] == HEX_USED)
			continue;
		first = HexFirstSlot(g, mover, hi);
		for (c = 0; c < HEX_CELLS; c++) {
			s32 v;

			if (g->cell[c].owner != HEX_EMPTY)
				continue;
			v = val[first][c];
			if (!found || v > bestVal) {
				found = 1;
				bestVal = v;
			}
		}
	}
	if (!found)
		return 0;
	*outVal = bestVal;

	found = 0;
	for (hi = 0; hi < HEX_HAND; hi++) {
		u8 first;

		if (g->hand[mover][hi] == HEX_USED)
			continue;
		first = HexFirstSlot(g, mover, hi);
		for (c = 0; c < HEX_CELLS; c++) {
			s32 damage;
			s32 pressure;
			u32 key;

			if (g->cell[c].owner != HEX_EMPTY || val[first][c] != bestVal)
				continue;
			if (!tieBreak) {
				if (!found) {
					found = 1;
					*outHand = hi;
					*outCell = c;
				}
				continue;
			}
			HexAggression(g, types, hi, c, &damage, &pressure);
			key = HexMoveKey(seed, (u32)hi * HEX_CELLS + c);
			if (!found || damage > bestDamage || (damage == bestDamage && (pressure > bestPressure ||
			    (pressure == bestPressure && key > bestKey)))) {
				found = 1;
				bestDamage = damage;
				bestPressure = pressure;
				bestKey = key;
				*outHand = hi;
				*outCell = c;
			}
		}
	}
	return found;
}

/*
 * Level 1 (greedy) for g->turn: the value is the difference right after the move (HexPlaceFast). The move is HexChooseBest's
 * (with tieBreak, the aggression and then the key; without it, the first move of the best value). Returns FALSE if there is
 * no legal move.
 */
static bool32 HexPickGreedy(const struct HexGame *g, const struct HexTileType *types, u32 seed, bool32 tieBreak, u8 *outHand,
			    u8 *outCell, s32 *outDelta)
{
	u8 mover = g->turn;
	struct HexBoard bi;
	s16 val[HEX_HAND][HEX_CELLS];
	u8 hi;
	u8 c;

	HexBoardPrepare(g, types, mover, &bi);
	for (hi = 0; hi < HEX_HAND; hi++) {
		if (g->hand[mover][hi] == HEX_USED || HexFirstSlot(g, mover, hi) != hi)
			continue;
		for (c = 0; c < HEX_CELLS; c++) {
			if (g->cell[c].owner == HEX_EMPTY)
				val[hi][c] = (s16)HexPlaceFast(g, types, &bi, g->hand[mover][hi], c, mover);
		}
	}
	return HexChooseBest(g, types, seed, tieBreak, val, outHand, outCell, outDelta);
}

/*
 * Picks the best move of g->turn: level 1 maximises the difference right after the move; level 2 maximises it after the
 * opponent's greedy (level 1, value only) reply. Ties go to the more aggressive move (HexChooseBest). Returns FALSE if there is
 * no legal move.
 */
static bool32 HexPickMove(const struct HexGame *g, const struct HexTileType *types, u8 level, u32 seed, u8 *outHand,
			  u8 *outCell)
{
	u8 mover = g->turn;
	s16 val[HEX_HAND][HEX_CELLS];
	s32 bestVal;
	u8 hi;
	u8 c;

	if (level < 2) {
		s32 delta = 0;

		return HexPickGreedy(g, types, seed, 1, outHand, outCell, &delta);
	}
	/* One copy and one reply search per type and cell (HexFirstSlot). */
	for (hi = 0; hi < HEX_HAND; hi++) {
		if (g->hand[mover][hi] == HEX_USED || HexFirstSlot(g, mover, hi) != hi)
			continue;
		for (c = 0; c < HEX_CELLS; c++) {
			struct HexGame sim;
			s32 v;
			s32 replyDelta = 0;
			u8 replyHand;
			u8 replyCell;

			if (g->cell[c].owner != HEX_EMPTY)
				continue;
			sim = *g;
			Hex_Place(&sim, types, hi, c);
			v = HexEval(&sim, mover);
			/* The opponent's greedy reply changes (mover - other) by -replyDelta (delta is from the replier's view). */
			if (!Hex_IsOver(&sim) && HexPickGreedy(&sim, types, 0, 0, &replyHand, &replyCell, &replyDelta))
				v -= replyDelta;
			val[hi][c] = (s16)v;
		}
	}
	return HexChooseBest(g, types, seed, 1, val, outHand, outCell, &bestVal);
}

void Hex_Init(struct HexGame *g, const u8 *blueHand, const u8 *redHand, u8 first)
{
	u8 i;
	u8 c;

	for (c = 0; c < HEX_CELLS; c++) {
		g->cell[c].type = 0;
		g->cell[c].owner = HEX_EMPTY;
		g->cell[c].hp = 0;
	}
	for (i = 0; i < HEX_HAND; i++) {
		g->hand[0][i] = HEX_USED;
		g->hand[HEX_BLUE][i] = blueHand[i];
		g->hand[HEX_RED][i] = redHand[i];
	}
	g->turn = first;
	g->placed = 0;
	g->lastCell = HEX_NONE;
	g->lastHit = 0;
	g->lastCounter = 0;
	g->lastFlip = 0;
}

s8 Hex_Neighbor(u8 cell, u8 dir)
{
	if (cell >= HEX_CELLS || dir >= HEX_DIRS)
		return -1;
	return sHexNeighbor[cell][dir];
}

bool32 Hex_IsLegal(const struct HexGame *g, u8 handIndex, u8 cell)
{
	if (Hex_IsOver(g) || handIndex >= HEX_HAND || cell >= HEX_CELLS)
		return 0;
	if (g->hand[g->turn][handIndex] == HEX_USED)
		return 0;
	return g->cell[cell].owner == HEX_EMPTY;
}

u32 Hex_Place(struct HexGame *g, const struct HexTileType *types, u8 handIndex, u8 cell)
{
	u8 owner;
	u8 other;
	u8 type;
	const struct HexTileType *tt;
	u16 hit = 0;
	u16 counter = 0;
	u16 flip = 0;
	u8 d;
	u8 i;
	s8 n;

	if (!Hex_IsLegal(g, handIndex, cell))
		return 0;

	owner = g->turn;
	other = (u8)(3 - owner);
	type = g->hand[owner][handIndex];
	tt = &types[type];

	g->hand[owner][handIndex] = HEX_USED;
	g->cell[cell].type = type;
	g->cell[cell].owner = owner;
	g->cell[cell].hp = (s8)tt->maxHp;

	/* 1. Attack: enemy tiles in the reach directions lose the attack. */
	for (d = 0; d < HEX_DIRS; d++) {
		if (!(tt->reach & (1 << d)))
			continue;
		n = Hex_Neighbor(cell, d);
		if (n < 0 || g->cell[n].owner != other)
			continue;
		g->cell[n].hp = (s8)(g->cell[n].hp - tt->attack);
		hit |= (u16)(1 << n);
	}

	/*
	 * 2. Counterattack: an enemy neighbour whose reach points back at the placed cell hurts the placed tile. Counts
	 * neighbours at 0 HP or below that are not flipped yet.
	 */
	for (d = 0; d < HEX_DIRS; d++) {
		u8 back = HexOpposite(d);
		const struct HexTileType *nt;

		n = Hex_Neighbor(cell, d);
		if (n < 0 || g->cell[n].owner != other)
			continue;
		nt = &types[g->cell[n].type];
		if (!(nt->reach & (1 << back)))
			continue;
		g->cell[cell].hp = (s8)(g->cell[cell].hp - nt->attack);
		counter |= (u16)(1 << n);
	}

	/* 3. Flip: every tile with hp <= 0 changes owner and is restored to its maxHp. */
	for (i = 0; i < HEX_CELLS; i++) {
		struct HexCell *hc = &g->cell[i];

		if (hc->owner == HEX_EMPTY || hc->hp > 0)
			continue;
		hc->owner = (u8)(3 - hc->owner);
		hc->hp = (s8)types[hc->type].maxHp;
		flip |= (u16)(1 << i);
	}

	g->turn = other;
	g->placed++;
	g->lastCell = cell;
	g->lastHit = hit;
	g->lastCounter = counter;
	g->lastFlip = flip;
	return flip;
}

u32 Hex_Score(const struct HexGame *g, u8 owner)
{
	u32 score = 0;
	u8 i;

	if (owner != HEX_BLUE && owner != HEX_RED)
		return 0;
	for (i = 0; i < HEX_CELLS; i++) {
		if (g->cell[i].owner == owner)
			score++;
	}
	for (i = 0; i < HEX_HAND; i++) {
		if (g->hand[owner][i] != HEX_USED)
			score++;
	}
	return score;
}

bool32 Hex_IsOver(const struct HexGame *g)
{
	return g->placed >= HEX_CELLS;
}

u8 Hex_Winner(const struct HexGame *g)
{
	u32 blue = Hex_Score(g, HEX_BLUE);
	u32 red = Hex_Score(g, HEX_RED);

	if (blue > red)
		return HEX_BLUE;
	if (red > blue)
		return HEX_RED;
	return 0;
}

void Hex_AiChoose(const struct HexGame *g, const struct HexTileType *types, u8 level, u32 seed, u8 *outHandIndex,
		  u8 *outCell)
{
	u8 hi;
	u8 c;
	u32 count = 0;
	u32 pick;
	u32 seen = 0;
	u32 r;

	*outHandIndex = HEX_NONE;
	*outCell = HEX_NONE;
	if (Hex_IsOver(g))
		return;

	if (level == 0) {
		/* Random legal move: count the legal moves, then take the one at index (r * count) >> 16. */
		for (hi = 0; hi < HEX_HAND; hi++) {
			for (c = 0; c < HEX_CELLS; c++) {
				if (Hex_IsLegal(g, hi, c))
					count++;
			}
		}
		if (count == 0)
			return;
		r = HexHash(seed) & 0xFFFF;
		pick = (r * count) >> 16;
		for (hi = 0; hi < HEX_HAND; hi++) {
			for (c = 0; c < HEX_CELLS; c++) {
				if (!Hex_IsLegal(g, hi, c))
					continue;
				if (seen == pick) {
					*outHandIndex = hi;
					*outCell = c;
					return;
				}
				seen++;
			}
		}
		return;
	}

	if (level > 2)
		level = 2;
	HexPickMove(g, types, level, seed, outHandIndex, outCell);
}
