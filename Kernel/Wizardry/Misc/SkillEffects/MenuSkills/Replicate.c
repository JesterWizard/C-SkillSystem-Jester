#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Replicate) && (COMMON_SKILL_VALID(SID_Replicate))
static bool IsBattleUnitProxy(const struct Unit *unit)
{
	return unit == &gBattleActor.unit || unit == &gBattleTarget.unit;
}

struct Unit *GetReplicateLinkedUnit(struct Unit *unit)
{
	struct Unit *linked;

	if (!UNIT_IS_VALID(unit))
		return NULL;

	if (unit->_u3B == 0 || unit->_u3B == unit->index)
		return NULL;

	linked = GetUnit(unit->_u3B);

	if (!UNIT_IS_VALID(linked))
		return NULL;

	if (linked->_u3B != unit->index)
		return NULL;

	return linked;
}

void ClearReplicateUnitLink(struct Unit *unit)
{
	struct Unit *linked;
	u8 linkedIndex;

	if (!UNIT_IS_VALID(unit) || unit->_u3B == 0)
		return;

	linkedIndex = unit->_u3B;
	unit->_u3B = 0;

	linked = GetUnit(linkedIndex);

	if (UNIT_IS_VALID(linked) && linked->_u3B == unit->index)
		linked->_u3B = 0;
}

void LinkReplicateUnits(struct Unit *unit_a, struct Unit *unit_b)
{
	if (!UNIT_IS_VALID(unit_a) || !UNIT_IS_VALID(unit_b) || unit_a == unit_b)
		return;

	ClearReplicateUnitLink(unit_a);
	ClearReplicateUnitLink(unit_b);

	unit_a->_u3B = unit_b->index;
	unit_b->_u3B = unit_a->index;
}

void SyncReplicateLinkedHp(struct Unit *unit)
{
	struct Unit *linked;
	int hp;

	if (!UNIT_IS_VALID(unit) || IsBattleUnitProxy(unit))
		return;

	linked = GetReplicateLinkedUnit(unit);

	if (!linked)
		return;

	hp = unit->curHP;

	if (hp < 0)
		hp = 0;

	if (hp > GetUnitMaxHp(unit))
		hp = GetUnitMaxHp(unit);

	unit->curHP = hp;

	if (hp > GetUnitMaxHp(linked))
		hp = GetUnitMaxHp(linked);

	linked->curHP = hp;
}

int Replicate_OnBattleToUnit(struct BattleUnit *bu, struct Unit *unit)
{
	SyncReplicateLinkedHp(unit);
	return 0;
}

void Replicate_OnClearUnit(struct Unit *unit)
{
	ClearReplicateUnitLink(unit);
}

void Replicate_OnUnitKill(struct Unit *unit)
{
	struct Unit *linked = GetReplicateLinkedUnit(unit);

	if (!linked)
		return;

	ClearReplicateUnitLink(unit);

	if (linked->state & (US_DEAD | US_BIT16))
		return;

	linked->curHP = 0;
	UnitKill(linked);
}

u8 Replicate_Usability(const struct MenuItemDef *def, int number)
{

	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	/* If a replicant already exists, we don't want to create another */
	for (int i = FACTION_BLUE; i < FACTION_GREEN; i++)
	{
		struct Unit * unit = GetUnit(i);
		
		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_DEAD | US_BIT16))
			continue;

		if ((unit->pCharacterData->number == gActiveUnit->pCharacterData->number) && GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_REPLICATE)
			return MENU_NOTSHOWN;
	}

	if (!HasSelectTarget(gActiveUnit, MakeTargetListForLightRune))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;

}

static u8 Replicate_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	gActionData.unk08 = SID_Replicate;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Replicate_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{

	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	MakeTargetListForLightRune(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Replicate_OnSelectTarget),
		GetStringFromIndex(0x87E));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void SummonReplicaUnit(int x, int y)
{
    gUnitDef1.charIndex       = UNIT_CHAR_ID(gActiveUnit);
    gUnitDef1.classIndex      = UNIT_CLASS_ID(gActiveUnit);
    gUnitDef1.autolevel       = TRUE;
    gUnitDef1.allegiance      = FACTION_ID_BLUE;
    gUnitDef1.level           = gActiveUnit->level;
    gUnitDef1.xPosition       = x;
    gUnitDef1.yPosition       = y;
    gUnitDef1.redaCount       = 0;
    gUnitDef1.redas           = NULL;
    gUnitDef1.genMonster      = FALSE;
    gUnitDef1.itemDrop        = FALSE;
    gUnitDef1.items[0]        = gActiveUnit->items[0];
    gUnitDef1.items[1]        = gActiveUnit->items[1];
    gUnitDef1.items[2]        = gActiveUnit->items[2];
    gUnitDef1.items[3]        = gActiveUnit->items[3];
    //gUnitDef1.items[4]        = gActiveUnit->items[4];

    LoadUnits(&gUnitDef1);
}

static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(
        BG_0,
        -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
        -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

// Prepare the screen before summoning.
static void PrepareMenuReplica(void)
{
    //EndAllMus();
	SummonReplicaUnit(gActionData.xOther, gActionData.yOther);
	SyncReplicateLinkedHp(gActiveUnit);
	RefreshUnitSprites();
}

static void set_actor_unit(void)
{
    gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

static const EventScr EventScr_MenuReplica[] = {
    CALL(EventScr_MuSkillAnim)
	ASMC(callback_anim)
    ASMC(PrepareMenuReplica)
    ASMC(set_actor_unit)
    NOFADE
    ENDA
};


bool Action_Replicate(ProcPtr parent)
{
	// NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
    KernelCallEvent(EventScr_MenuReplica, EV_EXEC_CUTSCENE, parent);
	return true;
}
#endif
