#include "common-chax.h"
#include "kernel-lib.h"
#include "lvup.h"
#include "strmag.h"

/**
 * Class growths: with gpKernelDesignerConfig->class_growths on, a unit's class
 * growths are merged into its personal growths before skill and status bonuses.
 * GetUnit*Growth in Lvup/GrowthGetter.c builds on these base getters.
 */

int GetClassGrowthBase(int personal, int job)
{
	switch (gpKernelDesignerConfig->class_growths) {
	case CLASS_GROWTHS_ADD:
		return personal + job;

	case CLASS_GROWTHS_AVERAGE:
		return (personal + job) / 2;

	default:
		return personal;
	}
}

int GetUnitBaseHpGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthHP, unit->pClassData->growthHP); }
int GetUnitBasePowGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthPow, unit->pClassData->growthPow); }
int GetUnitBaseMagGrowth(struct Unit* unit) { return GetClassGrowthBase(GetUnitBasicMagGrowth(unit), GetUnitJobBasedBasicMagGrowth(unit)); }
int GetUnitBaseSklGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthSkl, unit->pClassData->growthSkl); }
int GetUnitBaseSpdGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthSpd, unit->pClassData->growthSpd); }
int GetUnitBaseLckGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthLck, unit->pClassData->growthLck); }
int GetUnitBaseDefGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthDef, unit->pClassData->growthDef); }
int GetUnitBaseResGrowth(struct Unit* unit) { return GetClassGrowthBase(unit->pCharacterData->growthRes, unit->pClassData->growthRes); }

/* Class change growth preview */
const char *const gClassGrowthStatNames[CLASS_GROWTH_MAX] = {
	[CLASS_GROWTH_HP]  = "HP",
	[CLASS_GROWTH_POW] = "Str",
	[CLASS_GROWTH_MAG] = "Mag",
	[CLASS_GROWTH_SKL] = "Skl",
	[CLASS_GROWTH_SPD] = "Spd",
	[CLASS_GROWTH_LCK] = "Lck",
	[CLASS_GROWTH_DEF] = "Def",
	[CLASS_GROWTH_RES] = "Res",
};

int GetUnitGrowthByStat(struct Unit* unit, int stat)
{
	switch (stat) {
	case CLASS_GROWTH_HP:  return GetUnitHpGrowth(unit);
	case CLASS_GROWTH_POW: return GetUnitPowGrowth(unit);
	case CLASS_GROWTH_MAG: return GetUnitMagGrowth(unit);
	case CLASS_GROWTH_SKL: return GetUnitSklGrowth(unit);
	case CLASS_GROWTH_SPD: return GetUnitSpdGrowth(unit);
	case CLASS_GROWTH_LCK: return GetUnitLckGrowth(unit);
	case CLASS_GROWTH_DEF: return GetUnitDefGrowth(unit);
	case CLASS_GROWTH_RES: return GetUnitResGrowth(unit);
	default: return 0;
	}
}

void GetClassChangeGrowthDiffs(struct Unit* unit, int jid, int diffs[CLASS_GROWTH_MAX])
{
	struct Unit preview = *unit;
	int i;

	preview.pClassData = GetClassData(jid);

	/* Run each unit through all stats in turn so its skill list is only built once */
	for (i = 0; i < CLASS_GROWTH_MAX; i++)
		diffs[i] = -GetUnitGrowthByStat(unit, i);

	for (i = 0; i < CLASS_GROWTH_MAX; i++)
		diffs[i] += GetUnitGrowthByStat(&preview, i);
}

/**
 * Growth changes are drawn green when they go up and red when they go down.
 * The text palettes have no such groups, so a screen borrows one of its
 * unused groups; this proc lives under the screen and puts the original
 * colours back when the screen ends.
 */
struct ProcTextColorKeeper {
	PROC_HEADER;
	u16 offset;
	u16 saved[3];
};

static void TextColorKeeper_OnEnd(struct ProcTextColorKeeper *proc)
{
	CpuCopy16(proc->saved, gPaletteBuffer + proc->offset, sizeof(proc->saved));
	EnablePaletteSync();
}

static const struct ProcCmd ProcScr_TextColorKeeper[] = {
	PROC_NAME("ClassGrowthTextColor"),
	PROC_SET_END_CB(TextColorKeeper_OnEnd),
	PROC_BLOCK,
};

void SetClassGrowthTextColor(ProcPtr screen, int palid, int colorId, const u16 colors[3])
{
	struct ProcTextColorKeeper *proc;
	struct ProcFindIterator it;
	int offset = palid * 0x10 + colorId * 3 + 1;

	Proc_FindBegin(&it, ProcScr_TextColorKeeper);
	while ((proc = Proc_FindNext(&it)) != NULL)
		if (proc->offset == offset)
			break;

	if (!proc) {
		proc = Proc_Start(ProcScr_TextColorKeeper, screen);
		proc->offset = offset;
		CpuCopy16(gPaletteBuffer + offset, proc->saved, sizeof(proc->saved));
	}

	CpuCopy16(colors, gPaletteBuffer + offset, sizeof(proc->saved));
	EnablePaletteSync();
}
