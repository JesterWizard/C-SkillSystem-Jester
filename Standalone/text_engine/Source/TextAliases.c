#include "TextEngine.h"

/*
 * Lets a message spell the text engine's codes as their textdefs.txt names,
 * so "[ToggleOnBigText]HALT![ToggleOffBigText]" works when it is typed into
 * FEBuilder's text editor, which stores unknown names as plain text.
 * "[Name 60]" or "[Name 0x3C]" adds argument bytes after the code.
 */

struct TextAlias {
	const char *name;
	u8 code[4];
};

#include "TextAliasTable.h"

#define TEXT_ALIAS_MAX_ARGS 4

/* Returns the length of name if text starts with it, followed by ']' or ' '. */
static int TextAlias_MatchName(const char *text, const char *name)
{
	int i;

	for (i = 0; name[i]; i++)
		if (text[i] != name[i])
			return 0;

	return (text[i] == ']' || text[i] == ' ') ? i : 0;
}

/* Parses a decimal or 0x-prefixed number from 1 to 255, or returns 0. */
static int TextAlias_ParseNumber(const char **cursor)
{
	const char *p = *cursor;
	int base = 10;
	int value = 0;
	int digits = 0;

	if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
		base = 16;
		p += 2;
	}

	for (;; p++, digits++) {
		int digit;

		if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else if (base == 16 && *p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if (base == 16 && *p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else
			break;

		value = value * base + digit;
		if (value > 0xFF)
			return 0;
	}

	if (digits == 0)
		return 0;

	*cursor = p;
	return value;
}

/*
 * If text starts with a known "[Name ...]", writes its bytes to out and
 * returns how many characters of text it used.  Otherwise returns 0.
 */
static int TextAlias_Expand(const char *text, u8 *out, int *outLength)
{
	const struct TextAlias *alias;
	const char *p;
	int length = 0;
	int args = 0;
	int i;

	for (alias = sTextAliases; alias->name; alias++) {
		int nameLength = TextAlias_MatchName(text + 1, alias->name);

		if (nameLength) {
			p = text + 1 + nameLength;
			break;
		}
	}

	if (!alias->name)
		return 0;

	for (i = 0; i < (int)sizeof(alias->code) && alias->code[i]; i++)
		out[length++] = alias->code[i];

	while (*p == ' ') {
		int value;

		p++;
		value = TextAlias_ParseNumber(&p);
		if (!value || ++args > TEXT_ALIAS_MAX_ARGS)
			return 0;

		out[length++] = value;
	}

	if (*p != ']')
		return 0;

	*outLength = length;
	return p + 1 - text;
}

/*
 * Rewrites every known "[Name ...]" in a RAM message into its code bytes, in
 * place.  The bytes are never longer than the name, and text that isn't a
 * known name is left as it is.
 */
void TextEngine_ExpandAliases(char *str)
{
	u8 *src = (u8 *)str;
	u8 *dst = (u8 *)str;
	u8 bytes[sizeof(((struct TextAlias *)0)->code) + TEXT_ALIAS_MAX_ARGS];

	if ((u32)str < 0x02000000 || (u32)str >= 0x04000000)
		return;

	while (*src) {
		int length;
		int used;
		int i;

		/* Copy codes whole, so an argument byte is never read as '['. */
		if (*src == 0x80 && src[1]) {
			*dst++ = *src++;
			*dst++ = *src++;
			continue;
		}

		if (*src == 0x10 && src[1] && src[2]) {
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
			continue;
		}

		if (*src == '[') {
			used = TextAlias_Expand((const char *)src, bytes, &length);
			if (used) {
				for (i = 0; i < length; i++)
					*dst++ = bytes[i];

				src += used;
				continue;
			}
		}

		*dst++ = *src++;
	}

	*dst = 0;
}
