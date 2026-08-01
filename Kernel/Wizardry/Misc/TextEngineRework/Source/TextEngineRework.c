#include "common-chax.h"
#include "kernel-lib.h"

#include "bmlib.h"
#include "fontgrp.h"
#include "m4a.h"
#include "scene.h"

/*
 * The vanilla dialogue state is allocated in EWRAM by the game.  Keep these
 * aliases local to this module instead of adding another global definition to
 * the shared library.
 */
#define sTextEngineState (*(struct TalkState **)0x0859133C)
#define sTextEngineText  (*(struct Text (*)[3])0x030000D0)

#define TEXT_ENGINE_FACE_ATTRIBUTES_OFFSET 0x4C
#define TEXT_ENGINE_SHAKE_PRINT_FLAG_OFFSET 0x5D

enum TextEngineFaceAttribute {
	TEXT_ENGINE_ATTR_FONT,
	TEXT_ENGINE_ATTR_COLOR_GROUP,
	TEXT_ENGINE_ATTR_BOX_PALETTE,
	TEXT_ENGINE_ATTR_BOX_TYPE,
	TEXT_ENGINE_ATTR_BOOP_PITCH,
};

enum {
	TEXT_ENGINE_MAX_EXTRA_CODE = 0x3C,
};

enum {
	TEXT_ENGINE_FACE_JUMP_PERIOD = 16,
	TEXT_ENGINE_FACE_JUMP_HEIGHT = 6,
	TEXT_ENGINE_PRINT_SHAKE_FRAMES = 4,
};

struct TextEngineFaceJumpProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct FaceProc *face;
	/* 30 */ s16 timer;
	/* 32 */ s16 offset;
};

struct TextEnginePrintShakeProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ s16 timer;
	/* 2E */ s16 baseX;
	/* 30 */ s16 baseY;
};

/*
 * These tables are emitted by _Text_Engine_Tables.txt, which is included by
 * the installer after this C module has been linked.
 */
extern struct Glyph **FontGlyphsPointerTable[];
extern const u16 TextPaletteTable[];
extern const u16 TextBoxBgPaletteTable[];
extern const u32 *TextBoxTypePointerTable[];

extern const struct ProcCmd gProcScr_TalkSkipListener[];
extern const struct ProcCmd gProcScr_TalkPause[];
extern const struct ProcCmd gProcScr_TalkShiftClearAll[];
extern const struct ProcCmd gProcScr_TalkFaceMove[];
extern const struct ChoiceEntryInfo gYesNoTalkChoice[];
extern const struct ChoiceEntryInfo gBuySellTalkChoice[];

typedef void (*TextEngineUnsetFaceDisplayBitsFunc)(int position);

#define TextEngineUnsetFaceDisplayBits \
	((TextEngineUnsetFaceDisplayBitsFunc)(uintptr_t)0x080089C5)

int TalkInterpret(ProcPtr proc);
int GetStringTextWidthWithDialogueCodes(const char *text, int stopAtCurrentBox);
struct Proc *StartTalkFaceMove_C(int talkFaceFrom, int talkFaceTo, s8 isSwap);
void TextEngine_OnCharacterPrinted(void);

static void TextEngineFaceJump_OnIdle(struct TextEngineFaceJumpProc *proc);
static void TextEngineFaceJump_OnEnd(struct TextEngineFaceJumpProc *proc);
static void TextEnginePrintShake_OnIdle(struct TextEnginePrintShakeProc *proc);
static void TextEnginePrintShake_OnEnd(struct TextEnginePrintShakeProc *proc);

static const s8 sTextEnginePrintShakeOffsets[][2] = {
	{ +1, -1 },
	{ -1, +1 },
	{ +1,  0 },
	{  0,  0 },
};

static const struct ProcCmd gProcScr_TextEngineFaceJump[] = {
	PROC_NAME("TextEngineFaceJump"),
	PROC_SET_END_CB(TextEngineFaceJump_OnEnd),
	PROC_REPEAT(TextEngineFaceJump_OnIdle),
	PROC_END,
};

static const struct ProcCmd gProcScr_TextEnginePrintShake[] = {
	PROC_NAME("TextEnginePrintShake"),
	PROC_SET_END_CB(TextEnginePrintShake_OnEnd),
	PROC_REPEAT(TextEnginePrintShake_OnIdle),
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

static u8 *TextEngine_GetCurrentSpeakerAttributes(void)
{
	return (u8 *)sTextEngineState + 0x58;
}

static u8 *TextEngine_GetShakePrintFlag(void)
{
	return (u8 *)sTextEngineState + TEXT_ENGINE_SHAKE_PRINT_FLAG_OFFSET;
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
	if (proc->face)
		proc->face->yPos += proc->offset;
}

static void TextEngineFaceJump_OnIdle(struct TextEngineFaceJumpProc *proc)
{
	struct FaceProc *face = proc->face;
	int phase;
	int offset;
	s16 trueY;

	if (!face) {
		Proc_End(proc);
		return;
	}

	/*
	 * Undo the previous frame's offset first so other systems that also
	 * touch yPos (face moves, fades) compose cleanly with continuous jump.
	 */
	trueY = face->yPos + proc->offset;
	proc->timer++;

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

	face->yPos = trueY - offset;
	proc->offset = offset;
}

static void TextEngine_StartFaceJump(struct FaceProc *face)
{
	struct TextEngineFaceJumpProc *jump;

	if (!face || TextEngine_FindFaceJumpProc(face))
		return;

	jump = (struct TextEngineFaceJumpProc *)Proc_Start(
		gProcScr_TextEngineFaceJump,
		face
	);
	if (!jump)
		return;

	jump->face = face;
	jump->timer = 0;
	jump->offset = 0;
}

static void TextEngine_StopFaceJump(struct FaceProc *face)
{
	struct TextEngineFaceJumpProc *jump = TextEngine_FindFaceJumpProc(face);

	if (jump)
		Proc_End(jump);
}

static void TextEnginePrintShake_Apply(struct TextEnginePrintShakeProc *proc, int frame)
{
	const s8 *offset = sTextEnginePrintShakeOffsets[frame];

	BG_SetPosition(BG_0, proc->baseX + offset[0], proc->baseY + offset[1]);
}

static void TextEnginePrintShake_OnEnd(struct TextEnginePrintShakeProc *proc)
{
	BG_SetPosition(BG_0, proc->baseX, proc->baseY);
}

static void TextEnginePrintShake_OnIdle(struct TextEnginePrintShakeProc *proc)
{
	if (proc->timer >= TEXT_ENGINE_PRINT_SHAKE_FRAMES) {
		Proc_End(proc);
		return;
	}

	TextEnginePrintShake_Apply(proc, proc->timer);
	proc->timer++;
}

static void TextEngine_StartPrintShake(void)
{
	struct TextEnginePrintShakeProc *shake =
		(struct TextEnginePrintShakeProc *)Proc_Find(gProcScr_TextEnginePrintShake);

	if (shake) {
		/*
		 * Keep the original resting scroll and restart the punch so rapid
		 * glyphs keep jittering instead of stacking offsets forever.
		 */
		shake->timer = 0;
		TextEnginePrintShake_Apply(shake, 0);
		return;
	}

	shake = (struct TextEnginePrintShakeProc *)Proc_Start(
		gProcScr_TextEnginePrintShake,
		PROC_TREE_3
	);
	if (!shake)
		return;

	shake->timer = 0;
	shake->baseX = gLCDControlBuffer.bgoffset[BG_0].x;
	shake->baseY = gLCDControlBuffer.bgoffset[BG_0].y;
	TextEnginePrintShake_Apply(shake, 0);
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

	if (!CheckTalkFlag(TALK_FLAG_SPRITE)) {
		LoadObjUIGfx();
		BG_SetPosition(BG_0, 0, 0);
		BG_SetPosition(BG_1, 0, 0);
	}

	Proc_Start(gProcScr_TalkSkipListener, PROC_TREE_3);

	for (position = 0; position < (int)sizeof(defaultFaceXPositions); position++)
		positionTable[position] = defaultFaceXPositions[position];

	current[TEXT_ENGINE_ATTR_FONT] = 0;
	current[TEXT_ENGINE_ATTR_COLOR_GROUP] = 1;
	current[TEXT_ENGINE_ATTR_BOX_PALETTE] = 0;
	current[TEXT_ENGINE_ATTR_BOX_TYPE] = 0;
	current[TEXT_ENGINE_ATTR_BOOP_PITCH] = 13;
	*TextEngine_GetShakePrintFlag() = 0;
	Proc_EndEach(gProcScr_TextEnginePrintShake);
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

	InitTextFont(font, fontVram, fontTileOffset, 2);
	SetInitTalkTextFont();
	sTextEngineState->lines = lines;

	for (line = 0; line < 3; line++) {
		InitText(&texts[line], 30);
		Text_SetColor(&texts[line], 1);
	}

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
	return gBG0TilemapBuffer + TILEMAP_INDEX(
		state->xText,
		state->yText + 6
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
	proc->unk66 = 8;
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
	SetActiveTalkFace(to);
}

static void TextEngine_LoadFace(ProcPtr parent, int options)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *face;
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

	faceId = faceText[0] | (faceText[1] << 8);
	if (faceId == 0xFFFF)
		faceId = GetUnitPortraitId(gActiveUnit);
	else
		faceId -= 0x100;

	face = TextEngine_GetFaceProcByPosition(position);

	if (face) {
		sub_80066E0(face, faceId);
	} else {
		face = StartFaceAuto(faceId, GetTalkFaceHPos(position) * 8, 80, faceDisplay);
		state->faces[position] = face;

		if (face) {
			StartFaceFadeIn(face);
			SetTalkFaceLayer(position, CheckTalkFlag(TALK_FLAG_4));
			StartTemporaryLock(parent, 8);
		}
	}

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
			u8 subCode;

			cursor++;
			subCode = *cursor;

			if (subCode > TEXT_ENGINE_MAX_EXTRA_CODE)
				continue;

			switch (subCode) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				cursor++;
				continue;

			case 0x05:
				NumberToStringAscii(state->userNumber, state->userNumberString);
				lineWidth += TextEngine_WidthInternal(
					(const u8 *)state->userNumberString,
					stopAtCurrentBox
				);
				cursor++;
				continue;

			case 0x06:
				lineWidth += TextEngine_WidthInternal(
					(const u8 *)state->userString,
					stopAtCurrentBox
				);
				cursor++;
				continue;

			case 0x07:
			case 0x08:
			case 0x09:
				cursor++;
				continue;

			case 0x0A:
			case 0x0B:
			case 0x0C:
			case 0x0D:
			case 0x0E:
			case 0x0F:
			case 0x10:
			case 0x11:
				activePosition = subCode - 0x0A;
				cursor++;
				continue;

			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
				continue;

			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1A:
			case 0x1B:
			case 0x21:
			case 0x24:
			case 0x25:
				cursor++;
				continue;

			case 0x20:
				lineWidth += GetStringTextLen(GetTacticianName());
				cursor++;
				continue;

			case 0x22:
			case 0x23:
				continue;

			case 0x26:
				UpdateFontGlyphSet(cursor[1] - 1);
				cursor += 2;
				continue;

			case 0x27:
				cursor += 3;
				continue;

			case 0x28:
			case 0x29:
			case 0x2A:
			case 0x2B:
			case 0x2C:
				cursor += 2;
				continue;

			case 0x2D:
				cursor += 5;
				continue;

			case 0x2E:
				cursor += 3;
				continue;

			case 0x2F:
				cursor += 9;
				continue;

			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
				activePosition = subCode - 0x30;
				cursor += 2;
				continue;

			case 0x38:
				cursor += 2;
				continue;

			case 0x39:
			case 0x3A:
			case 0x3B:
			case 0x3C:
				cursor++;
				continue;
			}
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
	PROC_REPEAT(DecompressTextBoxGraphics),
	PROC_END,
};

static int TextEngine_HandleExtendedCode(ProcPtr proc, u8 subCode)
{
	struct TalkState *state = sTextEngineState;
	u8 *argument = (u8 *)state->str;
	struct FaceProc *face;
	int colorGroup;

	switch (subCode) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		colorGroup = subCode + 1;
		return TextEngine_HandleVanillaColor(state, colorGroup);

	case 0x04:
		LockTalk(proc);
		return 3;

	case 0x05:
		NumberToStringAscii(state->userNumber, state->userNumberString);
		state->strBackup = state->str - 2;
		state->str = state->userNumberString;
		return TalkInterpret(proc);

	case 0x06:
		state->strBackup = state->str - 2;
		state->str = state->userString;
		return TalkInterpret(proc);

	case 0x07:
	case 0x08:
		return 3;

	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x22:
	case 0x23:
		return 0;

	case 0x09:
		return 0;

	case 0x0A:
	case 0x0B:
	case 0x0C:
	case 0x0D:
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
		TextEngine_CallMoveFaceAndWriteSpeed(
			state->activeFaceSlot,
			subCode - 0x0A,
			0
		);
		return 3;

	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
		face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
		if (face) {
			int blinkControl;

			switch (subCode) {
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
		}
		return 3;

	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
		face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
		if (face) {
			int eyeControl;

			switch (subCode) {
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
		}
		return 3;

	case 0x20:
		state->strBackup = state->str - 2;
		state->str = GetTacticianName();
		return TalkInterpret(proc);

	case 0x21:
		return TextEngine_HandleVanillaColor(state, 4);

	case 0x24:
		if (state->unk38)
			state->unk38(proc);
		return 3;

	case 0x25:
		state->invertedFlags = 3 - (state->invertedFlags & 1);
		return 3;

	case 0x26:
		TextEngine_SetCurrentAttributeAndFace(
			state->activeFaceSlot,
			TEXT_ENGINE_ATTR_FONT,
			argument[0] - 1
		);
		UpdateFontGlyphSet(argument[0] - 1);
		state->str++;
		return TalkInterpret(proc);

	case 0x27: {
		int group = argument[0] - 1;
		int palette = argument[1] - 1;
		int destination = gActiveFont->palid * 0x20 + group * 6 + 2;

		CopyToPaletteBuffer(&TextPaletteTable[palette * 3], destination, 6);
		state->str += 2;
		return 3;
	}

	case 0x28:
		TextEngine_SetCurrentAttributeAndFace(
			state->activeFaceSlot,
			TEXT_ENGINE_ATTR_COLOR_GROUP,
			argument[0] - 1
		);
		ChangeTextColorID(argument[0] - 1);
		state->str++;
		return TalkInterpret(proc);

	case 0x29:
		TextEngine_SetCurrentAttributeAndFace(
			state->activeFaceSlot,
			TEXT_ENGINE_ATTR_BOX_PALETTE,
			argument[0] - 1
		);
		UpdateTextBoxBgPalette(argument[0] - 1);
		state->str++;
		return 3;

	case 0x2A:
		TextEngine_SetCurrentAttributeAndFace(
			state->activeFaceSlot,
			TEXT_ENGINE_ATTR_BOX_TYPE,
			argument[0] - 1
		);
		state->str++;
		return TalkInterpret(proc);

	case 0x2B:
		state->lines = argument[0];
		state->str++;
		return TalkInterpret(proc);

	case 0x2C:
		TextEngine_SetCurrentAttributeAndFace(
			state->activeFaceSlot,
			TEXT_ENGINE_ATTR_BOOP_PITCH,
			argument[0] - 1
		);
		state->str++;
		return TalkInterpret(proc);

	case 0x2D: {
		u16 song = (argument[0] & 0xF)
			| ((argument[1] & 0xF) << 4)
			| ((argument[2] & 0xF) << 8)
			| ((argument[3] & 0xF) << 12);

		m4aSongNumStart(song);
		state->str += 4;
		return 3;
	}

	case 0x2E: {
		int position = argument[0] - 1;
		u8 x = argument[1];

		if (x == 0x80)
			x = 0;

		((u8 *)state + 0x50)[position] = x;
		state->str += 2;
		return TalkInterpret(proc);
	}

	case 0x2F: {
		u8 options = argument[2] & 0x7F;
		u8 *faceAttributes;

		TextEngine_LoadFace(proc, options & 1);

		face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
		faceAttributes = TextEngine_GetFaceAttributes(face);

		if (faceAttributes) {
			faceAttributes[TEXT_ENGINE_ATTR_FONT] = argument[3] - 1;
			faceAttributes[TEXT_ENGINE_ATTR_COLOR_GROUP] = argument[4] - 1;
			faceAttributes[TEXT_ENGINE_ATTR_BOX_PALETTE] = argument[5] - 1;
			faceAttributes[TEXT_ENGINE_ATTR_BOX_TYPE] = argument[6] - 1;
			faceAttributes[TEXT_ENGINE_ATTR_BOOP_PITCH] = argument[7] - 1;
		}

		if ((options & 2) && face)
			sub_80064D4(face, 2);

		state->str += 8;
		return 3;
	}

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
		TextEngine_CallMoveFaceAndWriteSpeed(
			state->activeFaceSlot,
			subCode - 0x30,
			argument[0]
		);
		state->str++;
		return 3;

	case 0x38:
		if (argument[0] == 0xFF)
			state->printDelay = GetTextDisplaySpeed();
		else
			state->printDelay = argument[0];

		state->str++;
		return TalkInterpret(proc);

	case 0x39:
		TextEngine_StartFaceJump(
			TextEngine_GetFaceProcByPosition(state->activeFaceSlot)
		);
		return 3;

	case 0x3A:
		TextEngine_StopFaceJump(
			TextEngine_GetFaceProcByPosition(state->activeFaceSlot)
		);
		return 3;

	case 0x3B:
		*TextEngine_GetShakePrintFlag() = 1;
		return 3;

	case 0x3C:
		*TextEngine_GetShakePrintFlag() = 0;
		Proc_EndEach(gProcScr_TextEnginePrintShake);
		return 3;

	default:
		return 1;
	}
}

LYN_REPLACE_CHECK(TalkInterpret);
int TalkInterpret(ProcPtr proc)
{
	struct TalkState *state = sTextEngineState;
	u8 *text;
	u8 code;
	struct FaceProc *face;

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
			code = text[1];
			state->str = (char *)(text + 2);

			if (code <= TEXT_ENGINE_MAX_EXTRA_CODE)
				return TextEngine_HandleExtendedCode(proc, code);

			return 1;
		}

		if (code > 0x1D)
			return 1;

		state->str = (char *)(text + 1);

		switch (code) {
		case CHFE_L_NL:
			if (state->putLines == 1 || state->lineActive == 1)
				state->lineActive++;

			state->putLines = 0;
			return 2;

		case CHFE_L_2NL:
			if (CheckTalkFlag(TALK_FLAG_7)) {
				TalkFlushAllLine();
				state->str++;
			} else if (!CheckTalkFlag(TALK_FLAG_INSTANTSHIFT)) {
				Proc_StartBlocking(gProcScr_TalkShiftClearAll, proc);
			} else {
				ClearTalkText();
			}

			state->str++;
			return 3;

		case CHFE_L_A:
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
			if (TalkHasCorrectBubble())
				ClearTalkBubble();

			face = TextEngine_GetFaceProcByPosition(state->activeFaceSlot);
			if (face) {
				StartFaceFadeOut(face);
				state->faces[state->activeFaceSlot] = NULL;
			}

			StartTemporaryLock(proc, 0x10);
			return 3;

		case CHFE_L_CloseSpeechFast:
		case CHFE_L_CloseSpeechSlow:
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
		case CHFE_L_DEnd:
			state->activeWidth = 2 + (
				GetStringTextWidthWithDialogueCodes(state->str, TalkHasCorrectBubble()) + 7
			) / 8;
			continue;

		default:
			return 1;
		}
	}
}
