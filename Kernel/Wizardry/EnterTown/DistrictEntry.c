#include "common-chax.h"

/*
 * Entering a district from the world map's node menu (WMMenu_OnDistrictSelected, EnterTown.c).
 *
 * The world map's chapter start (ProcScr_WorldMapMain label 0x0E) sets the chapter from the district override
 * (WorldMap_CallIntroEvent) and then removes the map at once (WorldMap_HideEverything), with no fade: the screen cut
 * from the full world map to black in one frame, the base chapters' "black screen flash". This proc fades the world
 * map to black first, with the world map blocked, then takes it to that label. The district map fades in from black.
 */
#define WM_LABEL_START_CHAPTER 0x0E

/* Fade speed in 1/16 steps per frame (q4): 0x20 takes about 8 frames, StartFastFadeToBlack about 3 */
#define DISTRICT_ENTRY_FADE_SPEED 0x20

static void DistrictEntryFade_Start(ProcPtr proc)
{
	(void)proc;
	StartFadeToBlack(DISTRICT_ENTRY_FADE_SPEED);
}

static void DistrictEntryFade_Go(ProcPtr proc)
{
	Proc_Goto(((struct Proc *)proc)->proc_parent, WM_LABEL_START_CHAPTER);
}

static const struct ProcCmd ProcScr_DistrictEntryFade[] = {
	PROC_NAME("DistrictEntryFade"),
	PROC_CALL(DistrictEntryFade_Start),
	PROC_REPEAT(WaitForFade),
	PROC_CALL(DistrictEntryFade_Go),
	PROC_END,
};

/* worldMapMain: the ProcScr_WorldMapMain proc */
void StartDistrictEntryFade(ProcPtr worldMapMain)
{
	Proc_StartBlocking(ProcScr_DistrictEntryFade, worldMapMain);
}
