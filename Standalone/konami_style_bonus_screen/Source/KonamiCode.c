#include "global.h"
#include "proc.h"
#include "hardware.h"
#include "m4a.h"
#include "soundwrapper.h"
#include "fontgrp.h"
#include "rng.h"
#include "opanim.h"
#include "gamecontrol.h"
#include "bmsave.h"
#include "bonusclaim.h"
#include "constants/items.h"
#include "gba/syscall.h"

/**
 * Port of the C Skill System "konami_style_bonus_screen" Easter egg.
 *
 * Press Up Up Down Down Left Right Left Right B A on the title screen:
 *  - a random stat booster is added to the vanilla Bonus Claim data
 *    (Extras > Bonus Claim), and
 *  - a debug-font text screen is shown until the player presses a button,
 *    which soft-resets the game.
 */

/* Shown on the bonus screen (debug font, ASCII only, 30 columns) */
#define KONAMI_BONUS_NOTE "\
      Congratulations!\n\
\n\
You found the BONUS\n\
from kernel developers!\n\
Hope you can enjoy this game!\n\
\n\
          Love\n\
          Mokha.Li\n\
\n\
Repo:\n\
https://github.com/FireEmblem\n\
Universe/fe8u-cskillsys\n\
\n\
Contact:\n\
https://github.com/MokhaLeee\n\
\n\
Now you can restart the\n\
game and continue playing!\
"

/* Name of the Bonus Claim entry (max 16 chars) */
#define KONAMI_BONUS_CONTENT_MSG "Mokha bonus"

#define TITLE_IDLE_DEMO_TIMEOUT 0x32F /* vanilla: start the class reel after this many frames */
#define SONG_TITLE_PRESS_START  0x6D

extern u8 gKonamiComboStep; /* Source/RamAlloc.s */

s8 LoadBonusContentData(void *buf); /* not declared by FE-CLib */
int sub_8003ABC(u16 held, u16 pressed);

static const u16 sKonamiCode[] = {
	DPAD_UP,
	DPAD_UP,
	DPAD_DOWN,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	DPAD_LEFT,
	DPAD_RIGHT,
	B_BUTTON,
	A_BUTTON,
};

static const u8 sKonamiBonusItems[] = {
	ITEM_BOOSTER_HP,
	ITEM_BOOSTER_POW,
	ITEM_BOOSTER_SKL,
	ITEM_BOOSTER_SPD,
	ITEM_BOOSTER_LCK,
	ITEM_BOOSTER_DEF,
	ITEM_BOOSTER_RES,
	ITEM_BOOSTER_MOV,
	ITEM_BOOSTER_CON,
};

static int KonamiStrEq(const char *a, const char *b)
{
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

static void GiveKonamiBonus(void)
{
	int i;
	int item = sKonamiBonusItems[NextRN_N(ARRAY_COUNT(sKonamiBonusItems))];
	struct BonusClaimEnt *buf = (void *)gGenericBuffer;

	if (!LoadBonusContentData(buf))
		return;

	/* Leave real (non-Konami) bonus content alone */
	for (i = 0; i < 0x10; i++) {
		if (buf[i].unseen != TRUE)
			continue;

		if (!KonamiStrEq(buf[i].str, KONAMI_BONUS_CONTENT_MSG))
			return;
	}

	for (i = 0; i < 0x10; i++) {
		const char *src = KONAMI_BONUS_CONTENT_MSG;
		char *dst = buf[i].str;

		if (buf[i].unseen == TRUE)
			continue;

		buf[i].unseen = TRUE;
		buf[i].kind = BONUSKIND_ITEM0;
		buf[i].itemId = item;

		while ((*dst++ = *src++) != '\0')
			;

		break;
	}

	SaveBonusContentData(buf);
}

static void KonamiBonusScreen_VBlank(void)
{
	INTR_CHECK = INTR_FLAG_VBLANK;

	FlushLCDControl();
	FlushBackgrounds();
	FlushTiles();
	m4aSoundVSync();
	m4aSoundMain();
}

static void __attribute__((noreturn)) StartKonamiBonusScreen(void)
{
	int i = 0;

	PrintStringToDBG(KONAMI_BONUS_NOTE);

	SetInterrupt_LCDVBlank(KonamiBonusScreen_VBlank);

	SetupDebugFontForBG(BG_2, 0);
	BG_SetPosition(BG_2, 0, 0);

	SetDispEnable(0, 0, 1, 0, 0);
	SetWinEnable(0, 0, 0);
	SetBlendNone();

	while (1) {
		UpdateKeyStatus(gKeyStatusPtr);
		sub_8003ABC(gKeyStatusPtr->heldKeys, gKeyStatusPtr->newKeys);
		VBlankIntrWait();
		SoftResetIfKeyComboPressed();

		if (++i > 128) {
			if (gKeyStatusPtr->heldKeys & (L_BUTTON | R_BUTTON | A_BUTTON | B_BUTTON))
				SoftReset(0);

			if (gKeyStatusPtr->heldKeys & (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON))
				SoftReset(0);
		}
	}
}

/**
 * Replaces vanilla Title_IDLE (0x080C6354): vanilla behaviour
 * (A/START starts the game, idle timeout starts the class reel)
 * plus the Konami code listener.
 */
void Title_IDLE_Konami(struct TitleScreenProc *proc)
{
	u16 keys = gKeyStatusPtr->newKeys;

	proc->timer_idle++;
	proc->timer = (proc->timer + 1) & 0x3F;

	if (proc->timer_idle == 1)
		gKonamiComboStep = 0;

	if (keys != 0) {
		if (keys == sKonamiCode[gKonamiComboStep]) {
			gKonamiComboStep++;

			if (gKonamiComboStep >= ARRAY_COUNT(sKonamiCode)) {
				GiveKonamiBonus();
				StartKonamiBonusScreen();
			}

			/* This key belongs to the code: don't start the game with the B/A in it */
			keys = 0;
		} else {
			gKonamiComboStep = 0;
		}
	}

	if (keys & (A_BUTTON | START_BUTTON)) {
		PlaySoundEffect(SONG_TITLE_PRESS_START);
		SetNextGameActionId(GAME_ACTION_EVENT_RETURN);
		Proc_Break(proc);
	} else if (proc->timer_idle == TITLE_IDLE_DEMO_TIMEOUT) {
		SetNextGameActionId(GAME_ACTION_CLASS_REEL);
		Proc_Break(proc);
	}
}

/**
 * Replaces vanilla LoadSavedEid8A (0x080A5290): every save slot may receive
 * Bonus Claim items (vanilla requires event flag 0x8A in that save).
 */
bool LoadSavedEid8A_Always(int slot)
{
	(void)slot;
	return TRUE;
}
