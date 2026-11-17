#include "common-chax.h"
#include "ekrbattle.h"
#include "efxbattle.h"
#include "bmio.h"
#include "bmlib.h"
#include "feb-compat.h"
#include "bmudisp.h"
#include "mu.h"

/*
 * Debugger, "Battle anim" (VeslyDebugger_Installer.event, page 3): a panel on the left of the map, beside the debugger's
 * menu (which stays open on the right, blocked while the viewer runs), with one platform in it and a battle animation
 * on the platform, facing left as the player's does in battle, and its ID and the character's name above. Up and down
 * step through the characters (each in their default class, then every character-and-class row of CustomAnimeTable), L
 * and R by ten; A switches between the normal attack and the critical; B goes back to the menu.
 *
 * The animation runs on the game's mini battle-animation engine, the one the class reel on the title screen uses
 * (ClassInfoDisplay_Init, FE8U 0x080B37FC): the same AnimBuffer, buffers and platform, at the same addresses. They are
 * battle-animation work RAM, free outside a battle. The viewer's own data is in gBanimScrLeft (0x0200F1C8..0x02011BC8),
 * the battle-script buffer, also free on the map: the list at 0x0200F200, the label text at 0x0200F500, and BG0 and BG1's
 * tilemaps backed up at 0x0200F600 and 0x0200FE00.
 *
 * A mage or staff user casts, but no spell is run: the class reel's spell buffer (sViewMagicFx) has magicFuncIdx 0 (the
 * class reel's Dummy spell, which does nothing), so the cast animation plays without a spell and the attack goes on.
 */

/* The class reel's (gOpInfoData and its buffers, gUnk_Opinfo_0201DB00) */
#define sViewAnim     ((struct AnimBuffer *)0x02000000)
#define sViewMagicFx  ((struct AnimMagicFxBuffer *)0x0200A2D8)
#define sViewPlatform ((struct BanimUnkStructComm *)0x0201DB00)
/*
 * The viewer's own work RAM, in the battle-script buffer gBanimScrLeft (0x0200F1C8..0x02011BC8): free on the map.
 * The list, the label text, and the BG0 and BG1 tilemaps the viewer's panel covers (put back on the way out).
 */
#define sViewEntries  ((struct ViewEntry *)0x0200F200)
#define sViewText     ((struct Text *)0x0200F500)
#define sViewBg0Backup ((u16 *)0x0200F600)
#define sViewBg1Backup ((u16 *)0x0200FE00)

/* The mini engine and the class reel's platform: FE-CLib's names, what they do */
#define MiniAnimRestart    sub_805A7B4 /* starts the mode in roundType again */
#define MiniAnimRoundDone  sub_805A96C /* the mode in roundType has played to its end */
#define MiniAnimEnd        sub_805AA28
#define NewPlatform        sub_805AA68
#define PlatformSetPos     sub_805AE40
#define EndPlatform        sub_805AE14

void sub_805AA68(struct BanimUnkStructComm *platform);
void sub_805AE40(struct BanimUnkStructComm *platform, s16 xl, s16 yl, s16 xr, s16 yr);
void sub_805AE14(struct BanimUnkStructComm *platform);
void PutNumberHex(u16 *tm, int color, int number); /* C_Code.c */

#define ROUND_ATTACK 0
#define ROUND_CRIT   1
#define ROUND_STAND  6

#define VIEW_X 104 /* the animation's position, in the engine's space (the class reel's is x 260, on the right) */
#define VIEW_Y 88
#define PLATFORM_X 80 /* with VIEW_X: the platform's box is x 41..129 on screen, centred on the window (x 84) */
#define PLATFORM_Y 104

/*
 * While the viewer is up, BG1 (the panel and the menu's window) and the map's BG2 are both at this priority, so the
 * panel is over the map (BG1 is the lower layer number). The animation (priority 2) and the platform (3) are over both:
 * a sprite is drawn over a BG of the same priority.
 */
#define VIEW_BG_PRIO 3
#define VIEW_MAX_ENTRIES 0x180
/* frames before the viewer reads keys: the A that opened it is still in the key state for a few frames (counted in timer) */
#define VIEW_SETTLE 12

struct ViewEntry {
	u8 pid, jid;
};

struct ProcBanimViewer {
	PROC_HEADER;

	u16 count;
	u16 index;
	u8 crit;
	u8 timer;
	u16 chr_counter; /* the font's tile allocator, put back on the way out */
	u8 hidden_mus; /* the bits of the MU slots hidden while the viewer is up (the selected unit's MU too) */
	u8 bg1_priority, bg2_priority; /* the panel's and the map's BG priorities, put back on the way out */
};

static const struct BattleAnimDef *ViewAnimDef(int pid, int jid, bool *custom)
{
	const struct IndividualAnimConf *conf = GetPrIndividualAnimConf();
	const struct ClassData *jinfo;
	int i;

	*custom = false;
	if (conf) {
		for (i = 0; conf[i].pid != 0 || conf[i].jid != 0; i++) {
			if (conf[i].pid == pid && conf[i].jid == jid) {
				*custom = true;
				return conf[i].anim_conf;
			}
		}
	}
	jinfo = GetClassData(jid);
	return jinfo ? jinfo->pBattleAnimDef : NULL;
}

/* The animation shown (an index of banim_data): the first weapon type that isn't unarmed, else the first. -1: none */
static int ViewAnimId(const struct BattleAnimDef *def)
{
	int i;

	if (!def)
		return -1;
	for (i = 0; def[i].wtype != 0 || def[i].index != 0; i++) {
		if ((def[i].wtype >> 8) == 1 && (def[i].wtype & 0xFF) != ITYPE_ITEM)
			return def[i].index - 1;
	}
	return def[0].index ? def[0].index - 1 : -1;
}

static int ViewCharaPal(int pid, int jid, bool custom)
{
	int i;

	/* A character palette is laid out for its class's own animations (HPBarPalettes.c) */
	if (custom)
		return -1;
	for (i = 0; i < 7; i++) {
		if (gAnimCharaPalConfig[pid - 1][i] == jid)
			return gAnimCharaPalIt[pid - 1][i] - 1;
	}
	return -1;
}

static bool ViewEntryValid(int pid, int jid)
{
	const struct CharacterData *pinfo = GetCharacterData(pid);
	bool custom;

	if (!pinfo || !pinfo->nameTextId || jid < 1)
		return false;
	return ViewAnimId(ViewAnimDef(pid, jid, &custom)) >= 0;
}

static void ViewBuildList(struct ProcBanimViewer *proc)
{
	const struct IndividualAnimConf *conf = GetPrIndividualAnimConf();
	int pid, i, n = 0;

	for (pid = 1; pid < 0x100 && n < VIEW_MAX_ENTRIES; pid++) {
		const struct CharacterData *pinfo = GetCharacterData(pid);

		if (!pinfo || !ViewEntryValid(pid, pinfo->defaultClass))
			continue;
		sViewEntries[n].pid = pid;
		sViewEntries[n].jid = pinfo->defaultClass;
		n++;

		/* the character's custom animations in other classes, next to them */
		for (i = 0; conf && (conf[i].pid != 0 || conf[i].jid != 0) && n < VIEW_MAX_ENTRIES; i++) {
			if (conf[i].pid == pid && conf[i].jid != pinfo->defaultClass && ViewEntryValid(pid, conf[i].jid)) {
				sViewEntries[n].pid = pid;
				sViewEntries[n].jid = conf[i].jid;
				n++;
			}
		}
	}
	proc->count = n;

	/* start on the selected unit: its character, in its class if the list has it */
	proc->index = 0;
	for (i = 0; gActiveUnit && i < n; i++) {
		if (sViewEntries[i].pid != UNIT_CHAR_ID(gActiveUnit))
			continue;
		if (sViewEntries[proc->index].pid != sViewEntries[i].pid)
			proc->index = i;
		if (sViewEntries[i].jid == UNIT_CLASS_ID(gActiveUnit)) {
			proc->index = i;
			break;
		}
	}
}

/* Sets a w x h rect of a tilemap, from (x, y), to 0 (FE-CLib has no helper for it) */
static void ViewFillRect(u16 *tm, int x, int y, int w, int h)
{
	int i, j;

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++)
			tm[TILEMAP_INDEX(x + i, y + j)] = 0;
	}
}

/* The panel's interior on BG0 (its frame is on BG1): the labels are drawn over it */
static void ViewClearPanel(u16 *tm)
{
	ViewFillRect(tm, 1, 1, 19, 18);
}

static void ViewDrawLabel(struct ProcBanimViewer *proc)
{
	const struct ViewEntry *ent = &sViewEntries[proc->index];
	const struct ClassData *jinfo = GetClassData(ent->jid);
	struct Text *th = sViewText;
	bool custom;
	int anim = ViewAnimId(ViewAnimDef(ent->pid, ent->jid, &custom));

	ViewClearPanel(gBG0TilemapBuffer);
	ClearText(&th[0]);
	ClearText(&th[1]);
	ClearText(&th[2]);

	/* the animation's ID (as BattleAnims.event writes it), the character and the class above; the mode below */
	PutNumberHex(gBG0TilemapBuffer + TILEMAP_INDEX(4, 3), TEXT_COLOR_SYSTEM_GOLD, anim + 1);
	Text_SetColor(&th[0], TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&th[0], GetStringFromIndex(GetCharacterData(ent->pid)->nameTextId));
	PutText(&th[0], gBG0TilemapBuffer + TILEMAP_INDEX(6, 3));

	Text_SetColor(&th[1], TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[1], jinfo ? GetStringFromIndex(jinfo->nameTextId) : "");
	PutText(&th[1], gBG0TilemapBuffer + TILEMAP_INDEX(6, 5));

	Text_SetColor(&th[2], proc->crit ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[2], proc->crit ? "Critical" : "Normal");
	PutText(&th[2], gBG0TilemapBuffer + TILEMAP_INDEX(6, 16));

	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* The panel on the left: the map's BG1 is put back, and the frame drawn on it */
static void ViewDrawPanel(void)
{
	CpuCopy16(sViewBg1Backup, gBG1TilemapBuffer, 0x800);
	DrawUiFrame(gBG1TilemapBuffer, 0, 0, 21, 20, TILEREF(0, 0), 0);
	BG_EnableSyncByMask(BG1_SYNC_BIT);
}

/* (Re)starts the current entry's animation, attacking */
static void ViewLoadAnim(struct ProcBanimViewer *proc)
{
	const struct ViewEntry *ent = &sViewEntries[proc->index];
	bool custom;
	const struct BattleAnimDef *def = ViewAnimDef(ent->pid, ent->jid, &custom);

	sViewAnim->animId = ViewAnimId(def);
	sViewAnim->charPalId = ViewCharaPal(ent->pid, ent->jid, custom);
	sViewAnim->roundType = proc->crit ? ROUND_CRIT : ROUND_ATTACK;
	MiniAnimRestart(sViewAnim);
	proc->timer = 0;
}

static void BanimViewer_Init(struct ProcBanimViewer *proc)
{
	ViewBuildList(proc);
	proc->crit = false;
	proc->timer = 0;
}

static void BanimViewer_Setup(struct ProcBanimViewer *proc)
{
	struct BanimUnkStructComm *pf = sViewPlatform;
	struct AnimMagicFxBuffer *fx = sViewMagicFx;
	int i;

	/*
	 * The map stays on screen, the debugger's menu beside the panel. BG0 and BG1 are backed up for BanimViewer_End.
	 * The map's unit sprites are hidden by PutUnitSpritesOam while this proc runs (DebugBanimViewer_IsOpen).
	 */
	CpuCopy16(gBG0TilemapBuffer, sViewBg0Backup, 0x800);
	CpuCopy16(gBG1TilemapBuffer, sViewBg1Backup, 0x800);

	/* the moving-unit sprites (the selected unit's, too) are hidden; End shows the ones it hid */
	proc->hidden_mus = 0;
	for (i = 0; i < MU_MAX_COUNT; i++) {
		struct MuProc *mu = GetMu(i);

		if (mu) {
			HideMu(mu);
			proc->hidden_mus |= 1 << i;
		}
	}

	/* the font's tiles are the panel's text now: its allocator goes back on the way out */
	proc->chr_counter = gActiveFont->chr_counter;
	InitText(&sViewText[0], 8);
	InitText(&sViewText[1], 8);
	InitText(&sViewText[2], 5);

	/* the panel on the left: the frame on BG1, its interior cleared on BG0 for the labels */
	ViewFillRect(gBG0TilemapBuffer, 0, 0, 21, 20);
	ViewDrawPanel();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	proc->bg1_priority = BG_GetControlBuffer(BG_1)->priority;
	proc->bg2_priority = BG_GetControlBuffer(BG_2)->priority;
	BG_SetPriority(BG_1, VIEW_BG_PRIO);
	BG_SetPriority(BG_2, VIEW_BG_PRIO);

	NewEfxAnimeDrvProc();

	/* the platform: the left one, the right one off the screen */
	pf->terrain_l = 1;
	pf->pal_l = 10;
	pf->chr_l = 0x380;
	pf->terrain_r = 1;
	pf->pal_r = 11;
	pf->chr_r = 0x3C0;
	pf->distance = 0;
	pf->unk0E = -1;
	pf->unk1C = (void *)0x06010000;
	pf->unk20 = (void *)0x0201DB28;
	NewPlatform(pf);
	PlatformSetPos(pf, PLATFORM_X, PLATFORM_Y, 400, PLATFORM_Y);

	/* no spell: magicFuncIdx 0 is the class reel's Dummy, so the cast plays and the attack goes on */
	fx->magicFuncIdx = 0;
	fx->xOffsetBg = fx->yOffsetBg = 0;
	fx->xOffsetObj = fx->yOffsetObj = 0;
	fx->bgChr = fx->bgPalId = fx->objChr = fx->objPalId = fx->bg = 0;
	fx->bgTmBuf = NULL;
	fx->bgImgBuf = fx->bgTsaBuf = fx->objImgBuf = NULL;
	fx->resetCallback = NULL;

	sViewAnim->xPos = VIEW_X;
	sViewAnim->yPos = VIEW_Y;
	sViewAnim->genericPalId = 0;
	sViewAnim->state2 = 1;
	sViewAnim->oam2Tile = 0x180;
	sViewAnim->oam2Pal = 2;
	sViewAnim->pImgSheetBuf = (void *)0x02000038;
	sViewAnim->unk_24 = (void *)0x02002038;
	sViewAnim->unk_20 = (void *)0x02007838;
	sViewAnim->unk_28 = (void *)0x020078D8;
	sViewAnim->unk_30 = fx;
	sViewAnim->animId = 0;
	sViewAnim->charPalId = -1;
	sViewAnim->roundType = ROUND_STAND;
	NewEkrUnitMainMini(sViewAnim);

	ViewLoadAnim(proc);
	ViewDrawLabel(proc);
}

static void BanimViewer_Loop(struct ProcBanimViewer *proc)
{
	u16 keys = gKeyStatusPtr->repeatedKeys;
	int step = 0;

	if (proc->timer < VIEW_SETTLE) {
		proc->timer++;
		return;
	}

	if (gKeyStatusPtr->newKeys & (B_BUTTON | START_BUTTON)) {
		m4aSongNumStart(0x6B);
		Proc_Break(proc);
		return;
	}

	if (keys & DPAD_DOWN)
		step = 1;
	else if (keys & DPAD_UP)
		step = proc->count - 1;
	else if (keys & R_BUTTON)
		step = 10;
	else if (keys & L_BUTTON)
		step = proc->count - 10 % proc->count;

	if (step && proc->count > 1) {
		proc->index += step;
		while (proc->index >= proc->count)
			proc->index -= proc->count;
		m4aSongNumStart(0x66);
		ViewLoadAnim(proc);
		ViewDrawLabel(proc);
		return;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		proc->crit = !proc->crit;
		m4aSongNumStart(0x6A);
		ViewLoadAnim(proc);
		ViewDrawLabel(proc);
		return;
	}

	/* attack again a moment after the last attack ends */
	if (MiniAnimRoundDone(sViewAnim) && ++proc->timer > 40)
		ViewLoadAnim(proc);
}

static void BanimViewer_End(struct ProcBanimViewer *proc)
{
	int i;

	MiniAnimEnd(sViewAnim);
	EndPlatform(sViewPlatform);
	EndEfxAnimeDrvProc();

	CpuCopy16(sViewBg0Backup, gBG0TilemapBuffer, 0x800);
	CpuCopy16(sViewBg1Backup, gBG1TilemapBuffer, 0x800);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	gActiveFont->chr_counter = proc->chr_counter;
	BG_SetPriority(BG_1, proc->bg1_priority);
	BG_SetPriority(BG_2, proc->bg2_priority);
	for (i = 0; i < MU_MAX_COUNT; i++) {
		struct MuProc *mu = GetMu(i);

		if (mu && (proc->hidden_mus & (1 << i)))
			ShowMu(mu);
	}
	LoadObjUIGfx();
	ApplyUnitSpritePalettes();
	RefreshUnitSprites();
	ForceSyncUnitSpriteSheet();
}

static const struct ProcCmd ProcScr_BanimViewer[] = {
	PROC_NAME("DebugBanimViewer"),
	PROC_CALL(BanimViewer_Init),
	PROC_CALL(BanimViewer_Setup),
	PROC_REPEAT(BanimViewer_Loop),
	PROC_CALL(BanimViewer_End),
	PROC_END,
};

/* The debugger's menu starts it (C_Code.c, ViewBanimNow) */
void StartDebugBanimViewer(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_BanimViewer, parent);
}

/* The viewer is on screen: the map's unit sprites are not drawn (MirrorSprites.c, PutUnitSpritesOam) */
bool DebugBanimViewer_IsOpen(void)
{
	return Proc_Find(ProcScr_BanimViewer) != NULL;
}
