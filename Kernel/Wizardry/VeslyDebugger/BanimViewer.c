#include "common-chax.h"
#include "ekrbattle.h"
#include "efxbattle.h"
#include "bmio.h"
#include "bmlib.h"
#include "feb-compat.h"
#include "bmudisp.h"

/*
 * Debugger, "Battle anim" (VeslyDebugger_Installer.event, page 3): a screen of its own with one platform on the left
 * and a battle animation on it, its ID and the character's name above. Up and down step through the characters (each
 * in their default class, then every character-and-class row of CustomAnimeTable), L and R by ten; A switches between
 * the normal attack and the critical; B goes back to the map.
 *
 * The animation runs on the game's mini battle-animation engine, the one the class reel on the title screen uses
 * (ClassInfoDisplay_Init, FE8U 0x080B37FC): the same AnimBuffer, buffers and platform, at the same addresses. They are
 * battle-animation work RAM, free outside a battle. No spell effects: the class reel's spell buffers reach map RAM.
 */

/* The class reel's (gOpInfoData and its buffers, gUnk_Opinfo_0201DB00) */
#define sViewAnim     ((struct AnimBuffer *)0x02000000)
#define sViewMagicFx  ((struct AnimMagicFxBuffer *)0x0200A2D8)
#define sViewPlatform ((struct BanimUnkStructComm *)0x0201DB00)
/* The list, after the class reel's buffers (battle-animation work RAM too) */
#define sViewEntries  ((struct ViewEntry *)0x0200CB00)
#define sViewText     ((struct Text *)(0x0200CB00 + 2 * VIEW_MAX_ENTRIES))

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

#define VIEW_X 128 /* the animation's position, in the engine's space (the class reel's is x 260, on the right) */
#define VIEW_Y 88
#define PLATFORM_X 104
#define PLATFORM_Y 104

#define VIEW_MAX_ENTRIES 0x180

struct ViewEntry {
	u8 pid, jid;
};

struct ProcBanimViewer {
	PROC_HEADER;

	u16 count;
	u16 index;
	u8 crit;
	u8 timer;
	u16 backdrop;
	s16 camera_x, camera_y; /* the map's, put back on the way out */
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

static void ViewDrawLabel(struct ProcBanimViewer *proc)
{
	const struct ViewEntry *ent = &sViewEntries[proc->index];
	const struct ClassData *jinfo = GetClassData(ent->jid);
	struct Text *th = sViewText;
	bool custom;
	int anim = ViewAnimId(ViewAnimDef(ent->pid, ent->jid, &custom));

	BG_Fill(gBG0TilemapBuffer, 0);
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

/*
 * A custom animation stores one OAM for both sides (Tools/banim/feditor_dump.py) and leaves the left one to
 * AutoGenLeftOAM, which only hooks the full battle's UpdateBanimFrame. The mini engine has just decompressed that OAM
 * for the left side: mirror it the way AutoGenLeftOAM does (every entry but the end codes and the special ones gets
 * the h-flip bit and x = -x - width).
 */
static const u8 sObjWidth[3][4] = {
	{ 8, 16, 32, 64 },  /* square */
	{ 16, 32, 32, 64 }, /* wide */
	{ 8, 8, 16, 32 },   /* tall */
};

static void ViewMirrorOam(int anim)
{
	const struct BattleAnim *banim = &banim_data[anim];
	u8 *oam = sViewAnim->unk_24;
	int size, k;

	if (banim->oam_l != banim->oam_r)
		return;

	size = *(const u32 *)banim->oam_r >> 8;
	for (k = 0; k + 12 <= size; k += 12) {
		u16 a0 = oam[k] | (oam[k + 1] << 8);
		u16 a1 = oam[k + 2] | (oam[k + 3] << 8);
		s16 x = oam[k + 6] | (oam[k + 7] << 8);
		int shape = a0 >> 14, sz = a1 >> 14;

		if (oam[k] == 1 || a1 == 0xFFFF || shape > 2)
			continue;
		a1 |= 0x1000;
		x = -x - sObjWidth[shape][sz];
		oam[k + 2] = a1;
		oam[k + 3] = a1 >> 8;
		oam[k + 6] = x;
		oam[k + 7] = x >> 8;
	}
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
	ViewMirrorOam(sViewAnim->animId);
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
	struct AnimMagicFxBuffer *fx = sViewMagicFx;
	struct BanimUnkStructComm *pf = sViewPlatform;

	BMapDispSuspend();
	/*
	 * The map still draws its units' sprites (whose tiles the animation and the platform now use): move the camera
	 * far off the map, and they are all off the screen.
	 */
	proc->camera_x = gBmSt.camera.x;
	proc->camera_y = gBmSt.camera.y;
	gBmSt.camera.x = 0x4000;
	gBmSt.camera.y = 0x4000;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	BG_SetPosition(BG_0, 0, 0);

	proc->backdrop = gPaletteBuffer[0];
	gPaletteBuffer[0] = 0;
	EnablePaletteSync();
	SetDispEnable(TRUE, FALSE, FALSE, FALSE, TRUE);

	/* the text from scratch, as the debugger's other screens do (SomeMenuInit) */
	ResetTextFont();
	SetTextFontGlyphs(0);
	SetTextFont(0);
	ResetText();
	InitText(&sViewText[0], 8);
	InitText(&sViewText[1], 8);
	InitText(&sViewText[2], 5);

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
	MiniAnimEnd(sViewAnim);
	EndPlatform(sViewPlatform);
	EndEfxAnimeDrvProc();

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
	gPaletteBuffer[0] = proc->backdrop;
	EnablePaletteSync();

	gBmSt.camera.x = proc->camera_x;
	gBmSt.camera.y = proc->camera_y;
	BMapDispResume();
	RefreshBMapGraphics();
	SetDispEnable(TRUE, TRUE, TRUE, TRUE, TRUE);
	LoadObjUIGfx();
	ApplyUnitSpritePalettes();
	ForceSyncUnitSpriteSheet();
	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static const struct ProcCmd ProcScr_BanimViewer[] = {
	PROC_NAME("DebugBanimViewer"),
	PROC_CALL(BanimViewer_Init),
	PROC_CALL(StartMidLockingFadeToBlack),
	PROC_YIELD,
	PROC_CALL(BanimViewer_Setup),
	PROC_CALL(StartMidLockingFadeFromBlack),
	PROC_YIELD,
	PROC_REPEAT(BanimViewer_Loop),
	PROC_CALL(StartMidLockingFadeToBlack),
	PROC_YIELD,
	PROC_CALL(BanimViewer_End),
	PROC_CALL(StartMidLockingFadeFromBlack),
	PROC_YIELD,
	PROC_END,
};

/* The debugger proc waits for it (C_Code.c, BanimViewLabel) */
void StartDebugBanimViewer(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_BanimViewer, parent);
}
