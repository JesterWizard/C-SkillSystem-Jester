#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "prepscreen.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

#define PL_HEX_EXIT 2

struct ProcHexScreen {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct Text text[3];
	/* 44 */ u8 gaveStarters;
};

/* "Cards: N" with N in decimal (N >= 0). */
static void HexScreen_CardsLine(char *buf, int n)
{
	static const char prefix[] = "Cards: ";
	char digits[12];
	int len = 0;
	int i;

	for (i = 0; prefix[i] != 0; i++)
		buf[i] = prefix[i];

	do {
		digits[len++] = (char)('0' + (n - (n / 10) * 10));
		n = n / 10;
	} while (n > 0 && len < (int)sizeof(digits));

	while (len > 0)
		buf[i++] = digits[--len];

	buf[i] = 0;
}

static void HexScreen_DrawLine(struct Text *th, int x, int y, int colorId, const char *str)
{
	ClearText(th);
	Text_SetParams(th, 0, colorId);
	Text_DrawString(th, str);
	PutText(th, TILEMAP_LOCATED(gBG0TilemapBuffer, x, y));
}

static void HexScreen_Draw(struct ProcHexScreen *proc)
{
	char buf[32];

	/* First open with no cards: hand out the starter set. */
	if (HexCol_Total() == 0) {
		HexCol_GiveStarters();
		proc->gaveStarters = 1;
	}

	HexScreen_DrawLine(&proc->text[0], 2, 2, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_PREP_HEX));

	HexScreen_CardsLine(buf, HexCol_Total());
	HexScreen_DrawLine(&proc->text[1], 2, 5, TEXT_COLOR_SYSTEM_WHITE, buf);

	if (proc->gaveStarters)
		HexScreen_DrawLine(&proc->text[2], 2, 8, TEXT_COLOR_SYSTEM_GREEN, "Starter cards added");

	BG_EnableSyncByMask(0x1);
}

static void HexScreen_Init(struct ProcHexScreen *proc)
{
	SetupBackgrounds(NULL);
	ResetText();
	SetTextFont(0);
	InitSystemTextFont();
	LoadUiFrameGraphics();

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);

	InitText(&proc->text[0], 12);
	InitText(&proc->text[1], 12);
	InitText(&proc->text[2], 20);

	proc->gaveStarters = 0;
	HexScreen_Draw(proc);
	SetDefaultColorEffects();
}

static void HexScreen_Loop(struct ProcHexScreen *proc)
{
	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		Proc_Goto(proc, PL_HEX_EXIT);
	}
}

static void HexScreen_OnEnd(struct ProcHexScreen *proc)
{
	struct ProcAtMenu *pproc = proc->proc_parent;

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_EnableSyncByMask(0x1);

	/* Same hand-off as the other prep sub-screens: the prep menu redraws itself. cur_cmd puts the cursor back on the Hex row. */
	pproc->cur_cmd = PREP_MAINMENU_HEX;
	pproc->state = 1;
}

const struct ProcCmd ProcScr_HexScreen[] = {
	PROC_NAME("HexScreen"),
	PROC_YIELD,
	PROC_SET_END_CB(HexScreen_OnEnd),

	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(HexScreen_Init),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_REPEAT(HexScreen_Loop),

	PROC_LABEL(PL_HEX_EXIT),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_END,
};

#endif /* CONFIG_HEX_CARDS */
