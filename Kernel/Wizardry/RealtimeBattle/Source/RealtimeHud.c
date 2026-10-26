#include "common-chax.h"
#include "constants/texts.h"
#include "kernel/realtime-battle.h"

/* Seconds left in the turn at which the clock turns gold */
#define REALTIME_HUD_WARN_SECONDS 10

/* Second line of the goal window: "Turn 3   0:24" */
void RealtimeBattle_DrawGoalClock(struct Text *text)
{
	/*
	 * Split by subtraction: pairing "/" with "%" (or the multiply-subtract
	 * equivalent) makes GCC emit __aeabi_idivmod, which this toolchain links
	 * to a helper that returns the remainder in the wrong register.
	 */
	int seconds = RealtimeBattle_GetTurnSecondsLeft();
	int minutes = 0;
	int tens = 0;
	int color;
	int turnX;

	gRealtimeBattleState.hudSeconds = seconds;

	while (seconds >= 60 && minutes < 9) {
		seconds -= 60;
		minutes++;
	}

	if (seconds >= 60)
		seconds = 59;

	while (seconds >= 10) {
		seconds -= 10;
		tens++;
	}

	color = (minutes == 0 && tens * 10 + seconds <= REALTIME_HUD_WARN_SECONDS)
		? TEXT_COLOR_SYSTEM_GOLD
		: TEXT_COLOR_SYSTEM_BLUE;

	/* Numbers are right-aligned on x, so push the turn right as it grows */
	if (gPlaySt.chapterTurnNumber >= 100)
		turnX = 40;
	else if (gPlaySt.chapterTurnNumber >= 10)
		turnX = 32;
	else
		turnX = 24;

	Text_InsertDrawString(text, 2, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_01C2)); // "Turn"
	Text_InsertDrawNumberOrBlank(text, turnX, TEXT_COLOR_SYSTEM_BLUE, gPlaySt.chapterTurnNumber);

	Text_InsertDrawNumberOrBlank(text, 44, color, minutes);
	Text_InsertDrawString(text, 52, TEXT_COLOR_SYSTEM_WHITE, ":");
	Text_InsertDrawNumberOrBlank(text, 56, color, tens);
	Text_InsertDrawNumberOrBlank(text, 64, color, seconds);
}

bool RealtimeBattle_GoalClockChanged(void)
{
	return gRealtimeBattleState.hudSeconds != RealtimeBattle_GetTurnSecondsLeft();
}
