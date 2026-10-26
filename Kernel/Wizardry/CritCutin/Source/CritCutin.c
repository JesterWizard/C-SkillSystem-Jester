#include "common-chax.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "combo-attack.h"
#include "efx-anim.h"
#include "banim-hack.h"

/**
 * Critical cut-in, in the style of Lex Talionis' FE7 remake.
 *
 * Before a critical hit by a unit with its own portrait, an electric band
 * opens across the middle of the screen and the attacker's portrait slides
 * across it, entering from the attacker's side. The battle sprites are hidden
 * under the band while it plays. Started from C07 (attack start), after the
 * HP cost and skill boxes.
 *
 * The band is a spell BG animation on BG1 (Contents/EfxAnim/Source/CritCutin)
 * and the portrait is a face sprite in the battle face slot.
 */

extern const struct EfxAnimConf EfxCritCutin;

/* Portrait path, in frames from the start of the band */
enum crit_cutin_timing {
	CUTIN_ENTER_START = 6,
	CUTIN_ENTER_LEN   = 12,
	CUTIN_DRIFT_LEN   = 26,
	CUTIN_EXIT_LEN    = 10,

	CUTIN_DRIFT_START = CUTIN_ENTER_START + CUTIN_ENTER_LEN,
	CUTIN_EXIT_START  = CUTIN_DRIFT_START + CUTIN_DRIFT_LEN,
	CUTIN_EXIT_END    = CUTIN_EXIT_START + CUTIN_EXIT_LEN,
};

/* Portrait centre x for an attacker on the right; mirrored for the left */
enum crit_cutin_path {
	CUTIN_X_OFFSCREEN_NEAR = 296,
	CUTIN_X_ENTERED        = 150,
	CUTIN_X_DRIFTED        = 110,
	CUTIN_X_OFFSCREEN_FAR  = -56,

	/* Face sprites are 80px tall from yPos; this sits them on the band's bottom edge (y 103) */
	CUTIN_FACE_Y = 24,
};

struct ProcCritCutin {
	PROC_HEADER;
	struct Anim *anim;
	struct FaceProc *face;
	s16 timer, frame;
	int clock;
	u16 hidden_anims;
	u16 face_pal[0x10];
};

static int CritCutinPortrait(struct Anim *anim)
{
	int round = anim->nextRoundId - 1;
	struct BattleHit *hit;
	struct Unit *unit = NULL;

	if (round < 0 || round >= NEW_BATTLE_HIT_MAX)
		return 0;

	hit = prBattleHitArray + round * BattleHitArrayWidth;
	if (!(hit->attributes & BATTLE_HIT_ATTR_CRIT))
		return 0;

	if (gpKernelDesignerConfig->engage_combo_attack && round < COMBO_ATK_MAX)
		unit = GetMapAnimComboUnit(round);

	if (unit == NULL)
		unit = (GetAnimPosition(anim) == EKR_POS_L) ? &gpEkrBattleUnitLeft->unit : &gpEkrBattleUnitRight->unit;

	/* Generic units only have their class card */
	return unit->pCharacterData->portraitId;
}

static int CritCutinFaceX(struct ProcCritCutin *proc)
{
	int k, x;

	if (proc->clock < CUTIN_DRIFT_START) {
		/* Ease out from the attacker's edge */
		k = CUTIN_ENTER_LEN - (proc->clock - CUTIN_ENTER_START);
		x = CUTIN_X_ENTERED + (CUTIN_X_OFFSCREEN_NEAR - CUTIN_X_ENTERED) * k * k / (CUTIN_ENTER_LEN * CUTIN_ENTER_LEN);
	} else if (proc->clock < CUTIN_EXIT_START) {
		k = proc->clock - CUTIN_DRIFT_START;
		x = CUTIN_X_ENTERED + (CUTIN_X_DRIFTED - CUTIN_X_ENTERED) * k / CUTIN_DRIFT_LEN;
	} else {
		/* Ease in towards the far edge */
		k = proc->clock - CUTIN_EXIT_START;
		x = CUTIN_X_DRIFTED + (CUTIN_X_OFFSCREEN_FAR - CUTIN_X_DRIFTED) * k * k / (CUTIN_EXIT_LEN * CUTIN_EXIT_LEN);
	}

	if (GetAnimPosition(proc->anim) == EKR_POS_L)
		x = DISPLAY_WIDTH - x;

	return x;
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
		if (proc->hidden_anims & (1 << i))
			gAnims[i]->state &= ~ANIM_BIT_HIDDEN;
}

static void CritCutin_OnInit(struct ProcCritCutin *proc)
{
	int slot, disp;

	proc->timer = 0;
	proc->frame = 0;
	proc->clock = 0;
	proc->face = NULL;

	SpellFx_ClearBG1Position();
	SetDefaultColorEffects_();
	CritCutin_HideAnims(proc);

	if (EfxCritCutin.sfx != 0)
		PlaySFX(EfxCritCutin.sfx, 0x100, proc->anim->xPosition, 1);

	/* Other battle effects share the face palette, put it back afterwards */
	CpuCopy16(gPaletteBuffer + 0x100 + 0x10 * OBJPAL_BANIM_FACE, proc->face_pal, sizeof(proc->face_pal));

	for (slot = 0; slot < 4; slot++)
		if (gFaces[slot] == NULL)
			break;

	if (slot == 4)
		return;

	disp = FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_0) | FACE_DISP_HIDDEN | FACE_DISP_BIT_12;
	if (GetAnimPosition(proc->anim) == EKR_POS_L)
		disp |= FACE_DISP_FLIPPED;

	SetupFaceGfxDataInBanim();
	proc->face = StartFace(slot, CritCutinPortrait(proc->anim), CUTIN_X_OFFSCREEN_NEAR, CUTIN_FACE_Y, disp);
}

static void CritCutin_Loop(struct ProcCritCutin *proc)
{
	const struct EfxAnimConf *conf = &EfxCritCutin;
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

		if (proc->clock < CUTIN_ENTER_START || proc->clock >= CUTIN_EXIT_END)
			disp |= FACE_DISP_HIDDEN;

		SetFaceDisplayBits(proc->face, disp);
		proc->face->xPos = CritCutinFaceX(proc);
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
}

static const struct ProcCmd ProcScr_CritCutin[] = {
	PROC_NAME("CritCutin"),
	PROC_YIELD,
	PROC_CALL(CritCutin_OnInit),
	PROC_REPEAT(CritCutin_Loop),
	PROC_CALL(CritCutin_OnEnd),
	PROC_END
};

/* Called from BanimHack C07 */
void NewCritCutin(struct Anim *anim)
{
	struct ProcCritCutin *proc;

	if (!gpKernelDesignerConfig->crit_cutin)
		return;

	/* The far-far mini anim stays on screen and shares the face palette */
	if (gEkrDistanceType == EKR_DISTANCE_FARFAR)
		return;

	if (CritCutinPortrait(anim) == 0)
		return;

	proc = Proc_Start(ProcScr_CritCutin, PROC_TREE_3);
	proc->anim = anim;
}

bool CritCutinExists(void)
{
	return Proc_Exists(ProcScr_CritCutin);
}
