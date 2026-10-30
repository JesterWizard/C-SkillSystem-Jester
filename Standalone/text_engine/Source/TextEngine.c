#include "TextEngine.h"

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
#include "constants/classes.h"
#include "constants/songs.h"


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
	 * The plate's last text column is never printed to; its top tile holds
	 * the scene tab's bottom corner, merged with the box's top edge.
	 */
	TEXT_ENGINE_NAMEPLATE_SPARE_COLUMN = TEXT_ENGINE_NAMEPLATE_WIDTH - 1,
	/* Talk-bubble tiles in BG1 VRAM: 0x10 corner, 0x11 top edge. */
	TEXT_ENGINE_BUBBLE_TILE_CORNER = 0x10,
	TEXT_ENGINE_BUBBLE_TILE_EDGE = 0x11,
	TEXT_ENGINE_BUBBLE_PALETTE = 3,
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
	/*
	 * Emotion marks sit past the float-glyph tiles (0x72-0x79).
	 * 0x7C-0x7F is one 16x16 sprite. Pal 0xB is not the map-sprite pal.
	 */
	TEXT_ENGINE_EMOTION_OBJ_CHR = 0x7C,
	TEXT_ENGINE_EMOTION_OBJ_PAL = 0xB,
	TEXT_ENGINE_EMOTION_SWEAT = 0,
	TEXT_ENGINE_EMOTION_ANGER = 1,
	TEXT_ENGINE_EMOTION_SWEAT_TIME = 56,
	TEXT_ENGINE_EMOTION_ANGER_AFFINE = 31,
	TEXT_ENGINE_EMOTION_ANGER_SCALE_MIN = 8,
	TEXT_ENGINE_EMOTION_ANGER_SCALE_MAX = 24,
	TEXT_ENGINE_EMOTION_ANGER_PULSE_HALF = 8,
	TEXT_ENGINE_EMOTION_ANGER_PULSES = 4,
	TEXT_ENGINE_EMOTION_ANGER_TIME =
		TEXT_ENGINE_EMOTION_ANGER_PULSE_HALF * 2 * TEXT_ENGINE_EMOTION_ANGER_PULSES,
	TEXT_ENGINE_CMD_SWEAT = 0x57,
	TEXT_ENGINE_CMD_ANGER = 0x58,
	/*
	 * Scene layout: one full-width box pinned to the bottom of the screen,
	 * portraits standing behind it, and the speaker's name on a tab that
	 * straddles the box's top-left edge.
	 */
	TEXT_ENGINE_CMD_SCENE_LAYOUT_ON = 0x59,
	TEXT_ENGINE_CMD_SCENE_LAYOUT_OFF = 0x5A,
	TEXT_ENGINE_SCENE_BOX_X = 0,
	TEXT_ENGINE_SCENE_BOX_WIDTH = 30,
	TEXT_ENGINE_SCENE_SCREEN_ROWS = 20,
	/* Pixels of a 96x80 mug that stand behind the box's top edge. */
	TEXT_ENGINE_SCENE_FACE_OVERLAP = 16,
	TEXT_ENGINE_TALK_FACE_HEIGHT = 80,
	TEXT_ENGINE_TALK_FACE_Y = 80,
	TEXT_ENGINE_HALF_BODY_FACE_Y = 0x20,
	/* Name tab: first tile column, and rows above yText (tab bottom = box top). */
	TEXT_ENGINE_SCENE_NAMEPLATE_X = 2,
	TEXT_ENGINE_SCENE_NAMEPLATE_Y_OFFSET = 2,
	/* Big text: letters at twice the size, spanning two dialogue lines. */
	TEXT_ENGINE_CMD_BIG_TEXT_ON = 0x5B,
	TEXT_ENGINE_CMD_BIG_TEXT_OFF = 0x5C,
	TEXT_ENGINE_BIG_TEXT_SCALE = 2,
	/* Big letters print no faster than this so each one lands on its own. */
	TEXT_ENGINE_BIG_TEXT_PRINT_DELAY = 5,
	/*
	 * Each landing letter is a 16x16 affine OBJ at double size.  OBJ VRAM is
	 * 2D here, so a slot is tiles chr, chr + 1, chr + 32 and chr + 33; rows
	 * 1-2, columns 0x1B-0x1E are free during dialogue.
	 */
	TEXT_ENGINE_BIG_POP_SLOTS = 2,
	TEXT_ENGINE_BIG_POP_OBJ_CHR = 0x3B,
	TEXT_ENGINE_BIG_POP_SLOT_STRIDE = 2,
	TEXT_ENGINE_BIG_POP_AFFINE = 29,
	TEXT_ENGINE_BIG_POP_DURATION = 10,
	/* Frame the letter is at its biggest; the box jolts here. */
	TEXT_ENGINE_BIG_POP_IMPACT = 2,
	TEXT_ENGINE_BIG_JOLT_FRAMES = 6,
	/*
	 * Popups (an "Objection!"-style cut-in) borrow the dialogue text tiles,
	 * BG0 and the text palette while the box is closed.
	 */
	TEXT_ENGINE_CMD_POPUP = 0x5D,
	TEXT_ENGINE_POPUP_SHAKE_FRAMES = 10,
	/* Earliest frame A or B can dismiss the popup, and when it ends by itself. */
	TEXT_ENGINE_POPUP_MIN_FRAMES = 24,
	TEXT_ENGINE_POPUP_FRAMES = 60,
	/*
	 * Auto-advance: a text box moves on by itself after a number of frames,
	 * once ([AutoAdvance]) or at every [A] ([ToggleOnAutoAdvance]).
	 */
	TEXT_ENGINE_CMD_AUTO_ADVANCE = 0x5E,
	TEXT_ENGINE_CMD_AUTO_ADVANCE_ON = 0x5F,
	TEXT_ENGINE_CMD_AUTO_ADVANCE_OFF = 0x60,
	/*
	 * Rainbow text: letters take three text colour groups in turn, and the
	 * groups' colours turn around a hue wheel a third of a turn apart.
	 */
	TEXT_ENGINE_CMD_RAINBOW_ON = 0x61,
	TEXT_ENGINE_CMD_RAINBOW_OFF = 0x62,
	TEXT_ENGINE_RAINBOW_GROUPS = 3,
	/* Frames from one hue in sTextEngineRainbowHues to the next. */
	TEXT_ENGINE_RAINBOW_HUE_FRAMES = 8,
	/*
	 * Listener dimming: portraits of characters who aren't speaking darken
	 * by this many 32nds over this many frames.
	 */
	TEXT_ENGINE_DIM_STRENGTH = 10,
	TEXT_ENGINE_DIM_FRAMES = 8,
	TEXT_ENGINE_DIM_FACE_SLOTS = 4,
	/* Its own palette, a halfbody's second one and a 32-colour overlay. */
	TEXT_ENGINE_DIM_MAX_PALETTES = 3,
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

struct TextEngineEmotionProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ struct FaceProc *face;
	/* 30 */ s16 timer;
	/* 32 */ u8 kind;
	/* 33 */ u8 blendActive;
	/* 34 */ u16 savedBldcnt;
	/* 36 */ u8 savedCoeffA;
	/* 37 */ u8 savedCoeffB;
	/* 38 */ u8 savedBlendY;
	/* 39 */ u8 savedWinBlend;
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

struct TextEngineSceneLayoutState {
	u8 enabled;
	/* A scene box is already up: speaker changes redraw it without the open animation. */
	u8 boxShown;
	/* The box being opened right now skips the slide and tile animation. */
	u8 instantOpen;
	/* Nameplate setting from before the layout was turned on. */
	u8 savedNameplate;
};

extern EWRAM_DATA struct TextEngineSceneLayoutState sTextEngineSceneLayoutState;

struct TextEngineBigTextState {
	u8 enabled;
	/* The active line holds big text, so the line below it is taken too. */
	u8 lineBelow;
	/* Print delay to restore when big text ends, if it had to be slowed. */
	s8 savedPrintDelay;
	u8 printDelayForced;
};

struct TextEngineAutoAdvanceState {
	/* Every [A] moves on by itself after this many frames; 0 is off. */
	u8 frames;
};

struct TextEngineAutoAdvanceProc {
	PROC_HEADER;
	struct Proc *wait;
	int timer;
};

struct TextEngineRainbowProc {
	PROC_HEADER;
	/* New letters take the rainbow groups. Off still animates printed ones. */
	u8 active;
	u8 letter;
	u8 savedColor;
	u8 palid;
	u16 clock;
	/* The rainbow groups' colours before, in the text and float palettes. */
	u16 saved[2][TEXT_ENGINE_RAINBOW_GROUPS * 3];
};

struct TextEngineDimProc {
	PROC_HEADER;
	/* Per gFaces slot: the face last seen there and how dim it is. */
	struct FaceProc *face[TEXT_ENGINE_DIM_FACE_SLOTS];
	u8 level[TEXT_ENGINE_DIM_FACE_SLOTS];
};

/* bmlib's palette fade proc, which fades portraits in and out. */
struct TextEnginePalFadeProc {
	PROC_HEADER;
	struct PalFadeSt *st;
};

struct TextEngineBigPopProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ const struct Glyph *glyph;
	/* 30 */ struct Text *top;
	/* 34 */ struct Text *bottom;
	/* 38 */ s16 x;
	/* 3A */ s16 y;
	/* 3C */ s16 cursorX;
	/* 3E */ u8 timer;
	/* 3F */ u8 slot;
	/* 40 */ u8 color;
	/* 41 */ u8 width;
	/* 42 */ u8 bake;
};

/* One entry of TextEnginePopupTable (Popups/Popups.event). */
struct TextEnginePopup {
	const void *tiles;   /* LZ77 4bpp tiles; tile 0 is blank */
	const void *map;     /* LZ77 tilemap, width x height entries */
	const u16 *palette;  /* 16 colors */
	u8 width;            /* tiles, at most 30 */
	u8 height;           /* tiles, at most 20 */
	u16 song;            /* sound effect on impact; 0 for none */
};

struct TextEnginePopupProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ const struct TextEnginePopup *popup;
	/* 30 */ s16 timer;
	/* 32 */ u16 savedPalette[16];
	/* 52 */ u16 tileCount;
	/* 54 */ u8 shown;
};

struct TextEngineBigJoltProc {
	/* 00 */ PROC_HEADER;
	/* 2C */ s16 timer;
	/* 2E */ s16 bg0X;
	/* 30 */ s16 bg0Y;
	/* 32 */ s16 bg1X;
	/* 34 */ s16 bg1Y;
};

extern EWRAM_DATA struct TextEngineBigTextState sTextEngineBigTextState;
extern EWRAM_DATA struct TextEngineAutoAdvanceState sTextEngineAutoAdvanceState;

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

/*
 * Generic event portraits that no class uses as its default portrait.  The
 * scene layout names a speaker with one of these after the listed class.
 */
struct TextEnginePortraitClassName {
	u8 portraitId;
	u8 classId;
};

static const struct TextEnginePortraitClassName sTextEngineGenericPortraitClasses[] = {
	{ 0x58, CLASS_CIVILIAN_M1 }, /* Civilian man 1 */
	{ 0x5A, CLASS_CIVILIAN_M1 }, /* Civilian man 2 */
	{ 0x5B, CLASS_CIVILIAN_M1 }, /* Civilian man 3 */
	{ 0x5D, CLASS_CIVILIAN_M1 }, /* Civilian teen boy */
	{ 0x5F, CLASS_CIVILIAN_F1 }, /* Civilian old woman */
	{ 0x60, CLASS_CIVILIAN_F1 }, /* Civilian woman 1 */
	{ 0x6A, CLASS_SOLDIER },     /* Soldier, blue */
	{ 0x6B, CLASS_SOLDIER },     /* Soldier, brown */
	{ 0x6C, CLASS_FIGHTER },     /* Fighter, silver */
	{ 0x6D, CLASS_FIGHTER },     /* Fighter, brown */
	{ 0x6F, CLASS_SOLDIER },     /* Soldier, green */
	{ 0x70, CLASS_SOLDIER },     /* Soldier, purple */
	{ 0x71, CLASS_SOLDIER },     /* Soldier, silver */
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
extern const struct TextEnginePopup TextEnginePopupTable[];

extern const struct ProcCmd gProcScr_TalkSkipListener[];
extern const struct ProcCmd gProcScr_TalkPause[];
extern const struct ProcCmd gProcScr_TalkWaitForInput[];
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
static void TextEngineEmotion_OnIdle(struct TextEngineEmotionProc *proc);
static void TextEngine_CleanupEmotion(void);
static void TextEngine_CleanupRainbow(void);
static void TextEngine_CleanupAutoAdvance(void);
static void TextEngine_StartDim(void);
static void TextEngine_CleanupDim(void);
static void TextEngine_RainbowColorLetter(struct Text *text, const char *str);

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

static const struct ProcCmd gProcScr_TextEngineEmotion[] = {
	PROC_NAME("TextEngineEmotion"),
	PROC_REPEAT(TextEngineEmotion_OnIdle),
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

static bool TextEngine_IsSceneLayoutActive(void)
{
	/* Sprite talk, bubble-less talk and battle quotes keep their own layouts. */
	return sTextEngineSceneLayoutState.enabled &&
		!CheckTalkFlag(TALK_FLAG_SPRITE) &&
		!CheckTalkFlag(TALK_FLAG_NOBUBBLE) &&
		!IsBattleDeamonActive();
}

static int TextEngine_GetSceneBoxHeight(void)
{
	return sTextEngineState->lines * 2 + 2;
}

static int TextEngine_GetSceneBoxY(void)
{
	return TEXT_ENGINE_SCENE_SCREEN_ROWS - TextEngine_GetSceneBoxHeight();
}

static int TextEngine_GetTalkFaceY(bool halfBody)
{
	/* Half-body mugs already stand on the bottom edge, behind any box. */
	if (halfBody)
		return TEXT_ENGINE_HALF_BODY_FACE_Y;

	if (TextEngine_IsSceneLayoutActive())
		return TextEngine_GetSceneBoxY() * 8
			+ TEXT_ENGINE_SCENE_FACE_OVERLAP
			- TEXT_ENGINE_TALK_FACE_HEIGHT;

	return TEXT_ENGINE_TALK_FACE_Y;
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

/*
 * Generic speakers have no character entry for their portrait.  Name them
 * after a class instead, the way the scene layout's tab shows "Soldier" for a
 * generic soldier: first the generic event portrait table, then the class
 * that lists the portrait as its default.  Entries whose number does not
 * match their slot are past the class table.
 */
static u16 TextEngine_FindClassNameByPortrait(int portraitId)
{
	const struct ClassData *job;
	int classId;
	int i;

	if (portraitId <= 0)
		return 0;

	for (i = 0; i < (int)ARRAY_COUNT(sTextEngineGenericPortraitClasses); i++) {
		if (sTextEngineGenericPortraitClasses[i].portraitId != portraitId)
			continue;

		job = GetClassData(sTextEngineGenericPortraitClasses[i].classId);
		if (job && job->nameTextId)
			return job->nameTextId;
	}

	for (classId = 1; classId < 0x100; classId++) {
		job = GetClassData(classId);

		if (!job || job->number != classId)
			continue;

		if (job->nameTextId && job->defaultPortraitId == portraitId)
			return job->nameTextId;
	}

	return 0;
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

static int TextEngine_GetTilePixel(const u8 *tile, int x, int y)
{
	return (tile[y * 4 + x / 2] >> ((x & 1) * 4)) & 0xF;
}

/*
 * The scene tab's bottom row replaces the box's top-edge tiles on BG1, so the
 * see-through pixels outside its rounded corners would show the scene.  Build
 * the bottom-left corner (bubble corner flipped down) with those pixels taken
 * from the box's top edge, so the box line and fill continue behind the tab.
 * Returns the BG1 tile number, or -1 when BG1 cannot reach the spare tile.
 */
static int TextEngine_BuildSceneTabCornerTile(void)
{
	struct Font *font = (struct Font *)0x030000F0;
	u8 *bg1Chr = (u8 *)(VRAM + GetBackgroundTileDataOffset(BG_1));
	const u8 *corner = bg1Chr + TEXT_ENGINE_BUBBLE_TILE_CORNER * CHR_SIZE;
	const u8 *edge = bg1Chr + TEXT_ENGINE_BUBBLE_TILE_EDGE * CHR_SIZE;
	u8 *dest = (u8 *)font->vramDest + (
		sTextEngineNameplateState.text.chr_position
		+ TEXT_ENGINE_NAMEPLATE_SPARE_COLUMN
	) * 2 * CHR_SIZE;
	int tile = (dest - bg1Chr) / CHR_SIZE;
	int x;
	int y;
	int i;

	if (dest < bg1Chr || tile >= 0x400)
		return -1;

	/* VRAM takes 16-bit writes: four pixels at a time. */
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x += 4) {
			u16 pixels = 0;

			for (i = 0; i < 4; i++) {
				int color = TextEngine_GetTilePixel(corner, x + i, 7 - y);

				if (!color)
					color = TextEngine_GetTilePixel(edge, x + i, y);

				pixels |= color << (i * 4);
			}

			*(u16 *)(dest + y * 4 + x / 2) = pixels;
		}
	}

	return tile;
}

/*
 * The tab is drawn when the box starts opening; rebuild its corner once the
 * final box graphics are in so it does not keep an opening-frame tile.
 */
static void TextEngine_RefreshSceneTabCorner(void)
{
	if (sTextEngineNameplateState.active == 1 && TextEngine_IsSceneLayoutActive())
		TextEngine_BuildSceneTabCornerTile();
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
	bool sceneLayout;

	(void)proc;

	/* The dialogue is frozen behind the chatlog; do not redraw over it. */
	if (Chatlog_IsVisible())
		return;

	TextEngine_ClearSpeakerNameplate();

	/* 2 = tried and drew nothing; the next clear allows another try. */
	sTextEngineNameplateState.active = 2;

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

	sceneLayout = TextEngine_IsSceneLayoutActive();

	if (!nameTextId) {
		character = TextEngine_FindCharacterByPortrait(face->faceId);
		if (character)
			nameTextId = character->nameTextId;
		else if (sceneLayout)
			nameTextId = TextEngine_FindClassNameByPortrait(face->faceId);
	}

	if (!nameTextId)
		return;

	name = GetStringFromIndex(nameTextId);
	if (!name || !*name)
		return;

	dialogueWidth = state->activeWidth;
	if (dialogueWidth <= 2)
		return;

	nameY = state->yText - (sceneLayout
		? TEXT_ENGINE_SCENE_NAMEPLATE_Y_OFFSET
		: TEXT_ENGINE_NAMEPLATE_Y_OFFSET);
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
	 * The scene layout pins it to the box's left edge as a tab instead.
	 * PutTalkBubbleTm width/height include the border tiles.
	 */
	innerWidth = (nameWidth + 7) / 8 + TEXT_ENGINE_NAMEPLATE_PAD_TILES;
	if (innerWidth < 1)
		innerWidth = 1;
	/* Keep clear of the column that holds the scene tab's corner tile. */
	if (innerWidth > TEXT_ENGINE_NAMEPLATE_SPARE_COLUMN)
		innerWidth = TEXT_ENGINE_NAMEPLATE_SPARE_COLUMN;
	if (innerWidth > dialogueWidth - 2)
		innerWidth = dialogueWidth - 2;

	bg1Width = innerWidth + 2;
	if (sceneLayout)
		bg1X = TEXT_ENGINE_SCENE_NAMEPLATE_X;
	else
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

	/* The scene tab's bottom row sits on the box's top edge: merge its corners. */
	if (sceneLayout) {
		int cornerTile = TextEngine_BuildSceneTabCornerTile();

		if (cornerTile >= 0) {
			u16 *bottom = gBG1TilemapBuffer + TILEMAP_INDEX(bg1X, bg1Y + 1);

			bottom[0] = TILEREF(cornerTile, TEXT_ENGINE_BUBBLE_PALETTE);
			bottom[bg1Width - 1] =
				TILEREF(cornerTile, TEXT_ENGINE_BUBBLE_PALETTE) + 0x400;
		}
	}
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

/*
 * 16x16 marks. 0 is clear.
 * Sweat: 1 outline, 2 lower shadow, 3 body, 5 shine, 6 lower edge, 7 glint.
 * Anger uses 4 only, and fades by rewriting that one entry.
 */
static const u16 sTextEngineEmotionPalette[] = {
	0,
	RGB(3, 4, 8),
	RGB(5, 16, 25),
	RGB(8, 23, 28),
	RGB(20, 5, 6),
	RGB(22, 28, 30),
	RGB(7, 17, 23),
	RGB(31, 31, 31),
	0, 0, 0, 0, 0, 0, 0, 0,
};

/* Matches Preview/sweat-drop.png. Point at the top, shine on the right. */
static const u8 sTextEngineSweatPixels[16 * 16] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,3,1,0,0,0,0,0,0,0,
	0,0,0,0,0,1,3,3,3,1,0,0,0,0,0,0,
	0,0,0,0,1,3,3,3,5,3,1,0,0,0,0,0,
	0,0,0,0,1,3,3,3,3,3,1,0,0,0,0,0,
	0,0,0,1,3,3,3,3,5,5,3,1,0,0,0,0,
	0,0,0,1,3,3,3,3,5,7,3,1,0,0,0,0,
	0,0,1,3,3,3,3,3,5,7,5,3,1,0,0,0,
	0,0,1,3,3,3,3,3,5,7,7,5,1,0,0,0,
	0,0,1,3,3,3,3,3,3,3,3,3,1,0,0,0,
	0,0,1,6,2,2,2,2,2,2,6,1,0,0,0,0,
	0,0,0,1,6,6,2,2,2,6,6,1,0,0,0,0,
	0,0,0,0,1,1,6,6,6,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,
};

/* Four separated arms around an open center, matching the anger mark. */
static const u8 sTextEngineAngerPixels[16 * 16] = {
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	4,4,4,4,4,4,4,0,0,4,4,4,4,4,4,4,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
	0,0,0,0,4,4,4,0,0,4,4,4,0,0,0,0,
};

static void TextEngine_UploadEmotionGfx(int kind)
{
	const u8 *pixels = kind == TEXT_ENGINE_EMOTION_ANGER
		? sTextEngineAngerPixels
		: sTextEngineSweatPixels;
	int tile;

	ApplyPalette(
		sTextEngineEmotionPalette,
		0x10 + TEXT_ENGINE_EMOTION_OBJ_PAL
	);

	for (tile = 0; tile < 4; tile++) {
		int tx = tile & 1;
		int ty = tile >> 1;
		u32 *dst = (u32 *)OBJ_CHR_ADDR(TEXT_ENGINE_EMOTION_OBJ_CHR + tile);
		int row;

		for (row = 0; row < 8; row++) {
			u32 word = 0;
			int col;

			for (col = 0; col < 8; col++) {
				u32 px = pixels[(ty * 8 + row) * 16 + tx * 8 + col] & 0xF;

				word |= px << (col * 4);
			}

			dst[row] = word;
		}
	}
}

/* 16 is normal size. Each pulse grows from half size to 1.5x, then shrinks back. */
static int TextEngine_AngerScale(int timer)
{
	int half = TEXT_ENGINE_EMOTION_ANGER_PULSE_HALF;
	int phase = timer % (half * 2);

	if (phase < half) {
		return Interpolate(
			INTERPOLATE_LINEAR,
			TEXT_ENGINE_EMOTION_ANGER_SCALE_MIN,
			TEXT_ENGINE_EMOTION_ANGER_SCALE_MAX,
			phase,
			half - 1
		);
	}

	return Interpolate(
		INTERPOLATE_LINEAR,
		TEXT_ENGINE_EMOTION_ANGER_SCALE_MAX,
		TEXT_ENGINE_EMOTION_ANGER_SCALE_MIN,
		phase - half,
		half - 1
	);
}

static void TextEngineEmotion_OnIdle(struct TextEngineEmotionProc *proc)
{
	struct FaceProc *face = proc->face;
	int duration;
	int x;
	int y;
	int fall;
	int angerScale = 16;

	if (!face) {
		Proc_End(proc);
		return;
	}

	if (Chatlog_IsVisible())
		return;

	duration = proc->kind == TEXT_ENGINE_EMOTION_ANGER
		? TEXT_ENGINE_EMOTION_ANGER_TIME
		: TEXT_ENGINE_EMOTION_SWEAT_TIME;

	if (proc->timer >= duration) {
		Proc_End(proc);
		return;
	}

	/* Blink out over the last few frames. Anger pulses instead. */
	if (proc->kind != TEXT_ENGINE_EMOTION_ANGER &&
		proc->timer + 6 >= duration && (proc->timer & 1)) {
		proc->timer++;
		return;
	}

	/*
	 * Eye chip is 32x16. Sweat sits on the cheek under that chip.
	 * The cross sits on the forehead, centered on the eyes.
	 */
	if (face->pFaceInfo) {
		int flipped = GetFaceDisplayBits(face) & FACE_DISP_FLIPPED;

		x = 4 - face->pFaceInfo->xEyes;
		x = flipped ? x : -x;
		x = x * 8 + face->xPos - 16;
		y = face->yPos + face->pFaceInfo->yEyes * 8;
	} else {
		x = face->xPos + 24;
		y = face->yPos + 24;
	}

	if (proc->kind == TEXT_ENGINE_EMOTION_ANGER) {
		x += 4;
		y -= 14;
		angerScale = TextEngine_AngerScale(proc->timer);
	} else {
		if (GetFaceDisplayBits(face) & FACE_DISP_FLIPPED)
			x -= 12;
		else
			x += 4;
		y -= 6;
		fall = proc->timer < 8 ? proc->timer : 8;
		y = Interpolate(INTERPOLATE_SQUARE, y - 8, y, fall, 8);
	}

	if (y < 0)
		y = 0;

	/*
	 * Dialogue OBJ VRAM is 2D, so one 16x16 reads its bottom tiles 32
	 * slots later and only the top half appears. Four 8x8s use the
	 * consecutive tiles we actually uploaded.
	 */
	if (y > DISPLAY_HEIGHT - 16)
		y = DISPLAY_HEIGHT - 16;

	if (proc->kind == TEXT_ENGINE_EMOTION_ANGER) {
		int pa = (256 * 16) / angerScale;
		int spread = angerScale / 4;

		SetObjAffine(TEXT_ENGINE_EMOTION_ANGER_AFFINE, pa, 0, 0, pa);

		for (fall = 0; fall < 4; fall++) {
			int sx = x + ((fall & 1) ? spread : -spread);
			int sy = y + ((fall & 2) ? spread : -spread);

			PutSpriteExt(
				0,
				TextEngine_ApplyStaticOam1(
					OAM1_X(sx) | OAM1_AFFINE_ID(TEXT_ENGINE_EMOTION_ANGER_AFFINE),
					sy
				),
				TextEngine_ApplyFxOam0(
					OAM0_Y(sy) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE
				),
				gObject_8x8,
				OAM2_CHR(TEXT_ENGINE_EMOTION_OBJ_CHR + fall)
					| OAM2_PAL(TEXT_ENGINE_EMOTION_OBJ_PAL)
					| OAM2_LAYER(0)
			);
		}
	} else {
		for (fall = 0; fall < 4; fall++) {
			int sx = x + (fall & 1) * 8;
			int sy = y + (fall >> 1) * 8;

			PutSpriteExt(
				0,
				TextEngine_ApplyStaticOam1(OAM1_X(sx), sy),
				TextEngine_ApplyFxOam0(OAM0_Y(sy)),
				gObject_8x8,
				OAM2_CHR(TEXT_ENGINE_EMOTION_OBJ_CHR + fall)
					| OAM2_PAL(TEXT_ENGINE_EMOTION_OBJ_PAL)
					| OAM2_LAYER(0)
			);
		}
	}
	proc->timer++;
}

static void TextEngine_StartEmotion(struct FaceProc *face, int kind, ProcPtr parent)
{
	struct TextEngineEmotionProc *proc;

	if (!face)
		return;

	Proc_EndEach(gProcScr_TextEngineEmotion);
	TextEngine_UploadEmotionGfx(kind);

	proc = (struct TextEngineEmotionProc *)Proc_StartBlocking(
		gProcScr_TextEngineEmotion,
		parent
	);
	if (!proc)
		return;

	proc->face = face;
	proc->timer = 0;
	proc->kind = kind;
}

static void TextEngine_CleanupEmotion(void)
{
	Proc_EndEach(gProcScr_TextEngineEmotion);
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

static int TextEngineAsh_CountFaceChips(struct FaceProc *face, int step)
{
	struct ExceedPortraitPiece pieces[EXCEED_PORTRAIT_MAX_PIECES];

	/* A 16x16 exceed piece is one chip at step 16 or 32. */
	return TextEngineAsh_CountPieces(face->sprite, step) +
		ExceedPortrait_GetFacePieces(face, pieces);
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

/* Chips for one w x h OBJ of the face, at (objX, objY) from its origin. */
static void TextEngineAsh_DrawObjChips(
	struct TextEngineAshDissolveProc *proc,
	int objX,
	int objY,
	int w,
	int h,
	int hflip,
	int objChr,
	u32 *seed)
{
	struct FaceProc *face = proc->face;
	int t = proc->timer;
	int step = proc->step;
	int mosaic = (gLCDControlBuffer.mosaic >> 8) & 0xF;
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

			rnd = TextEngineAsh_NextRnd(seed);
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

static void TextEngineAsh_DrawChips(struct TextEngineAshDissolveProc *proc)
{
	struct FaceProc *face = proc->face;
	struct ExceedPortraitPiece pieces[EXCEED_PORTRAIT_MAX_PIECES];
	const u16 *sprite;
	const u16 *obj;
	int count;
	int i;
	u32 seed;

	if (face == NULL || face->sprite == NULL || proc->step <= 0)
		return;

	sprite = face->sprite;
	count = sprite[0];
	obj = sprite + 1;
	seed = proc->seed;

	for (i = 0; i < count; i++, obj += 3) {
		int shape = obj[0] >> 14;
		int size = obj[1] >> 14;

		TextEngineAsh_DrawObjChips(
			proc,
			TextEngineAsh_OamX(obj[1]),
			TextEngineAsh_OamY(obj[0]),
			sTextEngineAshObjWidth[shape][size],
			sTextEngineAshObjHeight[shape][size],
			(obj[1] & OAM1_HFLIP) != 0,
			obj[2] & 0x3FF,
			&seed);
	}

	/* Exceed-portrait pieces dissolve with the rest of the face. */
	count = ExceedPortrait_GetFacePieces(face, pieces);
	for (i = 0; i < count; i++) {
		TextEngineAsh_DrawObjChips(
			proc,
			pieces[i].x,
			pieces[i].y,
			16,
			16,
			pieces[i].hflip,
			pieces[i].chr,
			&seed);
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

	count = TextEngineAsh_CountFaceChips(face, 16);
	if (count > TEXT_ENGINE_ASH_OAM_BUDGET)
		step = 32;

	count = TextEngineAsh_CountFaceChips(face, step);
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
	bool chatlogVisible = Chatlog_IsVisible();
	int line;

	for (line = 0; line < DISPLAY_HEIGHT; line++) {
		s16 offset = 0;
		int intensity = 0;

		if (!chatlogVisible) {
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

/*
 * Every screen fx proc installs TextEngineWave_OnHBlank, so without it there
 * is no offset. Map sprites call these per sprite every frame, and the
 * Proc_Find scans below are too slow to run that often.
 */
static bool TextEngine_IsScreenFxActive(void)
{
	return sHBlankHandler2 == TextEngineWave_OnHBlank;
}

s16 TextEngine_GetStaticOffsetAtY(int y)
{
	s16 offset = 0;

	if (!TextEngine_IsScreenFxActive())
		return 0;

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
	if (!TextEngine_IsScreenFxActive())
		return 0;

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
	int screenY = OAM0_Y(yOam0);

	if (object == NULL)
		return;

	/* The slip row is the Y field only; OAM Y 160-255 is above the screen. */
	if (screenY >= DISPLAY_HEIGHT)
		screenY -= 0x100;

	/*
	 * PutSpriteExt stores this object pointer until OAM flush, so the data
	 * must outlive this call. Apply one scanline slip at the face Y instead
	 * of rewriting pieces into a stack buffer.
	 */
	PutSpriteExt(
		layer,
		TextEngine_ApplyStaticOam1(xOam1, screenY),
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
	int savedColor;

	SetInitTalkTextFont();
	savedCursor = Text_GetCursor(proc->targetText);
	savedColor = Text_GetColor(proc->targetText);
	Text_SetColor(proc->targetText, proc->color);
	Text_SetCursor(proc->targetText, proc->cursorX);
	Text_DrawCharacter(proc->targetText, proc->ch);
	/*
	 * Text_DrawCharacter advances the cursor.  Later glyphs may already have
	 * reserved space past this one, so restore the live cursor afterward.
	 * The line's colour may have changed since this letter started too.
	 */
	Text_SetCursor(proc->targetText, savedCursor);
	Text_SetColor(proc->targetText, savedColor);
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

/*
 * Pixel-double one 8-column half of a glyph (half 0 = columns 0-7, 1 = 8-15)
 * into a 16x16 glyph, taking source rows firstRow to firstRow + 7.
 */
static void TextEngine_DoubleGlyphQuarter(
	const struct Glyph *source,
	struct Glyph *dest,
	int firstRow,
	int half
)
{
	int row;
	int px;

	for (row = 0; row < 16; row++) {
		u32 bits = source->bitmap[firstRow + row / 2] >> (half * 16);
		u32 doubled = 0;

		for (px = 0; px < 8; px++) {
			u32 color = (bits >> (px * 2)) & 3;

			doubled |= (color | (color << 2)) << (px * 4);
		}

		dest->bitmap[row] = doubled;
	}
}

/*
 * Draw a pixel-doubled glyph at cursor x: top half into one line's Text and
 * bottom half into the next.  The lines' live cursors are left untouched.
 */
static void TextEngine_DrawDoubledGlyph(
	struct Text *top,
	struct Text *bottom,
	const struct Glyph *glyph,
	int x,
	int width,
	int color
)
{
	struct Glyph big;
	int topCursor = Text_GetCursor(top);
	int bottomCursor = Text_GetCursor(bottom);
	int half;

	SetInitTalkTextFont();
	Text_SetColor(top, color);
	Text_SetColor(bottom, color);

	big.width = 16;
	for (half = 0; half < 2; half++) {
		/* Narrow letters have nothing in their right half. */
		if (half && width <= 8)
			break;

		TextEngine_DoubleGlyphQuarter(glyph, &big, 0, half);
		Text_SetCursor(top, x + half * 16);
		gActiveFont->drawGlyph(top, &big);

		TextEngine_DoubleGlyphQuarter(glyph, &big, 8, half);
		Text_SetCursor(bottom, x + half * 16);
		gActiveFont->drawGlyph(bottom, &big);
	}

	Text_SetCursor(top, topCursor);
	Text_SetCursor(bottom, bottomCursor);
}

/*
 * Landing curve, in 1/16 of the glyph's normal size: burst from small to
 * 2.5x, spring back under 2x, then settle on 2x, the baked big-text size.
 */
static const u8 sTextEngineBigPopScale[TEXT_ENGINE_BIG_POP_DURATION] = {
	12, 26, 40, 40, 35, 30, 30, 33, 32, 32,
};

/* Tilt while flying in, as cos/sin * 256: -14, +10, -4 degrees, then level. */
static const s16 sTextEngineBigPopTilt[TEXT_ENGINE_BIG_POP_DURATION][2] = {
	{ 248, -62 },
	{ 252, 44 },
	{ 255, -18 },
	{ 256, 0 }, { 256, 0 }, { 256, 0 }, { 256, 0 },
	{ 256, 0 }, { 256, 0 }, { 256, 0 },
};

static const s8 sTextEngineBigJoltOffsets[TEXT_ENGINE_BIG_JOLT_FRAMES][2] = {
	{ +2, -2 },
	{ -2, +1 },
	{ +1, +2 },
	{ -1, -1 },
	{ +1,  0 },
	{  0,  0 },
};

static void TextEngineBigJolt_Apply(struct TextEngineBigJoltProc *proc)
{
	int x = 0;
	int y = 0;

	if (proc->timer < TEXT_ENGINE_BIG_JOLT_FRAMES) {
		x = sTextEngineBigJoltOffsets[proc->timer][0];
		y = sTextEngineBigJoltOffsets[proc->timer][1];
	}

	BG_SetPosition(BG_0, proc->bg0X + x, proc->bg0Y + y);
	BG_SetPosition(BG_1, proc->bg1X + x, proc->bg1Y + y);
}

static void TextEngineBigJolt_OnEnd(struct TextEngineBigJoltProc *proc)
{
	proc->timer = TEXT_ENGINE_BIG_JOLT_FRAMES;
	TextEngineBigJolt_Apply(proc);
}

static void TextEngineBigJolt_OnIdle(struct TextEngineBigJoltProc *proc)
{
	proc->timer++;
	TextEngineBigJolt_Apply(proc);

	if (proc->timer >= TEXT_ENGINE_BIG_JOLT_FRAMES)
		Proc_End(proc);
}

static const struct ProcCmd gProcScr_TextEngineBigJolt[] = {
	PROC_NAME("TextEngineBigJolt"),
	PROC_SET_END_CB(TextEngineBigJolt_OnEnd),
	PROC_REPEAT(TextEngineBigJolt_OnIdle),
	PROC_END,
};

/* Shake the box and its text together; a new landing restarts the shake. */
static void TextEngine_StartBigJolt(void)
{
	struct TextEngineBigJoltProc *proc =
		(struct TextEngineBigJoltProc *)Proc_Find(gProcScr_TextEngineBigJolt);

	if (!proc) {
		proc = (struct TextEngineBigJoltProc *)Proc_Start(
			gProcScr_TextEngineBigJolt,
			PROC_TREE_3
		);
		if (!proc)
			return;

		proc->bg0X = gLCDControlBuffer.bgoffset[BG_0].x;
		proc->bg0Y = gLCDControlBuffer.bgoffset[BG_0].y;
		proc->bg1X = gLCDControlBuffer.bgoffset[BG_1].x;
		proc->bg1Y = gLCDControlBuffer.bgoffset[BG_1].y;
	}

	proc->timer = 0;
	TextEngineBigJolt_Apply(proc);
}

static int TextEngine_GetBigPopObjChr(int slot)
{
	return TEXT_ENGINE_BIG_POP_OBJ_CHR + slot * TEXT_ENGINE_BIG_POP_SLOT_STRIDE;
}

/* Pack the normal-size glyph into the slot's 2x2 OBJ tiles (2D layout). */
static void TextEngine_UploadBigPopGlyph(const struct Glyph *glyph, int slot, int color)
{
	u16 *lut = GetColorLut(color);
	int chr = TextEngine_GetBigPopObjChr(slot);
	u32 *tiles[4];
	int row;
	int i;

	tiles[0] = (u32 *)OBJ_CHR_ADDR(chr);
	tiles[1] = (u32 *)OBJ_CHR_ADDR(chr + 1);
	tiles[2] = (u32 *)OBJ_CHR_ADDR(chr + 32);
	tiles[3] = (u32 *)OBJ_CHR_ADDR(chr + 33);

	for (row = 0; row < 16; row++) {
		u32 bits = glyph->bitmap[row];

		for (i = 0; i < 2; i++) {
			u32 half = bits >> (i * 16);

			tiles[(row / 8) * 2 + i][row & 7] =
				lut[half & 0xFF] | ((u32)lut[(half >> 8) & 0xFF] << 16);
		}
	}
}

/* OBJ palette for the landing letters: the live dialogue text palette. */
static void TextEngine_PrepareBigPopPalette(void)
{
	struct Font *font = (struct Font *)0x030000F0;

	CpuCopy16(
		gPaletteBuffer + font->palid * 0x10,
		gPaletteBuffer + (0x10 + TEXT_ENGINE_FLOAT_OBJ_PAL) * 0x10,
		0x20
	);
	EnablePaletteSync();
}

static void TextEngineBigPop_OnEnd(struct TextEngineBigPopProc *proc)
{
	int chr = TextEngine_GetBigPopObjChr(proc->slot);

	CpuFastFill(0, (void *)OBJ_CHR_ADDR(chr), CHR_SIZE * 2);
	CpuFastFill(0, (void *)OBJ_CHR_ADDR(chr + 32), CHR_SIZE * 2);

	if (!proc->bake)
		return;

	TextEngine_DrawDoubledGlyph(
		proc->top,
		proc->bottom,
		proc->glyph,
		proc->cursorX,
		proc->width,
		proc->color
	);
	TalkBgSync(1);
}

static void TextEngineBigPop_OnIdle(struct TextEngineBigPopProc *proc)
{
	int affine = TEXT_ENGINE_BIG_POP_AFFINE + proc->slot;
	int scale;
	int inverse;
	int cosine;
	int sine;
	int x = proc->x;
	int y = proc->y;

	if (Chatlog_IsVisible())
		return;

	if (proc->timer >= TEXT_ENGINE_BIG_POP_DURATION) {
		Proc_End(proc);
		return;
	}

	if (proc->timer == TEXT_ENGINE_BIG_POP_IMPACT)
		TextEngine_StartBigJolt();

	/* A little scatter while the letter is still flying in. */
	if (proc->timer < TEXT_ENGINE_BIG_POP_IMPACT) {
		x += (proc->timer & 1) ? 2 : -2;
		y += (proc->timer & 1) ? -1 : 1;
	}

	/* OBJ affine maps screen to texture: inverse scale, inverse rotation. */
	scale = sTextEngineBigPopScale[proc->timer];
	inverse = (256 * 16) / scale;
	cosine = sTextEngineBigPopTilt[proc->timer][0];
	sine = sTextEngineBigPopTilt[proc->timer][1];
	SetObjAffine(
		affine,
		(cosine * inverse) >> 8,
		(sine * inverse) >> 8,
		(-sine * inverse) >> 8,
		(cosine * inverse) >> 8
	);

	/* Double-size 16x16: the 32x32 frame matches the baked letter at 2x. */
	PutSpriteExt(
		0,
		TextEngine_ApplyStaticOam1(OAM1_X(x) | OAM1_AFFINE_ID(affine), y),
		TextEngine_ApplyFxOam0(OAM0_Y(y) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE),
		gObject_16x16,
		OAM2_CHR(TextEngine_GetBigPopObjChr(proc->slot))
			| OAM2_PAL(TEXT_ENGINE_FLOAT_OBJ_PAL)
			| OAM2_LAYER(0)
	);

	proc->timer++;
}

static const struct ProcCmd gProcScr_TextEngineBigPop[] = {
	PROC_NAME("TextEngineBigPop"),
	PROC_SET_END_CB(TextEngineBigPop_OnEnd),
	PROC_REPEAT(TextEngineBigPop_OnIdle),
	PROC_END,
};

static int TextEngine_FindFreeBigPopSlot(void)
{
	struct ProcFindIterator it;
	struct TextEngineBigPopProc *proc;
	u8 used = 0;
	int slot;

	Proc_FindBegin(&it, gProcScr_TextEngineBigPop);
	while ((proc = (struct TextEngineBigPopProc *)Proc_FindNext(&it)) != NULL)
		used |= 1 << proc->slot;

	for (slot = 0; slot < TEXT_ENGINE_BIG_POP_SLOTS; slot++) {
		if (!(used & (1 << slot)))
			return slot;
	}

	return -1;
}

/*
 * End every landing letter.  bake 1 draws them into the text first (lines
 * about to scroll); bake 0 drops them (the box they belonged to is gone).
 */
static void TextEngine_EndBigPops(u8 bake)
{
	struct ProcFindIterator it;
	struct TextEngineBigPopProc *proc;

	Proc_FindBegin(&it, gProcScr_TextEngineBigPop);
	while ((proc = (struct TextEngineBigPopProc *)Proc_FindNext(&it)) != NULL)
		proc->bake = bake;

	Proc_EndEach(gProcScr_TextEngineBigPop);
}

/* Returns 1 when the letter will land as an animated sprite. */
static s8 TextEngine_StartBigPop(
	struct Text *top,
	struct Text *bottom,
	const struct Glyph *glyph,
	int cursorX,
	int width
)
{
	struct TalkState *state = sTextEngineState;
	struct TextEngineBigPopProc *proc;
	int slot;

	/* Skipping ahead prints at once, without the landing. */
	if (state->instantScroll)
		return 0;

	slot = TextEngine_FindFreeBigPopSlot();
	if (slot < 0)
		return 0;

	proc = (struct TextEngineBigPopProc *)Proc_Start(
		gProcScr_TextEngineBigPop,
		PROC_TREE_3
	);
	if (!proc)
		return 0;

	proc->glyph = glyph;
	proc->top = top;
	proc->bottom = bottom;
	proc->cursorX = cursorX;
	proc->x = state->xText * 8 + cursorX;
	proc->y = state->yText * 8 + state->lineActive * 16;
	proc->timer = 0;
	proc->slot = slot;
	proc->color = state->printColor;
	proc->width = width;
	proc->bake = 1;

	TextEngine_PrepareBigPopPalette();
	TextEngine_UploadBigPopGlyph(glyph, slot, proc->color);
	return 1;
}

/*
 * Big text draws each letter at twice the size: the top half goes into the
 * active line and the bottom half into the line below.  Each letter lands as
 * a sprite first, then is baked into the lines.  Normal-size letters that
 * follow on the same line go into the line below so they share the big
 * letters' baseline.  Returns 1 when the character was handled here.
 */
static s8 TextEngine_TryDrawBigGlyph(struct Text *text, const char **str)
{
	struct TalkState *state = sTextEngineState;
	struct Text *below;
	struct Glyph *glyph;
	char ch[5];
	const char *next;
	u32 width;
	int len;
	int x;
	int i;

	if (!sTextEngineBigTextState.enabled && !sTextEngineBigTextState.lineBelow)
		return 0;

	if (!text || !str || !*str || CheckTalkFlag(TALK_FLAG_SPRITE))
		return 0;

	/* Big text needs the line below; on the last line it prints normally. */
	if (state->lineActive + 1 >= state->lines)
		return 0;

	SetInitTalkTextFont();
	below = TextEngine_GetLineText(state, state->lineActive + 1);
	x = Text_GetCursor(text);

	if (!sTextEngineBigTextState.enabled) {
		Text_SetCursor(below, x);
		*str = Text_DrawCharacter(below, *str);
		Text_SetCursor(text, Text_GetCursor(below));
		return 1;
	}

	next = GetCharTextLen(*str, &width);
	if (!next || next == *str || width == 0)
		return 0;

	len = next - *str;
	if (len <= 0 || len > 4)
		return 0;

	for (i = 0; i < len; i++)
		ch[i] = (*str)[i];
	ch[len] = 0;

	glyph = TextEngine_ResolveGlyph(ch);
	if (!glyph)
		return 0;

	if (!sTextEngineBigTextState.lineBelow) {
		/* First big letter on this line: show the line below it too. */
		Text_SetColor(below, state->printColor);
		PutText(
			below,
			gBG0TilemapBuffer + TILEMAP_INDEX(
				state->xText,
				state->yText + (state->lineActive + 1) * 2
			)
		);
		TalkBgSync(1);
		sTextEngineBigTextState.lineBelow = 1;
	}

	if (!TextEngine_StartBigPop(text, below, glyph, x, width))
		TextEngine_DrawDoubledGlyph(text, below, glyph, x, width, state->printColor);

	Text_SetCursor(text, x + width * TEXT_ENGINE_BIG_TEXT_SCALE);
	Text_SetCursor(below, x + width * TEXT_ENGINE_BIG_TEXT_SCALE);
	*str = next;
	return 1;
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

	/* Page scrolls wipe BG0, nameplate included; put it back on the next letter. */
	if (sTextEngineNameplateState.enabled &&
		!sTextEngineNameplateState.active &&
		!CheckTalkFlag(TALK_FLAG_SPRITE) &&
		TalkHasCorrectBubble())
		TextEngine_DrawSpeakerNameplate(NULL);

	if (str)
		TextEngine_RainbowColorLetter(text, *str);

	if (TextEngine_TryDrawBigGlyph(text, str))
		return 1;

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
	struct FaceProc *face;
	u8 *faceAttributes;
	struct MusicPlayerInfo *mplayInfo;
	u8 pitch;

	/*
	 * Talk_OnIdle calls this after a glyph is drawn. UTF-8 lead bytes are
	 * >= 0x80, so a high-bit test would silence every non-ASCII letter.
	 */
	if (!text || *text == 0)
		return;

	if (gPlaySt.config.disableSoundEffects)
		return;

	face = TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot);
	faceAttributes = TextEngine_GetFaceAttributes(face);
	if (faceAttributes)
		pitch = faceAttributes[TEXT_ENGINE_ATTR_BOOP_PITCH];
	else
		pitch = TextEngine_GetCurrentSpeakerAttributes()[TEXT_ENGINE_ATTR_BOOP_PITCH];

	if (pitch >= TEXT_ENGINE_BOOP_PITCH_COUNT)
		return;

	/*
	 * Tequila's hook loads the word at gMPlayTable+0x24 and indexes
	 * TextBoopTable by 12 bytes per pitch.
	 */
	mplayInfo = *(struct MusicPlayerInfo **)((u8 *)gMPlayTable + 0x24);
	if (!mplayInfo)
		return;

	MPlayStart(
		mplayInfo,
		(struct SongHeader *)((const u8 *)TextBoopTable + pitch * 12)
	);
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
	TextEngine_GetCurrentSpeakerAttributes()[TEXT_ENGINE_ATTR_BOOP_PITCH] = 12;
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
	/* Script 0x0D is stored as index 12: one octave above the lowest boop. */
	current[TEXT_ENGINE_ATTR_BOOP_PITCH] = 12;
	TextEngine_RunCommandCleanup();
	TextEngine_PrepareFloatPalette();
	TextEngine_StartDim();
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
	Proc_EndEach(gProcScr_TextEngineEmotion);
	TextEngine_EndBigPops(0);
	Proc_EndEach(gProcScr_TextEngineBigJolt);
	TextEngine_CleanupAutoAdvance();
	TextEngine_CleanupRainbow();
	TextEngine_CleanupDim();
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

	/* Land letters still in flight, then scroll a big line's lower half out with it. */
	TextEngine_EndBigPops(1);
	state->lineActive += sTextEngineBigTextState.lineBelow;
	sTextEngineBigTextState.lineBelow = 0;

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
	TextEngine_EndBigPops(1);
	sTextEngineBigTextState.lineBelow = 0;
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

static void TextEngineSceneOpen_PutBox(struct Proc *proc)
{
	struct TalkState *state = sTextEngineState;
	int height = TextEngine_GetSceneBoxHeight();
	int y = TextEngine_GetSceneBoxY();

	/* Latch now: the tile animation clears instantOpen once it consumes it. */
	proc->unk64 = sTextEngineSceneLayoutState.instantOpen;

	/*
	 * The previous speaker's tab backed up box-border tiles; drop it before
	 * redrawing so a box of a different height cannot get them restored.
	 */
	TextEngine_ClearSpeakerNameplate();
	BG_Fill(gBG1TilemapBuffer, 0);

	state->xText = TEXT_ENGINE_SCENE_BOX_X + 1;
	state->yText = y + 1;

	PutTalkBubbleTm(
		BG_1,
		TEXT_ENGINE_SCENE_BOX_X,
		y,
		TEXT_ENGINE_SCENE_BOX_WIDTH,
		height
	);

	/* Same palette handoff as PutTalkBubble; the scene box has no tail. */
	if (state->invertedFlags & 2) {
		TalkToggleInvertedPalette(state->invertedFlags & 1);
		state->invertedFlags ^= 2;
	}

	InitTalkTextWin(
		TEXT_ENGINE_SCENE_BOX_X,
		y,
		TEXT_ENGINE_SCENE_BOX_WIDTH,
		height
	);

	if (proc->unk64) {
		/* The box was up for the previous speaker: land fully open. */
		BG_SetPosition(BG_1, 0, 0);
		if (!CheckTalkFlag(TALK_FLAG_8))
			SetBlendAlpha(16, 1);
	}

	sTextEngineSceneLayoutState.boxShown = 1;
	StartOpenTalkBubble();
	TalkBgSync(2);
}

static void TextEngineSceneOpen_OnIdle(struct Proc *proc)
{
	if (proc->unk64) {
		Proc_Break(proc);
		return;
	}

	/* Vanilla slide-up and blend fade. */
	TalkOpen_OnIdle(proc);
}

static const struct ProcCmd gProcScr_TextEngineSceneOpen[] = {
	PROC_MARK(PROC_MARK_TALK),
	PROC_CALL(TalkOpen_InitBlend),
	PROC_CALL(TextEngineSceneOpen_PutBox),
	PROC_REPEAT(TextEngineSceneOpen_OnIdle),
	PROC_END,
};

LYN_REPLACE_CHECK(StartTalkOpen);
void StartTalkOpen_C(int talkFace, ProcPtr parent)
{
	struct TalkState *state = sTextEngineState;
	struct Proc *proc;

	/* A new box starts with empty lines; drop letters from the old one. */
	TextEngine_EndBigPops(0);
	sTextEngineBigTextState.lineBelow = 0;

	if (TextEngine_IsSceneLayoutActive()) {
		state->activeWidth = TEXT_ENGINE_SCENE_BOX_WIDTH;
		sTextEngineSceneLayoutState.instantOpen =
			sTextEngineSceneLayoutState.boxShown;

		/*
		 * Set the speaker first: the scene-open proc draws the box and
		 * nameplate from PROC_CALLs that run inside Proc_StartBlocking.
		 */
		state->speakingFaceSlot = talkFace;
		state->speakingWidth = state->activeWidth;
		Proc_StartBlocking(gProcScr_TextEngineSceneOpen, parent);
		return;
	}

	sTextEngineSceneLayoutState.instantOpen = 0;
	proc = (struct Proc *)Proc_StartBlocking(gProcScr_TalkOpen, parent);

	proc->unk64 = GetTalkFaceHPos(talkFace);
	/* A half-body speaker's bubble runs along the bottom of the screen. */
	proc->unk66 = Portraits_IsHalfBodyFace(state->faces[talkFace]) ? 0x12 : 0x08;
	proc->unk68 = state->activeWidth;
	proc->unk6A = state->lines * 2 + 2;

	if (proc->unk64 < 0)
		proc->unk64 = 0;
	else if (proc->unk64 > 29)
		proc->unk64 = 30;

	state->speakingFaceSlot = talkFace;
	state->speakingWidth = state->activeWidth;
}

/*
 * Promotion screen description box. InitTalk_C clears three 30-tile lines
 * and the nameplate from tile 0x100 (0x100-0x1EF), which covers the box's
 * vanilla tiles at 0x180. Load it just past them instead, as vanilla does
 * for its two talk lines.
 */
#define CLASSCHG_UI_BOX_CHR 0x1F0

LYN_REPLACE_CHECK(ClassChgLoadUI);
void ClassChgLoadUI_C(void)
{
	Decompress(
		gUnknown_08A30800,
		(void *)(VRAM + (CLASSCHG_UI_BOX_CHR << 5) + GetBackgroundTileDataOffset(BG_2))
	);
	RegisterTsaWithOffset(
		gBG2TilemapBuffer,
		gUnknown_08A30978,
		TILEREF(CLASSCHG_UI_BOX_CHR, 1)
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
		face = StartFaceAuto(
			faceId,
			GetTalkFaceHPos(position) * 8,
			TextEngine_GetTalkFaceY(
				Portraits_TalkFaceWillBeHalfBody(faceId, faceDisplay)),
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
	int glyphScale = sTextEngineBigTextState.enabled ? TEXT_ENGINE_BIG_TEXT_SCALE : 1;

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

			/* A popup closes the box, so the box ends here. */
			if (command->code == TEXT_ENGINE_CMD_POPUP)
				goto width_done;

			if (command->code == TEXT_ENGINE_CMD_BIG_TEXT_ON)
				glyphScale = TEXT_ENGINE_BIG_TEXT_SCALE;
			else if (command->code == TEXT_ENGINE_CMD_BIG_TEXT_OFF)
				glyphScale = 1;

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
			lineWidth += glyphWidth * glyphScale;
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
	int width;

	/* StartTalkExt measures a new message here before anything else reads it. */
	if (!stopAtCurrentBox && text == sTextEngineState->str)
		TextEngine_ExpandAliases((char *)text);

	width = TextEngine_WidthInternal((const u8 *)text, stopAtCurrentBox);

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

	/* The caller turns this into 2 + tiles; the scene box is always full width. */
	if (TextEngine_IsSceneLayoutActive())
		return (TEXT_ENGINE_SCENE_BOX_WIDTH - 2) * 8;

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

	/* A scene box redrawn for a new speaker skips straight to the final frame. */
	if (animationFrame == 0 && sTextEngineSceneLayoutState.instantOpen) {
		sTextEngineSceneLayoutState.instantOpen = 0;

		for (frame = graphics; frame[1]; frame++)
			;

		destination = (void *)(0x06000200 + GetBackgroundTileDataOffset(1));
		Decompress((const void *)frame[0], destination);
		TextEngine_RefreshSceneTabCorner();
		Proc_Break(proc);
		return;
	}

	frame = graphics + (animationFrame >> 1);
	currentGraphics = frame[0];
	nextGraphics = frame[1];

	destination = (void *)(0x06000200 + GetBackgroundTileDataOffset(1));
	Decompress((const void *)currentGraphics, destination);

	if (!nextGraphics) {
		TextEngine_RefreshSceneTabCorner();
		Proc_Break(proc);
	}
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
		TextEngine_GetCurrentSpeakerAttributes()[TEXT_ENGINE_ATTR_BOOP_PITCH] =
			arguments[7] - 1;
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

static const s8 sTextEnginePopupShake[TEXT_ENGINE_POPUP_SHAKE_FRAMES][2] = {
	{ +6, -4 },
	{ -5, +3 },
	{ +4, +5 },
	{ -4, -3 },
	{ +3, +2 },
	{ -2, -2 },
	{ +2, +1 },
	{ -1,  0 },
	{ +1,  0 },
	{  0,  0 },
};

/* Popup index from the script is 1-based; the table ends at a NULL entry. */
static const struct TextEnginePopup *TextEngine_GetPopup(int index)
{
	const struct TextEnginePopup *popup = TextEnginePopupTable;

	if (index < 1)
		return NULL;

	for (; popup->tiles; popup++) {
		if (--index == 0)
			return popup;
	}

	return NULL;
}

/*
 * Put the popup on BG0, centered, using the dialogue font's tiles and
 * palette.  Returns 0 (nothing shown) when it does not fit in them.
 */
static s8 TextEnginePopup_Show(struct TextEnginePopupProc *proc)
{
	struct Font *font = (struct Font *)0x030000F0;
	const struct TextEnginePopup *popup = proc->popup;
	u16 *map = (u16 *)gGenericBuffer;
	u16 *palette = gPaletteBuffer + font->palid * 0x10;
	int chr = font->tileref & 0x3FF;
	int tileCount = (*(const u32 *)popup->tiles >> 8) / CHR_SIZE;
	int left = (30 - popup->width) / 2;
	int top = (20 - popup->height) / 2;
	int x;
	int y;

	if (tileCount > font->chr_counter * 2 ||
		popup->width > 30 ||
		popup->height > 20)
		return 0;

	Decompress(popup->tiles, font->vramDest);
	Decompress(popup->map, map);

	CpuCopy16(palette, proc->savedPalette, sizeof(proc->savedPalette));
	CpuCopy16(popup->palette, palette, sizeof(proc->savedPalette));
	EnablePaletteSync();

	BG_Fill(gBG0TilemapBuffer, 0);
	for (y = 0; y < popup->height; y++) {
		for (x = 0; x < popup->width; x++) {
			u16 entry = map[y * popup->width + x];

			if (!(entry & 0x3FF))
				continue;

			/* Keep grit's flip bits; point at the font tiles and palette. */
			gBG0TilemapBuffer[TILEMAP_INDEX(left + x, top + y)] =
				TILEREF(chr + (entry & 0x3FF), font->palid) | (entry & 0xC00);
		}
	}

	BG_SetPosition(BG_0, 0, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT);

	proc->tileCount = tileCount;
	proc->shown = 1;

	if (popup->song)
		PlaySoundEffect(popup->song);

	StartScreenImpactFlash();
	return 1;
}

static void TextEnginePopup_OnEnd(struct TextEnginePopupProc *proc)
{
	struct Font *font = (struct Font *)0x030000F0;

	if (!proc->shown)
		return;

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_SetPosition(BG_0, 0, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT);

	/* Hand the text tiles and palette back clean. */
	CpuFastFill(0, font->vramDest, proc->tileCount * CHR_SIZE);
	CpuCopy16(
		proc->savedPalette,
		gPaletteBuffer + font->palid * 0x10,
		sizeof(proc->savedPalette)
	);
	EnablePaletteSync();
}

static void TextEnginePopup_OnIdle(struct TextEnginePopupProc *proc)
{
	if (proc->timer == 0 && !TextEnginePopup_Show(proc)) {
		Proc_Break(proc);
		return;
	}

	if (proc->timer < TEXT_ENGINE_POPUP_SHAKE_FRAMES) {
		BG_SetPosition(
			BG_0,
			sTextEnginePopupShake[proc->timer][0],
			sTextEnginePopupShake[proc->timer][1]
		);
	}

	proc->timer++;

	if (proc->timer >= TEXT_ENGINE_POPUP_FRAMES ||
		(proc->timer >= TEXT_ENGINE_POPUP_MIN_FRAMES &&
		 (gKeyStatusPtr->newKeys & (A_BUTTON | B_BUTTON))))
		Proc_Break(proc);
}

static const struct ProcCmd gProcScr_TextEnginePopup[] = {
	PROC_NAME("TextEnginePopup"),
	PROC_SET_END_CB(TextEnginePopup_OnEnd),
	/* Let the cleared box tilemap reach VRAM before reusing its tiles. */
	PROC_SLEEP(1),
	PROC_REPEAT(TextEnginePopup_OnIdle),
	PROC_END,
};

static int TextEngine_CommandPopup(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	const struct TextEnginePopup *popup = TextEngine_GetPopup(arguments[0]);
	struct TextEnginePopupProc *popupProc;

	(void)command;

	if (!popup || CheckTalkFlag(TALK_FLAG_SPRITE))
		return TalkInterpret(proc);

	/* The popup cuts in over everything: close the box first. */
	TextEngine_EndBigPops(0);
	Proc_EndEach(gProcScr_TextEngineBigJolt);
	TextEngine_ClearSpeakerNameplate();
	ClearTalkBubble();
	sTextEngineSceneLayoutState.boxShown = 0;
	sTextEngineBigTextState.lineBelow = 0;

	/* Shown from its first idle frame, after these fields are set. */
	popupProc = (struct TextEnginePopupProc *)Proc_StartBlocking(
		gProcScr_TextEnginePopup,
		proc
	);
	popupProc->popup = popup;
	popupProc->timer = 0;
	popupProc->shown = 0;
	return 3;
}

static void TextEngine_SetSceneLayout(u8 enabled)
{
	struct TalkState *state = sTextEngineState;
	int oldFaceY;
	int deltaY;
	int i;

	if (sTextEngineSceneLayoutState.enabled == enabled)
		return;

	oldFaceY = TextEngine_GetTalkFaceY(false);

	/* Close the open box so the next text reopens in the new layout. */
	TextEngine_ClearSpeakerNameplate();
	if (TalkHasCorrectBubble())
		ClearTalkBubble();

	/* The scene layout always shows the speaker tab. */
	if (enabled) {
		sTextEngineSceneLayoutState.savedNameplate =
			sTextEngineNameplateState.enabled;
		sTextEngineNameplateState.enabled = 1;
	} else {
		sTextEngineNameplateState.enabled =
			sTextEngineSceneLayoutState.savedNameplate;
	}

	sTextEngineSceneLayoutState.enabled = enabled;
	sTextEngineSceneLayoutState.boxShown = 0;
	sTextEngineSceneLayoutState.instantOpen = 0;

	/*
	 * Portraits already on screen step up behind the box, or back down.
	 * Half-bodies already stand on the bottom edge and stay put.
	 */
	deltaY = TextEngine_GetTalkFaceY(false) - oldFaceY;
	for (i = 0; i < (int)ARRAY_COUNT(state->faces); i++) {
		if (state->faces[i] && !Portraits_IsHalfBodyFace(state->faces[i]))
			state->faces[i]->yPos += deltaY;
	}
}

static int TextEngine_CommandStartSceneLayout(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_SetSceneLayout(1);
	return 3;
}

static int TextEngine_CommandStopSceneLayout(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)proc;
	(void)command;
	(void)arguments;

	TextEngine_SetSceneLayout(0);
	return 3;
}

static int TextEngine_CommandStartBigText(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;

	(void)command;
	(void)arguments;

	/* Slow the print down so each big letter lands on its own. */
	if (!sTextEngineBigTextState.enabled &&
		state->printDelay < TEXT_ENGINE_BIG_TEXT_PRINT_DELAY) {
		sTextEngineBigTextState.savedPrintDelay = state->printDelay;
		sTextEngineBigTextState.printDelayForced = 1;
		state->printDelay = TEXT_ENGINE_BIG_TEXT_PRINT_DELAY;
	}

	sTextEngineBigTextState.enabled = 1;
	return TalkInterpret(proc);
}

static int TextEngine_CommandStopBigText(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;
	(void)arguments;

	/* lineBelow stays set so the rest of this line keeps the big baseline. */
	sTextEngineBigTextState.enabled = 0;

	if (sTextEngineBigTextState.printDelayForced) {
		sTextEngineState->printDelay = sTextEngineBigTextState.savedPrintDelay;
		sTextEngineBigTextState.printDelayForced = 0;
	}

	return TalkInterpret(proc);
}

static void TextEngine_CleanupBigText(void)
{
	sTextEngineBigTextState.enabled = 0;
	sTextEngineBigTextState.lineBelow = 0;
	sTextEngineBigTextState.printDelayForced = 0;
	TextEngine_EndBigPops(0);
	Proc_EndEach(gProcScr_TextEngineBigJolt);
}

static void TextEngine_CleanupSceneLayout(void)
{
	sTextEngineSceneLayoutState.enabled = 0;
	sTextEngineSceneLayoutState.boxShown = 0;
	sTextEngineSceneLayoutState.instantOpen = 0;
	sTextEngineSceneLayoutState.savedNameplate = 0;
}

static void TextEngineAutoAdvance_OnIdle(struct TextEngineAutoAdvanceProc *proc)
{
	/* The player already moved on. */
	if (Proc_Find(gProcScr_TalkWaitForInput) != proc->wait) {
		Proc_Break(proc);
		return;
	}

	/* Don't move on under the conversation log. */
	if (Chatlog_IsVisible())
		return;

	if (--proc->timer > 0)
		return;

	Proc_End(proc->wait);
	Proc_Break(proc);
}

static const struct ProcCmd gProcScr_TextEngineAutoAdvance[] = {
	PROC_NAME("TextEngineAutoAdvance"),
	PROC_REPEAT(TextEngineAutoAdvance_OnIdle),
	PROC_END,
};

/*
 * StartTalkWaitForInput, plus a timer that ends the wait after autoFrames
 * frames if the player hasn't pressed anything by then.  The vanilla wait
 * proc stays in charge of input, so the skip listener leaves it alone.
 */
static void TextEngine_StartWaitForInput(ProcPtr parent, int x, int y, int autoFrames)
{
	struct Proc *wait = Proc_StartBlocking(gProcScr_TalkWaitForInput, parent);
	struct TextEngineAutoAdvanceProc *timer;

	wait->unk64 = x;
	wait->unk66 = y;
	wait->unk68 = 0;

	if (autoFrames <= 0)
		return;

	timer = Proc_Start(gProcScr_TextEngineAutoAdvance, PROC_TREE_3);
	timer->wait = wait;
	timer->timer = autoFrames;
}

static void TextEngine_CleanupAutoAdvance(void)
{
	sTextEngineAutoAdvanceState.frames = 0;
	Proc_EndEach(gProcScr_TextEngineAutoAdvance);
}

static int TextEngine_CommandAutoAdvance(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;

	/* Like [A], with the arrow drawn below the screen. */
	Chatlog_CommitPage();
	TextEngine_StartWaitForInput(proc, 0, DISPLAY_HEIGHT, arguments[0]);
	return 3;
}

static int TextEngine_CommandStartAutoAdvance(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;

	sTextEngineAutoAdvanceState.frames = arguments[0];
	return TalkInterpret(proc);
}

static int TextEngine_CommandStopAutoAdvance(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	(void)command;
	(void)arguments;

	sTextEngineAutoAdvanceState.frames = 0;
	return TalkInterpret(proc);
}

/* Readable on the light text box, in hue order. */
static const u16 sTextEngineRainbowHues[] = {
	RGB(28, 4, 4),   /* red */
	RGB(30, 14, 2),  /* orange */
	RGB(25, 20, 0),  /* gold */
	RGB(6, 22, 4),   /* green */
	RGB(2, 20, 18),  /* teal */
	RGB(4, 11, 30),  /* blue */
	RGB(16, 6, 28),  /* violet */
	RGB(28, 4, 20),  /* magenta */
};

/*
 * Text colour ids (Text_SetColor) the letters take in turn: groups 3, 4 and
 * 1 in script numbering, which normal dialogue leaves unused.
 */
static const u8 sTextEngineRainbowGroups[TEXT_ENGINE_RAINBOW_GROUPS] = { 2, 3, 0 };

static u16 *TextEngine_RainbowGroupColors(int palid, int bank, int group)
{
	/* Bank 0 is the dialogue font's BG palette, bank 1 the float glyphs' OBJ palette. */
	int pal = bank ? 0x10 + TEXT_ENGINE_FLOAT_OBJ_PAL : palid;

	return gPaletteBuffer + pal * 0x10 + group * 3 + 1;
}

/* a and b mixed, with weight eighths of b. */
static u16 TextEngine_MixColor(int a, int b, int weight)
{
	int red = ((a & 0x1F) * (8 - weight) + (b & 0x1F) * weight) >> 3;
	int green = (((a >> 5) & 0x1F) * (8 - weight) + ((b >> 5) & 0x1F) * weight) >> 3;
	int blue = (((a >> 10) & 0x1F) * (8 - weight) + ((b >> 10) & 0x1F) * weight) >> 3;

	return RGB(red, green, blue);
}

static void TextEngine_RainbowApply(struct TextEngineRainbowProc *proc)
{
	const int period = ARRAY_COUNT(sTextEngineRainbowHues) * TEXT_ENGINE_RAINBOW_HUE_FRAMES;
	/* The shading blends towards the default text's, so it suits the box. */
	const u16 *base = TextEngine_RainbowGroupColors(proc->palid, 0, 1);
	int i, bank;

	for (i = 0; i < TEXT_ENGINE_RAINBOW_GROUPS; i++) {
		int phase = (proc->clock + i * (period / TEXT_ENGINE_RAINBOW_GROUPS)) % period;
		int hue = phase / TEXT_ENGINE_RAINBOW_HUE_FRAMES;
		u16 colors[3];

		/* Glyph pixels 1 and 2 are the edge and shadow, 3 the letter itself. */
		colors[2] = TextEngine_MixColor(
			sTextEngineRainbowHues[hue],
			sTextEngineRainbowHues[(hue + 1) % ARRAY_COUNT(sTextEngineRainbowHues)],
			phase % TEXT_ENGINE_RAINBOW_HUE_FRAMES
		);
		colors[1] = TextEngine_MixColor(colors[2], base[1], 5);
		colors[0] = TextEngine_MixColor(colors[2], base[0], 6);

		for (bank = 0; bank < 2; bank++)
			CpuCopy16(
				colors,
				TextEngine_RainbowGroupColors(proc->palid, bank, sTextEngineRainbowGroups[i]),
				sizeof(colors)
			);
	}

	EnablePaletteSync();
}

static void TextEngineRainbow_OnIdle(struct TextEngineRainbowProc *proc)
{
	/* Both swap the dialogue palette out for their own while they show. */
	if (Chatlog_IsVisible() || Proc_Find(gProcScr_TextEnginePopup))
		return;

	proc->clock++;
	TextEngine_RainbowApply(proc);
}

static void TextEngineRainbow_OnEnd(struct TextEngineRainbowProc *proc)
{
	int i, bank;

	for (i = 0; i < TEXT_ENGINE_RAINBOW_GROUPS; i++)
		for (bank = 0; bank < 2; bank++)
			CpuCopy16(
				&proc->saved[bank][i * 3],
				TextEngine_RainbowGroupColors(proc->palid, bank, sTextEngineRainbowGroups[i]),
				3 * sizeof(u16)
			);

	EnablePaletteSync();
}

static const struct ProcCmd gProcScr_TextEngineRainbow[] = {
	PROC_NAME("TextEngineRainbow"),
	PROC_SET_END_CB(TextEngineRainbow_OnEnd),
	PROC_REPEAT(TextEngineRainbow_OnIdle),
	PROC_END,
};

/* Called before each dialogue letter is drawn. */
static void TextEngine_RainbowColorLetter(struct Text *text, const char *str)
{
	struct TextEngineRainbowProc *proc = Proc_Find(gProcScr_TextEngineRainbow);
	int group;

	if (!proc || !proc->active || !text || !str || !*str)
		return;

	/* Spaces show no colour, so they don't take a turn. */
	if (*str == ' ')
		return;

	/*
	 * No % here: the game's __aeabi_uidivmod is its __umodsi3, which
	 * doesn't return the remainder where GCC looks for it.
	 */
	group = sTextEngineRainbowGroups[proc->letter];
	if (++proc->letter >= TEXT_ENGINE_RAINBOW_GROUPS)
		proc->letter = 0;

	Text_SetColor(text, group);
	sTextEngineState->printColor = group;
}

static void TextEngine_CleanupRainbow(void)
{
	Proc_EndEach(gProcScr_TextEngineRainbow);
}

static int TextEngine_CommandStartRainbow(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TalkState *state = sTextEngineState;
	struct TextEngineRainbowProc *rainbow;
	int i, bank;

	(void)command;
	(void)arguments;

	/* Sprite talk and world map text draw letters another way. */
	if (CheckTalkFlag(TALK_FLAG_SPRITE) || CheckTalkFlag(TALK_FLAG_7))
		return TalkInterpret(proc);

	rainbow = Proc_Find(gProcScr_TextEngineRainbow);
	if (!rainbow) {
		/*
		 * SetInitTalkTextFont reloads the whole text palette, and the
		 * letter effects call it for every letter.  The VSync tree runs
		 * just before the palette is uploaded, so the rainbow is always
		 * the last thing written.
		 */
		rainbow = Proc_Start(gProcScr_TextEngineRainbow, PROC_TREE_VSYNC);
		rainbow->active = 0;
		rainbow->clock = 0;
		rainbow->palid = ((struct Font *)0x030000F0)->palid;

		for (i = 0; i < TEXT_ENGINE_RAINBOW_GROUPS; i++)
			for (bank = 0; bank < 2; bank++)
				CpuCopy16(
					TextEngine_RainbowGroupColors(rainbow->palid, bank, sTextEngineRainbowGroups[i]),
					&rainbow->saved[bank][i * 3],
					3 * sizeof(u16)
				);

		TextEngine_RainbowApply(rainbow);
	}

	if (!rainbow->active) {
		rainbow->active = 1;
		rainbow->letter = 0;
		rainbow->savedColor = state->printColor;
	}

	return TalkInterpret(proc);
}

static int TextEngine_CommandStopRainbow(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	struct TextEngineRainbowProc *rainbow = Proc_Find(gProcScr_TextEngineRainbow);

	(void)command;
	(void)arguments;

	/* Letters already printed keep cycling until the dialogue ends. */
	if (rainbow && rainbow->active) {
		rainbow->active = 0;
		ChangeTextColorID(rainbow->savedColor);
	}

	return TalkInterpret(proc);
}

/*
 * Listener dimming: while someone speaks, the other talk portraits darken a
 * little, and light up again when they speak.  The dimmed colours are worked
 * out from the portrait's own palette data, so nothing has to be saved.
 */

/* The OBJ palettes a face draws with, and the colours each one holds lit. */
static int TextEngine_GetFacePalettes(struct FaceProc *face, u8 *pals, const u16 **colors)
{
	const struct FaceData *info = face->pFaceInfo;
	s32 overlayOam2;
	int count = 0;

	if (!info || !info->pal)
		return 0;

	pals[count] = 0x10 + ((face->oam2 >> 12) & 0xF);
	colors[count++] = info->pal;

	overlayOam2 = Portrait32_GetOverlayOam2(face);
	if (overlayOam2 >= 0) {
		pals[count] = 0x10 + ((overlayOam2 >> 12) & 0xF);
		colors[count++] = info->pal + 0x10;
	}

	return count;
}

/* Fading in or out: leave those palettes to the fade. */
static bool TextEngine_IsPalFading(int pal)
{
	struct ProcFindIterator it;
	struct TextEnginePalFadeProc *fade;

	Proc_FindBegin(&it, ProcScr_PalFade);
	while ((fade = (struct TextEnginePalFadeProc *)Proc_FindNext(&it)) != NULL) {
		if (fade->st && fade->st->pal == gPaletteBuffer + pal * 0x10)
			return true;
	}

	return false;
}

static bool TextEngine_IsTalkFace(struct FaceProc *face)
{
	struct TalkState *state = sTextEngineState;
	int i;

	for (i = 0; i < (int)ARRAY_COUNT(state->faces); i++)
		if (state->faces[i] == face)
			return true;

	return false;
}

/* Writes the face's palettes at level DIM_FRAMES-ths of the way to dim. */
static bool TextEngine_PutFaceDim(struct FaceProc *face, int level)
{
	u8 pals[TEXT_ENGINE_DIM_MAX_PALETTES];
	const u16 *colors[TEXT_ENGINE_DIM_MAX_PALETTES];
	int mul = 32 - TEXT_ENGINE_DIM_STRENGTH * level / TEXT_ENGINE_DIM_FRAMES;
	int count = TextEngine_GetFacePalettes(face, pals, colors);
	int i, j;

	for (i = 0; i < count; i++)
		if (TextEngine_IsPalFading(pals[i]))
			return false;

	for (i = 0; i < count; i++) {
		u16 *dst = gPaletteBuffer + pals[i] * 0x10;

		for (j = 0; j < 0x10; j++) {
			int c = colors[i][j];

			dst[j] = RGB(
				((c & 0x1F) * mul) >> 5,
				(((c >> 5) & 0x1F) * mul) >> 5,
				(((c >> 10) & 0x1F) * mul) >> 5
			);
		}
	}

	return true;
}

static void TextEngineDim_OnIdle(struct TextEngineDimProc *proc)
{
	struct TalkState *state = sTextEngineState;
	struct FaceProc *speaker = NULL;
	bool changed = false;
	int slot;

	/* The log swaps palettes out for its own while it shows. */
	if (Chatlog_IsVisible())
		return;

	if (state->activeFaceSlot < ARRAY_COUNT(state->faces))
		speaker = state->faces[state->activeFaceSlot];

	for (slot = 0; slot < TEXT_ENGINE_DIM_FACE_SLOTS; slot++) {
		struct FaceProc *face = gFaces[slot];
		int level = proc->level[slot];
		int target = 0;

		/* A new face in this slot starts lit. */
		if (face != proc->face[slot]) {
			proc->face[slot] = face;
			proc->level[slot] = 0;
			level = 0;
		}

		if (!face)
			continue;

		/* With no one speaking, everyone is lit. */
		if (speaker && face != speaker && TextEngine_IsTalkFace(face))
			target = TEXT_ENGINE_DIM_FRAMES;

		/* Lit and staying lit: the palette is the face's own. */
		if (level == 0 && target == 0)
			continue;

		if (level < target)
			level++;
		else if (level > target)
			level--;

		/*
		 * Dimmed faces are rewritten every frame, in case something
		 * reloaded the palette.  A face mid-fade keeps its level.
		 */
		if (TextEngine_PutFaceDim(face, level)) {
			proc->level[slot] = level;
			changed = true;
		}
	}

	if (changed)
		EnablePaletteSync();
}

static void TextEngineDim_OnEnd(struct TextEngineDimProc *proc)
{
	bool changed = false;
	int slot;

	/* Faces still on screen after the dialogue go back to their own colours. */
	for (slot = 0; slot < TEXT_ENGINE_DIM_FACE_SLOTS; slot++) {
		if (proc->level[slot] && gFaces[slot] == proc->face[slot] && gFaces[slot])
			changed |= TextEngine_PutFaceDim(gFaces[slot], 0);
	}

	if (changed)
		EnablePaletteSync();
}

static const struct ProcCmd gProcScr_TextEngineDim[] = {
	PROC_NAME("TextEngineDim"),
	PROC_SET_END_CB(TextEngineDim_OnEnd),
	PROC_REPEAT(TextEngineDim_OnIdle),
	PROC_END,
};

static void TextEngine_StartDim(void)
{
	struct TextEngineDimProc *proc;
	int slot;

	Proc_EndEach(gProcScr_TextEngineDim);

	if (!gTextEngineConfig.dim_listener_portraits || CheckTalkFlag(TALK_FLAG_SPRITE))
		return;

	/*
	 * On the VSync tree, after the frame's other palette writes, like the
	 * rainbow: a face change reloads the lit palette.
	 */
	proc = Proc_Start(gProcScr_TextEngineDim, PROC_TREE_VSYNC);

	for (slot = 0; slot < TEXT_ENGINE_DIM_FACE_SLOTS; slot++) {
		proc->face[slot] = gFaces[slot];
		proc->level[slot] = 0;
	}
}

static void TextEngine_CleanupDim(void)
{
	Proc_EndEach(gProcScr_TextEngineDim);
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

static int TextEngine_CommandEmotion(
	ProcPtr proc,
	const struct TextEngineCommandDescriptor *command,
	const u8 *arguments
)
{
	int kind = command->code == TEXT_ENGINE_CMD_ANGER
		? TEXT_ENGINE_EMOTION_ANGER
		: TEXT_ENGINE_EMOTION_SWEAT;

	(void)arguments;

	if (!TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot))
		return TalkInterpret(proc);

	TextEngine_StartEmotion(
		TextEngine_GetFaceProcByPosition(sTextEngineState->activeFaceSlot),
		kind,
		proc
	);
	return 3;
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
	{ TEXT_ENGINE_CMD_SWEAT, 0, TextEngine_CommandEmotion, NULL, TextEngine_CleanupEmotion },
	{ TEXT_ENGINE_CMD_ANGER, 0, TextEngine_CommandEmotion, NULL, TextEngine_CleanupEmotion },
	{ TEXT_ENGINE_CMD_SCENE_LAYOUT_ON, 0, TextEngine_CommandStartSceneLayout, NULL, TextEngine_CleanupSceneLayout },
	{ TEXT_ENGINE_CMD_SCENE_LAYOUT_OFF, 0, TextEngine_CommandStopSceneLayout, NULL, NULL },
	{ TEXT_ENGINE_CMD_BIG_TEXT_ON, 0, TextEngine_CommandStartBigText, NULL, TextEngine_CleanupBigText },
	{ TEXT_ENGINE_CMD_BIG_TEXT_OFF, 0, TextEngine_CommandStopBigText, NULL, NULL },
	{ TEXT_ENGINE_CMD_POPUP, 1, TextEngine_CommandPopup, NULL, NULL },
	{ TEXT_ENGINE_CMD_AUTO_ADVANCE, 1, TextEngine_CommandAutoAdvance, NULL, TextEngine_CleanupAutoAdvance },
	{ TEXT_ENGINE_CMD_AUTO_ADVANCE_ON, 1, TextEngine_CommandStartAutoAdvance, NULL, TextEngine_CleanupAutoAdvance },
	{ TEXT_ENGINE_CMD_AUTO_ADVANCE_OFF, 0, TextEngine_CommandStopAutoAdvance, NULL, NULL },
	{ TEXT_ENGINE_CMD_RAINBOW_ON, 0, TextEngine_CommandStartRainbow, NULL, TextEngine_CleanupRainbow },
	{ TEXT_ENGINE_CMD_RAINBOW_OFF, 0, TextEngine_CommandStopRainbow, NULL, NULL },
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
			if (sTextEngineBigTextState.lineBelow) {
				/* A big line also filled the line below it. */
				sTextEngineBigTextState.lineBelow = 0;

				if (state->lineActive + 2 >= state->lines) {
					/* No room under it: clear the box instead of scrolling half a letter away. */
					state->lineActive++;
					Proc_StartBlocking(gProcScr_TalkShiftClearAll, proc);
					return 3;
				}

				state->lineActive += 2;
				state->putLines = 0;
				return 2;
			}

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

		case CHFE_L_A: {
			/* After big text the arrow sits on the big letters' baseline. */
			int line = state->lineActive + sTextEngineBigTextState.lineBelow;

			Chatlog_CommitPage();
			TextEngine_StartWaitForInput(
				proc,
				state->xText * 8 + Text_GetCursor(TextEngine_GetLineText(state, line)) + 4,
				state->yText * 8 + line * 16 + 8,
				sTextEngineAutoAdvanceState.frames
			);
			return 3;
		}

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
			if (TalkHasCorrectBubble()) {
				ClearTalkBubble();
				sTextEngineSceneLayoutState.boxShown = 0;
			}

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
			sTextEngineSceneLayoutState.boxShown = 0;
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
			if (TextEngine_IsSceneLayoutActive()) {
				state->activeWidth = TEXT_ENGINE_SCENE_BOX_WIDTH;
				continue;
			}

			state->activeWidth = 2 + (
				GetStringTextWidthWithDialogueCodes(state->str, TalkHasCorrectBubble()) + 7
			) / 8;
			continue;

		default:
			return 1;
		}
	}
}
