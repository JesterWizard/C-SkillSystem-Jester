#pragma once

#include "common-chax.h"

/*
 * "Previously on your quest..." (Kernel/Wizardry/Recap): the last story chapters' key scenes, up to four, shown in
 * greyscale when a game save or a suspend is loaded. See Documentation/Features/Recap.md.
 */

#define RECAP_MAGIC  0x32504352 /* "RCP2" (0x18 bytes; "RCAP" was the 0x10-byte first form) */
#define RECAP_NONE   0xFF       /* no background */
#define RECAP_JOIN_BG 0x18 /* behind a recruit: the grassland Chapter 3 opens with (not the CG of the moment) */
#define RECAP_UNITS  2          /* characters who died or joined */
#define RECAP_SCENES (2 + RECAP_UNITS)
#define RECAP_FACES  2          /* portraits shown with the opening and with the ending */

/* RecapData.flags */
#define RECAP_OPENING_OVER (1 << 0) /* the chapter's first phase has started: its opening is over */

enum recap_kind {
	RECAP_OPENING = 1,
	RECAP_DIED,
	RECAP_JOINED,
	RECAP_ENDING,
};

/* In RAM and in the tail of every game save and of the suspend save (SaveTail.c): 0x18 bytes */
struct RecapData {
	/* 00 */ u32 magic;
	/* 04 */ u8 chapter;                 /* the story chapter recorded */
	/* 05 */ u8 openBg;                  /* its first background, or RECAP_NONE */
	/* 06 */ u8 endBg;                   /* the last background once it was won, or RECAP_NONE */
	/* 07 */ u8 unitCount;
	/* 08 */ u8 unitKind[RECAP_UNITS];   /* RECAP_DIED or RECAP_JOINED */
	/* 0A */ u8 unitPid[RECAP_UNITS];
	/* 0C */ u8 unitBg[RECAP_UNITS];     /* the background shown last when it happened, or RECAP_NONE */
	/* 0E */ u8 lastBg;
	/* 0F */ u8 flags;                   /* RECAP_OPENING_OVER */
	/* 10 */ u16 openFace[RECAP_FACES];  /* the first portraits to speak in the opening, or 0 */
	/* 14 */ u16 endFace[RECAP_FACES];   /* the first portraits to speak once it was won, or 0 */
};

struct RecapState {
	/* 00 */ struct RecapData data;
	/* 18 */ u32 army[2];                /* bit pid: the character was in the army (pids 1-0x3F) */
	/* 20 */ u8 pending;                 /* the save just loaded has a recap to show */
	/* 21 */ u8 pad[3];
};

extern struct RecapState gRecap;

#define RECAP_HISTORY       1          /* earlier story chapters kept to fill the recap: the one before */
#define RECAP_HISTORY_MAGIC 0x31484352 /* "RCH1" */

/* The record of the story chapter before gRecap's (chapter[0]). In RAM and in the tail of every game save and of the
 * suspend save (SaveTail.c), after the chapter intro's share: 0x1C bytes */
struct RecapHistory {
	/* 00 */ u32 magic;
	/* 04 */ struct RecapData chapter[RECAP_HISTORY]; /* magic 0: none */
};

extern struct RecapHistory gRecapHistory;

/* The story summaries: { chapter, opening message, ending message }, ends with chapter 0xFF (Kernel/Wizardry/Recap) */
struct RecapChapterText {
	u8 chapter;
	u16 open;
	u16 end;
};

extern const struct RecapChapterText gRecapChapterTexts[];

void Recap_NoteBackground(int bg);
void Recap_OnUnitKill(struct Unit *unit);
void Recap_NoteFace(int fid);
bool Recap_PrePhase(ProcPtr proc);
void Recap_SaveTail(u8 *dst, const u32 size);
void Recap_LoadTail(u8 *src, const u32 size);
void Recap_SaveHistoryTail(u8 *dst, const u32 size);
void Recap_LoadHistoryTail(u8 *src, const u32 size);
void Recap_PostSaveMenu(ProcPtr proc);
