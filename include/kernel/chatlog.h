#pragma once

#include "common-chax.h"

enum {
	CHATLOG_CAP = 15,
	CHATLOG_TEXT_LEN = 72,
	CHATLOG_VISIBLE = 4,
	CHATLOG_LINES_PER_ENTRY = 3,
	CHATLOG_PANEL_W = 32,
	CHATLOG_PANEL_H = 20,
	CHATLOG_ENTRY_H = 4,
	CHATLOG_FLAG_VISIBLE = (1 << 0),
	CHATLOG_FLAG_LINEOPEN = (1 << 1),
};

struct ChatLogEntry {
	u16 faceId;
	u16 nameTextId;
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
void Chatlog_AppendNewline(void);
void Chatlog_CommitPage(void);
void Chatlog_StartSession(void);
void Chatlog_EndSession(void);
void ChapterInit_ResetChatlog(void);
void SaveChatLogSuspendState(u8 *dst, const u32 size);
void LoadChatLogSuspendState(u8 *src, const u32 size);
