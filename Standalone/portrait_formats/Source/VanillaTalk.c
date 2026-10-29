#include "Portraits.h"

#include "bmlib.h"
#include "bmunit.h"
#include "ekrbattle.h"
#include "face.h"
#include "scene.h"

/*
 * Vanilla dialogue: half-body speakers stand on the bottom edge of the
 * screen with their speech bubble along the bottom, as in the C Skill
 * System's text engine.  Not installed with Standalone/text_engine, which
 * does the same in its own StartTalkOpen and face loading.
 */

#define sTalkState (*(struct TalkState **)0x0859133C)

enum {
	TALK_FACE_Y = 80,
	TALK_HALFBODY_FACE_Y = 32,
	TALK_BUBBLE_Y = 8,
	TALK_HALFBODY_BUBBLE_Y = 18,
};

/* Replaces TalkLoadFace (0x08007854). */
void TalkLoadFace_Portraits(ProcPtr proc)
{
	int faceDisp = 0;
	int faceId;
	int slot;

	if (sTalkState->activeFaceSlot == 0xFF)
		SetActiveTalkFace(1);

	if ((s8)IsBattleDeamonActive())
		SetupFaceGfxDataInBanim();
	else
		faceDisp |= FACE_DISP_KIND(FACE_96x80);

	if (GetTalkFaceHPos(sTalkState->activeFaceSlot) <= 14)
		faceDisp |= FACE_DISP_FLIPPED;

	faceId = sTalkState->str[0];
	faceId = (sTalkState->str[1] * 0x100) + faceId;

	if (faceId == 0xFFFF)
		faceId = GetUnitPortraitId(gActiveUnit);
	else
		faceId = faceId - 0x100;

	slot = sTalkState->activeFaceSlot;

	if (sTalkState->faces[slot] != NULL) {
		sub_80066E0(sTalkState->faces[slot], faceId);
		return;
	}

	sTalkState->faces[slot] = StartFaceAuto(
		faceId,
		GetTalkFaceHPos(slot) * 8,
		Portraits_TalkFaceWillBeHalfBody(faceId, faceDisp) ? TALK_HALFBODY_FACE_Y : TALK_FACE_Y,
		faceDisp
	);

	StartFaceFadeIn(sTalkState->faces[slot]);
	SetTalkFaceLayer(slot, CheckTalkFlag(TALK_FLAG_4));
	StartTemporaryLock(proc, 8);
}

/* Replaces StartTalkOpen (0x080088A8). */
void StartTalkOpen_Portraits(int talkFace, ProcPtr parent)
{
	struct Proc *proc = Proc_StartBlocking(gProcScr_TalkOpen, parent);

	proc->unk64 = GetTalkFaceHPos(talkFace);
	proc->unk66 = Portraits_IsHalfBodyFace(sTalkState->faces[talkFace])
		? TALK_HALFBODY_BUBBLE_Y
		: TALK_BUBBLE_Y;
	proc->unk68 = sTalkState->activeWidth;
	proc->unk6A = 6;

	if (proc->unk64 < 0)
		proc->unk64 = 0;

	if (proc->unk64 >= 30)
		proc->unk64 = 30;

	sTalkState->speakingFaceSlot = talkFace;
	sTalkState->speakingWidth = sTalkState->activeWidth;
}
