#include "common-chax.h"
#include "bwl.h"
#include "constants/texts.h"
#include "icon-rework.h"
#include "kernel/skill-tree.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "stat-screen.h"

enum {
	SKILL_TREE_MESSAGE_NONE,
	SKILL_TREE_MESSAGE_LEARNED,
	SKILL_TREE_MESSAGE_NO_SP,
	SKILL_TREE_MESSAGE_NO_SPACE,
	SKILL_TREE_MESSAGE_LOCKED,
};

enum {
	SKILL_TREE_TEXT_TITLE = STATSCREEN_TEXT_BWL,      /* width 16 */
	SKILL_TREE_TEXT_SP = STATSCREEN_TEXT_ITEM0,       /* width 8 */
	SKILL_TREE_TEXT_COST = STATSCREEN_TEXT_ITEM1,     /* width 8 */
	SKILL_TREE_TEXT_MSG = STATSCREEN_TEXT_ITEM2,      /* width 8 */
	SKILL_TREE_TEXT_YES = STATSCREEN_TEXT_SUPPORT0,   /* width 7 */
	SKILL_TREE_TEXT_NO = STATSCREEN_TEXT_SUPPORT1,    /* width 7 */
};

static void StatScreen_SyncPageAmt(void)
{
	int pageAmt = GetStatPageCount();

	if (pageAmt <= 0)
		pageAmt = 1;

	gStatScreen.pageAmt = pageAmt;

	/* Keep page in visible range [0, pageAmt). OOB pages blank the panel. */
	if (gStatScreen.page >= pageAmt)
		gStatScreen.page = pageAmt - 1;
}

/*
 * Faded skill icons reuse BGPAL 6/7 as half-bright copies of the item-icon pals.
 * Restore the stolen banks when leaving the tree page.
 */
static void SkillTree_ApplyFadedIconPalettes(void)
{
	int bank;
	int i;

	for (bank = 0; bank < 2; ++bank) {
		u16 *src = &gPaletteBuffer[0x10 * (STATSCREEN_BGPAL_ITEMICONS + bank)];
		u16 *dst = &gPaletteBuffer[0x10 * (STATSCREEN_BGPAL_6 + bank)];

		for (i = 0; i < 0x10; ++i) {
			u16 c = src[i];
			int r = (c & 0x1F) >> 1;
			int g = ((c >> 5) & 0x1F) >> 1;
			int b = ((c >> 10) & 0x1F) >> 1;

			dst[i] = RGB(r, g, b);
		}
	}

	EnablePaletteSync();
}

static void SkillTree_RestoreStolenPalettes(void)
{
	if (gStatScreen.unit != NULL && UNIT_FACTION(gStatScreen.unit) == FACTION_RED)
		ApplyPalette(sStatBarPaletteLookup[1], STATSCREEN_BGPAL_6);
	else
		ApplyPalette(sStatBarPaletteLookup[0], STATSCREEN_BGPAL_6);

	LoadIconPalette(1, STATSCREEN_BGPAL_7);
	EnablePaletteSync();
}

static bool SkillTree_IsActivePage(void)
{
	if (!IsStatScreenPageAvailable(PAGE_SKILL_TREE))
		return false;

	StatScreen_SyncPageAmt();
	return TranslateStatPageId(gStatScreen.page) == PAGE_SKILL_TREE;
}

static void StatScreen_ChangePage(struct Proc *proc, int direction)
{
	int pageAmt;
	int page;
	bool leavingTree;

	StatScreen_SyncPageAmt();

	pageAmt = gStatScreen.pageAmt;
	if (pageAmt <= 1)
		return;

	if (gStatScreen.inTransition && !Proc_Find(gProcScr_SSPageSlide))
		gStatScreen.inTransition = FALSE;

	if (Proc_Find(gProcScr_SSPageSlide) || gStatScreen.inTransition)
		return;

	leavingTree = SkillTree_IsActivePage();

	page = gStatScreen.page + direction;
	if (page < 0)
		page = pageAmt - 1;
	else if (page >= pageAmt)
		page = 0;

	gStatScreen.page = page;

	if (leavingTree) {
		SkillTree_RestoreStolenPalettes();
		gSkillTreePageDrawn = false;
	}

	StartPageSlide(direction < 0 ? DPAD_LEFT : DPAD_RIGHT, page, proc);
}

static const struct SkillTreeNode *SkillTree_GetCursorNode(const struct UnitSkillTree *tree)
{
	if (tree == NULL || tree->count == 0)
		return NULL;

	if (gSkillTreeCursor >= tree->count || gSkillTreeCursor >= SKILL_TREE_MAX_NODES)
		gSkillTreeCursor = 0;

	return &tree->nodes[gSkillTreeCursor];
}

/*
 * Pick the nearest visible node in a direction by layout (x/y), not path adj.
 * Prefer same-row/column alignment, then closer distance along the axis.
 */
static int SkillTree_FindNeighbor(const struct UnitSkillTree *tree, int from, int direction)
{
	const struct SkillTreeNode *cur;
	int best = SKILL_TREE_NODE_NONE;
	int bestScore = 0x7FFFFFFF;
	int i;

	if (tree == NULL || from < 0 || from >= tree->count || from >= SKILL_TREE_MAX_NODES)
		return SKILL_TREE_NODE_NONE;

	cur = &tree->nodes[from];

	for (i = 0; i < tree->count && i < SKILL_TREE_MAX_NODES; ++i) {
		const struct SkillTreeNode *node = &tree->nodes[i];
		int dx;
		int dy;
		int primary;
		int secondary;
		int score;

		if (i == from || node->sid == 0)
			continue;

		dx = (int)node->x - (int)cur->x;
		dy = (int)node->y - (int)cur->y;

		switch (direction) {
		case SKILL_TREE_DIR_LEFT:
			if (dx >= 0)
				continue;
			primary = -dx;
			secondary = dy < 0 ? -dy : dy;
			break;
		case SKILL_TREE_DIR_RIGHT:
			if (dx <= 0)
				continue;
			primary = dx;
			secondary = dy < 0 ? -dy : dy;
			break;
		case SKILL_TREE_DIR_UP:
			if (dy >= 0)
				continue;
			primary = -dy;
			secondary = dx < 0 ? -dx : dx;
			break;
		case SKILL_TREE_DIR_DOWN:
			if (dy <= 0)
				continue;
			primary = dy;
			secondary = dx < 0 ? -dx : dx;
			break;
		default:
			continue;
		}

		/* Alignment outweighs axial distance so same-row hops win. */
		score = secondary * 256 + primary;
		if (score < bestScore) {
			bestScore = score;
			best = i;
		}
	}

	return best;
}

static void SkillTree_ResetState(void)
{
	u8 pid = gStatScreen.unit != NULL ? UNIT_CHAR_ID(gStatScreen.unit) : 0xFF;

	gSkillTreeCursor = 0;
	gSkillTreeCursorPid = pid;
	gSkillTreeLastPage = gStatScreen.page;
	gSkillTreeConfirming = false;
	gSkillTreeConfirmChoice = 0;
	gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
	gSkillTreePageDrawn = false;
}

static void SkillTree_ResetStateIfNeeded(void)
{
	u8 pid = gStatScreen.unit != NULL ? UNIT_CHAR_ID(gStatScreen.unit) : 0xFF;

	if (
		gSkillTreeLastPage != gStatScreen.page
		|| gSkillTreeCursorPid != pid
		|| gSkillTreeCursor >= SKILL_TREE_MAX_NODES
	)
		SkillTree_ResetState();
}

static void SkillTree_DrawText(int textId, int x, int y, int color, const char *str)
{
	ClearText(&gStatScreen.text[textId]);
	PutDrawText(
		&gStatScreen.text[textId],
		gUiTmScratchA + TILEMAP_INDEX(x, y),
		color,
		0,
		0,
		str
	);
}

static void SkillTree_DrawNode(struct Unit *unit, const struct UnitSkillTree *tree, int nodeIndex)
{
	const struct SkillTreeNode *node = &tree->nodes[nodeIndex];
	bool learned = IsSkillTreeNodeLearned(unit, node);
	bool available = IsSkillTreeNodeAvailable(unit, tree, nodeIndex);
	int iconPal = GetSkillIconPal(node->sid);
	int basePal;

	if (node->sid == 0)
		return;

	/* Learned or unselectable → half-bright palette; buyable stays full color. */
	if (learned || !available)
		basePal = STATSCREEN_BGPAL_6;
	else
		basePal = STATSCREEN_BGPAL_ITEMICONS;

	DrawIcon(
		gUiTmScratchA + TILEMAP_INDEX(node->x, node->y),
		SKILL_ICON(node->sid),
		TILEREF(0, basePal + iconPal)
	);
}

static void SkillTree_DrawConfirmPrompt(const struct SkillTreeNode *node)
{
	int yesColor = gSkillTreeConfirmChoice == 0 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY;
	int noColor = gSkillTreeConfirmChoice == 1 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY;

	SkillTree_DrawText(SKILL_TREE_TEXT_MSG, 1, 16, TEXT_COLOR_SYSTEM_GOLD, "Learn?");
	PutNumber(
		gUiTmScratchA + TILEMAP_INDEX(6, 16),
		TEXT_COLOR_SYSTEM_BLUE,
		node != NULL ? GetSkillTreeSpCost(node->sid) : 0
	);
	SkillTree_DrawText(SKILL_TREE_TEXT_YES, 10, 16, yesColor, "Yes");
	SkillTree_DrawText(SKILL_TREE_TEXT_NO, 13, 16, noColor, "No");
}

static void SkillTree_DrawSp(struct NewBwl *bwl)
{
	/* "SP" then up to 3 digits with ones at x=16 (tiles 14-16). */
	SkillTree_DrawText(SKILL_TREE_TEXT_SP, 12, 16, TEXT_COLOR_SYSTEM_GOLD, "SP");
	PutNumber(
		gUiTmScratchA + TILEMAP_INDEX(16, 16),
		TEXT_COLOR_SYSTEM_BLUE,
		bwl != NULL ? bwl->skillPoints : 0
	);
}

static void SkillTree_DrawSelectedInfo(struct Unit *unit, const struct UnitSkillTree *tree)
{
	const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);
	const char *msg = NULL;
	int msgColor = TEXT_COLOR_SYSTEM_GRAY;

	(void)unit;

	if (node == NULL || gSkillTreeConfirming)
		return;

	if (gSkillTreeMessage == SKILL_TREE_MESSAGE_LEARNED) {
		msg = "Learned";
		msgColor = TEXT_COLOR_SYSTEM_GREEN;
	} else if (gSkillTreeMessage == SKILL_TREE_MESSAGE_NO_SP) {
		msg = "No SP";
	} else if (gSkillTreeMessage == SKILL_TREE_MESSAGE_NO_SPACE) {
		msg = "No slots";
	} else if (gSkillTreeMessage == SKILL_TREE_MESSAGE_LOCKED) {
		msg = "Locked";
	}

	if (msg != NULL) {
		SkillTree_DrawText(SKILL_TREE_TEXT_MSG, 1, 16, msgColor, msg);
	} else {
		/* "Cost" + up to 3 digits (ones at x=6 → tiles 4-6). */
		SkillTree_DrawText(SKILL_TREE_TEXT_COST, 1, 16, TEXT_COLOR_SYSTEM_GOLD, "Cost");
		PutNumber(
			gUiTmScratchA + TILEMAP_INDEX(6, 16),
			TEXT_COLOR_SYSTEM_BLUE,
			GetSkillTreeSpCost(node->sid)
		);
	}
}

static void SkillTree_DrawCursor(const struct UnitSkillTree *tree)
{
	const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);

	if (node != NULL)
		DisplayUiHand((12 + node->x) * 8 - 4, (2 + node->y) * 8);
}

void DrawPageSkillTree(void)
{
	struct Unit *unit = gStatScreen.unit;
	const struct UnitSkillTree *tree;
	struct NewBwl *bwl;
	int i;

	SkillTree_ResetStateIfNeeded();
	tree = GetUnitSkillTree(unit);

	ResetIconGraphics();
	LoadIconPalettes(STATSCREEN_BGPAL_ITEMICONS);
	SkillTree_ApplyFadedIconPalettes();

	for (i = STATSCREEN_TEXT_STATUS; i < STATSCREEN_TEXT_MAX; ++i)
		ClearText(&gStatScreen.text[i]);

	SkillTree_DrawText(SKILL_TREE_TEXT_TITLE, 6, 0, TEXT_COLOR_SYSTEM_GOLD, "Skill Tree");

	bwl = unit != NULL ? GetNewBwl(UNIT_CHAR_ID(unit)) : NULL;

	if (tree == NULL) {
		SkillTree_DrawSp(bwl);
		SkillTree_DrawText(SKILL_TREE_TEXT_MSG, 2, 8, TEXT_COLOR_SYSTEM_GRAY, "No tree");
		HideSysHandCursor();
		return;
	}

	for (i = 0; i < tree->count && i < SKILL_TREE_MAX_NODES; ++i)
		SkillTree_DrawNode(unit, tree, i);

	if (gSkillTreeConfirming)
		SkillTree_DrawConfirmPrompt(SkillTree_GetCursorNode(tree));
	else {
		SkillTree_DrawSelectedInfo(unit, tree);
		SkillTree_DrawSp(bwl);
	}
}

static void SkillTree_FlushToScreen(void)
{
	DisplayPage(gStatScreen.page);
	TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(12, 2), 18, 18);
	TileMap_CopyRect(gUiTmScratchC, gBG2TilemapBuffer + TILEMAP_INDEX(12, 2), 18, 18);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
	gSkillTreePageDrawn = true;
}

/*
 * Cursor / Cost / SP updates must not ResetIconGraphics.
 * Reloading every icon each move floods RegisterDataMove and can crash.
 */
static void SkillTree_RedrawFooter(void)
{
	struct Unit *unit = gStatScreen.unit;
	const struct UnitSkillTree *tree = GetUnitSkillTree(unit);
	struct NewBwl *bwl = unit != NULL ? GetNewBwl(UNIT_CHAR_ID(unit)) : NULL;

	TileMap_FillRect(gUiTmScratchA + TILEMAP_INDEX(0, 16), 18, 2, 0);
	ClearText(&gStatScreen.text[SKILL_TREE_TEXT_SP]);
	ClearText(&gStatScreen.text[SKILL_TREE_TEXT_COST]);
	ClearText(&gStatScreen.text[SKILL_TREE_TEXT_MSG]);
	ClearText(&gStatScreen.text[SKILL_TREE_TEXT_YES]);
	ClearText(&gStatScreen.text[SKILL_TREE_TEXT_NO]);

	if (gSkillTreeConfirming)
		SkillTree_DrawConfirmPrompt(SkillTree_GetCursorNode(tree));
	else {
		SkillTree_DrawSelectedInfo(unit, tree);
		SkillTree_DrawSp(bwl);
	}

	TileMap_CopyRect(
		gUiTmScratchA + TILEMAP_INDEX(0, 16),
		gBG0TilemapBuffer + TILEMAP_INDEX(12, 18),
		18,
		2
	);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillTree_Redraw(void)
{
	if (!SkillTree_IsActivePage())
		return;

	/* Learning / confirm changes icon fade state — full redraw. */
	SkillTree_FlushToScreen();
}

static void SkillTree_RedrawSelection(void)
{
	if (!SkillTree_IsActivePage())
		return;

	if (!gSkillTreePageDrawn)
		SkillTree_FlushToScreen();
	else
		SkillTree_RedrawFooter();
}

static bool SkillTree_TryLearn(struct Unit *unit, const struct UnitSkillTree *tree)
{
	const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);
	struct NewBwl *bwl;
	u8 cost;

	if (node == NULL || !IsSkillTreeNodeAvailable(unit, tree, gSkillTreeCursor))
		return false;

	cost = GetSkillTreeSpCost(node->sid);
	bwl = GetNewBwl(UNIT_CHAR_ID(unit));

	if (bwl == NULL || bwl->skillPoints < cost) {
		gSkillTreeMessage = SKILL_TREE_MESSAGE_NO_SP;
		return false;
	}

	if (GetSkillSlot(unit, node->sid) < 0 && GetFreeSkillSlot(unit) < 0) {
		gSkillTreeMessage = SKILL_TREE_MESSAGE_NO_SPACE;
		return false;
	}

	if (AddSkill(unit, node->sid) != 0) {
		gSkillTreeMessage = SKILL_TREE_MESSAGE_NO_SPACE;
		return false;
	}

	bwl->skillPoints -= cost;
	gSkillTreeMessage = SKILL_TREE_MESSAGE_LEARNED;
	return true;
}

static bool SkillTree_HandleConfirmInput(void)
{
	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		gSkillTreeConfirming = false;
		gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		SkillTree_RedrawSelection();
		return true;
	}

	if (gKeyStatusPtr->newKeys & (DPAD_LEFT | DPAD_UP)) {
		gSkillTreeConfirmChoice = 0;
		PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		SkillTree_RedrawSelection();
		return true;
	}

	if (gKeyStatusPtr->newKeys & (DPAD_RIGHT | DPAD_DOWN)) {
		gSkillTreeConfirmChoice = 1;
		PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		SkillTree_RedrawSelection();
		return true;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		if (gSkillTreeConfirmChoice == 0) {
			struct Unit *unit = gStatScreen.unit;
			const struct UnitSkillTree *tree = GetUnitSkillTree(unit);

			if (SkillTree_TryLearn(unit, tree)) {
				PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
				gSkillTreeConfirming = false;
				/* Icon fade states changed after learning. */
				SkillTree_Redraw();
			} else {
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				gSkillTreeConfirming = false;
				SkillTree_RedrawSelection();
			}
		} else {
			gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
			PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
			gSkillTreeConfirming = false;
			SkillTree_RedrawSelection();
		}

		return true;
	}

	return true;
}

bool SkillTree_HandleStatScreenInput(struct Proc *proc)
{
	struct Unit *unit;
	const struct UnitSkillTree *tree;

	if (!SkillTree_IsActivePage())
		return false;

	if (gStatScreen.inTransition && !Proc_Find(gProcScr_SSPageSlide))
		gStatScreen.inTransition = FALSE;

	if (Proc_Find(gProcScr_SSPageSlide))
		return true;

	SkillTree_ResetStateIfNeeded();

	/* Draw the page once after the slide; never reload icons every idle frame. */
	if (!gSkillTreePageDrawn)
		SkillTree_FlushToScreen();

	unit = gStatScreen.unit;
	tree = GetUnitSkillTree(unit);

	if (tree != NULL)
		SkillTree_DrawCursor(tree);

	if (gSkillTreeConfirming)
		return SkillTree_HandleConfirmInput();

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		HideSysHandCursor();
		return false;
	}

	if (gKeyStatusPtr->newKeys & R_BUTTON) {
		const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);

		if (node != NULL && GetSkillDescMsg(node->sid) != 0) {
			gStatScreen.help = NULL;
			Proc_Goto(proc, 0);
			StartStatScreenHelp(gStatScreen.page, proc);
		}

		return true;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);

		if (node != NULL) {
			if (IsSkillTreeNodeLearned(unit, node)) {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_LEARNED;
				SkillTree_RedrawSelection();
			} else if (IsSkillTreeNodeAvailable(unit, tree, gSkillTreeCursor)) {
				gSkillTreeConfirming = true;
				gSkillTreeConfirmChoice = 0;
				gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
				SkillTree_RedrawSelection();
			} else if (IsSkillTreeNodeLocked(unit, tree, gSkillTreeCursor)) {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_LOCKED;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				SkillTree_RedrawSelection();
			} else {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_NO_SP;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				SkillTree_RedrawSelection();
			}
		}

		return true;
	}

	/* D-pad moves by on-screen layout; Left/Right change page only at an edge. */
	if (tree != NULL && (gKeyStatusPtr->repeatedKeys & (DPAD_UP | DPAD_DOWN | DPAD_LEFT | DPAD_RIGHT))) {
		int direction;
		int next;

		if (gKeyStatusPtr->repeatedKeys & DPAD_UP)
			direction = SKILL_TREE_DIR_UP;
		else if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN)
			direction = SKILL_TREE_DIR_DOWN;
		else if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT)
			direction = SKILL_TREE_DIR_LEFT;
		else
			direction = SKILL_TREE_DIR_RIGHT;

		next = SkillTree_FindNeighbor(tree, gSkillTreeCursor, direction);

		if (next != SKILL_TREE_NODE_NONE) {
			gSkillTreeCursor = next;
			gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			SkillTree_RedrawSelection();
			return true;
		}

		if (
			(direction == SKILL_TREE_DIR_LEFT || direction == SKILL_TREE_DIR_RIGHT)
			&& (gKeyStatusPtr->newKeys & (DPAD_LEFT | DPAD_RIGHT))
		) {
			HideSysHandCursor();
			StatScreen_ChangePage(proc, direction == SKILL_TREE_DIR_LEFT ? -1 : 1);
		}

		return true;
	}

	return true;
}

void StartSkillTreeScreenHelp(int pageid, struct Proc *proc)
{
	const struct UnitSkillTree *tree = GetUnitSkillTree(gStatScreen.unit);
	const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);
	u16 mid;

	(void)pageid;
	(void)proc;

	if (node == NULL) {
		gStatScreen.help = &gHelpInfo_Ss2Rank0;
		return;
	}

	mid = GetSkillDescMsg(node->sid);
	if (mid == 0) {
		gStatScreen.help = &gHelpInfo_Ss2Rank0;
		return;
	}

	sMutableHbi.adjUp = NULL;
	sMutableHbi.adjDown = NULL;
	sMutableHbi.adjLeft = NULL;
	sMutableHbi.adjRight = NULL;
	sMutableHbi.xDisplay = 12 * 8 + node->x * 8;
	sMutableHbi.yDisplay = 2 * 8 + node->y * 8;
	sMutableHbi.mid = mid;
	sMutableHbi.redirect = NULL;
	sMutableHbi.populate = NULL;
	gStatScreen.help = &sMutableHbi;
}

LYN_REPLACE_CHECK(StatScreen_OnIdle);
void StatScreen_OnIdle(struct Proc *proc)
{
	struct Unit *unit;

	if (SkillTree_HandleStatScreenInput(proc))
		return;

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		gLCDControlBuffer.dispcnt.bg0_on = TRUE;
		gLCDControlBuffer.dispcnt.bg1_on = FALSE;
		gLCDControlBuffer.dispcnt.bg2_on = TRUE;
		gLCDControlBuffer.dispcnt.bg3_on = TRUE;
		gLCDControlBuffer.dispcnt.obj_on = TRUE;

		SetBlendConfig(3, 0, 0, 0x10);
		SetBlendTargetA(0, 0, 0, 0, 0);
		SetBlendBackdropA(1);

		gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
		EnablePaletteSync();

		Proc_Break(proc);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
	} else if (gKeyStatusPtr->newKeys & DPAD_LEFT) {
		StatScreen_ChangePage(proc, -1);
	} else if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
		StatScreen_ChangePage(proc, +1);
	} else if (gKeyStatusPtr->repeatedKeys & DPAD_UP) {
		unit = FindNextUnit(gStatScreen.unit, -1);
		StartUnitSlide(unit, -1, proc);
	} else if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		unit = FindNextUnit(gStatScreen.unit, +1);
		StartUnitSlide(unit, +1, proc);
	} else if ((gKeyStatusPtr->repeatedKeys & A_BUTTON) && (gStatScreen.unit->rescue)) {
		unit = GetUnit(gStatScreen.unit->rescue);
		StartUnitSlide(unit, (gStatScreen.unit->state & US_RESCUING) ? +1 : -1, proc);
	} else if (gKeyStatusPtr->newKeys & R_BUTTON) {
		Proc_Goto(proc, 0);
		StartStatScreenHelp(gStatScreen.page, proc);
	}
}
