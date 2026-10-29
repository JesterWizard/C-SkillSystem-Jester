#include "global.h"
#include "proc.h"
#include "hardware.h"
#include "face.h"
#include "scene.h"
#include "anime.h"
#include "ekrbattle.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "constants/video-banim.h"

/**
 * Critical cut-in, in the style of Lex Talionis' FE7 remake.
 *
 * Before a critical hit by a unit with its own portrait, an electric band
 * opens across the middle of the screen with the attacker's portrait held
 * still in it. The battle sprites are hidden under the band while it plays.
 *
 * Standalone port of the C Skill System's CritCutin (Kernel/Wizardry/CritCutin)
 * and the part of its BanimHack C07 handler that starts it. Vanilla's banim
 * command dispatcher (BattleAIS_ExecCommands) jumps through a table of case
 * addresses; entry 7 (C07, attack start) is repointed to CritCutin_C07Thunk
 * (CritCutinThunk.s), which calls CritCutin_OnC07 below and then either
 * resumes the dispatcher (the command is handled) or falls through to the
 * vanilla C07 case (Sure Shot / Great Shield / Pierce skill backgrounds).
 *
 * The band is a spell BG animation on BG1 (Gfx/CritCutin.event) and the
 * portrait is a face sprite in a free face slot.
 */

struct CritCutinConf {
	const s16 *frame_confs;
	const u16 *const *tsas;
	const u16 *const *imgs;
	const u16 *const *pals;
	u16 sfx;
};

extern const struct CritCutinConf CritCutinConf;

/**
 * The portrait shows while the band is fully open: after the 6 frames of its
 * opening and before the 4 frames of its closing (Gfx/CritCutin.event).
 */
enum crit_cutin_layout {
	CUTIN_FACE_SHOW = 6,
	CUTIN_FACE_HIDE = 54,

	/**
	 * Face sprites are 96x80, x is their centre and y their top. The portrait
	 * sits over the attacker's battle animation, nudged towards the attacker's
	 * side of the screen and kept fully on screen. Its bottom edge is on the
	 * band's bottom border.
	 */
	CUTIN_FACE_X_TOWARDS_SIDE = 8,
	CUTIN_FACE_X_MIN = 48 + 8,
	CUTIN_FACE_X_MAX = DISPLAY_WIDTH - 48 - 8,
	CUTIN_FACE_Y = 25,

	/* Frames to wait for the attacker's anims to leave C07 after the band */
	CUTIN_RELEASE_TIMEOUT = 30,
};

struct ProcCritCutin {
	PROC_HEADER;
	struct Anim *anim;
	struct FaceProc *face;
	s16 timer, frame;
	int clock;
	u16 hidden_anims;
	u16 releasing;
	u16 face_pal[0x10];
};

static void CritCutin_OnInit(struct ProcCritCutin *proc);
static void CritCutin_Loop(struct ProcCritCutin *proc);
static void CritCutin_OnEnd(struct ProcCritCutin *proc);
static void CritCutin_WaitRelease(struct ProcCritCutin *proc);

static const struct ProcCmd ProcScr_CritCutin[] = {
	PROC_NAME("CritCutin"),
	PROC_YIELD,
	PROC_CALL(CritCutin_OnInit),
	PROC_REPEAT(CritCutin_Loop),
	PROC_CALL(CritCutin_OnEnd),
	PROC_REPEAT(CritCutin_WaitRelease),
	PROC_END
};

static int CritCutinPortrait(struct Anim *anim)
{
	int round = anim->nextRoundId - 1;
	int type;
	struct Unit *unit;

	if (round < 0)
		return 0;

	type = GetBattleAnimRoundType(round * 2 + GetAnimPosition(anim));
	if (type != ANIM_ROUND_CRIT_CLOSE && type != ANIM_ROUND_CRIT_FAR)
		return 0;

	unit = (GetAnimPosition(anim) == EKR_POS_L) ? &gpEkrBattleUnitLeft->unit : &gpEkrBattleUnitRight->unit;

	/* Generic units only have their class card */
	return unit->pCharacterData->portraitId;
}

static void LockSetAnimState(struct Anim *anim)
{
	struct Anim *anim1 = gAnims[GetAnimPosition(anim) * 2];
	struct Anim *anim2 = gAnims[GetAnimPosition(anim) * 2 + 1];

	anim->state3 |= ANIM_BIT3_BLOCKING;
	anim->state |= ANIM_BIT_FROZEN;

	anim1->state3 |= ANIM_BIT3_BLOCKING;
	anim1->state |= ANIM_BIT_FROZEN;

	anim2->state3 |= ANIM_BIT3_BLOCKING;
	anim2->state |= ANIM_BIT_FROZEN;
}

static void UnlockAnimState(struct Anim *anim)
{
	struct Anim *anim1 = gAnims[GetAnimPosition(anim) * 2];
	struct Anim *anim2 = gAnims[GetAnimPosition(anim) * 2 + 1];

	anim->state3 |= ANIM_BIT3_BLOCKEND;
	anim->state &= ~ANIM_BIT_FROZEN;

	anim1->state3 |= ANIM_BIT3_BLOCKEND;
	anim1->state &= ~ANIM_BIT_FROZEN;

	anim2->state3 |= ANIM_BIT3_BLOCKEND;
	anim2->state &= ~ANIM_BIT_FROZEN;
}

static void CritCutin_HideAnims(struct ProcCritCutin *proc)
{
	int i;

	proc->hidden_anims = 0;
	for (i = 0; i < 4; i++) {
		if (gAnims[i] == NULL || (gAnims[i]->state & ANIM_BIT_HIDDEN))
			continue;

		gAnims[i]->state |= ANIM_BIT_HIDDEN;
		proc->hidden_anims |= 1 << i;
	}
}

static void CritCutin_ShowAnims(struct ProcCritCutin *proc)
{
	int i;

	for (i = 0; i < 4; i++)
		if ((proc->hidden_anims & (1 << i)) && gAnims[i] != NULL)
			gAnims[i]->state &= ~ANIM_BIT_HIDDEN;
}

static void CritCutin_OnInit(struct ProcCritCutin *proc)
{
	int slot, disp, x;

	proc->timer = 0;
	proc->frame = 0;
	proc->clock = 0;
	proc->face = NULL;
	proc->releasing = false;

	LockSetAnimState(proc->anim);

	SpellFx_ClearBG1Position();
	SetDefaultColorEffects_();
	CritCutin_HideAnims(proc);

	if (CritCutinConf.sfx != 0)
		PlaySFX(CritCutinConf.sfx, 0x100, proc->anim->xPosition, 1);

	/* Other battle effects share the face palette, put it back afterwards */
	CpuCopy16(gPaletteBuffer + 0x100 + 0x10 * OBJPAL_BANIM_FACE, proc->face_pal, sizeof(proc->face_pal));

	for (slot = 0; slot < FACE_SLOT_COUNT; slot++)
		if (gFaces[slot] == NULL)
			break;

	if (slot == FACE_SLOT_COUNT)
		return;

	disp = FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_0) | FACE_DISP_HIDDEN | FACE_DISP_BIT_12;
	x = proc->anim->xPosition;

	if (GetAnimPosition(proc->anim) == EKR_POS_L) {
		disp |= FACE_DISP_FLIPPED;
		x -= CUTIN_FACE_X_TOWARDS_SIDE;
	} else {
		x += CUTIN_FACE_X_TOWARDS_SIDE;
	}

	if (x < CUTIN_FACE_X_MIN)
		x = CUTIN_FACE_X_MIN;
	if (x > CUTIN_FACE_X_MAX)
		x = CUTIN_FACE_X_MAX;

	SetupFaceGfxDataInBanim();
	proc->face = StartFace(slot, CritCutinPortrait(proc->anim), x, CUTIN_FACE_Y, disp);
}

static void CritCutin_Loop(struct ProcCritCutin *proc)
{
	const struct CritCutinConf *conf = &CritCutinConf;
	int ret = EfxAdvanceFrameLut(&proc->timer, &proc->frame, conf->frame_confs);

	if (ret >= 0) {
		int i;

		SpellFx_WriteBgMap(proc->anim, conf->tsas[ret], conf->tsas[ret]);
		SpellFx_RegisterBgGfx(conf->imgs[ret], 0x2000);
		SpellFx_RegisterBgPal(conf->pals[ret], 0x20);

		for (i = 0; i < 0x14; i++) {
			gBG1TilemapBuffer[0x20 * i + 0x1E] = gBG1TilemapBuffer[0];
			gBG1TilemapBuffer[0x20 * i + 0x1F] = gBG1TilemapBuffer[0];
		}
	}

	if (proc->face) {
		int disp = GetFaceDisplayBits(proc->face) & ~FACE_DISP_HIDDEN;

		if (proc->clock < CUTIN_FACE_SHOW || proc->clock >= CUTIN_FACE_HIDE)
			disp |= FACE_DISP_HIDDEN;

		SetFaceDisplayBits(proc->face, disp);
	}

	proc->clock++;

	if (ret == -1)
		Proc_Break(proc);
}

static void CritCutin_OnEnd(struct ProcCritCutin *proc)
{
	if (proc->face)
		EndFace(proc->face);

	CpuCopy16(proc->face_pal, gPaletteBuffer + 0x100 + 0x10 * OBJPAL_BANIM_FACE, sizeof(proc->face_pal));
	EnablePaletteSync();

	SpellFx_ClearBG1();
	SetDefaultColorEffects_();
	CritCutin_ShowAnims(proc);

	UnlockAnimState(proc->anim);
	proc->clock = 0;
	proc->releasing = true;
}

/**
 * Stay alive until the attacker's anims have run C07 again and released the
 * block, so CritCutin_OnC07 can tell its own block from vanilla's.
 */
static void CritCutin_WaitRelease(struct ProcCritCutin *proc)
{
	int pos = GetAnimPosition(proc->anim);
	struct Anim *anim1 = gAnims[pos * 2];
	struct Anim *anim2 = gAnims[pos * 2 + 1];

	if (++proc->clock >= CUTIN_RELEASE_TIMEOUT)
		Proc_Break(proc);

	if (anim1 && (anim1->state3 & ANIM_BIT3_BLOCKING))
		return;

	if (anim2 && (anim2->state3 & ANIM_BIT3_BLOCKING))
		return;

	Proc_Break(proc);
}

/**
 * Called by CritCutin_C07Thunk for each banim C07 (attack start).
 * Returns nonzero when the command is handled here, 0 to run vanilla C07.
 */
int CritCutin_OnC07(struct Anim *anim)
{
	struct ProcCritCutin *proc = Proc_Find(ProcScr_CritCutin);

	if (anim->state3 & ANIM_BIT3_BLOCKING) {
		/* Only our own block makes C07 run again: wait or release it */
		if (proc == NULL || GetAnimPosition(proc->anim) != GetAnimPosition(anim))
			return 0;

		if (!(anim->state3 & ANIM_BIT3_BLOCKEND)) {
			anim->pScrCurrent = anim->pScrCurrent - 1;
			return 1;
		}

		/* Cut-in done: carry on with vanilla C07 (skill backgrounds) */
		anim->state3 &= ~(ANIM_BIT3_BLOCKING | ANIM_BIT3_BLOCKEND);
		return 0;
	}

	if (proc != NULL) {
		/* The other layer of the same attacker joins the block */
		if (!proc->releasing && GetAnimPosition(proc->anim) == GetAnimPosition(anim)) {
			anim->pScrCurrent = anim->pScrCurrent - 1;
			anim->state3 |= ANIM_BIT3_BLOCKING;
			return 1;
		}
		return 0;
	}

	/**
	 * Only in real battles at close or far range: the far-far mini anim stays
	 * on screen and shares the face palette, and the promotion and other
	 * scripted anims are no attacks.
	 */
	if (gEkrDistanceType != EKR_DISTANCE_CLOSE && gEkrDistanceType != EKR_DISTANCE_FAR)
		return 0;

	if (CritCutinPortrait(anim) == 0)
		return 0;

	/* Block the script on this C07 until the cut-in has played */
	anim->pScrCurrent = anim->pScrCurrent - 1;
	anim->state3 |= ANIM_BIT3_BLOCKING;

	if (GetAISLayerId(anim) == 0) {
		proc = Proc_Start(ProcScr_CritCutin, PROC_TREE_3);
		proc->anim = anim;
	}

	return 1;
}
