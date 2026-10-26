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
 * opens across the middle of the screen with the attacker's portrait held in
 * the centre of it. The battle sprites are hidden under the band while it
 * plays. Started from C07 (attack start), after the HP cost and skill boxes.
 *
 * The band is a spell BG animation on BG1 (Contents/EfxAnim/Source/CritCutin)
 * and the portrait is a face sprite in the battle face slot.
 */

extern const struct EfxAnimConf EfxCritCutin;

/**
 * The portrait shows while the band is fully open: after the 6 frames of its
 * opening and before the 4 frames of its closing (CritCutin.efx.txt).
 */
enum crit_cutin_layout {
	CUTIN_FACE_SHOW = 6,
	CUTIN_FACE_HIDE = 54,

	/* Face sprites are 96x80, x is their centre and y their top; this fills the band (y 27-106) */
	CUTIN_FACE_X = DISPLAY_WIDTH / 2,
	CUTIN_FACE_Y = 27,
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
	proc->face = StartFace(slot, CritCutinPortrait(proc->anim), CUTIN_FACE_X, CUTIN_FACE_Y, disp);
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
