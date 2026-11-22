#include "common-chax.h"
#include "tutorials.h"
#include "constants/texts.h"

/*
 * Extras > Tutorials' list, in order: a title (the banner's text, up to 128 pixels) and a text, which the screen wraps
 * to its box and shows four lines a page. The text is at the end of Contents/Texts/Source/texts/Districts.txt.
 */
const struct Tutorial gTutorials[] = {
	{ MSG_TUT_INTRO_TITLE, MSG_TUT_INTRO },
	{ MSG_TUT_SKILLS_TITLE, MSG_TUT_SKILLS },
	{ MSG_TUT_MAGIC_TITLE, MSG_TUT_MAGIC },
	{ MSG_TUT_ARTS_TITLE, MSG_TUT_ARTS },
	{ MSG_TUT_WEAPONS_TITLE, MSG_TUT_WEAPONS },
	{ MSG_TUT_DISTRICTS_TITLE, MSG_TUT_DISTRICTS },
	{ MSG_TUT_FISHING_TITLE, MSG_TUT_FISHING },
	{ MSG_TUT_CROSSING_TITLE, MSG_TUT_CROSSING },
	{ MSG_TUT_TOWER_TITLE, MSG_TUT_TOWER },
	{ MSG_TUT_HEX_TITLE, MSG_TUT_HEX },
	{ MSG_TUT_GUILD_TITLE, MSG_TUT_GUILD },
	{ MSG_TUT_PROMOTE_TITLE, MSG_TUT_PROMOTE },
	{ MSG_TUT_STATUS_TITLE, MSG_TUT_STATUS },
	{ MSG_TUT_THREAT_TITLE, MSG_TUT_THREAT },
	{ 0 },
};
