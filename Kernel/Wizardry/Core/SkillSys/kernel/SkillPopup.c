#include "common-chax.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "popup-reowrk.h"
#include "constants/texts.h"

#define SKILL_POPSTACK_DEPTH 8

struct PopupSkillStack {
	int cur;
	u16 sids[SKILL_POPSTACK_DEPTH];
};

extern struct PopupSkillStack sPopupSkillStack;

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
}

void PushSkillListStack(u16 sid)
{
	if (sPopupSkillStack.cur < SKILL_POPSTACK_DEPTH)
		sPopupSkillStack.sids[sPopupSkillStack.cur++] = sid;
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

/**
 * Configs
 */
bool PopR_SetupLearnSkill(void)
{
	int sid = PopSkillListStack();

	if (COMMON_SKILL_VALID(sid)) {
		SetPopupItem(sid);
		return true;
	}

	return false;
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
