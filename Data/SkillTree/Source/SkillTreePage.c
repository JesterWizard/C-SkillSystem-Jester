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

	StatScreen_SyncPageAmt();

	pageAmt = gStatScreen.pageAmt;
	if (pageAmt <= 1)
		return;

	if (gStatScreen.inTransition && !Proc_Find(gProcScr_SSPageSlide))
		gStatScreen.inTransition = FALSE;

	if (Proc_Find(gProcScr_SSPageSlide) || gStatScreen.inTransition)
		return;

	page = gStatScreen.page + direction;
	if (page < 0)
		page = pageAmt - 1;
	else if (page >= pageAmt)
		page = 0;

	gStatScreen.page = page;
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

static void SkillTree_ResetState(void)
{
	u8 pid = gStatScreen.unit != NULL ? UNIT_CHAR_ID(gStatScreen.unit) : 0xFF;

	gSkillTreeCursor = 0;
	gSkillTreeCursorPid = pid;
	gSkillTreeLastPage = gStatScreen.page;
	gSkillTreeConfirming = false;
	gSkillTreeConfirmChoice = 0;
	gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
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
	int iconPal;

	if (node->sid == 0)
		return;

	if (learned || available)
		iconPal = GetSkillIconPal(node->sid);
	else
		iconPal = 0;

	DrawIcon(
		gUiTmScratchA + TILEMAP_INDEX(node->x, node->y),
		SKILL_ICON(node->sid),
		TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + iconPal)
	);
}

static void SkillTree_DrawConfirmPrompt(const struct SkillTreeNode *node)
{
	int yesColor = gSkillTreeConfirmChoice == 0 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY;
	int noColor = gSkillTreeConfirmChoice == 1 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_GRAY;

	SkillTree_DrawText(SKILL_TREE_TEXT_MSG, 1, 16, TEXT_COLOR_SYSTEM_GOLD, "Learn?");
	PutNumber(
		gUiTmScratchA + TILEMAP_INDEX(7, 16),
		TEXT_COLOR_SYSTEM_BLUE,
		node != NULL ? GetSkillTreeSpCost(node->sid) : 0
	);
	SkillTree_DrawText(SKILL_TREE_TEXT_YES, 10, 16, yesColor, "Yes");
	SkillTree_DrawText(SKILL_TREE_TEXT_NO, 13, 16, noColor, "No");
}

static void SkillTree_DrawSp(struct NewBwl *bwl)
{
	SkillTree_DrawText(SKILL_TREE_TEXT_SP, 14, 16, TEXT_COLOR_SYSTEM_GOLD, "SP");
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
		SkillTree_DrawText(SKILL_TREE_TEXT_COST, 1, 16, TEXT_COLOR_SYSTEM_GOLD, "Cost");
		PutNumber(
			gUiTmScratchA + TILEMAP_INDEX(5, 16),
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
}

static void SkillTree_Redraw(void)
{
	if (!SkillTree_IsActivePage())
		return;

	SkillTree_FlushToScreen();
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
		SkillTree_Redraw();
		return true;
	}

	if (gKeyStatusPtr->newKeys & (DPAD_LEFT | DPAD_UP)) {
		gSkillTreeConfirmChoice = 0;
		PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		SkillTree_Redraw();
		return true;
	}

	if (gKeyStatusPtr->newKeys & (DPAD_RIGHT | DPAD_DOWN)) {
		gSkillTreeConfirmChoice = 1;
		PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		SkillTree_Redraw();
		return true;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		if (gSkillTreeConfirmChoice == 0) {
			struct Unit *unit = gStatScreen.unit;
			const struct UnitSkillTree *tree = GetUnitSkillTree(unit);

			if (SkillTree_TryLearn(unit, tree)) {
				PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
			} else {
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
			}
		} else {
			gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
			PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		}

		gSkillTreeConfirming = false;
		SkillTree_Redraw();
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

	/*
	 * Page-slide alone can leave the skill-tree panel blank; flush once per idle
	 * frame after the slide ends so icons/title/SP always appear.
	 */
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

	/* Left / right always change pages (wrap: 5/5 → 1/5). Prefer newKeys to avoid skip. */
	if (gKeyStatusPtr->newKeys & DPAD_LEFT) {
		HideSysHandCursor();
		StatScreen_ChangePage(proc, -1);
		return true;
	}

	if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
		HideSysHandCursor();
		StatScreen_ChangePage(proc, +1);
		return true;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		const struct SkillTreeNode *node = SkillTree_GetCursorNode(tree);

		if (node != NULL) {
			if (IsSkillTreeNodeLearned(unit, node)) {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_LEARNED;
				SkillTree_Redraw();
			} else if (IsSkillTreeNodeAvailable(unit, tree, gSkillTreeCursor)) {
				gSkillTreeConfirming = true;
				gSkillTreeConfirmChoice = 0;
				gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
				SkillTree_Redraw();
			} else if (IsSkillTreeNodeLocked(unit, tree, gSkillTreeCursor)) {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_LOCKED;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				SkillTree_Redraw();
			} else {
				gSkillTreeMessage = SKILL_TREE_MESSAGE_NO_SP;
				PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				SkillTree_Redraw();
			}
		}

		return true;
	}

	/* Up / down move the tree cursor (left/right are reserved for pages). */
	if (tree != NULL && (gKeyStatusPtr->repeatedKeys & (DPAD_UP | DPAD_DOWN))) {
		int direction = (gKeyStatusPtr->repeatedKeys & DPAD_UP)
			? SKILL_TREE_DIR_UP
			: SKILL_TREE_DIR_DOWN;
		int next = tree->nodes[gSkillTreeCursor].adj[direction];

		if (next != SKILL_TREE_NODE_NONE && next < tree->count && next < SKILL_TREE_MAX_NODES) {
			gSkillTreeCursor = next;
			gSkillTreeMessage = SKILL_TREE_MESSAGE_NONE;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
			/* Flush already ran this frame; redraw for new selection. */
			SkillTree_Redraw();
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
