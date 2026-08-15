#include "common-chax.h"
#include "kernel-lib.h"
#include "utf8.h"
#include "kernel/chatlog.h"

#include "bmlib.h"
#include "ctc.h"
#include "fontgrp.h"
#include "m4a.h"
#include "scene.h"

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
#define TEXT_ENGINE_BOUNCE_PRINT_FLAG_OFFSET 0x5E

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
static void TextEngineGlyphFloat_OnIdle(struct TextEngineGlyphFloatProc *proc);
static void TextEngineGlyphFloat_OnEnd(struct TextEngineGlyphFloatProc *proc);

static const s8 sTextEnginePrintShakeOffsets[][2] = {
	{ +1, -1 },
	{ -1, +1 },
	{ +1,  0 },
	{  0,  0 },
};

static const s8 sTextEngineFaceVibrateOffsets[] = {
	0, +1, 0, -1,
};

extern EWRAM_DATA s16 sTextEngineWaveOffsets[2][DISPLAY_HEIGHT];
extern EWRAM_DATA volatile u8 sTextEngineWaveActiveBuffer;

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

static const struct ProcCmd gProcScr_TextEngineGlyphFloat[] = {
	PROC_NAME("TextEngineGlyphFloat"),
	PROC_SET_END_CB(TextEngineGlyphFloat_OnEnd),
	PROC_REPEAT(TextEngineGlyphFloat_OnIdle),
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

static u8 *TextEngine_GetBouncePrintFlag(void)
{
	return (u8 *)sTextEngineState + TEXT_ENGINE_BOUNCE_PRINT_FLAG_OFFSET;
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

static void TextEngineWave_BuildBuffer(
	struct TextEngineWaveProc *proc,
	int buffer
)
{
	int line;

	for (line = 0; line < DISPLAY_HEIGHT; line++) {
		sTextEngineWaveOffsets[buffer][line] =
			(SIN(proc->phase + line * TEXT_ENGINE_WAVE_FREQUENCY)
				* TEXT_ENGINE_WAVE_AMPLITUDE) >> 8;
	}
}

static void TextEngineWave_ApplyBaseOffsets(void)
{
	REG_BG0HOFS = gLCDControlBuffer.bgoffset[BG_0].x;
	REG_BG1HOFS = gLCDControlBuffer.bgoffset[BG_1].x;
	REG_BG2HOFS = gLCDControlBuffer.bgoffset[BG_2].x;
	REG_BG3HOFS = gLCDControlBuffer.bgoffset[BG_3].x;
}

static void TextEngineWave_OnHBlank(void)
{
	u16 line = REG_VCOUNT;
	s16 offset;

	if (line >= DISPLAY_HEIGHT)
		return;

	offset = sTextEngineWaveOffsets[sTextEngineWaveActiveBuffer][line];
	REG_BG0HOFS = gLCDControlBuffer.bgoffset[BG_0].x + offset;
	REG_BG1HOFS = gLCDControlBuffer.bgoffset[BG_1].x + offset;
	REG_BG2HOFS = gLCDControlBuffer.bgoffset[BG_2].x + offset;
	REG_BG3HOFS = gLCDControlBuffer.bgoffset[BG_3].x + offset;
}

static void TextEngineWave_OnEnd(struct TextEngineWaveProc *proc)
{
	SetSecondaryHBlankHandler(proc->previousHBlankHandler);
	TextEngineWave_ApplyBaseOffsets();
}

static void TextEngineWave_OnIdle(struct TextEngineWaveProc *proc)
{
	int nextBuffer = proc->activeBuffer ^ 1;

	/* Keep the per-scanline shift flat so the log is not sheared. */
	if (Chatlog_IsVisible()) {
		int line;

		for (line = 0; line < DISPLAY_HEIGHT; line++)
			sTextEngineWaveOffsets[proc->activeBuffer][line] = 0;
		return;
	}

	proc->phase += TEXT_ENGINE_WAVE_SPEED;
	TextEngineWave_BuildBuffer(proc, nextBuffer);
	proc->activeBuffer = nextBuffer;
	sTextEngineWaveActiveBuffer = nextBuffer;
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
	proc->previousHBlankHandler = sHBlankHandler2;
	TextEngineWave_BuildBuffer(proc, 0);
	TextEngineWave_BuildBuffer(proc, 1);
	sTextEngineWaveActiveBuffer = 0;

	/*
	 * Keep the primary HBlank slot available for existing dialogue effects.
	 * The secondary slot is restored when the wave proc ends.
	 */
	SetSecondaryHBlankHandler(TextEngineWave_OnHBlank);
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
	int y;
	int oam2;

	/*
	 * Freeze mid-flight glyphs while the log is up: ending would bake them
	 * into the talk font tiles the log is currently borrowing.
	 */
	if (Chatlog_IsVisible())
		return;

	if (proc->timer >= TEXT_ENGINE_FLOAT_DURATION) {
		Proc_End(proc);
		return;
	}

	if (proc->glyph) {
		TextEngine_UploadGlyphToObj(
			proc->glyph,
			TextEngine_GetFloatObjChr(proc->slot),
			proc->color
		);
	}

	y = Interpolate(
		INTERPOLATE_RCUBIC,
		proc->destY - TEXT_ENGINE_FLOAT_LIFT,
		proc->destY,
		proc->timer,
		TEXT_ENGINE_FLOAT_DURATION
	);

	oam2 = OAM2_CHR(TextEngine_GetFloatObjChr(proc->slot))
		| OAM2_PAL(TEXT_ENGINE_FLOAT_OBJ_PAL)
		| OAM2_LAYER(0);

	PutSpriteExt(4, OAM1_X(proc->screenX), OAM0_Y(y), gObject_8x16, oam2);
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

	if (!*TextEngine_GetBouncePrintFlag())
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
	Chatlog_EndSession();
	TextEngine_ClearSpeakerNameplate();
	TextEngine_ClearFaceNameTextIds();
	Proc_EndEach(gProcScr_TalkSkipListener);
	Proc_EndEach(gProcScr_TalkShiftClearAll);
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
		return talkFace <= 2 ? 4 : 26;

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

	if (IsBattleDeamonActive()) {
		SetupFaceGfxDataInBanim();
	} else {
		faceDisplay |= FACE_DISP_KIND(FACE_96x80);
	}

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

static int TextEngine_CommandStartPrintBounce(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	*TextEngine_GetBouncePrintFlag() = 1;
	TextEngine_PrepareFloatPalette();
	return 3;
}

static int TextEngine_CommandStopPrintBounce(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	*TextEngine_GetBouncePrintFlag() = 0;
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

static void TextEngine_CleanupPrintBounce(void)
{
	*TextEngine_GetBouncePrintFlag() = 0;
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
	{ 0x3D, 0, TextEngine_CommandStartPrintBounce, NULL, TextEngine_CleanupPrintBounce },
	{ 0x3E, 0, TextEngine_CommandStopPrintBounce, NULL, NULL },
	{ 0x3F, 0, TextEngine_CommandStartWave, NULL, TextEngine_CleanupWave },
	{ 0x40, 0, TextEngine_CommandStopWave, NULL, NULL },
	{ 0x41, 0, TextEngine_CommandStartFaceVibrate, NULL, TextEngine_CleanupFaceMotion },
	{ 0x42, 0, TextEngine_CommandStopFaceVibrate, NULL, NULL },
	{ 0x43, 0, TextEngine_CommandStartFaceShimmy, NULL, TextEngine_CleanupFaceMotion },
	{ 0x44, 0, TextEngine_CommandStopFaceShimmy, NULL, NULL },
	{ 0x45, 0, TextEngine_CommandStartNameplate, NULL, TextEngine_CleanupNameplate },
	{ 0x46, 0, TextEngine_CommandStopNameplate, NULL, NULL },
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
