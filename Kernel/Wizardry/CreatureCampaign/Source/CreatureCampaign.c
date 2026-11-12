#include "common-chax.h"
#include "kernel-lib.h"
#include "bmsave.h"
#include "gamecontrol.h"

/*
 * The Creature Campaign without Extras > Map (Documentation/Features/WorldMapStoryChapters.md, "The postgame").
 *
 * The save made at the end of the game is the postgame's: GameCtrl_DeclareCompletedChapter sets PLAY_FLAG_POSTGAME
 * with PLAY_FLAG_COMPLETE, so the file is titled "Creature Campaign" (GetChapterTitleExtra's title 0x57). After the
 * ending and "Fin", the game loads that save and goes on to the postgame's world map. Loading it from the title does
 * the same; so does loading a cleared save made before this (it plays the ending again in vanilla).
 */

//! FE8U = 0x080314B0: the last chapter is won, before its save menu
LYN_REPLACE_CHECK(GameCtrl_DeclareCompletedChapter);
void GameCtrl_DeclareCompletedChapter(void)
{
	RegisterChapterTimeAndTurnCount(&gPlaySt);

	ComputeChapterRankings();
	SaveEndgameRankings();

	gPlaySt.chapterStateBits |= PLAY_FLAG_COMPLETE | PLAY_FLAG_POSTGAME;
}

//! FE8U = 0x08009D44: a loaded game, before the world map. A cleared game is the Creature Campaign.
LYN_REPLACE_CHECK(GameCtrl_CheckGameCompleteAndBranch);
void GameCtrl_CheckGameCompleteAndBranch(struct GameCtrlProc *proc)
{
	if (gPlaySt.chapterStateBits & PLAY_FLAG_COMPLETE)
		gPlaySt.chapterStateBits |= PLAY_FLAG_POSTGAME;
}

/* The save made at the end of this game: the last one written, this playthrough's and in the postgame */
static int CreatureCampaign_EndSaveSlot(void)
{
	struct PlaySt playSt;
	int slot = ReadLastGameSaveId();

	if (slot < 0 || slot > 2 || !IsSaveValid(slot))
		return -1;

	ReadGameSavePlaySt(slot, &playSt);

	if (playSt.playthroughIdentifier != gPlaySt.playthroughIdentifier)
		return -1;

	if ((playSt.chapterStateBits & (PLAY_FLAG_COMPLETE | PLAY_FLAG_POSTGAME)) !=
	    (PLAY_FLAG_COMPLETE | PLAY_FLAG_POSTGAME))
		return -1;

	return slot;
}

/*
 * After the ending's "Fin", in place of vanilla's _SetGameEndFlag in gProcScr_GameControl (the installer patches the
 * PROC_CALL: the function is too short for a LynJump). Vanilla then goes back to the title; with the end save, the game
 * goes on in it.
 */
void CreatureCampaign_PostEnding(ProcPtr proc)
{
	struct GameCtrlProc *gameCtrl = proc;
	int slot;

	SetGameEndFlag();

	slot = CreatureCampaign_EndSaveSlot();
	if (slot < 0)
		return;

	/* As Restart Chapter on that file does (PostSaveMenuHandler) */
	ReadGameSave(slot);
	gameCtrl->nextAction = slot + 1;
	Proc_Goto(gameCtrl, LGAMECTRL_EXEC_BM);
}
