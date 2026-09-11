#include <common-chax.h>
#include <kernel-lib.h>
#include <utf8.h>
#include <kernel/chatlog.h>

FORCE_DECLARE STATIC_DECLAR bool is_ascii(u32 unicod)
{
	return unicod < 0x80;
}

STATIC_DECLAR struct Glyph *GetCharGlyphUnicode(u32 unicode_ch, struct Font *font)
{
	struct Glyph *glyph;
	int hi = (unicode_ch >> 0x8) & 0xFF;
	int lo = unicode_ch & 0xFF;

	/* For now, we can only support for group 1 of unicode (U_0000 ~ U_FFFF) */
	if (unicode_ch >= 0x10000) {
		Errorf("Unicode %#x overflow!", unicode_ch);
		return NULL;
	}

	for (glyph = font->glyphs[lo]; glyph != NULL; glyph = glyph->sjisNext) {
		if (glyph->sjisByte1 == hi)
			return glyph;
	}

	/**
	 * If we failed to get the glyph, maybe we can try on reverting narrow fonts
	 */
#if 0
	if (!is_ascii(unicode_ch))
		return GetCharGlyphUnicode(NarrowFontsUnicodeToAscii(unicode_ch), font);
#endif

	Errorf("Failed to get glyph: %#x", unicode_ch);
	return NULL;
}

LYN_REPLACE_CHECK(GetCharTextLen);
const char *GetCharTextLen(const char *str, u32 *width)
{
	struct Glyph *glyph;
	u32 unicod;
	int ret, decode_len;

	ret = DecodeUtf8(str, &unicod, &decode_len);
	if (ret)
		return GetCharTextLen("?", width);

	glyph = GetCharGlyphUnicode(unicod, gActiveFont);
	if (glyph == NULL)
		return GetCharTextLen("?", width);

	*width = glyph->width;
	return str + decode_len;
}

LYN_REPLACE_CHECK(GetStringTextLen);
int GetStringTextLen(const char *str)
{
	u32 _wid;
	int width = 0;

	while (*str != 0 && *str != CHAR_NEWLINE) {
		str = GetCharTextLen(str, &_wid);
		width += _wid;
	}
	return width;
}

LYN_REPLACE_CHECK(Text_DrawCharacter);
const char *Text_DrawCharacter(struct Text *text, const char *str)
{
	struct Glyph *glyph;
	u32 unicod;
	int ret, decode_len;

	ret = DecodeUtf8(str, &unicod, &decode_len);
	if (ret) {
		unicod = '?';
		decode_len = 1;
	}

	glyph = GetCharGlyphUnicode(unicod, gActiveFont);
	if (glyph == NULL)
		glyph = GetCharGlyphUnicode('?', gActiveFont);

	gActiveFont->drawGlyph(text, glyph);
	if (ret == 0)
		Chatlog_AppendUnicode(unicod);
	return str + decode_len;
}

LYN_REPLACE_CHECK(Text_DrawCharacterAscii);
const char *Text_DrawCharacterAscii(struct Text *text, const char *str)
{
	/* Vanilla ASCII path is incompatible with this kernel's UTF-8 fonts. */
	return Text_DrawCharacter(text, str);
}

LYN_REPLACE_CHECK(Text_DrawString);
void Text_DrawString(struct Text *text, const char *str)
{
	while (*str != 0 && *str != CHAR_NEWLINE)
		str = Text_DrawCharacter(text, str);
}

#if defined(CONFIG_FONT_MOTHER_3) || defined(CONFIG_FONT_RIVIERA) || defined(CONFIG_FONT_MINISH_CAP) || defined(CONFIG_FONT_RED_RESCUE_TEAM)
/* 1px ink-to-ink gap. Ones-place right edge is start+PITCH so 11 and 23 line up. */
#define TEXT_NUMBER_PITCH 6
#define TEXT_NUMBER_GAP 1
#define TEXT_NUMBER_EQUAL_SPACING 1
#elif defined(CONFIG_FONT_ADVANCE_WARS_2) || defined(CONFIG_FONT_SUPER_STAR_SAGA)
#define TEXT_NUMBER_PITCH 8
#define TEXT_NUMBER_RIGHT_ALIGN 1
#define TEXT_NUMBER_EQUAL_SPACING 0
#else
#define TEXT_NUMBER_PITCH 8
#define TEXT_NUMBER_RIGHT_ALIGN 0
#define TEXT_NUMBER_EQUAL_SPACING 0
#endif

STATIC_DECLAR void DrawNumberDigit(struct Text *text, int digit)
{
	char buf[2];
	u32 width;
	int start = text->x;

	buf[0] = '0' + digit;
	buf[1] = '\0';
	GetCharTextLen(buf, &width);

#if TEXT_NUMBER_EQUAL_SPACING
	/* x is this digit's right edge. Next digit's right edge is left of us minus the gap. */
	text->x = start - (int)width;
	Text_DrawCharacter(text, buf);
	text->x = start - (int)width - TEXT_NUMBER_GAP;
#else
	if (TEXT_NUMBER_RIGHT_ALIGN) {
		int pad = TEXT_NUMBER_PITCH - (int)width;

		if (pad < 0)
			pad = 0;
		text->x = start + pad;
	}

	Text_DrawCharacter(text, buf);
	text->x = start - TEXT_NUMBER_PITCH;
#endif
}

LYN_REPLACE_CHECK(Text_DrawNumber);
void Text_DrawNumber(struct Text *text, int n)
{
#if TEXT_NUMBER_EQUAL_SPACING
	text->x += TEXT_NUMBER_PITCH;
#endif

	if (n == 0) {
		DrawNumberDigit(text, 0);
		return;
	}

	while (n != 0) {
		DrawNumberDigit(text, k_umod(n, 10));
		n = k_udiv(n, 10);
	}
}

LYN_REPLACE_CHECK(InsertPrefix);
void InsertPrefix(char *str, const char *insert_str, s8 c)
{
	int len = strlen(str);

	if (insert_str != NULL) {
		int len_sert = strlen(insert_str);

		for (int i = len; i >= 0; i--)
			str[i + len_sert] = str[i];

		for (int i = 0; i < len_sert; i++)
			str[i] = insert_str[i];

		return;
	}

	for (int i = len; i >= 0; i--)
		str[i + 1] = str[i];

	str[0] = ' ';
}
