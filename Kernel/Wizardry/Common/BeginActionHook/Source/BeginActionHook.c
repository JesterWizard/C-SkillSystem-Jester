#include "common-chax.h"
#include "kernel-lib.h"
#include "pair-up.h"
//#include <stdio.h>
#include <unistd.h>

typedef void (*BeginActionFunc_t)(struct Unit *unit);
extern BeginActionFunc_t const *const gpBeginActionHooks;

#define PAIR_UP_VIEWED_UNIT_ADDR ((volatile u8 *) 0x0203A956)

static struct Unit *PairUp_TrySwitchViewedUnit(struct Unit *unit)
{
	struct Unit *previous;
	struct Unit *leader;

	if (!unit || gBmSt.taken_action)
		return unit;

	previous = GetUnit(*PAIR_UP_VIEWED_UNIT_ADDR);
	if (!previous || previous == unit)
		return unit;

	if (PairUp_IsLeader(previous) && PairUp_GetSupport(previous) == unit) {
		PairUp_Switch(previous);
		return unit;
	}

	if (PairUp_IsSupport(previous)
		&& (leader = PairUp_GetLeader(previous)) == unit
		&& PairUp_Switch(leader))
		return previous;

	return unit;
}

typedef struct
{
	const int key;
	const int values[3];
} VoiceIds;

FORCE_DECLARE static const VoiceIds character_voice_ids[] =
{
    {0},
    { CHARACTER_JOSHUA,    {0x26D, 0x26E, 0x26F} },
    { CHARACTER_MYRRH,    {0x270, 0x271, 0x272} },
    { CHARACTER_LUTE,    {0x273, 0x274, 0x275} },
    { CHARACTER_INNES,    {0x276, 0x277, 0x278} },
    { CHARACTER_EIRIKA,    {0x279, 0x27A, 0x27B} },
    { CHARACTER_ROSS,    {0x27C, 0x27D, 0x27E} },
    { CHARACTER_VIGARDE,    {0x27F, 0x280, 0x281} },
    { CHARACTER_KYLE,    {0x282, 0x283, 0x284} },
    { CHARACTER_SALEH,    {0x285, 0x286, 0x287} },
    { CHARACTER_GERIK,    {0x288, 0x289, 0x28A} },
    { CHARACTER_GILLIAM,    {0x28B, 0x28C, 0x28D} },
    { CHARACTER_DOZLA,    {0x28E, 0x28F, 0x290} },
    { CHARACTER_GLEN_CC,    {0x291, 0x292, 0x293} },
    { CHARACTER_CAELLACH_CC,    {0x294, 0x295, 0x296} },
    { CHARACTER_RENNAC,    {0x297, 0x298, 0x299} },
    { CHARACTER_COLM,    {0x29A, 0x29B, 0x29C} },
    { CHARACTER_NATASHA,    {0x29D, 0x29E, 0x29F} },
    { CHARACTER_NEIMI,    {0x2A0, 0x2A1, 0x2A2} },
    { CHARACTER_SELENA,    {0x2A3, 0x2A4, 0x2A5} },
    { CHARACTER_AMELIA,    {0x2A6, 0x2A7, 0x2A8} },
    { CHARACTER_TANA,    {0x2A9, 0x2AA, 0x2AB} },
    { CHARACTER_KNOLL,    {0x2AC, 0x2AD, 0x2AE} },
    { CHARACTER_RIEV,    {0x2AF, 0x2B0, 0x2B1} },
    { CHARACTER_EPHRAIM,    {0x2B2, 0x2B3, 0x2B4} },
    { CHARACTER_EWAN,    {0x2B5, 0x2B6, 0x2B7} },
    { CHARACTER_FORDE,    {0x2B8, 0x2B9, 0x2BA} },
    { CHARACTER_VANESSA,    {0x2BB, 0x385, 0x386} },
    { CHARACTER_SYRENE,    {0x387, 0x388, 0x389} },
    { CHARACTER_LYON_CC,    {0x38A, 0x38B, 0x38C} },
    { CHARACTER_DUESSEL,    {0x38D, 0x38E, 0x38F} },
    { CHARACTER_VALTER_CC,    {0x390, 0x391, 0x392} },
    { CHARACTER_SETH,    {0x393, 0x394, 0x395} },
    { CHARACTER_LARACHEL,    {0x396, 0x397, 0x398} },
    { CHARACTER_CORMAG,    {0x399, 0x39A, 0x39B} },
    { CHARACTER_TETHYS,    {0x39C, 0x39D, 0x39E} },
    { CHARACTER_ARTUR,    {0x39F, 0x3A0, 0x3A1} },
    { CHARACTER_MARISA,    {0x3A2, 0x3A3, 0x3A4} },
};

struct BgmVolumeDelayProc {
    PROC_HEADER;

    u16 framesLeft;
    u16 restoreVolume;
};

static void BgmVolumeDelay_OnLoop(struct BgmVolumeDelayProc *proc)
{
    if (proc->framesLeft--) {
        return; // yield
    }

    Sound_SetSEVolume(proc->restoreVolume);
    Proc_End(proc);
}

static const struct ProcCmd ProcScr_BgmVolumeDelay[] = {
    PROC_REPEAT(BgmVolumeDelay_OnLoop),
    PROC_END,
};

static void PlayUnitVoiceWithBGMReduced(u16 voiceId, int frames)
{
    struct BgmVolumeDelayProc *proc;

    Sound_SetSEVolume(60);      // volume to reduce BGM to (lower than what MUSI provides)
    PlaySoundEffect(voiceId);   // play voice

    proc = Proc_Start(ProcScr_BgmVolumeDelay, Proc_Find(gProcScr_PlayerPhase));

    proc->framesLeft    = frames;
    proc->restoreVolume = 0x100;
}


LYN_REPLACE_CHECK(UnitBeginAction);
void UnitBeginAction(struct Unit *unit)
{
	unit = PairUp_TrySwitchViewedUnit(unit);
	*PAIR_UP_VIEWED_UNIT_ADDR = unit->index;

    gActiveUnit = unit;
    gActiveUnitId = unit->index;

    gActiveUnitMoveOrigin.x = unit->xPos;
    gActiveUnitMoveOrigin.y = unit->yPos;

    gActionData.xMove = unit->xPos;
    gActionData.yMove = unit->yPos;
    gActionData.subjectIndex   = unit->index;
    gActionData.unitActionType = 0;
    gActionData.moveCount      = 0;

    gBmSt.taken_action = 0;
    gBmSt.unk3F        = 0xFF;

    sub_802C334();

	// Play a random unit selection quote
    u8 charId = unit->pCharacterData->number;
    for (unsigned i = 0; i < ARRAY_COUNT(character_voice_ids); i++)
	{
        if (charId == character_voice_ids[i].key)
		{
            int voiceId = character_voice_ids[i].values[NextRN_N(3)];

            PlayUnitVoiceWithBGMReduced(voiceId, 45); // Hold for 45 frames before restoring
            break;
        }
    }

    gActiveUnit->state |= US_HIDDEN;
    gBmMapUnit[unit->yPos][unit->xPos] = 0;

#if CHAX
    for (const BeginActionFunc_t *it = gpBeginActionHooks; *it; it++)
        (*it)(unit);
#endif
}