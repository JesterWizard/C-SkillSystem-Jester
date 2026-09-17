#include "common-chax.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "popup-reowrk.h"
#include "constants/texts.h"

#define SKILL_POPSTACK_DEPTH 8
#define WRANK_POPSTACK_DEPTH 8

struct PopupSkillStack {
	int cur;
	u16 sids[SKILL_POPSTACK_DEPTH];
};

struct WRankPopupStack {
	int cur;
	u16 sids[WRANK_POPSTACK_DEPTH];
	u8 levels[WRANK_POPSTACK_DEPTH];
};

extern struct PopupSkillStack sPopupSkillStack;
extern struct WRankPopupStack sWRankPopupStack;

static u16 GetPopupSkillSid(void)
{
	u16 item = gPopupItem;

	if (IsSkillScrollItem(item)) {
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
		if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
			return ITEM_USES(item);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
		if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
			return ITEM_USES(item) + 0xFF;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
		if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
			return ITEM_USES(item) + 0x1FF;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
		if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
			return ITEM_USES(item) + 0x2FF;
#endif
	}

	if (COMMON_SKILL_VALID(item))
		return item;

	return 0;
}

void ResetPopupSkillStack(void)
{
	memset(&sPopupSkillStack, 0, sizeof(struct PopupSkillStack));

	sPopupSkillStack.cur = 0;
	memset(&sWRankPopupStack, 0, sizeof(struct WRankPopupStack));
}

void PushSkillListStack(u16 sid)
{
	if (sPopupSkillStack.cur < SKILL_POPSTACK_DEPTH)
		sPopupSkillStack.sids[sPopupSkillStack.cur++] = sid;
}

void PushWRankSkillPopup(u16 sid, int newLevel)
{
	if (sWRankPopupStack.cur < WRANK_POPSTACK_DEPTH) {
		sWRankPopupStack.sids[sWRankPopupStack.cur] = sid;
		sWRankPopupStack.levels[sWRankPopupStack.cur] = newLevel;
		sWRankPopupStack.cur++;
	}
}

int PopSkillListStack(void)
{
	if (sPopupSkillStack.cur > 0)
		return sPopupSkillStack.sids[--sPopupSkillStack.cur];

	return 0;
}

bool SkillPopupHasPendingSkills(void)
{
	return (sPopupSkillStack.cur > 0);
}

/**
 * Components
 */
int PoprGetLen_SkillIcon(struct PopupProc *proc, const struct PopupInstruction *inst)
{
	u16 sid = GetPopupSkillSid();

	proc->iconX = proc->xGfxSize;
	proc->iconId = SKILL_ICON(sid);
	LoadIconPalette(GetSkillIconPal(sid), proc->iconPalId);
	return 0x10;
}

void PoprDisp_SkillIcon(struct Text *text, const struct PopupInstruction *inst)
{
	Text_Skip(text, 0x10);
}

int PoprGetLen_SkillName(struct PopupProc *proc, const struct PopupInstruction *inst)
{
	return GetStringTextLen(GetSkillNameStr(GetPopupSkillSid()));
}

void PoprDisp_SkillName(struct Text *text, const struct PopupInstruction *inst)
{
	Text_DrawString(text, GetSkillNameStr(GetPopupSkillSid()));
}

int PoprGetLen_WTypeName(struct PopupProc *proc, const struct PopupInstruction *inst)
{
	return GetStringTextLen(GetWeaponTypeDisplayString(gPopupNumber));
}

void PoprDisp_WTypeName(struct Text *text, const struct PopupInstruction *inst)
{
	Text_DrawString(text, GetWeaponTypeDisplayString(gPopupNumber));
}

/**
 * Configs
 */
bool PopR_SetupLearnSkill(void)
{
	int sid = PopSkillListStack();

	if (COMMON_SKILL_VALID(sid)) {
		SetPopupItem(sid);
		if (!gpPopupUnit)
			SetPopupUnit(gActiveUnit);
		return true;
	}

	return false;
}

bool PopR_SetupWRankSkillUpgrade(void)
{
	u16 sid;

	if (sWRankPopupStack.cur <= 0)
		return false;

	sWRankPopupStack.cur--;
	sid = sWRankPopupStack.sids[sWRankPopupStack.cur];
	if (!COMMON_SKILL_VALID(sid))
		return false;

	SetPopupItem(sid);
	SetPopupNumber(sWRankPopupStack.levels[sWRankPopupStack.cur]);
	if (!gpPopupUnit)
		SetPopupUnit(gActiveUnit);
	return true;
}

struct PopupInstruction const PopupScr_LearnSkill[] = {
    POPUP_SOUND(0x5A),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_Learned),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	CHAX_POPUP_SKILL_ICON,
    POPUP_SPACE(2),
	CHAX_POPUP_SKILL_NAME,
    POPUP_END
};

struct PopupInstruction const PopupScr_WRankSkillUpgrade[] = {
	POPUP_SOUND(0x5A),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	CHAX_POPUP_SKILL_ICON,
	POPUP_SPACE(2),
	CHAX_POPUP_SKILL_NAME,
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_WRankSkill_Upgraded),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	POPUP_NUM,
	POPUP_END
};
