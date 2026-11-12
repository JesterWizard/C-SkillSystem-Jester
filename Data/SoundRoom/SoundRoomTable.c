// SPDX-License-Identifier: GPL-2.0-only
/*
 * The Sound Room's song list: vanilla's 69 songs (gSoundRoomTable, 0x08A20E74)
 * and the custom songs after them. SoundRoom.event points vanilla's code here.
 *
 * A song with no displayCondFunc shows once it has played in the game (the
 * save keeps one bit per song id, so such a song needs an id below 0x100).
 * A song with one shows when the function returns true.
 */
#include "common-chax.h"
#include "soundroom.h"
#include "constants/songs.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-songs.h"

/* a custom song no chapter plays: always in the list */
static s8 SoundRoom_AlwaysShown(ProcPtr proc)
{
	return true;
}

const struct SoundRoomEnt gSoundRoomTable_NEW[] = {
	{
		.bgmId = SONG_THE_VALIANT,
		.songLength = 2640,
		.displayCondFunc = NULL,
		.nameTextId = 0x78B,
	},
	{
		.bgmId = SONG_MAIN_THEME_EXT,
		.songLength = 5880,
		.displayCondFunc = NULL,
		.nameTextId = 0x78C,
	},
	{
		.bgmId = SONG_PROLOGUE,
		.songLength = 4140,
		.displayCondFunc = NULL,
		.nameTextId = 0x78D,
	},
	{
		.bgmId = SONG_THE_BEGINNING,
		.songLength = 3720,
		.displayCondFunc = NULL,
		.nameTextId = 0x78E,
	},
	{
		.bgmId = SONG_TREASURED_MEMORIES,
		.songLength = 4560,
		.displayCondFunc = NULL,
		.nameTextId = 0x78F,
	},
	{
		.bgmId = SONG_GRIM_JOURNEY,
		.songLength = 4440,
		.displayCondFunc = NULL,
		.nameTextId = 0x790,
	},
	{
		.bgmId = SONG_TIES_OF_FRIENDSHIP,
		.songLength = 3840,
		.displayCondFunc = NULL,
		.nameTextId = 0x791,
	},
	{
		.bgmId = SONG_RAY_OF_HOPE,
		.songLength = 3900,
		.displayCondFunc = NULL,
		.nameTextId = 0x792,
	},
	{
		.bgmId = SONG_DISTANT_ROADS,
		.songLength = 6420,
		.displayCondFunc = NULL,
		.nameTextId = 0x793,
	},
	{
		.bgmId = SONG_RISE_ABOVE,
		.songLength = 5640,
		.displayCondFunc = NULL,
		.nameTextId = 0x794,
	},
	{
		.bgmId = SONG_FOLLOW_ME,
		.songLength = 4980,
		.displayCondFunc = NULL,
		.nameTextId = 0x795,
	},
	{
		.bgmId = SONG_DETERMINATION,
		.songLength = 4560,
		.displayCondFunc = NULL,
		.nameTextId = 0x796,
	},
	{
		.bgmId = SONG_TRUTH_DESPAIR_AND_HOPE,
		.songLength = 5820,
		.displayCondFunc = NULL,
		.nameTextId = 0x797,
	},
	{
		.bgmId = SONG_LAND_OF_PROMISE,
		.songLength = 4680,
		.displayCondFunc = NULL,
		.nameTextId = 0x798,
	},
	{
		.bgmId = SONG_BINDING_VOW,
		.songLength = 3840,
		.displayCondFunc = NULL,
		.nameTextId = 0x799,
	},
	{
		.bgmId = SONG_GRASP_AT_VICTORY,
		.songLength = 2460,
		.displayCondFunc = NULL,
		.nameTextId = 0x79A,
	},
	{
		.bgmId = SONG_CONFRONT_THE_PAST,
		.songLength = 4980,
		.displayCondFunc = NULL,
		.nameTextId = 0x79B,
	},
	{
		.bgmId = SONG_SHADOWS_APPROACH,
		.songLength = 5640,
		.displayCondFunc = NULL,
		.nameTextId = 0x79C,
	},
	{
		.bgmId = SONG_SHADOW_OF_THE_ENEMY,
		.songLength = 2820,
		.displayCondFunc = NULL,
		.nameTextId = 0x79D,
	},
	{
		.bgmId = SONG_ASSAULT,
		.songLength = 5100,
		.displayCondFunc = NULL,
		.nameTextId = 0x79E,
	},
	{
		.bgmId = SONG_FROM_THE_DARKNESS,
		.songLength = 4380,
		.displayCondFunc = NULL,
		.nameTextId = 0x79F,
	},
	{
		.bgmId = SONG_TWISTED_SHADOWS_A,
		.songLength = 4440,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A0,
	},
	{
		.bgmId = SONG_TWISTED_SHADOWS_B,
		.songLength = 4440,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A1,
	},
	{
		.bgmId = SONG_ENVOY_FROM_THE_DARK,
		.songLength = 4980,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A2,
	},
	{
		.bgmId = SONG_ATTACK,
		.songLength = 1500,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A3,
	},
	{
		.bgmId = SONG_DEFENSE,
		.songLength = 1680,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A4,
	},
	{
		.bgmId = SONG_THE_BATTLE_MUST_BE_WON,
		.songLength = 1680,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A5,
	},
	{
		.bgmId = SONG_POWERFUL_FOE,
		.songLength = 1440,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A6,
	},
	{
		.bgmId = SONG_THE_PRINCES_DESPAIR,
		.songLength = 2040,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A7,
	},
	{
		.bgmId = SONG_RETURN_OF_THE_DEMON_KING,
		.songLength = 3840,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A8,
	},
	{
		.bgmId = SONG_SACRED_STRENGTH,
		.songLength = 660,
		.displayCondFunc = NULL,
		.nameTextId = 0x7A9,
	},
	{
		.bgmId = SONG_TETHYS,
		.songLength = 1440,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AA,
	},
	{
		.bgmId = SONG_HEALING,
		.songLength = 840,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AB,
	},
	{
		.bgmId = SONG_CURING,
		.songLength = 660,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AC,
	},
	{
		.bgmId = SONG_TO_A_HIGHER_PLACE,
		.songLength = 1800,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AD,
	},
	{
		.bgmId = SONG_ADVANCE,
		.songLength = 3180,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AE,
	},
	{
		.bgmId = SONG_TENSION,
		.songLength = 2040,
		.displayCondFunc = NULL,
		.nameTextId = 0x7AF,
	},
	{
		.bgmId = SONG_RAID,
		.songLength = 2520,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B0,
	},
	{
		.bgmId = SONG_REUNION,
		.songLength = 2700,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B1,
	},
	{
		.bgmId = SONG_INDIGNATION,
		.songLength = 2940,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B2,
	},
	{
		.bgmId = SONG_SORROW,
		.songLength = 3180,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B3,
	},
	{
		.bgmId = SONG_LAUGHTER,
		.songLength = 2160,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B4,
	},
	{
		.bgmId = SONG_LEGACY,
		.songLength = 4200,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B5,
	},
	{
		.bgmId = SONG_LYON,
		.songLength = 2880,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B6,
	},
	{
		.bgmId = SONG_LOST_HEART,
		.songLength = 3720,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B7,
	},
	{
		.bgmId = SONG_LYON_ORGAN_ARRANGEMENT,
		.songLength = 3900,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B8,
	},
	{
		.bgmId = SONG_THE_FINAL_BATTLE,
		.songLength = 2880,
		.displayCondFunc = NULL,
		.nameTextId = 0x7B9,
	},
	{
		.bgmId = SONG_SOLVE_THE_RIDDLE,
		.songLength = 3300,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BA,
	},
	{
		.bgmId = SONG_LIGHTS_IN_THE_DARK,
		.songLength = 3420,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BB,
	},
	{
		.bgmId = SONG_COMRADES,
		.songLength = 2160,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BC,
	},
	{
		.bgmId = SONG_VICTORY,
		.songLength = 3120,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BD,
	},
	{
		.bgmId = SONG_INTO_THE_SHADOW_OF_VICTORY,
		.songLength = 2220,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BE,
	},
	{
		.bgmId = SONG_MAIN_THEME_REPRISE,
		.songLength = 3960,
		.displayCondFunc = NULL,
		.nameTextId = 0x7BF,
	},
	{
		.bgmId = SONG_COMBAT_PREPARATION,
		.songLength = 3960,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C0,
	},
	{
		.bgmId = SONG_SHOPS,
		.songLength = 1260,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C1,
	},
	{
		.bgmId = SONG_ARMORIES,
		.songLength = 1080,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C2,
	},
	{
		.bgmId = SONG_BONDS,
		.songLength = 900,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C3,
	},
	{
		.bgmId = SONG_COLOSSEUM_ENTRANCE,
		.songLength = 2520,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C4,
	},
	{
		.bgmId = SONG_IN_THE_COLOSSEUM,
		.songLength = 1740,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C5,
	},
	{
		.bgmId = SONG_COLOSSEUM_VICTORY,
		.songLength = 360,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C6,
	},
	{
		.bgmId = SONG_COLOSSEUM_DEFEAT,
		.songLength = 1320,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C7,
	},
	{
		.bgmId = SONG_VICTORY_SONG_A,
		.songLength = 420,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C8,
	},
	{
		.bgmId = SONG_VICTORY_SONG_B,
		.songLength = 420,
		.displayCondFunc = NULL,
		.nameTextId = 0x7C9,
	},
	{
		.bgmId = SONG_GAME_OVER,
		.songLength = 2160,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CA,
	},
	{
		.bgmId = SONG_IN_SORROWS_SHROUD,
		.songLength = 1620,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CB,
	},
	{
		.bgmId = SONG_RECORDS,
		.songLength = 2760,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CC,
	},
	{
		.bgmId = SONG_VICTORY_AND_THE_FUTURE,
		.songLength = 6240,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CD,
	},
	{
		.bgmId = SONG_FLY_WITH_THE_BREEZE,
		.songLength = 9120,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CE,
	},
	{
		.bgmId = SONG_EPILOGUE,
		.songLength = 9360,
		.displayCondFunc = NULL,
		.nameTextId = 0x7CF,
	},
	{
		.bgmId = SONG_LORD_OF_THE_DANCE,
		.songLength = SONG_LORD_OF_THE_DANCE_FRAMES,
		.displayCondFunc = SoundRoom_AlwaysShown,
		.nameTextId = MSG_SOUNDROOM_LORD_OF_THE_DANCE,
	},
	{
		.bgmId = -1,
	},
};
