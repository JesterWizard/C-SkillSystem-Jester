#include "common-chax.h"
#include "kernel-lib.h"
#include "district.h"
#include "event.h"
#include "eventscript.h"
#include "recap.h"
#include "constants/texts.h"
#include "constants/songs.h"

/*
 * "Previously on your quest..." (Documentation/Features/Recap.md)
 *
 * Recording, for the last story chapter played:
 *   - its first background (the opening) and the last one shown once it was won (the ending), from event command 0x21
 *     (Gallery_Event21, Kernel/Wizardry/Gallery);
 *   - up to two characters who died (gKillUnitHooks) or joined (the army at each background shown, the ending's
 *     included, against the army when the chapter's recap started);
 *   - the first two portraits to speak in the opening (until the chapter's first phase, Recap_PrePhase) and once it
 *     was won (TextEngine_LoadFace, Kernel/Wizardry/TextEngineRework).
 * A CG wins: one shown during the opening replaces an ordinary opening background, and once the ending has a CG,
 * only another CG replaces it.
 * A chapter's recap starts with its opening (CallBeginningEvents), before an opening's cutscene map loads change
 * gPlaySt.chapterIndex (the prologue's load Chapter 16's map and Outside Renais). Saving never starts one: on the
 * world map the next chapter is already set.
 * A district visit is not a story chapter: it neither resets nor adds to the recap.
 *
 * Showing: the save menu's last step (ProcScr_SaveMenu, PostSaveMenuHandler) loads the save; Recap_PostSaveMenu runs
 * it, then, if that save had a recap, shows each scene in greyscale before the game goes on. A: next, START: skip.
 */

#define RECAP_TEXT_W  26 /* tiles: the box's inside */
#define RECAP_LINES   2
#define RECAP_BOX_Y   14 /* the summary box: rows 14-19 */
#define RECAP_FACE_X  120 /* one portrait, centred; two: RECAP_FACE_X -/+ RECAP_FACE_DX */
#define RECAP_FACE_DX 60
#define RECAP_FACE_Y  40  /* the portraits' feet go behind the summary box, as in a conversation */
/* Sprite layer 2: in front of the background (BG3), behind the box (BG1) and the text (BG0) */
#define RECAP_FACE_DISP (FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_2))

/* ---- Recording ---- */

/* Backgrounds_224_256_Colors' background table: a tsa of 0 or 1 marks a 256- or 224-colour background */
struct RecapBgEntry {
	const void *gfx;
	const void *tsa;
	const u16 *pal;
};

extern const struct RecapBgEntry bgTable[];

static bool Recap_IsMultiPalBg(int bg)
{
	return bg != RECAP_NONE && (uintptr_t)bgTable[bg].tsa <= 1;
}


static bool Recap_Recording(void)
{
	return gRecap.data.magic == RECAP_MAGIC && !District_IsDistrictChapter(gPlaySt.chapterIndex);
}

static bool Recap_InArmy(int pid)
{
	return (gRecap.army[pid >> 5] & (1u << (pid & 31))) != 0;
}

static void Recap_MarkArmy(int pid)
{
	gRecap.army[pid >> 5] |= 1u << (pid & 31);
}

/* The army's characters now (ids 1-0x3F, alive) */
static void Recap_SnapshotArmy(void)
{
	int i, pid;
	struct Unit *unit;

	gRecap.army[0] = gRecap.army[1] = 0;
	for (i = 1; i < 0x40; i++) {
		unit = GetUnit(i);
		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;
		pid = UNIT_CHAR_ID(unit);
		if (pid < 0x40)
			Recap_MarkArmy(pid);
	}
}

/* The chapter's opening starts (CallBeginningEvents): a new recap, but for a district */
static void Recap_Start(void)
{
	if (District_IsDistrictChapter(gPlaySt.chapterIndex))
		return;

	CpuFill16(0, &gRecap, sizeof(gRecap));
	gRecap.data.magic = RECAP_MAGIC;
	gRecap.data.chapter = gPlaySt.chapterIndex;
	gRecap.data.openBg = RECAP_NONE;
	gRecap.data.endBg = RECAP_NONE;
	gRecap.data.lastBg = RECAP_NONE;
	gRecap.data.unitBg[0] = RECAP_NONE;
	gRecap.data.unitBg[1] = RECAP_NONE;
	Recap_SnapshotArmy();
}

static void Recap_AddJoins(void);

/* Event command 0x21 (Gallery_Event21): a background is shown */
void Recap_NoteBackground(int bg)
{
	if (!Recap_Recording() || bg < 0 || bg >= RECAP_NONE)
		return;

	Recap_AddJoins();

	gRecap.data.lastBg = bg;

	if (gRecap.data.openBg == RECAP_NONE)
		gRecap.data.openBg = bg;
	else if (!(gRecap.data.flags & RECAP_OPENING_OVER) && Recap_IsMultiPalBg(bg) &&
		 !Recap_IsMultiPalBg(gRecap.data.openBg))
		gRecap.data.openBg = bg;

	if (CheckFlag(EVFLAG_WIN) && (!Recap_IsMultiPalBg(gRecap.data.endBg) || Recap_IsMultiPalBg(bg)))
		gRecap.data.endBg = bg;
}

/* TextEngine_LoadFace: a portrait speaks in an event (not a battle's quote) */
void Recap_NoteFace(int fid)
{
	u16 *faces;

	if (!Recap_Recording() || fid <= 0)
		return;

	if (CheckFlag(EVFLAG_WIN))
		faces = gRecap.data.endFace;
	else if (!(gRecap.data.flags & RECAP_OPENING_OVER))
		faces = gRecap.data.openFace;
	else
		return;

	if (faces[0] == fid || faces[1] == fid)
		return;

	if (faces[0] == 0)
		faces[0] = fid;
	else if (faces[1] == 0)
		faces[1] = fid;
}

/* gPrePhaseFuncs: the chapter's first phase starts, its opening is over */
bool Recap_PrePhase(ProcPtr proc)
{
	if (Recap_Recording())
		gRecap.data.flags |= RECAP_OPENING_OVER;
	return false;
}

static void Recap_AddUnit(int kind, int pid)
{
	struct RecapData *d = &gRecap.data;
	int i;

	if (d->unitCount >= RECAP_UNITS)
		return;

	for (i = 0; i < d->unitCount; i++)
		if (d->unitPid[i] == pid)
			return;

	d->unitKind[d->unitCount] = kind;
	d->unitPid[d->unitCount] = pid;
	d->unitBg[d->unitCount] = d->lastBg;
	d->unitCount++;
}

/* gKillUnitHooks: a character of the army died (battle-unit copies are skipped, as GaidenMagicUses_OnUnitKill does) */
void Recap_OnUnitKill(struct Unit *unit)
{
	if (!Recap_Recording() || !UNIT_IS_VALID(unit) || GetUnit(unit->index) != unit)
		return;

	if (UNIT_FACTION(unit) != FACTION_BLUE || UNIT_CHAR_ID(unit) >= 0x40)
		return;

	Recap_AddUnit(RECAP_DIED, UNIT_CHAR_ID(unit));
}

/* Characters in the army now who were not when the recap started (not in the prologue: the lords arrive there) */
static void Recap_AddJoins(void)
{
	int i, pid;
	struct Unit *unit;

	if (!Recap_Recording() || gRecap.data.chapter != gPlaySt.chapterIndex || gRecap.data.chapter == 0)
		return;

	for (i = 1; i < 0x40; i++) {
		unit = GetUnit(i);
		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;
		pid = UNIT_CHAR_ID(unit);
		if (pid >= 0x40 || Recap_InArmy(pid))
			continue;
		Recap_MarkArmy(pid);
		Recap_AddUnit(RECAP_JOINED, pid);
	}
}

/* ---- The save tail (SaveTail.c) ---- */

void Recap_SaveTail(u8 *dst, const u32 size)
{
	if (size < sizeof(struct RecapData))
		return;

	Recap_AddJoins();
	WriteAndVerifySramFast(&gRecap.data, dst, sizeof(struct RecapData));
}

void Recap_LoadTail(u8 *src, const u32 size)
{
	struct RecapData buf;

	CpuFill16(0, &gRecap, sizeof(gRecap));
	if (src == NULL || size < sizeof(buf))
		return;

	ReadSramFast(src, &buf, sizeof(buf));
	if (buf.magic != RECAP_MAGIC)
		return;

	gRecap.data = buf;
	gRecap.pending = true;

	/* The army as saved (its units are loaded before the tail): who joins from here on is new */
	Recap_SnapshotArmy();
}

/* ---- The screen ---- */

struct ProcRecap {
	PROC_HEADER;
	/* 29 */ u8 index;
	/* 2A */ u8 total;
	/* 2B */ u8 kind[RECAP_SCENES];
	/* 2F */ u8 pid[RECAP_SCENES];
	/* 33 */ u8 bg[RECAP_SCENES];
	/* 37 */ u8 faces;	/* portraits on screen (slots 0 and 1) */
	/* 38 */ struct Text header;
	/* 40 */ struct Text lines[RECAP_LINES];
};

static void Recap_AddScene(struct ProcRecap *proc, int kind, int pid, int bg)
{
	if (proc->total >= RECAP_SCENES)
		return;

	proc->kind[proc->total] = kind;
	proc->pid[proc->total] = pid;
	proc->bg[proc->total] = bg;
	proc->total++;
}

/* The scenes, in order: the opening, who died or joined, the ending */
static void Recap_BuildScenes(struct ProcRecap *proc)
{
	const struct RecapData *d = &gRecap.data;
	int i, bg;

	proc->total = 0;
	if (d->openBg != RECAP_NONE)
		Recap_AddScene(proc, RECAP_OPENING, 0, d->openBg);

	for (i = 0; i < d->unitCount && i < RECAP_UNITS; i++) {
		bg = d->unitBg[i] != RECAP_NONE ? d->unitBg[i] : d->openBg;
		Recap_AddScene(proc, d->unitKind[i], d->unitPid[i], bg);
	}

	if (d->endBg != RECAP_NONE)
		Recap_AddScene(proc, RECAP_ENDING, 0, d->endBg);
}

/* dst gets src appended, within max bytes (with the end); returns dst's new length */
static int Recap_Append(char *dst, int len, const char *src, int max)
{
	while (*src && len < max - 1)
		dst[len++] = *src++;
	dst[len] = '\0';
	return len;
}

static int Recap_ChapterMsg(int chapter, bool ending)
{
	const struct RecapChapterText *it;

	for (it = gRecapChapterTexts; it->chapter != 0xFF; it++)
		if (it->chapter == chapter)
			return ending ? it->end : it->open;
	return 0;
}

/* The scene's summary, into buf */
static void Recap_Summary(struct ProcRecap *proc, char *buf, int max)
{
	int i = proc->index, msg, len;

	buf[0] = '\0';
	switch (proc->kind[i]) {
	case RECAP_OPENING:
	case RECAP_ENDING:
		msg = Recap_ChapterMsg(gRecap.data.chapter, proc->kind[i] == RECAP_ENDING);
		if (msg == 0)
			msg = proc->kind[i] == RECAP_ENDING ? MSG_RECAP_ENDING : MSG_RECAP_OPENING;
		Recap_Append(buf, 0, GetStringFromIndex(msg), max);
		break;

	case RECAP_DIED:
	case RECAP_JOINED:
		len = Recap_Append(buf, 0, GetStringFromIndex(GetCharacterData(proc->pid[i])->nameTextId), max);
		len = Recap_Append(buf, len, " ", max);
		Recap_Append(buf, len, GetStringFromIndex(proc->kind[i] == RECAP_DIED ? MSG_RECAP_FELL : MSG_RECAP_JOINED), max);
		break;
	}
}

static void Recap_PutLine(struct Text *t, int y, const char *str)
{
	ClearText(t);
	Text_InsertDrawString(t, 0, TEXT_COLOR_SYSTEM_WHITE, str);
	PutText(t, gBG0TilemapBuffer + TILEMAP_INDEX(2, y));
}

/* The summary, word-wrapped onto the box's lines */
static void Recap_DrawSummary(struct ProcRecap *proc, const char *str)
{
	char line[0x60], cand[0x60];
	int n = 0, len = 0, wl, clen;
	const char *w;

	line[0] = '\0';
	while (*str && n < RECAP_LINES) {
		w = str;
		while (*w == ' ')
			w++;
		if (*w == '\0')
			break;
		for (wl = 0; w[wl] && w[wl] != ' '; wl++)
			;

		/* the line with this word */
		clen = Recap_Append(cand, 0, line, sizeof(cand));
		if (clen)
			clen = Recap_Append(cand, clen, " ", sizeof(cand));
		while (wl-- > 0 && clen < (int)sizeof(cand) - 1)
			cand[clen++] = *w++;
		cand[clen] = '\0';

		if (len && GetStringTextLen(cand) > RECAP_TEXT_W * 8) {
			/* the word goes on the next line */
			Recap_PutLine(&proc->lines[n], RECAP_BOX_Y + 1 + 2 * n, line);
			n++;
			while (*str == ' ')
				str++;
			len = 0;
			line[0] = '\0';
			continue;
		}

		len = Recap_Append(line, 0, cand, sizeof(line));
		str = w;
	}

	if (len && n < RECAP_LINES)
		Recap_PutLine(&proc->lines[n], RECAP_BOX_Y + 1 + 2 * n, line);
}

static void Recap_DrawHeader(struct ProcRecap *proc)
{
	char buf[0x50];
	int len;

	len = Recap_Append(buf, 0, GetStringFromIndex(MSG_RECAP_HEADER), sizeof(buf));
	len = Recap_Append(buf, len, "  ", sizeof(buf));
	if (len < (int)sizeof(buf) - 4) {
		buf[len++] = '0' + proc->index + 1;
		buf[len++] = '/';
		buf[len++] = '0' + proc->total;
		buf[len] = '\0';
	}

	ClearText(&proc->header);
	Text_InsertDrawString(&proc->header, 0, TEXT_COLOR_SYSTEM_WHITE, buf);
	PutText(&proc->header, gBG0TilemapBuffer + TILEMAP_INDEX(1, 1));
}

/*
 * The background as the events show it (vanilla's EventShowTextBgDirect, as the Gallery does): the table is
 * Backgrounds_224_256_Colors' (bgTable), with its 224-colour CGs, which leave BG palettes 0 and 1 to the text and the
 * frames.
 */
/* Every non-empty tile of a 32x32 tilemap that uses palette `from` uses palette `to` instead */
static void Recap_RemapPalette(u16 *tilemap, int from, int to)
{
	int i;

	for (i = 0; i < 0x400; i++)
		if ((tilemap[i] & 0x3FF) && (tilemap[i] >> 12) == from)
			tilemap[i] = (tilemap[i] & 0x0FFF) | (to << 12);
}

/*
 * A 224-colour CG keeps BG palettes 0, 1 and 4-15 and leaves 2 and 3 (Backgrounds_224_256_Colors, CGC_LoadMultiPalBG):
 * the text (palette 0) and the frames (palette 1) move there, and the CG gets its palettes 0 and 1 back.
 */
static void Recap_MoveUiPalettes(int bg)
{
	CpuCopy16(gPaletteBuffer + 0x00, gPaletteBuffer + 0x20, 0x20);
	CpuCopy16(gPaletteBuffer + 0x10, gPaletteBuffer + 0x30, 0x20);
	CpuCopy16(bgTable[bg].pal, gPaletteBuffer, 0x40);

	Recap_RemapPalette(gBG0TilemapBuffer, 0, 2);
	Recap_RemapPalette(gBG0TilemapBuffer, 1, 3);
	Recap_RemapPalette(gBG1TilemapBuffer, 0, 2);
	Recap_RemapPalette(gBG1TilemapBuffer, 1, 3);
}

static void Recap_DrawBackground(int bg)
{
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	if (bg == RECAP_NONE) {
		BG_Fill(gBG3TilemapBuffer, 0);
		return;
	}

	EventShowTextBgDirect(EVSUBCMD_REMOVEPORTRAITS, bg);
}

static void Recap_EndFaces(struct ProcRecap *proc)
{
	int n;

	for (n = 0; n < proc->faces; n++)
		EndFaceById(n);
	proc->faces = 0;
}

/* A portrait, in slot proc->faces; x is its centre */
static struct FaceProc *Recap_PutFace(struct ProcRecap *proc, int fid, int x, bool flipped)
{
	struct FaceProc *face;

	if (fid <= 0 || proc->faces >= RECAP_FACES)
		return NULL;

	face = StartFace(proc->faces, fid, x, RECAP_FACE_Y, RECAP_FACE_DISP | (flipped ? FACE_DISP_FLIPPED : 0));
	proc->faces++;
	return face;
}

/* The opening's or the ending's speakers: one in the middle, or two facing each other */
static void Recap_PutSpeakers(struct ProcRecap *proc, const u16 *faces)
{
	if (faces[0] && faces[1]) {
		Recap_PutFace(proc, faces[0], RECAP_FACE_X - RECAP_FACE_DX, true);
		Recap_PutFace(proc, faces[1], RECAP_FACE_X + RECAP_FACE_DX, false);
	} else if (faces[0]) {
		Recap_PutFace(proc, faces[0], RECAP_FACE_X, false);
	}
}

static void Recap_Init(struct ProcRecap *proc)
{
	proc->index = 0;
	proc->faces = 0;
	Recap_BuildScenes(proc);

	SetupBackgrounds(NULL);
	ResetFaces();
	EndHelpPromptSprite(); /* the save menu's "R: Info" sprite, still alive under it */
}

/* The text and the frames, for each scene: the fade between scenes darkens their palettes too */
static void Recap_SetupText(struct ProcRecap *proc)
{
	int n;

	ResetText();
	InitSystemTextFont();
	LoadUiFrameGraphics();

	InitText(&proc->header, 28);
	for (n = 0; n < RECAP_LINES; n++)
		InitText(&proc->lines[n], RECAP_TEXT_W);
}

/* The scene, drawn while the screen is black */
static void Recap_DrawScene(struct ProcRecap *proc)
{
	char buf[0x80];
	int i = proc->index;
	struct FaceProc *face;

	Recap_EndFaces(proc);

	/* The background first: a 224-colour CG sets up the BG layers its own way and clears their tilemaps */
	SetupBackgrounds(NULL);
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	Recap_DrawBackground(proc->bg[i]);
	Recap_SetupText(proc);

	DrawUiFrame2(0, 0, 30, 4, 0);
	DrawUiFrame2(0, RECAP_BOX_Y, 30, 20 - RECAP_BOX_Y, 0);
	Recap_DrawHeader(proc);

	Recap_Summary(proc, buf, sizeof(buf));
	Recap_DrawSummary(proc, buf);

	/* The opening's and the ending's speakers, but not over a CG: it is the scene already */
	switch (proc->kind[i]) {
	case RECAP_OPENING:
		if (!Recap_IsMultiPalBg(proc->bg[i]))
			Recap_PutSpeakers(proc, gRecap.data.openFace);
		break;

	case RECAP_ENDING:
		if (!Recap_IsMultiPalBg(proc->bg[i]))
			Recap_PutSpeakers(proc, gRecap.data.endFace);
		break;

	case RECAP_DIED:
	case RECAP_JOINED:
		face = Recap_PutFace(proc, GetCharacterData(proc->pid[i])->portraitId, RECAP_FACE_X, false);
		if (face && proc->kind[i] == RECAP_DIED)
			sub_80064D4(face, 2); /* the fallen close their eyes (the text engine's [CloseEyes]) */
		break;
	}

	if (Recap_IsMultiPalBg(proc->bg[i]))
		Recap_MoveUiPalettes(proc->bg[i]);

	SetDispEnable(1, 1, 0, 1, 1);
	SetDefaultColorEffects();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
}

/* Every colour in grey (a frame after drawing: the portrait's palette is in by then) */
static void Recap_Greyscale(struct ProcRecap *proc)
{
	int i, c, l;

	for (i = 0; i < 0x200; i++) {
		c = gPaletteBuffer[i];
		l = ((c & 0x1F) * 77 + ((c >> 5) & 0x1F) * 150 + ((c >> 10) & 0x1F) * 29) >> 8;
		gPaletteBuffer[i] = l | (l << 5) | (l << 10);
	}
	EnablePaletteSync();
}

static void Recap_WaitFade(struct ProcRecap *proc)
{
	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

static void Recap_Loop(struct ProcRecap *proc)
{
	u16 press = gKeyStatusPtr->newKeys;

	if (press & (START_BUTTON | B_BUTTON)) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		proc->index = proc->total;
		Proc_Break(proc);
		return;
	}

	if (press & A_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		Proc_Break(proc);
	}
}

static void Recap_Next(struct ProcRecap *proc)
{
	if (proc->index < proc->total)
		proc->index++;
	if (proc->index < proc->total)
		Proc_Goto(proc, 0);
}

static void Recap_OnEnd(struct ProcRecap *proc)
{
	Recap_EndFaces(proc);

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	gRecap.pending = false;
}

static const struct ProcCmd ProcScr_Recap[] = {
	PROC_NAME("Recap"),
	PROC_SET_END_CB(Recap_OnEnd),
	PROC_CALL(Recap_Init),

	PROC_LABEL(0),
	PROC_CALL(Recap_DrawScene),
	PROC_YIELD,
	PROC_CALL(Recap_Greyscale),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Recap_WaitFade),
	PROC_REPEAT(Recap_Loop),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Recap_WaitFade),
	PROC_CALL(Recap_Next),
	PROC_END,
};

/* Vanilla's (FE8U 0x080153D4), but the chapter's recap starts first */
LYN_REPLACE_CHECK(CallBeginningEvents);
int CallBeginningEvents(void)
{
	const struct ChapterEventGroup *events = GetChapterEventDataPointer(gPlaySt.chapterIndex);

	Recap_Start();

	if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
		CallEvent(events->beginningSceneEvents, 1);
	else
		CallEvent((const u16 *)EventScr_SkirmishCommonBeginning, 1);

	return 0;
}

static int Recap_SceneCount(void)
{
	const struct RecapData *d = &gRecap.data;

	return (d->openBg != RECAP_NONE) + d->unitCount + (d->endBg != RECAP_NONE);
}

/*
 * ProcScr_SaveMenu's last step (Recap_Installer.event): vanilla's PostSaveMenuHandler loads the save (Resume or
 * Restart), whose tail sets gRecap.pending; the recap then plays, the screen still black, and the save menu ends after it.
 */
void Recap_PostSaveMenu(ProcPtr proc)
{
	gRecap.pending = false;
	PostSaveMenuHandler((struct SaveMenuProc *)proc);

	if (gRecap.pending && gRecap.data.magic == RECAP_MAGIC && Recap_SceneCount() > 0)
		Proc_StartBlocking(ProcScr_Recap, proc);
	else
		gRecap.pending = false;
}

/* ---- The story summaries ---- */

const struct RecapChapterText gRecapChapterTexts[] = {
	{ 0x00, MSG_RECAP_CH00_OPEN, MSG_RECAP_CH00_END },
	{ 0x01, MSG_RECAP_CH01_OPEN, MSG_RECAP_CH01_END },
	{ 0x38, MSG_RECAP_INTERMISSION_OPEN, 0 },
	{ 0x02, MSG_RECAP_CH02_OPEN, MSG_RECAP_CH02_END },
	{ 0x03, MSG_RECAP_CH03_OPEN, MSG_RECAP_CH03_END },
	{ 0x04, MSG_RECAP_CH04_OPEN, MSG_RECAP_CH04_END },
	{ 0x06, MSG_RECAP_CH05_OPEN, MSG_RECAP_CH05_END },
	{ 0x05, MSG_RECAP_CH05X_OPEN, MSG_RECAP_CH05X_END },
	{ 0x07, MSG_RECAP_CH06_OPEN, MSG_RECAP_CH06_END },
	{ 0x08, MSG_RECAP_CH07_OPEN, MSG_RECAP_CH07_END },
	{ 0x09, MSG_RECAP_CH08_OPEN, MSG_RECAP_CH08_END },
	{ 0x0A, MSG_RECAP_CH09_OPEN, MSG_RECAP_CH09_END },
	{ 0xFF, 0, 0 },
};
