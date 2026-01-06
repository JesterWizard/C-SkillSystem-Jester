#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"

#define LOCAL_TRACE 0

bool UNIT_STONED(struct Unit *unit)
{
	return (UNIT_STATUS_INDEX(unit) == UNIT_STATUS_PETRIFY || UNIT_STATUS_INDEX(unit) == UNIT_STATUS_13);
}

LYN_REPLACE_CHECK(SetUnitStatus);
void SetUnitStatus(struct Unit *unit, int status)
{
	if (status == 0) {
		SetUnitStatusIndex(unit, 0);
		SetUnitStatusDuration(unit, 0);
	} else if (status >= NEW_UNIT_STATUS_MAX) {
		Errorf("Status error: %d", status);
		hang();
	} else {
		int duration = gpDebuffInfos[status].duration;

#if defined(SID_Residium) && (COMMON_SKILL_VALID(SID_Residium))
    if (SkillTester(unit, SID_Residium))
    {
		for (int i = 0; i < (int)ARRAY_COUNT(buffs); i++)
		{
			if (buffs[i] == status)
				duration += 1;
		}
    }
#endif

#if defined(SID_Malidiction) && (COMMON_SKILL_VALID(SID_Malidiction))
    if (SkillTester(gSubjectUnit, SID_Malidiction))
    {
		for (int i = 0; i < (int)ARRAY_COUNT(buffs); i++)
		{
			if (debuffs[i] == status)
				duration += 1;
		}
    }
#endif

		if (duration == 0)
			duration = 3;

		LTRACEF("[pid=0x%02X] sttaus=%d, dura=%d", status, duration);

		SetUnitStatusIndex(unit, status);
		SetUnitStatusDuration(unit, duration);
	}
}

LYN_REPLACE_CHECK(SetUnitStatusExt);
void SetUnitStatusExt(struct Unit *unit, int status, int duration)
{
	if (status >= NEW_UNIT_STATUS_MAX) {
		Errorf("Status overflow: %d", status);
		hang();
	}

	if (duration == 0)
		duration = 3;

	SetUnitStatusIndex(unit, status);
	SetUnitStatusDuration(unit, duration);
}

LYN_REPLACE_CHECK(GetUnitStatusName);
char *GetUnitStatusName(struct Unit *unit)
{
	int msg = gpDebuffInfos[GetUnitStatusIndex(unit)].name;

	if (msg == 0)
		msg = 0x52B; /* --[X] */

	return GetStringFromIndex(msg);
}

LYN_REPLACE_CHECK(GetUnitRescueName);
char *GetUnitRescueName(struct Unit *unit)
{
	if (!unit->rescue)
		return GetStringFromIndex(gpDebuffInfos[UNIT_STATUS_NONE].name);

	return GetStringFromIndex(GetUnit(unit->rescue)->pCharacterData->nameTextId);
}

LYN_REPLACE_CHECK(HbPopulate_SSStatus);
void HbPopulate_SSStatus(struct HelpBoxProc *proc)
{
	u8 index = GetUnitStatusIndex(gStatScreen.unit);

	proc->mid = gpDebuffInfos[index].desc;
}

LYN_REPLACE_CHECK(PutUnitMapUiStatus);
void PutUnitMapUiStatus(u16 *buffer, struct Unit *unit)
{
	int tileIdx = 0x16F;
	const u8 *img;

	if (!unit)
		return;

	img = gpDebuffInfos[GetUnitStatusIndex(unit)].img;

	if (img) {
		CpuFastCopy(img, (void *)BG_VRAM + 0x2DE0, 0xA0);

		buffer[0] = tileIdx++;
		buffer[1] = tileIdx++;
		buffer[2] = tileIdx++;
		buffer[3] = tileIdx++;
		buffer[4] = tileIdx++;
		buffer[5] = 0;
		buffer[6] = TILEREF(0x128 + GetUnitStatusDuration(unit), 1);
	}
}

typedef struct
{
	const int key;
	const int values[3];
} DescriptionStrings;

static const DescriptionStrings character_description_strings[] =
{
	{0},
	{ CHARACTER_EIRIKA,    {MSG_Eirika_Description_One,    MSG_Eirika_Description_Two,    MSG_Eirika_Description_Three} },
	{ CHARACTER_SETH,      {MSG_Seth_Description_One,      MSG_Seth_Description_Two,      MSG_Seth_Description_Three} },
	{ CHARACTER_GILLIAM,   {MSG_Gilliam_Description_One,   MSG_Gilliam_Description_Two,   MSG_Gilliam_Description_Three} },
	{ CHARACTER_FRANZ,     {MSG_Franz_Description_One,     MSG_Franz_Description_Two,     MSG_Franz_Description_Three} },
	{ CHARACTER_MOULDER,   {MSG_Moulder_Description_One,   MSG_Moulder_Description_Two,   MSG_Moulder_Description_Three} },
	{ CHARACTER_VANESSA,   {MSG_Vanessa_Description_One,   MSG_Vanessa_Description_Two,   MSG_Vanessa_Description_Three} },
	{ CHARACTER_ROSS,      {MSG_Ross_Description_One,      MSG_Ross_Description_Two,      MSG_Ross_Description_Three} },
	{ CHARACTER_NEIMI,     {MSG_Neimi_Description_One,     MSG_Neimi_Description_Two,     MSG_Neimi_Description_Three} },
	{ CHARACTER_COLM,      {MSG_Colm_Description_One,      MSG_Colm_Description_Two,      MSG_Colm_Description_Three} },
	{ CHARACTER_GARCIA,    {MSG_Garcia_Description_One,    MSG_Garcia_Description_Two,    MSG_Garcia_Description_Three} },
	{ CHARACTER_INNES,     {MSG_Innes_Description_One,     MSG_Innes_Description_Two,     MSG_Innes_Description_Three} },
	{ CHARACTER_LUTE,      {MSG_Lute_Description_One,      MSG_Lute_Description_Two,      MSG_Lute_Description_Three} },
	{ CHARACTER_NATASHA,   {MSG_Natasha_Description_One,   MSG_Natasha_Description_Two,   MSG_Natasha_Description_Three} },
	{ CHARACTER_CORMAG,    {MSG_Cormag_Description_One,    MSG_Cormag_Description_Two,    MSG_Cormag_Description_Three} },
	{ CHARACTER_EPHRAIM,   {MSG_Ephraim_Description_One,   MSG_Ephraim_Description_Two,   MSG_Ephraim_Description_Three} },
	{ CHARACTER_FORDE,     {MSG_Forde_Description_One,     MSG_Forde_Description_Two,     MSG_Forde_Description_Three} },
	{ CHARACTER_KYLE,      {MSG_Kyle_Description_One,      MSG_Kyle_Description_Two,      MSG_Kyle_Description_Three} },
	{ CHARACTER_AMELIA,    {MSG_Amelia_Description_One,    MSG_Amelia_Description_Two,    MSG_Amelia_Description_Three} },
	{ CHARACTER_ARTUR,     {MSG_Artur_Description_One,     MSG_Artur_Description_Two,     MSG_Artur_Description_Three} },
	{ CHARACTER_GERIK,     {MSG_Gerik_Description_One,     MSG_Gerik_Description_Two,     MSG_Gerik_Description_Three} },
	{ CHARACTER_TETHYS,    {MSG_Tethys_Description_One,    MSG_Tethys_Description_Two,    MSG_Tethys_Description_Three} },
	{ CHARACTER_MARISA,    {MSG_Marisa_Description_One,    MSG_Marisa_Description_Two,    MSG_Marisa_Description_Three} },
	{ CHARACTER_SALEH,     {MSG_Saleh_Description_One,     MSG_Saleh_Description_Two,     MSG_Saleh_Description_Three} },
	{ CHARACTER_EWAN,      {MSG_Ewan_Description_One,      MSG_Ewan_Description_Two,      MSG_Ewan_Description_Three} },
	{ CHARACTER_LARACHEL,  {MSG_Larachel_Description_One,  MSG_Larachel_Description_Two,  MSG_Larachel_Description_Three} },
	{ CHARACTER_DOZLA,     {MSG_Dozla_Description_One,     MSG_Dozla_Description_Two,     MSG_Dozla_Description_Three} },
	{ CHARACTER_ENEMY_1B,  {MSG_Enemy_1B_Description_One,  MSG_Enemy_1B_Description_Two,  MSG_Enemy_1B_Description_Three} },
	{ CHARACTER_RENNAC,    {MSG_Rennac_Description_One,    MSG_Rennac_Description_Two,    MSG_Rennac_Description_Three} },
	{ CHARACTER_DUESSEL,   {MSG_Duessel_Description_One,   MSG_Duessel_Description_Two,   MSG_Duessel_Description_Three} },
	{ CHARACTER_MYRRH,     {MSG_Myrrh_Description_One,     MSG_Myrrh_Description_Two,     MSG_Myrrh_Description_Three} },
	{ CHARACTER_KNOLL,     {MSG_Knoll_Description_One,     MSG_Knoll_Description_Two,     MSG_Knoll_Description_Three} },
	{ CHARACTER_JOSHUA,    {MSG_Joshua_Description_One,    MSG_Joshua_Description_Two,    MSG_Joshua_Description_Three} },
	{ CHARACTER_SYRENE,    {MSG_Syrene_Description_One,    MSG_Syrene_Description_Two,    MSG_Syrene_Description_Three} },
	{ CHARACTER_TANA,      {MSG_Tana_Description_One,      MSG_Tana_Description_Two,      MSG_Tana_Description_Three} },
	{ CHARACTER_LYON,      {MSG_Lyon_Description_One,      MSG_Lyon_Description_Two,      MSG_Lyon_Description_Three} },
	{ CHARACTER_ORSON,     {MSG_Orson_Description_One,     MSG_Orson_Description_Two,     MSG_Orson_Description_Three} },
	{ CHARACTER_ORSON_CH5X,{MSG_Orson_CH5X_Description_One,MSG_Orson_CH5X_Description_Two,MSG_Orson_CH5X_Description_Three} },
	{ CHARACTER_GLEN,      {MSG_Glen_Description_One,      MSG_Glen_Description_Two,      MSG_Glen_Description_Three} },
	{ CHARACTER_SELENA,    {MSG_Selena_Description_One,    MSG_Selena_Description_Two,    MSG_Selena_Description_Three} },
	{ CHARACTER_VALTER,    {MSG_Valter_Description_One,    MSG_Valter_Description_Two,    MSG_Valter_Description_Three} },
	{ CHARACTER_RIEV,      {MSG_Riev_Description_One,      MSG_Riev_Description_Two,      MSG_Riev_Description_Three} },
	{ CHARACTER_CAELLACH,  {MSG_Caellach_Description_One,  MSG_Caellach_Description_Two,  MSG_Caellach_Description_Three} },
	{ CHARACTER_FADO,      {MSG_Fado_Description_One,      MSG_Fado_Description_Two,      MSG_Fado_Description_Three} },
	{ CHARACTER_ISMAIRE,   {MSG_Ismaire_Description_One,   MSG_Ismaire_Description_Two,   MSG_Ismaire_Description_Three} },
	{ CHARACTER_HAYDEN,    {MSG_Hayden_Description_One,    MSG_Hayden_Description_Two,    MSG_Hayden_Description_Three} },

};

LYN_REPLACE_CHECK(HbPopulate_SSCharacter);
void HbPopulate_SSCharacter(struct HelpBoxProc* proc)
{
	int activeUnitCharId = gStatScreen.unit->pCharacterData->number;
    int midDesc = gCharacterData_NEW[activeUnitCharId-1].descTextId;

	if (gpKernelDesignerConfig->variable_unit_descriptions == true)
	{
		switch (gPlaySt.chapterIndex)
		{
		case 1:
			midDesc = character_description_strings[gStatScreen.unit->pCharacterData->number].values[0];
			break;
		case 2:
			midDesc = character_description_strings[gStatScreen.unit->pCharacterData->number].values[1];
			break;
		case 3:
			midDesc = character_description_strings[gStatScreen.unit->pCharacterData->number].values[2];
			break;
		default:
			break;
		}
	}

    if (midDesc)
        proc->mid = midDesc;
    else
        proc->mid = 0x6BE; // TODO: mid constants
}