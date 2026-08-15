#pragma once

#include "common-chax.h"

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

bool Chatlog_IsVisible(void);
void Chatlog_AppendPrinted(const char *str);
void Chatlog_AppendPrintedSpan(const char *start, const char *end);
void Chatlog_AppendUnicode(u32 unicod);
void Chatlog_BeginGlyphCapture(void);
void Chatlog_EndGlyphCapture(void);
void Chatlog_AppendNewline(void);
void Chatlog_AppendSoftBreak(void);
void Chatlog_CommitPage(void);
void Chatlog_StartSession(void);
void Chatlog_EndSession(void);
void ChapterInit_ResetChatlog(void);
void SaveChatLogSuspendState(u8 *dst, const u32 size);
void LoadChatLogSuspendState(u8 *src, const u32 size);
u16 TextEngine_GetSpeakingNameTextId(void);
int TextEngine_GetSpeakingFaceId(void);
u8 TextEngine_GetSpeakingCharacterId(void);
