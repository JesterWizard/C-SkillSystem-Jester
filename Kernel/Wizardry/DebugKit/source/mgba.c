#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "mgba.h"
#include "configs/config-debug.h"

#define REG_DEBUG_ENABLE ((vu16 *) 0x4FFF780)
#define REG_DEBUG_FLAGS ((vu16 *) 0x4FFF700)
#define REG_DEBUG_STRING ((char *) 0x4FFF600)

extern int mgba_print_level;

void mgba_print(const char *buf)
{
	#if !CONFIG_MGBA_PRINTF_EN
	(void) buf;
	return;
	#endif

	strcpy(REG_DEBUG_STRING, buf);

	mgba_print_level &= 0x7;
	*REG_DEBUG_FLAGS = mgba_print_level | 0x100;
}

void mgba_printf(int level, const char *ptr, ...)
{
	#if !CONFIG_MGBA_PRINTF_EN
	(void) level;
	(void) ptr;
	return;
	#else
	va_list args;
	char buf[0x100];

	level &= 0x7;
	va_start(args, ptr);
	vsprintf(buf, ptr, args);
	va_end(args);

	mgba_print_level = level;
	mgba_print(buf);
	#endif
}

bool mgba_open(void)
{
	#if !CONFIG_MGBA_PRINTF_EN
	return false;
	#else
	mgba_print_level = MGBA_LOG_INFO;
	*REG_DEBUG_ENABLE = 0xC0DE;
	return *REG_DEBUG_ENABLE == 0x1DEA;
	#endif
}

void mgba_close(void)
{
	#if !CONFIG_MGBA_PRINTF_EN
	return;
	#endif

	*REG_DEBUG_ENABLE = 0;
}
