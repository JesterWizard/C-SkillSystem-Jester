#include "common-chax.h"
#include "kernel-lib.h"
#include "wm-story-chapter.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/maps.h"

/*
 * The Tower of Valni, one floor at a time (valni_floors_once, Data/DesignerConfig/designer-config.c).
 *
 * Each floor can be challenged once. Clearing one goes straight on to the next floor not cleared yet, if the story has
 * unlocked it (gValniFloorUnlockChapter), with no "Continue?" prompt; otherwise back to the world map. The tower's node
 * then starts the first floor not cleared yet, once it is unlocked: clear floors 1-5, and floor 6 is where the next visit
 * starts, when the story reaches it. With every unlocked floor cleared, the node starts nothing.
 *
 * In the Creature Campaign (the postgame) the floors are vanilla's again: any floor, any number of times.
 *
 * Floor n is chapter VALNI_FIRST_FLOOR + n (vanilla's 0x24..0x2B). A floor's cleared mark is permanent flag
 * GLOBAL_FLAG_VALNI_FLOOR_1 + n, saved with the game. Each floor's ending calls vanilla's next-floor script
 * (0x089EE920, the "Continue?" prompt), and floor 8's the tower-cleared one (0x089EE97C): ValniFloors.event makes them
 * call the scripts below.
 */
#define VALNI_FIRST_FLOOR 0x24
#define VALNI_FLOORS 8

/* Vanilla's event commands, as words (they are written out in ROM; see 0x089EE920) */
#define EV_ASMC(fn) 0x00000D40, (u32)(fn)
#define EV_ENDA 0x00000120


static bool Valni_OnceMode(void)
{
	return gpKernelDesignerConfig->valni_floors_once && !(gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME);
}

static int Valni_FloorOf(int chapter)
{
	if (chapter < VALNI_FIRST_FLOOR || chapter >= VALNI_FIRST_FLOOR + VALNI_FLOORS)
		return -1;
	return chapter - VALNI_FIRST_FLOOR;
}

static bool Valni_FloorCleared(int floor)
{
	return CheckFlag(GLOBAL_FLAG_VALNI_FLOOR_1 + floor) != 0;
}

static bool Valni_FloorUnlocked(int floor)
{
	int chapter = gValniFloorUnlockChapter[floor];

	return chapter != PREP_MENU_LOCKED && WmStoryReached(chapter);
}

/* The chapter of the floor to challenge next, or -1: the first not cleared, if it is unlocked */
static int Valni_NextFloorChapter(void)
{
	int floor;

	for (floor = 0; floor < VALNI_FLOORS; floor++) {
		if (Valni_FloorCleared(floor))
			continue;
		return Valni_FloorUnlocked(floor) ? VALNI_FIRST_FLOOR + floor : -1;
	}
	return -1;
}

/* GetWmLocChapter (WmStoryChapter.c): the tower's node starts the next floor, or nothing (0xFF) */
int ValniFloors_NodeChapter(int chapter)
{
	int next;

	if (chapter != VALNI_FIRST_FLOOR || !Valni_OnceMode())
		return chapter;

	next = Valni_NextFloorChapter();
	return next < 0 ? 0xFF : next;
}

static void Valni_MarkCleared(void)
{
	int floor = Valni_FloorOf(gPlaySt.chapterIndex);

	if (floor >= 0)
		SetFlag(GLOBAL_FLAG_VALNI_FLOOR_1 + floor);
}

/*
 * ASMC at a floor's end: the floor is cleared. Slot C: 0 for vanilla's prompt (once mode off), else the next floor's
 * chapter to go on to (slots 2 and 3, for vanilla's move command), or 1 for the world map.
 */
void ValniFloors_FloorEnd(ProcPtr proc)
{
	int next;

	if (!Valni_OnceMode()) {
		gEventSlots[EVT_SLOT_C] = 0;
		return;
	}

	Valni_MarkCleared();
	next = Valni_NextFloorChapter();
	if (next < 0) {
		gEventSlots[EVT_SLOT_C] = 1;
		return;
	}
	gEventSlots[EVT_SLOT_2] = next;
	gEventSlots[EVT_SLOT_3] = next;
	gEventSlots[EVT_SLOT_C] = next;
}

/* Floor 8's end: cleared too, then vanilla's congratulations */
void ValniFloors_TowerCleared(ProcPtr proc)
{
	if (Valni_OnceMode())
		Valni_MarkCleared();
	DisplayCongratulationsMapAnim(proc);
}

/*
 * The next-floor script. Its first words are vanilla's 0x089EE920 again for the vanilla path (once mode off); the once
 * mode skips the prompt: the next floor (vanilla's "yes": move to the chapter in slot 2, then UnlockPostgameAllyByEnemyCount)
 * or the world map (vanilla's "no": MNTS, then StartRetreatProcessing).
 */
/* Vanilla's 0x089EE920, as it was: "Continue?", then the next floor or the world map */
static const u32 EventScr_ValniFloorEnd_Vanilla[] = {
	0x00041020, EV_ASMC(DisplayMapClearMapAnim), 0x00001A23, 0x000B0540, 0xFFFFFFFF, 0x08FD1B20, 0x00001D20,
	0x00070540, 0x00000001, 0x00000C40, 0x0007000C, 0xFFFF2A21, EV_ASMC(StartRetreatProcessing), 0x00010920,
	0x00000820, 0x00320620, 0xFFFF2A23, EV_ASMC(UnlockPostgameAllyByEnemyCount), 0x00010820, EV_ENDA,
};

/* Once mode: no prompt. The next floor (vanilla's "yes": the chapter in slot 2) or the world map (vanilla's "no") */
static const u32 EventScr_ValniFloorEnd_Once[] = {
	EV_ASMC(DisplayMapClearMapAnim),
	0x00070540, 0x00000001,		/* SVAL slot 7, 1 */
	0x00010C41, 0x0007000C,		/* BNE label 1: slot C != 1 (a next floor) */
	0xFFFF2A21, EV_ASMC(StartRetreatProcessing),
	0x00020920,			/* GOTO label 2 */
	0x00010820,			/* label 1 */
	0x00320620, 0xFFFF2A23, EV_ASMC(UnlockPostgameAllyByEnemyCount),
	0x00020820,			/* label 2 */
	EV_ENDA,
};

/* Floors 1-7's endings call this (ValniFloors.event): the vanilla script or the once mode's */
const u32 EventScr_ValniFloorEnd[] = {
	EV_ASMC(ValniFloors_FloorEnd),
	0x00010C41, 0x0000000C,		/* BNE label 1: slot C != 0 (once mode) */
	0x00000A40, (u32)EventScr_ValniFloorEnd_Vanilla,
	0x00020920,			/* GOTO label 2 */
	0x00010820,			/* label 1 */
	0x00000A40, (u32)EventScr_ValniFloorEnd_Once,
	0x00020820,			/* label 2 */
	EV_ENDA,
};
