#include "common-chax.h"
#include "ekrbattle.h"
#include "double-badge.h"
#include "efx-skill.h"
#include "banim-hack.h"

/*
 * Kernel/Wizardry/DoubleBadge: the battle forecast's "xN" sprite (Gfx_BKSEL, BattleSys/data.event) above each side's
 * Hit/Dmg/Crit box in battle animations. It bobs up and down.
 */

extern const u8 Gfx_BKSEL[12][0x80]; /* the battle forecast's "xN" sprites, 16x16 (BattleSys/data.event) */
extern const u16 Pal_BKSEL[];

#define BADGE_CHR_L 988  /* 2D OBJ tiles 988, 989, 1020, 1021 */
#define BADGE_CHR_R 990  /* 2D OBJ tiles 990, 991, 1022, 1023 */
#define BADGE_PAL   15   /* shared with the skill and anim number popups and the level-up face: hidden while they show */
#define BADGE_Y     92   /* the 16x16 badge's top; the boxes' Hit rows are at y 112 */

static const u16 sObj_Badge16x16[] = { 1, OAM0_SHAPE_16x16, OAM1_SIZE_16x16, 0 };

/* A gentle bob: 0, -1, -2, -1 pixels, 8 frames each */
static const s8 sBadgeBob[4] = { 0, -1, -2, -1 };

void DoubleBadge_Set(int actor, int target)
{
	gDoubleBadge.strikes[0] = actor;
	gDoubleBadge.strikes[1] = target;
}

static int DoubleBadge_Strikes(struct BattleUnit *bu)
{
	if (bu == &gBattleActor)
		return gDoubleBadge.strikes[0];
	if (bu == &gBattleTarget)
		return gDoubleBadge.strikes[1];
	return 0;
}

static void DoubleBadge_Put(int x, int y, int chr)
{
	CallARM_PushToSecondaryOAM(OAM1_X(0x200 + x), OAM0_Y(0x100 + y), sObj_Badge16x16, OAM2_CHR(chr) | OAM2_PAL(BADGE_PAL) | OAM2_LAYER(0));
}

static void DoubleBadge_Side(struct ProcEkrGauge *gauge, struct BattleUnit *bu, int x, int chr)
{
	int n = DoubleBadge_Strikes(bu);
	int dx = gauge->unk32, dy = gauge->unk3A;
	int bob = sBadgeBob[(GetGameClock() >> 3) & 3];

	if (n < 2)
		return;
	if (n > 9)
		n = 10; /* "xN" */

	/* top row: tiles chr, chr + 1; bottom row: chr + 32, chr + 33 (2D mapping) */
	RegisterDataMove(Gfx_BKSEL[n], (void *)(0x06010000 + chr * 0x20), 0x40);
	RegisterDataMove(Gfx_BKSEL[n] + 0x40, (void *)(0x06010000 + (chr + 32) * 0x20), 0x40);
	DoubleBadge_Put(x + dx, BADGE_Y + dy + bob, chr);
}

static void DoubleBadge_Loop(ProcPtr proc)
{
	struct ProcEkrGauge *gauge = gpProcEkrGauge;
	u16 *pal = gPaletteBuffer + 0x100 + BADGE_PAL * 16;
	int i;

	if (!gauge || gauge->valid == 1) /* hidden: vanilla's ekrGaugeMain skips drawing then */
		return;

	/*
	 * These share OBJ palette 15 and their sprites use it: draw nothing while any shows. So does the level-up box's
	 * portrait (EkrLvup_InitLevelUpBox's gUnknown_087592CC): resetting the palette under it drew the face in the
	 * badge's colours.
	 */
	if (EfxSkillExists() || EfxSkillBoxExists() || EkrSkillExists() || EfxAnimNumberExists() || CritCutinExists())
		return;
	if (Proc_Find(ProcScr_EkrLevelup))
		return;

	for (i = 0; i < 16; i++) {
		if (pal[i] != Pal_BKSEL[i])
			break;
	}
	if (i < 16) {
		CpuCopy16(Pal_BKSEL, pal, 0x20);
		EnablePaletteSync();
	}

	DoubleBadge_Side(gauge, gpEkrBattleUnitLeft, 24, BADGE_CHR_L);
	DoubleBadge_Side(gauge, gpEkrBattleUnitRight, 222, BADGE_CHR_R);
}

static const struct ProcCmd ProcScr_DoubleBadge[] = {
	PROC_NAME("DoubleBadge"),
	PROC_REPEAT(DoubleBadge_Loop),
	PROC_END,
};

/* A child of the HUD's proc, so it ends with it */
void StartDoubleBadge(ProcPtr gauge)
{
	Proc_Start(ProcScr_DoubleBadge, gauge);
}
