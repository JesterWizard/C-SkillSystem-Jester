#include <common-chax.h>
#include <skill-system.h>
#include <popup-reowrk.h>

/*
 * The class change waits for its battle animation to end. The skills the promotion taught (TryAddSkillPromotion) are
 * normally shown by the animation's own popups (gPromotPopupConfigs); any still pending are shown here, one map popup
 * each, set up as the popup configs do (PopR_SetupLearnSkill: the skill and the unit). Before, one popup was started
 * without that setup: with no popup unit set, its unit name failed the IS_EWRAM_PTR assertion and the game reset.
 */
LYN_REPLACE_CHECK(ClassChgPostConfirmWaitBanimEnd);
void ClassChgPostConfirmWaitBanimEnd(struct ProcClassChgPostConfirm *proc)
{
	if (proc->game_lock != GetGameLock())
		return;

	/* The popup blocks this proc; the repeat comes back for the next skill */
	if (PopR_SetupLearnSkill()) {
		NewPopup_Simple(PopupScr_LearnSkill, 0x60, 0, proc);
		return;
	}

	Proc_Break(proc);
}
