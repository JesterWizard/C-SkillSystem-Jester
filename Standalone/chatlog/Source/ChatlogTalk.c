#include "Chatlog.h"
#include "scene.h"
#include "face.h"
#include "bmunit.h"
#include "soundwrapper.h"
#include "constants/songs.h"

/*
 * Feeds the conversation log from vanilla FE8U dialogue: a copy of vanilla
 * Talk_OnIdle (0x08006C34) that starts the log's session, passes it each
 * letter and each line or page break, and holds the text while the log is
 * open.  The speaker lookups are the standalone text engine's.
 */

#define sTalkState (*(struct TalkState **)0x0859133C)
#define sTalkText  (*(struct Text (*)[3])0x030000D0)

#define TALK_TEXT_BY_LINE(line) (&sTalkText[DivRem((line) + sTalkState->topTextNum, sTalkState->lines)])

extern const struct ProcCmd gProcScr_Talk[];

/* --- speaker ------------------------------------------------------------- */

struct ChatlogPortraitAlias {
	u8 portraitId;
	u8 canonicalPortraitId;
};

/* Variant portraits (eyes closed, past, ...) and the character they belong to. */
static const struct ChatlogPortraitAlias sChatlogPortraitAliases[] = {
	{ 0x03, 0x02 }, /* Eirika closed */
	{ 0x0B, 0x0A }, /* Neimi closed */
	{ 0x0D, 0x0C }, /* Colm closed */
	{ 0x12, 0x11 }, /* Natasha closed */
	{ 0x15, 0x14 }, /* Ephraim closed */
	{ 0x17, 0x16 }, /* Forde closed */
	{ 0x1D, 0x1C }, /* Tethys closed */
	{ 0x1F, 0x1E }, /* Marisa closed */
	{ 0x27, 0x26 }, /* Myrrh closed */
	{ 0x28, 0x26 }, /* Myrrh with wings */
	{ 0x2D, 0x02 }, /* Eirika past */
	{ 0x2E, 0x14 }, /* Ephraim past */
	{ 0x2F, 0x29 }, /* Knoll past */
	{ 0x41, 0x40 }, /* Vigarde healthy */
	{ 0x47, 0x46 }, /* Lyon closed */
	{ 0x4A, 0x46 }, /* Lyon Demon King */
	{ 0x4E, 0x40 }, /* Vigarde past */
	{ 0x4F, 0x40 }, /* Vigarde past, closed */
	{ 0x50, 0x46 }, /* Lyon past */
	{ 0x57, 0x56 }, /* Ismaire closed */
};

static const struct CharacterData *Chatlog_FindCharacterWithPortrait(int portraitId)
{
	const struct CharacterData *character;
	int characterId;

	for (characterId = 1; characterId < 0x100; characterId++) {
		character = GetCharacterData(characterId);

		if (!character || !character->nameTextId)
			continue;

		if (character->portraitId == portraitId)
			return character;
	}

	return NULL;
}

static const struct CharacterData *Chatlog_FindCharacterByPortrait(int portraitId)
{
	const struct CharacterData *character = Chatlog_FindCharacterWithPortrait(portraitId);
	int i;

	if (character)
		return character;

	for (i = 0; i < (int)ARRAY_COUNT(sChatlogPortraitAliases); i++) {
		if (sChatlogPortraitAliases[i].portraitId == portraitId)
			return Chatlog_FindCharacterWithPortrait(sChatlogPortraitAliases[i].canonicalPortraitId);
	}

	return NULL;
}

static struct FaceProc *Chatlog_GetTalkFace(int slot)
{
	if (slot < 0 || slot >= (int)ARRAY_COUNT(sTalkState->faces))
		return NULL;

	return sTalkState->faces[slot];
}

/*
 * Static so calls to it stay inside this object: lyn misplaces calls from a
 * second object file to a global function in that same file.
 */
static int Chatlog_GetSpeakingFaceId(void)
{
	struct TalkState *state = sTalkState;
	struct FaceProc *face;
	int slot;
	int i;

	if (!state)
		return 0;

	slot = (s8)state->speakingFaceSlot;
	if (slot < 0)
		slot = state->activeFaceSlot;

	face = Chatlog_GetTalkFace(slot);
	if (face && face->faceId)
		return face->faceId;

	if (state->activeFaceSlot != slot) {
		face = Chatlog_GetTalkFace(state->activeFaceSlot);
		if (face && face->faceId)
			return face->faceId;
	}

	for (i = 0; i < (int)ARRAY_COUNT(state->faces); i++) {
		face = state->faces[i];
		if (face && face->faceId)
			return face->faceId;
	}

	return 0;
}

int TextEngine_GetSpeakingFaceId(void)
{
	return Chatlog_GetSpeakingFaceId();
}

u16 TextEngine_GetSpeakingNameTextId(void)
{
	struct TalkState *state = sTalkState;
	const struct CharacterData *character;
	struct FaceProc *face;
	int slot;

	if (!state)
		return 0;

	slot = (s8)state->speakingFaceSlot;
	if (slot < 0)
		slot = state->activeFaceSlot;

	face = Chatlog_GetTalkFace(slot);
	if (!face)
		return 0;

	character = Chatlog_FindCharacterByPortrait(face->faceId);
	if (!character)
		return 0;

	return character->nameTextId;
}

u8 TextEngine_GetSpeakingCharacterId(void)
{
	const struct CharacterData *character;
	int faceId = Chatlog_GetSpeakingFaceId();

	if (!faceId)
		return 0;

	character = Chatlog_FindCharacterByPortrait(faceId);
	if (!character)
		return 0;

	return character->number;
}

/* --- Talk_OnIdle --------------------------------------------------------- */

/* The line and page breaks TalkInterpret is about to act on. */
static void Chatlog_PeekTalkCode(void)
{
	const char *str = sTalkState->str;

	while (*str == CHFE_L_NormalPrint || *str == CHFE_L_FastPrint || *str == CHFE_L_CloseSpeechFast)
		str++;

	switch (*str) {
	case CHFE_L_NL:
		Chatlog_AppendSoftBreak();
		break;

	case CHFE_L_2NL:
	case CHFE_L_A:
		Chatlog_CommitPage();
		break;
	}
}

void Talk_OnIdle_Chatlog(ProcPtr proc)
{
	if (!Chatlog_IsSessionActive())
		Chatlog_StartSessionUnder(proc);

	if (Chatlog_IsVisible())
		return;

	if (IsTalkFaceMoving())
		return;

	if (!sTalkState->instantScroll) {
		sTalkState->printClock++;

		if (sTalkState->printClock < sTalkState->printDelay)
			return;
	}

	sTalkState->printClock = 0;

	while (1) {
		const char *printedText;

		SetTalkFaceNoMouthMove(sTalkState->activeFaceSlot);
		Chatlog_PeekTalkCode();

		switch (TalkInterpret(proc)) {
		case 0:
			Proc_Break(proc);
			return;

		case 2:
			if (sTalkState->instantScroll || sTalkState->printDelay <= 0)
				break;

			return;

		case 3:
			sTalkState->printClock = sTalkState->printDelay;
			sTalkState->instantScroll = 0;
			return;

		case 1:
		default:
			if (!CheckTalkFlag(TALK_FLAG_SPRITE)) {
				if (TalkPrepNextChar(proc) == 1)
					return;
			} else {
				if (TalkSpritePrepNextChar(proc) == 1)
					return;
			}

			printedText = sTalkState->str;
			sTalkState->str = Text_DrawCharacter(TALK_TEXT_BY_LINE(sTalkState->lineActive), sTalkState->str);
			Chatlog_AppendPrinted(printedText);

			if (!CheckTalkFlag(TALK_FLAG_SILENT)) {
				if (CheckTalkFlag(TALK_FLAG_7)) {
					PlaySoundEffect(SONG_7A);
				} else {
					if ((GetTextDisplaySpeed() == 1) && !(GetGameClock() & 1))
						break;

					if (sTalkState->instantScroll && sTalkState->unk82)
						break;

					sTalkState->unk82 = 1;
					PlaySoundEffect(SONG_6E);
				}
			}
		}

		if (!sTalkState->instantScroll && sTalkState->printDelay > 0)
			return;
	}
}
