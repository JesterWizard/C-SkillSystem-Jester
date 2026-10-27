#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "map-task.h"
#include "constants/skills.h"

#if defined(SID_Forage) && (COMMON_SKILL_VALID(SID_Forage))

/**
 * Forage: while a deployed player unit has the skill, the tiles of every
 * hidden-item area that is still buried twinkle with sparkles.
 *
 * A hidden-item area is an AREA misc event whose script calls
 * EventScr_GiveTreasureToLuckyDog, like the desert of chapter 15. Its flag
 * is set once the item is dug up; a failed luck roll clears it again.
 */

enum forage_shimmer {
	/* Each tile runs a loop of FORAGE_CYCLE steps, 4 frames a step. */
	FORAGE_CYCLE = 32,
	/* A sparkle shows for FORAGE_SPARK_STEPS steps of that loop. */
	FORAGE_SPARK_STEPS = 5,
	/* Commands to scan in an AREA event's script before giving up. */
	FORAGE_SCRIPT_SCAN = 32,
};

/* Small, medium, large, medium, small (OBJ chr in the map task sheet). */
static const u16 sForageSparkChr[FORAGE_SPARK_STEPS] = {
	0x3A, 0x3F, 0x5F, 0x3F, 0x3A,
};

static bool IsForageTreasureScript(const u16 *scr)
{
	int i;

	for (i = 0; i < FORAGE_SCRIPT_SCAN; i++) {
		int len = EVT_CMD_LEN(scr);

		if (scr[0] == _EvtCmd(EV_CMD_CALL, 4, 0) &&
		    ((u32)scr[2] | ((u32)scr[3] << 16)) == (u32)EventScr_GiveTreasureToLuckyDog)
			return true;

		if ((scr[0] >> 8) == EV_CMD_END || len == 0)
			break;

		scr += len;
	}
	return false;
}

static bool IsForageUnitDeployed(void)
{
	int i;

	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 0x40; i++) {
		struct Unit *unit = gUnitLookup[i];

		if (!UNIT_IS_VALID(unit) || (unit->state & (US_UNAVAILABLE | US_HIDDEN)))
			continue;

		if (SkillTester(unit, SID_Forage))
			return true;
	}
	return false;
}

static void PutForageAreaOam(int x1, int y1, int x2, int y2, int clock)
{
	int x, y;

	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			/* Neighbouring tiles are far apart in the loop, so the patch twinkles. */
			int step = (clock + x * 7 + y * 13) & (FORAGE_CYCLE - 1);
			int ix, iy;

			if (step >= FORAGE_SPARK_STEPS)
				continue;

			ix = x * 16 - gBmSt.camera.x + ((x * 5 + y * 3) & 7);
			iy = y * 16 - gBmSt.camera.y + ((x * 3 + y * 5) & 7);

			if (ix < -8 || ix >= DISPLAY_WIDTH || iy < -8 || iy >= DISPLAY_HEIGHT)
				continue;

			CallARM_PushToSecondaryOAM(
				OAM1_X(ix), OAM0_Y(iy), gObject_8x8,
				OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(sForageSparkChr[step]));
		}
	}
}

void PutForageShimmerOam(void)
{
	const EventListScr *list = GetChapterEventDataPointer(gPlaySt.chapterIndex)->miscBasedEvents;
	bool unit_checked = false;
	int clock = GetGameClock() >> 2;

	if (!list)
		return;

	for (;;) {
		u16 cmd = EVT_CMD_LO(list[0]);
		u16 flag = EVT_CMD_HI(list[0]);

		if (cmd == EVT_LIST_CMD_END || cmd > EVT_LIST_CMD_10)
			break;

		if (cmd == EVT_LIST_CMD_AREA && (flag == 0 || !CheckFlag(flag)) &&
		    IsForageTreasureScript((const u16 *)list[1])) {
			/* Only look for a Forage unit on maps that have something buried. */
			if (!unit_checked) {
				if (!IsForageUnitDeployed())
					return;
				unit_checked = true;
			}
			PutForageAreaOam(list[2] & 0xFF, (list[2] >> 8) & 0xFF,
					 (list[2] >> 16) & 0xFF, (list[2] >> 24) & 0xFF, clock);
		}
		list += gEventListCmdInfoTable[cmd].length;
	}
}

#endif
