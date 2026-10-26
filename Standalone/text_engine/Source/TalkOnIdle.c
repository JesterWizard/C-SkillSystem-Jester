#include "TextEngine.h"
#include "scene.h"
#include "soundwrapper.h"
#include "constants/songs.h"

/*
 * Vanilla Talk_OnIdle (0x08006C34) with the text engine's per-letter hooks:
 * letter-print effects, big text and rainbow text draw the letter
 * themselves, shake-on-print reacts to it, and the boop uses the speaker's
 * pitch. From MiscFunctions.c in the C Skill System, without its
 * conversation log and voice acting.
 */

#define sTalkState (*(struct TalkState **)0x0859133C)
#define sTalkText  (*(struct Text (*)[3])0x030000D0)

#define TALK_TEXT_BY_LINE(line) (&sTalkText[k_umod(((line) + sTalkState->topTextNum), sTalkState->lines)])

void TextEngine_OnCharacterPrinted(void);
void TextEngine_PlayTextBoop(const char *text);
s8 TextEngine_TryStartGlyphFloat(struct Text *text, const char **str);

LYN_REPLACE_CHECK(Talk_OnIdle);
void Talk_OnIdle(ProcPtr proc)
{
	if (IsTalkFaceMoving())
		return;

	if (!sTalkState->instantScroll) {
		sTalkState->printClock++;

		if (sTalkState->printClock < sTalkState->printDelay)
			return;
	}

	sTalkState->printClock = 0;

	while (1) {
		u8 talkFace = sTalkState->activeFaceSlot;
		const char *printedText;
		struct Text *th;

		if (talkFace < ARRAY_COUNT(sTalkState->faces) && sTalkState->faces[talkFace] != NULL)
			SetTalkFaceNoMouthMove(talkFace);

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
			th = TALK_TEXT_BY_LINE(sTalkState->lineActive);

			if (!TextEngine_TryStartGlyphFloat(th, (const char **)&sTalkState->str))
				sTalkState->str = Text_DrawCharacter(th, sTalkState->str);

			TextEngine_OnCharacterPrinted();

			if (!CheckTalkFlag(TALK_FLAG_SILENT)) {
				if (CheckTalkFlag(TALK_FLAG_7)) {
					/* World map text */
					PlaySoundEffect(SONG_7A);
				} else {
					if ((GetTextDisplaySpeed() == 1) && !(GetGameClock() & 1))
						break;

					if (sTalkState->instantScroll && sTalkState->unk82)
						break;

					sTalkState->unk82 = 1;
					TextEngine_PlayTextBoop(printedText);
				}
			}
		}

		if (!sTalkState->instantScroll && sTalkState->printDelay > 0)
			return;
	}
}
