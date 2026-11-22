#include "common-chax.h"
#include "kernel-lib.h"
#include "enemy-promo-fx.h"
#include "constants/texts.h"

/*
 * The map effect of an enemy's instant promotion (promote_enemy_on_kill, Shiren-style): BattleHit.c promotes the
 * enemy that kills a unit and queues it in gEnemyPromoFx. After the action, the camera goes to it, its old class's
 * map sprite flashes white, turns into the new class's sprite and fades back, and a popup names the new class.
 * Not the promotion animation (ProcScr_PromoMain and friends): the enemy promotes in place, on the map.
 */

#define PROMO_FX_FLASH_WHITE 0      /* gMuFlashPalLut[0], all white */
#define PROMO_FX_SE_FLASH    0x3C9  /* the map's status-recovery shimmer */
#define PROMO_FX_SE_CHANGE   0x5A   /* SONG_SE_UPDATE */

struct EnemyPromoFxProc {
	PROC_HEADER;
	struct Unit *unit;
	struct MuProc *mu;
};

static const struct PopupInstruction PopupScr_EnemyPromoted[] = {
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
	POPUP_MSG(MSG_ENEMY_PROMOTED),
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	POPUP_STR(gEnemyPromoFx.cls),
	POPUP_END
};

static void EnemyPromoFx_Camera(struct EnemyPromoFxProc *proc)
{
	EnsureCameraOntoPosition(proc, proc->unit->xPos, proc->unit->yPos);
}

static struct MuProc *EnemyPromoFx_StartMu(struct Unit *unit)
{
	struct MuProc *mu = StartMu(unit);

	if (mu)
		SetMuDefaultFacing(mu);

	return mu;
}

/* The old class's sprite (the unit is already promoted: its class is swapped back just to start the MU) */
static void EnemyPromoFx_FlashOld(struct EnemyPromoFxProc *proc)
{
	struct Unit *unit = proc->unit;
	const struct ClassData *jinfo = unit->pClassData;

	/* The action's own MU can still stand on the unit's tile (in its new class, over ours) */
	EndAllMus();
	HideUnitSprite(unit);

	unit->pClassData = GetClassData(gEnemyPromoFx.old_jid);
	proc->mu = EnemyPromoFx_StartMu(unit);
	unit->pClassData = jinfo;

	PlaySoundEffect(PROMO_FX_SE_FLASH);

	if (proc->mu)
		StartMuFadeIntoFlash(proc->mu, PROMO_FX_FLASH_WHITE);
}

/* Still white: the new class's sprite takes its place with a white hit flash, which fades back to its colours */
static void EnemyPromoFx_SwapToNew(struct EnemyPromoFxProc *proc)
{
	if (proc->mu)
		EndMu(proc->mu);

	proc->mu = EnemyPromoFx_StartMu(proc->unit);
	if (!proc->mu)
		return;

	PlaySoundEffect(PROMO_FX_SE_CHANGE);
	StartMuHitFlash(proc->mu, PROMO_FX_FLASH_WHITE);
}

static void EnemyPromoFx_End(struct EnemyPromoFxProc *proc)
{
	if (proc->mu)
		EndMu(proc->mu);

	proc->mu = NULL;
	ShowUnitSprite(proc->unit);
	RefreshUnitSprites();
	/* The map sprite sheet holds the classes on the map: load the new one's */
	ForceSyncUnitSpriteSheet();
}

static void EnemyPromoFx_Popup(struct EnemyPromoFxProc *proc)
{
	SetPopupUnit(proc->unit);
	NewPopup_Simple(PopupScr_EnemyPromoted, 0x60, 0, proc);
}

static const struct ProcCmd ProcScr_EnemyPromoFx[] = {
	PROC_NAME("EnemyPromoFx"),
	PROC_YIELD,
	PROC_CALL(EnemyPromoFx_Camera),
	PROC_YIELD,
	PROC_CALL(EnemyPromoFx_FlashOld),
	PROC_SLEEP(36),
	PROC_CALL(EnemyPromoFx_SwapToNew),
	PROC_SLEEP(40),
	PROC_CALL(EnemyPromoFx_Popup),
	PROC_YIELD,
	PROC_CALL(EnemyPromoFx_End),
	PROC_END,
};

bool PostAction_EnemyPromoFx(ProcPtr parent)
{
	struct EnemyPromoFxProc *proc;
	struct Unit *unit;
	const char *str;
	int i;

	if (gEnemyPromoFx.uid == 0)
		return false;

	unit = GetUnit(gEnemyPromoFx.uid);
	gEnemyPromoFx.uid = 0;

	if (!UNIT_ALIVE(unit) || !unit->pClassData)
		return false;

	if (UNIT_CLASS_ID(unit) == gEnemyPromoFx.old_jid)
		return false;

	/* The popup's class name: GetStringFromIndex's buffer is reused for the unit's name and the message */
	str = GetStringFromIndex(unit->pClassData->nameTextId);
	for (i = 0; i < (int)sizeof(gEnemyPromoFx.cls) - 1 && str[i]; i++)
		gEnemyPromoFx.cls[i] = str[i];
	gEnemyPromoFx.cls[i] = '\0';

	proc = Proc_StartBlocking(ProcScr_EnemyPromoFx, parent);
	proc->unit = unit;
	proc->mu = NULL;
	return true;
}

void ChapterInit_ResetEnemyPromoFx(ProcPtr proc)
{
	gEnemyPromoFx.uid = 0;
}
