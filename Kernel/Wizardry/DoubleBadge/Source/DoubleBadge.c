#include "common-chax.h"
#include "ekrbattle.h"
#include "double-badge.h"
#include "banim-hack.h"

/*
 * Kernel/Wizardry/DoubleBadge: the battle forecast's "xN" sprite (Gfx_BKSEL, BattleSys/data.event) above each side's
 * Hit/Dmg/Crit box in battle animations. It bobs up and down.
 */

extern const u8 Gfx_BKSEL[12][0x80]; /* the battle forecast's "xN" sprites, 16x16 (BattleSys/data.event) */
extern const u16 Pal_BKSEL[];

/*
 * 2D OBJ tiles 320, 321, 352, 353 (left) and 322, 323, 354, 355 (right): free in battle. Spells load their sprites at
 * tiles 64-319 (vanilla's largest is 0x2000 bytes from 0x06010800), the HP bars use 461-495 and 384-447, and the
 * animations' sheets 512-1023. The badge was at 988-991 and 1020-1023, the last tiles of the right side's sheet:
 * custom animations that use them lost tiles to it (a missing shoe) and drew over it (a garbled "x2").
 */
#define BADGE_CHR_L 320
#define BADGE_CHR_R 322
/*
 * OBJ palette 6: no sprite uses it in a battle (checked every frame of a physical, a critical, a magic battle, a
 * level-up and Myrrh's transformation). Each side's animation has two rows, 7-8 (left) and 9-10 (right), and the
 * second is for special animations: Myrrh's transformation (banim 0xC4) draws her cocoon and dragon with row 8 or 10.
 * The badge was on 10 and reset it to its own colours every frame, so the dragon came out pink until the
 * transformation ended. Before that it was on 15, which the damage numbers, the skill popups and the level-up face
 * also use, so the badge had to hide while they showed: it blinked out at every hit.
 */
#define BADGE_PAL   6
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

static void DoubleBadge_Side(struct ProcEkrGauge *gauge, struct BattleUnit *bu, int pos, int x, int chr)
{
	int n;
	int dx = gauge->unk32, dy = gauge->unk3A;
	int bob = sBadgeBob[(GetGameClock() >> 3) & 3];

	/*
	 * Only a side on screen that fights with a weapon. A staff, a dance or a refresh has no strikes, and its battle
	 * doesn't go through BattleUnwind, which sets them: the last fight's counts would show (an "x2" on the empty side
	 * of a heal). The acting unit's item decides for both sides: the target of a heal holds a weapon too.
	 */
	if (!gBanimValid[pos] || !(GetItemAttributes(bu->weaponBefore) & IA_WEAPON))
		return;
	if (!(GetItemAttributes(gBattleActor.weaponBefore) & IA_WEAPON))
		return;

	n = DoubleBadge_Strikes(bu);
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

	/* Not on a promotion screen: the last battle's counts would show there */
	if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
		return;

	/* Not over the critical cut-in or the level-up box, which cover the HUD */
	if (CritCutinExists() || Proc_Find(ProcScr_EkrLevelup))
		return;

	for (i = 0; i < 16; i++) {
		if (pal[i] != Pal_BKSEL[i])
			break;
	}
	if (i < 16) {
		CpuCopy16(Pal_BKSEL, pal, 0x20);
		EnablePaletteSync();
	}

	DoubleBadge_Side(gauge, gpEkrBattleUnitLeft, EKR_POS_L, 24, BADGE_CHR_L);
	DoubleBadge_Side(gauge, gpEkrBattleUnitRight, EKR_POS_R, 222, BADGE_CHR_R);
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
