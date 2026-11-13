#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "skill-codex.h"
#include "extras-ui.h"
#include "constants/texts.h"
#include "constants/songs.h"

/*
 * The Skill Codex screen (Documentation/Features/SkillCodex.md), started from Extras > Skill Codex.
 *
 *  - The banner on top (the Sound Room's song banner, BG2): the name of the skill under the cursor, or "???".
 *  - On the left, in a window: "Skills", five names (the skills not met yet as "???"), how many are met; the hand on
 *    the name under the cursor. Up and down move it, left and right five rows, L and R to the previous or next skill
 *    met.
 *  - On the right, in a window: the skill's icon and name, and its description, wrapped, up to six lines.
 *  - B goes back to Extras.
 *
 * The list is every skill id with a description (SkillCodex_Listed), in id order. It is not kept as a table: the
 * cursor and the top row are skill ids, and moving walks the ids.
 */

#define VISIBLE_ROWS   5

/* The list's window and its rows (tiles) */
#define LIST_X         0
#define LIST_Y         4
#define LIST_W         12
#define LIST_H         16
#define TAB_Y          (LIST_Y + 1)
#define ROW_Y          (LIST_Y + 3)
#define NAME_X         (LIST_X + 3)
#define COUNT_Y        (LIST_Y + 13)

/* The info window (tiles) */
#define INFO_X         12
#define INFO_Y         4
#define INFO_W         18
#define INFO_H         16
#define ICON_X         (INFO_X + 1)
#define ICON_Y         (INFO_Y + 1)
#define SUB_X          (INFO_X + 4)
#define DESC_X         (INFO_X + 1)
#define DESC_Y         (INFO_Y + 3)
#define DESC_LINES     6
#define INFO_IN_W      16 /* tiles inside the window */

#define PAL_ICONS      4 /* and 5: the skill icons' banks */
#define PAL_MURAL      14 /* and 15 */

#define SE_MOVE  SONG_SE_SYS_CURSOR_UD1
#define SE_PAGE  SONG_SE_SYS_CURSOR_LR1
#define SE_BAD   SONG_6C
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1

struct SkillCodexScreen {
	u16 cursor;	/* skill ids */
	u16 top;
	u16 count;	/* skills listed */
	u16 met;	/* of them, met */
	struct SkillCodexBits bits;	/* every save's, together */
	struct Text title;
	struct Text heading;
	struct Text countText;
	struct Text names[VISIBLE_ROWS];
	struct Text name;
	struct Text desc[DESC_LINES];
};

/* The Extras screens never run together: the codex uses the Bestiary screen's RAM */
extern u8 gBestiaryScreen[];
#define CS ((struct SkillCodexScreen *)gBestiaryScreen)

_Static_assert(sizeof(struct SkillCodexScreen) <= 0x160, "the codex fits in gBestiaryScreen (0x160 bytes)");

struct ProcSkillCodex {
	PROC_HEADER;
	struct DispCnt dispBak;
};

enum {
	L_CODEX_MAIN = 1,
	L_CODEX_EXIT,
};

/* BG0 text, BG1 the windows, BG2 the banner and the icon, BG3 the mural */
static const u16 sCodexBgConfig[12] = {
	0x0000, 0x6000, 0,
	0x0000, 0x6800, 0,
	0x0000, 0x7000, 0,
	0x8000, 0x7800, 0,
};

/* ---- Walking the list ---- */

static bool SkillCodex_Met(int sid)
{
	return SkillCodex_BitSet(CS->bits.skills, sid);
}

/* The next listed skill after sid that way (step 1 or -1), or -1 if none */
static int SkillCodex_Step(int sid, int step)
{
	for (sid += step; sid > 0 && sid < SKILL_CODEX_SIDS; sid += step)
		if (SkillCodex_Listed(sid))
			return sid;

	return -1;
}

/* n listed skills that way, stopping at the end */
static int SkillCodex_Walk(int sid, int n, int step)
{
	while (n-- > 0) {
		int next = SkillCodex_Step(sid, step);

		if (next < 0)
			break;
		sid = next;
	}
	return sid;
}

/* Rows between top and sid (sid at or after top) */
static int SkillCodex_RowsBetween(int top, int sid)
{
	int n = 0;

	while (top < sid && top >= 0) {
		top = SkillCodex_Step(top, 1);
		n++;
	}
	return n;
}

/* ---- Drawing ---- */

static void SkillCodex_DrawTitle(void)
{
	const char *str = GetStringFromIndex(MSG_CODEX_LOCKED);

	if (CS->count != 0 && SkillCodex_Met(CS->cursor))
		str = GetSkillNameStr_NormalFont(CS->cursor);

	ExtrasUi_PutCentred(&CS->title, 160, TEXT_COLOR_SYSTEM_WHITE, str, gBG0TilemapBuffer + TILEMAP_INDEX(5, 1));
}

static void SkillCodex_DrawList(void)
{
	char buf[16], *p;
	int r, sid = CS->top;

	ExtrasUi_PutCentred(&CS->heading, (LIST_W - 2) * 8, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_CODEX_SKILLS),
			    gBG0TilemapBuffer + TILEMAP_INDEX(LIST_X + 1, TAB_Y));

	for (r = 0; r < VISIBLE_ROWS; r++) {
		u16 *tm = gBG0TilemapBuffer + TILEMAP_INDEX(NAME_X, ROW_Y + r * 2);

		if (CS->count == 0 || sid < 0) {
			ClearText(&CS->names[r]);
			PutText(&CS->names[r], tm);
			continue;
		}

		if (SkillCodex_Met(sid))
			ExtrasUi_PutLeft(&CS->names[r], TEXT_COLOR_SYSTEM_WHITE, GetSkillNameStr(sid), tm);
		else
			ExtrasUi_PutLeft(&CS->names[r], TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_CODEX_LOCKED), tm);

		sid = SkillCodex_Step(sid, 1);
	}

	p = ExtrasUi_PutNumber(buf, CS->met);
	*p++ = '/';
	ExtrasUi_PutNumber(p, CS->count);
	ExtrasUi_PutCentred(&CS->countText, (LIST_W - 2) * 8, TEXT_COLOR_SYSTEM_WHITE, buf,
			    gBG0TilemapBuffer + TILEMAP_INDEX(LIST_X + 1, COUNT_Y));
}

/* Descriptions start with the skill's name ("Name: what it does"): the line above says it, so it goes */
static const char *SkillCodex_DescText(int sid)
{
	const char *str = GetStringFromIndex(GetSkillDescMsg(sid));
	int i;

	for (i = 0; str[i] != '\0' && i < 40; i++)
		if (str[i] == ':')
			return str + i + 1;

	return str;
}

static void SkillCodex_DrawInfo(void)
{
	int x, y, sid = CS->cursor;

	for (y = 0; y < 2; y++)
		for (x = 0; x < 2; x++)
			gBG2TilemapBuffer[TILEMAP_INDEX(ICON_X + x, ICON_Y + y)] = 0;

	if (CS->count == 0 || !SkillCodex_Met(sid)) {
		ExtrasUi_PutLeft(&CS->name, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_CODEX_NOT_MET),
				 gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, ICON_Y));
		ExtrasUi_DrawWrapped(CS->desc, DESC_LINES, INFO_IN_W * 8, NULL,
				     gBG0TilemapBuffer + TILEMAP_INDEX(DESC_X, DESC_Y));
	} else {
		DrawIcon(gBG2TilemapBuffer + TILEMAP_INDEX(ICON_X, ICON_Y), SKILL_ICON(sid),
			 TILEREF(0, PAL_ICONS + GetSkillIconPal(sid)));
		ExtrasUi_PutLeft(&CS->name, TEXT_COLOR_SYSTEM_GOLD, GetSkillNameStr(sid),
				 gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, ICON_Y));
		ExtrasUi_DrawWrapped(CS->desc, DESC_LINES, INFO_IN_W * 8, SkillCodex_DescText(sid),
				     gBG0TilemapBuffer + TILEMAP_INDEX(DESC_X, DESC_Y));
	}

	SkillCodex_DrawTitle();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG2_SYNC_BIT);
}

static void SkillCodex_Redraw(void)
{
	SkillCodex_DrawList();
	SkillCodex_DrawInfo();
}

static void SkillCodex_PutSprites(void)
{
	if (CS->count != 0)
		DisplayUiHand(LIST_X * 8 + 16, (ROW_Y + SkillCodex_RowsBetween(CS->top, CS->cursor) * 2) * 8);
}

/* ---- The screen ---- */

static void SkillCodex_Setup(struct ProcSkillCodex *proc)
{
	int i;

	SetupBackgrounds((u16 *)sCodexBgConfig);
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_Fill(gBG3TilemapBuffer, 0);
	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;
	for (i = 0; i < 4; i++)
		BG_SetPosition(i, 0, 0);

	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	ResetIconGraphics();
	LoadIconPalettes(PAL_ICONS);

	InitText(&CS->title, 20);
	InitText(&CS->heading, LIST_W - 2);
	InitText(&CS->countText, LIST_W - 2);
	for (i = 0; i < VISIBLE_ROWS; i++)
		InitText(&CS->names[i], LIST_W - 4);
	InitText(&CS->name, INFO_IN_W - 3);
	for (i = 0; i < DESC_LINES; i++)
		InitText(&CS->desc[i], INFO_IN_W);

	ExtrasUi_LoadBanner();
	DrawUiFrame(gBG1TilemapBuffer, LIST_X, LIST_Y, LIST_W, LIST_H, 0, 0);
	DrawUiFrame(gBG1TilemapBuffer, INFO_X, INFO_Y, INFO_W, INFO_H, 0, 0);

	StartMuralBackground(proc, NULL, PAL_MURAL);

	SetDispEnable(1, 1, 1, 1, 1);
	SetDefaultColorEffects();

	SkillCodex_Redraw();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
	EnablePaletteSync();
}

static void SkillCodex_Init(struct ProcSkillCodex *proc)
{
	int sid;

	proc->dispBak = gLCDControlBuffer.dispcnt;

	CpuFill16(0, CS, sizeof(*CS));
	SkillCodex_ReadCart(&CS->bits);

	for (sid = 1; sid < SKILL_CODEX_SIDS; sid++) {
		if (!SkillCodex_Listed(sid))
			continue;

		if (CS->count++ == 0)
			CS->cursor = CS->top = sid;
		if (SkillCodex_Met(sid))
			CS->met++;
	}

	SkillCodex_Setup(proc);
}

/* The cursor to `to` (a listed skill), the top row following it */
static void SkillCodex_MoveTo(int to)
{
	if (to < 0 || to == CS->cursor)
		return;

	CS->cursor = to;
	PlaySoundEffect(SE_MOVE);

	if (CS->cursor < CS->top)
		CS->top = CS->cursor;
	if (SkillCodex_RowsBetween(CS->top, CS->cursor) > VISIBLE_ROWS - 1)
		CS->top = SkillCodex_Walk(CS->cursor, VISIBLE_ROWS - 1, -1);

	SkillCodex_Redraw();
}

/* The next met skill that way, or none */
static int SkillCodex_NextMet(int step)
{
	int sid = CS->cursor;

	for (sid = SkillCodex_Step(sid, step); sid >= 0; sid = SkillCodex_Step(sid, step)) {
		if (SkillCodex_Met(sid))
			return sid;
	}

	return -1;
}

static void SkillCodex_Loop(struct ProcSkillCodex *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;

	SkillCodex_PutSprites();

	if (press & B_BUTTON) {
		PlaySoundEffect(SE_LEAVE);
		Proc_Goto(proc, L_CODEX_EXIT);
		return;
	}

	if (CS->count == 0)
		return;

	if (repeat & (L_BUTTON | R_BUTTON)) {
		int to = SkillCodex_NextMet((repeat & R_BUTTON) ? 1 : -1);

		if (to < 0) {
			PlaySoundEffect(SE_BAD);
			return;
		}

		SkillCodex_MoveTo(to);
		return;
	}

	if (repeat & DPAD_DOWN)
		SkillCodex_MoveTo(SkillCodex_Step(CS->cursor, 1));
	else if (repeat & DPAD_UP)
		SkillCodex_MoveTo(SkillCodex_Step(CS->cursor, -1));
	else if (repeat & DPAD_RIGHT)
		SkillCodex_MoveTo(SkillCodex_Walk(CS->cursor, VISIBLE_ROWS, 1));
	else if (repeat & DPAD_LEFT)
		SkillCodex_MoveTo(SkillCodex_Walk(CS->cursor, VISIBLE_ROWS, -1));
}

/* A fade, with the hand kept on screen */
static void SkillCodex_FadeWait(struct ProcSkillCodex *proc)
{
	SkillCodex_PutSprites();

	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

static void SkillCodex_OnEnd(struct ProcSkillCodex *proc)
{
	EndMuralBackground();
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

static const struct ProcCmd ProcScr_SkillCodex[] = {
	PROC_NAME("SkillCodex"),
	PROC_SET_END_CB(SkillCodex_OnEnd),
	PROC_YIELD,
	PROC_CALL(SkillCodex_Init),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(SkillCodex_FadeWait),

	PROC_LABEL(L_CODEX_MAIN),
	PROC_REPEAT(SkillCodex_Loop),

	PROC_LABEL(L_CODEX_EXIT),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(SkillCodex_FadeWait),
	PROC_END,
};

ProcPtr StartSkillCodexScreen(ProcPtr parent)
{
	return Proc_StartBlocking(ProcScr_SkillCodex, parent);
}
