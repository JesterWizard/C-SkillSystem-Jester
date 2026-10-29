#pragma once

/*
 * Stand-ins for the C Skill System kernel pieces TextEngine.c was written
 * against, so the same code builds on clean FE8U with FE-CLib alone.
 */

#include "gbafe.h"
#include <stdint.h>
#include <string.h>

extern struct Font *gActiveFont;

/* The kernel's unsigned modulo; small positive values here, so BIOS DivRem does. */
static inline u32 k_umod(u32 a, u32 b) { return DivRem(a, b); }

/* Shared with the kernel's other sprite code; only the text engine uses them here. */
s16 TextEngine_GetStaticOffsetAtY(int y);
s16 TextEngine_GetFxOffsetY(void);
int TextEngine_ApplyStaticOam1(int xOam1, int screenY);
int TextEngine_ApplyFxOam0(int yOam0);
void TextEngine_PutFaceSprite(int layer, int xOam1, int yOam0, const u16 *object, int oam2);

/* Source/TextAliases.c: "[ToggleOnBigText]" and friends typed as plain text. */
void TextEngine_ExpandAliases(char *str);

/* Hooks are placed by Installer.event instead of LynJump. */
#define LYN_REPLACE_CHECK(name)

/* Settings a ROM hacker can change in Installer.event without recompiling. */
struct TextEngineConfig {
	/* Dim the portraits of characters who aren't speaking. */
	u8 dim_listener_portraits;
};

extern const struct TextEngineConfig gTextEngineConfig;

/*
 * Half-body and 32-colour portraits: Standalone/portrait_formats'
 * Source/Portraits.c (built here into Source/Portraits.lyn.event) when
 * Installer.event defines TEXT_ENGINE_HALF_BODY_PORTRAITS, the no-ops in
 * Source/PortraitStubs.c otherwise.  Linked through Event Assembler labels.
 */
bool Portraits_IsHalfBodyFace(struct FaceProc *face);
bool Portraits_TalkFaceWillBeHalfBody(int fid, int disp);
void HalfBody_OnTalkFaceClear(struct FaceProc *face);
s32 Portrait32_GetOverlayOam2(struct FaceProc *face);

/*
 * The conversation log: Source/Chatlog.c when Installer.event defines
 * TEXT_ENGINE_CHATLOG, the no-ops in Source/ChatlogStubs.c otherwise.
 * Either one is linked to the engine through Event Assembler labels.
 */
#include "Chatlog.h"

/* Vanilla FE8U text is one byte per character. */
static inline int DecodeUtf8(const char *str, u32 *unicode_out, int *len)
{
	*unicode_out = (u8)*str;
	*len = 1;
	return 0;
}
