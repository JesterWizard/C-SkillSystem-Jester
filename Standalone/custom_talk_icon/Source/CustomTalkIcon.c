#include "global.h"
#include "bmudisp.h"
#include "bmunit.h"
#include "bmreliance.h"
#include "chapterdata.h"
#include "constants/event-flags.h"
#include "ctc.h"
#include "eventinfo.h"
#include "eventscript.h"
#include "functions.h"
#include "types.h"
#include "variables.h"

typedef void (*VoidFunc)(void);
#define Vanilla_PutUnitSpriteIconsOam ((VoidFunc)0x080275E9)

#define EVT_CMD_LO(cmd) (((cmd) & 0x0000FFFF))
#define EVT_CMD_HI(cmd) (((cmd) & 0xFFFF0000) >> 16)

extern struct EventListCmdInfo CONST_DATA gEventListCmdInfoTable[];

struct EvCheck03 {
	u32 unk0;
	u32 script;
	u8 pidA;
	u8 pidB;
	u16 fillerA;
	u16 unkC;
	u16 unkE;
};

static bool UnitOnMapAvailable(struct Unit *unit)
{
	return UNIT_IS_VALID(unit) && !(unit->state & (US_UNAVAILABLE | US_HIDDEN));
}

static u8 GetTalkee(struct Unit *unit)
{
	int i;
	const struct EventListCmdInfo *cmd_info = &gEventListCmdInfoTable[EVT_LIST_CMD_CHAR];
	const EventListScr *list = GetChapterEventDataPointer(gPlaySt.chapterIndex)->characterBasedEvents;

	for (;;) {
		u8 cmd = EVT_CMD_LO(list[0]);

		if (cmd == EVT_LIST_CMD_END)
			break;

		if (cmd != EVT_LIST_CMD_CHAR)
			continue;

		if (!CheckFlag(EVT_CMD_HI(list[0]))) {
			const struct EvCheck03 *_chunk = (const void *)list;
			struct EventInfo info = {
				.listScript = list,
				.pidA = UNIT_CHAR_ID(unit),
				.pidB = _chunk->pidB,
			};

			if (cmd_info->func(&info) == true) {
				struct Unit *talkee = GetUnitFromCharId(_chunk->pidB);

				if (UnitOnMapAvailable(talkee))
					return _chunk->pidB;
			}
		}

		list += cmd_info->length;
	}

	for (i = 0; i < GetUnitSupporterCount(unit); i++) {
		struct Unit *talkee = GetUnitSupporterUnit(unit, i);

		if (UnitOnMapAvailable(talkee) && CanUnitSupportNow(unit, i))
			return UNIT_CHAR_ID(talkee);
	}

	return 0;
}

static void PutCustomTalkIcon(int ix, int iy)
{
	CallARM_PushToSecondaryOAM(
		OAM1_X(0x200 + ix - 6),
		OAM0_Y(0x100 + iy - 14),
		gObject_32x8,
		OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xB00 / 0x20));

	CallARM_PushToSecondaryOAM(
		OAM1_X(0x200 + ix - 6),
		OAM0_Y(0x100 + iy - 6),
		gObject_32x8,
		OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(0xF00 / 0x20));
}

void PutUnitSpriteIconsOam_CustomTalkIcon(void)
{
	int i;
	int cached_talkee_id = 0;
	bool have_cached_talkee = false;

	Vanilla_PutUnitSpriteIconsOam();

	if (CheckFlag(EVFLAG_HIDE_BLINKING_ICON))
		return;

	if (gBmSt.gameStateBits & BM_FLAG_1) {
		cached_talkee_id = GetTalkee(gActiveUnit);
		have_cached_talkee = (cached_talkee_id != 0);
	}

	if (!have_cached_talkee)
		return;

	for (i = 1; i <= 0xC0; i++) {
		struct Unit *unit = GetUnit(i);
		int ix;
		int iy;

		if (!UNIT_IS_VALID(unit) || (unit->state & US_HIDDEN))
			continue;

		if (!unit->pMapSpriteHandle || (unit->pMapSpriteHandle->config & 0x80))
			continue;

		if (UNIT_CHAR_ID(unit) != cached_talkee_id)
			continue;

		ix = unit->xPos * 16 - gBmSt.camera.x;
		iy = unit->yPos * 16 - gBmSt.camera.y;

		if (ix < -16 || ix > DISPLAY_WIDTH)
			continue;

		if (iy < -16 || iy > DISPLAY_HEIGHT)
			continue;

		PutCustomTalkIcon(ix, iy);
	}
}
