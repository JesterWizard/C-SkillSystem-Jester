#pragma once

#include "common-chax.h"

/*
 * Portrait expressions: alternate portraits of a character (angry, sad, ...) that dialogue swaps in place with
 * [Expression][N] (or [Angry], [Sad], ... in Contents/Texts/textdefs.txt). The table is gPortraitExpressions in
 * Data/CustomPortraits/PortraitExpressions.c; Documentation/Features/PortraitExpressions.md has the whole workflow.
 *
 * Expression numbers start at 1: a 0 byte would end the dialogue string.
 */
enum portrait_expression {
	EXPRESSION_NEUTRAL = 1,     /* back to the base portrait */
	EXPRESSION_ANGRY = 2,
	EXPRESSION_SAD = 3,
	EXPRESSION_HAPPY = 4,
	EXPRESSION_SURPRISED = 5,
	EXPRESSION_DETERMINED = 6,
	EXPRESSION_HURT = 7,
	EXPRESSION_CONFUSED = 8,
};

struct PortraitExpression {
	u8 basePortrait;  /* the character's usual portrait id */
	u8 expression;    /* EXPRESSION_* */
	u8 portrait;      /* the portrait id that shows it */
	u8 pad;
};

/* Ends with an entry whose basePortrait is 0. */
extern const struct PortraitExpression gPortraitExpressions[];

/*
 * Frame sets: an expression drawn with the base portrait and its own eyes and mouth, instead of a whole portrait. A
 * row of gPortraitFrames (Data/CustomPortraits/PortraitExpressions.c) gives a base portrait and an expression its
 * frames: the eyes open, half shut and shut, then the six mouth frames in the mouth sheet's order (talking, then
 * smiling), each 32x16 (8 tiles: the top four, then the bottom four), 4bpp in the base's palette. portrait_tool.py
 * makes the data from a 96x48 frame sheet.
 *
 * [Expression][N] uses a full portrait (gPortraitExpressions) when there is one, else a frame set. The eyes are written
 * over the face's graphics (OBJ VRAM) and its blink frames; the mouth draws from the set. Regular portraits only: a
 * halfbody has no separate eye and mouth frames.
 */
#define PORTRAIT_FRAME_SIZE (8 * 0x20)
#define PORTRAIT_FRAMES_EYES 3
#define PORTRAIT_FRAMES_SIZE ((PORTRAIT_FRAMES_EYES + 6) * PORTRAIT_FRAME_SIZE)

struct PortraitFrames {
	u8 basePortrait;
	u8 expression;    /* EXPRESSION_* */
	u8 pad[2];
	const u8 *frames; /* PORTRAIT_FRAMES_SIZE bytes */
};

/* Ends with an entry whose basePortrait is 0. */
extern const struct PortraitFrames gPortraitFrames[];

/* The frame set an expression of a base portrait has, or NULL */
const struct PortraitFrames *PortraitFrames_Find(int basePortrait, int expression);

/* Text engine: show a frame set on a face (NULL: none, and the base eyes come back); the mouth frames to draw */
void PortraitFrames_Set(struct FaceProc *face, const struct PortraitFrames *set);
const u8 *PortraitFrames_Mouth(struct FaceProc *face);

/* Face graphics: a new face (Face_OnInit) shows no set; a face changed in place (sub_8006650) shows its set again */
void PortraitFrames_OnNewFace(struct FaceProc *face);
void PortraitFrames_OnFaceLoaded(struct FaceProc *face);
