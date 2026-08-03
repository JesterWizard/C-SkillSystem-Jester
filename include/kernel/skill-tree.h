#pragma once

#include "common-chax.h"

#define SKILL_TREE_NODE_NONE 0xFF
#define SKILL_TREE_MAX_NODES 24

enum SkillTreeNodeDirection {
	SKILL_TREE_DIR_UP,
	SKILL_TREE_DIR_DOWN,
	SKILL_TREE_DIR_LEFT,
	SKILL_TREE_DIR_RIGHT,
};

struct SkillTreeSpCost {
	u16 sid;
	u8 cost;
};

struct SkillTreeNode {
	u16 sid;
	u8 path;
	u8 parent;
	u8 x;
	u8 y;
	u8 adj[4];
};

struct UnitSkillTree {
	u8 pid;
	u8 count;
	struct SkillTreeNode nodes[SKILL_TREE_MAX_NODES];
};

struct HelpBoxInfo;

extern EWRAM_DATA u8 gSkillTreeCursor;
extern EWRAM_DATA u8 gSkillTreeCursorPid;
extern EWRAM_DATA u8 gSkillTreeLastPage;
extern EWRAM_DATA u8 gSkillTreeConfirming;
extern EWRAM_DATA u8 gSkillTreeConfirmChoice;
extern EWRAM_DATA u8 gSkillTreeMessage;
extern EWRAM_DATA u8 gSkillTreePageDrawn;
extern EWRAM_DATA struct HelpBoxInfo gSkillTreeHelp;

extern const struct SkillTreeSpCost gSkillTreeSpCostTable[];
extern const struct UnitSkillTree gUnitSkillTreeTable[];

u8 GetSkillTreeSpCost(u16 sid);
const struct UnitSkillTree *GetUnitSkillTree(struct Unit *unit);
u8 GetSkillTreeChosenPath(const struct UnitSkillTree *tree, struct Unit *unit);
bool IsSkillTreeNodeLearned(struct Unit *unit, const struct SkillTreeNode *node);
bool IsSkillTreeNodeAvailable(struct Unit *unit, const struct UnitSkillTree *tree, int nodeIndex);
bool IsSkillTreeNodeLocked(struct Unit *unit, const struct UnitSkillTree *tree, int nodeIndex);
