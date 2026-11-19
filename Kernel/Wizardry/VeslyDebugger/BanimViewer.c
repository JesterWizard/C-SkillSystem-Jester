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
 * on the platform, facing left as the player's does in battle, and the class, its ID and the variant above. The list
 * is by class: each class's own animation ("Generic"), then every character with a custom animation (CustomAnimeTable)
 * or a character palette in that class. Left and right go to the previous or next class, L and R ten classes at a
 * time; up and down step through the class's variants. A switches between the normal attack and the critical; B goes
 * back to the menu.
 *
 * The attack plays to its end: at each wait (C01, C02, C18: in battle, the HP bar draining) the engine stops and the
 * viewer resumes it a moment later, as the promotion screen does; once the mode has ended, it attacks again.
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
#define MiniAnimWaiting    sub_805A96C /* stopped at a wait (C01, C02, C18): either layer's nextRoundId is 0xFFFE */
#define MiniAnimResume     sub_805A990 /* goes on past the wait (state3 bit 3 on both layers) */
#define MiniAnimModeEnded  sub_805A9A4 /* the mode has ended: the front layer's nextRoundId is 0xFFFF */
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
#define VIEW_MAX_ENTRIES 0x180 /* sViewEntries has room for 0x180 (0x0200F200..0x0200F500) */
#define VIEW_LAST_CLASS 0x7F   /* CLASS_PUPIL_T1, the last row of gClassData_NEW (Data/ClassData/ClassData.c) */
/* frames before the viewer reads keys after it opens: the A that opened it is still in the key state for a few frames */
#define VIEW_SETTLE 12
/* frames after the last step before the new animation loads: holding a direction scrolls the list without reloading */
#define VIEW_RELOAD_DELAY 6
/* frames the attack stays at a wait (in battle, the HP bar draining) before it goes on */
#define VIEW_WAIT_HOLD 24
/* frames between the end of the attack and the next one */
#define VIEW_REPLAY_DELAY 40

/* pid 0: the class's own animation, with its own palette ("Generic") */
struct ViewEntry {
	u8 pid, jid;
};

struct ProcBanimViewer {
	PROC_HEADER;

	u16 count;
	u16 index;
	u8 crit;
	u8 settle;  /* frames since the viewer opened, up to VIEW_SETTLE */
	u16 chr_counter; /* the font's tile allocator, put back on the way out */
	u8 hidden_mus; /* the bits of the MU slots hidden while the viewer is up (the selected unit's MU too) */
	u8 bg1_priority, bg2_priority; /* the panel's and the map's BG priorities, put back on the way out */
	u8 reload;  /* frames left before the selected entry's animation loads (0: loaded) */
	u8 timer;   /* frames at the current wait, or since the attack ended */
};

static const struct BattleAnimDef *ViewAnimDef(int pid, int jid, bool *custom)
{
	const struct IndividualAnimConf *conf = GetPrIndividualAnimConf();
	const struct ClassData *jinfo;
	int i;

	*custom = false;
	if (conf && pid) {
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
	if (custom || !pid)
		return -1;
	for (i = 0; i < 7; i++) {
		if (gAnimCharaPalConfig[pid - 1][i] == jid)
			return gAnimCharaPalIt[pid - 1][i] - 1;
	}
	return -1;
}

static bool ViewEntryValid(int pid, int jid)
{
	bool custom;

	return ViewAnimId(ViewAnimDef(pid, jid, &custom)) >= 0;
}

static void ViewAddEntry(struct ProcBanimViewer *proc, int pid, int jid)
{
	if (proc->count >= VIEW_MAX_ENTRIES)
		return;
	sViewEntries[proc->count].pid = pid;
	sViewEntries[proc->count].jid = jid;
	proc->count++;
}

static void ViewBuildList(struct ProcBanimViewer *proc)
{
	const struct IndividualAnimConf *conf = GetPrIndividualAnimConf();
	u32 custom[8]; /* a bit per character with a CustomAnimeTable row for the class */
	int pid, jid, i;

	proc->count = 0;
	for (jid = 1; jid <= VIEW_LAST_CLASS; jid++) {
		const struct ClassData *jinfo = GetClassData(jid);
		bool generic;

		if (!jinfo || !jinfo->nameTextId)
			continue;
		generic = ViewEntryValid(0, jid);
		if (generic)
			ViewAddEntry(proc, 0, jid);

		CpuFill32(0, custom, sizeof(custom));
		for (i = 0; conf && (conf[i].pid != 0 || conf[i].jid != 0); i++) {
			if (conf[i].jid == jid && conf[i].pid)
				custom[conf[i].pid >> 5] |= 1 << (conf[i].pid & 31);
		}

		/* every named character with a custom animation, or a palette for the class's own, in the class */
		for (pid = 1; pid < 0x100; pid++) {
			const struct CharacterData *pinfo = GetCharacterData(pid);
			bool own;

			if (!pinfo || !pinfo->nameTextId)
				continue;
			if (custom[pid >> 5] & (1 << (pid & 31)))
				own = ViewEntryValid(pid, jid);
			else
				own = generic && ViewCharaPal(pid, jid, false) >= 0;
			if (own)
				ViewAddEntry(proc, pid, jid);
		}
	}

	/* start on the selected unit: its character in its class, else its class's own animation */
	proc->index = 0;
	for (i = 0; gActiveUnit && i < proc->count; i++) {
		if (sViewEntries[i].jid != UNIT_CLASS_ID(gActiveUnit))
			continue;
		if (sViewEntries[proc->index].jid != sViewEntries[i].jid)
			proc->index = i;
		if (sViewEntries[i].pid == UNIT_CHAR_ID(gActiveUnit)) {
			proc->index = i;
			break;
		}
	}
}

/* The first and one-past-last entries of the class that entry `index` is in */
static void ViewClassRange(struct ProcBanimViewer *proc, int index, int *first, int *last)
{
	int jid = sViewEntries[index].jid;

	*first = index;
	while (*first > 0 && sViewEntries[*first - 1].jid == jid)
		(*first)--;
	*last = index + 1;
	while (*last < proc->count && sViewEntries[*last].jid == jid)
		(*last)++;
}

/* The class number (from 1) of entry `index`, and the number of classes */
static int ViewClassOrdinal(struct ProcBanimViewer *proc, int index, int *total)
{
	int i, n = 0, at = 0;

	for (i = 0; i < proc->count; i++) {
		if (i == 0 || sViewEntries[i].jid != sViewEntries[i - 1].jid)
			n++;
		if (i == index)
			at = n;
	}
	*total = n;
	return at;
}

/* Moves to the first entry of the class `step` classes away (wrapping) */
static void ViewStepClass(struct ProcBanimViewer *proc, int step)
{
	int first, last, n;

	for (n = 0; n < (step < 0 ? -step : step); n++) {
		ViewClassRange(proc, proc->index, &first, &last);
		if (step > 0)
			proc->index = last < proc->count ? last : 0;
		else
			proc->index = first > 0 ? first - 1 : proc->count - 1;
	}
	ViewClassRange(proc, proc->index, &first, &last);
	proc->index = first;
}

/* Moves to the next (+1) or previous (-1) variant of the class (wrapping within it) */
static void ViewStepVariant(struct ProcBanimViewer *proc, int step)
{
	int first, last;

	ViewClassRange(proc, proc->index, &first, &last);
	if (step > 0)
		proc->index = proc->index + 1 < last ? proc->index + 1 : first;
	else
		proc->index = proc->index > first ? proc->index - 1 : last - 1;
}

/* "n/total" */
static void ViewFormatCount(char *buf, int n, int total)
{
	char tmp[8];
	int len, i;

	for (i = 0; i < 2; i++) {
		int v = i ? total : n;

		len = 0;
		do {
			tmp[len++] = '0' + DivRem(v, 10);
			v = Div(v, 10);
		} while (v);
		while (len)
			*buf++ = tmp[--len];
		if (!i)
			*buf++ = '/';
	}
	*buf = 0;
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
	int first, last, classes, i;
	char count[12];

	ViewClearPanel(gBG0TilemapBuffer);
	for (i = 0; i < 4; i++)
		ClearText(&th[i]);

	/* the class, its number of the classes; the animation's ID (as BattleAnims.event writes it) */
	PutNumberHex(gBG0TilemapBuffer + TILEMAP_INDEX(3, 1), TEXT_COLOR_SYSTEM_GOLD, anim + 1);
	Text_SetColor(&th[0], TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&th[0], jinfo ? GetStringFromIndex(jinfo->nameTextId) : "");
	ViewFormatCount(count, ViewClassOrdinal(proc, proc->index, &classes), classes);
	Text_SetCursor(&th[0], 80);
	Text_SetColor(&th[0], TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[0], count);
	PutText(&th[0], gBG0TilemapBuffer + TILEMAP_INDEX(5, 1));

	/* the variant: the class's own animation or a character's, its number of the class's variants */
	ViewClassRange(proc, proc->index, &first, &last);
	Text_SetColor(&th[1], ent->pid ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[1], ent->pid ? GetStringFromIndex(GetCharacterData(ent->pid)->nameTextId) : "Generic");
	ViewFormatCount(count, proc->index - first + 1, last - first);
	Text_SetCursor(&th[1], 80);
	Text_SetColor(&th[1], TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[1], count);
	PutText(&th[1], gBG0TilemapBuffer + TILEMAP_INDEX(5, 3));

	/* the mode, and the keys */
	Text_SetColor(&th[2], proc->crit ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_WHITE);
	Text_DrawString(&th[2], proc->crit ? "Critical" : "Normal");
	Text_SetCursor(&th[2], 56);
	Text_SetColor(&th[2], TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[2], "A: Normal/Critical");
	PutText(&th[2], gBG0TilemapBuffer + TILEMAP_INDEX(2, 15));

	/* the font has <, > and ^ (Fonts/Glyph/fe8u): </> is left and right, ^/v up and down */
	Text_SetColor(&th[3], TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(&th[3], "</> Class    ^/v Variant");
	PutText(&th[3], gBG0TilemapBuffer + TILEMAP_INDEX(2, 17));

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
	proc->reload = 0;
}

static void BanimViewer_Init(struct ProcBanimViewer *proc)
{
	ViewBuildList(proc);
	proc->crit = false;
	proc->settle = 0;
	proc->reload = 0;
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
	InitText(&sViewText[0], 14);
	InitText(&sViewText[1], 14);
	InitText(&sViewText[2], 17);
	InitText(&sViewText[3], 17);

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

/* The attack plays through its waits to its end, then again after a moment */
static void ViewUpdateAnim(struct ProcBanimViewer *proc)
{
	if (MiniAnimModeEnded(sViewAnim)) {
		if (++proc->timer > VIEW_REPLAY_DELAY)
			ViewLoadAnim(proc);
		return;
	}

	if (MiniAnimWaiting(sViewAnim)) {
		if (++proc->timer > VIEW_WAIT_HOLD) {
			MiniAnimResume(sViewAnim);
			proc->timer = 0;
		}
		return;
	}

	proc->timer = 0;
}

static void BanimViewer_Loop(struct ProcBanimViewer *proc)
{
	u16 keys = gKeyStatusPtr->repeatedKeys;
	int old = proc->index;

	if (proc->settle < VIEW_SETTLE) {
		proc->settle++;
		ViewUpdateAnim(proc);
		return;
	}

	if (gKeyStatusPtr->newKeys & (B_BUTTON | START_BUTTON)) {
		m4aSongNumStart(0x6B);
		Proc_Break(proc);
		return;
	}

	if (proc->count > 1) {
		if (keys & DPAD_RIGHT)
			ViewStepClass(proc, 1);
		else if (keys & DPAD_LEFT)
			ViewStepClass(proc, -1);
		else if (keys & R_BUTTON)
			ViewStepClass(proc, 10);
		else if (keys & L_BUTTON)
			ViewStepClass(proc, -10);
		else if (keys & DPAD_DOWN)
			ViewStepVariant(proc, 1);
		else if (keys & DPAD_UP)
			ViewStepVariant(proc, -1);
	}

	if (proc->index != old) {
		/* the labels follow at once; the animation loads once the selection has stayed for a few frames */
		m4aSongNumStart(0x66);
		ViewDrawLabel(proc);
		proc->reload = VIEW_RELOAD_DELAY;
		return;
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		proc->crit = !proc->crit;
		m4aSongNumStart(0x6A);
		ViewLoadAnim(proc);
		ViewDrawLabel(proc);
		return;
	}

	if (proc->reload) {
		if (--proc->reload == 0)
			ViewLoadAnim(proc);
		return;
	}

	ViewUpdateAnim(proc);
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
