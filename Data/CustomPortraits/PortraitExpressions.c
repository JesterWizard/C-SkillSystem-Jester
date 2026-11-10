#include <common-chax.h>
#include "kernel/portrait-expressions.h"

/*
 * Each row gives one character one expression: [Expression][N] in dialogue swaps the speaker's portrait for the row
 * with its base portrait and that expression, in place (no fade). [Neutral] goes back to the base portrait. The
 * portraits themselves are installed in CustomPortraits.event; see Documentation/Features/PortraitExpressions.md.
 */
const struct PortraitExpression gPortraitExpressions[] = {
	{ .basePortrait = 0x02, .expression = EXPRESSION_ANGRY, .portrait = 0xB0 }, /* Eirika: Portraits/eirika_angry.png */
	{ .basePortrait = 0x02, .expression = EXPRESSION_SAD, .portrait = 0xB1 },   /* Eirika: Portraits/eirika_sad.png */
	{ .basePortrait = 0x14, .expression = EXPRESSION_ANGRY, .portrait = 0xB2 }, /* Ephraim: Portraits/ephraim_angry.png */
	{ .basePortrait = 0x04, .expression = EXPRESSION_HURT, .portrait = 0xB3 },  /* Seth: Portraits/seth_hurt.png */
	{ .basePortrait = 0x46, .expression = EXPRESSION_SAD, .portrait = 0xB4 },   /* Lyon: Portraits/lyon_sad.png */
	{ .basePortrait = 0 },
};

/*
 * Frame sets: an expression with no portrait of its own, drawn on the base portrait with these eyes and mouths. Each
 * is a frame sheet in Data/CustomPortraits/Frames/, made into data with portrait_tool.py frames and included in
 * CustomPortraits.event. A portrait in gPortraitExpressions wins over a frame set for the same expression.
 */
extern const u8 PortraitFrames_EirikaDetermined[];
extern const u8 PortraitFrames_EphraimDetermined[];

const struct PortraitFrames gPortraitFrames[] = {
	/* Eirika: her angry brows over her own mouth (Frames/eirika_determined.png) */
	{ .basePortrait = 0x02, .expression = EXPRESSION_DETERMINED, .frames = PortraitFrames_EirikaDetermined },
	/* Ephraim: the same, from his angry portrait (Frames/ephraim_determined.png) */
	{ .basePortrait = 0x14, .expression = EXPRESSION_DETERMINED, .frames = PortraitFrames_EphraimDetermined },
	{ .basePortrait = 0 },
};
