/*
 * Host tests for the Hex rules engine and AI (Kernel/Wizardry/Minigames/Hex/Source/HexCore.c, include/kernel/hex-core.h).
 *
 * Built and run by Tools/hex/run_hex_tests.sh:
 *   Tools/hex/run_hex_tests.sh (gcc -DHEX_HOST with Tools/hex/shim and include/kernel on the include path)
 * Exit status 0 when every check passes, 1 otherwise.
 */
#include <stdio.h>
#include <string.h>
#include "hex-core.h"

static int sChecks = 0;
static int sFailures = 0;

#define CHECK(cond)                                                                 \
    do {                                                                            \
        sChecks++;                                                                  \
        if (!(cond)) {                                                              \
            sFailures++;                                                            \
            printf("  FAIL line %d: %s\n", __LINE__, #cond);                        \
        }                                                                           \
    } while (0)

/* Test tile types. The Wolf's reach is up-right (1), right (2), down-left (4): 2 | 4 | 16 = 22. */
enum { T_WOLF = 0, T_KOBOLD = 1, T_TANK = 2, T_SPRITE = 3, T_BRUTE = 4, T_COUNT = 5 };
static const struct HexTileType sTypes[T_COUNT] = {
    { 1, 2, 22 }, /* Wolf: attack 1, maxHp 2, reach up-right | right | down-left */
    { 3, 1, 32 }, /* Kobold: attack 3, maxHp 1, reach left */
    { 1, 5, 32 }, /* Tank: attack 1, maxHp 5, reach left */
    { 1, 3, 4 },  /* Sprite: attack 1, maxHp 3, reach right */
    { 2, 9, 0 },  /* Brute: attack 2, maxHp 9, no reach */
};

/* Independent geometry for test 1: row, half-cell x and the neighbour directions (same axial layout as the header). */
static const int kRow[HEX_CELLS] = { 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4 };
static const int kX[HEX_CELLS] = { -1, 1, -2, 0, 2, -3, -1, 1, 3, -2, 0, 2, -1, 1 };
static const int kDr[HEX_DIRS] = { -1, -1, 0, 1, 1, 0 };
static const int kDx[HEX_DIRS] = { -1, 1, 2, 1, -1, -2 };
static const char* kDirName[HEX_DIRS] = { "UL", "UR", "R", "DR", "DL", "L" };

static u32 sRng = 1;
static u32 Rng(void)
{
    sRng = sRng * 1103515245u + 12345u;
    return (sRng >> 8) & 0xFFFFFFu;
}

/* Expected neighbour from the coordinates, -1 if no cell there. */
static int ExpectedNeighbor(int cell, int d)
{
    int i;
    for (i = 0; i < HEX_CELLS; i++) {
        if (kRow[i] == kRow[cell] + kDr[d] && kX[i] == kX[cell] + kDx[d]) {
            return i;
        }
    }
    return -1;
}

/* A move is legal if the slot holds a tile, the cell is empty and the game goes on. Checked without the engine. */
static int LegalMove(const struct HexGame* g, int hi, int c)
{
    if (hi < 0 || hi >= HEX_HAND || c < 0 || c >= HEX_CELLS) {
        return 0;
    }
    if (g->placed >= HEX_CELLS) {
        return 0;
    }
    if (g->hand[g->turn][hi] == HEX_USED) {
        return 0;
    }
    return g->cell[c].owner == HEX_EMPTY;
}

static void TestNeighbourTable(void)
{
    int c;
    int d;
    int rowCount[5] = { 0, 0, 0, 0, 0 };
    int expectedRows[5] = { 2, 3, 4, 3, 2 };
    int centre[4] = { 3, 6, 7, 10 };
    int i;

    printf("1. neighbour table\n");
    for (c = 0; c < HEX_CELLS; c++) {
        int n = 0;
        printf("  %02d:", c);
        for (d = 0; d < HEX_DIRS; d++) {
            int nb = Hex_Neighbor((u8)c, (u8)d);
            printf(" %s=%s", kDirName[d], nb < 0 ? "--" : "");
            if (nb >= 0) {
                printf("%02d", nb);
                n++;
            }
            CHECK(nb == ExpectedNeighbor(c, d));
            CHECK(nb < 0 || Hex_Neighbor((u8)nb, (u8)((d + 3) % 6)) == c); /* symmetry */
        }
        printf("  (%d neighbours)\n", n);
        rowCount[kRow[c]]++;
    }
    for (i = 0; i < 5; i++) {
        CHECK(rowCount[i] == expectedRows[i]);
    }
    for (i = 0; i < 4; i++) {
        int n = 0;
        for (d = 0; d < HEX_DIRS; d++) {
            if (Hex_Neighbor((u8)centre[i], (u8)d) >= 0) {
                n++;
            }
        }
        CHECK(n == 6);
    }
    CHECK(Hex_Neighbor(HEX_CELLS, 0) == -1);
    CHECK(Hex_Neighbor(0, HEX_DIRS) == -1);
    printf("  rows: %d %d %d %d %d\n", rowCount[0], rowCount[1], rowCount[2], rowCount[3], rowCount[4]);
}

static void TestWorkedExample(void)
{
    struct HexGame g;
    u8 blue[HEX_HAND] = { T_WOLF, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD };
    u8 red[HEX_HAND] = { T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD, T_KOBOLD };
    u32 flips;

    printf("2. worked example (Wolf on 03 by Blue, Kobold on 04 by Red)\n");
    Hex_Init(&g, blue, red, HEX_BLUE);
    flips = Hex_Place(&g, sTypes, 0, 3);
    CHECK(flips == 0);
    CHECK(g.cell[3].owner == HEX_BLUE && g.cell[3].hp == 2);

    flips = Hex_Place(&g, sTypes, 0, 4);
    CHECK(flips == ((1u << 3) | (1u << 4)));
    CHECK(g.lastFlip == flips);
    CHECK(g.lastHit == (1u << 3));     /* Kobold attacks the Wolf */
    CHECK(g.lastCounter == (1u << 3)); /* Wolf counterattacks the Kobold */
    CHECK(g.cell[3].type == T_WOLF && g.cell[3].owner == HEX_RED && g.cell[3].hp == 2);
    CHECK(g.cell[4].type == T_KOBOLD && g.cell[4].owner == HEX_BLUE && g.cell[4].hp == 1);
    CHECK(Hex_Score(&g, HEX_BLUE) == 7);
    CHECK(Hex_Score(&g, HEX_RED) == 7);
    CHECK(g.turn == HEX_BLUE && g.placed == 2);
    printf("  Wolf 03: owner %d hp %d | Kobold 04: owner %d hp %d | flips 0x%02X\n", g.cell[3].owner, g.cell[3].hp,
        g.cell[4].owner, g.cell[4].hp, (unsigned)flips);
}

static void TestCounterFromZeroHp(void)
{
    struct HexGame g;
    u8 blue[HEX_HAND] = { T_BRUTE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE };
    u8 red[HEX_HAND] = { T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE };
    u32 flips;

    printf("3. counterattack from a 0 HP tile that is not flipped yet\n");
    Hex_Init(&g, blue, red, HEX_BLUE);
    /* Craft: a Red Kobold (attack 3, reach left) on 04 at 0 HP, still owned by Red. */
    g.cell[4].type = T_KOBOLD;
    g.cell[4].owner = HEX_RED;
    g.cell[4].hp = 0;

    flips = Hex_Place(&g, sTypes, 0, 3); /* Blue Brute (maxHp 9) on 03, next to the 0 HP Kobold */
    CHECK(g.lastCounter == (1u << 4));   /* the 0 HP Kobold still counterattacks */
    CHECK(g.cell[3].owner == HEX_BLUE && g.cell[3].hp == 9 - 3);
    CHECK(flips == (1u << 4));           /* and it flips at the end of the placement */
    CHECK(g.cell[4].owner == HEX_BLUE && g.cell[4].hp == 1);
    printf("  Brute 03 hp %d (9 - 3), Kobold 04 flipped to owner %d hp %d\n", g.cell[3].hp, g.cell[4].owner, g.cell[4].hp);
}

static void TestDamagePersists(void)
{
    struct HexGame g;
    u8 blue[HEX_HAND] = { T_WOLF, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE };
    u8 red[HEX_HAND] = { T_TANK, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE, T_SPRITE };
    u32 flips;

    printf("4. partial damage persists, a flipped tile resets to maxHp\n");
    Hex_Init(&g, blue, red, HEX_BLUE);
    Hex_Place(&g, sTypes, 0, 3); /* Blue Wolf on 03 */
    flips = Hex_Place(&g, sTypes, 0, 4); /* Red Tank (attack 1) on 04 hits the Wolf: 2 -> 1; the Wolf hits back: 5 -> 4 */
    CHECK(flips == 0);
    CHECK(g.cell[3].owner == HEX_BLUE && g.cell[3].hp == 1);
    CHECK(g.cell[4].owner == HEX_RED && g.cell[4].hp == 4);

    flips = Hex_Place(&g, sTypes, 1, 0); /* Blue Sprite on 00, not touching anything */
    CHECK(flips == 0);
    CHECK(g.cell[3].owner == HEX_BLUE && g.cell[3].hp == 1); /* still damaged a turn later */
    CHECK(g.cell[4].owner == HEX_RED && g.cell[4].hp == 4);

    flips = Hex_Place(&g, sTypes, 1, 2); /* Red Sprite on 02 (reach right): Wolf 1 -> 0, flips */
    CHECK(flips == (1u << 3));
    CHECK(g.cell[3].owner == HEX_RED && g.cell[3].hp == 2); /* reset to maxHp */
    CHECK(g.lastHit == (1u << 3));
    CHECK(g.lastCounter == 0);

    /* An illegal move (cell occupied) changes nothing. */
    {
        u8 placedBefore = g.placed;
        CHECK(Hex_Place(&g, sTypes, 2, 3) == 0); /* unused Blue slot, occupied cell */
        CHECK(g.placed == placedBefore);
    }
    printf("  Wolf 03 owner %d hp %d (after flip), Tank 04 hp %d\n", g.cell[3].owner, g.cell[3].hp, g.cell[4].hp);
}

/* Random tile table for the full games. */
static void RandomTypes(struct HexTileType* t, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        t[i].attack = (u8)(1 + Rng() % 9);
        t[i].maxHp = (u8)(1 + Rng() % 9);
        t[i].reach = (u8)(Rng() % 64);
    }
}

static void RandomHand(u8* h, int ntypes)
{
    int i;
    for (i = 0; i < HEX_HAND; i++) {
        h[i] = (u8)(Rng() % ntypes);
    }
}

/* Plays one full game with random hands and first player (from gameSeed). blueLevel / redLevel are the AI levels of the two
 * sides. Sets *illegal when the AI returned a move that is not legal. */
static void PlayGame(struct HexGame* g, const struct HexTileType* types, int ntypes, u32 gameSeed, u8 blueLevel, u8 redLevel, int* illegal)
{
    u8 blue[HEX_HAND];
    u8 red[HEX_HAND];
    u8 first;
    *illegal = 0;

    RandomHand(blue, ntypes);
    RandomHand(red, ntypes);
    first = (gameSeed & 2) ? HEX_RED : HEX_BLUE;
    Hex_Init(g, blue, red, first);
    while (!Hex_IsOver(g)) {
        u8 side = g->turn;
        u8 level = (side == HEX_BLUE) ? blueLevel : redLevel;
        u8 hi;
        u8 c;
        u32 seed = gameSeed * 1000u + g->placed;
        Hex_AiChoose(g, types, level, seed, &hi, &c);
        if (!LegalMove(g, hi, c)) {
            *illegal = 1;
            return;
        }
        Hex_Place(g, types, hi, c);
    }
}

static void TestFullRandomGames(void)
{
    int game;
    int blueWins = 0;
    int redWins = 0;
    int draws = 0;
    int bad = 0;
    int illegal;
    struct HexTileType types[8];
    struct HexGame g;

    printf("5. full random games, level 1 (Blue) vs level 0 (Red), 14 moves each\n");
    sRng = 2024;
    for (game = 0; game < 50; game++) {
        int cell;
        int sum;
        int sb;
        int sr;
        u8 expected;
        RandomTypes(types, 8);
        PlayGame(&g, types, 8, (u32)game + 1, 1, 0, &illegal);
        if (illegal) {
            bad++;
            continue;
        }
        CHECK(g.placed == HEX_CELLS);
        CHECK(Hex_IsOver(&g));
        for (cell = 0; cell < HEX_CELLS; cell++) {
            CHECK(g.cell[cell].owner == HEX_BLUE || g.cell[cell].owner == HEX_RED);
        }
        sb = (int)Hex_Score(&g, HEX_BLUE);
        sr = (int)Hex_Score(&g, HEX_RED);
        sum = sb + sr;
        CHECK(sum == HEX_CELLS);
        expected = (u8)(sb > sr ? HEX_BLUE : (sr > sb ? HEX_RED : 0));
        CHECK(Hex_Winner(&g) == expected);
        if (Hex_Winner(&g) == HEX_BLUE) {
            blueWins++;
        } else if (Hex_Winner(&g) == HEX_RED) {
            redWins++;
        } else {
            draws++;
        }
    }
    CHECK(bad == 0);
    printf("  50 games: blue wins %d, red wins %d, draws %d, illegal moves %d\n", blueWins, redWins, draws, bad);
}

static void TestAiLevel2(void)
{
    int game;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int bad = 0;
    int illegal;
    struct HexTileType types[8];
    struct HexGame g;

    printf("6. AI level 2 vs level 0, 200 seeded games (level 2 alternates sides)\n");
    sRng = 777;
    for (game = 0; game < 200; game++) {
        u8 l2Side = (game & 1) ? HEX_RED : HEX_BLUE;
        u8 winner;
        RandomTypes(types, 8);
        PlayGame(&g, types, 8, (u32)game * 7u + 3u, (l2Side == HEX_BLUE) ? 2 : 0, (l2Side == HEX_RED) ? 2 : 0, &illegal);
        if (illegal) {
            bad++;
            continue;
        }
        CHECK(g.placed == HEX_CELLS);
        winner = Hex_Winner(&g);
        if (winner == 0) {
            draws++;
        } else if (winner == l2Side) {
            wins++;
        } else {
            losses++;
        }
    }
    CHECK(bad == 0);
    printf("  level 2 wins %d, draws %d, losses %d, illegal moves %d -> win rate %d.%d%%\n", wins, draws, losses, bad,
        (wins * 100) / 200, ((wins * 1000) / 200) % 10);
    CHECK(wins * 100 >= 70 * 200);
}

/* ---- Reference (the level 1 / level 2 search as it was before the delta evaluation, copied from hexCore.c) ----------------
 * Used to check that the AI still picks a move of the best reference value. Hash and key are copies of hexCore.c's. */
static u32 RefHexHash(u32 x)
{
    x ^= x >> 16;
    x *= 0x7FEB352Du;
    x ^= x >> 15;
    x *= 0x846CA68Bu;
    x ^= x >> 16;
    return x;
}

static u32 RefHexMoveKey(u32 seed, u32 move)
{
    return RefHexHash(seed + move * 0x9E3779B1u);
}

static s32 RefEval(const struct HexGame* g, u8 me)
{
    return (s32)Hex_Score(g, me) - (s32)Hex_Score(g, (u8)(3 - me));
}

static bool32 RefPickMove(const struct HexGame* g, const struct HexTileType* types, u8 level, u32 seed, u8* outHand, u8* outCell)
{
    u8 mover = g->turn;
    bool32 found = 0;
    s32 bestVal = 0;
    u32 bestKey = 0;
    u8 hi;
    u8 c;

    for (hi = 0; hi < HEX_HAND; hi++) {
        if (g->hand[mover][hi] == HEX_USED) {
            continue;
        }
        for (c = 0; c < HEX_CELLS; c++) {
            struct HexGame sim;
            s32 val;
            u32 move;
            u32 key;
            u8 replyHand;
            u8 replyCell;

            if (g->cell[c].owner != HEX_EMPTY) {
                continue;
            }
            move = (u32)hi * HEX_CELLS + c;
            key = RefHexMoveKey(seed, move);
            sim = *g;
            Hex_Place(&sim, types, hi, c);
            if (level >= 2 && !Hex_IsOver(&sim)) {
                if (RefPickMove(&sim, types, 1, RefHexHash(key + 1), &replyHand, &replyCell)) {
                    Hex_Place(&sim, types, replyHand, replyCell);
                }
            }
            val = RefEval(&sim, mover);
            if (!found || val > bestVal || (val == bestVal && key > bestKey)) {
                found = 1;
                bestVal = val;
                bestKey = key;
                *outHand = hi;
                *outCell = c;
            }
        }
    }
    return found;
}

/* The value the reference gives a move: the difference for the mover after the move, and at level 2 after the opponent's
 * greedy reply (the reply's value does not depend on its tie-break, so the seed is fixed). */
static s32 RefMoveValue(const struct HexGame* g, const struct HexTileType* types, u8 level, u8 hi, u8 c)
{
    struct HexGame sim = *g;
    u8 mover = g->turn;

    Hex_Place(&sim, types, hi, c);
    if (level >= 2 && !Hex_IsOver(&sim)) {
        u8 replyHand;
        u8 replyCell;
        if (RefPickMove(&sim, types, 1, 12345u, &replyHand, &replyCell)) {
            Hex_Place(&sim, types, replyHand, replyCell);
        }
    }
    return RefEval(&sim, mover);
}

/* Legal moves of g->turn into hi[] / c[] (room for 98). Returns the count. */
static int CollectMoves(const struct HexGame* g, u8* hi, u8* c)
{
    int n = 0;
    int h;
    int k;
    for (h = 0; h < HEX_HAND; h++) {
        for (k = 0; k < HEX_CELLS; k++) {
            if (LegalMove(g, h, k)) {
                hi[n] = (u8)h;
                c[n] = (u8)k;
                n++;
            }
        }
    }
    return n;
}

/* A random position with `placed` tiles on the board, made by random legal moves. */
static void RandomPosition(struct HexGame* g, const struct HexTileType* types, int ntypes, int placed)
{
    u8 blue[HEX_HAND];
    u8 red[HEX_HAND];
    u8 mh[98];
    u8 mc[98];
    int n;

    RandomHand(blue, ntypes);
    RandomHand(red, ntypes);
    Hex_Init(g, blue, red, (Rng() & 1) ? HEX_RED : HEX_BLUE);
    for (n = 0; n < placed; n++) {
        int count = CollectMoves(g, mh, mc);
        int pick = (int)(Rng() % (u32)count);
        Hex_Place(g, types, mh[pick], mc[pick]);
    }
}

static s32 Eval(const struct HexGame* g, u8 me)
{
    return (s32)Hex_Score(g, me) - (s32)Hex_Score(g, (u8)(3 - me));
}

static void TestPlaceDelta(void)
{
    int pos;
    int checks = 0;
    int dirtyChecks = 0;
    struct HexTileType types[8];
    struct HexGame g;

    printf("7. Hex_PlaceDelta equals the real change of (mover - other) after Hex_Place, every legal move\n");
    sRng = 4242;
    for (pos = 0; pos < 2000; pos++) {
        int placed = (int)(Rng() % 14);
        int count;
        int m;
        u8 mh[98];
        u8 mc[98];
        u8 mover;
        RandomTypes(types, 8);
        RandomPosition(&g, types, 8, placed);
        mover = g.turn;
        count = CollectMoves(&g, mh, mc);
        for (m = 0; m < count; m++) {
            struct HexGame sim = g;
            s32 want;
            s32 got;
            Hex_Place(&sim, types, mh[m], mc[m]);
            want = Eval(&sim, mover) - Eval(&g, mover);
            got = Hex_PlaceDelta(&g, types, g.hand[mover][mh[m]], mc[m], mover);
            CHECK(got == want);
            CHECK(Hex_PlaceDeltaSlow(&g, types, g.hand[mover][mh[m]], mc[m], mover) == want);
            checks++;
        }
    }
    /* Positions the game never reaches (a tile at 0 HP or below that is still owned) must agree too: Hex_Place flips them. */
    for (pos = 0; pos < 500; pos++) {
        int placed = (int)(Rng() % 12);
        int count;
        int m;
        int cell;
        u8 mh[98];
        u8 mc[98];
        u8 mover;
        RandomTypes(types, 8);
        RandomPosition(&g, types, 8, placed);
        for (cell = 0; cell < HEX_CELLS; cell++) {
            if (g.cell[cell].owner != HEX_EMPTY && Rng() % 4 == 0) {
                g.cell[cell].hp = (s8)((int)(Rng() % 5) - 3);
            }
        }
        mover = g.turn;
        count = CollectMoves(&g, mh, mc);
        for (m = 0; m < count; m++) {
            struct HexGame sim = g;
            Hex_Place(&sim, types, mh[m], mc[m]);
            CHECK(Hex_PlaceDelta(&g, types, g.hand[mover][mh[m]], mc[m], mover) == Eval(&sim, mover) - Eval(&g, mover));
            CHECK(Hex_PlaceDeltaSlow(&g, types, g.hand[mover][mh[m]], mc[m], mover) == Eval(&sim, mover) - Eval(&g, mover));
            dirtyChecks++;
        }
    }
    printf("  %d random mid-game positions: %d moves checked; %d moves in 500 positions with owned 0-or-less HP tiles\n", 2000,
        checks, dirtyChecks);
}

static void TestAiAgainstReference(void)
{
    int pos;
    int worse = 0;
    int moveDiffers = 0;
    int level;
    struct HexTileType types[8];
    struct HexGame g;

    printf("8. AI picks a move of the reference's best value (levels 1 and 2, 300 random positions)\n");
    sRng = 9001;
    for (pos = 0; pos < 300; pos++) {
        int placed = (int)(Rng() % 14);
        u8 mh[98];
        u8 mc[98];
        RandomTypes(types, 8);
        RandomPosition(&g, types, 8, placed);
        for (level = 1; level <= 2; level++) {
            int count = CollectMoves(&g, mh, mc);
            s32 best = 0;
            s32 refVal;
            u8 aiHand;
            u8 aiCell;
            s32 aiVal;
            u8 rh;
            u8 rc;
            u32 seed;
            int m;
            for (m = 0; m < count; m++) {
                s32 v = RefMoveValue(&g, types, (u8)level, mh[m], mc[m]);
                if (m == 0 || v > best) {
                    best = v;
                }
            }
            /* The reference's own pick must reach the same best value (checks RefPickMove itself). */
            RefPickMove(&g, types, (u8)level, 999u + (u32)pos, &rh, &rc);
            refVal = RefMoveValue(&g, types, (u8)level, rh, rc);
            CHECK(refVal == best);

            seed = Rng();
            Hex_AiChoose(&g, types, (u8)level, seed, &aiHand, &aiCell);
            CHECK(LegalMove(&g, aiHand, aiCell));
            aiVal = RefMoveValue(&g, types, (u8)level, aiHand, aiCell);
            if (aiVal < best) {
                worse++;
            }
            /* Same seed, same tie-break keys: the reference's pick for this seed is the same move. */
            RefPickMove(&g, types, (u8)level, seed, &rh, &rc);
            if (aiHand != rh || aiCell != rc) {
                moveDiffers++;
            }
            CHECK(aiVal == best);
            CHECK(aiHand == rh && aiCell == rc);
        }
    }
    printf("  300 positions x 2 levels: worse than the reference %d, different move from the reference pick (same seed) %d\n", worse,
        moveDiffers);
}

/* Plays one seeded game with both sides on the new AI (useRef 0) or both on the reference (useRef 1), returns the final board. */
static void PlaySeeded(struct HexGame* g, const struct HexTileType* types, int game, u8 level, int useRef)
{
    u8 blue[HEX_HAND];
    u8 red[HEX_HAND];
    sRng = 5000u + (u32)game * 13u;
    RandomHand(blue, 8);
    RandomHand(red, 8);
    Hex_Init(g, blue, red, (game & 2) ? HEX_RED : HEX_BLUE);
    while (!Hex_IsOver(g)) {
        u8 hi;
        u8 c;
        u32 seed = (u32)game * 1000u + g->placed;
        if (useRef) {
            RefPickMove(g, types, level, seed, &hi, &c);
        } else {
            Hex_AiChoose(g, types, level, seed, &hi, &c);
        }
        Hex_Place(g, types, hi, c);
    }
}

static void TestSameAsReference(void)
{
    int game;
    int level;
    int sameGames = 0;
    int games = 0;
    int blueWins = 0;
    struct HexTileType types[8];

    printf("9. the new AI plays the same games as the reference (levels 1 and 2, 300 seeded games each)\n");
    for (level = 1; level <= 2; level++) {
        for (game = 0; game < 300; game++) {
            struct HexGame a;
            struct HexGame b;
            sRng = 77u + (u32)game;
            RandomTypes(types, 8);
            PlaySeeded(&a, types, game, (u8)level, 0);
            PlaySeeded(&b, types, game, (u8)level, 1);
            CHECK(memcmp(&a, &b, sizeof(struct HexGame)) == 0);
            if (memcmp(&a, &b, sizeof(struct HexGame)) == 0) {
                sameGames++;
            }
            games++;
            if (Hex_Winner(&a) == HEX_BLUE) {
                blueWins++;
            }
        }
    }
    printf("  %d of %d games identical to the reference (blue won %d of the new-AI games)\n", sameGames, games, blueWins);
}

int main(void)
{
    TestNeighbourTable();
    TestWorkedExample();
    TestCounterFromZeroHp();
    TestDamagePersists();
    TestFullRandomGames();
    TestAiLevel2();
    TestPlaceDelta();
    TestAiAgainstReference();
    TestSameAsReference();

    printf("%d checks, %d failed\n", sChecks, sFailures);
    if (sFailures != 0) {
        printf("FAILED\n");
        return 1;
    }
    printf("ALL HEX TESTS PASSED\n");
    return 0;
}
