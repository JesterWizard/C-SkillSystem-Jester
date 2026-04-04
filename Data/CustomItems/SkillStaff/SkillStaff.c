#include "common-chax.h"
#include "item-sys.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "icon-rework.h"
#include "mapanim.h"
#include "popup.h"
#include "bm.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "bmphase.h"
#include "jester_headers/custom-functions.h"
#include "uimenu.h"
#include "uiselecttarget.h"
#include "proc.h"

#define SKILL_STAFF_DURATION 3
#define SKILL_STAFF_MAX_ACTIVE 16
#define SKILL_STAFF_MAX_CHOICES 4

extern u16 gUnknown_085A0D4C[];

struct SkillStaffGrantEntry {
    u8 unitId;
    u8 turnsRemaining;
    u16 skillId;
};

struct SkillStaffSuspendState {
    u8 activeCount;
    u8 lastTickTurnNumber;
    u8 _pad[2];
    struct SkillStaffGrantEntry grants[SKILL_STAFF_MAX_ACTIVE];
};

struct SkillStaffMenuState {
    u8 targetUnitId;
    u8 active;
    u8 accepted;
    u8 _pad;
    u16 skills[SKILL_STAFF_MAX_CHOICES];
    u8 count;
    u8 _pad2[3];
};

static const int sSkillStaffAdjTileOffsets[4][2] = {
    {-1, 0},
    { 1, 0},
    { 0,-1},
    { 0, 1},
};

static const u16 sSkillStaffGrantList[] = {
#if defined(SID_StaffSavant) && (COMMON_SKILL_VALID(SID_StaffSavant))
    SID_StaffSavant,
#endif
#if defined(SID_PowerStaff) && (COMMON_SKILL_VALID(SID_PowerStaff))
    SID_PowerStaff,
#endif
#if defined(SID_StaffGuard) && (COMMON_SKILL_VALID(SID_StaffGuard))
    SID_StaffGuard,
#endif
#if defined(SID_StaffParagon) && (COMMON_SKILL_VALID(SID_StaffParagon))
    SID_StaffParagon,
#endif
};

extern struct SkillStaffSuspendState sSkillStaffSuspendState;
extern struct SkillStaffMenuState sSkillStaffMenuState;

STATIC_DECLAR const struct MenuItemDef SkillStaffMenuItems[];
STATIC_DECLAR const struct MenuDef SkillStaffMenuDef;
STATIC_DECLAR const struct ProcCmd ProcScr_SkillStaffApply[];
static void SkillStaff_StartSkillMenu(ProcPtr proc);
static void SkillStaffMenu_ClearUi(void);
static bool SkillStaffMenu_IsRunning(ProcPtr proc);
static void SkillStaffMenu_Cleanup(ProcPtr proc);
static void MakeTargetListForSkillStaff(struct Unit *unit);

static void SkillStaff_ResetMenuState(void)
{
    memset(&sSkillStaffMenuState, 0, sizeof(sSkillStaffMenuState));
    sSkillStaffMenuState.count = 0;
    memset(sSkillStaffMenuState.skills, 0, sizeof(sSkillStaffMenuState.skills));
}

static void SkillStaff_DrawPortrait(struct Unit *unit)
{
    int portraitId;

    if (!UNIT_IS_VALID(unit))
        return;

    portraitId = GetUnitPortraitId(unit);
    CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);
    PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, portraitId, 0x200, 5);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static bool SkillStaff_IsGrantableSkill(struct Unit *unit, u16 sid)
{
    if (!UNIT_IS_VALID(unit))
        return false;

    if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
        return false;

    if (!AreUnitsAllied(gActiveUnit->index, unit->index))
        return false;

    if (GetFreeSkillSlot(unit) < 0)
        return false;

    if (SkillTester(unit, sid))
        return false;

    return true;
}

static bool SkillStaff_TargetHasAnyGrantableSkill(struct Unit *unit)
{
    int i;

    if (!UNIT_IS_VALID(unit))
        return false;

    for (i = 0; i < sSkillStaffSuspendState.activeCount; i++) {
        if (sSkillStaffSuspendState.grants[i].unitId == unit->index)
            return false;
    }

    for (i = 0; i < (int)ARRAY_COUNT(sSkillStaffGrantList); i++) {
        if (SkillStaff_IsGrantableSkill(unit, sSkillStaffGrantList[i]))
            return true;
    }

    return false;
}

static int SkillStaff_BuildGrantableSkillList(struct Unit *unit)
{
    int i;

    SkillStaff_ResetMenuState();
    sSkillStaffMenuState.targetUnitId = unit->index;

    for (i = 0; i < (int)ARRAY_COUNT(sSkillStaffGrantList); i++) {
        u16 sid = sSkillStaffGrantList[i];

        if (!SkillStaff_IsGrantableSkill(unit, sid))
            continue;

        if (sSkillStaffMenuState.count >= SKILL_STAFF_MAX_CHOICES)
            break;

        sSkillStaffMenuState.skills[sSkillStaffMenuState.count++] = sid;
    }

    return sSkillStaffMenuState.count;
}

static bool SkillStaff_AddTemporarySkill(struct Unit *unit, u16 sid)
{
    int slot;

    if (!UNIT_IS_VALID(unit))
        return false;

    if (SkillStaff_IsGrantableSkill(unit, sid) == false)
        return false;

    if (sSkillStaffSuspendState.activeCount >= SKILL_STAFF_MAX_ACTIVE)
        return false;

    slot = GetFreeSkillSlot(unit);
    if (slot < 0)
        return false;

    SET_SKILL(unit, slot, sid);
    ResetSkillLists();

    sSkillStaffSuspendState.grants[sSkillStaffSuspendState.activeCount].unitId = unit->index;
    sSkillStaffSuspendState.grants[sSkillStaffSuspendState.activeCount].turnsRemaining = SKILL_STAFF_DURATION;
    sSkillStaffSuspendState.grants[sSkillStaffSuspendState.activeCount].skillId = sid;
    sSkillStaffSuspendState.activeCount++;
    sSkillStaffSuspendState.lastTickTurnNumber = (u8)gPlaySt.chapterTurnNumber;

    return true;
}

static void SkillStaff_RemoveGrantAt(int index)
{
    int i;

    if (index < 0 || index >= sSkillStaffSuspendState.activeCount)
        return;

    for (i = index + 1; i < sSkillStaffSuspendState.activeCount; i++)
        sSkillStaffSuspendState.grants[i - 1] = sSkillStaffSuspendState.grants[i];

    sSkillStaffSuspendState.activeCount--;
}

static void SkillStaff_ClearSuspendState(void)
{
    int i;

    for (i = 0; i < sSkillStaffSuspendState.activeCount; i++) {
        struct Unit *unit = GetUnit(sSkillStaffSuspendState.grants[i].unitId);

        if (UNIT_IS_VALID(unit))
            RemoveSkill(unit, sSkillStaffSuspendState.grants[i].skillId);
    }

    memset(&sSkillStaffSuspendState, 0, sizeof(sSkillStaffSuspendState));
    sSkillStaffSuspendState.lastTickTurnNumber = 0xFF;
}

static void SkillStaff_TickSuspendState(void)
{
    int i = 0;

    while (i < sSkillStaffSuspendState.activeCount) {
        struct SkillStaffGrantEntry *entry = &sSkillStaffSuspendState.grants[i];
        struct Unit *unit = GetUnit(entry->unitId);

        if (entry->turnsRemaining > 0)
            entry->turnsRemaining--;

        if (entry->turnsRemaining == 0) {
            if (UNIT_IS_VALID(unit))
                RemoveSkill(unit, entry->skillId);

            SkillStaff_RemoveGrantAt(i);
            continue;
        }

        i++;
    }
}

static void SkillStaff_TickCurrentTurn(void)
{
    u8 currentTurnNumber = (u8)gPlaySt.chapterTurnNumber;

    if (sSkillStaffSuspendState.lastTickTurnNumber == currentTurnNumber)
        return;

    sSkillStaffSuspendState.lastTickTurnNumber = currentTurnNumber;
    SkillStaff_TickSuspendState();
}

static bool SkillStaff_PopupRunning(ProcPtr proc)
{
    return Proc_Exists(ProcScr_Popup);
}

static void SkillStaff_ShowExpBar(ProcPtr proc)
{
    if (!sSkillStaffMenuState.accepted)
        return;

    if (gBattleActor.expGain <= 0)
        return;

    gManimSt.actorCount = 1;
    gManimSt.hp_changing = 0;
    gManimSt.subjectActorId = 0;
    gManimSt.targetActorId = 0;
    SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);

    {
        struct MAExpBarProc *barProc = Proc_StartBlocking(ProcScr_MapAnimExpBar, proc);

        barProc->expFrom = gBattleActor.expPrevious;
        barProc->expTo = gBattleActor.expPrevious + gBattleActor.expGain;
        barProc->actorId = 0;
    }
}

static bool SkillStaff_ExpBarRunning(ProcPtr proc)
{
    return Proc_Exists(ProcScr_MapAnimExpBar);
}

static bool SkillStaffMenu_IsRunning(ProcPtr proc)
{
    return sSkillStaffMenuState.active;
}

static void SkillStaffMenu_Cleanup(ProcPtr proc)
{
    SkillStaffMenu_ClearUi();
    SkillStaff_ResetMenuState();
}

static bool SkillStaff_ApplyCanProceed(struct Unit *unit, struct Unit *target)
{
    return UNIT_IS_VALID(unit) && UNIT_IS_VALID(target);
}

static void SkillStaff_ApplyEffect(ProcPtr proc)
{
    struct Unit *unit = GetUnit(gActionData.subjectIndex);
    struct Unit *target = GetUnit(gActionData.targetIndex);
    u16 sid = gActionData.unk08;

    if (!sSkillStaffMenuState.accepted) {
        return;
    }

    if (!SkillStaff_ApplyCanProceed(unit, target))
        return;

    BattleInitItemEffect(unit, gActionData.itemSlotIndex);
    BattleInitItemEffectTarget(target);

    if (SkillStaff_AddTemporarySkill(target, sid)) {
        SetPopupUnit(target);
        SetPopupItem(sid);
        NewPopup_Simple(PopupScr_LearnSkill, 0x5A, 0, proc);
    }

    BattleApplyItemEffect(proc);
}

STATIC_DECLAR const struct ProcCmd ProcScr_SkillStaffApply[] = {
    PROC_CALL(SkillStaff_StartSkillMenu),
    PROC_WHILE(SkillStaffMenu_IsRunning),
    PROC_CALL(SkillStaff_ApplyEffect),
    PROC_WHILE(SkillStaff_PopupRunning),
    PROC_CALL(SkillStaff_ShowExpBar),
    PROC_WHILE(SkillStaff_ExpBarRunning),
    PROC_CALL(SkillStaffMenu_Cleanup),
    PROC_END,
};

static bool SkillStaff_UnitTargetListTryAdd(int x, int y)
{
    struct Unit *unit = GetUnitAtPosition(x, y);

    if (!SkillStaff_TargetHasAnyGrantableSkill(unit))
        return false;

    AddTarget(x, y, unit->index, 0);
    return true;
}

static void MakeTargetListForSkillStaff(struct Unit *unit)
{
    int i;

    gSubjectUnit = unit;
    InitTargets(unit->xPos, unit->yPos);
    BmMapFill(gBmMapRange, 0);

    for (i = 0; i < 4; i++) {
        int x = unit->xPos + sSkillStaffAdjTileOffsets[i][0];
        int y = unit->yPos + sSkillStaffAdjTileOffsets[i][1];

        if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
            continue;

        SkillStaff_UnitTargetListTryAdd(x, y);
    }
}

static u8 SkillStaffMenu_Usability(const struct MenuItemDef *self, int number)
{
    struct Unit *target;

    if (number >= sSkillStaffMenuState.count)
        return MENU_NOTSHOWN;

    target = GetUnit(sSkillStaffMenuState.targetUnitId);

    if (SkillStaff_IsGrantableSkill(target, sSkillStaffMenuState.skills[number]))
        return MENU_ENABLED;

    return MENU_NOTSHOWN;
}

static int SkillStaffMenu_OnDraw(struct MenuProc *menu, struct MenuItemProc *item)
{
    u16 sid;

    (void)menu;

    if (item->itemNumber < 0 || item->itemNumber >= sSkillStaffMenuState.count)
        return 0;

    sid = sSkillStaffMenuState.skills[item->itemNumber];

    ClearText(&item->text);
    Text_SetColor(&item->text, TEXT_COLOR_SYSTEM_GOLD);
    Text_DrawString(&item->text, GetSkillNameStr(sid));
    DrawIcon(
        TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile, item->yTile),
        SKILL_ICON(sid),
        TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));
    PutText(&item->text, TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 2, item->yTile));
    BG_EnableSyncByMask(BG0_SYNC_BIT);

    return 0;
}

static u8 SkillStaffMenu_HelpBox(struct MenuProc *menu, struct MenuItemProc *item)
{
    u16 sid;

    (void)menu;

    if (item->itemNumber < 0 || item->itemNumber >= sSkillStaffMenuState.count)
        return 0;

    sid = sSkillStaffMenuState.skills[item->itemNumber];
    StartHelpBox(item->xTile * 8, item->yTile * 8, GetSkillDescMsg(sid));

    return 0;
}

static void SkillStaffMenu_ClearUi(void)
{
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);
    HideSysHandCursor();
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

static int SkillStaffMenu_OnSwitchIn(struct MenuProc *menu, struct MenuItemProc *item)
{
    struct Unit *target;

    (void)menu;
    (void)item;

    target = GetUnit(sSkillStaffMenuState.targetUnitId);
    SkillStaff_DrawPortrait(target);

    return 0;
}

static u8 SkillStaffMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item)
{
    gActionData.unitActionType = 0;
    gActionData.targetIndex = 0;
    gActionData.unk08 = 0;

    sSkillStaffMenuState.active = 0;
    sSkillStaffMenuState.accepted = 0;

    SkillStaffMenu_ClearUi();

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 SkillStaffMenu_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    u16 sid;

    if (item->itemNumber < 0 || item->itemNumber >= sSkillStaffMenuState.count)
        return 0;

    sid = sSkillStaffMenuState.skills[item->itemNumber];
    gActionData.unk08 = sid;
    sSkillStaffMenuState.accepted = 1;
    sSkillStaffMenuState.active = 0;

    SkillStaffMenu_ClearUi();

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

STATIC_DECLAR const struct MenuItemDef SkillStaffMenuItems[] = {
    { .isAvailable = SkillStaffMenu_Usability, .onDraw = SkillStaffMenu_OnDraw, .onSelected = SkillStaffMenu_OnSelected, .onSwitchIn = SkillStaffMenu_OnSwitchIn },
    { .isAvailable = SkillStaffMenu_Usability, .onDraw = SkillStaffMenu_OnDraw, .onSelected = SkillStaffMenu_OnSelected, .onSwitchIn = SkillStaffMenu_OnSwitchIn },
    { .isAvailable = SkillStaffMenu_Usability, .onDraw = SkillStaffMenu_OnDraw, .onSelected = SkillStaffMenu_OnSelected, .onSwitchIn = SkillStaffMenu_OnSwitchIn },
    { .isAvailable = SkillStaffMenu_Usability, .onDraw = SkillStaffMenu_OnDraw, .onSelected = SkillStaffMenu_OnSelected, .onSwitchIn = SkillStaffMenu_OnSwitchIn },
    { 0 }
};

STATIC_DECLAR const struct MenuDef SkillStaffMenuDef = {
    {15, 1, 12, 0},
    0,
    SkillStaffMenuItems,
    0, 0, 0,
    SkillStaffMenu_OnCancel,
    MenuAutoHelpBoxSelect,
    SkillStaffMenu_HelpBox,
};

static void SkillStaff_StartSkillMenu(ProcPtr proc)
{
    struct MenuProc *menu;
    struct Unit *target;

    target = GetUnit(sSkillStaffMenuState.targetUnitId);

    ResetIconGraphics();
    LoadIconPalettes(4);
    menu = StartOrphanMenu(&SkillStaffMenuDef);

    sSkillStaffMenuState.active = 1;
    sSkillStaffMenuState.accepted = 0;

    SkillStaff_DrawPortrait(target);

    StartSubtitleHelp(menu, GetStringFromIndex(MSG_ITEM_SKILL_STAFF_SKILL_SUBTITLE));
}

static u8 SkillStaff_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
    struct Unit *unit = GetUnit(target->uid);

    if (!SkillStaff_BuildGrantableSkillList(unit)) {
        gActionData.unitActionType = 0;
        gActionData.targetIndex = 0;
        gActionData.unk08 = 0;

        return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
    }

    gActionData.targetIndex = target->uid;
    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

static bool SkillStaff_Usability(struct Unit *unit, int item)
{
    if (unit->state & US_CANTOING)
        return false;

    SkillStaff_ResetMenuState();
    MakeTargetListForSkillStaff(unit);

    return GetSelectTargetCount() > 0;
}

void ChapterInit_ResetSkillStaffTempState(void)
{
    SkillStaff_ClearSuspendState();
}

bool PrePhase_TickSkillStaffTempState(ProcPtr proc)
{
    SkillStaff_TickCurrentTurn();
    return false;
}

void SaveSkillStaffSuspendState(u8 *dst, const u32 size)
{
    Assert(size == sizeof(sSkillStaffSuspendState));
    WriteAndVerifySramFast(&sSkillStaffSuspendState, dst, size);
}

void LoadSkillStaffSuspendState(u8 *src, const u32 size)
{
    Assert(size == sizeof(sSkillStaffSuspendState));
    ReadSramFast(src, &sSkillStaffSuspendState, size);
}

bool IER_Usability_SkillStaff(struct Unit *unit, int item)
{
    return SkillStaff_Usability(unit, item);
}

void IER_Effect_SkillStaff(struct Unit *unit, int item)
{
    gActionData.unk08 = ITEM_STAFF_SKILL;
    gActionData.subjectIndex = unit->index;
    SetStaffUseAction(unit);

    SkillStaff_ResetMenuState();
    MakeTargetListForSkillStaff(unit);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_PutTrap, SkillStaff_OnSelectTarget),
        GetStringFromIndex(MSG_ITEM_SKILL_STAFF_SUBTITLE));
}

void IER_Action_SkillStaff(ProcPtr proc, struct Unit *unit, int item)
{
    Proc_StartBlocking(ProcScr_SkillStaffApply, proc);
}