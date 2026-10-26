#include "global.h"
#include "bmunit.h"
#include "statscreen.h"
#include "variables.h"

/**
 * One row of the description table defined in Installer.event.
 * The table ends at the first row whose pid is 0.
 */
struct VariableUnitDescription {
	u8 pid;      /* character ID */
	u8 chapter;  /* gPlaySt.chapterIndex this row applies to */
	u16 msg;     /* text ID shown instead of the character's description (0 = keep default) */
};

extern const struct VariableUnitDescription VariableUnitDescriptionTable[];

/* Vanilla fallback text when a character has no description */
#define MSG_STATSCREEN_NO_DESCRIPTION 0x6BE

static int GetVariableUnitDescription(int pid, int chapter)
{
	const struct VariableUnitDescription *it;

	for (it = VariableUnitDescriptionTable; it->pid != 0; it++)
		if (it->pid == pid && it->chapter == chapter)
			return it->msg;

	return 0;
}

/**
 * Replaces vanilla HbPopulate_SSCharacter (FE8U 0x08088BD4): the help box
 * shown for the character name on stat screen page 1.
 */
void HbPopulate_SSCharacter_Variable(struct HelpBoxProc *proc)
{
	const struct CharacterData *pinfo = gStatScreen.unit->pCharacterData;
	int msg = GetVariableUnitDescription(pinfo->number, (u8)gPlaySt.chapterIndex);

	if (msg == 0)
		msg = pinfo->descTextId;

	if (msg != 0)
		proc->mid = msg;
	else
		proc->mid = MSG_STATSCREEN_NO_DESCRIPTION;
}
