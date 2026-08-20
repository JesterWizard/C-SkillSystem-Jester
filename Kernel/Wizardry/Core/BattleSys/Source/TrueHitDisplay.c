#include "common-chax.h"
#include "kernel-lib.h"
#include "battle-system.h"

/**
 * Rounded true-hit probabilities for GBA 2RN:
 * Roll2RN succeeds when ((RN1 + RN2) / 2) < displayed.
 * Values match Serenes Forest, rounded to nearest percent.
 */
static const u8 sTrue2RNTable[101] = {
	  0,   0,   0,   0,   0,   1,   1,   1,   1,   2,
	  2,   3,   3,   4,   4,   5,   5,   6,   7,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,
	 18,  20,  21,  22,  23,  25,  26,  28,  29,  31,
	 32,  34,  36,  37,  39,  41,  43,  45,  47,  49,
	 51,  52,  54,  56,  58,  60,  62,  63,  65,  67,
	 68,  70,  72,  73,  74,  76,  77,  79,  80,  81,
	 82,  83,  85,  86,  87,  88,  89,  90,  91,  91,
	 92,  93,  94,  94,  95,  96,  96,  97,  97,  98,
	 98,  98,  99,  99,  99, 100, 100, 100, 100, 100,
	100,
};

int GetDisplayedTrueHitRate(int rate)
{
	/* Designer gate off, or player toggle OFF (1): keep displayed hit. */
	if (gpKernelDesignerConfig->show_true_2rn != true
		|| gPlaySt.config.show_true_2rn != 0)
		return rate;

	if (rate < 0 || rate > 100)
		return rate;

	return sTrue2RNTable[rate];
}
