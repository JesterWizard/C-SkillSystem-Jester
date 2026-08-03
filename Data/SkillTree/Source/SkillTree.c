#include "common-chax.h"
#include "bwl.h"
#include "constants/characters.h"
#include "constants/skills.h"
#include "kernel/skill-tree.h"
#include "skill-system.h"

/*
 * PoC skills stay in the 0x01-0xFE equippable range so icons, names, and AddSkill
 * all use the same dynamic skill path.
 */
const struct SkillTreeSpCost gSkillTreeSpCostTable[] = {
	{ SID_LifeAndDeath,  5 },
	{ SID_StrongRiposte, 10 },
	{ SID_FaireSword,    10 },
	{ SID_Patience,      10 },
	{ SID_Avoid,         10 },
	{ SID_Vantage,       15 },
	{ SID_Renewal,       15 },
	{ SID_Desperation,   15 },
	{ SID_FlashingBlade, 20 },
	{ SID_LunaAttack,    20 },
	{ SID_Adept,         20 },
	{ SID_Chivalry,      20 },
	{ SID_QuickDraw,     20 },
	{ SID_Astra,         25 },
	{ SID_Sol,           25 },
	{ SID_RightfulKing,  25 },
	{ 0,                  0 },
};

const struct UnitSkillTree gUnitSkillTreeTable[] = {
	{
		.pid = CHARACTER_EIRIKA,
		.count = 16,
		.nodes = {
			{
				.sid = SID_LifeAndDeath,
				.path = 0,
				.parent = SKILL_TREE_NODE_NONE,
				.x = 7,
				.y = 13,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_StrongRiposte,
				.path = 0,
				.parent = 0,
				.x = 7,
				.y = 11,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_FaireSword,
				.path = 1,
				.parent = 1,
				.x = 1,
				.y = 9,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Patience,
				.path = 2,
				.parent = 1,
				.x = 7,
				.y = 9,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Avoid,
				.path = 3,
				.parent = 1,
				.x = 13,
				.y = 9,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Vantage,
				.path = 1,
				.parent = 2,
				.x = 1,
				.y = 7,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Renewal,
				.path = 2,
				.parent = 3,
				.x = 7,
				.y = 7,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Desperation,
				.path = 3,
				.parent = 4,
				.x = 13,
				.y = 7,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_FlashingBlade,
				.path = 1,
				.parent = 5,
				.x = 1,
				.y = 5,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_LunaAttack,
				.path = 1,
				.parent = 5,
				.x = 4,
				.y = 5,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Adept,
				.path = 2,
				.parent = 6,
				.x = 7,
				.y = 5,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Chivalry,
				.path = 3,
				.parent = 7,
				.x = 13,
				.y = 5,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_QuickDraw,
				.path = 3,
				.parent = 7,
				.x = 16,
				.y = 5,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Astra,
				.path = 1,
				.parent = 5,
				.x = 1,
				.y = 3,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_Sol,
				.path = 2,
				.parent = 10,
				.x = 7,
				.y = 3,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
			{
				.sid = SID_RightfulKing,
				.path = 3,
				.parent = 7,
				.x = 13,
				.y = 3,
				.adj = {
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
					SKILL_TREE_NODE_NONE,
				},
			},
		},
	},
	{ 0 },
};

u8 GetSkillTreeSpCost(u16 sid)
{
	for (int i = 0; gSkillTreeSpCostTable[i].sid != 0; ++i) {
		if (gSkillTreeSpCostTable[i].sid == sid)
			return gSkillTreeSpCostTable[i].cost;
	}

	return 0;
}

const struct UnitSkillTree *GetUnitSkillTree(struct Unit *unit)
{
	if (unit == NULL)
		return NULL;

	for (int i = 0; gUnitSkillTreeTable[i].pid != 0; ++i) {
		if (gUnitSkillTreeTable[i].pid == UNIT_CHAR_ID(unit))
			return &gUnitSkillTreeTable[i];
	}

	return NULL;
}

u8 GetSkillTreeChosenPath(const struct UnitSkillTree *tree, struct Unit *unit)
{
	if (tree == NULL || unit == NULL)
		return 0;

	for (int i = 0; i < tree->count && i < SKILL_TREE_MAX_NODES; ++i) {
		const struct SkillTreeNode *node = &tree->nodes[i];

		if (node->path != 0 && IsSkillTreeNodeLearned(unit, node))
			return node->path;
	}

	return 0;
}

bool IsSkillTreeNodeLearned(struct Unit *unit, const struct SkillTreeNode *node)
{
	return unit != NULL && node != NULL && node->sid != 0 && IsSkillLearned(unit, node->sid);
}

static bool SkillTreeNodePassesPathAndParentChecks(
	struct Unit *unit,
	const struct UnitSkillTree *tree,
	int nodeIndex
)
{
	const struct SkillTreeNode *node;
	u8 chosenPath;

	if (unit == NULL || tree == NULL || nodeIndex < 0 || nodeIndex >= tree->count || nodeIndex >= SKILL_TREE_MAX_NODES)
		return false;

	node = &tree->nodes[nodeIndex];

	if (node->sid == 0 || IsSkillTreeNodeLearned(unit, node))
		return false;

	chosenPath = GetSkillTreeChosenPath(tree, unit);
	if (chosenPath != 0 && node->path != 0 && node->path != chosenPath)
		return false;

	if (node->parent != SKILL_TREE_NODE_NONE) {
		if (node->parent >= tree->count || node->parent >= SKILL_TREE_MAX_NODES)
			return false;

		if (!IsSkillTreeNodeLearned(unit, &tree->nodes[node->parent]))
			return false;
	}

	return true;
}

bool IsSkillTreeNodeAvailable(struct Unit *unit, const struct UnitSkillTree *tree, int nodeIndex)
{
	const struct SkillTreeNode *node;
	struct NewBwl *bwl;

	if (!SkillTreeNodePassesPathAndParentChecks(unit, tree, nodeIndex))
		return false;

	node = &tree->nodes[nodeIndex];
	bwl = GetNewBwl(UNIT_CHAR_ID(unit));

	return bwl != NULL && bwl->skillPoints >= GetSkillTreeSpCost(node->sid);
}

bool IsSkillTreeNodeLocked(struct Unit *unit, const struct UnitSkillTree *tree, int nodeIndex)
{
	if (unit == NULL || tree == NULL || nodeIndex < 0 || nodeIndex >= tree->count || nodeIndex >= SKILL_TREE_MAX_NODES)
		return true;

	if (IsSkillTreeNodeLearned(unit, &tree->nodes[nodeIndex]))
		return false;

	return !SkillTreeNodePassesPathAndParentChecks(unit, tree, nodeIndex);
}
