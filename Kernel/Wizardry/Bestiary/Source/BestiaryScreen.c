#include "common-chax.h"
#include "kernel-lib.h"
#include "strmag.h"
#include "bestiary.h"
#include "extras-ui.h"
#include "utf8.h"
#include "constants/texts.h"
#include "constants/songs.h"

/*
 * The Bestiary screen (Documentation/Features/Bestiary.md), started from Extras > Bestiary.
 *
 *  - The banner on top (the Sound Room's song banner, BG2): the name under the cursor, or "???".
 *  - On the left, in a window: the tab ("Characters" or "Classes"), five names, how many of the tab are met; the hand
 *    on the name under the cursor. Up and down move it, L and R change the tab.
 *  - On the right, in a window: the character's mini portrait (BG2) or the class's map sprite (OBJ, the map's sprite
 *    sheet); the class (or the class's movement); bases or growths (A switches); the description, wrapped.
 *  - B goes back to Extras.
 */

#define VISIBLE_ROWS   5
#define MAX_ROWS       128

/* The list's window and its rows (tiles) */
#define LIST_X         0
#define LIST_Y         4
#define LIST_W         11
#define LIST_H         16
#define TAB_Y          (LIST_Y + 1)
#define ROW_Y          (LIST_Y + 3)
#define NAME_X         (LIST_X + 3)
#define COUNT_Y        (LIST_Y + 13)

/* The info window (tiles) */
#define INFO_X         11
#define INFO_Y         4
#define INFO_W         19
#define INFO_H         16
#define IMAGE_X        (INFO_X + 1)
#define IMAGE_Y        (INFO_Y + 1)
#define SUB_X          (INFO_X + 5)
#define STATS_X        (INFO_X + 1)
#define STATS_Y        (INFO_Y + 5)
#define DESC_Y         (INFO_Y + 9)
#define DESC_LINES     3
#define INFO_IN_W      17 /* tiles inside the window */
#define NAME_PX        ((LIST_W - 4) * 8) /* a name's room in the list, right of the hand */
#define STATS_PX       (INFO_IN_W * 8)
#define DESC_PX        (INFO_IN_W * 8)
#define DESC_MARGIN    4 /* pixels free left and right of the description */

#define CHIBI_TILE     EXTRAS_UI_FREE_TILE
#define PAL_CHIBI      4
#define PAL_MURAL      14 /* and 15 */

/* The class's map sprite: in the image's place, the red army's palette, over the window */
#define SMS_X          ((IMAGE_X * 8) + 8)
#define SMS_Y          ((IMAGE_Y * 8) + 12)
#define SMS_OAM2       (0xD000 | 0x0400)

#define SE_MOVE  SONG_SE_SYS_CURSOR_UD1
#define SE_PAGE  SONG_SE_SYS_CURSOR_LR1
#define SE_LEAVE SONG_SE_SYS_WINDOW_CANSEL1

struct BestiaryScreen {
	u8 tab;
	u8 cursor;
	u8 top;	/* the first row on screen */
	u8 page;
	u8 count;	/* the tab's rows */
	u8 met;	/* of them, met */
	u8 smsClass;	/* the class whose map sprite shows, 0 for none */
	u8 smsSync;	/* the sprite sheet is to be sent to VRAM */
	struct BestiaryBits sram;	/* every save's, together */
	u8 rows[MAX_ROWS];
	u8 open[MAX_ROWS / 8];
	struct Text title;
	struct Text tabName;
	struct Text countText;
	struct Text names[VISIBLE_ROWS];
	struct Text sub;
	struct Text pageText;
	struct Text stats[2];
	struct Text desc[DESC_LINES];
};

enum {
	TAB_CHARACTERS,
	TAB_CLASSES,
};

enum {
	PAGE_BASES,
	PAGE_GROWTHS,
};

extern struct BestiaryScreen gBestiaryScreen;
#define BS (&gBestiaryScreen)

_Static_assert(sizeof(struct BestiaryScreen) <= 0x160, "gBestiaryScreen is 0x160 bytes (config-memmap.s)");

struct ProcBestiary {
	PROC_HEADER;
	struct DispCnt dispBak;
};

enum {
	L_BESTIARY_MAIN = 1,
	L_BESTIARY_EXIT,
};

/* BG0 text, BG1 the windows, BG2 the banner and the portrait, BG3 the mural */
static const u16 sBestiaryBgConfig[12] = {
	0x0000, 0x6000, 0,
	0x0000, 0x6800, 0,
	0x0000, 0x7000, 0,
	0x8000, 0x7800, 0,
};

static const u16 sStatMsgs[8] = {
	MSG_BESTIARY_STAT_HP, MSG_BESTIARY_STAT_STR, MSG_BESTIARY_STAT_MAG, MSG_BESTIARY_STAT_SKL,
	MSG_BESTIARY_STAT_SPD, MSG_BESTIARY_STAT_LCK, MSG_BESTIARY_STAT_DEF, MSG_BESTIARY_STAT_RES,
};

/* ---- The rows ---- */

static const u8 *Bestiary_TabList(int tab)
{
	return tab == TAB_CHARACTERS ? gBestiaryCharacters : gBestiaryClasses;
}

static int Bestiary_NameOf(int tab, int id)
{
	return tab == TAB_CHARACTERS ? GetCharacterData(id)->nameTextId : GetClassData(id)->nameTextId;
}

/* A row is shown if it has a name (and, for a character, a portrait) no earlier row has */
static bool Bestiary_RowShown(int tab, const u8 *list, int i)
{
	int j, name = Bestiary_NameOf(tab, list[i]);

	if (name == 0)
		return false;

	if (tab == TAB_CHARACTERS && GetCharacterData(list[i])->portraitId == 0)
		return false;

	for (j = 0; j < i; j++)
		if (Bestiary_NameOf(tab, list[j]) == name)
			return false;

	return true;
}

/* Met: any character (or class) of that name has been seen */
static bool Bestiary_RowMet(int tab, int id)
{
	const u8 *bits = tab == TAB_CHARACTERS ? BS->sram.chars : BS->sram.classes;
	int b, name = Bestiary_NameOf(tab, id);

	for (b = 1; b < 0x100; b++)
		if (Bestiary_BitSet(bits, b) && Bestiary_NameOf(tab, b) == name)
			return true;

	return false;
}

static bool Bestiary_Open(int row)
{
	return (BS->open[row >> 3] >> (row & 7)) & 1;
}

static void Bestiary_BuildRows(void)
{
	const u8 *list = Bestiary_TabList(BS->tab);
	int i;

	BS->count = 0;
	BS->met = 0;
	CpuFill16(0, BS->open, sizeof(BS->open));

	for (i = 0; list[i] != 0 && BS->count < MAX_ROWS; i++) {
		if (!Bestiary_RowShown(BS->tab, list, i))
			continue;

		if (Bestiary_RowMet(BS->tab, list[i])) {
			BS->open[BS->count >> 3] |= 1 << (BS->count & 7);
			BS->met++;
		}
		BS->rows[BS->count++] = list[i];
	}
}

/* ---- The banner, the list ---- */

static int Bestiary_CursorName(void)
{
	return Bestiary_NameOf(BS->tab, BS->rows[BS->cursor]);
}

static void Bestiary_DrawTitle(void)
{
	const char *str = GetStringFromIndex(MSG_BESTIARY_LOCKED);

	if (BS->count != 0 && Bestiary_Open(BS->cursor))
		str = GetStringFromIndex(Bestiary_CursorName());

	/* 20 tiles from tile 5: centred on the banner (and the screen) */
	ExtrasUi_PutCentred(&BS->title, 160, TEXT_COLOR_SYSTEM_WHITE, str, gBG0TilemapBuffer + TILEMAP_INDEX(5, 1));
}

/* A name for the list: in the narrow font when the normal one is wider than its room */
static const char *Bestiary_ListName(int msg)
{
	char *str = GetStringFromIndex(msg);

	if (GetStringTextLen(str) <= NAME_PX)
		return str;

	return Utf8ToNarrowFonts(str);
}

static void Bestiary_DrawList(void)
{
	char buf[16], *p;
	int r;

	/* The tab's name over the names, flush with them */
	ExtrasUi_PutLeft(&BS->tabName, TEXT_COLOR_SYSTEM_GOLD,
			 GetStringFromIndex(BS->tab == TAB_CHARACTERS ? MSG_BESTIARY_CHARACTERS : MSG_BESTIARY_CLASSES),
			 gBG0TilemapBuffer + TILEMAP_INDEX(NAME_X, TAB_Y));

	for (r = 0; r < VISIBLE_ROWS; r++) {
		int row = BS->top + r;
		u16 *tm = gBG0TilemapBuffer + TILEMAP_INDEX(NAME_X, ROW_Y + r * 2);

		if (row >= BS->count) {
			ClearText(&BS->names[r]);
			PutText(&BS->names[r], tm);
			continue;
		}

		if (Bestiary_Open(row))
			ExtrasUi_PutLeft(&BS->names[r], TEXT_COLOR_SYSTEM_WHITE,
					 Bestiary_ListName(Bestiary_NameOf(BS->tab, BS->rows[row])), tm);
		else
			ExtrasUi_PutLeft(&BS->names[r], TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_BESTIARY_LOCKED), tm);
	}

	p = ExtrasUi_PutNumber(buf, BS->met);
	*p++ = '/';
	ExtrasUi_PutNumber(p, BS->count);
	ExtrasUi_PutCentred(&BS->countText, (LIST_W - 2) * 8, TEXT_COLOR_SYSTEM_WHITE, buf,
			    gBG0TilemapBuffer + TILEMAP_INDEX(LIST_X + 1, COUNT_Y));
}

/* ---- The info window ---- */

static void Bestiary_ClearImage(void)
{
	int x, y;

	for (y = 0; y < 4; y++)
		for (x = 0; x < 4; x++)
			gBG2TilemapBuffer[TILEMAP_INDEX(IMAGE_X + x, IMAGE_Y + y)] = 0;
}

/*
 * A table across the window: the stats' names in gold, spaced by their own widths with the same gap between each
 * two (the names differ: "Mag" is half as wide again as "HP", and equal columns put it against "Skl"); each value
 * under its name in blue, centred on it.
 */
static void Bestiary_DrawStats(const int *vals)
{
	int c, w[8], sum = 0, gap, x;

	ClearText(&BS->stats[0]);
	ClearText(&BS->stats[1]);

	for (c = 0; c < 8; c++) {
		/* GetStringFromIndex's buffer is shared: each name is read again to draw it */
		w[c] = GetStringTextLen(GetStringFromIndex(sStatMsgs[c]));
		sum += w[c];
	}

	gap = sum < STATS_PX ? Div(STATS_PX - sum, 8) : 0;
	x = gap / 2;

	for (c = 0; c < 8; c++) {
		char buf[8];
		int vw;

		Text_InsertDrawString(&BS->stats[0], x, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(sStatMsgs[c]));

		ExtrasUi_PutNumber(buf, vals[c]);
		vw = GetStringTextLen(buf);
		Text_InsertDrawString(&BS->stats[1], x + (w[c] - vw) / 2, TEXT_COLOR_SYSTEM_BLUE, buf);

		x += w[c] + gap;
	}

	PutText(&BS->stats[0], gBG0TilemapBuffer + TILEMAP_INDEX(STATS_X, STATS_Y));
	PutText(&BS->stats[1], gBG0TilemapBuffer + TILEMAP_INDEX(STATS_X, STATS_Y + 2));
}

static void Bestiary_ClearStats(void)
{
	int r;

	for (r = 0; r < 2; r++) {
		ClearText(&BS->stats[r]);
		PutText(&BS->stats[r], gBG0TilemapBuffer + TILEMAP_INDEX(STATS_X, STATS_Y + r * 2));
	}
}

static void Bestiary_DrawDesc(int msg)
{
	ExtrasUi_DrawWrapped(BS->desc, DESC_LINES, DESC_PX, DESC_MARGIN, msg != 0 ? GetStringFromIndex(msg) : NULL,
			     gBG0TilemapBuffer + TILEMAP_INDEX(STATS_X, DESC_Y));
}

static void Bestiary_DrawCharacter(int pid)
{
	const struct CharacterData *cd = GetCharacterData(pid);
	const struct ClassData *jd = GetClassData(cd->defaultClass);
	char buf[24], *p;
	int vals[8];
	int fid = cd->miniPortrait ? 0x7F00 + cd->miniPortrait : cd->portraitId;

	PutFaceChibi(fid, gBG2TilemapBuffer + TILEMAP_INDEX(IMAGE_X, IMAGE_Y), CHIBI_TILE, PAL_CHIBI, 0);

	ExtrasUi_PutLeft(&BS->sub, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(jd->nameTextId),
			 gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y));

	if (BS->page == PAGE_BASES) {
		vals[0] = cd->baseHP + jd->baseHP;
		vals[1] = cd->basePow + jd->basePow;
		vals[2] = gpMagicPInfos[pid].base + gpMagicJInfos[jd->number].base;
		vals[3] = cd->baseSkl + jd->baseSkl;
		vals[4] = cd->baseSpd + jd->baseSpd;
		vals[5] = cd->baseLck;
		vals[6] = cd->baseDef + jd->baseDef;
		vals[7] = cd->baseRes + jd->baseRes;
	} else {
		vals[0] = cd->growthHP;
		vals[1] = cd->growthPow;
		vals[2] = gpMagicPInfos[pid].growth;
		vals[3] = cd->growthSkl;
		vals[4] = cd->growthSpd;
		vals[5] = cd->growthLck;
		vals[6] = cd->growthDef;
		vals[7] = cd->growthRes;
	}

	/* "Bases  Lv 4" or "Growths" */
	p = ExtrasUi_Append(buf, GetStringFromIndex(BS->page == PAGE_BASES ? MSG_BESTIARY_BASES : MSG_BESTIARY_GROWTHS));
	ClearText(&BS->pageText);
	Text_InsertDrawString(&BS->pageText, 0, TEXT_COLOR_SYSTEM_GOLD, buf);
	if (BS->page == PAGE_BASES) {
		p = ExtrasUi_Append(buf, GetStringFromIndex(MSG_BESTIARY_LEVEL));
		*p++ = ' ';
		ExtrasUi_PutNumber(p, cd->baseLevel);
		Text_InsertDrawString(&BS->pageText, 56, TEXT_COLOR_SYSTEM_WHITE, buf);
	}
	PutText(&BS->pageText, gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y + 2));

	Bestiary_DrawStats(vals);
	Bestiary_DrawDesc(cd->descTextId);
}

static void Bestiary_DrawClass(int jid)
{
	const struct ClassData *jd = GetClassData(jid);
	char buf[24], *p;
	int vals[8];

	p = ExtrasUi_Append(buf, GetStringFromIndex(MSG_BESTIARY_MOVE));
	*p++ = ' ';
	ExtrasUi_PutNumber(p, jd->baseMov);
	ExtrasUi_PutLeft(&BS->sub, TEXT_COLOR_SYSTEM_WHITE, buf, gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y));

	if (BS->page == PAGE_BASES) {
		vals[0] = jd->baseHP;
		vals[1] = jd->basePow;
		vals[2] = gpMagicJInfos[jid].base;
		vals[3] = jd->baseSkl;
		vals[4] = jd->baseSpd;
		vals[5] = 0;
		vals[6] = jd->baseDef;
		vals[7] = jd->baseRes;
	} else {
		vals[0] = jd->growthHP;
		vals[1] = jd->growthPow;
		vals[2] = gpMagicJInfos[jid].growth;
		vals[3] = jd->growthSkl;
		vals[4] = jd->growthSpd;
		vals[5] = jd->growthLck;
		vals[6] = jd->growthDef;
		vals[7] = jd->growthRes;
	}

	ExtrasUi_PutLeft(&BS->pageText, TEXT_COLOR_SYSTEM_GOLD,
			 GetStringFromIndex(BS->page == PAGE_BASES ? MSG_BESTIARY_BASES : MSG_BESTIARY_GROWTHS),
			 gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y + 2));

	Bestiary_DrawStats(vals);
	Bestiary_DrawDesc(jd->descTextId);

	/* The map sprite: a new class's sheet goes to VRAM on the next frame */
	if (BS->smsClass != jid) {
		ResetUnitSprites();
		BS->smsClass = jid;
		BS->smsSync = true;
	}
}

static void Bestiary_DrawInfo(void)
{
	Bestiary_ClearImage();
	BS->smsClass = 0;

	if (BS->count == 0 || !Bestiary_Open(BS->cursor)) {
		ExtrasUi_PutLeft(&BS->sub, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(MSG_BESTIARY_NOT_MET),
				 gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y));
		ClearText(&BS->pageText);
		PutText(&BS->pageText, gBG0TilemapBuffer + TILEMAP_INDEX(SUB_X, IMAGE_Y + 2));
		Bestiary_ClearStats();
		Bestiary_DrawDesc(0);
	} else if (BS->tab == TAB_CHARACTERS) {
		Bestiary_DrawCharacter(BS->rows[BS->cursor]);
	} else {
		Bestiary_DrawClass(BS->rows[BS->cursor]);
	}

	Bestiary_DrawTitle();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG2_SYNC_BIT);
	EnablePaletteSync();
}

static void Bestiary_Redraw(void)
{
	Bestiary_DrawList();
	Bestiary_DrawInfo();
}

/* The hand on the cursor's row, the class's map sprite */
static void Bestiary_PutSprites(void)
{
	if (BS->count != 0)
		DisplayUiHand(LIST_X * 8 + 16, (ROW_Y + (BS->cursor - BS->top) * 2) * 8);

	if (BS->smsClass == 0)
		return;

	PutUnitSpriteForClassId(0, SMS_X, SMS_Y, SMS_OAM2, BS->smsClass);

	if (BS->smsSync) {
		ForceSyncUnitSpriteSheet();
		BS->smsSync = false;
	} else {
		SyncUnitSpriteSheet();
	}
}

/* ---- The screen ---- */

static void Bestiary_Setup(struct ProcBestiary *proc)
{
	int i;

	SetupBackgrounds((u16 *)sBestiaryBgConfig);
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

	ResetUnitSprites();
	ApplyUnitSpritePalettes();

	InitText(&BS->title, 20);
	InitText(&BS->tabName, LIST_W - 4);
	InitText(&BS->countText, LIST_W - 2);
	for (i = 0; i < VISIBLE_ROWS; i++)
		InitText(&BS->names[i], LIST_W - 4);
	InitText(&BS->sub, INFO_IN_W - 4);
	InitText(&BS->pageText, INFO_IN_W - 4);
	for (i = 0; i < 2; i++)
		InitText(&BS->stats[i], INFO_IN_W);
	for (i = 0; i < DESC_LINES; i++)
		InitText(&BS->desc[i], INFO_IN_W);

	ExtrasUi_LoadBanner();
	DrawUiFrame(gBG1TilemapBuffer, LIST_X, LIST_Y, LIST_W, LIST_H, 0, 0);
	DrawUiFrame(gBG1TilemapBuffer, INFO_X, INFO_Y, INFO_W, INFO_H, 0, 0);

	StartMuralBackground(proc, NULL, PAL_MURAL);

	gLCDControlBuffer.dispcnt.obj1dMap = 0;
	SetDispEnable(1, 1, 1, 1, 1);
	SetDefaultColorEffects();

	Bestiary_Redraw();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
}

static void Bestiary_Init(struct ProcBestiary *proc)
{
	proc->dispBak = gLCDControlBuffer.dispcnt;

	CpuFill16(0, BS, sizeof(*BS));
	Bestiary_ReadCart(&BS->sram);
	BS->tab = TAB_CHARACTERS;
	Bestiary_BuildRows();

	Bestiary_Setup(proc);
}

static void Bestiary_Move(int to)
{
	if (BS->count == 0)
		return;
	if (to < 0)
		to = 0;
	if (to >= BS->count)
		to = BS->count - 1;
	if (to == BS->cursor)
		return;

	BS->cursor = to;
	PlaySoundEffect(SE_MOVE);

	if (BS->cursor < BS->top)
		BS->top = BS->cursor;
	if (BS->cursor > BS->top + VISIBLE_ROWS - 1)
		BS->top = BS->cursor - (VISIBLE_ROWS - 1);

	Bestiary_Redraw();
}

static void Bestiary_SetTab(int tab)
{
	PlaySoundEffect(SE_PAGE);
	BS->tab = tab;
	BS->cursor = 0;
	BS->top = 0;
	Bestiary_BuildRows();
	Bestiary_Redraw();
}

static void Bestiary_Loop(struct ProcBestiary *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;

	Bestiary_PutSprites();

	if (press & B_BUTTON) {
		PlaySoundEffect(SE_LEAVE);
		Proc_Goto(proc, L_BESTIARY_EXIT);
		return;
	}

	if (press & A_BUTTON) {
		if (BS->count != 0 && Bestiary_Open(BS->cursor)) {
			PlaySoundEffect(SE_PAGE);
			BS->page ^= 1;
			Bestiary_DrawInfo();
		}
		return;
	}

	if (press & (L_BUTTON | R_BUTTON)) {
		Bestiary_SetTab(BS->tab ^ 1);
		return;
	}

	if (repeat & DPAD_DOWN)
		Bestiary_Move(BS->cursor + 1);
	else if (repeat & DPAD_UP)
		Bestiary_Move(BS->cursor - 1);
	else if (repeat & DPAD_RIGHT)
		Bestiary_Move(BS->cursor + VISIBLE_ROWS);
	else if (repeat & DPAD_LEFT)
		Bestiary_Move(BS->cursor - VISIBLE_ROWS);
}

/* A fade, with the sprites kept on screen */
static void Bestiary_FadeWait(struct ProcBestiary *proc)
{
	Bestiary_PutSprites();

	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

static void Bestiary_OnEnd(struct ProcBestiary *proc)
{
	EndMuralBackground();
	gLCDControlBuffer.bg3cnt.colorMode = 0;
	gLCDControlBuffer.dispcnt = proc->dispBak;
	SetDefaultColorEffects();
}

static const struct ProcCmd ProcScr_Bestiary[] = {
	PROC_NAME("Bestiary"),
	PROC_SET_END_CB(Bestiary_OnEnd),
	PROC_YIELD,
	PROC_CALL(Bestiary_Init),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Bestiary_FadeWait),

	PROC_LABEL(L_BESTIARY_MAIN),
	PROC_REPEAT(Bestiary_Loop),

	PROC_LABEL(L_BESTIARY_EXIT),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Bestiary_FadeWait),
	PROC_END,
};

ProcPtr StartBestiaryScreen(ProcPtr parent)
{
	return Proc_StartBlocking(ProcScr_Bestiary, parent);
}
