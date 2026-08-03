
// LynJump.event replacements (detector round-trip)
LYN_REPLACE_CHECK(EquipUnitItemSlot);
LYN_REPLACE_CHECK(UnitChangeFaction);
LYN_REPLACE_CHECK(IsExtraBonusClaimEnabled);
LYN_REPLACE_CHECK(sub_80AA550);
LYN_REPLACE_CHECK(sub_80B0674);
LYN_REPLACE_CHECK(sub_80B06FC);
LYN_REPLACE_CHECK(InitBonusClaimData);
LYN_REPLACE_CHECK(DrawBonusClaimItemText);
LYN_REPLACE_CHECK(SetBonusItemClaimed);
LYN_REPLACE_CHECK(SetupBonusClaimTargets);
LYN_REPLACE_CHECK(BonusClaim_Init);
LYN_REPLACE_CHECK(BonusClaim_Loop_MainKeyHandler);
LYN_REPLACE_CHECK(BonusClaim_DrawTargetUnitSprites);
LYN_REPLACE_CHECK(sub_80B1008);
LYN_REPLACE_CHECK(TryClaimBonusItem);
LYN_REPLACE_CHECK(BonusClaim_Loop_SelectTargetKeyHandler);
LYN_REPLACE_CHECK(BonusClaim_EndSelectTargetSubMenu);
LYN_REPLACE_CHECK(BonusClaim_DrawItemSentPopup);
LYN_REPLACE_CHECK(BonusClaim_Loop_PopupDisplayTimer);
LYN_REPLACE_CHECK(BonusClaim_ClearItemSentPopup);
LYN_REPLACE_CHECK(BonusClaim_OnEnd);
LYN_REPLACE_CHECK(StartBonusClaimScreen);

// the entirety of BonusClaim.c replaces functions, too
void EquipUnitItemSlot(struct Unit * unit, int itemSlot)
{
    int item = unit->items[itemSlot];

    switch (itemSlot) // compiler was using memmove and causing a crash, so I swapped to a switch case
    {
        case 4:
            unit->items[4] = unit->items[3]; // no break;
        case 3:
            unit->items[3] = unit->items[2];
        case 2:
            unit->items[2] = unit->items[1];
        case 1:
            unit->items[1] = unit->items[0];
    }

    unit->items[0] = item;

    if (UNIT_FACTION(unit) == FACTION_BLUE)
    {
        UnlockAchievementByEquip(ITEM_INDEX(item));
    }
}

void UnitChangeFaction(struct Unit * unit, int faction)
{
    struct Unit * newUnit = GetFreeUnit(faction);

    if (gActiveUnit == unit)
        gActiveUnit = newUnit;

    if (gActiveUnitId == unit->index)
        gActiveUnitId = newUnit->index;

    if (gActionData.subjectIndex == unit->index)
        gActionData.subjectIndex = newUnit->index;

    CopyUnit(unit, newUnit);
    ClearUnit(unit);

    if (newUnit->exp == UNIT_EXP_DISABLED)
    {
        if ((faction == FACTION_BLUE) && (newUnit->level != UNIT_LEVEL_MAX))
            newUnit->exp = 0;
        else
            newUnit->exp = UNIT_EXP_DISABLED;
    }

    newUnit->state = newUnit->state & ~US_DROP_ITEM;

    if (newUnit->rescue)
        GetUnit(newUnit->rescue)->rescue = newUnit->index;

    if (faction == FACTION_BLUE)
    {
        UnlockAchievementByRecruitment(newUnit->pCharacterData->number);
    }
}

//! FE8U = 0x0808CB34
void TerrainDisplay_Init_asdf(struct PlayerInterfaceProc * proc) // start
{
    proc->windowQuadrant = -1;
    proc->isRetracting = false;
    proc->showHideClock = 0;
    proc->cursorQuadrant = 1;

    InitTextDb(proc->texts, 5);

    RestartNotificationProc(proc);
    // CreateBonusContentData();

    return;
}
