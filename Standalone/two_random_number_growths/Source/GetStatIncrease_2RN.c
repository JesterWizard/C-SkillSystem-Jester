#include "global.h"
#include "rng.h"

/**
 * Vanilla GetStatIncrease with Roll1RN replaced by Roll2RN for the remainder roll.
 */
int GetStatIncrease_2RN(int growth)
{
	int result = 0;

	while (growth > 100) {
		result++;
		growth -= 100;
	}

	if (Roll2RN(growth))
		result++;

	return result;
}
