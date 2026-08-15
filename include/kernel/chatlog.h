#pragma once

#include "common-chax.h"

enum {
	CHATLOG_CAP = 15,
	CHATLOG_NAME_LEN = 16,
	CHATLOG_TEXT_LEN = 56,
	CHATLOG_VISIBLE = 4,
	CHATLOG_LINES_PER_ENTRY = 3,
	CHATLOG_PANEL_W = 32,
	CHATLOG_PANEL_H = 20,
	CHATLOG_ENTRY_H = 4,
	CHATLOG_FLAG_VISIBLE = (1 << 0),
	CHATLOG_FLAG_LINEOPEN = (1 << 1),
};

struct ChatLogEntry {
	u8 charId;
	u8 _pad;
	u16 portraitId;
	char name[CHATLOG_NAME_LEN];
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
void Chatlog_CommitPage(void);
void Chatlog_StartSession(void);
void Chatlog_EndSession(void);
void ChapterInit_ResetChatlog(void);
void SaveChatLogSuspendState(u8 *dst, const u32 size);
void LoadChatLogSuspendState(u8 *src, const u32 size);
u16 TextEngine_GetSpeakingNameTextId(void);
int TextEngine_GetSpeakingFaceId(void);
u8 TextEngine_GetSpeakingCharacterId(void);
