#include "common-chax.h"
#include "district.h"

/*
 * World-map district flags (#710): each shown node with an unlocked district carries the next chapter's flag, in green,
 * on the world map and on its minimap, so the player sees where Enter District is.
 *
 * - The world map: vanilla draws the next chapter's flag with an animation (Sprite_08A97AEC, animation 1) whose tiles the
 *   node icon display loads, at the node icon's flag origin (GmapScreen2_Loop). A proc under the world map runs the same
 *   animation over each district node, while the node icons are drawn.
 * - The minimap: WmMinimap_PutStoryNodeIcon below is vanilla's, which puts the flag object (0x08590F44, OBJ chr 0xF7) on
 *   the next node, and puts it on the district nodes too.
 * - Green: the flags' palette, copied every frame (so fades and blinks follow) with red and green swapped, in an OBJ
 *   palette the world map leaves empty (8).
 *
 * The next chapter's node keeps its red flag only.
 */

/*
 * Both flags are drawn in OBJ palette 4: the minimap's directly, the world map's through its animation, whose frames
 * add 1 to the base palette (the node icon display's, 3). Their green copy goes to OBJ palette 8.
 */
#define FLAG_PAL 4
#define DISTRICT_FLAG_PAL 8
#define MINIMAP_FLAG_CHR 0xF7

/* FE8U 0x080C3770 (FE-CLib has no declaration) */
void WmMinimap_PutStoryNodeIcon(struct GMapRadarProc *proc);

#define MinimapFlagObject ((const u16 *)0x08590F44)

struct DistrictFlagProc {
	PROC_HEADER;
	struct APHandle *ap;
};

/* The pal-th OBJ palette, with red and green swapped, into OBJ palette dst */
static void District_GreenPalette(int pal, int dst)
{
	const u16 *src = gPaletteBuffer + 0x100 + 0x10 * pal;
	u16 *out = gPaletteBuffer + 0x100 + 0x10 * dst;
	int i, c;

	for (i = 0; i < 0x10; i++) {
		c = src[i];
		out[i] = (c & 0x7C00) | ((c & 0x1F) << 5) | ((c >> 5) & 0x1F);
	}
	EnablePaletteSync();
}

/* The district nodes to flag: shown, unlocked, and not the next chapter's (it has the red flag) */
static bool District_NodeFlagged(const struct DistrictNode *it, int next)
{
	return it->node < NODE_MAX && it->node != next && (gGMData.nodes[it->node].state & 1) && District_IsUnlocked(it);
}

static void DistrictFlag_Init(struct DistrictFlagProc *proc)
{
	proc->ap = AP_Create(Sprite_08A97AEC, 11);
	AP_SwitchAnimation(proc->ap, 1);
}

static void DistrictFlag_Loop(struct DistrictFlagProc *proc)
{
	struct GmNodeIconDisplayProc *icons = GM_ICON;
	const struct DistrictNode *it;
	const struct GMapNodeData *node;
	const struct NodeIcon *icon;
	int x, y, next;
	bool advanced = false;

	if (!icons || !icons->skip || !icons->pScreenProc)
		return;

	next = icons->merge_next_node ? icons->nodeId : -1;
	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++) {
		if (!District_NodeFlagged(it, next))
			continue;

		node = &gWMNodeData[it->node];
		icon = gWMNodeIconData +
		       ((gGMData.nodes[it->node].state & 2) ? node->iconPreClear : node->iconPostClear);
		x = node->x - icon->xCenter + icon->xFlagOrigin - icons->pScreenProc->x;
		y = node->y - icon->yCenter + icon->yFlagOrigin - icons->pScreenProc->y;
		if (y < -0x20 || y >= 0xC0 || x < -0x20 || x >= 0x110)
			continue;

		if (!advanced) {
			District_GreenPalette(FLAG_PAL, DISTRICT_FLAG_PAL);
			proc->ap->tileBase = (icons->chr + 0x1000) / CHR_SIZE + OAM2_PAL(DISTRICT_FLAG_PAL - 1) + OAM2_LAYER(2);
			AP_Update(proc->ap, OAM1_X(x), OAM0_Y(y));
			advanced = true;
		} else {
			AP_Display(proc->ap, OAM1_X(x), OAM0_Y(y));
		}
	}
}

static void DistrictFlag_End(struct DistrictFlagProc *proc)
{
	AP_Delete(proc->ap);
}

static const struct ProcCmd ProcScr_DistrictFlags[] = {
	PROC_NAME("DistrictFlags"),
	PROC_CALL(DistrictFlag_Init),
	PROC_SET_END_CB(DistrictFlag_End),
	PROC_REPEAT(DistrictFlag_Loop),
	PROC_END,
};

/* The world map's main proc, each frame: the flags' proc runs under it, and ends with it */
void District_StartWorldMapFlags(ProcPtr worldMapMain)
{
	if (worldMapMain && !Proc_Find(ProcScr_DistrictFlags))
		Proc_Start(ProcScr_DistrictFlags, worldMapMain);
}

/* Vanilla's minimap flag (WmMinimap_PutStoryNodeIcon) at a node, with OBJ attribute 2 oam2 */
static void Minimap_PutFlag(struct GMapRadarProc *proc, int nodeId, int oam2)
{
	int x = proc->unk_2f * 8 + Div(gWMNodeData[nodeId].x * 63, 480);
	int y = proc->unk_30 * 8 + Div(gWMNodeData[nodeId].y * 43, 320) + 7;

	if ((unsigned int)(x + 31) > 0x12F || y < -32 || y > 191)
		return;

	PutSpriteExt(0, (x - 1) & 0x1FF, y & 0xFF, MinimapFlagObject, oam2);
}

LYN_REPLACE_CHECK(WmMinimap_PutStoryNodeIcon);
void WmMinimap_PutStoryNodeIcon(struct GMapRadarProc *proc)
{
	const struct DistrictNode *it;
	int next = GetNextUnclearedNode(&gGMData);
	bool green = false;

	if (next >= 0)
		Minimap_PutFlag(proc, next, MINIMAP_FLAG_CHR + OAM2_PAL(FLAG_PAL));

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++) {
		if (!District_NodeFlagged(it, next))
			continue;
		if (!green) {
			District_GreenPalette(FLAG_PAL, DISTRICT_FLAG_PAL);
			green = true;
		}
		Minimap_PutFlag(proc, it->node, MINIMAP_FLAG_CHR + OAM2_PAL(DISTRICT_FLAG_PAL));
	}
}
