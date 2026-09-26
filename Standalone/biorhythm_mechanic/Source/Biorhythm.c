#include "global.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "constants/characters.h"
#include "variables.h"

#define MAX_BIORHYTHM_STATES 5

extern const u8 BiorhythmMechanicEnabled;

struct BiorhythmPInfoConfig {
	int biorhythm[MAX_BIORHYTHM_STATES];
	int startOffset;
};

#define BIORHYTHM_CYCLE -15, -5, 0, 5, 15

static const struct BiorhythmPInfoConfig gBiorhythmPInfoConfigList[0x100] = {
	[CHARACTER_EIRIKA]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_SETH]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_GILLIAM]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_FRANZ]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_MOULDER]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_VANESSA]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_ROSS]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_NEIMI]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_COLM]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_GARCIA]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_INNES]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_LUTE]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_NATASHA]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_CORMAG]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_EPHRAIM]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_FORDE]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_KYLE]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_AMELIA]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_ARTUR]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_GERIK]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_TETHYS]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_MARISA]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_SALEH]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_EWAN]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_LARACHEL]    = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_DOZLA]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_RENNAC]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_DUESSEL]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_MYRRH]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_KNOLL]       = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_JOSHUA]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_SYRENE]      = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_TANA]        = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_LYON_CC]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_ORSON_CC]    = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_GLEN_CC]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_SELENA_CC]   = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_VALTER_CC]   = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_RIEV_CC]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_CAELLACH_CC] = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_FADO_CC]     = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_ISMAIRE_CC]  = { { BIORHYTHM_CYCLE }, 0 },
	[CHARACTER_HAYDEN_CC]   = { { BIORHYTHM_CYCLE }, 0 },
};

static int GetBiorhythmBonus(struct BattleUnit *bu)
{
	unsigned charId = UNIT_CHAR_ID(&bu->unit);
	const struct BiorhythmPInfoConfig *config;
	unsigned index;
	int i;

	if (!BiorhythmMechanicEnabled || charId >= 0x100)
		return 0;

	config = &gBiorhythmPInfoConfigList[charId];

	for (i = 0; i < MAX_BIORHYTHM_STATES; i++) {
		if (config->biorhythm[i] != 0)
			break;
	}

	if (i == MAX_BIORHYTHM_STATES)
		return 0;

	index = (unsigned)((gPlaySt.chapterTurnNumber - 1) + config->startOffset) % MAX_BIORHYTHM_STATES;
	return config->biorhythm[index];
}

void ComputeBattleUnitHitRate_Biorhythm(struct BattleUnit *bu)
{
	int status = (bu->unit.skl * 2) + GetItemHit(bu->weapon) + (bu->unit.lck / 2) + bu->wTriangleHitBonus;

	status += GetBiorhythmBonus(bu);
	bu->battleHitRate = status;
}

void ComputeBattleUnitAvoidRate_Biorhythm(struct BattleUnit *bu)
{
	int status = (bu->battleSpeed * 2) + bu->terrainAvoid + bu->unit.lck;

	if (status < 0)
		status = 0;

	status += GetBiorhythmBonus(bu);

	if (status < 0)
		status = 0;

	bu->battleAvoidRate = status;
}
