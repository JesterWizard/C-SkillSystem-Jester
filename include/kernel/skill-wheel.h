#pragma once

#include "common-chax.h"

/*
 * Spinning prize wheel shared by the Spin For Gold and Spin For Luck menu
 * skills (Kernel/Wizardry/SkillEffects/MenuSkills/SkillWheel.c).
 *
 * The wheel spins until A is pressed, slows down and stops on a random
 * section under the arrow. Section i of the art is centred i sections
 * clockwise from the top.
 */

#define SKILL_WHEEL_SECTIONS 8

struct SkillWheelConf {
	/* 64x64 wheel art: 4bpp tiles in row order (a grit .4bpp of a 64x64 PNG). */
	const u8 *gfx;

	/* 16 colours. The arrow is drawn with indices 1-3 (outline, gold, dark gold). */
	const u16 *pal;

	/* Shown below the wheel while it spins. */
	u16 prompt;

	/*
	 * Result line for the section that won, shown when the wheel stops.
	 * Either write it into buf (room for 32 chars) and return buf, or
	 * return another string. Sets *color to a TEXT_COLOR_SYSTEM_* value.
	 */
	const char *(*describe)(int section, char *buf, int *color);

	/* Applies the result once the wheel has closed. May start a blocking child of proc. */
	void (*apply)(ProcPtr proc, int section);
};

/* Shows the wheel; parent is blocked until it closes. Any cost must be paid before. */
void StartSkillWheel(const struct SkillWheelConf *conf, ProcPtr parent);
