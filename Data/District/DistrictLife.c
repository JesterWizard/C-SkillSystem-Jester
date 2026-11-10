#include <common-chax.h>
#include "constants/characters.h"
#include "jester_headers/maps.h"
#include "kernel/district.h"

/*
 * District life (include/kernel/district.h, Kernel/Wizardry/EnterTown/DistrictLife.c): the time of day by the story
 * chapter, its tints, and the townsfolk's places.
 */

/* The time of day in every district, from each story chapter on (in story order; the last row reached wins) */
const struct DistrictTime gDistrictTimes[] = {
	{ 0x00, DISTRICT_TIME_DAY },
	{ CHAPTER_07, DISTRICT_TIME_EVENING },
	{ CHAPTER_08, DISTRICT_TIME_NIGHT },
	{ CHAPTER_09, DISTRICT_TIME_MORNING },
	{ CHAPTER_10, DISTRICT_TIME_DAY },
	{ CHAPTER_12, DISTRICT_TIME_EVENING },
	{ CHAPTER_13, DISTRICT_TIME_NIGHT },
	{ CHAPTER_15, DISTRICT_TIME_MORNING },
	{ CHAPTER_17, DISTRICT_TIME_DAY },
	{ DISTRICT_LIFE_END, 0 },
};

/* Red, green, blue: channel * mul / 16 + add */
const struct DistrictTint gDistrictTints[DISTRICT_TIMES] = {
	[DISTRICT_TIME_DAY]     = { { 16, 16, 16 }, { 0, 0, 0 } },
	[DISTRICT_TIME_EVENING] = { { 16, 12, 10 }, { 2, 0, 0 } },
	[DISTRICT_TIME_NIGHT]   = { { 8, 9, 13 }, { 0, 0, 2 } },
	[DISTRICT_TIME_MORNING] = { { 16, 15, 13 }, { 1, 1, 0 } },
};

#define SERAFEW 0x3B
#define PORT_KIRIS 0x3C
#define CAER_PELYN 0x41
#define JEHANNA 0x42

#define SPOT(district, pid, story, x, y) { (district), (pid), (story), (x), (y), 0 }

/* Where townsfolk stand from a story chapter on (the last row that has come wins) */
const struct DistrictNpcSpot gDistrictNpcSpots[] = {
	SPOT(SERAFEW, CHARACTER_CITIZEN_YOUNG_MAN, CHAPTER_08, 9, 4),
	SPOT(SERAFEW, CHARACTER_CITIZEN, CHAPTER_09, 10, 13),
	SPOT(SERAFEW, CHARACTER_CITIZEN_TEEN_GIRL, CHAPTER_10, 8, 9),

	SPOT(PORT_KIRIS, CHARACTER_CITIZEN_YOUNG_MAN, CHAPTER_12, 4, 6),
	SPOT(PORT_KIRIS, CHARACTER_CITIZEN_TEEN_GIRL, CHAPTER_13, 16, 6),

	SPOT(CAER_PELYN, CHARACTER_CITIZEN, CHAPTER_14, 11, 10),
	SPOT(CAER_PELYN, CHARACTER_CITIZEN_YOUNG_GIRL, CHAPTER_16, 10, 16),

	SPOT(JEHANNA, CHARACTER_CITIZEN, CHAPTER_17, 11, 6),
	SPOT(JEHANNA, CHARACTER_CITIZEN_YOUNG_MAN, CHAPTER_18, 3, 8),

	{ .chapter = DISTRICT_LIFE_END },
};
