#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "weapon-slots.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

void PrepItemUseArmsScroll_OnEnd(struct ProcPrepItemUseJunaFruit *proc)
{
	struct ProcPrepItemUse *parent = proc->proc_parent;

    gActionData.unk08 = -1;
    int weaponRank = GetHighestWeaponRank(parent->unit);

    if (weaponRank > 7)
        return;
    else 
        if (parent->unit->ranks[weaponRank] >= WPN_EXP_A)
            parent->unit->ranks[weaponRank] = WPN_EXP_S;
        else if (parent->unit->ranks[weaponRank] >= WPN_EXP_B)
            parent->unit->ranks[weaponRank] = WPN_EXP_A;
        else if (parent->unit->ranks[weaponRank] >= WPN_EXP_C)
            parent->unit->ranks[weaponRank] = WPN_EXP_B;
        else if (parent->unit->ranks[weaponRank] >= WPN_EXP_D)
            parent->unit->ranks[weaponRank] = WPN_EXP_C;
        else if (parent->unit->ranks[weaponRank] >= WPN_EXP_E)
            parent->unit->ranks[weaponRank] = WPN_EXP_D;

    int slot = gActionData.itemSlotIndex;

	UnitUpdateUsedItem(parent->unit, slot);
	PrepItemUseJuna_OnEnd(proc);
}

void PrepItemUseArmsScroll_OnDraw(struct ProcPrepItemUseJunaFruit * proc, int item, int x, int y)
{
    const char * str = GetStringFromIndex(MSG_Weapon_Rank_Increased);
    struct Text * text = &gPrepItemTexts[TEXT_PREPITEM_POPUP];
    int width = GetStringTextLen(str);

    ClearText(text);
    PutDrawText(
        text,
        TILEMAP_LOCATED(gBG2TilemapBuffer, x - 1, y),
        TEXT_COLOR_SYSTEM_WHITE,
        0, 0, str
    );

    BG_EnableSyncByMask(BG2_SYNC_BIT);

    proc->xpos = x * 7;
    proc->ypos = y * 8 - 4;
    proc->width = width / 8 + 3;
    proc->height = 2;
}

void PrepItemUseArmsScroll_OnInit(struct ProcPrepItemUseJunaFruit *proc)
{
	struct ProcPrepItemUse *parent = proc->proc_parent;

	DrawPrepScreenItemUseStatBars(parent->unit, 0);
	DrawPrepScreenItemUseStatValues(parent->unit);

	PrepItemUseArmsScroll_OnDraw(proc, parent->unit->items[parent->slot], 0x11, 0x0E);

	proc->timer = 0x78;
	PlaySoundEffect(0x5A);
}

const struct ProcCmd ProcScr_PrepItemUseArmsScroll[] = {
	PROC_SET_END_CB(PrepItemUseArmsScroll_OnEnd),
	PROC_CALL(PrepItemUseArmsScroll_OnInit),
	PROC_REPEAT(PrepItemUseJuna_IDLE),
	PROC_END
};

int GetHighestWeaponRank(struct Unit * unit) 
{
    int weaponLevel = 0; // 0 means the unit doesn't have a rank in this weapon type
    int weaponRank = -1; // If we return -1, then there are no weapon ranks we can increase for this unit
    int i;

    for (i = 0; i < UNIT_WEAPON_SLOT_COUNT; i++)
    {
        int wtype = GetUnitWeaponSlotType(unit, i);

        if (wtype == WEAPON_SLOT_NONE)
            continue;

        if (unit->ranks[i] > weaponLevel && unit->ranks[i] < WPN_EXP_S)
        {
            weaponLevel = unit->ranks[i];
            weaponRank = i;
        }
    }

    return weaponRank;
}

bool CanUnitUseArmsScroll(struct Unit * unit) 
{
    if (GetHighestWeaponRank(unit) == -1)
        return false;
    else 
        return true;
};

void ItemUseAction_ArmsScroll(struct Unit * unit)
{
    gActionData.unk08 = -1;
    int weaponRank = GetHighestWeaponRank(unit);
    SetItemUseAction(unit);

    if (weaponRank > 7)
        return;
    else 
        if (unit->ranks[weaponRank] >= WPN_EXP_A)
            unit->ranks[weaponRank] = WPN_EXP_S;
        else if (unit->ranks[weaponRank] >= WPN_EXP_B)
            unit->ranks[weaponRank] = WPN_EXP_A;
        else if (unit->ranks[weaponRank] >= WPN_EXP_C)
            unit->ranks[weaponRank] = WPN_EXP_B;
        else if (unit->ranks[weaponRank] >= WPN_EXP_D)
            unit->ranks[weaponRank] = WPN_EXP_C;
        else if (unit->ranks[weaponRank] >= WPN_EXP_E)
            unit->ranks[weaponRank] = WPN_EXP_D;

    int slot = gActionData.itemSlotIndex;
    UnitUpdateUsedItem(unit, slot);
    NewPopup_VerySimple(MSG_Weapon_Rank_Increased, 0x5A, Proc_Find(gProcScr_PlayerPhase));
}

// /**
//  * IER port
//  */
bool IER_Usability_ArmsScroll(struct Unit *unit, int item)
{
	return CanUnitUseArmsScroll(unit);
}

bool IER_PrepUsability_ArmsScroll(struct Unit *unit, int item)
{
	return CanUnitUseArmsScroll(unit);
}

void IER_ActionEffect_ArmsScroll(ProcPtr proc, struct Unit *unit, int item)
{
	ItemUseAction_ArmsScroll(unit);
}

void IER_PrepEffect_ArmsScroll(struct ProcPrepItemUse * proc, u16 item)
{
    Proc_StartBlocking(ProcScr_PrepItemUseArmsScroll, proc);
}