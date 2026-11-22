#include "common-chax.h"
#include "tutorials.h"
#include "constants/texts.h"
#include "TutorialTitles.h"

/*
 * Extras > Tutorials' list, in order: a title (its message, and its image, which Tools/tutorial-titles/titles.py
 * draws from the message: up to 128 pixels) and a text, which the screen wraps to its box. Keep a text to the box's
 * five lines (a longer one shows four lines a page). The texts are at the end of Contents/Texts/Source/texts/Districts.txt.
 */
const struct Tutorial gTutorials[] = {
	{ MSG_TUT_INTRO_TITLE, MSG_TUT_INTRO, Img_TutTitle_INTRO },
	{ MSG_TUT_SKILLS_TITLE, MSG_TUT_SKILLS, Img_TutTitle_SKILLS },
	{ MSG_TUT_EQUIP_TITLE, MSG_TUT_EQUIP, Img_TutTitle_EQUIP },
	{ MSG_TUT_CAPACITY_TITLE, MSG_TUT_CAPACITY, Img_TutTitle_CAPACITY },
	{ MSG_TUT_SCROLLS_TITLE, MSG_TUT_SCROLLS, Img_TutTitle_SCROLLS },
	{ MSG_TUT_MAGIC_TITLE, MSG_TUT_MAGIC, Img_TutTitle_MAGIC },
	{ MSG_TUT_CASTING_TITLE, MSG_TUT_CASTING, Img_TutTitle_CASTING },
	{ MSG_TUT_USES_TITLE, MSG_TUT_USES, Img_TutTitle_USES },
	{ MSG_TUT_ARTS_TITLE, MSG_TUT_ARTS, Img_TutTitle_ARTS },
	{ MSG_TUT_ARTUSE_TITLE, MSG_TUT_ARTUSE, Img_TutTitle_ARTUSE },
	{ MSG_TUT_WEAPONS_TITLE, MSG_TUT_WEAPONS, Img_TutTitle_WEAPONS },
	{ MSG_TUT_DISTRICTS_TITLE, MSG_TUT_DISTRICTS, Img_TutTitle_DISTRICTS },
	{ MSG_TUT_OPENING_TITLE, MSG_TUT_OPENING, Img_TutTitle_OPENING },
	{ MSG_TUT_VISIT_TITLE, MSG_TUT_VISIT, Img_TutTitle_VISIT },
	{ MSG_TUT_FISHING_TITLE, MSG_TUT_FISHING, Img_TutTitle_FISHING },
	{ MSG_TUT_REEL_TITLE, MSG_TUT_REEL, Img_TutTitle_REEL },
	{ MSG_TUT_CATCH_TITLE, MSG_TUT_CATCH, Img_TutTitle_CATCH },
	{ MSG_TUT_CROSSING_TITLE, MSG_TUT_CROSSING, Img_TutTitle_CROSSING },
	{ MSG_TUT_CROSSRULE_TITLE, MSG_TUT_CROSSRULE, Img_TutTitle_CROSSRULE },
	{ MSG_TUT_TOWER_TITLE, MSG_TUT_TOWER, Img_TutTitle_TOWER },
	{ MSG_TUT_TOWERRULE_TITLE, MSG_TUT_TOWERRULE, Img_TutTitle_TOWERRULE },
	{ MSG_TUT_HEX_TITLE, MSG_TUT_HEX, Img_TutTitle_HEX },
	{ MSG_TUT_HEXRULE_TITLE, MSG_TUT_HEXRULE, Img_TutTitle_HEXRULE },
	{ MSG_TUT_GUILD_TITLE, MSG_TUT_GUILD, Img_TutTitle_GUILD },
	{ MSG_TUT_TOKENS_TITLE, MSG_TUT_TOKENS, Img_TutTitle_TOKENS },
	{ MSG_TUT_PROMOTE_TITLE, MSG_TUT_PROMOTE, Img_TutTitle_PROMOTE },
	{ MSG_TUT_PROMOPT_TITLE, MSG_TUT_PROMOPT, Img_TutTitle_PROMOPT },
	{ MSG_TUT_STATUS_TITLE, MSG_TUT_STATUS, Img_TutTitle_STATUS },
	{ MSG_TUT_PAGES_TITLE, MSG_TUT_PAGES, Img_TutTitle_PAGES },
	{ MSG_TUT_THREAT_TITLE, MSG_TUT_THREAT, Img_TutTitle_THREAT },
	{ 0 },
};
