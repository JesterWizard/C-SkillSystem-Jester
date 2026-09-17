#include "common-chax.h"
#include "kernel-lib.h"
#include "utf8.h"
#include "kernel/chatlog.h"

#include "bmlib.h"
#include "ctc.h"
#include "event.h"
#include "face.h"
#include "fontgrp.h"
#include "hardware.h"
#include "m4a.h"
#include "rng.h"
#include "scene.h"
#include "soundwrapper.h"
#include "constants/songs.h"

extern void HalfBody_OnTalkFaceClear(struct FaceProc *proc);

/*
 * The vanilla dialogue state is allocated in EWRAM by the game.  Keep these
 * aliases local to this module instead of adding another global definition to
 * the shared library.
 */
#define sTextEngineState (*(struct TalkState **)0x0859133C)
#define sTextEngineText  (*(struct Text (*)[3])0x030000D0)

#define TEXT_ENGINE_FACE_ATTRIBUTES_OFFSET 0x4C
#define TEXT_ENGINE_SHAKE_PRINT_FLAG_OFFSET 0x5D
#define TEXT_ENGINE_LETTER_FX_MODE_OFFSET 0x5E

enum TextEngineFaceAttribute {
	TEXT_ENGINE_ATTR_FONT,
	TEXT_ENGINE_ATTR_COLOR_GROUP,
	TEXT_ENGINE_ATTR_BOX_PALETTE,
	TEXT_ENGINE_ATTR_BOX_TYPE,
	TEXT_ENGINE_ATTR_BOOP_PITCH,
};

enum {
	TEXT_ENGINE_FACE_MOTION_JUMP = 0,
	TEXT_ENGINE_FACE_MOTION_VIBRATE = 1,
	TEXT_ENGINE_FACE_MOTION_SHIMMY = 2,
	TEXT_ENGINE_FACE_JUMP_PERIOD = 16,
	TEXT_ENGINE_FACE_JUMP_HEIGHT = 6,
	TEXT_ENGINE_FACE_VIBRATE_PERIOD = 4,
	TEXT_ENGINE_FACE_SHIMMY_PERIOD = 4,
	TEXT_ENGINE_PRINT_SHAKE_FRAMES = 4,
	TEXT_ENGINE_PRINT_FX_INACTIVE = -1,
	TEXT_ENGINE_WAVE_AMPLITUDE = 2,
	TEXT_ENGINE_WAVE_FREQUENCY = 2,
	TEXT_ENGINE_WAVE_SPEED = 2,
	/* Horizontal screen tear: scanlines below the split jump this many pixels. */
	TEXT_ENGINE_TEAR_SHIFT = 40,
	/* How fast the split crawls down the screen, in scanlines per frame. */
	TEXT_ENGINE_TEAR_SPEED = 3,
	/* Video-game static: selected scanlines flash white, no BG slide. */
	TEXT_ENGINE_STATIC_CLUSTERS = 3,
	TEXT_ENGINE_FLOAT_SLOTS = 4,
	/*
	 * LoadObjUIGfx packs a 0x12x4 sheet into OBJ VRAM with 32-tile pitch:
	 * rows at 0x00, 0x20, 0x40, 0x60.  Stay clear of those and of
	 * OBJCHR_MAPSPRITES (0x80).  0x72-0x7F is free for 8x16 float slots.
	 */
	TEXT_ENGINE_FLOAT_OBJ_CHR = 0x72,
	/* OBJPAL_MAPSPRITES is 0xC — never reuse it for float glyphs. */
	TEXT_ENGINE_FLOAT_OBJ_PAL = 0xA,
	TEXT_ENGINE_FLOAT_SLOT_STRIDE = 2,
	TEXT_ENGINE_FLOAT_DURATION = 12,
	TEXT_ENGINE_FLOAT_LIFT = 10,
	TEXT_ENGINE_DRIP_LIFT = 22,
	TEXT_ENGINE_WAVY_AMPLITUDE = 3,
	TEXT_ENGINE_SCRAMBLE_LOCK = 4,
	TEXT_ENGINE_GHOST_ECHO = 6,
	TEXT_ENGINE_LETTER_FX_NONE = 0,
	TEXT_ENGINE_LETTER_FX_BOUNCE = 1,
	TEXT_ENGINE_LETTER_FX_WAVY = 2,
	TEXT_ENGINE_LETTER_FX_SCRAMBLE = 3,
	TEXT_ENGINE_LETTER_FX_DRIP = 4,
	TEXT_ENGINE_LETTER_FX_GHOST = 5,
	TEXT_ENGINE_BOOP_PITCH_COUNT = 25,
	/*
	 * Nameplates are drawn over the top edge of the dialogue box.  These
	 * values intentionally live next to the engine constants so projects can
	 * pick a different custom font or text group without changing scripts.
	 */
	TEXT_ENGINE_NAMEPLATE_FONT = 1,
	TEXT_ENGINE_NAMEPLATE_COLOR = TEXT_COLOR_0DEF,
	TEXT_ENGINE_NAMEPLATE_WIDTH = 30,
	TEXT_ENGINE_NAMEPLATE_HEIGHT = 2,
	/* Tiles above yText; 4 = sit 16px above the dialogue top edge. */
	TEXT_ENGINE_NAMEPLATE_Y_OFFSET = 4,
	/* Extra inner tiles so short names are not flush against the frame. */
	TEXT_ENGINE_NAMEPLATE_PAD_TILES = 2,
	/* PutTalkBubbleTm width/height include the border tiles. */
	TEXT_ENGINE_NAMEPLATE_BG1_MAX_WIDTH = TEXT_ENGINE_NAMEPLATE_WIDTH + 2,
	/*
	 * Ash dissolve rebuilds the mug from its OBJ list each frame, so
	 * chip motion lives on the proc (seed + origin) instead of EWRAM.
	 */
	TEXT_ENGINE_ASH_OAM_BUDGET = 40,
	TEXT_ENGINE_ASH_DURATION = 48,
	TEXT_ENGINE_ASH_HOLD = 4,
	TEXT_ENGINE_ASH_FADE_START = 24,
	TEXT_ENGINE_ASH_MOSAIC_MAX = 3,
	TEXT_ENGINE_ASH_OVERLAY_CHIP_LIMIT = 32,
	TEXT_ENGINE_CMD_CLEAR_FACE_ASH = 0x4B,
	TEXT_ENGINE_CMD_WAVY_PRINT_ON = 0x4C,
	TEXT_ENGINE_CMD_WAVY_PRINT_OFF = 0x4D,
	TEXT_ENGINE_CMD_SCRAMBLE_PRINT_ON = 0x4E,
	TEXT_ENGINE_CMD_SCRAMBLE_PRINT_OFF = 0x4F,
	TEXT_ENGINE_CMD_DRIP_PRINT_ON = 0x50,
	TEXT_ENGINE_CMD_DRIP_PRINT_OFF = 0x51,
	TEXT_ENGINE_CMD_GHOST_PRINT_ON = 0x52,
	TEXT_ENGINE_CMD_GHOST_PRINT_OFF = 0x53,
	TEXT_ENGINE_CMD_EARTHQUAKE_ON = 0x54,
	TEXT_ENGINE_CMD_EARTHQUAKE_OFF = 0x55,
	TEXT_ENGINE_CMD_IMPACT_FLASH = 0x56,
	TEXT_ENGINE_EARTHQUAKE_PERIOD = 2,
	/* Hard white hit, then a short decay.  One-shot, not a toggle. */
	TEXT_ENGINE_IMPACT_FLASH_HOLD = 2,
	TEXT_ENGINE_IMPACT_FLASH_DURATION = 12,
	TEXT_ENGINE_IMPACT_FLASH_PEAK = 16,
};

struct TextEngineFaceJumpProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct FaceProc *face;
	/* 30 */ s16 timer;
	/* 32 */ s16 offset;
	/* 34 */ u8 mode;
	/* 35 */ u8 unused;
};

struct TextEnginePrintFxProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ s16 shakeTimer;
	/* 2E */ s16 unusedBounce;
	/* 30 */ s16 baseX;
	/* 32 */ s16 baseY;
};

struct TextEngineWaveProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ s16 phase;
	/* 2E */ u8 activeBuffer;
	/* 2F */ u8 unused;
	/* 30 */ void (*previousHBlankHandler)(void);
};

struct TextEngineScreenGlitchProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ void (*previousHBlankHandler)(void);
};

struct TextEngineScreenStaticProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ void (*previousHBlankHandler)(void);
};

struct TextEngineScreenEarthquakeProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ void (*previousHBlankHandler)(void);
	/* 30 */ s16 timer;
};

struct TextEngineImpactFlashProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ s16 timer;
	/* 2E */ u16 savedBldcnt;
	/* 30 */ u8 savedCoeffA;
	/* 31 */ u8 savedCoeffB;
	/* 32 */ u8 savedBlendY;
	/* 33 */ u8 savedWinBlend;
};

struct TextEngineEarthquakeOffset {
	s8 x;
	s8 y;
};

struct TextEngineScanlineFxRam {
	s8 x[DISPLAY_HEIGHT];
	u8 blend[DISPLAY_HEIGHT / 2];
};

struct TextEngineGlyphFloatProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct Text *targetText;
	/* 30 */ struct Glyph *glyph;
	/* 34 */ s16 cursorX;
	/* 36 */ s16 screenX;
	/* 38 */ s16 destY;
	/* 3A */ s16 timer;
	/* 3C */ u8 slot;
	/* 3D */ u8 color;
	/* 3E */ u8 width;
	/* 3F */ char ch[5];
	/* 44 */ u8 mode;
	/* 45 */ u8 duration;
};

struct TextEngineAshDissolveProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct FaceProc *face;
	/* 30 */ s16 timer;
	/* 32 */ u16 savedMosaic;
	/* 34 */ u16 savedBldcnt;
	/* 36 */ u16 overlayOam2;
	/* 38 */ u8 savedCoeffA;
	/* 39 */ u8 savedCoeffB;
	/* 3A */ u8 savedBlendY;
	/* 3B */ u8 savedWoutBlend;
	/* 3C */ u8 layer;
	/* 3D */ u8 overlayEnabled;
	/* 3E */ u8 step;
	/* 3F */ u8 unused;
	/* 40 */ s16 originX;
	/* 42 */ s16 originY;
	/* 44 */ u32 seed;
};

struct TextEngineNameplateState {
	struct Text text;
	u16 faceNameTextIds[8];
	u16 bg1Backup[TEXT_ENGINE_NAMEPLATE_BG1_MAX_WIDTH * TEXT_ENGINE_NAMEPLATE_HEIGHT];
	u8 active;
	u8 enabled;
	u8 x;
	u8 y;
	u8 width;
	u8 bg1X;
	u8 bg1Y;
	u8 bg1Width;
};

extern EWRAM_DATA struct TextEngineNameplateState sTextEngineNameplateState;

struct TextEnginePortraitNameAlias {
	u8 portraitId;
	u8 canonicalPortraitId;
};

static const struct TextEnginePortraitNameAlias sTextEnginePortraitNameAliases[] = {
	{ 0x03, 0x02 }, /* Eirika closed */
	{ 0x0B, 0x0A }, /* Neimi closed */
	{ 0x0D, 0x0C }, /* Colm closed */
	{ 0x12, 0x11 }, /* Natasha closed */
	{ 0x15, 0x14 }, /* Ephraim closed */
	{ 0x17, 0x16 }, /* Forde closed */
	{ 0x1D, 0x1C }, /* Tethys closed */
	{ 0x1F, 0x1E }, /* Marisa closed */
	{ 0x27, 0x26 }, /* Myrrh closed */
	{ 0x28, 0x26 }, /* Myrrh with wings */
	{ 0x2D, 0x02 }, /* Eirika past */
	{ 0x2E, 0x14 }, /* Ephraim past */
	{ 0x2F, 0x29 }, /* Knoll past */
	{ 0x41, 0x40 }, /* Vigarde healthy */
	{ 0x47, 0x46 }, /* Lyon closed */
	{ 0x4A, 0x46 }, /* Lyon Demon King */
	{ 0x4E, 0x40 }, /* Vigarde past */
	{ 0x4F, 0x40 }, /* Vigarde past, closed */
	{ 0x50, 0x46 }, /* Lyon past */
	{ 0x57, 0x56 }, /* Ismaire closed */
};

struct TextEngineCommandDescriptor;

typedef int (*TextEngineCommandHandler)(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
);

typedef void (*TextEngineCommandWidthHandler)(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
);

typedef void (*TextEngineCommandCleanupHandler)(void);

struct TextEngineCommandDescriptor {
	u8 code;
	u8 argumentCount;
	TextEngineCommandHandler handler;
	TextEngineCommandWidthHandler width;
	TextEngineCommandCleanupHandler cleanup;
};

/*
 * These tables are emitted by _Text_Engine_Tables.txt, which is included by
 * the installer after this C module has been linked.
 */
extern struct Glyph **FontGlyphsPointerTable[];
extern const u16 TextPaletteTable[];
extern const u16 TextBoxBgPaletteTable[];
extern const u32 *TextBoxTypePointerTable[];
extern const struct SongHeader TextBoopTable[];

extern const struct ProcCmd gProcScr_TalkSkipListener[];
extern const struct ProcCmd gProcScr_TalkPause[];
extern const struct ProcCmd gProcScr_TalkShiftClearAll[];
extern const struct ProcCmd gProcScr_TalkFaceMove[];
extern const struct ChoiceEntryInfo gYesNoTalkChoice[];
extern const struct ChoiceEntryInfo gBuySellTalkChoice[];

typedef void (*TextEngineUnsetFaceDisplayBitsFunc)(int position);

#define TextEngineUnsetFaceDisplayBits \
	((TextEngineUnsetFaceDisplayBitsFunc)(uintptr_t)0x080089C5)

extern u16 *GetColorLut(int color);

int TalkInterpret(ProcPtr proc);
int GetStringTextWidthWithDialogueCodes(const char *text, int stopAtCurrentBox);
struct Proc *StartTalkFaceMove_C(int talkFaceFrom, int talkFaceTo, s8 isSwap);
void UpdateFontGlyphSet(int font);
void TextEngine_OnCharacterPrinted(void);
void TextEngine_PlayTextBoop(const char *text);
s8 TextEngine_TryStartGlyphFloat(struct Text *text, const char **str);

static const struct TextEngineCommandDescriptor *TextEngine_FindCommand(u8 code);
static void TextEngine_RunCommandCleanup(void);
static int TextEngine_WidthInternal(const u8 *cursor, int stopAtCurrentBox);
static struct FaceProc *TextEngine_GetFaceProcByPosition(int position);
static void TextEngine_ClearSpeakerNameplate(void);
static void TextEngine_DrawSpeakerNameplate(ProcPtr proc);
static u8 *TextEngine_GetFaceAttributes(struct FaceProc *face);

static void TextEngineFaceJump_OnIdle(struct TextEngineFaceJumpProc *proc);
static void TextEngineFaceJump_OnEnd(struct TextEngineFaceJumpProc *proc);
static void TextEngine_StartFaceVibrate(struct FaceProc *face);
static void TextEngine_StopFaceVibrate(struct FaceProc *face);
static void TextEngine_StartFaceShimmy(struct FaceProc *face);
static void TextEngine_StopFaceShimmy(struct FaceProc *face);
static void TextEnginePrintFx_OnIdle(struct TextEnginePrintFxProc *proc);
static void TextEnginePrintFx_OnEnd(struct TextEnginePrintFxProc *proc);
static void TextEngineWave_OnIdle(struct TextEngineWaveProc *proc);
static void TextEngineWave_OnEnd(struct TextEngineWaveProc *proc);
static void TextEngineWave_OnHBlank(void);
static void TextEngineWave_ApplyBaseOffsets(void);
static void TextEngineScreenGlitch_OnIdle(ProcPtr proc);
static void TextEngineScreenGlitch_OnEnd(struct TextEngineScreenGlitchProc *proc);
static void TextEngineScreenStatic_OnIdle(ProcPtr proc);
static void TextEngineScreenStatic_OnEnd(struct TextEngineScreenStaticProc *proc);
static void TextEngineScreenEarthquake_OnIdle(struct TextEngineScreenEarthquakeProc *proc);
static void TextEngineScreenEarthquake_OnEnd(struct TextEngineScreenEarthquakeProc *proc);
static void TextEngineImpactFlash_OnIdle(struct TextEngineImpactFlashProc *proc);
static void TextEngineImpactFlash_OnEnd(struct TextEngineImpactFlashProc *proc);
static void TextEngineGlyphFloat_OnIdle(struct TextEngineGlyphFloatProc *proc);
static void TextEngineGlyphFloat_OnEnd(struct TextEngineGlyphFloatProc *proc);
static void TextEngineAshDissolve_OnIdle(struct TextEngineAshDissolveProc *proc);
static void TextEngineAshDissolve_OnEnd(struct TextEngineAshDissolveProc *proc);
static void TextEngine_StartAshDissolve(struct FaceProc *face);
static void TextEngine_CleanupAshDissolve(void);

static const s8 sTextEnginePrintShakeOffsets[][2] = {
	{ +1, -1 },
	{ -1, +1 },
	{ +1,  0 },
	{  0,  0 },
};

static const s8 sTextEngineFaceVibrateOffsets[] = {
	0, +1, 0, -1,
};

static const s8 sTextEngineEarthquakeOffsets[][2] = {
	{ +2,  0 },
	{ -2, +2 },
	{ +2, -2 },
	{ -1, +1 },
	{ +1, -2 },
	{ -2, +1 },
	{ +2, -1 },
	{  0, +2 },
};

extern EWRAM_DATA struct TextEngineScanlineFxRam sTextEngineWaveOffsets;
extern EWRAM_DATA struct TextEngineEarthquakeOffset sTextEngineEarthquakeOffset;

static const struct ProcCmd gProcScr_TextEngineFaceJump[] = {
	PROC_NAME("TextEngineFaceJump"),
	PROC_SET_END_CB(TextEngineFaceJump_OnEnd),
	PROC_REPEAT(TextEngineFaceJump_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEnginePrintFx[] = {
	PROC_NAME("TextEnginePrintFx"),
	PROC_SET_END_CB(TextEnginePrintFx_OnEnd),
	PROC_REPEAT(TextEnginePrintFx_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineWave[] = {
	PROC_NAME("TextEngineWave"),
	PROC_SET_END_CB(TextEngineWave_OnEnd),
	PROC_REPEAT(TextEngineWave_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineScreenGlitch[] = {
	PROC_NAME("TextEngineScreenGlitch"),
	PROC_SET_END_CB(TextEngineScreenGlitch_OnEnd),
	PROC_REPEAT(TextEngineScreenGlitch_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineScreenStatic[] = {
	PROC_NAME("TextEngineScreenStatic"),
	PROC_SET_END_CB(TextEngineScreenStatic_OnEnd),
	PROC_REPEAT(TextEngineScreenStatic_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineScreenEarthquake[] = {
	PROC_NAME("TextEngineScreenEarthquake"),
	PROC_SET_END_CB(TextEngineScreenEarthquake_OnEnd),
	PROC_REPEAT(TextEngineScreenEarthquake_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineImpactFlash[] = {
	PROC_NAME("TextEngineImpactFlash"),
	PROC_SET_END_CB(TextEngineImpactFlash_OnEnd),
	PROC_REPEAT(TextEngineImpactFlash_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineGlyphFloat[] = {
	PROC_NAME("TextEngineGlyphFloat"),
	PROC_SET_END_CB(TextEngineGlyphFloat_OnEnd),
	PROC_REPEAT(TextEngineGlyphFloat_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEngineAshDissolve[] = {
	PROC_NAME("TextEngineAshDissolve"),
	PROC_SET_END_CB(TextEngineAshDissolve_OnEnd),
	PROC_REPEAT(TextEngineAshDissolve_OnIdle),
	PROC_END,
};

static struct Text *TextEngine_GetLineText(const struct TalkState *state, int line)
{
	int index;

	if (!state->lines)
		return &sTextEngineText[0];

	/*
	 * Keep this in sync with Talk_OnIdle's line selection.  The target
	 * runtime's normal modulo path is not reliable for this calculation.
	 */
	index = k_umod(line + state->topTextNum, state->lines);
	return &sTextEngineText[index];
}

static const struct CharacterData *TextEngine_FindCharacterByPortrait(int portraitId)
{
	const struct CharacterData *character;
	int canonicalPortraitId = portraitId;
	int characterId;

	/*
	 * Prefer an exact match so projects can define a distinct character
	 * entry for a variant portrait.  Fall back to the base portrait aliases
	 * used by the stock face definitions when no exact entry exists.
	 */
	for (characterId = 1; characterId < 0x100; characterId++) {
		character = GetCharacterData(characterId);

		if (!character ||
			!character->nameTextId)
			continue;

		if (character->portraitId == portraitId)
			return character;
	}

	for (characterId = 0;
		characterId < (int)ARRAY_COUNT(sTextEnginePortraitNameAliases);
		characterId++
	) {
		if (sTextEnginePortraitNameAliases[characterId].portraitId == portraitId) {
			canonicalPortraitId =
				sTextEnginePortraitNameAliases[characterId].canonicalPortraitId;
			break;
		}
	}

	if (canonicalPortraitId == portraitId)
		return NULL;

	for (characterId = 1; characterId < 0x100; characterId++) {
		character = GetCharacterData(characterId);

		if (!character ||
			!character->nameTextId)
			continue;

		if (character->portraitId == canonicalPortraitId)
			return character;
	}

	return NULL;
}

static void TextEngine_ClearSpeakerNameplate(void)
{
	int row;
	int col;
	u16 *backup;

	if (Chatlog_IsVisible())
		return;

	if (sTextEngineNameplateState.active != 1) {
		sTextEngineNameplateState.active = 0;
		return;
	}

	TileMap_FillRect(
		gBG0TilemapBuffer + TILEMAP_INDEX(
			sTextEngineNameplateState.x,
			sTextEngineNameplateState.y
		),
		sTextEngineNameplateState.width,
		TEXT_ENGINE_NAMEPLATE_HEIGHT,
		0
	);

	/*
	 * Restore the dialogue-bubble tiles that the nameplate frame overwrote
	 * on BG1 (typically the shared top border row).
	 */
	backup = sTextEngineNameplateState.bg1Backup;
	for (row = 0; row < TEXT_ENGINE_NAMEPLATE_HEIGHT; row++) {
		for (col = 0; col < sTextEngineNameplateState.bg1Width; col++) {
			gBG1TilemapBuffer[TILEMAP_INDEX(
				sTextEngineNameplateState.bg1X + col,
				sTextEngineNameplateState.bg1Y + row
			)] = *backup++;
		}
	}

	/* Restore vanilla InitTalkTextWin outside-window masking. */
	SetWOutLayers(0, 1, 1, 1, 1);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	sTextEngineNameplateState.active = 0;
}

static void TextEngine_ClearFaceNameTextIds(void)
{
	int i;

	for (i = 0;
		i < (int)ARRAY_COUNT(sTextEngineNameplateState.faceNameTextIds);
		i++
	)
		sTextEngineNameplateState.faceNameTextIds[i] = 0;
}

static int TextEngine_CopyNameplateString(
	const char *source,
	char *target,
	int targetSize,
	int maxWidth
)
{
	const char *cursor = source;
	int targetLength = 0;
	int textWidth = 0;

	while (*cursor && targetLength < targetSize - 1) {
		const char *next;
		int charLength;
		int charWidth;
		u32 glyphWidth;

		next = GetCharTextLen(cursor, &glyphWidth);
		if (!next || next <= cursor)
			break;

		charLength = (int)(next - cursor);
		charWidth = (int)glyphWidth;

		if (charLength <= 0 ||
			targetLength + charLength >= targetSize ||
			charWidth <= 0 ||
			textWidth + charWidth > maxWidth)
			break;

		memcpy(target + targetLength, cursor, charLength);
		targetLength += charLength;
		textWidth += charWidth;
		cursor = next;
	}

	target[targetLength] = '\0';
	return textWidth;
}

static void TextEngine_DrawSpeakerNameplate(ProcPtr proc)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	const struct CharacterData *character;
	const char *name;
	char nameBuffer[0x20];
	struct Glyph **originalGlyphs;
	u16 *backup;
	int innerWidth;
	int nameWidth;
	int nameX;
	int nameY;
	int bg1X;
	int bg1Y;
	int bg1Width;
	int dialogueWidth;
	int speakingFaceSlot;
	int nameFont;
	int nameColor;
	int row;
	int col;
	u16 nameTextId;
	u8 *faceAttributes;

	(void)proc;

	/* The dialogue is frozen behind the chatlog; do not redraw over it. */
	if (Chatlog_IsVisible())
		return;

	TextEngine_ClearSpeakerNameplate();

	if (!state ||
		!sTextEngineNameplateState.enabled ||
		CheckTalkFlag(TALK_FLAG_SPRITE) ||
		CheckTalkFlag(TALK_FLAG_NOBUBBLE))
		return;

	speakingFaceSlot = (s8)state->speakingFaceSlot;
	face = TextEngine_GetFaceProcByPosition(speakingFaceSlot);
	if (!face)
		return;

	nameTextId = 0;
	if (speakingFaceSlot >= 0 &&
		speakingFaceSlot <
			(int)ARRAY_COUNT(sTextEngineNameplateState.faceNameTextIds))
		nameTextId = sTextEngineNameplateState.faceNameTextIds[speakingFaceSlot];

	if (!nameTextId) {
		character = TextEngine_FindCharacterByPortrait(face->faceId);
		if (!character)
			return;

		nameTextId = character->nameTextId;
	}

	name = GetStringFromIndex(nameTextId);
	if (!name || !*name)
		return;

	dialogueWidth = state->activeWidth;
	if (dialogueWidth <= 2)
		return;

	nameY = state->yText - TEXT_ENGINE_NAMEPLATE_Y_OFFSET;
	if (nameY < 0)
		return;

	ClearText(&sTextEngineNameplateState.text);
	sTextEngineNameplateState.text.tile_width = TEXT_ENGINE_NAMEPLATE_WIDTH;

	originalGlyphs = gActiveFont->glyphs;
	nameFont = TEXT_ENGINE_NAMEPLATE_FONT;
	nameColor = TEXT_ENGINE_NAMEPLATE_COLOR;
	faceAttributes = TextEngine_GetFaceAttributes(face);
	if (faceAttributes) {
		nameFont = faceAttributes[TEXT_ENGINE_ATTR_FONT];
		nameColor = faceAttributes[TEXT_ENGINE_ATTR_COLOR_GROUP];
	}

	UpdateFontGlyphSet(nameFont);
	nameWidth = TextEngine_CopyNameplateString(
		name,
		nameBuffer,
		sizeof(nameBuffer),
		TEXT_ENGINE_NAMEPLATE_WIDTH * 8
	);

	if (!nameWidth) {
		gActiveFont->glyphs = originalGlyphs;
		return;
	}

	/*
	 * Size the BG1 frame to the name, then center it over the dialogue bubble.
	 * PutTalkBubbleTm width/height include the border tiles.
	 */
	innerWidth = (nameWidth + 7) / 8 + TEXT_ENGINE_NAMEPLATE_PAD_TILES;
	if (innerWidth < 1)
		innerWidth = 1;
	if (innerWidth > TEXT_ENGINE_NAMEPLATE_WIDTH)
		innerWidth = TEXT_ENGINE_NAMEPLATE_WIDTH;
	if (innerWidth > dialogueWidth - 2)
		innerWidth = dialogueWidth - 2;

	bg1Width = innerWidth + 2;
	bg1X = (state->xText - 1) + (dialogueWidth - bg1Width) / 2;
	bg1Y = nameY;
	if (bg1X < 0 ||
		bg1Y < 0 ||
		bg1Width > TEXT_ENGINE_NAMEPLATE_BG1_MAX_WIDTH ||
		bg1X + bg1Width > 32 ||
		bg1Y + TEXT_ENGINE_NAMEPLATE_HEIGHT > 32) {
		gActiveFont->glyphs = originalGlyphs;
		return;
	}

	sTextEngineNameplateState.text.tile_width = innerWidth;
	nameX = GetStringTextCenteredPos(innerWidth * 8, nameBuffer);
	if (nameX < 0)
		nameX = 0;

	Text_SetParams(
		&sTextEngineNameplateState.text,
		nameX,
		nameColor
	);
	Text_DrawString(&sTextEngineNameplateState.text, nameBuffer);
	gActiveFont->glyphs = originalGlyphs;

	backup = sTextEngineNameplateState.bg1Backup;
	for (row = 0; row < TEXT_ENGINE_NAMEPLATE_HEIGHT; row++) {
		for (col = 0; col < bg1Width; col++) {
			*backup++ = gBG1TilemapBuffer[TILEMAP_INDEX(
				bg1X + col,
				bg1Y + row
			)];
		}
	}

	PutTalkBubbleTm(BG_1, bg1X, bg1Y, bg1Width, TEXT_ENGINE_NAMEPLATE_HEIGHT);
	PutText(
		&sTextEngineNameplateState.text,
		gBG0TilemapBuffer + TILEMAP_INDEX(bg1X + 1, nameY)
	);

	/*
	 * Nameplates sit above the WIN0 dialogue clip region.  Enable BG0 outside
	 * the window for as long as the plate is active.
	 */
	SetWOutLayers(1, 1, 1, 1, 1);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	sTextEngineNameplateState.x = bg1X + 1;
	sTextEngineNameplateState.y = nameY;
	sTextEngineNameplateState.width = innerWidth;
	sTextEngineNameplateState.bg1X = bg1X;
	sTextEngineNameplateState.bg1Y = bg1Y;
	sTextEngineNameplateState.bg1Width = bg1Width;
	sTextEngineNameplateState.active = 1;
}

static u8 *TextEngine_GetCurrentSpeakerAttributes(void)
{
	return (u8 *)sTextEngineState + 0x58;
}

static u8 *TextEngine_GetShakePrintFlag(void)
{
	return (u8 *)sTextEngineState + TEXT_ENGINE_SHAKE_PRINT_FLAG_OFFSET;
}

static u8 *TextEngine_GetLetterFxMode(void)
{
	return (u8 *)sTextEngineState + TEXT_ENGINE_LETTER_FX_MODE_OFFSET;
}

static u8 *TextEngine_GetFaceAttributes(struct FaceProc *face)
{
	if (!face)
		return NULL;

	return (u8 *)face + TEXT_ENGINE_FACE_ATTRIBUTES_OFFSET;
}

static struct FaceProc *TextEngine_GetFaceProcByPosition(int position)
{
	if (position < 0 || position >= (int)ARRAY_COUNT(sTextEngineState->faces))
		return NULL;

	return sTextEngineState->faces[position];
}

u16 TextEngine_GetSpeakingNameTextId(void)
{
	struct TalkState *state = sTextEngineState;
	const struct CharacterData *character;
	struct FaceProc *face;
	int slot;

	if (!state)
		return 0;

	slot = (s8)state->speakingFaceSlot;
	if (slot < 0)
		slot = state->activeFaceSlot;

	face = TextEngine_GetFaceProcByPosition(slot);
	if (!face)
		return 0;

	character = TextEngine_FindCharacterByPortrait(face->faceId);
	if (!character)
		return 0;

	return character->nameTextId;
}

int TextEngine_GetSpeakingFaceId(void)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	int slot;
	int i;

	if (!state)
		return 0;

	slot = (s8)state->speakingFaceSlot;
	if (slot < 0)
		slot = state->activeFaceSlot;

	face = TextEngine_GetFaceProcByPosition(slot);
	if (face && face->faceId)
		return face->faceId;

	if (state->activeFaceSlot != slot) {
		face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
		if (face && face->faceId)
			return face->faceId;
	}

	for (i = 0; i < (int)ARRAY_COUNT(state->faces); i++) {
		face = state->faces[i];
		if (face && face->faceId)
			return face->faceId;
	}

	return 0;
}

u8 TextEngine_GetSpeakingCharacterId(void)
{
	const struct CharacterData *character;
	int faceId = TextEngine_GetSpeakingFaceId();

	if (!faceId)
		return 0;

	character = TextEngine_FindCharacterByPortrait(faceId);
	if (!character)
		return 0;

	return character->number;
}

static void TextEngine_SetFaceAttribute(struct FaceProc *face, int attribute, u8 value)
{
	u8 *faceAttributes = TextEngine_GetFaceAttributes(face);

	if (faceAttributes)
		faceAttributes[attribute] = value;
}

static void TextEngine_SetCurrentAttributeAndFace(int position, int attribute, u8 value)
{
	struct FaceProc *face = TextEngine_GetFaceProcByPosition(position);

	TextEngine_SetFaceAttribute(face, attribute, value);
	TextEngine_GetCurrentSpeakerAttributes()[attribute] = value;
}

static struct TextEngineFaceJumpProc *TextEngine_FindFaceJumpProc(struct FaceProc *face)
{
	struct ProcFindIterator it;
	struct TextEngineFaceJumpProc *jump;

	Proc_FindBegin(&it, gProcScr_TextEngineFaceJump);
	while ((jump = (struct TextEngineFaceJumpProc *)Proc_FindNext(&it)) != NULL) {
		if (jump->face == face)
			return jump;
	}

	return NULL;
}

static void TextEngineFaceJump_OnEnd(struct TextEngineFaceJumpProc *proc)
{
	if (!proc->face)
		return;

	if (proc->mode == TEXT_ENGINE_FACE_MOTION_SHIMMY)
		proc->face->xPos += proc->offset;
	else
		proc->face->yPos += proc->offset;
}

static void TextEngineFaceJump_OnIdle(struct TextEngineFaceJumpProc *proc)
{
	struct FaceProc *face = proc->face;
	int phase;
	int offset;
	s16 truePosition;

	if (!face) {
		Proc_End(proc);
		return;
	}

	/*
	 * Undo the previous frame's offset first so other systems that also
	 * touch the active position (face moves, fades) compose cleanly with
	 * continuous motion.
	 */
	if (proc->mode == TEXT_ENGINE_FACE_MOTION_SHIMMY)
		truePosition = face->xPos + proc->offset;
	else
		truePosition = face->yPos + proc->offset;

	proc->timer++;

	if (proc->mode == TEXT_ENGINE_FACE_MOTION_VIBRATE) {
		phase = proc->timer % TEXT_ENGINE_FACE_VIBRATE_PERIOD;
		offset = sTextEngineFaceVibrateOffsets[phase];
	} else if (proc->mode == TEXT_ENGINE_FACE_MOTION_SHIMMY) {
		phase = proc->timer % TEXT_ENGINE_FACE_SHIMMY_PERIOD;
		offset = sTextEngineFaceVibrateOffsets[phase];
	} else {
		phase = proc->timer % TEXT_ENGINE_FACE_JUMP_PERIOD;
		if (phase < TEXT_ENGINE_FACE_JUMP_PERIOD / 2) {
			offset = Interpolate(
				INTERPOLATE_SQUARE,
				0,
				TEXT_ENGINE_FACE_JUMP_HEIGHT,
				phase,
				TEXT_ENGINE_FACE_JUMP_PERIOD / 2
			);
		} else {
			offset = Interpolate(
				INTERPOLATE_RSQUARE,
				TEXT_ENGINE_FACE_JUMP_HEIGHT,
				0,
				phase - TEXT_ENGINE_FACE_JUMP_PERIOD / 2,
				TEXT_ENGINE_FACE_JUMP_PERIOD / 2
			);
		}
	}

	if (proc->mode == TEXT_ENGINE_FACE_MOTION_SHIMMY)
		face->xPos = truePosition - offset;
	else
		face->yPos = truePosition - offset;

	proc->offset = offset;
}

static void TextEngine_StartFaceMotion(struct FaceProc *face, u8 mode)
{
	struct TextEngineFaceJumpProc *jump;

	if (!face)
		return;

	jump = TextEngine_FindFaceJumpProc(face);
	if (jump) {
		if (jump->mode == mode)
			return;

		Proc_End(jump);
	}

	jump = (struct TextEngineFaceJumpProc *)Proc_Start(
		gProcScr_TextEngineFaceJump,
		face
	);
	if (!jump)
		return;

	jump->face = face;
	jump->timer = 0;
	jump->offset = 0;
	jump->mode = mode;
}

static void TextEngine_StartFaceJump(struct FaceProc *face)
{
	TextEngine_StartFaceMotion(face, TEXT_ENGINE_FACE_MOTION_JUMP);
}

static void TextEngine_StartFaceVibrate(struct FaceProc *face)
{
	TextEngine_StartFaceMotion(face, TEXT_ENGINE_FACE_MOTION_VIBRATE);
}

static void TextEngine_StartFaceShimmy(struct FaceProc *face)
{
	TextEngine_StartFaceMotion(face, TEXT_ENGINE_FACE_MOTION_SHIMMY);
}

static void TextEngine_StopFaceMotion(struct FaceProc *face, u8 mode)
{
	struct TextEngineFaceJumpProc *jump = TextEngine_FindFaceJumpProc(face);

	if (jump && jump->mode == mode)
		Proc_End(jump);
}

static void TextEngine_StopFaceJump(struct FaceProc *face)
{
	TextEngine_StopFaceMotion(face, TEXT_ENGINE_FACE_MOTION_JUMP);
}

static void TextEngine_StopFaceVibrate(struct FaceProc *face)
{
	TextEngine_StopFaceMotion(face, TEXT_ENGINE_FACE_MOTION_VIBRATE);
}

static void TextEngine_StopFaceShimmy(struct FaceProc *face)
{
	TextEngine_StopFaceMotion(face, TEXT_ENGINE_FACE_MOTION_SHIMMY);
}

static const u8 sTextEngineAshObjWidth[4][4] = {
	{ 8, 16, 32, 64 },
	{ 16, 32, 32, 64 },
	{ 8, 8, 16, 32 },
	{ 8, 8, 8, 8 },
};

static const u8 sTextEngineAshObjHeight[4][4] = {
	{ 8, 16, 32, 64 },
	{ 8, 8, 16, 32 },
	{ 16, 32, 32, 64 },
	{ 8, 8, 8, 8 },
};

static int TextEngineAsh_OamX(u16 oam1)
{
	int x = oam1 & 0x1FF;

	if (x & 0x100)
		x -= 0x200;

	return x;
}

static int TextEngineAsh_OamY(u16 oam0)
{
	int y = oam0 & 0xFF;

	if (y >= 0x80)
		y -= 0x100;

	return y;
}

static const u16 *TextEngineAsh_GetSprite(int w, int h)
{
	if (w == 8 && h == 8)
		return gObject_8x8;
	if (w == 16 && h == 16)
		return gObject_16x16;
	if (w == 32 && h == 32)
		return gObject_32x32;
	if (w == 64 && h == 64)
		return gObject_64x64;
	if (w == 16 && h == 8)
		return gObject_16x8;
	if (w == 32 && h == 8)
		return gObject_32x8;
	if (w == 32 && h == 16)
		return gObject_32x16;
	if (w == 64 && h == 32)
		return gObject_64x32;
	if (w == 8 && h == 16)
		return gObject_8x16;
	if (w == 8 && h == 32)
		return gObject_8x32;
	if (w == 16 && h == 32)
		return gObject_16x32;
	if (w == 32 && h == 64)
		return gObject_32x64;

	return gObject_16x16;
}

static int TextEngineAsh_CountPieces(const u16 *sprite, int step)
{
	int i;
	int n = 0;
	int count;
	const u16 *obj;

	if (sprite == NULL || step <= 0)
		return 0;

	count = sprite[0];
	obj = sprite + 1;

	for (i = 0; i < count; i++, obj += 3) {
		int shape = obj[0] >> 14;
		int size = obj[1] >> 14;
		int w = sTextEngineAshObjWidth[shape][size];
		int h = sTextEngineAshObjHeight[shape][size];
		int tx;
		int ty;

		for (ty = 0; ty < h; ty += step) {
			for (tx = 0; tx < w; tx += step)
				n++;
		}
	}

	return n;
}

static u32 TextEngineAsh_NextRnd(u32 *seed)
{
	*seed = *seed * 1103515245 + 12345;
	return *seed;
}

static void TextEngineAsh_HideLiveFace(struct FaceProc *face)
{
	if (face->unk_44 != NULL) {
		Proc_End(face->unk_44);
		face->unk_44 = NULL;
	}

	if (face->pBlinkProc != NULL) {
		Proc_End(face->pBlinkProc);
		face->pBlinkProc = NULL;
	}

	/* Do not call SetFaceDisplayBits — that rebuilds the live mug OAM. */
	face->displayBits |= FACE_DISP_HIDDEN;
}

static u16 TextEngineAsh_PackBldcnt(void)
{
	const struct BlendCnt *b = &gLCDControlBuffer.bldcnt;

	return (u16)(
		(b->target1_bg0_on)
		| (b->target1_bg1_on << 1)
		| (b->target1_bg2_on << 2)
		| (b->target1_bg3_on << 3)
		| (b->target1_obj_on << 4)
		| (b->target1_bd_on << 5)
		| (b->effect << 6)
		| (b->target2_bg0_on << 8)
		| (b->target2_bg1_on << 9)
		| (b->target2_bg2_on << 10)
		| (b->target2_bg3_on << 11)
		| (b->target2_obj_on << 12)
		| (b->target2_bd_on << 13)
	);
}

static void TextEngineAsh_UnpackBldcnt(u16 value)
{
	struct BlendCnt *b = &gLCDControlBuffer.bldcnt;

	b->target1_bg0_on = (value >> 0) & 1;
	b->target1_bg1_on = (value >> 1) & 1;
	b->target1_bg2_on = (value >> 2) & 1;
	b->target1_bg3_on = (value >> 3) & 1;
	b->target1_obj_on = (value >> 4) & 1;
	b->target1_bd_on = (value >> 5) & 1;
	b->effect = (value >> 6) & 3;
	b->target2_bg0_on = (value >> 8) & 1;
	b->target2_bg1_on = (value >> 9) & 1;
	b->target2_bg2_on = (value >> 10) & 1;
	b->target2_bg3_on = (value >> 11) & 1;
	b->target2_obj_on = (value >> 12) & 1;
	b->target2_bd_on = (value >> 13) & 1;
}

static void TextEngineAsh_RestoreBlend(struct TextEngineAshDissolveProc *proc)
{
	TextEngineAsh_UnpackBldcnt(proc->savedBldcnt);
	gLCDControlBuffer.blendCoeffA = proc->savedCoeffA;
	gLCDControlBuffer.blendCoeffB = proc->savedCoeffB;
	gLCDControlBuffer.blendY = proc->savedBlendY;
	gLCDControlBuffer.mosaic = proc->savedMosaic;
	gLCDControlBuffer.wincnt.wout_enableBlend = proc->savedWoutBlend;
}

static void TextEngineAsh_ApplyFx(struct TextEngineAshDissolveProc *proc)
{
	int mosaic = 0;
	int eva;
	int t = proc->timer;

	if (t > TEXT_ENGINE_ASH_HOLD) {
		mosaic = Interpolate(
			INTERPOLATE_LINEAR,
			0,
			TEXT_ENGINE_ASH_MOSAIC_MAX,
			t - TEXT_ENGINE_ASH_HOLD,
			TEXT_ENGINE_ASH_DURATION - TEXT_ENGINE_ASH_HOLD
		);
	}

	gLCDControlBuffer.mosaic =
		(proc->savedMosaic & 0x00FF) | (mosaic << 8) | (mosaic << 12);

	if (t < TEXT_ENGINE_ASH_FADE_START)
		return;

	eva = Interpolate(
		INTERPOLATE_LINEAR,
		16,
		0,
		t - TEXT_ENGINE_ASH_FADE_START,
		TEXT_ENGINE_ASH_DURATION - TEXT_ENGINE_ASH_FADE_START
	);
	if (eva < 0)
		eva = 0;

	SetBlendConfig(BLEND_EFFECT_ALPHA, eva, 16 - eva, 0);
	SetBlendTargetA(0, 0, 0, 0, 0);
	SetBlendTargetB(1, 1, 1, 1, 0);
	SetBlendBackdropB(1);
	gLCDControlBuffer.wincnt.wout_enableBlend = 1;
}

static void TextEngineAsh_DrawChips(struct TextEngineAshDissolveProc *proc)
{
	struct FaceProc *face = proc->face;
	const u16 *sprite;
	const u16 *obj;
	int count;
	int i;
	int t = proc->timer;
	int step = proc->step;
	int mosaic = (gLCDControlBuffer.mosaic >> 8) & 0xF;
	u32 seed;

	if (face == NULL || face->sprite == NULL || step <= 0)
		return;

	sprite = face->sprite;
	count = sprite[0];
	obj = sprite + 1;
	seed = proc->seed;

	for (i = 0; i < count; i++, obj += 3) {
		int shape = obj[0] >> 14;
		int size = obj[1] >> 14;
		int hflip = (obj[1] & OAM1_HFLIP) != 0;
		int w = sTextEngineAshObjWidth[shape][size];
		int h = sTextEngineAshObjHeight[shape][size];
		int objX = TextEngineAsh_OamX(obj[1]);
		int objY = TextEngineAsh_OamY(obj[0]);
		int objChr = obj[2] & 0x3FF;
		int tx;
		int ty;

		for (ty = 0; ty < h; ty += step) {
			int ph = h - ty;

			if (ph > step)
				ph = step;

			for (tx = 0; tx < w; tx += step) {
				int pw = w - tx;
				int srcTx;
				int chrOff;
				int x;
				int y;
				int delay;
				int vx;
				int vy;
				int moving;
				int oam0;
				int oam1;
				int oam2;
				u32 rnd;
				const u16 *piece;

				if (pw > step)
					pw = step;

				rnd = TextEngineAsh_NextRnd(&seed);
				delay = (rnd >> 16) & 7;
				vx = ((int)((rnd >> 8) & 7)) - 3;
				vy = -1 - ((int)((rnd >> 4) & 3));

				srcTx = hflip ? (w - tx - pw) : tx;
				chrOff = objChr + (ty / 8) * 32 + (srcTx / 8);
				x = proc->originX + objX + tx;
				y = proc->originY + objY + ty;
				moving = (t > TEXT_ENGINE_ASH_HOLD + delay);
				if (moving) {
					int frames = t - TEXT_ENGINE_ASH_HOLD - delay;

					x += vx * frames;
					y += vy * frames;
				}

				if (x <= -pw || x >= DISPLAY_WIDTH)
					continue;
				if (y <= -ph || y >= DISPLAY_HEIGHT)
					continue;

				oam0 = OAM0_Y(y);
				if (moving && mosaic > 0)
					oam0 |= OAM0_MOSAIC;
				if (t >= TEXT_ENGINE_ASH_FADE_START)
					oam0 |= OAM0_BLEND;

				oam1 = OAM1_X(x);
				if (hflip)
					oam1 |= OAM1_HFLIP;

				oam2 = (face->oam2 & 0xFC00) | ((face->oam2 + chrOff) & 0x3FF);
				piece = TextEngineAsh_GetSprite(pw, ph);
				TextEngine_PutFaceSprite(proc->layer, oam1, oam0, piece, oam2);

				if (proc->overlayEnabled)
					TextEngine_PutFaceSprite(
						proc->layer,
						oam1,
						oam0,
						piece,
						(proc->overlayOam2 & 0xFC00) |
							((proc->overlayOam2 + chrOff) & 0x3FF)
					);
			}
		}
	}
}

static void TextEngineAshDissolve_OnIdle(struct TextEngineAshDissolveProc *proc)
{
	if (proc->face == NULL ||
		gFaces[proc->face->faceSlot] != proc->face) {
		Proc_Break(proc);
		return;
	}

	TextEngineAsh_ApplyFx(proc);
	TextEngineAsh_DrawChips(proc);

	proc->timer++;
	if (proc->timer >= TEXT_ENGINE_ASH_DURATION)
		Proc_Break(proc);
}

static void TextEngineAshDissolve_OnEnd(struct TextEngineAshDissolveProc *proc)
{
	TextEngineAsh_RestoreBlend(proc);

	if (proc->face != NULL &&
		gFaces[proc->face->faceSlot] == proc->face)
		EndFace(proc->face);

	proc->face = NULL;
}

static void TextEngine_StartAshDissolve(struct FaceProc *face)
{
	struct TextEngineAshDissolveProc *ash;
	struct TextEngineFaceJumpProc *jump;
	s32 overlay;
	int originX;
	int originY;
	int count;
	u8 step = 16;

	if (face == NULL)
		return;

	Proc_EndEach(gProcScr_TextEngineAshDissolve);

	originX = face->xPos;
	originY = face->yPos;
	jump = TextEngine_FindFaceJumpProc(face);
	if (jump)
		Proc_End(jump);

	HalfBody_OnTalkFaceClear(face);
	TextEngineAsh_HideLiveFace(face);

	count = TextEngineAsh_CountPieces(face->sprite, 16);
	if (count > TEXT_ENGINE_ASH_OAM_BUDGET)
		step = 32;

	count = TextEngineAsh_CountPieces(face->sprite, step);
	if (count == 0) {
		StartFaceFadeOut(face);
		return;
	}

	ash = (struct TextEngineAshDissolveProc *)Proc_Start(
		gProcScr_TextEngineAshDissolve,
		PROC_TREE_5
	);
	if (!ash) {
		StartFaceFadeOut(face);
		return;
	}

	ash->face = face;
	ash->timer = 0;
	ash->layer = face->spriteLayer;
	ash->step = step;
	ash->originX = originX;
	ash->originY = originY;
	ash->seed = AdvanceGetLCGRNValue();
	ash->savedMosaic = gLCDControlBuffer.mosaic;
	ash->savedBldcnt = TextEngineAsh_PackBldcnt();
	ash->savedCoeffA = gLCDControlBuffer.blendCoeffA;
	ash->savedCoeffB = gLCDControlBuffer.blendCoeffB;
	ash->savedBlendY = gLCDControlBuffer.blendY;
	ash->savedWoutBlend = gLCDControlBuffer.wincnt.wout_enableBlend;

	overlay = Portrait32_GetOverlayOam2(face);
	if (overlay >= 0 && count <= TEXT_ENGINE_ASH_OVERLAY_CHIP_LIMIT) {
		ash->overlayEnabled = 1;
		ash->overlayOam2 = (u16)overlay;
	} else {
		ash->overlayEnabled = 0;
		ash->overlayOam2 = 0;
	}
}

static void TextEngine_CleanupAshDissolve(void)
{
	Proc_EndEach(gProcScr_TextEngineAshDissolve);
}

static void TextEnginePrintFx_Apply(struct TextEnginePrintFxProc *proc)
{
	s16 x = proc->baseX;
	s16 y = proc->baseY;

	if (proc->shakeTimer >= 0 &&
		proc->shakeTimer < TEXT_ENGINE_PRINT_SHAKE_FRAMES) {
		const s8 *offset = sTextEnginePrintShakeOffsets[proc->shakeTimer];

		x += offset[0];
		y += offset[1];
	}

	BG_SetPosition(BG_0, x, y);
}

static void TextEnginePrintFx_OnEnd(struct TextEnginePrintFxProc *proc)
{
	BG_SetPosition(BG_0, proc->baseX, proc->baseY);
}

static void TextEnginePrintFx_OnIdle(struct TextEnginePrintFxProc *proc)
{
	TextEnginePrintFx_Apply(proc);

	if (proc->shakeTimer >= 0)
		proc->shakeTimer++;

	if (proc->shakeTimer < 0 ||
		proc->shakeTimer >= TEXT_ENGINE_PRINT_SHAKE_FRAMES) {
		Proc_End(proc);
		return;
	}
}

static int TextEngine_TearLineOffset(u32 clock, int line)
{
	int split = (int)((clock * TEXT_ENGINE_TEAR_SPEED) % DISPLAY_HEIGHT);
	int shift = TEXT_ENGINE_TEAR_SHIFT;

	/*
	 * Classic vsync tear: everything below a rolling split is shifted on
	 * the horizontal axis. Flip direction so it does not always wrap the
	 * same way.
	 */
	if ((clock / DISPLAY_HEIGHT) & 1)
		shift = -shift;

	if (line >= split)
		return shift;

	return 0;
}

static u32 TextEngine_FxHash(u32 a, u32 b)
{
	u32 x = a * 1664525u + b * 1013904223u;

	x ^= x >> 16;
	x *= 0x7feb352du;
	x ^= x >> 15;
	return x;
}

static int TextEngine_StaticLineIntensity(u32 clock, int line)
{
	u32 held = clock >> 1;
	u32 h = TextEngine_FxHash(clock ^ (u32)line, 0x51u);
	int i;

	/* Occasional stray sparks anywhere on screen. */
	if ((h & 0x3F) < 3)
		return 5 + (int)((h >> 8) & 7);

	for (i = 0; i < TEXT_ENGINE_STATIC_CLUSTERS; i++) {
		u32 cluster = TextEngine_FxHash(held, (u32)(i + 1) * 0x9E3779B9u);
		int height = 6 + (int)((cluster >> 16) & 15);
		int y = (int)((cluster >> 8) % (DISPLAY_HEIGHT - height));
		int rel = line - y;
		int density;

		if (rel < 0 || rel >= height)
			continue;

		/* Lower threshold = denser cluster. 2-6 of every 8 lines. */
		density = 2 + (int)((cluster >> 24) & 5);
		if ((int)(h & 7) < density)
			return 6 + (int)((h >> 12) & 7);
	}

	return 0;
}

static void TextEngineScanlineFx_SetBlend(int line, int intensity)
{
	u8 *cell = &sTextEngineWaveOffsets.blend[line >> 1];
	u8 nibble = (u8)intensity & 0xF;

	if (line & 1)
		*cell = (u8)((*cell & 0x0F) | (nibble << 4));
	else
		*cell = (u8)((*cell & 0xF0) | nibble);
}

static void TextEngineScanlineFx_BuildBuffer(void)
{
	struct TextEngineWaveProc *wave =
		(struct TextEngineWaveProc *)Proc_Find(gProcScr_TextEngineWave);
	s8 glitchOn = Proc_Find(gProcScr_TextEngineScreenGlitch) != NULL;
	s8 staticOn = Proc_Find(gProcScr_TextEngineScreenStatic) != NULL;
	u32 clock = GetGameClock();
	int line;

	for (line = 0; line < DISPLAY_HEIGHT; line++) {
		s16 offset = 0;
		int intensity = 0;

		if (!Chatlog_IsVisible()) {
			if (wave)
				offset += (SIN(wave->phase + line * TEXT_ENGINE_WAVE_FREQUENCY)
					* TEXT_ENGINE_WAVE_AMPLITUDE) >> 8;
			if (glitchOn)
				offset += TextEngine_TearLineOffset(clock, line);
			if (staticOn) {
				intensity = TextEngine_StaticLineIntensity(clock, line);
				if (intensity > 15)
					intensity = 15;
			}
		}

		sTextEngineWaveOffsets.x[line] = (s8)offset;
		TextEngineScanlineFx_SetBlend(line, intensity);
	}
}

static ProcPtr TextEngineScanlineFx_FindOwner(ProcPtr exclude)
{
	ProcPtr proc;

	proc = Proc_Find(gProcScr_TextEngineWave);
	if (proc && proc != exclude)
		return proc;

	proc = Proc_Find(gProcScr_TextEngineScreenGlitch);
	if (proc && proc != exclude)
		return proc;

	proc = Proc_Find(gProcScr_TextEngineScreenStatic);
	if (proc && proc != exclude)
		return proc;

	proc = Proc_Find(gProcScr_TextEngineScreenEarthquake);
	if (proc && proc != exclude)
		return proc;

	return NULL;
}

static void TextEngineScanlineFx_AcquireHBlank(void (**outPrev)(void))
{
	if (sHBlankHandler2 == TextEngineWave_OnHBlank) {
		struct TextEngineWaveProc *wave =
			(struct TextEngineWaveProc *)Proc_Find(gProcScr_TextEngineWave);
		struct TextEngineScreenGlitchProc *glitch =
			(struct TextEngineScreenGlitchProc *)Proc_Find(gProcScr_TextEngineScreenGlitch);
		struct TextEngineScreenStaticProc *statik =
			(struct TextEngineScreenStaticProc *)Proc_Find(gProcScr_TextEngineScreenStatic);
		struct TextEngineScreenEarthquakeProc *quake =
			(struct TextEngineScreenEarthquakeProc *)Proc_Find(gProcScr_TextEngineScreenEarthquake);

		if (wave)
			*outPrev = wave->previousHBlankHandler;
		else if (glitch)
			*outPrev = glitch->previousHBlankHandler;
		else if (statik)
			*outPrev = statik->previousHBlankHandler;
		else if (quake)
			*outPrev = quake->previousHBlankHandler;
		else
			*outPrev = NULL;
		return;
	}

	if (!Proc_Find(gProcScr_TextEngineScreenEarthquake)) {
		sTextEngineEarthquakeOffset.x = 0;
		sTextEngineEarthquakeOffset.y = 0;
	}

	*outPrev = sHBlankHandler2;
	TextEngineScanlineFx_BuildBuffer();
	SetSecondaryHBlankHandler(TextEngineWave_OnHBlank);
}

static void TextEngineScanlineFx_ReleaseHBlank(void (*prev)(void), ProcPtr self)
{
	if (TextEngineScanlineFx_FindOwner(self))
		return;

	SetSecondaryHBlankHandler(prev);
	TextEngineWave_ApplyBaseOffsets();
}

static void TextEngine_RestoreBlend(void)
{
	u16 bldcnt;

	CpuCopy16(&gLCDControlBuffer.bldcnt, &bldcnt, sizeof(bldcnt));
	REG_BLDCNT = bldcnt;
	REG_BLDY = gLCDControlBuffer.blendY;
}

static void TextEngineWave_OnHBlank(void)
{
	u16 line = REG_VCOUNT;
	s16 offset;
	int blend;
	u8 cell;

	if (line >= DISPLAY_HEIGHT)
		return;

	offset = sTextEngineWaveOffsets.x[line] + sTextEngineEarthquakeOffset.x;
	REG_BG0HOFS = gLCDControlBuffer.bgoffset[BG_0].x + offset;
	REG_BG1HOFS = gLCDControlBuffer.bgoffset[BG_1].x + offset;
	REG_BG2HOFS = gLCDControlBuffer.bgoffset[BG_2].x + offset;
	REG_BG3HOFS = gLCDControlBuffer.bgoffset[BG_3].x + offset;
	REG_BG0VOFS = gLCDControlBuffer.bgoffset[BG_0].y + sTextEngineEarthquakeOffset.y;
	REG_BG1VOFS = gLCDControlBuffer.bgoffset[BG_1].y + sTextEngineEarthquakeOffset.y;
	REG_BG2VOFS = gLCDControlBuffer.bgoffset[BG_2].y + sTextEngineEarthquakeOffset.y;
	REG_BG3VOFS = gLCDControlBuffer.bgoffset[BG_3].y + sTextEngineEarthquakeOffset.y;

	cell = sTextEngineWaveOffsets.blend[line >> 1];
	blend = (line & 1) ? (cell >> 4) : (cell & 0xF);
	if (blend) {
		REG_WININ |= 0x2020;
		REG_WINOUT |= 0x20;
		REG_BLDCNT = BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 |
			BLDCNT_TGT1_BG3 | BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_LIGHTEN;
		REG_BLDY = blend;
	} else {
		TextEngine_RestoreBlend();
	}
}

static void TextEngineWave_ApplyBaseOffsets(void)
{
	REG_BG0HOFS = gLCDControlBuffer.bgoffset[BG_0].x;
	REG_BG1HOFS = gLCDControlBuffer.bgoffset[BG_1].x;
	REG_BG2HOFS = gLCDControlBuffer.bgoffset[BG_2].x;
	REG_BG3HOFS = gLCDControlBuffer.bgoffset[BG_3].x;
	REG_BG0VOFS = gLCDControlBuffer.bgoffset[BG_0].y;
	REG_BG1VOFS = gLCDControlBuffer.bgoffset[BG_1].y;
	REG_BG2VOFS = gLCDControlBuffer.bgoffset[BG_2].y;
	REG_BG3VOFS = gLCDControlBuffer.bgoffset[BG_3].y;
	TextEngine_RestoreBlend();
}

static void TextEngineWave_OnEnd(struct TextEngineWaveProc *proc)
{
	TextEngineScanlineFx_ReleaseHBlank(proc->previousHBlankHandler, proc);
}

static void TextEngineWave_OnIdle(struct TextEngineWaveProc *proc)
{
	proc->phase += TEXT_ENGINE_WAVE_SPEED;
	TextEngineScanlineFx_BuildBuffer();
}

static void TextEngine_StartWave(void)
{
	struct TextEngineWaveProc *proc;

	if (CheckTalkFlag(TALK_FLAG_SPRITE))
		return;

	proc = (struct TextEngineWaveProc *)Proc_Find(gProcScr_TextEngineWave);
	if (proc)
		return;

	proc = (struct TextEngineWaveProc *)Proc_Start(
		gProcScr_TextEngineWave,
		PROC_TREE_3
	);
	if (!proc)
		return;

	proc->phase = 0;
	proc->activeBuffer = 0;
	proc->unused = 0;
	TextEngineScanlineFx_AcquireHBlank(&proc->previousHBlankHandler);
}

static void TextEngineScreenGlitch_OnEnd(struct TextEngineScreenGlitchProc *proc)
{
	TextEngineScanlineFx_ReleaseHBlank(proc->previousHBlankHandler, proc);
}

static void TextEngineScreenGlitch_OnIdle(ProcPtr proc)
{
	(void)proc;

	/* Wave already rebuilds the shared scanline table when both are active. */
	if (Proc_Find(gProcScr_TextEngineWave))
		return;

	TextEngineScanlineFx_BuildBuffer();
}

void EnableScreenGlitch(void)
{
	struct TextEngineScreenGlitchProc *proc =
		(struct TextEngineScreenGlitchProc *)Proc_Find(gProcScr_TextEngineScreenGlitch);

	if (proc)
		return;

	proc = (struct TextEngineScreenGlitchProc *)Proc_Start(
		gProcScr_TextEngineScreenGlitch,
		PROC_TREE_3
	);
	if (!proc)
		return;

	TextEngineScanlineFx_AcquireHBlank(&proc->previousHBlankHandler);
}

void DisableScreenGlitch(void)
{
	Proc_EndEach(gProcScr_TextEngineScreenGlitch);
}

static void TextEngineScreenStatic_OnEnd(struct TextEngineScreenStaticProc *proc)
{
	TextEngineScanlineFx_ReleaseHBlank(proc->previousHBlankHandler, proc);
}

static void TextEngineScreenStatic_OnIdle(ProcPtr proc)
{
	(void)proc;

	if (Proc_Find(gProcScr_TextEngineWave))
		return;
	if (Proc_Find(gProcScr_TextEngineScreenGlitch))
		return;

	TextEngineScanlineFx_BuildBuffer();
}

void EnableScreenStatic(void)
{
	struct TextEngineScreenStaticProc *proc =
		(struct TextEngineScreenStaticProc *)Proc_Find(gProcScr_TextEngineScreenStatic);

	if (proc)
		return;

	proc = (struct TextEngineScreenStaticProc *)Proc_Start(
		gProcScr_TextEngineScreenStatic,
		PROC_TREE_3
	);
	if (!proc)
		return;

	TextEngineScanlineFx_AcquireHBlank(&proc->previousHBlankHandler);
}

void DisableScreenStatic(void)
{
	Proc_EndEach(gProcScr_TextEngineScreenStatic);
}

static void TextEngine_ClearEarthquakeOffset(void)
{
	sTextEngineEarthquakeOffset.x = 0;
	sTextEngineEarthquakeOffset.y = 0;
}

static void TextEngineScreenEarthquake_OnEnd(struct TextEngineScreenEarthquakeProc *proc)
{
	TextEngine_ClearEarthquakeOffset();
	TextEngineScanlineFx_ReleaseHBlank(proc->previousHBlankHandler, proc);
}

static void TextEngineScreenEarthquake_OnIdle(struct TextEngineScreenEarthquakeProc *proc)
{
	const s8 *offset;
	int index;

	if (Chatlog_IsVisible()) {
		TextEngine_ClearEarthquakeOffset();
		return;
	}

	index = (proc->timer / TEXT_ENGINE_EARTHQUAKE_PERIOD)
		% (int)ARRAY_COUNT(sTextEngineEarthquakeOffsets);
	offset = sTextEngineEarthquakeOffsets[index];
	sTextEngineEarthquakeOffset.x = offset[0];
	sTextEngineEarthquakeOffset.y = offset[1];
	proc->timer++;
}

void EnableScreenEarthquake(void)
{
	struct TextEngineScreenEarthquakeProc *proc =
		(struct TextEngineScreenEarthquakeProc *)Proc_Find(gProcScr_TextEngineScreenEarthquake);

	if (proc)
		return;

	proc = (struct TextEngineScreenEarthquakeProc *)Proc_Start(
		gProcScr_TextEngineScreenEarthquake,
		PROC_TREE_3
	);
	if (!proc)
		return;

	proc->timer = 0;
	TextEngine_ClearEarthquakeOffset();
	PlaySoundEffect(SONG_26A);
	TextEngineScanlineFx_AcquireHBlank(&proc->previousHBlankHandler);
}

void DisableScreenEarthquake(void)
{
	if (Proc_Find(gProcScr_TextEngineScreenEarthquake))
		Sound_FadeOutSE(4);

	Proc_EndEach(gProcScr_TextEngineScreenEarthquake);
}

static void TextEngineImpactFlash_SaveBlend(struct TextEngineImpactFlashProc *proc)
{
	proc->savedBldcnt = TextEngineAsh_PackBldcnt();
	proc->savedCoeffA = gLCDControlBuffer.blendCoeffA;
	proc->savedCoeffB = gLCDControlBuffer.blendCoeffB;
	proc->savedBlendY = gLCDControlBuffer.blendY;
	proc->savedWinBlend =
		gLCDControlBuffer.wincnt.win0_enableBlend |
		(gLCDControlBuffer.wincnt.win1_enableBlend << 1) |
		(gLCDControlBuffer.wincnt.wout_enableBlend << 2) |
		(gLCDControlBuffer.wincnt.wobj_enableBlend << 3);
}

static void TextEngineImpactFlash_RestoreBlend(struct TextEngineImpactFlashProc *proc)
{
	TextEngineAsh_UnpackBldcnt(proc->savedBldcnt);
	gLCDControlBuffer.blendCoeffA = proc->savedCoeffA;
	gLCDControlBuffer.blendCoeffB = proc->savedCoeffB;
	gLCDControlBuffer.blendY = proc->savedBlendY;
	gLCDControlBuffer.wincnt.win0_enableBlend = proc->savedWinBlend & 1;
	gLCDControlBuffer.wincnt.win1_enableBlend = (proc->savedWinBlend >> 1) & 1;
	gLCDControlBuffer.wincnt.wout_enableBlend = (proc->savedWinBlend >> 2) & 1;
	gLCDControlBuffer.wincnt.wobj_enableBlend = (proc->savedWinBlend >> 3) & 1;
	TextEngine_RestoreBlend();
}

static int TextEngineImpactFlash_GetIntensity(int timer)
{
	if (timer < TEXT_ENGINE_IMPACT_FLASH_HOLD)
		return TEXT_ENGINE_IMPACT_FLASH_PEAK;

	return Interpolate(
		INTERPOLATE_LINEAR,
		TEXT_ENGINE_IMPACT_FLASH_PEAK,
		0,
		timer - TEXT_ENGINE_IMPACT_FLASH_HOLD,
		TEXT_ENGINE_IMPACT_FLASH_DURATION - TEXT_ENGINE_IMPACT_FLASH_HOLD
	);
}

static void TextEngineImpactFlash_Apply(struct TextEngineImpactFlashProc *proc)
{
	int intensity = TextEngineImpactFlash_GetIntensity(proc->timer);

	SetBlendBrighten(intensity);
	SetBlendTargetA(1, 1, 1, 1, 1);
	SetBlendBackdropA(1);
	gLCDControlBuffer.wincnt.win0_enableBlend = 1;
	gLCDControlBuffer.wincnt.win1_enableBlend = 1;
	gLCDControlBuffer.wincnt.wout_enableBlend = 1;
	gLCDControlBuffer.wincnt.wobj_enableBlend = 1;
	TextEngine_RestoreBlend();
	/* Talk uses WIN0; force blend inside and outside so the hit is full-screen. */
	REG_WININ |= 0x2020;
	REG_WINOUT |= 0x2020;
}

static void TextEngineImpactFlash_OnEnd(struct TextEngineImpactFlashProc *proc)
{
	TextEngineImpactFlash_RestoreBlend(proc);
}

static void TextEngineImpactFlash_OnIdle(struct TextEngineImpactFlashProc *proc)
{
	if (Chatlog_IsVisible()) {
		TextEngineImpactFlash_RestoreBlend(proc);
		return;
	}

	proc->timer++;
	if (proc->timer >= TEXT_ENGINE_IMPACT_FLASH_DURATION) {
		Proc_End(proc);
		return;
	}

	TextEngineImpactFlash_Apply(proc);
}

void StartScreenImpactFlash(void)
{
	struct TextEngineImpactFlashProc *proc =
		(struct TextEngineImpactFlashProc *)Proc_Find(gProcScr_TextEngineImpactFlash);

	if (proc) {
		proc->timer = 0;
		TextEngineImpactFlash_Apply(proc);
		return;
	}

	proc = (struct TextEngineImpactFlashProc *)Proc_Start(
		gProcScr_TextEngineImpactFlash,
		PROC_TREE_3
	);
	if (!proc)
		return;

	proc->timer = 0;
	TextEngineImpactFlash_SaveBlend(proc);
	TextEngineImpactFlash_Apply(proc);
}

s16 TextEngine_GetStaticOffsetAtY(int y)
{
	s16 offset = 0;

	if (Proc_Find(gProcScr_TextEngineScreenEarthquake))
		offset += sTextEngineEarthquakeOffset.x;

	if (Proc_Find(gProcScr_TextEngineScreenGlitch)) {
		if (y < 0)
			y = 0;
		else if (y >= DISPLAY_HEIGHT)
			y = DISPLAY_HEIGHT - 1;

		offset += sTextEngineWaveOffsets.x[y];
	}

	return offset;
}

s16 TextEngine_GetFxOffsetY(void)
{
	if (!Proc_Find(gProcScr_TextEngineScreenEarthquake))
		return 0;

	return sTextEngineEarthquakeOffset.y;
}

int TextEngine_ApplyStaticOam1(int xOam1, int screenY)
{
	int flags = xOam1 & ~0x1FF;
	int x = xOam1 & 0x1FF;

	x += TextEngine_GetStaticOffsetAtY(screenY);
	return flags | (x & 0x1FF);
}

int TextEngine_ApplyFxOam0(int yOam0)
{
	int flags = yOam0 & ~0xFF;
	int y = yOam0 & 0xFF;

	y += TextEngine_GetFxOffsetY();
	return flags | (y & 0xFF);
}

void TextEngine_PutFaceSprite(int layer, int xOam1, int yOam0, const u16 *object, int oam2)
{
	if (object == NULL)
		return;

	/*
	 * PutSpriteExt stores this object pointer until OAM flush, so the data
	 * must outlive this call. Apply one scanline slip at the face Y instead
	 * of rewriting pieces into a stack buffer.
	 */
	PutSpriteExt(
		layer,
		TextEngine_ApplyStaticOam1(xOam1, yOam0),
		TextEngine_ApplyFxOam0(yOam0),
		object,
		oam2
	);
}

LYN_REPLACE_CHECK(sub_8005FE0);
void sub_8005FE0(struct FaceBlinkProc *proc)
{
	int oam1;
	int oam0;
	struct FaceProc *face = proc->pFaceProc;

	if (!(GetFaceDisplayBits(face) & (FACE_DISP_TALK_1 | FACE_DISP_TALK_2))) {
		int offsetA = (GetFaceDisplayBits(face) & FACE_DISP_SMILE) ? 0 : 24;

		offsetA += 16;
		Register2dChrMove(
			face->pFaceInfo->imgMouth + offsetA * 0x20,
			(void *)(((face->oam2 + 28) & 0x3FF) * 0x20 + 0x06010000),
			4,
			2
		);
	} else {
		proc->unk_32--;
		if (proc->unk_32 < 0) {
			int offsetB = (GetFaceDisplayBits(face) & FACE_DISP_SMILE) ? 0 : 24;

			proc->unk_32 = ((AdvanceGetLCGRNValue() >> 16) & 7) + 1;
			proc->blinkControl = (proc->blinkControl + 1) & 3;

			switch (proc->blinkControl) {
			case 1:
			case 3:
				offsetB += 8;
				break;

			case 2:
				offsetB += 16;
				break;

			case 0:
			default:
				offsetB += 0;
				break;
			}

			Register2dChrMove(
				face->pFaceInfo->imgMouth + offsetB * 0x20,
				(void *)(((face->oam2 + 28) & 0x3FF) * 0x20 + 0x06010000),
				4,
				2
			);
		}
	}

	oam1 = 4 - face->pFaceInfo->xMouth;
	oam1 = (GetFaceDisplayBits(face) & FACE_DISP_FLIPPED) ? oam1 : -oam1;
	oam1 = OAM1_X((oam1 * 8 + face->xPos) - 16);

	if (GetFaceDisplayBits(face) & FACE_DISP_FLIPPED)
		oam1 = oam1 + OAM1_HFLIP;

	if (GetFaceDisplayBits(face) & FACE_DISP_BLEND)
		oam0 = OAM0_BLEND;
	else
		oam0 = 0;

	oam0 += (face->yPos + (face->pFaceInfo->yMouth * 8)) & 0xFF;

	/* Same slip as the parent mug so the mouth does not detach. */
	oam1 = TextEngine_ApplyStaticOam1(oam1, face->yPos);
	oam0 = TextEngine_ApplyFxOam0(oam0);

	PutSpriteExt(
		face->spriteLayer,
		oam1,
		oam0,
		gObject_32x16,
		face->oam2 + 28
	);
}

void sub_8006134(struct FaceBlinkProc *proc, int unk);

LYN_REPLACE_CHECK(sub_8006134);
void sub_8006134(struct FaceBlinkProc *proc, int unk)
{
	int oam1;
	int oam0;
	s8 flag = 0;
	struct FaceProc *face = proc->pFaceProc;

	switch (unk) {
	case 0:
		unk = 88;
		break;

	case 1:
		unk = 24;
		break;

	case 0x80:
		unk = 88;
		flag = 1;
		break;

	case 0x81:
		unk = 24;
		flag = 1;
		break;

	default:
		return;
	}

	oam1 = 4 - face->pFaceInfo->xEyes;
	oam1 = (GetFaceDisplayBits(face) & FACE_DISP_FLIPPED) ? oam1 : -oam1;
	oam1 = ((oam1 * 8 + face->xPos) - 16) & 0x1FF;

	if (GetFaceDisplayBits(face) & FACE_DISP_FLIPPED)
		oam1 = oam1 + 0x1000;

	if (GetFaceDisplayBits(face) & FACE_DISP_BLEND)
		oam0 = OAM0_BLEND;
	else
		oam0 = 0;

	oam0 += (face->yPos + (face->pFaceInfo->yEyes * 8)) & 0xFF;
	oam1 = TextEngine_ApplyStaticOam1(oam1, face->yPos);
	oam0 = TextEngine_ApplyFxOam0(oam0);

	if (flag) {
		if (!(GetFaceDisplayBits(face) & FACE_DISP_FLIPPED))
			oam1 = oam1 + 16;

		PutSpriteExt(
			face->spriteLayer,
			oam1,
			oam0,
			gObject_16x16,
			face->oam2 + unk + 2
		);
	} else {
		PutSpriteExt(
			face->spriteLayer,
			oam1,
			oam0,
			gObject_32x16,
			face->oam2 + unk
		);
	}
}

static struct TextEnginePrintFxProc *TextEngine_EnsurePrintFx(void)
{
	struct TextEnginePrintFxProc *fx =
		(struct TextEnginePrintFxProc *)Proc_Find(gProcScr_TextEnginePrintFx);

	if (fx)
		return fx;

	fx = (struct TextEnginePrintFxProc *)Proc_Start(
		gProcScr_TextEnginePrintFx,
		PROC_TREE_3
	);
	if (!fx)
		return NULL;

	fx->shakeTimer = TEXT_ENGINE_PRINT_FX_INACTIVE;
	fx->unusedBounce = TEXT_ENGINE_PRINT_FX_INACTIVE;
	fx->baseX = gLCDControlBuffer.bgoffset[BG_0].x;
	fx->baseY = gLCDControlBuffer.bgoffset[BG_0].y;
	return fx;
}

static void TextEngine_StartPrintShake(void)
{
	struct TextEnginePrintFxProc *fx = TextEngine_EnsurePrintFx();

	if (!fx)
		return;

	fx->shakeTimer = 0;
	TextEnginePrintFx_Apply(fx);
}

static void TextEngine_StopWave(void)
{
	Proc_EndEach(gProcScr_TextEngineWave);
}

static int TextEngine_GetFloatObjChr(int slot)
{
	return TEXT_ENGINE_FLOAT_OBJ_CHR + slot * TEXT_ENGINE_FLOAT_SLOT_STRIDE;
}

static s8 TextEngine_IsFloatSlotBusy(int slot)
{
	struct ProcFindIterator it;
	struct TextEngineGlyphFloatProc *floatProc;

	Proc_FindBegin(&it, gProcScr_TextEngineGlyphFloat);
	while ((floatProc = (struct TextEngineGlyphFloatProc *)Proc_FindNext(&it)) != NULL) {
		if (floatProc->slot == slot)
			return 1;
	}

	return 0;
}

static int TextEngine_FindFreeFloatSlot(void)
{
	int slot;

	for (slot = 0; slot < TEXT_ENGINE_FLOAT_SLOTS; slot++) {
		if (!TextEngine_IsFloatSlotBusy(slot))
			return slot;
	}

	return -1;
}

static void TextEngine_PrepareFloatPalette(void)
{
	ApplyPalette(Pal_Text, 0x10 + TEXT_ENGINE_FLOAT_OBJ_PAL);
}

static struct Glyph *TextEngine_FindGlyph(u32 unicod)
{
	struct Glyph *glyph;
	int hi = (unicod >> 8) & 0xFF;
	int lo = unicod & 0xFF;

	if (!gActiveFont || unicod >= 0x10000)
		return NULL;

	for (glyph = gActiveFont->glyphs[lo]; glyph != NULL; glyph = glyph->sjisNext) {
		if (glyph->sjisByte1 == hi)
			return glyph;
	}

	return NULL;
}

/*
 * Pack one glyph into a pair of OBJ tiles for an 8x16 sprite (top, bottom).
 * Wide glyphs are clipped to 8px for the float preview; bake still uses the
 * full glyph width.  Keeping every slot at 2 tiles lets us fit in 0x72-0x7F.
 */
static void TextEngine_UploadGlyphToObj(const struct Glyph *glyph, int chr, int color)
{
	u16 *lut = GetColorLut(color);
	u32 *tileTop = (u32 *)OBJ_CHR_ADDR(chr);
	u32 *tileBot = (u32 *)OBJ_CHR_ADDR(chr + 1);
	int row;

	CpuFastFill(0, tileTop, 0x20 * 2);

	for (row = 0; row < 8; row++) {
		u32 bits = glyph->bitmap[row];

		tileTop[row] = lut[bits & 0xFF] | ((u32)lut[(bits >> 8) & 0xFF] << 16);
	}

	for (row = 0; row < 8; row++) {
		u32 bits = glyph->bitmap[row + 8];

		tileBot[row] = lut[bits & 0xFF] | ((u32)lut[(bits >> 8) & 0xFF] << 16);
	}
}

static struct Glyph *TextEngine_ResolveGlyph(const char *ch)
{
	u32 unicod;
	int decodeLen;
	struct Glyph *glyph;

	SetInitTalkTextFont();

	if (DecodeUtf8(ch, &unicod, &decodeLen) != 0) {
		unicod = '?';
		decodeLen = 1;
	}

	glyph = TextEngine_FindGlyph(unicod);
	if (!glyph)
		glyph = TextEngine_FindGlyph('?');

	return glyph;
}

static void TextEngine_ClearFloatSlotVram(int slot)
{
	int chr = TextEngine_GetFloatObjChr(slot);

	CpuFastFill(0, (void *)OBJ_CHR_ADDR(chr), 0x20 * 2);
}

static void TextEngine_BakeFloatedGlyph(struct TextEngineGlyphFloatProc *proc)
{
	int savedCursor;

	SetInitTalkTextFont();
	savedCursor = Text_GetCursor(proc->targetText);
	Text_SetColor(proc->targetText, proc->color);
	Text_SetCursor(proc->targetText, proc->cursorX);
	Text_DrawCharacter(proc->targetText, proc->ch);
	/*
	 * Text_DrawCharacter advances the cursor.  Later glyphs may already have
	 * reserved space past this one, so restore the live cursor afterward.
	 */
	Text_SetCursor(proc->targetText, savedCursor);
}

static void TextEngineGlyphFloat_OnEnd(struct TextEngineGlyphFloatProc *proc)
{
	TextEngine_BakeFloatedGlyph(proc);
	TextEngine_ClearFloatSlotVram(proc->slot);
}

static void TextEngineGlyphFloat_OnIdle(struct TextEngineGlyphFloatProc *proc)
{
	struct Glyph *glyph = proc->glyph;
	int x = proc->screenX;
	int y = proc->destY;
	int oam2;
	int duration = proc->duration;

	/*
	 * Freeze mid-flight glyphs while the log is up: ending would bake them
	 * into the talk font tiles the log is currently borrowing.
	 */
	if (Chatlog_IsVisible())
		return;

	if (duration <= 0)
		duration = TEXT_ENGINE_FLOAT_DURATION;

	if (proc->timer >= duration) {
		Proc_End(proc);
		return;
	}

	switch (proc->mode) {
	case TEXT_ENGINE_LETTER_FX_DRIP:
		y = Interpolate(
			INTERPOLATE_SQUARE,
			proc->destY - TEXT_ENGINE_DRIP_LIFT,
			proc->destY,
			proc->timer,
			duration
		);
		break;

	case TEXT_ENGINE_LETTER_FX_WAVY: {
		int amp = Interpolate(
			INTERPOLATE_LINEAR,
			TEXT_ENGINE_WAVY_AMPLITUDE,
			0,
			proc->timer,
			duration
		);
		int angle = (proc->screenX * 8 + proc->timer * 24) & 0xFF;

		x += (SIN(angle) * amp) >> 12;
		y += (COS(angle) * amp) >> 13;
		break;
	}

	case TEXT_ENGINE_LETTER_FX_SCRAMBLE:
		if (proc->timer + TEXT_ENGINE_SCRAMBLE_LOCK < duration) {
			struct Glyph *scramble;

			scramble = TextEngine_FindGlyph(
				'A' + ((proc->screenX + proc->timer * 17) % 26)
			);
			if (scramble)
				glyph = scramble;
			x += (proc->timer & 1);
		}
		break;

	case TEXT_ENGINE_LETTER_FX_GHOST:
		break;

	case TEXT_ENGINE_LETTER_FX_BOUNCE:
	default:
		y = Interpolate(
			INTERPOLATE_RCUBIC,
			proc->destY - TEXT_ENGINE_FLOAT_LIFT,
			proc->destY,
			proc->timer,
			duration
		);
		break;
	}

	if (glyph) {
		TextEngine_UploadGlyphToObj(
			glyph,
			TextEngine_GetFloatObjChr(proc->slot),
			proc->color
		);
	}

	oam2 = OAM2_CHR(TextEngine_GetFloatObjChr(proc->slot))
		| OAM2_PAL(TEXT_ENGINE_FLOAT_OBJ_PAL)
		| OAM2_LAYER(0);

	PutSpriteExt(
		4,
		TextEngine_ApplyStaticOam1(OAM1_X(x), y),
		TextEngine_ApplyFxOam0(OAM0_Y(y)),
		gObject_8x16,
		oam2
	);

	if (proc->mode == TEXT_ENGINE_LETTER_FX_GHOST) {
		int echo = Interpolate(
			INTERPOLATE_LINEAR,
			2,
			TEXT_ENGINE_GHOST_ECHO,
			proc->timer,
			duration
		);

		PutSpriteExt(
			4,
			TextEngine_ApplyStaticOam1(OAM1_X(x + echo), y - echo / 2),
			TextEngine_ApplyFxOam0(OAM0_Y(y - echo / 2)),
			gObject_8x16,
			oam2
		);
	}

	proc->timer++;
}

s8 TextEngine_TryStartGlyphFloat(struct Text *text, const char **str)
{
	struct TalkState *state = sTextEngineState;
	struct TextEngineGlyphFloatProc *floatProc;
	struct Glyph *glyph;
	u32 width;
	const char *next;
	int slot;
	int len;
	int i;
	int cursorX;

	if (!*TextEngine_GetLetterFxMode())
		return 0;

	if (!text || !str || !*str)
		return 0;

	if (state->instantScroll || CheckTalkFlag(TALK_FLAG_SPRITE))
		return 0;

	SetInitTalkTextFont();

	slot = TextEngine_FindFreeFloatSlot();
	if (slot < 0)
		return 0;

	next = GetCharTextLen(*str, &width);
	if (!next || next == *str || width == 0)
		return 0;

	len = next - *str;
	if (len <= 0 || len > 4)
		return 0;

	cursorX = Text_GetCursor(text);
	floatProc = (struct TextEngineGlyphFloatProc *)Proc_Start(
		gProcScr_TextEngineGlyphFloat,
		PROC_TREE_3
	);
	if (!floatProc)
		return 0;

	for (i = 0; i < len; i++)
		floatProc->ch[i] = (*str)[i];
	floatProc->ch[len] = 0;

	glyph = TextEngine_ResolveGlyph(floatProc->ch);
	if (!glyph) {
		Proc_End(floatProc);
		return 0;
	}

	TextEngine_PrepareFloatPalette();

	floatProc->targetText = text;
	floatProc->glyph = glyph;
	floatProc->cursorX = cursorX;
	floatProc->screenX = state->xText * 8 + cursorX;
	/* Match vanilla talk OBJ anchors (see wait-bubble placement). */
	floatProc->destY = state->yText * 8 + state->lineActive * 16 + 4;
	floatProc->timer = 0;
	floatProc->slot = slot;
	floatProc->color = state->printColor;
	floatProc->width = width;
	floatProc->mode = *TextEngine_GetLetterFxMode();
	switch (floatProc->mode) {
	case TEXT_ENGINE_LETTER_FX_DRIP:
		floatProc->duration = 18;
		break;
	case TEXT_ENGINE_LETTER_FX_WAVY:
		floatProc->duration = 16;
		break;
	case TEXT_ENGINE_LETTER_FX_SCRAMBLE:
		floatProc->duration = 14;
		break;
	default:
		floatProc->duration = TEXT_ENGINE_FLOAT_DURATION;
		break;
	}

	TextEngine_UploadGlyphToObj(
		glyph,
		TextEngine_GetFloatObjChr(slot),
		floatProc->color
	);
	SetInitTalkTextFont();
	Text_SetCursor(text, cursorX + width);
	*str = next;
	return 1;
}

void TextEngine_OnCharacterPrinted(void)
{
	struct TalkState *state = sTextEngineState;

	if (!*TextEngine_GetShakePrintFlag())
		return;

	/* Instant scroll dumps many glyphs in one frame; skip the punch noise. */
	if (state->instantScroll)
		return;

	if (CheckTalkFlag(TALK_FLAG_SPRITE))
		return;

	TextEngine_StartPrintShake();
}

void TextEngine_PlayTextBoop(const char *text)
{
	struct MusicPlayerInfo *mplayInfo = gMPlayTable[3].info;
	u8 pitch = TextEngine_GetCurrentSpeakerAttributes()[TEXT_ENGINE_ATTR_BOOP_PITCH];

	/*
	 * The original hook skipped control-code bytes before starting a boop.
	 * The caller supplies the character pointer from before Text_DrawCharacter
	 * advances the dialogue string.
	 */
	if (!text || ((u8)*text & 0x80))
		return;

	if (gPlaySt.config.disableSoundEffects)
		return;

	if (pitch >= TEXT_ENGINE_BOOP_PITCH_COUNT || !mplayInfo)
		return;

	MPlayStart(mplayInfo, (struct SongHeader *)&TextBoopTable[pitch]);
}

void UpdateFontGlyphSet(int font)
{
	gActiveFont->glyphs = FontGlyphsPointerTable[font];
}

void ChangeTextColorID(int colorGroup)
{
	struct TalkState *state = sTextEngineState;
	int line;

	for (line = 0; line < state->lines; line++)
		Text_SetColor(TextEngine_GetLineText(state, line), colorGroup);

	state->printColor = colorGroup;
}

void UpdateTextBoxBgPalette(int palette)
{
	CopyToPaletteBuffer(
		(const u8 *)TextBoxBgPaletteTable + palette * 0x20,
		0x60,
		0x20
	);
}

static void TextEngine_SetDefaultFaceAttributes(struct FaceProc *face)
{
	TextEngine_SetFaceAttribute(face, TEXT_ENGINE_ATTR_FONT, 0);
	TextEngine_SetFaceAttribute(face, TEXT_ENGINE_ATTR_COLOR_GROUP, 1);
	TextEngine_SetFaceAttribute(face, TEXT_ENGINE_ATTR_BOX_PALETTE, 0);
	TextEngine_SetFaceAttribute(face, TEXT_ENGINE_ATTR_BOX_TYPE, 0);
	TextEngine_SetFaceAttribute(face, TEXT_ENGINE_ATTR_BOOP_PITCH, 12);
}

static void TextEngine_UpdateAttributesFromFace(void)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	u8 *current = TextEngine_GetCurrentSpeakerAttributes();
	u8 *faceAttributes = TextEngine_GetFaceAttributes(face);

	if (!faceAttributes)
		return;

	if (current[TEXT_ENGINE_ATTR_FONT] != faceAttributes[TEXT_ENGINE_ATTR_FONT]) {
		current[TEXT_ENGINE_ATTR_FONT] = faceAttributes[TEXT_ENGINE_ATTR_FONT];
		UpdateFontGlyphSet(current[TEXT_ENGINE_ATTR_FONT]);
	}

	if (current[TEXT_ENGINE_ATTR_COLOR_GROUP] != faceAttributes[TEXT_ENGINE_ATTR_COLOR_GROUP]) {
		current[TEXT_ENGINE_ATTR_COLOR_GROUP] = faceAttributes[TEXT_ENGINE_ATTR_COLOR_GROUP];
		ChangeTextColorID(current[TEXT_ENGINE_ATTR_COLOR_GROUP]);
	}

	if (current[TEXT_ENGINE_ATTR_BOX_PALETTE] != faceAttributes[TEXT_ENGINE_ATTR_BOX_PALETTE]) {
		current[TEXT_ENGINE_ATTR_BOX_PALETTE] = faceAttributes[TEXT_ENGINE_ATTR_BOX_PALETTE];
		UpdateTextBoxBgPalette(current[TEXT_ENGINE_ATTR_BOX_PALETTE]);
	}

	current[TEXT_ENGINE_ATTR_BOX_TYPE] = faceAttributes[TEXT_ENGINE_ATTR_BOX_TYPE];
	current[TEXT_ENGINE_ATTR_BOOP_PITCH] = faceAttributes[TEXT_ENGINE_ATTR_BOOP_PITCH];
}

void Copy_Text_Attributes(ProcPtr proc)
{
	(void)proc;
	TextEngine_UpdateAttributesFromFace();
}

/*
 * Full replacement for Talk_OnInit.  Seeds the remappable face X table and
 * default speaker attributes after the vanilla skip-listener startup.
 */
LYN_REPLACE_CHECK(Talk_OnInit);
void Talk_OnInit_C(void)
{
	static const u8 defaultFaceXPositions[] = {
		0x03, 0x06, 0x09, 0x15, 0x18, 0x1B, 0xF8, 0x26,
	};
	struct TalkState *state = sTextEngineState;
	volatile u8 *positionTable = (volatile u8 *)state + 0x50;
	u8 *current = TextEngine_GetCurrentSpeakerAttributes();
	int position;

	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearFaceNameTextIds();
	sTextEngineNameplateState.enabled = 0;

	if (!CheckTalkFlag(TALK_FLAG_SPRITE)) {
		LoadObjUIGfx();
		BG_SetPosition(BG_0, 0, 0);
		BG_SetPosition(BG_1, 0, 0);
	}

	Proc_Start(gProcScr_TalkSkipListener, PROC_TREE_3);
	Chatlog_StartSession();

	for (position = 0; position < (int)sizeof(defaultFaceXPositions); position++)
		positionTable[position] = defaultFaceXPositions[position];

	current[TEXT_ENGINE_ATTR_FONT] = 0;
	current[TEXT_ENGINE_ATTR_COLOR_GROUP] = 1;
	current[TEXT_ENGINE_ATTR_BOX_PALETTE] = 0;
	current[TEXT_ENGINE_ATTR_BOX_TYPE] = 0;
	current[TEXT_ENGINE_ATTR_BOOP_PITCH] = 13;
	TextEngine_RunCommandCleanup();
	TextEngine_PrepareFloatPalette();
}

/*
 * Clear the BG0 nameplate when the talk proc ends.  The vanilla callback only
 * ends the skip-listener and face-shift helper procs.
 */
LYN_REPLACE_CHECK(Talk_OnEnd);
void Talk_OnEnd_C(void)
{
	if (!CheckTalkFlag(TALK_FLAG_SPRITE))
		BG_SetPosition(BG_1, 0, 0);

	Chatlog_EndSession();
	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearFaceNameTextIds();
	Proc_EndEach(gProcScr_TalkSkipListener);
	Proc_EndEach(gProcScr_TalkShiftClearAll);
	Proc_EndEach(gProcScr_TextEngineAshDissolve);
	Proc_EndEach(gProcScr_TextEngineImpactFlash);
}

LYN_REPLACE_CHECK(InitTalk);
void InitTalk_C(int fontTileOffset, int lines, s8 loadBoxGraphics)
{
	struct Font *font = (struct Font *)0x030000F0;
	struct Text *texts = (struct Text *)0x030000D0;
	void *fontVram;
	int line;

	fontVram = (void *)(
		VRAM
		+ ((fontTileOffset & 0x3FF) << 5)
		+ GetBackgroundTileDataOffset(BG_0)
	);

	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearFaceNameTextIds();
	InitTextFont(font, fontVram, fontTileOffset, 2);
	SetInitTalkTextFont();
	sTextEngineState->lines = lines;

	for (line = 0; line < 3; line++) {
		InitText(&texts[line], 30);
		Text_SetColor(&texts[line], 1);
	}
	InitText(&sTextEngineNameplateState.text, TEXT_ENGINE_NAMEPLATE_WIDTH);
	Text_SetColor(
		&sTextEngineNameplateState.text,
		TEXT_ENGINE_NAMEPLATE_COLOR
	);

	if (loadBoxGraphics) {
		Decompress(
			(const void *)0x089E8238,
			(void *)(VRAM + 0x200 + GetBackgroundTileDataOffset(BG_1))
		);
		CopyToPaletteBuffer((const void *)0x089E84D4, 0x60, 0x20);
	}

	ClearTalkFaceRefs();
}

static u16 *TextEngine_GetTalkClearTilemap(const struct TalkState *state)
{
	/*
	 * The scroll buffer is the block of rows directly below the visible
	 * text, so its origin follows the box height instead of assuming three
	 * lines.
	 */
	return gBG0TilemapBuffer + TILEMAP_INDEX(
		state->xText,
		state->yText + state->lines * 2
	);
}

static void TextEngine_ClearTalkTilemap(const struct TalkState *state)
{
	TileMap_FillRect(
		TextEngine_GetTalkClearTilemap(state),
		state->activeWidth - 2,
		state->lines * 2,
		0
	);
	TalkBgSync(1);
}

LYN_REPLACE_CHECK(TalkShiftClearAll_OnInit);
void TalkShiftClearAll_OnInit_C(struct Proc *proc)
{
	struct TalkState *state = sTextEngineState;
	int nextLine;

	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearTalkTilemap(state);
	proc->unk64 = 0;

	if (!state->lineActive) {
		proc->unk66 = 16;
		return;
	}

	nextLine = state->lineActive + 1;
	if (nextLine >= state->lines)
		nextLine = state->lines;

	proc->unk66 = nextLine * 16;
}

LYN_REPLACE_CHECK(TalkShiftClear_OnInit);
void TalkShiftClear_OnInit_C(struct Proc *proc)
{
	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearTalkTilemap(sTextEngineState);
	proc->unk64 = 0;
}

LYN_REPLACE_CHECK(GetTalkFaceHPos);
int GetTalkFaceHPos(int talkFace)
{
	if (IsBattleDeamonActive())
		/* 6/24 = 48px/192px: 96x80 faces sit fully on-screen. */
		return talkFace <= 2 ? 6 : 24;

	return ((s8 *)((u8 *)sTextEngineState + 0x50))[talkFace];
}

LYN_REPLACE_CHECK(TalkFaceMove_OnInit);
void TalkFaceMove_OnInitOverride(struct Proc *proc)
{
	int distance;

	/*
	 * Variable-speed moves write their duration before the proc starts.
	 * Preserve that value instead of recalculating the vanilla duration.
	 */
	if (proc->unk5C)
		return;

	proc->unk58 = 0;
	distance = GetTalkFaceHPos(proc->unk66) * 8 - proc->unk68;

	if (distance < 0)
		distance = -distance;

	proc->unk5C = distance > 24 ? 32 : 16;
}

/*
 * Full replacement for StartTalkFaceMove.  Returns the move proc so variable-
 * speed callers can write unk5C after creation.
 */
LYN_REPLACE_CHECK(StartTalkFaceMove);
struct Proc *StartTalkFaceMove_C(int talkFaceFrom, int talkFaceTo, s8 isSwap)
{
	struct Proc *proc;
	int slot = GetFaceIdByXPos(GetTalkFaceHPos(talkFaceFrom) * 8);

	if (slot == -1)
		return NULL;

	proc = (struct Proc *)Proc_Start(gProcScr_TalkFaceMove, gFaces[slot]);
	proc->unk64 = slot;
	proc->unk66 = talkFaceTo;
	proc->unk68 = gFaces[slot]->xPos;
	proc->unk6A = isSwap;
	return proc;
}

LYN_REPLACE_CHECK(StartTalkOpen);
void StartTalkOpen_C(int talkFace, ProcPtr parent)
{
	struct TalkState *state = sTextEngineState;
	struct Proc *proc = (struct Proc *)Proc_StartBlocking(
		gProcScr_TalkOpen,
		parent
	);

	proc->unk64 = GetTalkFaceHPos(talkFace);
	proc->unk66 = gpKernelDesignerConfig->half_body_portraits ? 0x12 : 0x08;
	proc->unk68 = state->activeWidth;
	proc->unk6A = state->lines * 2 + 2;

	if (proc->unk64 < 0)
		proc->unk64 = 0;
	else if (proc->unk64 > 29)
		proc->unk64 = 30;

	state->speakingFaceSlot = talkFace;
	state->speakingWidth = state->activeWidth;
}

LYN_REPLACE_CHECK(ClassChgLoadUI);
void ClassChgLoadUI_C(void)
{
	Decompress(
		gUnknown_08A30800,
		(void *)(VRAM + 0x3000 + GetBackgroundTileDataOffset(BG_2))
	);
	RegisterTsaWithOffset(
		gBG2TilemapBuffer,
		gUnknown_08A30978,
		0x11C0
	);
}

static void TextEngine_StartPause(ProcPtr parent, int pauseCode)
{
	struct Proc *proc;

	proc = Proc_StartBlocking(gProcScr_TalkPause, parent);
	proc->unk64 = GetTalkPauseCmdDuration(pauseCode);
}

static void TextEngine_DrawChoice(
	const struct ChoiceEntryInfo *choice,
	int defaultChoice,
	ProcPtr parent
)
{
	struct TalkState *state = sTextEngineState;
	struct Text *text = TextEngine_GetLineText(state, state->lineActive);
	u16 *tilemap = gBG0TilemapBuffer + TILEMAP_INDEX(
		state->xText,
		state->yText + state->lineActive * 2
	);

	StartTalkChoice(choice, text, tilemap, defaultChoice, state->printColor, parent);
}

static void TextEngine_CallMoveFaceAndWriteSpeed(int from, int to, int speed)
{
	struct TalkState *state = sTextEngineState;
	struct Proc *proc;
	struct FaceProc *oldFace;
	u16 oldNameTextId;
	int isSwap = 0;

	if (TextEngine_GetFaceProcByPosition(to)) {
		isSwap = 1;
		proc = StartTalkFaceMove_C(to, from, 1);
		if (proc) {
			proc->unk58 = 0;
			proc->unk5C = speed;
		}
	}

	proc = StartTalkFaceMove_C(from, to, isSwap);
	if (proc) {
		proc->unk58 = 0;
		proc->unk5C = speed;
	}

	oldFace = state->faces[from];
	state->faces[from] = state->faces[to];
	state->faces[to] = oldFace;

	oldNameTextId = sTextEngineNameplateState.faceNameTextIds[from];
	sTextEngineNameplateState.faceNameTextIds[from] =
		sTextEngineNameplateState.faceNameTextIds[to];
	sTextEngineNameplateState.faceNameTextIds[to] = oldNameTextId;
	SetActiveTalkFace(to);
}

static void TextEngine_LoadFace(ProcPtr parent, int options)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	const struct CharacterData *character;
	u16 faceArgument;
	u16 nameTextId = 0;
	int faceId;
	int faceDisplay = 0;
	int position;
	const u8 *faceText = (const u8 *)state->str;

	if (state->activeFaceSlot == 0xFF)
		SetActiveTalkFace(1);

	position = state->activeFaceSlot;

	/*
	 * Always draw the full 96x80 mug. Vanilla battle quotes omit
	 * FACE_96x80 (64x80 crop) and park the sprite at tile 4/26, which
	 * clips shoulders on the screen edge. Keep the banim VRAM config
	 * but use dialogue-sized faces.
	 */
	if (IsBattleDeamonActive())
		SetupFaceGfxDataInBanim();

	faceDisplay |= FACE_DISP_KIND(FACE_96x80);

	if (options == 0xFF) {
		if (GetTalkFaceHPos(position) <= 14)
			faceDisplay |= FACE_DISP_FLIPPED;
	} else if (options & 1) {
		faceDisplay |= FACE_DISP_FLIPPED;
	}

	faceArgument = faceText[0] | (faceText[1] << 8);
	faceId = faceArgument;
	if (faceArgument == 0xFFFF) {
		faceId = GetUnitPortraitId(gActiveUnit);
		if (UNIT_IS_VALID(gActiveUnit) && gActiveUnit->pCharacterData)
			nameTextId = gActiveUnit->pCharacterData->nameTextId;
	} else {
		faceId -= 0x100;
		character = TextEngine_FindCharacterByPortrait(faceId);
		if (character)
			nameTextId = character->nameTextId;
	}

	face = TextEngine_GetFaceProcByPosition(position);

	if (face) {
		sub_80066E0(face, faceId);
	} else {
		int faceY = gpKernelDesignerConfig->half_body_portraits ? 0x20 : 80;

		face = StartFaceAuto(
			faceId,
			GetTalkFaceHPos(position) * 8,
			faceY,
			faceDisplay
		);
		state->faces[position] = face;

		if (face) {
			StartFaceFadeIn(face);
			SetTalkFaceLayer(position, CheckTalkFlag(TALK_FLAG_4));
			StartTemporaryLock(parent, 8);
		}
	}

	if (position >= 0 &&
		position <
			(int)ARRAY_COUNT(sTextEngineNameplateState.faceNameTextIds))
		sTextEngineNameplateState.faceNameTextIds[position] =
			face ? nameTextId : 0;
}

static int TextEngine_HandleVanillaColor(struct TalkState *state, int colorGroup)
{
	if (state->printColor == colorGroup)
		colorGroup = 1;

	ChangeTextColorID(colorGroup);
	return 3;
}

static int TextEngine_WidthInternal(const u8 *cursor, int stopAtCurrentBox)
{
	struct TalkState *state = sTextEngineState;
	int activePosition = state->activeFaceSlot;
	int speakingPosition = (s8)state->speakingFaceSlot;
	int lineWidth = 0;
	int maxWidth = 0x18;

	while (1) {
		u8 code = *cursor;

		if (code == 0)
			break;

		if (code == 0x80) {
			const struct TextEngineCommandDescriptor *command;

			command = TextEngine_FindCommand(cursor[1]);
			if (!command) {
				/*
				 * Unknown extended commands are control bytes, not glyphs.
				 * Without a descriptor there is no safe way to infer any
				 * arguments, so skip the command header and continue.
				 */
				cursor += 2;
				continue;
			}

			if (command->code == TEXT_ENGINE_CMD_CLEAR_FACE_ASH &&
				activePosition == speakingPosition)
				goto width_done;

			if (command->width) {
				command->width(
					command,
					cursor + 2,
					stopAtCurrentBox,
					&lineWidth,
					&activePosition
				);
			}

			cursor += 2 + command->argumentCount;
			continue;
		} else if (code <= 0x1D) {
			switch (code) {
			case CHFE_L_X:
				goto width_done;

			case CHFE_L_NL:
			case CHFE_L_2NL:
				if (lineWidth > maxWidth)
					maxWidth = lineWidth;
				lineWidth = 0;
				cursor++;
				continue;

			case CHFE_L_A:
				lineWidth += 12;
				cursor++;
				continue;

			case CHFE_L_OpenFarLeft:
			case CHFE_L_OpenMidLeft:
			case CHFE_L_OpenLeft:
			case CHFE_L_OpenRight:
			case CHFE_L_OpenMidRight:
			case CHFE_L_OpenFarRight:
			case CHFE_L_OpenFarFarLeft:
			case CHFE_L_OpenFarFarRight:
				activePosition = code - CHFE_L_OpenFarLeft;
				cursor++;
				continue;

			case CHFE_L_LoadFace:
				cursor += 3;
				continue;

			case CHFE_L_ClearFace:
				if (activePosition == speakingPosition)
					goto width_done;
				cursor++;
				continue;

			case CHFE_L_NormalPrint:
			case CHFE_L_FastPrint:
			case CHFE_L_CloseSpeechFast:
				if (!stopAtCurrentBox)
					goto width_done;
				cursor++;
				continue;

			case CHFE_L_CloseSpeechSlow:
				goto width_done;

			case CHFE_L_ToggleMouthMove:
			case CHFE_L_ToggleSmile:
			case CHFE_L_Yes:
			case CHFE_L_No:
			case CHFE_L_BuySell:
			case CHFE_L_ShopContinue:
			case CHFE_L_SendToBack:
			case CHFE_L_FastPrint2:
				if (code == CHFE_L_Yes || code == CHFE_L_No ||
					code == CHFE_L_BuySell || code == CHFE_L_ShopContinue) {
					lineWidth += 80;
				}
				cursor++;
				continue;
			}
		}

		if (activePosition != speakingPosition && activePosition != 0xFF) {
			if (stopAtCurrentBox)
				break;

			stopAtCurrentBox = 1;
			speakingPosition = activePosition;
		}

		{
			u32 glyphWidth;
			cursor = (const u8 *)GetCharTextLen((const char *)cursor, &glyphWidth);
			lineWidth += glyphWidth;
		}
	}

width_done:
	if (lineWidth > maxWidth)
		maxWidth = lineWidth;

	return maxWidth;
}

LYN_REPLACE_CHECK(GetStringTextWidthWithDialogueCodes);
int GetStringTextWidthWithDialogueCodes(const char *text, int stopAtCurrentBox)
{
	struct Glyph **originalGlyphs = gActiveFont->glyphs;
	int width = TextEngine_WidthInternal((const u8 *)text, stopAtCurrentBox);

	gActiveFont->glyphs = originalGlyphs;
	return width;
}

int UpdateFontBeforeBoxWidthCalc(void)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	u8 *current = TextEngine_GetCurrentSpeakerAttributes();
	u8 *faceAttributes = TextEngine_GetFaceAttributes(face);
	const char *text;

	if (faceAttributes && current[TEXT_ENGINE_ATTR_FONT] != faceAttributes[TEXT_ENGINE_ATTR_FONT]) {
		current[TEXT_ENGINE_ATTR_FONT] = faceAttributes[TEXT_ENGINE_ATTR_FONT];
		UpdateFontGlyphSet(current[TEXT_ENGINE_ATTR_FONT]);
	}

	text = state->strBackup ? state->strBackup : state->str;
	return GetStringTextWidthWithDialogueCodes(text, 0);
}

void DecompressTextBoxGraphics(ProcPtr procPtr)
{
	struct Proc *proc = procPtr;
	const u32 *graphics;
	const u32 *frame;
	u8 boxType = TextEngine_GetCurrentSpeakerAttributes()[TEXT_ENGINE_ATTR_BOX_TYPE];
	u16 animationFrame = proc->unk64;
	u32 currentGraphics;
	u32 nextGraphics;
	void *destination;

	proc->unk64++;

	if (animationFrame & 1)
		return;

	graphics = TextBoxTypePointerTable[boxType];
	frame = graphics + (animationFrame >> 1);
	currentGraphics = frame[0];
	nextGraphics = frame[1];

	destination = (void *)(0x06000200 + GetBackgroundTileDataOffset(1));
	Decompress((const void *)currentGraphics, destination);

	if (!nextGraphics)
		Proc_Break(proc);
}

const struct ProcCmd gProc_DialogueBoxAppearingAnimation[] = {
	PROC_CALL(Copy_Text_Attributes),
	PROC_CALL(TextEngine_DrawSpeakerNameplate),
	PROC_REPEAT(DecompressTextBoxGraphics),
	PROC_END,
};

static void TextEngine_CommandWidthExpandNumber(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;
	(void)arguments;
	(void)activePosition;

	NumberToStringAscii(state->userNumber, state->userNumberString);
	*lineWidth += TextEngine_WidthInternal(
		(const u8 *)state->userNumberString,
		stopAtCurrentBox
	);
}

static void TextEngine_CommandWidthExpandUserString(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;
	(void)arguments;
	(void)activePosition;

	*lineWidth += TextEngine_WidthInternal(
		(const u8 *)state->userString,
		stopAtCurrentBox
	);
}

static void TextEngine_CommandWidthExpandTacticianName(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
)
{
	(void)command;
	(void)arguments;
	(void)stopAtCurrentBox;
	(void)activePosition;

	*lineWidth += GetStringTextLen(GetTacticianName());
}

static void TextEngine_CommandWidthSetFacePosition(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
)
{
	(void)arguments;
	(void)stopAtCurrentBox;
	(void)lineWidth;

	if (command->code < 0x30)
		*activePosition = command->code - 0x0A;
	else
		*activePosition = command->code - 0x30;
}

static void TextEngine_CommandWidthSetFont(
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments,
	int stopAtCurrentBox,
	int *lineWidth,
	int *activePosition
)
{
	(void)command;
	(void)stopAtCurrentBox;
	(void)lineWidth;
	(void)activePosition;

	UpdateFontGlyphSet(arguments[0] - 1);
}

static int TextEngine_CommandVanillaColor(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)arguments;

	return TextEngine_HandleVanillaColor(
		sTextEngineState,
		command->code + 1
	);
}

static int TextEngine_CommandPauseDialogue(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;
	(void)arguments;

	LockTalk(proc);
	return 3;
}

static int TextEngine_CommandPrintMonetaryAmount(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;

	NumberToStringAscii(state->userNumber, state->userNumberString);
	state->strBackup = (const char *)(arguments - 2);
	state->str = state->userNumberString;
	return TalkInterpret(proc);
}

static int TextEngine_CommandSwitchToMiniTextBuffer(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;

	state->strBackup = (const char *)(arguments - 2);
	state->str = state->userString;
	return TalkInterpret(proc);
}

static int TextEngine_CommandReturnThree(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	return 3;
}

static int TextEngine_CommandReturnZero(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	return 0;
}

static int TextEngine_CommandMoveFace(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)proc;
	(void)arguments;

	TextEngine_CallMoveFaceAndWriteSpeed(
		state->activeFaceSlot,
		command->code - 0x0A,
		0
	);
	return 3;
}

static int TextEngine_CommandFaceBlink(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	int blinkControl;

	(void)proc;
	(void)arguments;

	face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	if (!face)
		return 3;

	switch (command->code) {
	case 0x16:
		blinkControl = 0;
		break;
	case 0x17:
		blinkControl = 1;
		break;
	case 0x18:
		blinkControl = 3;
		break;
	case 0x19:
		blinkControl = 2;
		break;
	case 0x1A:
		blinkControl = 4;
		break;
	default:
		blinkControl = 5;
		break;
	}

	SetFaceBlinkControl(face, blinkControl);
	return 3;
}

static int TextEngine_CommandFaceEyes(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	int eyeControl;

	(void)proc;
	(void)arguments;

	face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	if (!face)
		return 3;

	switch (command->code) {
	case 0x1C:
		eyeControl = 0;
		break;
	case 0x1D:
		eyeControl = 2;
		break;
	case 0x1E:
		eyeControl = 3;
		break;
	default:
		eyeControl = 4;
		break;
	}

	sub_80064D4(face, eyeControl);
	return 3;
}

static int TextEngine_CommandTacticianName(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;

	state->strBackup = (const char *)(arguments - 2);
	state->str = GetTacticianName();
	return TalkInterpret(proc);
}

static int TextEngine_CommandToggleRed(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	return TextEngine_HandleVanillaColor(sTextEngineState, 4);
}

static int TextEngine_CommandExecuteRoutine(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;
	(void)arguments;

	if (sTextEngineState->unk38)
		sTextEngineState->unk38(proc);

	return 3;
}

static int TextEngine_CommandToggleColorInvert(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	sTextEngineState->invertedFlags = 3 - (sTextEngineState->invertedFlags & 1);
	return 3;
}

static int TextEngine_CommandChangeFont(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int font = arguments[0] - 1;

	(void)command;

	TextEngine_SetCurrentAttributeAndFace(
		sTextEngineState->activeFaceSlot,
		TEXT_ENGINE_ATTR_FONT,
		font
	);
	UpdateFontGlyphSet(font);
	return TalkInterpret(proc);
}

static int TextEngine_CommandChangeTextPalette(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int group = arguments[0] - 1;
	int palette = arguments[1] - 1;
	int destination = gActiveFont->palid * 0x20 + group * 6 + 2;

	(void)proc;
	(void)command;

	CopyToPaletteBuffer(&TextPaletteTable[palette * 3], destination, 6);
	return 3;
}

static int TextEngine_CommandChangeTextColorGroup(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int colorGroup = arguments[0] - 1;

	(void)command;

	TextEngine_SetCurrentAttributeAndFace(
		sTextEngineState->activeFaceSlot,
		TEXT_ENGINE_ATTR_COLOR_GROUP,
		colorGroup
	);
	ChangeTextColorID(colorGroup);
	return TalkInterpret(proc);
}

static int TextEngine_CommandChangeTextBoxBgPalette(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int palette = arguments[0] - 1;

	(void)proc;
	(void)command;

	TextEngine_SetCurrentAttributeAndFace(
		sTextEngineState->activeFaceSlot,
		TEXT_ENGINE_ATTR_BOX_PALETTE,
		palette
	);
	UpdateTextBoxBgPalette(palette);
	return 3;
}

static int TextEngine_CommandChangeTextBoxType(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int boxType = arguments[0] - 1;

	(void)command;

	TextEngine_SetCurrentAttributeAndFace(
		sTextEngineState->activeFaceSlot,
		TEXT_ENGINE_ATTR_BOX_TYPE,
		boxType
	);
	return TalkInterpret(proc);
}

static int TextEngine_CommandChangeTextBoxHeight(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;

	sTextEngineState->lines = arguments[0];
	return TalkInterpret(proc);
}

static int TextEngine_CommandChangeTextBoopPitch(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int pitch = arguments[0] - 1;

	(void)command;

	TextEngine_SetCurrentAttributeAndFace(
		sTextEngineState->activeFaceSlot,
		TEXT_ENGINE_ATTR_BOOP_PITCH,
		pitch
	);
	return TalkInterpret(proc);
}

static int TextEngine_CommandPlaySound(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	u16 song = (arguments[0] & 0xF)
		| ((arguments[1] & 0xF) << 4)
		| ((arguments[2] & 0xF) << 8)
		| ((arguments[3] & 0xF) << 12);

	(void)proc;
	(void)command;

	m4aSongNumStart(song);
	return 3;
}

static int TextEngine_CommandChangePortraitPosition(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int position = arguments[0] - 1;
	u8 x = arguments[1];

	(void)proc;
	(void)command;

	if (x == 0x80)
		x = 0;

	((u8 *)sTextEngineState + 0x50)[position] = x;
	return TalkInterpret(proc);
}

static int TextEngine_CommandLoadFaceFancy(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
	u8 *faceAttributes;
	u8 options = arguments[2] & 0x7F;
	const char *next;

	(void)command;

	/*
	 * TextEngine_LoadFace reads the portrait ID from state->str.  The
	 * descriptor dispatcher has already advanced state->str past all
	 * arguments, so temporarily point it back at the command payload.
	 */
	next = state->str;
	state->str = (const char *)arguments;
	TextEngine_LoadFace(proc, options & 1);
	state->str = next;

	face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	faceAttributes = TextEngine_GetFaceAttributes(face);

	if (faceAttributes) {
		faceAttributes[TEXT_ENGINE_ATTR_FONT] = arguments[3] - 1;
		faceAttributes[TEXT_ENGINE_ATTR_COLOR_GROUP] = arguments[4] - 1;
		faceAttributes[TEXT_ENGINE_ATTR_BOX_PALETTE] = arguments[5] - 1;
		faceAttributes[TEXT_ENGINE_ATTR_BOX_TYPE] = arguments[6] - 1;
		faceAttributes[TEXT_ENGINE_ATTR_BOOP_PITCH] = arguments[7] - 1;
	}

	if ((options & 2) && face)
		sub_80064D4(face, 2);

	return 3;
}

static int TextEngine_CommandMoveFaceVariableSpeed(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)proc;

	TextEngine_CallMoveFaceAndWriteSpeed(
		state->activeFaceSlot,
		command->code - 0x30,
		arguments[0]
	);
	return 3;
}

static int TextEngine_CommandChangeTextSpeed(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;

	if (arguments[0] == 0xFF)
		sTextEngineState->printDelay = GetTextDisplaySpeed();
	else
		sTextEngineState->printDelay = arguments[0];

	return TalkInterpret(proc);
}

static int TextEngine_CommandStartFaceJump(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StartFaceJump(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static int TextEngine_CommandStopFaceJump(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StopFaceJump(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static int TextEngine_CommandStartPrintShake(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	*TextEngine_GetShakePrintFlag() = 1;
	return 3;
}

static int TextEngine_CommandStopPrintShake(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	*TextEngine_GetShakePrintFlag() = 0;
	return 3;
}

static u8 TextEngine_LetterFxModeFromOnCode(u8 code)
{
	switch (code) {
	case 0x3D:
		return TEXT_ENGINE_LETTER_FX_BOUNCE;
	case TEXT_ENGINE_CMD_WAVY_PRINT_ON:
		return TEXT_ENGINE_LETTER_FX_WAVY;
	case TEXT_ENGINE_CMD_SCRAMBLE_PRINT_ON:
		return TEXT_ENGINE_LETTER_FX_SCRAMBLE;
	case TEXT_ENGINE_CMD_DRIP_PRINT_ON:
		return TEXT_ENGINE_LETTER_FX_DRIP;
	case TEXT_ENGINE_CMD_GHOST_PRINT_ON:
		return TEXT_ENGINE_LETTER_FX_GHOST;
	default:
		return TEXT_ENGINE_LETTER_FX_NONE;
	}
}

static int TextEngine_CommandStartLetterFx(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)arguments;

	*TextEngine_GetLetterFxMode() = TextEngine_LetterFxModeFromOnCode(command->code);
	TextEngine_PrepareFloatPalette();
	return 3;
}

static int TextEngine_CommandStopLetterFx(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	u8 mode = TextEngine_LetterFxModeFromOnCode(command->code - 1);

	(void)proc;
	(void)arguments;

	if (*TextEngine_GetLetterFxMode() == mode)
		*TextEngine_GetLetterFxMode() = TEXT_ENGINE_LETTER_FX_NONE;

	Proc_EndEach(gProcScr_TextEngineGlyphFloat);
	return 3;
}

static int TextEngine_CommandStartWave(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StartWave();
	return 3;
}

static int TextEngine_CommandStopWave(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StopWave();
	return 3;
}

static int TextEngine_CommandStartFaceVibrate(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StartFaceVibrate(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static int TextEngine_CommandStopFaceVibrate(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StopFaceVibrate(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static int TextEngine_CommandStartFaceShimmy(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StartFaceShimmy(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static int TextEngine_CommandStopFaceShimmy(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_StopFaceShimmy(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot)
	);
	return 3;
}

static void TextEngine_CleanupFaceMotion(void)
{
	Proc_EndEach(gProcScr_TextEngineFaceJump);
}

static void TextEngine_CleanupPrintShake(void)
{
	*TextEngine_GetShakePrintFlag() = 0;
	Proc_EndEach(gProcScr_TextEnginePrintFx);
}

static void TextEngine_CleanupLetterFx(void)
{
	*TextEngine_GetLetterFxMode() = TEXT_ENGINE_LETTER_FX_NONE;
	Proc_EndEach(gProcScr_TextEngineGlyphFloat);
}

static void TextEngine_CleanupWave(void)
{
	TextEngine_StopWave();
}

static int TextEngine_CommandStartNameplate(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;
	(void)arguments;

	sTextEngineNameplateState.enabled = 1;
	TextEngine_DrawSpeakerNameplate(proc);
	return 3;
}

static int TextEngine_CommandStopNameplate(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	sTextEngineNameplateState.enabled = 0;
	TextEngine_ClearSpeakerNameplate();
	return 3;
}

static void TextEngine_CleanupNameplate(void)
{
	TextEngine_ClearSpeakerNameplate();
	sTextEngineNameplateState.enabled = 0;
}

static int TextEngine_CommandStartScreenGlitch(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	EnableScreenGlitch();
	return 3;
}

static int TextEngine_CommandStopScreenGlitch(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	DisableScreenGlitch();
	return 3;
}

static int TextEngine_CommandStartScreenStatic(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	EnableScreenStatic();
	return 3;
}

static int TextEngine_CommandStopScreenStatic(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	DisableScreenStatic();
	return 3;
}

static int TextEngine_CommandStartScreenEarthquake(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	EnableScreenEarthquake();
	return 3;
}

static int TextEngine_CommandStopScreenEarthquake(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	DisableScreenEarthquake();
	return 3;
}

static int TextEngine_CommandImpactFlash(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	StartScreenImpactFlash();
	return 3;
}

static void TextEngine_CleanupImpactFlash(void)
{
	Proc_EndEach(gProcScr_TextEngineImpactFlash);
}

static int TextEngine_CommandClearFaceAsh(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;

	(void)command;
	(void)arguments;

	face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
	HalfBody_OnTalkFaceClear(face);

	TextEngine_ClearSpeakerNameplate();
	if (TalkHasCorrectBubble())
		ClearTalkBubble();

	if (face) {
		TextEngine_StartAshDissolve(face);
		state->faces[state->activeFaceSlot] = NULL;
	}
	if (state->activeFaceSlot <
		(int)ARRAY_COUNT(sTextEngineNameplateState.faceNameTextIds))
		sTextEngineNameplateState.faceNameTextIds[state->activeFaceSlot] = 0;

	StartTemporaryLock(proc, TEXT_ENGINE_ASH_DURATION);
	return 3;
}

/*
 * Keep every extended command's argument shape and behavior in one table.
 * argumentCount is the number of bytes after [0x80][code].
 */
static const struct TextEngineCommandDescriptor sTextEngineCommandTable[] = {
	{ 0x00, 0, TextEngine_CommandVanillaColor, NULL, NULL },
	{ 0x01, 0, TextEngine_CommandVanillaColor, NULL, NULL },
	{ 0x02, 0, TextEngine_CommandVanillaColor, NULL, NULL },
	{ 0x03, 0, TextEngine_CommandVanillaColor, NULL, NULL },
	{ 0x04, 0, TextEngine_CommandPauseDialogue, NULL, NULL },
	{ 0x05, 0, TextEngine_CommandPrintMonetaryAmount, TextEngine_CommandWidthExpandNumber, NULL },
	{ 0x06, 0, TextEngine_CommandSwitchToMiniTextBuffer, TextEngine_CommandWidthExpandUserString, NULL },
	{ 0x07, 0, TextEngine_CommandReturnThree, NULL, NULL },
	{ 0x08, 0, TextEngine_CommandReturnThree, NULL, NULL },
	{ 0x09, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x0A, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x0B, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x0C, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x0D, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x0E, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x0F, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x10, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x11, 0, TextEngine_CommandMoveFace, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x12, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x13, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x14, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x15, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x16, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x17, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x18, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x19, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x1A, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x1B, 0, TextEngine_CommandFaceBlink, NULL, NULL },
	{ 0x1C, 0, TextEngine_CommandFaceEyes, NULL, NULL },
	{ 0x1D, 0, TextEngine_CommandFaceEyes, NULL, NULL },
	{ 0x1E, 0, TextEngine_CommandFaceEyes, NULL, NULL },
	{ 0x1F, 0, TextEngine_CommandFaceEyes, NULL, NULL },
	{ 0x20, 0, TextEngine_CommandTacticianName, TextEngine_CommandWidthExpandTacticianName, NULL },
	{ 0x21, 0, TextEngine_CommandToggleRed, NULL, NULL },
	{ 0x22, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x23, 0, TextEngine_CommandReturnZero, NULL, NULL },
	{ 0x24, 0, TextEngine_CommandExecuteRoutine, NULL, NULL },
	{ 0x25, 0, TextEngine_CommandToggleColorInvert, NULL, NULL },
	{ 0x26, 1, TextEngine_CommandChangeFont, TextEngine_CommandWidthSetFont, NULL },
	{ 0x27, 2, TextEngine_CommandChangeTextPalette, NULL, NULL },
	{ 0x28, 1, TextEngine_CommandChangeTextColorGroup, NULL, NULL },
	{ 0x29, 1, TextEngine_CommandChangeTextBoxBgPalette, NULL, NULL },
	{ 0x2A, 1, TextEngine_CommandChangeTextBoxType, NULL, NULL },
	{ 0x2B, 1, TextEngine_CommandChangeTextBoxHeight, NULL, NULL },
	{ 0x2C, 1, TextEngine_CommandChangeTextBoopPitch, NULL, NULL },
	{ 0x2D, 4, TextEngine_CommandPlaySound, NULL, NULL },
	{ 0x2E, 2, TextEngine_CommandChangePortraitPosition, NULL, NULL },
	{ 0x2F, 8, TextEngine_CommandLoadFaceFancy, NULL, NULL },
	{ 0x30, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x31, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x32, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x33, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x34, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x35, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x36, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x37, 1, TextEngine_CommandMoveFaceVariableSpeed, TextEngine_CommandWidthSetFacePosition, NULL },
	{ 0x38, 1, TextEngine_CommandChangeTextSpeed, NULL, NULL },
	{ 0x39, 0, TextEngine_CommandStartFaceJump, NULL, TextEngine_CleanupFaceMotion },
	{ 0x3A, 0, TextEngine_CommandStopFaceJump, NULL, NULL },
	{ 0x3B, 0, TextEngine_CommandStartPrintShake, NULL, TextEngine_CleanupPrintShake },
	{ 0x3C, 0, TextEngine_CommandStopPrintShake, NULL, NULL },
	{ 0x3D, 0, TextEngine_CommandStartLetterFx, NULL, TextEngine_CleanupLetterFx },
	{ 0x3E, 0, TextEngine_CommandStopLetterFx, NULL, NULL },
	{ 0x3F, 0, TextEngine_CommandStartWave, NULL, TextEngine_CleanupWave },
	{ 0x40, 0, TextEngine_CommandStopWave, NULL, NULL },
	{ 0x41, 0, TextEngine_CommandStartFaceVibrate, NULL, TextEngine_CleanupFaceMotion },
	{ 0x42, 0, TextEngine_CommandStopFaceVibrate, NULL, NULL },
	{ 0x43, 0, TextEngine_CommandStartFaceShimmy, NULL, TextEngine_CleanupFaceMotion },
	{ 0x44, 0, TextEngine_CommandStopFaceShimmy, NULL, NULL },
	{ 0x45, 0, TextEngine_CommandStartNameplate, NULL, TextEngine_CleanupNameplate },
	{ 0x46, 0, TextEngine_CommandStopNameplate, NULL, NULL },
	{ 0x47, 0, TextEngine_CommandStartScreenGlitch, NULL, NULL },
	{ 0x48, 0, TextEngine_CommandStopScreenGlitch, NULL, NULL },
	{ 0x49, 0, TextEngine_CommandStartScreenStatic, NULL, NULL },
	{ 0x4A, 0, TextEngine_CommandStopScreenStatic, NULL, NULL },
	{ TEXT_ENGINE_CMD_CLEAR_FACE_ASH, 0, TextEngine_CommandClearFaceAsh, NULL, TextEngine_CleanupAshDissolve },
	{ TEXT_ENGINE_CMD_WAVY_PRINT_ON, 0, TextEngine_CommandStartLetterFx, NULL, TextEngine_CleanupLetterFx },
	{ TEXT_ENGINE_CMD_WAVY_PRINT_OFF, 0, TextEngine_CommandStopLetterFx, NULL, NULL },
	{ TEXT_ENGINE_CMD_SCRAMBLE_PRINT_ON, 0, TextEngine_CommandStartLetterFx, NULL, TextEngine_CleanupLetterFx },
	{ TEXT_ENGINE_CMD_SCRAMBLE_PRINT_OFF, 0, TextEngine_CommandStopLetterFx, NULL, NULL },
	{ TEXT_ENGINE_CMD_DRIP_PRINT_ON, 0, TextEngine_CommandStartLetterFx, NULL, TextEngine_CleanupLetterFx },
	{ TEXT_ENGINE_CMD_DRIP_PRINT_OFF, 0, TextEngine_CommandStopLetterFx, NULL, NULL },
	{ TEXT_ENGINE_CMD_GHOST_PRINT_ON, 0, TextEngine_CommandStartLetterFx, NULL, TextEngine_CleanupLetterFx },
	{ TEXT_ENGINE_CMD_GHOST_PRINT_OFF, 0, TextEngine_CommandStopLetterFx, NULL, NULL },
	{ TEXT_ENGINE_CMD_EARTHQUAKE_ON, 0, TextEngine_CommandStartScreenEarthquake, NULL, NULL },
	{ TEXT_ENGINE_CMD_EARTHQUAKE_OFF, 0, TextEngine_CommandStopScreenEarthquake, NULL, NULL },
	{ TEXT_ENGINE_CMD_IMPACT_FLASH, 0, TextEngine_CommandImpactFlash, NULL, TextEngine_CleanupImpactFlash },
};

static const struct TextEngineCommandDescriptor *TextEngine_FindCommand(u8 code)
{
	int i;

	for (i = 0; i < (int)ARRAY_COUNT(sTextEngineCommandTable); i++) {
		if (sTextEngineCommandTable[i].code == code)
			return &sTextEngineCommandTable[i];
	}

	return NULL;
}

static void TextEngine_RunCommandCleanup(void)
{
	int i;

	for (i = 0; i < (int)ARRAY_COUNT(sTextEngineCommandTable); i++) {
		TextEngineCommandCleanupHandler cleanup =
			sTextEngineCommandTable[i].cleanup;
		int previous;

		if (!cleanup)
			continue;

		for (previous = 0; previous < i; previous++) {
			if (sTextEngineCommandTable[previous].cleanup == cleanup)
				break;
		}

		if (previous == i)
			cleanup();
	}
}

LYN_REPLACE_CHECK(TalkInterpret);
int TalkInterpret(ProcPtr proc)
{
	struct TalkState *state = sTextEngineState;
	u8 *text;
	u8 code;
	struct FaceProc *face;
	const struct TextEngineCommandDescriptor *command;
	const u8 *arguments;

	while (1) {
		text = (u8 *)state->str;
		code = *text;

		if (code == 0) {
			if (!state->strBackup)
				return 0;

			state->str = state->strBackup + 2;
			state->strBackup = NULL;
			continue;
		}

		if (code == 0x80) {
			arguments = text + 2;
			state->str = (char *)arguments;
			command = TextEngine_FindCommand(text[1]);

			if (!command || !command->handler)
				return 1;

			state->str = (char *)(arguments + command->argumentCount);
			return command->handler(proc, command, arguments);
		}

		if (code > 0x1D)
			return 1;

		state->str = (char *)(text + 1);

		switch (code) {
		case CHFE_L_NL:
			Chatlog_AppendSoftBreak();
			if (state->putLines == 1 || state->lineActive == 1)
				state->lineActive++;

			state->putLines = 0;
			return 2;

		case CHFE_L_2NL:
			Chatlog_CommitPage();
			/*
			 * Only the world-map path consumes a second byte here; every
			 * other path must leave state->str on the byte right after the
			 * 0x02 code, or the following control code is swallowed.
			 */
			if (CheckTalkFlag(TALK_FLAG_7)) {
				TalkFlushAllLine();
				state->str++;
			} else if (!CheckTalkFlag(TALK_FLAG_INSTANTSHIFT)) {
				Proc_StartBlocking(gProcScr_TalkShiftClearAll, proc);
			} else {
				ClearTalkText();
			}

			return 3;

		case CHFE_L_A:
			Chatlog_CommitPage();
			StartTalkWaitForInput(
				proc,
				state->xText * 8 + Text_GetCursor(TextEngine_GetLineText(state, state->lineActive)) + 4,
				state->yText * 8 + state->lineActive * 16 + 8
			);
			return 3;

		case CHFE_L_Pause8:
		case CHFE_L_Pause16:
		case CHFE_L_Pause32:
		case CHFE_L_Pause64:
			if (state->instantScroll)
				return 2;

			TextEngine_StartPause(proc, code);
			return 3;

		case CHFE_L_OpenFarLeft:
		case CHFE_L_OpenMidLeft:
		case CHFE_L_OpenLeft:
		case CHFE_L_OpenRight:
		case CHFE_L_OpenMidRight:
		case CHFE_L_OpenFarRight:
		case CHFE_L_OpenFarFarLeft:
		case CHFE_L_OpenFarFarRight:
			TextEngineUnsetFaceDisplayBits(state->activeFaceSlot);
			SetActiveTalkFace(code - CHFE_L_OpenFarLeft);
			return 3;

		case CHFE_L_LoadFace:
			TextEngine_LoadFace(proc, 0xFF);
			state->str += 2;
			TextEngine_SetDefaultFaceAttributes(
				TextEngine_GetFaceProcByPosition(state->activeFaceSlot)
			);
			return 3;

		case CHFE_L_ClearFace:
			face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
			/* Hide halfbodies before bubble/nameplate teardown. */
			HalfBody_OnTalkFaceClear(face);

			TextEngine_ClearSpeakerNameplate();
			if (TalkHasCorrectBubble())
				ClearTalkBubble();

			if (face) {
				StartFaceFadeOut(face);
				state->faces[state->activeFaceSlot] = NULL;
			}
			if (state->activeFaceSlot <
				(int)ARRAY_COUNT(sTextEngineNameplateState.faceNameTextIds))
				sTextEngineNameplateState.faceNameTextIds[state->activeFaceSlot] = 0;

			StartTemporaryLock(proc, 0x10);
			return 3;

		case CHFE_L_CloseSpeechSlow:
			TextEngine_ClearSpeakerNameplate();
			ClearTalkBubble();
			return 3;

		case CHFE_L_ToggleMouthMove:
			state->mouthMoveEnabled = 1 - state->mouthMoveEnabled;
			return 3;

		case CHFE_L_ToggleSmile:
			state->faceSmileEnabled = 1 - state->faceSmileEnabled;
			return 3;

		case CHFE_L_Yes:
			TextEngine_DrawChoice(gYesNoTalkChoice, 1, proc);
			return 3;

		case CHFE_L_No:
			TextEngine_DrawChoice(gYesNoTalkChoice, 2, proc);
			return 3;

		case CHFE_L_BuySell:
			TextEngine_DrawChoice(gBuySellTalkChoice, 1, proc);
			return 3;

		case CHFE_L_ShopContinue:
			TextEngine_DrawChoice(gBuySellTalkChoice, 2, proc);
			return 3;

		case CHFE_L_SendToBack:
			SetTalkFlag(TALK_FLAG_4);
			return 3;

		case CHFE_L_FastPrint2:
			ClearTalkFlag(TALK_FLAG_4);
			return 3;

		case CHFE_L_NormalPrint:
		case CHFE_L_FastPrint:
		case CHFE_L_CloseSpeechFast:
			state->activeWidth = 2 + (
				GetStringTextWidthWithDialogueCodes(state->str, TalkHasCorrectBubble()) + 7
			) / 8;
			continue;

		default:
			return 1;
		}
	}
}
