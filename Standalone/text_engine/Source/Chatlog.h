#pragma once

/*
 * Conversation log (the C Skill System's chatlog_enabled): SELECT during
 * dialogue opens a scrollback log of the chapter's dialogue.
 *
 * Built into Source/Chatlog.lyn.event and installed only when Installer.event
 * defines TEXT_ENGINE_CHATLOG; otherwise Source/ChatlogStubs.lyn.event
 * provides no-op versions of the calls the text engine makes.
 */

#include "gbafe.h"

enum {
	CHATLOG_CAP = 17,
	CHATLOG_TEXT_LEN = 56,
	/* Max lines drawn at once: three speakers with a wrapped second line. */
	CHATLOG_VISIBLE = 6,
	/* Tile rows for a speaker's nameplate + first message line. */
	CHATLOG_ENTRY_H = 4,
	/* Tile rows for one 16px text line (a wrap continuation). */
	CHATLOG_LINE_H = 2,
	CHATLOG_FLAG_VISIBLE = (1 << 0),
	CHATLOG_FLAG_LINEOPEN = (1 << 1),
	/* Entry flags */
	CHATLOG_ENTRY_CONT = (1 << 0),
};

struct ChatLogEntry {
	u8 charId;
	u8 flags;
	u16 portraitId;
	u16 nameTextId;
	/* Pixels already drawn into text, used to wrap at capture time. */
	u8 width;
	u8 _pad;
	char text[CHATLOG_TEXT_LEN];
};

struct ChatLogState {
	u8 count;
	u8 head;
	u8 viewTop;
	u8 flags;
	struct ChatLogEntry entries[CHATLOG_CAP];
};

/*
 * Which chapter the log belongs to.  The C Skill System clears the log from a
 * chapter-start hook and keeps it in the suspend save; here the log stamps
 * itself with the chapter's start time (gPlaySt.time_chapter_started, set
 * only when a chapter starts or restarts) and clears itself when that
 * changes, so no chapter-start hook or save change is needed.
 */
struct ChatLogStamp {
	u32 magic;
	u32 chapterStartTime;
};

bool Chatlog_IsVisible(void);
bool Chatlog_IsSessionActive(void);
void Chatlog_AppendPrinted(const char *str);
void Chatlog_AppendSoftBreak(void);
void Chatlog_CommitPage(void);
void Chatlog_StartSession(void);
void Chatlog_StartSessionUnder(ProcPtr parent);
void Chatlog_EndSession(void);

/* Provided by the text engine. */
u16 TextEngine_GetSpeakingNameTextId(void);
int TextEngine_GetSpeakingFaceId(void);
u8 TextEngine_GetSpeakingCharacterId(void);
