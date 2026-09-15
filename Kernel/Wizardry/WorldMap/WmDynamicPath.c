#include "common-chax.h"
#include "WmDynamicPath.h"

#pragma GCC optimize ("Os")
#pragma GCC optimize ("no-jump-tables")

#define WM_ROUTE_W 60
#define WM_ROUTE_H 40
#define WM_MAX_PTS 48
#define WM_OCC_BYTES ((WM_ROUTE_W * WM_ROUTE_H + 7) / 8)

#define WM_HFLIP 0x0400
#define WM_VFLIP 0x0800

#define WM_T_HTOP 1
#define WM_T_HBOT 2
#define WM_T_VLEFT 3
#define WM_T_VRIGHT 4
#define WM_T_CORNER 5
#define WM_T_DIAG_A 6
#define WM_T_DIAG_B 7

#define WM_MAX_SPLINE 8

enum
{
    WM_DIR_H,
    WM_DIR_V,
    WM_DIR_DR,
    WM_DIR_DL,
};

enum
{
    WM_SH_DIAG,
    WM_SH_HJOG,
    WM_SH_VJOG,
    WM_SH_MIDHV,
    WM_SH_MIDVH,
    WM_SH_H45,
    WM_SH_V45,
};

struct WmPt
{
    s16 x;
    s16 y;
};

s16 GetWmNodeX(int nodeId)
{
    if (nodeId < 0 || nodeId >= NODE_MAX)
        return 0;

    return gWMNodeData[nodeId].x;
}

s16 GetWmNodeY(int nodeId)
{
    if (nodeId < 0 || nodeId >= NODE_MAX)
        return 0;

    return gWMNodeData[nodeId].y;
}

static int WmAbs(int v)
{
    return v < 0 ? -v : v;
}

static int WmSign(int v)
{
    if (v > 0)
        return 1;

    if (v < 0)
        return -1;

    return 0;
}

static int WmPixelToTile(int p)
{
    /* Magvel gfx uses floor(p/8). Serafew Y=183 → 22, matching Path 4/5. */
    return p >> 3;
}

static int WmIsSteep(int adx, int ady)
{
    int lo = adx < ady ? adx : ady;
    int hi = adx > ady ? adx : ady;

    return lo * 2 >= hi;
}

static void WmOccClear(u8 * occ)
{
    int i;

    for (i = 0; i < WM_OCC_BYTES; i++)
        occ[i] = 0;
}

static int WmOccTest(u8 * occ, int x, int y)
{
    int i;

    if (x < 0 || y < 0 || x >= WM_ROUTE_W || y >= WM_ROUTE_H)
        return 0;

    i = y * WM_ROUTE_W + x;
    return (occ[i >> 3] >> (i & 7)) & 1;
}

static void WmOccMark(u8 * occ, int x, int y)
{
    int i;

    if (x < 0 || y < 0 || x >= WM_ROUTE_W || y >= WM_ROUTE_H)
        return;

    i = y * WM_ROUTE_W + x;
    occ[i >> 3] |= (u8)(1 << (i & 7));
}

static void WmPutRouteTile(u16 * map, int stride, int x, int y, u16 tile, u16 oam2)
{
    if (x < 0 || y < 0 || x >= WM_ROUTE_W || y >= WM_ROUTE_H)
        return;

    if (map[y * stride + x] != 0)
        return;

    map[y * stride + x] = tile + oam2;
}

static void WmPutRouteTileForce(u16 * map, int stride, int x, int y, u16 tile, u16 oam2)
{
    if (x < 0 || y < 0 || x >= WM_ROUTE_W || y >= WM_ROUTE_H)
        return;

    map[y * stride + x] = tile + oam2;
}

static void WmPaintH(u16 * map, int stride, int x, int y, u16 oam2)
{
    WmPutRouteTile(map, stride, x, y - 1, WM_T_HTOP, oam2);
    WmPutRouteTile(map, stride, x, y, WM_T_HBOT, oam2);
}

static void WmPaintV(u16 * map, int stride, int x, int y, u16 oam2)
{
    WmPutRouteTile(map, stride, x - 1, y, WM_T_VLEFT, oam2);
    WmPutRouteTile(map, stride, x, y, WM_T_VRIGHT, oam2);
}

static void WmPaintDR(u16 * map, int stride, int x, int y, int continued, int paintEast, u16 oam2)
{
    /* Magvel 45° is an east pair (6,7). Later rows add a west flipped 7. */
    if (continued)
        WmPutRouteTile(map, stride, x - 1, y, WM_T_DIAG_B | WM_HFLIP | WM_VFLIP, oam2);

    WmPutRouteTile(map, stride, x, y, WM_T_DIAG_A, oam2);

    if (paintEast)
        WmPutRouteTile(map, stride, x + 1, y, WM_T_DIAG_B, oam2);
}

static void WmPaintHtoDR(u16 * map, int stride, int x, int y, u16 oam2)
{
    /* Path 0 ramp start: tile 7 on the HTOP row, then (6,7) on the H row. */
    WmPutRouteTile(map, stride, x, y - 1, WM_T_DIAG_B, oam2);
    WmPutRouteTile(map, stride, x, y, WM_T_DIAG_A, oam2);
    WmPutRouteTile(map, stride, x + 1, y, WM_T_DIAG_B, oam2);
}

static void WmPaintDL(u16 * map, int stride, int x, int y, u16 oam2)
{
    WmPutRouteTile(map, stride, x - 1, y, WM_T_DIAG_B | WM_HFLIP, oam2);
    WmPutRouteTile(map, stride, x, y, WM_T_DIAG_A | WM_HFLIP, oam2);
}

static int WmSegDir(struct WmPt a, struct WmPt b)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    if (dx != 0 && dy != 0)
        return dx > 0 ? WM_DIR_DR : WM_DIR_DL;

    if (dx != 0)
        return WM_DIR_H;

    return WM_DIR_V;
}

static void WmPaintElbow(u16 * map, int stride, int x, int y, int inDir, int inDx, int inDy, int outDx, int outDy, u16 oam2)
{
    u16 flip = 0;

    if (inDir == WM_DIR_H)
    {
        if (inDx < 0)
            flip |= WM_HFLIP;
        if (outDy < 0)
            flip |= WM_VFLIP;
    }
    else
    {
        if (outDx < 0)
            flip |= WM_HFLIP;
        if (inDy < 0)
            flip |= WM_VFLIP;
    }

    WmPutRouteTileForce(map, stride, x, y, WM_T_CORNER | flip, oam2);

    /* Fill the 2-wide pair so Magvel H/V art meets at the tile boundary. */
    if (inDir == WM_DIR_H)
    {
        WmPutRouteTile(map, stride, x, y - 1, WM_T_HTOP, oam2);
        WmPaintV(map, stride, x, y, oam2);
    }
    else
    {
        WmPutRouteTile(map, stride, x - 1, y, WM_T_VLEFT, oam2);
        WmPaintH(map, stride, x, y, oam2);
    }
}

static void WmPaintDir(u16 * map, int stride, int x, int y, int dir, u16 oam2)
{
    if (dir == WM_DIR_H)
        WmPaintH(map, stride, x, y, oam2);
    else if (dir == WM_DIR_V)
        WmPaintV(map, stride, x, y, oam2);
    else if (dir == WM_DIR_DR)
        WmPaintDR(map, stride, x, y, 0, 1, oam2);
    else if (dir == WM_DIR_DL)
        WmPaintDL(map, stride, x, y, oam2);
}

static void WmAddPt(struct WmPt * pts, int * n, int x, int y)
{
    if (*n >= WM_MAX_PTS)
        return;

    pts[*n].x = (s16)x;
    pts[*n].y = (s16)y;
    (*n)++;
}

static int WmBuildShape(int x1, int y1, int x2, int y2, int shape, int jog, struct WmPt * pts)
{
    int n = 0;
    int x;
    int y;
    int adx = WmAbs(x2 - x1);
    int ady = WmAbs(y2 - y1);
    int sx = WmSign(x2 - x1);
    int sy = WmSign(y2 - y1);
    int i;
    int diag;

    if (x1 == x2 && y1 == y2)
    {
        WmAddPt(pts, &n, x1, y1);
        return n;
    }

    if (shape == WM_SH_MIDHV)
    {
        int my = (y1 + y2) / 2;

        x = x1;
        y = y1;
        WmAddPt(pts, &n, x, y);

        while (y != my && n < WM_MAX_PTS)
        {
            y += WmSign(my - y);
            WmAddPt(pts, &n, x, y);
        }

        while (x != x2 && n < WM_MAX_PTS)
        {
            if (sx == 0)
                break;
            x += sx;
            WmAddPt(pts, &n, x, y);
        }

        while (y != y2 && n < WM_MAX_PTS)
        {
            if (sy == 0)
                break;
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    if (shape == WM_SH_MIDVH)
    {
        int mx = (x1 + x2) / 2;

        x = x1;
        y = y1;
        WmAddPt(pts, &n, x, y);

        while (x != mx && n < WM_MAX_PTS)
        {
            x += WmSign(mx - x);
            WmAddPt(pts, &n, x, y);
        }

        while (y != y2 && n < WM_MAX_PTS)
        {
            if (sy == 0)
                break;
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        while (x != x2 && n < WM_MAX_PTS)
        {
            if (sx == 0)
                break;
            x += sx;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    x = x1;
    y = y1;
    WmAddPt(pts, &n, x, y);

    if (shape == WM_SH_DIAG)
    {
        diag = adx < ady ? adx : ady;

        while (diag > 0 && n < WM_MAX_PTS)
        {
            x += sx;
            y += sy;
            diag--;
            WmAddPt(pts, &n, x, y);
        }

        while (x != x2 && n < WM_MAX_PTS)
        {
            if (sx == 0)
                break;
            x += sx;
            WmAddPt(pts, &n, x, y);
        }

        while (y != y2 && n < WM_MAX_PTS)
        {
            if (sy == 0)
                break;
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    if (shape == WM_SH_H45)
    {
        int wx = x1;
        int wy = y1;
        int ex = x2;
        int ey = y2;
        int extra;
        int head;

        if (x1 > x2)
        {
            wx = x2;
            wy = y2;
            ex = x1;
            ey = y1;
        }

        extra = WmAbs(ex - wx) - WmAbs(ey - wy);
        if (extra < 0)
            extra = 0;
        head = extra / 2;

        n = 0;
        x = wx;
        y = wy;
        sy = WmSign(ey - wy);
        WmAddPt(pts, &n, x, y);

        for (i = 0; i < head && x != ex && n < WM_MAX_PTS; i++)
        {
            x += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (y != ey && x != ex && n < WM_MAX_PTS)
        {
            x += 1;
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        while (x != ex && n < WM_MAX_PTS)
        {
            x += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (y != ey && n < WM_MAX_PTS)
        {
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    if (shape == WM_SH_V45)
    {
        int nx = x1;
        int ny = y1;
        int ex = x2;
        int ey = y2;
        int extra;
        int head;

        if (y1 > y2)
        {
            nx = x2;
            ny = y2;
            ex = x1;
            ey = y1;
        }

        extra = WmAbs(ey - ny) - WmAbs(ex - nx);
        if (extra < 0)
            extra = 0;
        head = extra / 2;

        n = 0;
        x = nx;
        y = ny;
        sx = WmSign(ex - nx);
        WmAddPt(pts, &n, x, y);

        for (i = 0; i < head && y != ey && n < WM_MAX_PTS; i++)
        {
            y += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (x != ex && y != ey && n < WM_MAX_PTS)
        {
            x += sx;
            y += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (y != ey && n < WM_MAX_PTS)
        {
            y += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (x != ex && n < WM_MAX_PTS)
        {
            x += sx;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    if (shape == WM_SH_HJOG)
    {
        int wx = x1;
        int wy = y1;
        int ex = x2;
        int ey = y2;

        /* West → east so the long run sits on the western node's row. */
        if (x1 > x2)
        {
            wx = x2;
            wy = y2;
            ex = x1;
            ey = y1;
        }

        n = 0;
        x = wx;
        y = wy;
        sy = WmSign(ey - wy);
        WmAddPt(pts, &n, x, y);

        for (i = 0; i < jog && x != ex && n < WM_MAX_PTS; i++)
        {
            x += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (y != ey && n < WM_MAX_PTS)
        {
            if (sy == 0)
                break;
            y += sy;
            WmAddPt(pts, &n, x, y);
        }

        while (x != ex && n < WM_MAX_PTS)
        {
            x += 1;
            WmAddPt(pts, &n, x, y);
        }

        return n;
    }

    {
        int nx = x1;
        int ny = y1;
        int sx2 = x2;
        int sy2 = y2;

        /* North → south so the long run sits on the northern node's column. */
        if (y1 > y2)
        {
            nx = x2;
            ny = y2;
            sx2 = x1;
            sy2 = y1;
        }

        n = 0;
        x = nx;
        y = ny;
        sx = WmSign(sx2 - nx);
        WmAddPt(pts, &n, x, y);

        for (i = 0; i < jog && y != sy2 && n < WM_MAX_PTS; i++)
        {
            y += 1;
            WmAddPt(pts, &n, x, y);
        }

        while (x != sx2 && n < WM_MAX_PTS)
        {
            if (sx == 0)
                break;
            x += sx;
            WmAddPt(pts, &n, x, y);
        }

        while (y != sy2 && n < WM_MAX_PTS)
        {
            y += 1;
            WmAddPt(pts, &n, x, y);
        }
    }

    return n;
}

static int WmIsHvBend(int a, int b)
{
    return (a == WM_DIR_H && b == WM_DIR_V) || (a == WM_DIR_V && b == WM_DIR_H);
}

static int WmChebyshev(int x1, int y1, int x2, int y2)
{
    int dx = WmAbs(x2 - x1);
    int dy = WmAbs(y2 - y1);

    return dx > dy ? dx : dy;
}

static int WmInJoinZone(struct WmPt * pts, int n, int x, int y)
{
    if (n < 1)
        return 0;

    if (WmChebyshev(x, y, pts[0].x, pts[0].y) <= 1)
        return 1;

    if (n > 1 && WmChebyshev(x, y, pts[n - 1].x, pts[n - 1].y) <= 1)
        return 1;

    return 0;
}

static int WmFootprintCells(int x, int y, int dir, int * ox, int * oy)
{
    int count = 1;

    ox[0] = x;
    oy[0] = y;

    if (dir == WM_DIR_H)
    {
        ox[1] = x;
        oy[1] = y - 1;
        count = 2;
    }
    else if (dir == WM_DIR_V)
    {
        ox[1] = x - 1;
        oy[1] = y;
        count = 2;
    }
    else if (dir == WM_DIR_DR)
    {
        ox[1] = x + 1;
        oy[1] = y;
        ox[2] = x - 1;
        oy[2] = y;
        count = 3;
    }
    else if (dir == WM_DIR_DL)
    {
        ox[1] = x - 1;
        oy[1] = y;
        count = 2;
    }

    return count;
}

static int WmFootprintHitsPath(u8 * occ, struct WmPt * pts, int n, int x, int y, int dir)
{
    int ox[3];
    int oy[3];
    int count = WmFootprintCells(x, y, dir, ox, oy);
    int i;

    for (i = 0; i < count; i++)
    {
        if (!WmInJoinZone(pts, n, ox[i], oy[i]) && WmOccTest(occ, ox[i], oy[i]))
            return 1;
    }

    return 0;
}

static int WmCenterlineClear(u8 * occ, struct WmPt * pts, int n)
{
    int i;
    int dir;

    if (n < 2)
        return 0;

    for (i = 0; i < n - 1; i++)
    {
        if (i == 0)
            dir = WmSegDir(pts[0], pts[1]);
        else
            dir = WmSegDir(pts[i - 1], pts[i]);

        if (WmFootprintHitsPath(occ, pts, n, pts[i].x, pts[i].y, dir))
            return 0;
    }

    return 1;
}

static void WmMarkCenterline(u8 * occ, struct WmPt * pts, int n)
{
    int i;
    int j;
    int dir;
    int ox[3];
    int oy[3];
    int count;

    if (n < 2)
        return;

    for (i = 0; i < n - 1; i++)
    {
        if (i == 0)
            dir = WmSegDir(pts[0], pts[1]);
        else
            dir = WmSegDir(pts[i - 1], pts[i]);

        count = WmFootprintCells(pts[i].x, pts[i].y, dir, ox, oy);

        for (j = 0; j < count; j++)
        {
            if (!WmInJoinZone(pts, n, ox[j], oy[j]))
                WmOccMark(occ, ox[j], oy[j]);
        }
    }
}

static int WmTryShape(u8 * occ, int x1, int y1, int x2, int y2, int shape, int jog, struct WmPt * pts)
{
    int n = WmBuildShape(x1, y1, x2, y2, shape, jog, pts);

    if (n < 2 || !WmCenterlineClear(occ, pts, n))
        return 0;

    return n;
}

static int WmPreferredShape(int adx, int ady)
{
    if (ady == 0)
        return WM_SH_HJOG;

    if (adx == 0)
        return WM_SH_VJOG;

    if (WmIsSteep(adx, ady))
        return WM_SH_DIAG;

    if (adx > ady)
        return WM_SH_H45;

    return WM_SH_V45;
}

static int WmPreferredJog(int shape, int adx, int ady)
{
    if (shape == WM_SH_HJOG)
        return adx;

    if (shape == WM_SH_VJOG)
        return ady;

    return 0;
}

static int WmPickCenterline(u8 * occ, int x1, int y1, int x2, int y2, struct WmPt * pts)
{
    int adx = WmAbs(x2 - x1);
    int ady = WmAbs(y2 - y1);
    int pref = WmPreferredShape(adx, ady);
    int n;
    int j;

    n = WmTryShape(occ, x1, y1, x2, y2, pref, WmPreferredJog(pref, adx, ady), pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_DIAG, 0, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_H45, 0, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_V45, 0, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_MIDHV, 0, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_MIDVH, 0, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_HJOG, adx, pts);
    if (n)
        return n;

    n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_VJOG, ady, pts);
    if (n)
        return n;

    for (j = 1; j < adx; j++)
    {
        n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_HJOG, j, pts);
        if (n)
            return n;
    }

    for (j = 1; j < ady; j++)
    {
        n = WmTryShape(occ, x1, y1, x2, y2, WM_SH_VJOG, j, pts);
        if (n)
            return n;
    }

    return WmBuildShape(x1, y1, x2, y2, pref, WmPreferredJog(pref, adx, ady), pts);
}

static void WmRasterPts(u16 * map, int stride, struct WmPt * pts, int n, u16 oam2)
{
    int i;
    int dir;
    int nd;
    int prevDir;
    int continued;
    int paintEast;

    if (n < 2)
        return;

    prevDir = -1;

    /* Skip dest when the last step is H/45° (those pairs extend east of
     * the node). Paint dest when the last step is V so vertical joins
     * meet the node the way Magvel Path 4 does. Skip the first H cell
     * so Magvel H-runs start one tile after the west node. */
    for (i = 0; i < n; i++)
    {
        if (i == 0)
            dir = WmSegDir(pts[0], pts[1]);
        else
            dir = WmSegDir(pts[i - 1], pts[i]);

        if (i == n - 1)
        {
            if (dir == WM_DIR_V)
                WmPaintV(map, stride, pts[i].x, pts[i].y, oam2);
            break;
        }

        nd = WmSegDir(pts[i], pts[i + 1]);

        if (i == 0 && dir == WM_DIR_H)
        {
            prevDir = dir;
            continue;
        }

        if (dir == WM_DIR_H && nd == WM_DIR_DR)
        {
            WmPaintHtoDR(map, stride, pts[i].x, pts[i].y, oam2);
            prevDir = WM_DIR_DR;
            continue;
        }
        else if (dir == WM_DIR_DR && nd == WM_DIR_H)
        {
            WmPutRouteTile(map, stride, pts[i].x - 1, pts[i].y, WM_T_DIAG_B | WM_HFLIP | WM_VFLIP, oam2);
            WmPaintH(map, stride, pts[i].x, pts[i].y, oam2);
        }
        else if (dir == WM_DIR_DR && nd == WM_DIR_V)
        {
            /* Magvel Path 1: 45° hands off to V one row before dest. */
            WmPaintV(map, stride, pts[i].x, pts[i].y, oam2);
        }
        else if (dir == WM_DIR_DR)
        {
            int nextNd;

            continued = prevDir == WM_DIR_DR;
            nextNd = -1;
            if (i + 2 < n)
                nextNd = WmSegDir(pts[i + 1], pts[i + 2]);
            paintEast = nd != WM_DIR_H && !(nd == WM_DIR_DR && nextNd == WM_DIR_H);
            WmPaintDR(map, stride, pts[i].x, pts[i].y, continued, paintEast, oam2);
        }
        else if (dir != nd && WmIsHvBend(dir, nd))
        {
            int inDx;
            int inDy;
            int outDx;
            int outDy;

            WmPaintDir(map, stride, pts[i].x, pts[i].y, dir, oam2);

            if (i == 0)
            {
                inDx = pts[1].x - pts[0].x;
                inDy = pts[1].y - pts[0].y;
            }
            else
            {
                inDx = pts[i].x - pts[i - 1].x;
                inDy = pts[i].y - pts[i - 1].y;
            }

            outDx = pts[i + 1].x - pts[i].x;
            outDy = pts[i + 1].y - pts[i].y;
            WmPaintElbow(map, stride, pts[i].x, pts[i].y, dir, inDx, inDy, outDx, outDy, oam2);
        }
        else
        {
            WmPaintDir(map, stride, pts[i].x, pts[i].y, dir, oam2);

            if (dir != nd && (nd == WM_DIR_H || nd == WM_DIR_V))
                WmPaintDir(map, stride, pts[i].x, pts[i].y, nd, oam2);
        }

        prevDir = dir;
    }
}

static int WmRoutePath(u8 * occ, int pathId, struct WmPt * pts)
{
    int fromNode = gWMPathData[pathId].node[0];
    int toNode = gWMPathData[pathId].node[1];

    return WmPickCenterline(
        occ,
        WmPixelToTile(GetWmNodeX(fromNode)),
        WmPixelToTile(GetWmNodeY(fromNode)),
        WmPixelToTile(GetWmNodeX(toNode)),
        WmPixelToTile(GetWmNodeY(toNode)),
        pts);
}

static void WmRasterPathId(int pathId, u16 * map, int stride, u8 * occ, u16 oam2)
{
    struct WmPt pts[WM_MAX_PTS];
    int n = WmRoutePath(occ, pathId, pts);

    WmRasterPts(map, stride, pts, n, oam2);
    WmMarkCenterline(occ, pts, n);
}

static void WmReversePts(struct WmPt * pts, int n)
{
    int i;

    for (i = 0; i < n / 2; i++)
    {
        s16 tx = pts[i].x;
        s16 ty = pts[i].y;

        pts[i].x = pts[n - 1 - i].x;
        pts[i].y = pts[n - 1 - i].y;
        pts[n - 1 - i].x = tx;
        pts[n - 1 - i].y = ty;
    }
}

static int WmManhattan(int x1, int y1, int x2, int y2)
{
    return WmAbs(x2 - x1) + WmAbs(y2 - y1);
}

static int WmTileCenter(int t)
{
    return (t << 3) + 4;
}

static int WmCollectCorners(struct WmPt * pts, int n, struct WmPt * out)
{
    int m = 0;
    int i;
    int dir;
    int nd;

    if (n < 3)
        return 0;

    for (i = 1; i < n - 1; i++)
    {
        dir = WmSegDir(pts[i - 1], pts[i]);
        nd = WmSegDir(pts[i], pts[i + 1]);

        if (dir != nd && m < WM_MAX_SPLINE - 2)
        {
            out[m].x = pts[i].x;
            out[m].y = pts[i].y;
            m++;
        }
    }

    return m;
}

static int WmRouteForTravel(int pathId, struct WmPt * pts)
{
    u8 occ[WM_OCC_BYTES];
    int nWanted = 0;
    int i;
    int n;

    WmOccClear(occ);

    for (i = 0; i < gGMData.openPaths.openPathsLength; i++)
    {
        int pid = gGMData.openPaths.openPaths[i];

        if (pid < 0 || pid >= WM_PATH_MAX)
            continue;

        if (pid == pathId)
            continue;

        n = WmRoutePath(occ, pid, pts);
        WmMarkCenterline(occ, pts, n);
    }

    nWanted = WmRoutePath(occ, pathId, pts);

    if (nWanted == 0)
    {
        WmOccClear(occ);
        nWanted = WmRoutePath(occ, pathId, pts);
    }

    return nWanted;
}

void WmDrawPathFromCurrentToDest(void)
{
    int fromNode = gGMData.units[0].location;
    int toNode = WMLoc_GetNextLocId(fromNode);
    int startingNode;
    int pathId;

    if (toNode < 0 || toNode >= NODE_MAX)
        return;

    pathId = sub_80BCDE4(fromNode, toNode, &startingNode);

    if (pathId < 0)
        return;

    AddAndDrawGmPath(pathId, 0x1e);
}

//! FE8U = 0x080BBC54
LYN_REPLACE_CHECK(sub_80BBC54);
void sub_80BBC54(struct GmRouteProc * proc)
{
    int i;
    int n;
    u16 oam2Base;

    if (proc == NULL || proc->pOpenPaths == NULL)
        return;

    oam2Base = (proc->chr / CHR_SIZE) | (proc->pal << 0xc);
    CpuFill16(0, gUnknown_02019D00, 0x12C0);

    n = proc->pOpenPaths->openPathsLength;
    if (n > WM_PATH_MAX)
        n = WM_PATH_MAX;
    if (n < 0)
        n = 0;

    for (i = 0; i < n; i++)
    {
        int pathId = proc->pOpenPaths->openPaths[i];
        u8 * gfx;

        if (pathId < 0 || pathId >= WM_PATH_MAX)
            continue;

        gfx = gWMPathData[pathId].gfxData;
        if (gfx != NULL)
        {
            /* Magvel slots already have baked route gfx. Regenerating every
             * open road during WM_DRAWPATH (Ch9 Path 08/09) was crashing. */
            sub_80BBBF4(gfx, gUnknown_02019D00, WM_ROUTE_W, oam2Base);
        }
        else
        {
            u8 occ[WM_OCC_BYTES];

            WmOccClear(occ);
            WmRasterPathId(pathId, gUnknown_02019D00, WM_ROUTE_W, occ, oam2Base);
        }
    }
}

//! FE8U = 0x080BCE34
LYN_REPLACE_CHECK(sub_80BCE34);
int sub_80BCE34(int nodeA, int nodeB, s16 c, u16 * d, int * e, int f)
{
    struct WmPt pts[WM_MAX_PTS];
    struct WmPt corners[WM_MAX_SPLINE];
    int pathId;
    int startingNodeIdx;
    int n0;
    int n1;
    int x1;
    int y1;
    int x2;
    int y2;
    int n;
    int nCorners;
    int i;
    int px;
    int py;
    int nx;
    int ny;
    int acc;
    int total;
    int elapsed;
    int count;

    pathId = sub_80BCDE4(nodeA, nodeB, &startingNodeIdx);

    if (pathId < 0)
        return 0;

    n0 = gWMPathData[pathId].node[startingNodeIdx];
    n1 = gWMPathData[pathId].node[1 - startingNodeIdx];
    x1 = GetWmNodeX(n0);
    y1 = GetWmNodeY(n0);
    x2 = GetWmNodeX(n1);
    y2 = GetWmNodeY(n1);

    if (gWMPathData[pathId].movementPath != NULL)
    {
        const struct GMapMovementPathData * mov = gWMPathData[pathId].movementPath;
        int wpCount = 0;
        int w;

        while (wpCount < WM_MAX_SPLINE - 2 && mov[wpCount].elapsedTime >= 0)
            wpCount++;

        *d = 0;
        e[0] = x1 << f;
        e[1] = y1 << f;
        d++;
        e += 2;
        count = 1;

        if (startingNodeIdx == 0)
        {
            for (w = 0; w < wpCount; w++)
            {
                *d = DivArm(0x1000, mov[w].elapsedTime * c);
                e[0] = mov[w].x << f;
                e[1] = mov[w].y << f;
                d++;
                e += 2;
                count++;
            }
        }
        else
        {
            for (w = wpCount - 1; w >= 0; w--)
            {
                *d = DivArm(0x1000, c * (0x1000 - mov[w].elapsedTime));
                e[0] = mov[w].x << f;
                e[1] = mov[w].y << f;
                d++;
                e += 2;
                count++;
            }
        }

        *d = c;
        e[0] = x2 << f;
        e[1] = y2 << f;
        return wpCount + 2;
    }

    n = WmRouteForTravel(pathId, pts);

    if (n >= 2)
    {
        int dStart = WmManhattan(pts[0].x, pts[0].y, WmPixelToTile(x1), WmPixelToTile(y1));
        int dEnd = WmManhattan(pts[n - 1].x, pts[n - 1].y, WmPixelToTile(x1), WmPixelToTile(y1));

        if (dEnd < dStart)
            WmReversePts(pts, n);
    }

    nCorners = WmCollectCorners(pts, n, corners);

    *d = 0;
    e[0] = x1 << f;
    e[1] = y1 << f;
    d++;
    e += 2;
    count = 1;

    total = 0;
    px = x1;
    py = y1;

    for (i = 0; i < nCorners; i++)
    {
        nx = WmTileCenter(corners[i].x);
        ny = WmTileCenter(corners[i].y);
        total += WmManhattan(px, py, nx, ny);
        px = nx;
        py = ny;
    }

    total += WmManhattan(px, py, x2, y2);

    if (total < 1)
        total = 1;

    acc = 0;
    px = x1;
    py = y1;

    for (i = 0; i < nCorners; i++)
    {
        nx = WmTileCenter(corners[i].x);
        ny = WmTileCenter(corners[i].y);
        acc += WmManhattan(px, py, nx, ny);
        elapsed = (acc * 0x1000) / total;

        if (elapsed < 1)
            elapsed = 1;

        if (elapsed > 0xfff)
            elapsed = 0xfff;

        *d = DivArm(0x1000, elapsed * c);
        e[0] = nx << f;
        e[1] = ny << f;
        d++;
        e += 2;
        count++;
        px = nx;
        py = ny;
    }

    *d = c;
    e[0] = x2 << f;
    e[1] = y2 << f;
    count++;

    return count;
}
