#include "Chatlog.h"

/*
 * No-op conversation log, installed when Installer.event leaves
 * TEXT_ENGINE_CHATLOG undefined.  The text engine calls these at the same
 * places it calls the real log in Source/Chatlog.c.
 */

bool Chatlog_IsVisible(void)
{
	return false;
}

void Chatlog_AppendPrinted(const char *str)
{
	(void)str;
}

void Chatlog_AppendSoftBreak(void)
{
}

void Chatlog_CommitPage(void)
{
}

void Chatlog_StartSession(void)
{
}

void Chatlog_EndSession(void)
{
}
