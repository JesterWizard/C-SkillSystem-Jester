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
};

struct PortraitExpression {
	u8 basePortrait;  /* the character's usual portrait id */
	u8 expression;    /* EXPRESSION_* */
	u8 portrait;      /* the portrait id that shows it */
	u8 pad;
};

/* Ends with an entry whose basePortrait is 0. */
extern const struct PortraitExpression gPortraitExpressions[];
