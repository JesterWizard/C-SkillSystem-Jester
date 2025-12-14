#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "battle-system.h"
#include "unit-expa.h"
#include "eventinfo.h"
#include "bmmenu.h"
#include "eventscript.h"
#include "EAstdlib.h"
#include "worldmap.h"
#include "constants/texts.h"
#include "action-expa.h"
#include "item-sys.h"
#include "player_interface.h"
#include "soundroom.h"
#include "bwl.h"
#include "debuff.h"
#include "traps.h"
#include "efxmagic.h"

#include "jester_headers/event-call.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"

#ifdef CONFIG_FORGING
#include "jester_headers/Forging.h"
#endif

typedef struct {
    /* 00 */ struct Font font;
    /* 18 */ struct Text th[18];
} PrepItemSuppyText;
extern const int sExpaConvoyItemAmount;

#if defined(SID_SummonPlus) && (COMMON_SKILL_VALID(SID_SummonPlus))
FORCE_DECLARE static const u8 classIndexes_SP[6] = { CLASS_TARVOS, CLASS_BAEL, CLASS_GARGOYLE, CLASS_GORGONEGG, CLASS_MOGALL, CLASS_MAUTHEDOOG };
FORCE_DECLARE static const u8 classPromotedIndexes_SP[6] = { CLASS_MAELDUIN, CLASS_ELDER_BAEL, CLASS_DEATHGOYLE, CLASS_GORGON, CLASS_ARCH_MOGALL, CLASS_GWYLLGI };

FORCE_DECLARE static const u8 classWeapons_SP[6] = { ITEM_AXE_IRON, ITEM_MONSTER_POISON_CLAW, ITEM_LANCE_JAVELIN, ITEM_MONSTER_DEMON_SURGE, ITEM_MONSTER_EVIL_EYE, ITEM_MONSTER_FIRE_FANG };
FORCE_DECLARE static const u8 classPromotedWeapons_SP[6] = { ITEM_AXE_BRAVE, ITEM_MONSTER_LETHAL_TALON, ITEM_LANCE_SPEAR, ITEM_MONSTER_SHADOW_SHOT, ITEM_MONSTER_CRIMSON_EYE, ITEM_MONSTER_HELL_FANG };
#endif

//! FE8U = 0x08098620
LYN_REPLACE_CHECK(PrepItemScreen_SetupGfx);
void PrepItemScreen_SetupGfx(struct PrepItemScreenProc* proc)
{
    int i;

    struct FaceVramEntry faceConfig[4] = {
        { 0x5800, 6, },
        { 0x6800, 7, },
        { 0x0000, 0, },
        { 0x0000, 0, },
    };

    SetupBackgrounds(gBgConfig_ItemUseScreen);

    gLCDControlBuffer.dispcnt.mode = 0;

    SetupFaceGfxData(faceConfig);

    SetDispEnable(0, 0, 0, 0, 0);

    SetupDebugFontForOBJ(0x06017800, 0);

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetText();
    ResetIconGraphics_();
    LoadIconPalettes(4);
    LoadUiFrameGraphics();
    ApplyPalette(gUiFramePaletteD, 2);
    LoadObjUIGfx();

    MakePrepUnitList();
    proc->hoverUnitIdx = UnitGetIndexInPrepList(PrepGetLatestCharId());

    ResetSysHandCursor(proc);
    StartParallelWorker(sub_809A274, proc);
    StartUiCursorHand(proc);

    SetPrimaryHBlankHandler(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    gPaletteBuffer[PAL_BACKDROP_OFFSET] = 0;
    EnablePaletteSync();

    for (i = 0; i < 15; i++) {
        InitText(gPrepItemTexts + i, 5);
    }

    for (i = 0; i < 5; i++) {
        InitText(&gPrepItemTexts[15 + i], 7);
        InitText(&gPrepItemTexts[20 + i], 7);
    }

    InitTextDb(&gPrepItemTexts[25], 8);
    InitTextDb(&gPrepItemTexts[26], 8);
    InitText(&gPrepItemTexts[27], 8);
    InitText(&gPrepItemTexts[30], 7);
    InitText(&gPrepItemTexts[31], 5);

    LoadHelpBoxGfx((void*)0x06014000, -1);

    SetBlendBackdropA(0);
    SetBlendBackdropB(0);

    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 0, 0);

    gLCDControlBuffer.wincnt.wout_enableBlend = 1;

    SetWinEnable(1, 0, 0);
    SetWin0Box(0, 4, 240, 68);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWOutLayers(1, 1, 0, 1, 1);

    BG_SetPosition(0, 4, -4);
    BG_SetPosition(1, 0, 4);
    BG_SetPosition(2, -40, (proc->unk_34 - 4) & 0xff);
    BG_EnableSyncByMask(7);

    ApplyUnitSpritePalettes();
    CpuFastFill(0, PAL_OBJ(0x0B), 0x20);

    ForceSyncUnitSpriteSheet();

    Decompress(Img_PrepTextShadow, (void*)0x06013E00);
    UiCursorHand_80ACA4C(0, 0, 0, 0xd0, 0x3c);
    DisplaySysHandCursorTextShadow(0x600, 1);

    RestartMuralBackground();

    if (proc->selectedUnitIdx != 0xff) {
        SetUiCursorHandConfig(
            0,
            ((proc->selectedUnitIdx % 3) * 64) + 24,
            ((proc->selectedUnitIdx / 3) * 16) + 4 - proc->unk_34,
            2
        );
        UpdatePrepItemScreenFace(0, GetUnitFromPrepList(proc->selectedUnitIdx), 60, 76, 0x0503);
    }

    StartMenuScrollBarExt(proc, 224, 11, 0x200, 4);

    UpdateMenuScrollBarConfig(6, proc->unk_34, ((PrepGetUnitAmount() - 1) / 3) + 1, 4);
    sub_8097668();
    PrepUpdateSMS();
    SetBlendConfig(1, 8, 8, 0);
    SetBlendTargetA(0, 1, 0, 0, 0);
    SetBlendTargetB(0, 1, 0, 0, 0);

    SetPrimaryHBlankHandler(PrepItemScreen_OnHBlank);

    StartDrawPrepFundsSprite(168, 133, 11, proc);

    StartDrawPrepUnitInfoBgSprites(proc);

    return;
}

//! FE8U = 0x0809B86C
LYN_REPLACE_CHECK(PrepItemTrade_Init);
void PrepItemTrade_Init(struct PrepMenuTradeProc* proc)
{
    const char* str;
    int i;

    struct FaceVramEntry faceConfig[4] = {
        { 0x5800, 6 },
        { 0x6800, 7 },
        { 0x0000, 0 },
        { 0x0000, 0 }
    };

    // TODO: Seems to be necessary to match - there is a "ldrh" here...
    SetupBackgrounds((void*)(u32)*gBgConfig_ItemUseScreen);

    SetupFaceGfxData(faceConfig);

    gLCDControlBuffer.bg0cnt.priority = 1;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, 0);

#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06012000, -1);
#else
    LoadHelpBoxGfx((void*)0x06014000, -1);
#endif

    LoadIconPalettes(4);

    RestartMuralBackground();

    for (i = 0; i < 5; i++) {
        InitTextDb(gPrepItemTexts + 15 + i, 7);
        InitTextDb(gPrepItemTexts + 20 + i, 7);
    }

    proc->selectedItemSlot = 0xff;

    if (proc->unk_40 != -1) {
        proc->cursorItemSlot = proc->unk_40 + 8;
    }
    else {
        if (GetUnitItemCount(proc->units[0]) == 0) {
            proc->cursorItemSlot = 8;
        }
        else {
            proc->cursorItemSlot = 0;
        }
    }

    proc->helpBoxItemSlot = 0xff;

    StartFace2(0, GetUnitPortraitId(proc->units[0]), 64, -4, (FACE_DISP_KIND(FACE_96x80_FLIPPED) | FACE_DISP_HLAYER(FACE_HLAYER_3)));
    StartFace2(1, GetUnitPortraitId(proc->units[1]), 174, -4, (FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_3)));

    DrawUiFrame2(1, 8, 14, 12, 0);
    DrawUiFrame2(15, 8, 14, 12, 0);

    BG_EnableSyncByMask(7);

    str = GetStringFromIndex(proc->units[0]->pCharacterData->nameTextId);
    PutDrawText(0, gBG0TilemapBuffer, 0, ((48 - GetStringTextLen(str)) / 2), 6, str);

    str = GetStringFromIndex(proc->units[1]->pCharacterData->nameTextId);
    PutDrawText(0, gBG0TilemapBuffer + 0x18, 0, ((48 - GetStringTextLen(str)) / 2), 6, str);

    DrawPrepScreenItems(gBG0TilemapBuffer + 0x122, gPrepItemTexts + 15, proc->units[0], 0);
    DrawPrepScreenItems(gBG0TilemapBuffer + 0x130, gPrepItemTexts + 20, proc->units[1], 0);

    StartUiCursorHand(proc);

    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ShowSysHandCursor(
        (proc->cursorItemSlot >> 3) * 0x70 + 0x10,
        (proc->cursorItemSlot & 7) * 0x10 + 0x48,
        0xb,
        0x800
    );

    StartHelpPromptSprite(200, 0x93, 2, proc);

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);

    EnableSysBrownBox(0, -0x28, -1, 1);
    EnableSysBrownBox(1, 0xb8, -1, 0);

    SetBlendConfig(1, 0xe, 4, 0);

    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    return;
}

//! FE8U = 0x0809D914
LYN_REPLACE_CHECK(sub_809D914);
void sub_809D914(struct PrepItemSupplyProc* proc)
{

    gLCDControlBuffer.dispcnt.mode = 0;
    SetupBackgrounds(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    gLCDControlBuffer.bg0cnt.priority = 1;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 0x28);

#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06012000, -1);
#else
    LoadHelpBoxGfx((void*)0x06013000, -1);
#endif

    LoadIconPalettes(4);
    RestartMuralBackground();
    BG_EnableSyncByMask(8);

    gLCDControlBuffer.dispcnt.bg0_on = 0;
    gLCDControlBuffer.dispcnt.bg1_on = 0;
    gLCDControlBuffer.dispcnt.bg2_on = 0;
    gLCDControlBuffer.dispcnt.bg3_on = 0;
    gLCDControlBuffer.dispcnt.obj_on = 0;

    return;
}

//! FE8U = 0x0809ED8C
LYN_REPLACE_CHECK(PrepItemList_InitGfx);
void PrepItemList_InitGfx(struct PrepItemListProc* proc)
{
    int i;
    const char* str;

    PrepItemSuppyText PrepItemSuppyTexts;

    gLCDControlBuffer.dispcnt.mode = 0;

    SetupBackgrounds(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

    gLCDControlBuffer.bg0cnt.priority = 1;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, proc->yOffsetPerPage[proc->currentPage] - 40);

#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06013000, -1);
#else
    LoadHelpBoxGfx((void*)0x06012000, -1);
#endif

    LoadIconPalettes(4);

    RestartMuralBackground();

    PutImg_PrepItemUseUnk(0x5000, 5);

    Decompress(gUnknown_08A1B9EC, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    BG_EnableSyncByMask(7);

    StartUiCursorHand(proc);

    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    gLCDControlBuffer.dispcnt.win0_on = 1;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;

    gLCDControlBuffer.win0_left = 128;
    gLCDControlBuffer.win0_top = 40;
    gLCDControlBuffer.win0_right = 224;
    gLCDControlBuffer.win0_bottom = 152;

    gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg2 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg3 = 1;
    gLCDControlBuffer.wincnt.win0_enableObj = 1;

    gLCDControlBuffer.wincnt.wout_enableBg0 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg1 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg2 = 0;
    gLCDControlBuffer.wincnt.wout_enableBg3 = 1;
    gLCDControlBuffer.wincnt.wout_enableObj = 1;

    StartGreenText(proc);

    StartHelpPromptSprite(195, 147, 9, proc);

    InitText(PrepItemSuppyTexts.th + 0, 6);
    InitText(PrepItemSuppyTexts.th + 1, 5);

    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < UNIT_ITEM_COUNT; i++) {
        InitText(PrepItemSuppyTexts.th + 2 + i, 7);
    }

    for (i = 0; i < 8; i++) {
        InitTextDb(PrepItemSuppyTexts.th + 7 + i, 7);
    }

    StoreConvoyWeaponIconGraphics(0x4000, 6);

    sub_809D8D4(gBG0TilemapBuffer + 0x6F, 0x4000, 6);

    Decompress(gUnknown_08A19CCC, (void*)0x06015000);
    ApplyPalette(Pal_SpinningArrow, 0x14);

    StartMenuScrollBarExt(proc, 225, 47, 0x5800, 9);
    sub_8097668();
    SomethingPrepListRelated(proc->unit, proc->currentPage, 3);
    sub_809F5F4(proc);

    sub_809D300(
        PrepItemSuppyTexts.th + 7,
        gBG2TilemapBuffer + 0xF,
        (proc->yOffsetPerPage[proc->currentPage]) >> 4,
        proc->unit
    );

    BG_EnableSyncByMask(4);

    DrawPrepScreenItems(gBG0TilemapBuffer + 0x6F + 0xb3, PrepItemSuppyTexts.th + 2, proc->unit, 0);
    sub_809EBF0();

    StartUiSpinningArrows(proc);
    LoadUiSpinningArrowGfx(0, 0x280, 2);
    SetUiSpinningArrowPositions(0x78, 0x18, 0xe9, 0x18);
    SetUiSpinningArrowConfig(3);

    StartParallelWorker(List_PutHighlightedCategorySprites, proc);

    StartFace2(0, GetUnitPortraitId(proc->unit), 64, -4, FACE_DISP_KIND(FACE_96x80_FLIPPED) | FACE_DISP_HLAYER(FACE_HLAYER_3));

    str = GetStringFromIndex(proc->unit->pCharacterData->nameTextId);

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);

    EnableSysBrownBox(0, -40, -1, 1);
    EnableSysBrownBox(1, 0x98, 6, 2);

    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    PutDrawText(PrepItemSuppyTexts.th, gBG0TilemapBuffer, 0, GetStringTextCenteredPos(48, str), 0, str);

    PrepItemList_DrawCurrentOwnerText(proc);

    return;
}

LYN_REPLACE_CHECK(TradeMenu_InitUnitNameDisplay);
void TradeMenu_InitUnitNameDisplay(struct TradeMenuProc* proc)
{
    char* str;
    int xStart;

    // TODO: constants
#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    StartSysBrownBox(6, 0x7080, 0x08, 0x800, 0x400, (struct Proc*)(proc));
#else
    StartSysBrownBox(6, 0x4800, 0x08, 0x800, 0x400, (struct Proc*)(proc));
#endif

    EnableSysBrownBox(0, -40, -1, 1);
    EnableSysBrownBox(1, 184, -1, 0);

    // TODO: special effect constants
    SetBlendConfig(1, 12, 6, 0);

    // TODO: name functions
    SetBlendTargetA(FALSE, FALSE, FALSE, FALSE, FALSE);
    SetBlendTargetB(TRUE, TRUE, TRUE, TRUE, TRUE);

    // TODO: text color constants

    str = GetStringFromIndex(UNIT_NAME_ID(proc->units[0]));
    xStart = ((8 * UNIT_PANEL_WIDTH) - GetStringTextLen(str)) / 2;

    PutDrawText(NULL, gBG0TilemapBuffer + TILEMAP_INDEX(0, 0), 0, xStart, UNIT_PANEL_WIDTH, str);

    str = GetStringFromIndex(UNIT_NAME_ID(proc->units[1]));
    xStart = ((8 * UNIT_PANEL_WIDTH) - GetStringTextLen(str)) / 2;

    PutDrawText(NULL, gBG0TilemapBuffer + TILEMAP_INDEX(24, 0), 0, xStart, UNIT_PANEL_WIDTH, str);

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

//! FE8U = 0x08032728
LYN_REPLACE_CHECK(KillUnitOnCombatDeath);
void KillUnitOnCombatDeath(struct Unit* unitA, struct Unit* unitB) {
    if (GetUnitCurrentHp(unitA) != 0) {
        return;
    }

#if (defined(SID_Capture) && (COMMON_SKILL_VALID(SID_Capture)))
    if (SkillTester(unitB, SID_Capture) && CheckBitUES(unitB, UES_BIT_CAPTURE_SKILL_USED))
    {
        UnitRescue(unitB, unitA);
        HideUnitSprite(unitA);
        ClearBitUES(unitB, UES_BIT_CAPTURE_SKILL_USED);
        return;
    }
#endif

/* Can still be viewed in the stat screen, but eh it's fine. */
#if defined(SID_Casual) && (COMMON_SKILL_VALID(SID_Casual))
    if (SkillTester(unitA, SID_Casual))
    {
        unitA->state |= US_HIDDEN;
        return;
    }
#endif

#if defined(SID_DestinyBond) && (COMMON_SKILL_VALID(SID_DestinyBond))
    if (SkillTester(unitA, SID_DestinyBond))
        UnitKill(unitB);
#endif

#ifdef CONFIG_SEND_INVENTORY_ON_DEATH
    if (UNIT_FACTION(unitA) == FACTION_BLUE)
    {
        for (int i = 0; i < 5; i++)
        {
            if (unitA->items[i] == 0)
                break;

            if (GetConvoyItemCount() < sExpaConvoyItemAmount)
                AddItemToConvoy(unitA->items[i]);
        }
    }
#endif

    PidStatsRecordDefeatInfo(unitA->pCharacterData->number, unitB->pCharacterData->number, DEFEAT_CAUSE_COMBAT);

    UnitKill(unitA);

    return;
}

//! FE8U = 0x08032750
LYN_REPLACE_CHECK(KillUnitOnArenaDeathMaybe);
void KillUnitOnArenaDeathMaybe(struct Unit* unit) {
    if (GetUnitCurrentHp(unit) != 0) {
        return;
    }

#ifdef CONFIG_SEND_INVENTORY_ON_DEATH
    if (UNIT_FACTION(unit) == FACTION_BLUE)
    {
        for (int i = 0; i < 5; i++)
        {
            if (unit->items[i] == 0)
                break;

            if (GetConvoyItemCount() < sExpaConvoyItemAmount)
                AddItemToConvoy(unit->items[i]);
        }
    }
#endif

    UnitKill(unit);

    PidStatsRecordDefeatInfo(unit->pCharacterData->number, 0, DEFEAT_CAUSE_ARENA);

    return;
}

//! FE8U = 0x0801D084
LYN_REPLACE_CHECK(PlayerPhase_PrepareAction);
s8 PlayerPhase_PrepareAction(ProcPtr proc)
{
    s8 cameraReturn;
    int item;

    cameraReturn = EnsureCameraOntoPosition(
        proc, GetUnit(gActionData.subjectIndex)->xPos, GetUnit(gActionData.subjectIndex)->yPos);
    cameraReturn ^= 1;

#ifdef CONFIG_NO_WAIT_AFTER_TRADING
    gBmSt.taken_action = 0;
#endif

    switch (gActionData.unitActionType)
    {
    case 0:
        /**
         * If character has use some action: such as trade with somebody,
         * then he may get chance to take another action but cannot change to another.
         * If player want to control other character, the current unit may caught wait action.
         */
        if (gBmSt.taken_action != 0)
        {
            gActionData.unitActionType = UNIT_ACTION_FORCE_WAIT;
            break;
        }

        PlayerPhase_BackToMove(proc);

        return 1;

    case UNIT_ACTION_TRADED:
        gBmSt.taken_action |= BM_TAKEN_ACTION_TRADE;
        PlayerPhase_CancelAction(proc);

        return 1;

    case UNIT_ACTION_TRADED_SUPPLY:
        gBmSt.taken_action |= BM_TAKEN_ACTION_SUPPLY;
        PlayerPhase_CancelAction(proc);

        return 1;

    case UNIT_ACTION_TAKE:
    case UNIT_ACTION_GIVE:
        gBmSt.taken_action |= BM_TAKEN_ACTION_TAKE;
        PlayerPhase_CancelAction(proc);

        return 1;

    case UNIT_ACTION_RIDE_BALLISTA:
    case UNIT_ACTION_EXIT_BALLISTA:
        gBmSt.taken_action |= BM_TAKEN_ACTION_BALLISTA;
        PlayerPhase_CancelAction(proc);

        return 1;

    case UNIT_ACTION_TRADED_1D:
        PlayerPhase_CancelAction(proc);

        return 1;
    }

    item = GetItemIndex(GetUnit(gActionData.subjectIndex)->items[gActionData.itemSlotIndex]);

    gBattleActor.hasItemEffectTarget = 0;

    switch (item)
    {
    case ITEM_HEAVENSEAL:
    case ITEM_HEROCREST:
    case ITEM_KNIGHTCREST:
    case ITEM_ORIONSBOLT:
    case ITEM_ELYSIANWHIP:
    case ITEM_GUIDINGRING:
    case ITEM_MASTERSEAL:
    case ITEM_OCEANSEAL:
    case ITEM_LUNARBRACE:
    case ITEM_SOLARBRACE:
    case ITEM_UNK_C1:
        return cameraReturn;
    }

    if ((gActionData.unitActionType != UNIT_ACTION_WAIT) && !gBmSt.just_resumed)
    {
        gActionData.suspendPointType = SUSPEND_POINT_DURINGACTION;
        WriteSuspendSave(SAVE_ID_SUSPEND);
    }

    return cameraReturn;
}

#ifdef CONFIG_QUALITY_OF_LIFE_EPILOGUE_FADE

//! FE8U = 0x080B723C
LYN_REPLACE_CHECK(PairedEndingBattleDisp_InitBlend);
void PairedEndingBattleDisp_InitBlend(struct EndingBattleDisplayProc* proc)
{
    proc->timer = 0;

    SetBlendAlpha(0x10, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 1, 0, 0);

    return;
}

//! FE8U = 0x080B7274
LYN_REPLACE_CHECK(PairedEndingBattleDisp_Loop_Blend);
void PairedEndingBattleDisp_Loop_Blend(struct EndingBattleDisplayProc* proc)
{
    int bldAmt = proc->timer >> 2;

    proc->timer++;

    SetBlendAlpha(0x10 - bldAmt, bldAmt);

    if (bldAmt == 8)
    {
        Proc_Break(proc);
    }

    return;
}

static const struct ProcCmd gProcScr_EndingBattleDisplay_Solo_NEW[] =
{
    PROC_YIELD,

    PROC_CALL(SoloEndingBattleDisp_Init),
    PROC_REPEAT(SoloEndingBattleDisp_Loop),

    PROC_SLEEP(16),

    PROC_CALL(PairedEndingBattleDisp_InitBlend),
    PROC_REPEAT(PairedEndingBattleDisp_Loop_Blend),

    PROC_END,
};

LYN_REPLACE_CHECK(StartSoloEndingBattleDisplay);
void StartSoloEndingBattleDisplay(struct CharacterEndingEnt* endingEnt, struct Unit* unit, struct CharacterEndingProc* parent)
{
    struct EndingBattleDisplayProc* proc = Proc_StartBlocking(gProcScr_EndingBattleDisplay_Solo_NEW, parent);

    proc->units[0] = unit;
    proc->units[1] = NULL;

    proc->pCharacterEnding = endingEnt;

    return;
}

#endif

#ifdef CONFIG_QUALITY_OF_LIFE_UNIT_NAME_DROP

struct PopupInstruction const NewItemDropPopup[] = {
    POPUP_SOUND(0x5A),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_UNIT_NAME,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_GOT_ITEM),             /* got */
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

struct PopupInstruction const NewItemPilferedPopup[] = {
    POPUP_SOUND(0x5A),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_UNIT_NAME,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_PILFERED_ITEM),       /* pilfered */
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

struct PopupInstruction const NewItemStolePopup[] = {
    POPUP_SOUND(0x5C),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_UNIT_NAME,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_STOLE_ITEM),          /* Stole */
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_SPACE(1),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

struct PopupInstruction const NewGoldGotPopup[] = {
    POPUP_SOUND(0x5A),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_UNIT_NAME,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_GOT_ITEM),               /* got */
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_NUM,
    POPUP_SPACE(3),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_GOLD),                   /* gold.[.] */
    POPUP_END
};

struct PopupInstruction const NewGoldStolenPopup[] = {
    POPUP_SOUND(0x5C),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_UNIT_NAME,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_STOLE_ITEM),              /* Stole */
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_NUM,
    POPUP_SPACE(3),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_GOLD),                   /* gold. */
    POPUP_END
};

LYN_REPLACE_CHECK(NewPopup_ItemGot_unused);
void NewPopup_ItemGot_unused(struct Unit* unit, u16 item, ProcPtr parent)
{
    SetPopupItem(item);
    SetPopupUnit(unit);

    if (FACTION_BLUE == UNIT_FACTION(unit))
        NewPopup_Simple(NewItemDropPopup, 0x60, 0x0, parent);
    else
        NewPopup_Simple(NewItemPilferedPopup, 0x60, 0x0, parent);
}

LYN_REPLACE_CHECK(NewPopup_GoldGot);
void NewPopup_GoldGot(ProcPtr parent, struct Unit* unit, int value)
{
    SetPopupNumber(value);
    SetPopupUnit(unit);

    if (FACTION_BLUE == UNIT_FACTION(unit))
    {
        value += GetPartyGoldAmount();
        SetPartyGoldAmount(value);
        NewPopup_Simple(NewGoldGotPopup, 0x60, 0x0, parent);
    }
    else
        NewPopup_Simple(NewGoldStolenPopup, 0x60, 0x0, parent);
}

LYN_REPLACE_CHECK(NewPopup_ItemStealing);
void NewPopup_ItemStealing(u16 item, ProcPtr parent)
{
    SetPopupItem(item);
    SetPopupUnit(gActiveUnit);

    NewPopup_Simple(NewItemStolePopup, 0x60, 0x0, parent);
}

#endif

LYN_REPLACE_CHECK(UnitDrop);
void UnitDrop(struct Unit* actor, int xTarget, int yTarget)
{
    struct Unit* target = GetUnit(actor->rescue);

    actor->state = actor->state & ~(US_RESCUING | US_RESCUED);
    target->state = target->state & ~(US_RESCUING | US_RESCUED | US_HIDDEN);

    /* Let rescued units move after the rescuer dies */
#ifdef CONFIG_DEATH_DANCE
    if (UNIT_FACTION(target) == gPlaySt.faction && actor->curHP != 0)
        target->state |= US_UNSELECTABLE; // TODO: US_GRAYED    
#else
    if (UNIT_FACTION(target) == gPlaySt.faction)
        target->state |= US_UNSELECTABLE; // TODO: US_GRAYED
#endif

    actor->rescue = 0;
    target->rescue = 0;

    target->xPos = xTarget;
    target->yPos = yTarget;

    /* If we've captured an enemy, dropping them will kill them immediately */
    if (target->curHP == 0)
    {
        PidStatsRecordDefeatInfo(target->pCharacterData->number, actor->pCharacterData->number, DEFEAT_CAUSE_COMBAT);
        UnitKill(target);
    }
}

LYN_REPLACE_CHECK(TradeMenu_InitItemDisplay);
void TradeMenu_InitItemDisplay(struct TradeMenuProc* proc)
{
    DrawUiFrame2(1, 8, 14, 12, 0);
    DrawUiFrame2(15, 8, 14, 12, 0);

    ResetTextFont();

    ResetIconGraphics();
    LoadIconPalettes(4); // TODO: palette id constant

    TradeMenu_InitItemText(proc);
    TradeMenu_RefreshItemText(proc);

#ifdef CONFIG_QUALITY_OF_LIFE_AI_TRADE_FIX
    bool noPortraitUnit_1 = false;
    bool noPortraitUnit_2 = false;

    if (proc->units[0]->pCharacterData->portraitId == 0)
        noPortraitUnit_1 = true;

    if (proc->units[1]->pCharacterData->portraitId == 0)
        noPortraitUnit_2 = true;

    if (!noPortraitUnit_1)
        StartFace(0, GetUnitPortraitId(proc->units[0]), 64, -4, 3);
    if (!noPortraitUnit_2)
        StartFace(1, GetUnitPortraitId(proc->units[1]), 176, -4, 2);

#else
    // TODO: face display type (arg 5) constants
    StartFace(0, GetUnitPortraitId(proc->units[0]), 64, -4, 3);
    StartFace(1, GetUnitPortraitId(proc->units[1]), 176, -4, 2);
#endif

    SetFaceBlinkControlById(0, 5);
    SetFaceBlinkControlById(1, 5);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

LYN_REPLACE_CHECK(StoreNumberStringOrDashesToSmallBuffer);
void StoreNumberStringOrDashesToSmallBuffer(int n)
{
    ClearSmallStringBuffer();
#ifdef CONFIG_UNLOCK_ALLY_MHP_LIMIT
    StoreNumberStringToSmallBuffer(n);
#else
    if (n == 255 || n == -1)
    {
        gNumberStr[7] = ':';
        gNumberStr[6] = ':';
    }
    else
    {
        StoreNumberStringToSmallBuffer(n);
    }
#endif
}

LYN_REPLACE_CHECK(GetClassData);
const struct ClassData* GetClassData(int classId) {
    if (classId < 1)
        return NULL;

    return gClassData_NEW + (classId - 1);
}

LYN_REPLACE_CHECK(RefreshUnitStealInventoryInfoWindow);
void RefreshUnitStealInventoryInfoWindow(struct Unit* unit)
{
    int i;
    int itemCount;
    int xPos;
    struct UnitInfoWindowProc* proc;

    itemCount = GetUnitItemCount(unit);

    xPos = GetUnitInfoWindowX(unit, 0xd);

    proc = UnitInfoWindow_DrawBase(0, unit, xPos, 0, 0xd, itemCount);

    bool stealPlus = false;

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
    if (SkillTester(gActiveUnit, SID_StealPlus))
        stealPlus = true;
#endif

    for (i = 0; i < itemCount; i++)
    {
        int yPos = 0 + i * 2 + 3;

        int item = unit->items[i];
        s8 stealable = IsItemStealable(item);

        // Skill holder cannot steal a unit's equipped weapon
        if (GetUnitEquippedWeaponSlot(GetUnit(gActionData.targetIndex)) == i && stealPlus)
            stealable = false;

        ClearText(proc->lines + i);

        Text_SetColor(proc->lines + i, stealable ? 0 : 1);
        Text_DrawString(proc->lines + i, GetItemName(item));

        PutText(proc->lines + i, gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 3, yPos));

        PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? 2 : 1, GetItemUses(item));

#ifdef CONFIG_FORGING
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

        if (CanItemBeForged(item)) {
            PutSpecialChar(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 8, yPos), stealable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 9, yPos), stealable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, GetItemForgeCount(item));
            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetForgedItemDurability(item));
        }
        else if (limits.maxCount == 0) {
            PutNumberOrBlank(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 11, yPos), stealable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }
#endif

        DrawIcon(gBG0TilemapBuffer + TILEMAP_INDEX(xPos + 1, yPos), GetItemIconId(item), 0x4000);
    }

    return;
}

//! FE8U = 0x0809D300 (DisplayConvoyItemStrings?)
LYN_REPLACE_CHECK(sub_809D300);
void sub_809D300(struct Text* textBase, u16* tm, int yLines, struct Unit* unit)
{
    int i;

    TileMap_FillRect(tm, 12, 31, 0);

    if (gUnknown_02012F56 == 0) {
        ClearText(textBase);
        Text_InsertDrawString(textBase, 0, 1, GetStringFromIndex(0x5a8)); // TODO: msgid "Nothing[.]"
        PutText(textBase, tm + 3);
        return;
    }

    for (i = yLines; (i < yLines + 7) && (i < gUnknown_02012F56); i++) {
        struct Text* th = textBase + (i & 7);
        int item = gPrepScreenItemList[i].item;
        int unusable = !IsItemDisplayUsable(unit, item);

        ClearText(th);

        Text_InsertDrawString(
            th,
            0,
            unusable,
            GetItemName(item)
        );

        DrawIcon(tm + TILEMAP_INDEX(1, i * 2 & 0x1f), GetItemIconId(item), 0x4000);

        PutText(th, tm + TILEMAP_INDEX(3, i * 2 & 0x1f));

#ifndef CONFIG_INFINITE_DURABILITY
        PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1, GetItemUses(item));
#endif

#ifdef CONFIG_FORGING
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutSpecialChar(tm + TILEMAP_INDEX(8, i * 2 & 0x1f),
                !unusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(tm + TILEMAP_INDEX(9, i * 2 & 0x1f),
                !unusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemForgeCount(item));
            PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1,
                GetForgedItemDurability(item));
        }
        else if (limits.maxCount == 0) {
            PutNumberOrBlank(tm + TILEMAP_INDEX(12, i * 2 & 0x1f), !unusable ? 2 : 1,
                GetItemUses(item));
        }
#endif
    }

    return;
}

//! FE8U = 0x0809D47C
LYN_REPLACE_CHECK(sub_809D47C);
void sub_809D47C(struct Text* textBase, u16* tm, int yLines, struct Unit* unit)
{
    if (gUnknown_02012F56 > yLines) {
        int y = (yLines * 2) & 0x1f;
        struct Text* th = textBase + (yLines & 7);
        int item = gPrepScreenItemList[yLines].item;
        int unusable = !IsItemDisplayUsable(unit, item);

        int offset = TILEMAP_INDEX(0, y);
        TileMap_FillRect(tm + offset, 12, 1, 0);

        ClearText(th);
        Text_InsertDrawString(th, 0, unusable, GetItemName(item));
        DrawIcon(tm + offset + 1, GetItemIconId(item), 0x4000);
        PutText(th, tm + offset + 3);

#ifndef CONFIG_INFINITE_DURABILITY
        PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
#endif

#ifdef CONFIG_FORGING
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutSpecialChar(tm + offset + 8,
                !unusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(tm + offset + 9,
                !unusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemForgeCount(item));
            PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY,
                GetForgedItemDurability(item));
        }
        else if (limits.maxCount == 0) {
            PutNumberOrBlank(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
        }
#endif
    }
}

//! FE8U = 0x08099F7C
LYN_REPLACE_CHECK(sub_8099F7C);
void sub_8099F7C(struct Text* th, u16* tm, struct Unit* unit, u16 flags) {
    int itemCount;
    int i;

    TileMap_FillRect(tm, 12, 20, 0);

    if ((flags & 2) != 0) {
        ResetIconGraphics();
    }

    if (!unit) {
        return;
    }

    itemCount = GetUnitItemCount(unit);

    for (i = 0; i < itemCount; th++, i++) {
        u16 item = unit->items[i];

        int isUnusable = ((flags & 4) != 0)
            ? !CanUnitUseItemPrepScreen(unit, item)
            : !IsItemDisplayUsable(unit, item);

        if ((flags & 1) == 0) {
            ClearText(th);
            Text_SetColor(th, isUnusable);
            Text_SetCursor(th, 0);
            Text_DrawString(th, GetItemName(item));
        }

        DrawIcon(tm + i * 0x40, GetItemIconId(item), 0x4000);

        PutText(th, tm + 2 + i * 0x40);

#ifdef CONFIG_FORGING
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutSpecialChar(tm + 8 + i * 0x40,
                !isUnusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(tm + 9 + i * 0x40,
                !isUnusable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemForgeCount(item));
            PutNumberOrBlank(tm + 11 + i * 0x40,
                !isUnusable ? TEXT_COLOR_SYSTEM_BLUE
                : TEXT_COLOR_SYSTEM_GRAY,
                GetForgedItemDurability(item));
        }
        else if (limits.maxCount == 0) {
            PutNumberOrBlank(tm + 11 + i * 0x40,
                !isUnusable ? TEXT_COLOR_SYSTEM_BLUE
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemUses(item));
        }
#endif

#ifndef CONFIG_INFINITE_DURABILITY
        PutNumberOrBlank(tm + 11 + i * 0x40, !isUnusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
#endif
    }

    return;
}

//! FE8U = 0x0809B74C
LYN_REPLACE_CHECK(DrawPrepScreenItems);
void DrawPrepScreenItems(u16* tm, struct Text* th, struct Unit* unit, u8 checkPrepUsability) {
    s8 isUsable;
    int i;
    int itemCount;

    TileMap_FillRect(tm, 11, 9, 0);

    itemCount = GetUnitItemCount(unit);

    for (i = 0; i < itemCount; i++) {
        int item = unit->items[i];

        if (checkPrepUsability != 0) {
            isUsable = CanUnitUseItemPrepScreen(unit, item);
        }
        else {
            isUsable = IsItemDisplayUsable(unit, item);
        }

        ClearText(th);
        PutDrawText(
            th,
            tm + i * 0x40 + 2,
            !isUsable ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
            0,
            0,
            GetItemName(item)
        );

#ifndef CONFIG_INFINITE_DURABILITY
        PutNumberOrBlank(tm + i * 0x40 + 0xB, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
#endif

#ifdef CONFIG_FORGING
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        if (CanItemBeForged(item)) {
            PutSpecialChar(tm + i * 0x40 + 8,
                isUsable ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY,
                TEXT_SPECIAL_PLUS);
            PutNumberOrBlank(tm + i * 0x40 + 9,
                isUsable ? TEXT_COLOR_SYSTEM_GOLD
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemForgeCount(item));
            PutNumberOrBlank(tm + i * 0x40 + 0xB,
                isUsable ? TEXT_COLOR_SYSTEM_BLUE
                : TEXT_COLOR_SYSTEM_GRAY,
                GetForgedItemDurability(item));
        }
        else if (limits.maxCount == 0) {
            PutNumberOrBlank(tm + i * 0x40 + 0xB,
                isUsable ? TEXT_COLOR_SYSTEM_BLUE
                : TEXT_COLOR_SYSTEM_GRAY,
                GetItemUses(item));
        }
#endif

        DrawIcon(tm + i * 0x40, GetItemIconId(item), 0x4000);

        th++;
    }

    return;
}

LYN_REPLACE_CHECK(ProcessMenuDpadInput);
void ProcessMenuDpadInput(struct MenuProc* proc)
{
    proc->itemPrevious = proc->itemCurrent;

    // Handle Up keyin

    if (gKeyStatusPtr->repeatedKeys & DPAD_UP)
    {
        if (proc->itemCurrent == 0)
        {
            if (gKeyStatusPtr->repeatedKeys != gKeyStatusPtr->newKeys)
                return;

            proc->itemCurrent = proc->itemCount;
        }

        proc->itemCurrent--;

        // Reset the last item forge count if we move to another item
#ifdef CONFIG_FORGING
        if (gActionData.unk08 == 10000) // Arbitrary value we set to indicate the forge menu is active
        {
            int item = gActiveUnit->items[proc->itemPrevious];
            gEventSlots[EVT_SLOT_7] = GetItemForgeCost(gActiveUnit->items[proc->itemCurrent]);
            SetItemForgeCount(item, gEventSlots[EVT_SLOT_8]);
            // Set the initial count of the latest item
            gEventSlots[EVT_SLOT_8] = GetItemForgeCount(gActiveUnit->items[proc->itemCurrent]);
        }
#endif

        /* A little something to change the monster image displayed as the user scrolls the summon select screen */
#if defined(SID_SummonPlus) && (COMMON_SKILL_VALID(SID_SummonPlus))
        if (gActionData.unk08 == SID_SummonPlus)
        {
            if (UNIT_CATTRIBUTES(gActiveUnit) & CA_PROMOTED)
                PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetClassData(classPromotedIndexes_SP[proc->itemCurrent])->defaultPortraitId, 0x200, 5);
            else
                PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetClassData(classIndexes_SP[proc->itemCurrent])->defaultPortraitId, 0x200, 5);
        }
#endif
    }

    // Handle down keyin

    if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN)
    {
        if (proc->itemCurrent == (proc->itemCount - 1))
        {
            if (gKeyStatusPtr->repeatedKeys != gKeyStatusPtr->newKeys)
                return;

            proc->itemCurrent = -1;
        }

        proc->itemCurrent++;

        // Reset the last item forge count if we move to another item
#ifdef CONFIG_FORGING
        if (gActionData.unk08 == 10000) // Arbitrary value we set to indicate the forge menu is active
        {
            int item = gActiveUnit->items[proc->itemPrevious];
            gEventSlots[EVT_SLOT_7] = GetItemForgeCost(gActiveUnit->items[proc->itemCurrent]);
            SetItemForgeCount(item, gEventSlots[EVT_SLOT_8]);
            // Set the initial count of the latest item
            gEventSlots[EVT_SLOT_8] = GetItemForgeCount(gActiveUnit->items[proc->itemCurrent]);
        }
#endif

        /* A little something to change the monster image displayed as the user scrolls the summon select screen */
#if defined(SID_SummonPlus) && (COMMON_SKILL_VALID(SID_SummonPlus))
        if (gActionData.unk08 == SID_SummonPlus)
        {
            if (UNIT_CATTRIBUTES(gActiveUnit) & CA_PROMOTED)
                PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetClassData(classPromotedIndexes_SP[proc->itemCurrent])->defaultPortraitId, 0x200, 5);
            else
                PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetClassData(classIndexes_SP[proc->itemCurrent])->defaultPortraitId, 0x200, 5);
        }
#endif
    }

#ifdef CONFIG_FORGING
    if (gActionData.unk08 == 10000) // Arbitrary value we set to indicate the forge menu is active
    {
        // Handle left/right input for forge menu
        int item = gActiveUnit->items[proc->itemCurrent];
        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];
        int count = GetItemForgeCount(item);

        if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT)
        {
            // Ensure the forge count can't go below what it started at (which is stored in EVT_SLOT_8)
            if (count > 0 && (u32)(count - 1) >= gEventSlots[EVT_SLOT_8])
            {
                int forgeSlot = ITEM_USES(item);
                if (forgeSlot >= 0)
                {
                    // Calculate cumulative cost increase  
                    u32 costAmount = (count + 1) * limits.baseCost;

                    item = GetItemIndex(item) | (forgeSlot << 8);
                    // Calculate cumulative cost reduction
                    u32 refundAmount = 0;
                    for (int i = count; i > count - 1; i--) // This loop refunds 'count * limits.baseCost'
                        refundAmount += i * limits.baseCost;

                    gActiveUnit->items[proc->itemCurrent] = DecrementForgeCount(item, 1);
                    gPlaySt.partyGoldAmount += refundAmount;
                    gEventSlots[EVT_SLOT_7] -= costAmount;

                    struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

                    if (GetItemForgeCount(gActiveUnit->items[proc->itemCurrent]) < limits.maxCount - 1)
                        MakeForgedItemUnbreakable(item, false);

                    // Refresh the menu display
                    if (proc->menuItems[proc->itemCurrent]->def->onSwitchIn)
                        proc->menuItems[proc->itemCurrent]->def->onSwitchIn(proc, proc->menuItems[proc->itemCurrent]);
                }
            }
        }

        if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT)
        {
            if (count < limits.maxCount - 1 && IsItemForgeable(item))
            {
                int forgeSlot = ITEM_USES(item);
                if (!forgeSlot)
                    forgeSlot = InitFreeForgedItemSlot(item);
                if (forgeSlot >= 0)
                {
                    // Calculate cumulative cost increase  
                    u32 costAmount = (count + 2) * limits.baseCost;

                    if (costAmount <= gPlaySt.partyGoldAmount)
                    {
                        item = GetItemIndex(item) | (forgeSlot << 8);
                        gPlaySt.partyGoldAmount -= costAmount;
                        gActiveUnit->items[proc->itemCurrent] = IncrementForgeCount(item, 1);
                        gEventSlots[EVT_SLOT_7] += costAmount;

                        struct ForgeLimits limits = gForgeLimits[GetItemIndex(item)];

                        if (GetItemForgeCount(gActiveUnit->items[proc->itemCurrent]) == limits.maxCount - 1)
                            MakeForgedItemUnbreakable(item, true);

                        // Refresh the menu display
                        if (proc->menuItems[proc->itemCurrent]->def->onSwitchIn)
                            proc->menuItems[proc->itemCurrent]->def->onSwitchIn(proc, proc->menuItems[proc->itemCurrent]);
                    }
                }
            }
        }
    }
#endif

    if (proc->itemPrevious != proc->itemCurrent)
    {
        DrawMenuItemHover(proc, proc->itemPrevious, FALSE);
        DrawMenuItemHover(proc, proc->itemCurrent, TRUE);

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    }

    // Call def's switch in/out funcs

    if (HasMenuChangedItem(proc))
    {
        if (proc->menuItems[proc->itemPrevious]->def->onSwitchOut)
            proc->menuItems[proc->itemPrevious]->def->onSwitchOut(proc, proc->menuItems[proc->itemPrevious]);

        if (proc->menuItems[proc->itemCurrent]->def->onSwitchIn)
            proc->menuItems[proc->itemCurrent]->def->onSwitchIn(proc, proc->menuItems[proc->itemCurrent]);
    }
}

/*
** This seems to cause a crash when selecting R text on the action menu
** so commenting it out for now and moving the additional custom stuff to UnitActionMenu_CancelRe
*/
// LYN_REPLACE_CHECK(MenuCancelSelect);
// u8 MenuCancelSelect(struct MenuProc* menu, struct MenuItemProc* item)
// {
//     return MENU_ACT_SKIPCURSOR | MENU_ACT_CLEAR | MENU_ACT_END | MENU_ACT_SND6B;
// }

struct SoundRoomEnt const gSoundRoomTable_NEW[] =
{
    {
        .bgmId = SONG_THE_VALIANT,
        .songLength = 2640,
        .displayCondFunc = NULL,
        .nameTextId = 0x78B,
    },
    {
        .bgmId = SONG_MAIN_THEME_EXT,
        .songLength = 5880,
        .displayCondFunc = NULL,
        .nameTextId = 0x78C,
    },
    {
        .bgmId = SONG_PROLOGUE,
        .songLength = 4140,
        .displayCondFunc = NULL,
        .nameTextId = 0x78D,
    },
    {
        .bgmId = SONG_THE_BEGINNING,
        .songLength = 3720,
        .displayCondFunc = NULL,
        .nameTextId = 0x78E,
    },
    {
        .bgmId = SONG_TREASURED_MEMORIES,
        .songLength = 4560,
        .displayCondFunc = NULL,
        .nameTextId = 0x78F,
    },
    {
        .bgmId = SONG_GRIM_JOURNEY,
        .songLength = 4440,
        .displayCondFunc = NULL,
        .nameTextId = 0x790,
    },
    {
        .bgmId = SONG_TIES_OF_FRIENDSHIP,
        .songLength = 3840,
        .displayCondFunc = NULL,
        .nameTextId = 0x791,
    },
    {
        .bgmId = SONG_RAY_OF_HOPE,
        .songLength = 3900,
        .displayCondFunc = NULL,
        .nameTextId = 0x792,
    },
    {
        .bgmId = SONG_DISTANT_ROADS,
        .songLength = 6420,
        .displayCondFunc = NULL,
        .nameTextId = 0x793,
    },
    {
        .bgmId = SONG_RISE_ABOVE,
        .songLength = 5640,
        .displayCondFunc = NULL,
        .nameTextId = 0x794,
    },
    {
        .bgmId = SONG_FOLLOW_ME,
        .songLength = 4980,
        .displayCondFunc = NULL,
        .nameTextId = 0x795,
    },
    {
        .bgmId = SONG_DETERMINATION,
        .songLength = 4560,
        .displayCondFunc = NULL,
        .nameTextId = 0x796,
    },
    {
        .bgmId = SONG_TRUTH_DESPAIR_AND_HOPE,
        .songLength = 5820,
        .displayCondFunc = NULL,
        .nameTextId = 0x797,
    },
    {
        .bgmId = SONG_LAND_OF_PROMISE,
        .songLength = 4680,
        .displayCondFunc = NULL,
        .nameTextId = 0x798,
    },
    {
        .bgmId = SONG_BINDING_VOW,
        .songLength = 3840,
        .displayCondFunc = NULL,
        .nameTextId = 0x799,
    },
    {
        .bgmId = SONG_GRASP_AT_VICTORY,
        .songLength = 2460,
        .displayCondFunc = NULL,
        .nameTextId = 0x79A,
    },
    {
        .bgmId = SONG_CONFRONT_THE_PAST,
        .songLength = 4980,
        .displayCondFunc = NULL,
        .nameTextId = 0x79B,
    },
    {
        .bgmId = SONG_SHADOWS_APPROACH,
        .songLength = 5640,
        .displayCondFunc = NULL,
        .nameTextId = 0x79C,
    },
    {
        .bgmId = SONG_SHADOW_OF_THE_ENEMY,
        .songLength = 2820,
        .displayCondFunc = NULL,
        .nameTextId = 0x79D,
    },
    {
        .bgmId = SONG_ASSAULT,
        .songLength = 5100,
        .displayCondFunc = NULL,
        .nameTextId = 0x79E,
    },
    {
        .bgmId = SONG_FROM_THE_DARKNESS,
        .songLength = 4380,
        .displayCondFunc = NULL,
        .nameTextId = 0x79F,
    },
    {
        .bgmId = SONG_TWISTED_SHADOWS_A,
        .songLength = 4440,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A0,
    },
    {
        .bgmId = SONG_TWISTED_SHADOWS_B,
        .songLength = 4440,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A1,
    },
    {
        .bgmId = SONG_ENVOY_FROM_THE_DARK,
        .songLength = 4980,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A2,
    },
    {
        .bgmId = SONG_ATTACK,
        .songLength = 1500,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A3,
    },
    {
        .bgmId = SONG_DEFENSE,
        .songLength = 1680,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A4,
    },
    {
        .bgmId = SONG_THE_BATTLE_MUST_BE_WON,
        .songLength = 1680,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A5,
    },
    {
        .bgmId = SONG_POWERFUL_FOE,
        .songLength = 1440,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A6,
    },
    {
        .bgmId = SONG_THE_PRINCES_DESPAIR,
        .songLength = 2040,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A7,
    },
    {
        .bgmId = SONG_RETURN_OF_THE_DEMON_KING,
        .songLength = 3840,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A8,
    },
    {
        .bgmId = SONG_SACRED_STRENGTH,
        .songLength = 660,
        .displayCondFunc = NULL,
        .nameTextId = 0x7A9,
    },
    {
        .bgmId = SONG_TETHYS,
        .songLength = 1440,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AA,
    },
    {
        .bgmId = SONG_HEALING,
        .songLength = 840,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AB,
    },
    {
        .bgmId = SONG_CURING,
        .songLength = 660,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AC,
    },
    {
        .bgmId = SONG_TO_A_HIGHER_PLACE,
        .songLength = 1800,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AD,
    },
    {
        .bgmId = SONG_ADVANCE,
        .songLength = 3180,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AE,
    },
    {
        .bgmId = SONG_TENSION,
        .songLength = 2040,
        .displayCondFunc = NULL,
        .nameTextId = 0x7AF,
    },
    {
        .bgmId = SONG_RAID,
        .songLength = 2520,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B0,
    },
    {
        .bgmId = SONG_REUNION,
        .songLength = 2700,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B1,
    },
    {
        .bgmId = SONG_INDIGNATION,
        .songLength = 2940,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B2,
    },
    {
        .bgmId = SONG_SORROW,
        .songLength = 3180,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B3,
    },
    {
        .bgmId = SONG_LAUGHTER,
        .songLength = 2160,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B4,
    },
    {
        .bgmId = SONG_LEGACY,
        .songLength = 4200,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B5,
    },
    {
        .bgmId = SONG_LYON,
        .songLength = 2880,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B6,
    },
    {
        .bgmId = SONG_LOST_HEART,
        .songLength = 3720,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B7,
    },
    {
        .bgmId = SONG_LYON_ORGAN_ARRANGEMENT,
        .songLength = 3900,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B8,
    },
    {
        .bgmId = SONG_THE_FINAL_BATTLE,
        .songLength = 2880,
        .displayCondFunc = NULL,
        .nameTextId = 0x7B9,
    },
    {
        .bgmId = SONG_SOLVE_THE_RIDDLE,
        .songLength = 3300,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BA,
    },
    {
        .bgmId = SONG_LIGHTS_IN_THE_DARK,
        .songLength = 3420,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BB,
    },
    {
        .bgmId = SONG_COMRADES,
        .songLength = 2160,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BC,
    },
    {
        .bgmId = SONG_VICTORY,
        .songLength = 3120,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BD,
    },
    {
        .bgmId = SONG_INTO_THE_SHADOW_OF_VICTORY,
        .songLength = 2220,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BE,
    },
    {
        .bgmId = SONG_MAIN_THEME_REPRISE,
        .songLength = 3960,
        .displayCondFunc = NULL,
        .nameTextId = 0x7BF,
    },
    {
        .bgmId = SONG_COMBAT_PREPARATION,
        .songLength = 3960,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C0,
    },
    {
        .bgmId = SONG_SHOPS,
        .songLength = 1260,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C1,
    },
    {
        .bgmId = SONG_ARMORIES,
        .songLength = 1080,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C2,
    },
    {
        .bgmId = SONG_BONDS,
        .songLength = 900,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C3,
    },
    {
        .bgmId = SONG_COLOSSEUM_ENTRANCE,
        .songLength = 2520,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C4,
    },
    {
        .bgmId = SONG_IN_THE_COLOSSEUM,
        .songLength = 1740,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C5,
    },
    {
        .bgmId = SONG_COLOSSEUM_VICTORY,
        .songLength = 360,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C6,
    },
    {
        .bgmId = SONG_COLOSSEUM_DEFEAT,
        .songLength = 1320,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C7,
    },
    {
        .bgmId = SONG_VICTORY_SONG_A,
        .songLength = 420,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C8,
    },
    {
        .bgmId = SONG_VICTORY_SONG_B,
        .songLength = 420,
        .displayCondFunc = NULL,
        .nameTextId = 0x7C9,
    },
    {
        .bgmId = SONG_GAME_OVER,
        .songLength = 2160,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CA,
    },
    {
        .bgmId = SONG_IN_SORROWS_SHROUD,
        .songLength = 1620,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CB,
    },
    {
        .bgmId = SONG_RECORDS,
        .songLength = 2760,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CC,
    },
    {
        .bgmId = SONG_VICTORY_AND_THE_FUTURE,
        .songLength = 6240,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CD,
    },
    {
        .bgmId = SONG_FLY_WITH_THE_BREEZE,
        .songLength = 9120,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CE,
    },
    {
        .bgmId = SONG_EPILOGUE,
        .songLength = 9360,
        .displayCondFunc = NULL,
        .nameTextId = 0x7CF,
    },
    {
        .bgmId = -1,
    },
};

//! FE8U = 0x0801538C
LYN_REPLACE_CHECK(SwitchPhases);
void SwitchPhases(void)
{
    switch (gPlaySt.faction)
    {
    case FACTION_BLUE:
        gPlaySt.faction = FACTION_RED;
        break;

    case FACTION_RED:
        gPlaySt.faction = FACTION_GREEN;
        break;

#ifdef CONFIG_FOURTH_ALLEGIANCE
    case FACTION_GREEN:
        gPlaySt.faction = FACTION_PURPLE;
        break;

    case FACTION_PURPLE:
        gPlaySt.faction = FACTION_BLUE;
        break;
#else
    case FACTION_GREEN:
        gPlaySt.faction = FACTION_BLUE;
        break;
#endif
    }

    if (gPlaySt.chapterTurnNumber < 999)
        gPlaySt.chapterTurnNumber++;

#ifndef CONFIG_VESLY_SUPPORT_POST_BATTLE
    ProcessTurnSupportExp();
#endif
}

LYN_REPLACE_CHECK(RefreshUnitsOnBmMap);
void RefreshUnitsOnBmMap(void) {
    struct Unit* unit;
    int i;

    // 1. Blue & Green units

    for (i = 1; i < FACTION_RED; ++i) {
        unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (unit->state & US_HIDDEN)
            continue;

        // Put unit on unit map
        gBmMapUnit[unit->yPos][unit->xPos] = i;

        FORCE_DECLARE int fogBoost = 0;

#if defined(SID_HazeHunter) && (COMMON_SKILL_VALID(SID_HazeHunter))
    if (SkillTester(unit, SID_HazeHunter))
        fogBoost += 5;
#endif

        // If fog is enabled, apply unit vision to fog map
        if (gPlaySt.chapterVisionRange)
            MapAddInRange(unit->xPos, unit->yPos, GetUnitFogViewRange(unit) + fogBoost, 1);
    }

    // 2. Red (& Purple) units

    if (gPlaySt.faction != FACTION_RED) {
        // 2.1. No red phase

#ifdef CONFIG_FOURTH_ALLEGIANCE
    for (i = FACTION_RED + 1; i < FACTION_PURPLE + 0x10; i++)
#else
    for (i = FACTION_RED + 1; i < FACTION_PURPLE; i++)
#endif
    {
            unit = GetUnit(i);

            if (!UNIT_IS_VALID(unit))
                continue;

            if (unit->state & US_HIDDEN)
                continue;

            // If unit is magic seal, set fog in range 0-10.
            // Magic seal set the fog map probably solely for the alternate map palette.
            if (UNIT_CATTRIBUTES(unit) & CA_MAGICSEAL)
                MapAddInRange(unit->xPos, unit->yPos, 10, -1);

#ifdef CONFIG_MULTIPLE_FOG_STAGES
            gBmMapUnit[unit->yPos][unit->xPos] = i;

            if (unit->state & US_BIT9)
                unit->state = (unit->state & ~US_BIT9) | US_BIT8;
#else
            if (gPlaySt.chapterVisionRange && !gBmMapFog[unit->yPos][unit->xPos]) {
                // If in fog, set unit bit on the hidden map, and set the "hidden in fog" state

                gBmMapHidden[unit->yPos][unit->xPos] |= HIDDEN_BIT_UNIT;
                unit->state = unit->state | US_BIT9;
            }
            else {
                // If not in fog, put unit on the map, and update state accordingly

                gBmMapUnit[unit->yPos][unit->xPos] = i;

                if (unit->state & US_BIT9)
                    unit->state = (unit->state & ~US_BIT9) | US_BIT8;
            }
#endif

        }
    }
    else {
        // 2.2. Yes red phase

        // This does mostly the same as the "No red phase" loop, except:
        // - It always puts the units on the unit map
        // - It never sets the "spotted" unit state bit (even if unit is seen)

        for (i = FACTION_RED + 1; i < FACTION_PURPLE + 6; ++i) {
            unit = GetUnit(i);

            if (!UNIT_IS_VALID(unit))
                continue;

            if (unit->state & US_HIDDEN)
                continue;

            // See above
            if (UNIT_CATTRIBUTES(unit) & CA_MAGICSEAL)
                MapAddInRange(unit->xPos, unit->yPos, 10, -1);

            if (gPlaySt.chapterVisionRange) {
                // Update unit state according to fog level

                if (!gBmMapFog[unit->yPos][unit->xPos])
                    unit->state = unit->state | US_BIT9;
                else
                    unit->state = unit->state & ~US_BIT9;
            }

            // Put on unit map
            gBmMapUnit[unit->yPos][unit->xPos] = i;
        }
    }
}

LYN_REPLACE_CHECK(TryAddUnitToTradeTargetList);
void TryAddUnitToTradeTargetList(struct Unit* unit) {

    if (!IsSameAllegiance(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (gSubjectUnit->pClassData->number == CLASS_PHANTOM || unit->pClassData->number == CLASS_PHANTOM) {
        return;
    }

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_REPLICATE)
        return;

    if (unit->statusIndex != UNIT_STATUS_BERSERK) {

        if (gSubjectUnit->items[0] != 0 || unit->items[0] != 0) {

            if (!(UNIT_CATTRIBUTES(unit) & CA_SUPPLY)) {
                AddTarget(unit->xPos, unit->yPos, unit->index, 0);
            }
        }
    }

    if (unit->state & US_RESCUING) {
        struct Unit* rescue = GetUnit(unit->rescue);

        if (UNIT_FACTION(rescue) != FACTION_BLUE) {
            return;
        }

        if (gSubjectUnit->items[0] == 0 && rescue->items[0] == 0) {
            return;
        }

        AddTarget(unit->xPos, unit->yPos, rescue->index, 0);
    }

    return;
}

LYN_REPLACE_CHECK(MakeTradeTargetList);
void MakeTradeTargetList(struct Unit * unit)
{
    int x = unit->xPos;
    int y = unit->yPos;
    bool shotputSkill = false;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

#if defined(SID_ShotputPlus) && (COMMON_SKILL_VALID(SID_ShotputPlus))
    if (SkillTesterPlus(unit, SID_ShotputPlus))
    {
        shotputSkill = true;
        MapAddInRange(x, y, 5, 1);
    }
#endif

#if defined(SID_Shotput) && (COMMON_SKILL_VALID(SID_Shotput))
    if (SkillTester(unit, SID_Shotput) && !shotputSkill)
        MapAddInRange(x, y, 3, 1);
#endif

    ForEachAdjacentUnit(x, y, TryAddUnitToTradeTargetList);

    if (gSubjectUnit->state & US_RESCUING)
    {
        int count = GetSelectTargetCount();
        TryAddUnitToTradeTargetList(GetUnit(gSubjectUnit->rescue));

        if (count != GetSelectTargetCount())
        {
            GetTarget(count)->x = gSubjectUnit->xPos;
            GetTarget(count)->y = gSubjectUnit->yPos;
        }
    }

    return;
}

LYN_REPLACE_CHECK(ItemSubMenu_IsTradeAvailable);
u8 ItemSubMenu_IsTradeAvailable(const struct MenuItemDef* def, int number) {
    if (gActiveUnit->state & US_HAS_MOVED) {
        return MENU_NOTSHOWN;
    }

    if (gBmSt.taken_action & BM_TAKEN_ACTION_TRADE) {
        return MENU_NOTSHOWN;
    }

    if (UNIT_CATTRIBUTES(gActiveUnit) & CA_SUPPLY) {
        return MENU_NOTSHOWN;
    }

    if (GetUnitStatusIndex(gActiveUnit) == NEW_UNIT_STATUS_REPLICATE)
        return MENU_NOTSHOWN;

    MakeTradeTargetList(gActiveUnit);

    if (GetSelectTargetCount() == 0) {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}


/*
** I've need to combine IsCharacterForceDeployed_ into IsCharcterForceDeployed as either
** the support menu or or the chapter prep menu would crash if I hooked the former or not
*/
LYN_REPLACE_CHECK(IsCharacterForceDeployed);
s8 IsCharacterForceDeployed(int char_id)
{
    if (0 != CheckInLinkArena())
        return 0;

    if (GetBattleMapKind() != BATTLEMAP_KIND_STORY)
        return 0;

#ifdef CONFIG_FE8_REWRITE
    const struct ForceDeploymentEnt gForceDeploymentList[] = {
        {CHARACTER_EIRIKA,  CHAPTER_MODE_COMMON,  -1  },
        {CHARACTER_EIRIKA,  CHAPTER_MODE_EIRIKA,  -1  },
        {CHARACTER_EPHRAIM, -1,                   -1  },
        {CHARACTER_ARTUR,   -1,                    4  },
        {CHARACTER_NATASHA, -1,                    6  },
        {CHARACTER_JOSHUA,  -1,                    6  },
        {CHARACTER_EIRIKA,  -1,                    10 },
        {CHARACTER_SALEH,   -1,                    12 },
        {CHARACTER_EPHRAIM, CHAPTER_MODE_EIRIKA,   21 },
        {CHARACTER_EIRIKA,  CHAPTER_MODE_EPHRAIM,  34 },
        {-1, 0, 0},
    };
#endif

    const struct ForceDeploymentEnt* it;

    for (it = gForceDeploymentList; it->pid != (u16)-1; it++)
    {
        if (it->route != 0xFF && it->route != gPlaySt.chapterModeIndex)
            continue;

        if (it->chapter != 0xFF && it->chapter != gPlaySt.chapterIndex)
            continue;

        if (char_id != it->pid)
            continue;

        return true;
    }
    return false;
}


//! FE8U = 0x0803247C
LYN_REPLACE_CHECK(ActionSteal);
s8 ActionSteal(ProcPtr proc) {
    int item;

    struct Unit* target = GetUnit(gActionData.targetIndex);

    if (target->state & US_DROP_ITEM) {
        if (gActionData.itemSlotIndex == (GetUnitItemCount(target) - 1)) {
            target->state &= ~US_DROP_ITEM;
        }
    }

    item = GetUnit(gActionData.targetIndex)->items[gActionData.itemSlotIndex];

#if defined(SID_Duplicate) && (COMMON_SKILL_VALID(SID_Duplicate))
    if (gActionData.unk08 != SID_Duplicate)
        UnitRemoveItem(GetUnit(gActionData.targetIndex), gActionData.itemSlotIndex);
#else
    UnitRemoveItem(GetUnit(gActionData.targetIndex), gActionData.itemSlotIndex);
#endif

    switch (ITEM_INDEX(item)) {
    case ITEM_1G:
    case ITEM_5G:
    case ITEM_10G:
    case ITEM_50G:
    case ITEM_100G:
    case ITEM_150G:
    case ITEM_200G:
    case ITEM_3000G:
    case ITEM_5000G:
        SetPartyGoldAmount(GetPartyGoldAmount() + GetItemCost(item));
        break;

    default:
#if defined(SID_Duplicate) && (COMMON_SKILL_VALID(SID_Duplicate))
        if (gActionData.unk08 == SID_Duplicate)
        {
            // Assuming 'item' is your initial u16 value
            u16 originalItem = item; // Keep original item if needed

            // Extract the Item ID
            u8 itemId = ITEM_INDEX(originalItem);

            // Set the desired uses to 1 (which is 1 << 8 when shifted)
            // Combine Item ID and new uses
            u16 newItemWithUses = itemId | (1 << 8);

            // Add the item with the updated uses to the unit's inventory
            UnitAddItem(GetUnit(gActionData.subjectIndex), newItemWithUses);
        }
        else
        {
            UnitAddItem(GetUnit(gActionData.subjectIndex), item);
        }
#else
        UnitAddItem(GetUnit(gActionData.subjectIndex), item);
#endif
        break;
    }

    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), -1);
    gBattleTarget.terrainId = TERRAIN_PLAINS;
    InitBattleUnit(&gBattleTarget, GetUnit(gActionData.targetIndex));
    gBattleTarget.weapon = item;
    BattleApplyMiscAction(proc);

    EndAllMus();
    BeginMapAnimForSteal();

    return 0;
}

LYN_REPLACE_CHECK(PutSubtitleHelpText);
void PutSubtitleHelpText(struct SubtitleHelpProc* proc, int y)
{
    static u16 lut[] = {
        0x00,
        0x04, 0x08, 0x0C, 0x10, 0x14, 0x18,
        0x44, 0x48, 0x4C, 0x50, 0x54, 0x58,
    };

    int i;
    /* JESTER - The modulo operator is broken here so I've swapped it out for k_umod */
    for (i = 0; i < 9; i++) {
        int x = (i * 32) - 32 + proc->textOffset;
        int index = k_umod((proc->textNum + i), proc->textCount);
        /* JESTER - Ensure the subtitle text doesn't overlap with extended item descriptions */
#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
        PutSprite(2, x, y, gObject_32x16, 0x4300 + lut[index]);
#else
        PutSprite(2, x, y, gObject_32x16, 0x4240 + lut[index]);
#endif
    }
    return;
}

LYN_REPLACE_CHECK(InitSubtitleHelpText);
void InitSubtitleHelpText(struct SubtitleHelpProc* proc)
{
    const char* iter;
    int line;
    u32 width;

    iter = proc->string;

    /* JESTER - Ensure the subtitle text doesn't overlap with extended item descriptions */
#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    InitSpriteTextFont(&proc->font, OBJ_VRAM0 + 0x6000, 0x14);
#else
    InitSpriteTextFont(&proc->font, OBJ_VRAM0 + 0x4800, 0x14);
#endif

    SetTextFontGlyphs(1);

    ApplyPalette(gUnknown_0859EF20, 0x14);

    for (line = 0; line < 2; line++) {
        InitSpriteText(proc->text + line);

        SpriteText_DrawBackgroundExt(proc->text + line, 0);
        Text_SetColor(proc->text + line, 0);
    }

    line = 0;

    if (iter != 0) {
        while (*iter > 1) {

            iter = Text_DrawCharacter(proc->text + line, iter);

            if (Text_GetCursor(proc->text + line) > 0xE0) {

                iter -= 2;
                line++;

                GetCharTextLen(iter, &width);

                Text_SetCursor(proc->text + line, (Text_GetCursor(proc->text) - width) - 0xC0);
            }
        }

        proc->textCount = ((GetStringTextLen(proc->string) + 16) >> 5) + 1;
        proc->textNum = proc->textCount - 1;
    }

    SetTextFont(0);

    return;
}


void TryAddUnitToAdjacentSameFactionTargetList(struct Unit* unit) {

    if (UNIT_FACTION(gSubjectUnit) != UNIT_FACTION(unit))
    {
        return;
    }

    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForAdjacentSameFaction(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToAdjacentSameFactionTargetList);

    return;
}

void TryAddUnitToAdjacentEnemyTargetList(struct Unit* unit) {

    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForAdjacentEnemies(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToAdjacentEnemyTargetList);

    return;
}

void TryAddUnitToAdjacentEnemyNonBossTargetList(struct Unit* unit) {

    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (UNIT_CATTRIBUTES(unit) & CA_BOSS) {
        return;
    }

    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForAdjacentNonBossEnemies(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToAdjacentEnemyNonBossTargetList);

    return;
}

void TryAddUnitToAdjacentUnitsTargetList(struct Unit* unit) {
    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForAdjacentUnits(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    BmMapFill(gBmMapRange, 0);

    ForEachAdjacentUnit(x, y, TryAddUnitToAdjacentUnitsTargetList);

    return;
}

void TryAddUnitToRangedStatusStavesTargetList(struct Unit* unit) {

    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->state & US_RESCUED) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForRangedStatusStaves(struct Unit* unit) {
    int x = unit->xPos;
    int y = unit->yPos;

    gSubjectUnit = unit;

    InitTargets(x, y);

    BmMapFill(gBmMapRange, 0);

    MapAddInRange(x, y, GetUnitMagBy2Range(gSubjectUnit), 1);

    ForEachUnitInRange(TryAddUnitToRangedStatusStavesTargetList);

    return;
}

void GiveScroll(void)
{
    FORCE_DECLARE u16 skillId = gEventSlots[EVT_SLOT_3];
    u16 charId = gEventSlots[EVT_SLOT_4];

    FORCE_DECLARE struct Unit* unit;
    unit = GetUnitFromCharId(charId);

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    for (int i = 0; i < 5; i++) {
        if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_1))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_1;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_2))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_2;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_3))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_3;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_4))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_4;
            break;
        }
    }

    // unsigned short *items;
    // items = GetConvoyItemArray();

    for (int i = 0; i < CONFIG_INSTALL_CONVOYEXPA_AMT; i++) {
        if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_1))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_1;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_2))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_2;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_3))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_3;
            break;
        }
        else if (unit->items[i] == ((skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_4))
        {
            unit->items[i] = (skillId << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_4;
            break;
        }
    }
#endif
}

LYN_REPLACE_CHECK(ItemGot_GotLeItem);
void ItemGot_GotLeItem(struct GotItemPopupProc* proc)
{
    /* Stop gap measure to enable scrolls to have the right IDs
    ** They will return a short (maybe a word?) with the first half being the skill ID
    ** and the last part being the skill scroll ID, which is currently 0xBD.
    ** Check if the item we're giving is above the byte limit to determine that
    ** we're giving a scroll.

    ** The limitation of this approach is that it locks us out of having other dynamically
    ** assigned effects and icons for other items in the future. It might be best to create
    ** another version of this function in that case
    */
    if (proc->item > 255)
        HandleNewItemGetFromDrop(proc->unit, proc->item, proc);
    else
        HandleNewItemGetFromDrop(proc->unit, MakeNewItem(proc->item), proc);
}

LYN_REPLACE_CHECK(ItemSubMenu_IsUseAvailable);
u8 ItemSubMenu_IsUseAvailable(const struct MenuItemDef* def, int number) {
    int item = gActiveUnit->items[gActionData.itemSlotIndex];

#ifdef CONFIG_IER_EN
    if (!GetIERevamp(item))
        return MENU_NOTSHOWN;
#else
    if (GetItemUseEffect(item) == 0) {
        return MENU_NOTSHOWN;
    }
#endif

    if (GetItemType(item) == ITYPE_STAFF) {
        return MENU_NOTSHOWN;
    }

    if (GetItemType(item) == ITYPE_12) {
        return MENU_NOTSHOWN;
    }

    if ((GetItemAttributes(item) & IA_WEAPON) && !CanUnitUseWeapon(gActiveUnit, item)) {
        return MENU_NOTSHOWN;
    }

    return CanUnitUseItem(gActiveUnit, item)
        ? MENU_ENABLED : MENU_DISABLED;
}

LYN_REPLACE_CHECK(ItemSubMenu_IsEquipAvailable);
u8 ItemSubMenu_IsEquipAvailable(const struct MenuItemDef* def, int number) {
    int item = gActiveUnit->items[gActionData.itemSlotIndex];

    if (!(GetItemAttributes(item) & IA_WEAPON)) {
        return MENU_NOTSHOWN;
    }

#if defined(SID_GorillaTactics) && (COMMON_SKILL_VALID(SID_GorillaTactics))
    if (CheckBitUES(gActiveUnit, UES_BIT_GorillaTactics_USED))
        return MENU_NOTSHOWN;
#endif

    return CanUnitUseWeapon(gActiveUnit, item)
        ? MENU_ENABLED : MENU_DISABLED;
}

LYN_REPLACE_CHECK(GetPickTrapType);
int GetPickTrapType(struct Unit* unit)
{
    struct Trap* trap;

    if ((trap = GetTrapAt(unit->xPos, unit->yPos)) == NULL)
        return TRAP_NONE;

    switch (trap->type) {
    case TRAP_BALLISTA:
        return TRAP_NONE;

    case TRAP_FIRETILE:
        if ((UNIT_CATTRIBUTES(unit) & CA_THIEF))
            return TRAP_FIRE_THIEF;

        break;

    case TRAP_MINE:
#if defined(SID_TrapBuster) && (COMMON_SKILL_VALID(SID_TrapBuster))
        if (SkillTester(unit, SID_TrapBuster))
        {
            if (GetUnitItemCount(unit) != UNIT_ITEM_COUNT)
                return TRAP_MINE_ASSASSIN;

            return TRAP_NONE;
        }
#endif
        if ((UNIT_CATTRIBUTES(unit) & CA_ASSASSIN))
        {
            if (GetUnitItemCount(unit) != UNIT_ITEM_COUNT)
                return TRAP_MINE_ASSASSIN;

            return TRAP_NONE;
        }
        else if ((UNIT_CATTRIBUTES(unit) & CA_STEAL))
            return TRAP_NONE;

        break;
    }

    return trap->type;
}

LYN_REPLACE_CHECK(RefreshMinesOnBmMap);
void RefreshMinesOnBmMap(void) {
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {
        switch (trap->type) {

        case TRAP_MINE:
#if defined(SID_TrapBuster) && (COMMON_SKILL_VALID(SID_TrapBuster))
            if (!SkillTester(gActiveUnit, SID_TrapBuster))
            {
                if (!gBmMapUnit[trap->yPos][trap->xPos])
                    gBmMapHidden[trap->yPos][trap->xPos] |= HIDDEN_BIT_TRAP;
            }
#else
            if (!gBmMapUnit[trap->yPos][trap->xPos])
                gBmMapHidden[trap->yPos][trap->xPos] |= HIDDEN_BIT_TRAP;
#endif

            break;

        } // switch (trap->type)
    }
}

LYN_REPLACE_CHECK(ProcMAExpBar_OnIncrement);
void ProcMAExpBar_OnIncrement(struct MAExpBarProc* proc)
{
    // Check if we've reached target BEFORE incrementing
    if (proc->expFrom == proc->expTo) {
        Proc_Break(proc);
        m4aSongNumStop(0x74);
        return;
    }

    proc->expFrom++;

    if (proc->expFrom >= 100)
    {
        proc->expFrom = 0;
        proc->expTo -= 100;
    }

    DrawMAExpBar(6, 8, proc->expFrom);
}

LYN_REPLACE_CHECK(MapAnim_DisplayExpBar);
void MapAnim_DisplayExpBar(ProcPtr proc)
{
    int actorNum = -1;
    switch (gManimSt.actorCount) {
    case 2:
        if (gManimSt.actor[1].bu->expGain != 0)
            actorNum = 1;
        // fallthrough

    case 1:
        if (gManimSt.actor[0].bu->expGain != 0)
            actorNum = 0;
        break;
    }

    if (actorNum >= 0) {
        struct MAExpBarProc* expProc = Proc_StartBlocking(ProcScr_MapAnimExpBar, proc);

        expProc->expFrom = gManimSt.actor[actorNum].bu->expPrevious;
        expProc->expTo = gManimSt.actor[actorNum].bu->expPrevious + gManimSt.actor[actorNum].bu->expGain;
        expProc->actorId = actorNum;
    }
}

LYN_REPLACE_CHECK(ProcMAExpBar_LevelUpIfPossible);
void ProcMAExpBar_LevelUpIfPossible(struct MAExpBarProc* proc)
{
    // if (proc->expTo >= 100)
    //     StartManimLevelUp(proc->actorId, (struct Proc*) proc);

    int actorNum = -1;
    switch (gManimSt.actorCount) {
    case 2:
        if (gManimSt.actor[1].bu->expGain != 0)
            actorNum = 1;
        // fallthrough

    case 1:
        if (gManimSt.actor[0].bu->expGain != 0)
            actorNum = 0;
        break;
    }

    if ((gManimSt.actor[actorNum].bu->expPrevious + gManimSt.actor[actorNum].bu->expGain) >= 100)
        StartManimLevelUp(proc->actorId, (struct Proc*)proc);
}

void TransferStatsandExperience(void)
{
    UpdateUnitFromBattle(gActiveUnit, &gBattleActor);
    gActiveUnit->state &= ~US_HIDDEN;       // Remove the hidden state that seems to get randomly set?
    gActiveUnit->state |= US_UNSELECTABLE;  // Ensure the unit cannot be selected again
}

static const struct ProcCmd ProcScr_AddExp[] = {
    PROC_CALL(LockGame),
    PROC_WHILE(BattleEventEngineExists),
    PROC_CALL(EndMapAnimInfoWindow),
    PROC_SLEEP(1),
    PROC_CALL(MapAnim_DisplayExpBar),
    PROC_YIELD,
    PROC_CALL(UnlockGame),
    PROC_CALL(MapAnim_Cleanup),
    PROC_CALL(TransferStatsandExperience),
    PROC_END
};

void BattleApplyMiscActionExpGains_Modular(int exp) {
    if (UNIT_FACTION(&gBattleActor.unit) != FACTION_BLUE)
        return;

    if (!CanBattleUnitGainLevels(&gBattleActor))
        return;

    if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
        return;

    gBattleActor.expGain = exp;
    gBattleActor.unit.exp += exp;

    CheckBattleUnitLevelUp(&gBattleActor);
}

void AddExp_Event(int exp)
{
    gManimSt.actorCount = 1;
    gManimSt.hp_changing = 0;
    gManimSt.subjectActorId = 0;
    gManimSt.targetActorId = 0;
    InitBattleUnit(&gBattleActor, gActiveUnit);
    BattleApplyMiscActionExpGains_Modular(exp);
    SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);
    Proc_StartBlocking(ProcScr_AddExp, PROC_TREE_3);
}

// use vanilla version so we don't lag by using hooked versions that accounts for pass etc
s8 Vanilla_CanUnitCrossTerrain(struct Unit* unit, int terrain)
{
    const s8* lookup = (s8*)GetUnitMovementCost(unit);
    return (lookup[terrain] > 0) ? TRUE : FALSE;
}

bool Generic_CanUnitBeOnPos(struct Unit* unit, s8 x, s8 y, int x2, int y2)
{
    if (x < 0 || y < 0)
        return 0; // position out of bounds
    if (x >= gBmMapSize.x || y >= gBmMapSize.y)
        return 0; // position out of bounds
    if (gBmMapUnit[y][x])
        return 0;
    if (gBmMapHidden[y][x] & 1)
        return 0; // a hidden unit is occupying this position
    if ((x2 == x) && (y2 == y))
        return 0;                                                  // exception / a battle unit is on this tile
    return Vanilla_CanUnitCrossTerrain(unit, gBmMapTerrain[y][x]); // CanUnitCrossTerrain(unit, gMapTerrain[y][x]);
}

bool isWeaponTriangleAdvantage(int attackerWeapon, int defenderWeapon)
{
    if (attackerWeapon == ITYPE_SWORD && defenderWeapon == ITYPE_AXE)
        return true;
    if (attackerWeapon == ITYPE_LANCE && defenderWeapon == ITYPE_SWORD)
        return true;
    if (attackerWeapon == ITYPE_AXE && defenderWeapon == ITYPE_LANCE)
        return true;
    if (attackerWeapon == ITYPE_ANIMA && defenderWeapon == ITYPE_LIGHT)
        return true;
    if (attackerWeapon == ITYPE_LIGHT && defenderWeapon == ITYPE_DARK)
        return true;
    if (attackerWeapon == ITYPE_DARK && defenderWeapon == ITYPE_ANIMA)
        return true;

    return false;
}

bool weaponHasSpecialEffect(int weaponAttributes)
{
    if (weaponAttributes & IA_NEGATE_DEFENSE)
        return true;
    if (weaponAttributes & IA_NEGATE_CRIT)
        return true;
    if (weaponAttributes & IA_NEGATE_FLYING)
        return true;
    if (weaponAttributes & IA_REVERTTRIANGLE)
        return true;
    if (weaponAttributes & IA_UNCOUNTERABLE)
        return true;
    if (weaponAttributes & IA_BRAVE)
        return true;
    if (weaponAttributes & IA_UNBREAKABLE)
        return true;

    return false;
}

int findMax(u8* array, int size) {
    u8 max = 0;
    FORCE_DECLARE u8 array_position = 0;

    for (int i = 0; i < size - 1; i++) {
        if (array[i] > max) {
            max = array[i];
            array_position = i;
        }
    }
    return array_position;
}

static inline u8 CanUnitAttackNow(struct Unit* unit, bool ignoreCanto)
{
    // If not ignoring canto, skip if unit can’t act
    if (!ignoreCanto)
    {
        if (unit->state & (US_HAS_MOVED | US_IN_BALLISTA))
            return MENU_NOTSHOWN;

        if (unit->state & US_CANTOING)
            return MENU_NOTSHOWN;
    }

    // Check all weapons in inventory
    for (int i = 0; i < UNIT_ITEM_COUNT; i++)
    {
        int item = unit->items[i];
        if (item == 0)
            break;

        if (!(GetItemAttributes(item) & IA_WEAPON))
            continue;

        if (!CanUnitUseWeaponNow(unit, item))
            continue;

        MakeTargetListForWeapon(unit, item);
        if (GetSelectTargetCount() == 0)
            continue;

        return MENU_ENABLED;
    }

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    // Unarmed combat fallback
    if (SkillTester(unit, SID_UnarmedCombat))
    {
        MakeTargetListForWeapon(unit, ITEM_SWORD_IRON);
        if (GetSelectTargetCount() > 0)
            return MENU_ENABLED;
    }
#endif

    return MENU_NOTSHOWN;
}

/*
** JESTER - I've rewritten the logic for this function and extracted it into a helper
** function, so that the SID_Warpath check doesn't duplicate the item looping code
*/
LYN_REPLACE_CHECK(AttackCommandUsability);
u8 AttackCommandUsability(const struct MenuItemDef* def, int number)
{
    struct Unit* unit = gActiveUnit;

    if (GetUnitStatusIndex(gActiveUnit) == NEW_UNIT_STATUS_BREAK)
        return MENU_DISABLED;

    if (GetUnitStatusIndex(gActiveUnit) == NEW_UNIT_STATUS_BOUND)
        if (!(GetItemAttributes(GetUnitEquippedWeapon(gActiveUnit)) & IA_MAGIC))
            return MENU_DISABLED;

#if defined(SID_GridMasterPlus) && (COMMON_SKILL_VALID(SID_GridMasterPlus))
    if (SkillTesterPlus(unit, SID_GridMasterPlus) && (unit->state & US_CANTOING))
        return MENU_ENABLED;
#endif

#if defined(SID_Warpath) && (COMMON_SKILL_VALID(SID_Warpath))
    if (SkillTester(unit, SID_Warpath) && (unit->state & US_CANTOING))
        return CanUnitAttackNow(unit, true);
#endif

    // Default: respect canto/movement restrictions
    return CanUnitAttackNow(unit, false);
}

LYN_REPLACE_CHECK(DisplayUnitStandingAttackRange);
int DisplayUnitStandingAttackRange(struct MenuProc* menu, struct MenuItemProc* menuItem) {
    BmMapFill(gBmMapMovement, -1);
    BmMapFill(gBmMapRange, 0);

    if (gActiveUnit->state & US_IN_BALLISTA) {
        MapAddInBoundedRange(gActiveUnit->xPos, gActiveUnit->yPos, 1, 10);
    }
    else {
        int reach = GetUnitWeaponReachBits(gActiveUnit, -1);

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
        if (SkillTester(gActiveUnit, SID_UnarmedCombat) && reach < 3)
            reach = 3;
#endif  

        GenerateUnitStandingReachRange(gActiveUnit, reach);
    }

    DisplayMoveRangeGraphics(3);

    return 0;
}

LYN_REPLACE_CHECK(UnitActionMenu_Attack);
u8 UnitActionMenu_Attack(struct MenuProc* menu, struct MenuItemProc* menuItem) {

    if (menuItem->availability == MENU_DISABLED) {
        MenuFrozenHelpBox(menu, 0x858); // TODO: msgid "There's no more ammo for[NL]the ballista.[.]"
        return MENU_ACT_SND6B;
    }

    ResetIconGraphics();

    LoadIconPalettes(4);

    if (gActiveUnit->state & US_IN_BALLISTA) {
        return StartUnitBallistaSelect(menu, menuItem);
    }

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    if (SkillTester(gActiveUnit, SID_UnarmedCombat))
    {
        ClearBg0Bg1();
        MakeTargetListForWeapon(gActiveUnit, ITEM_SWORD_IRON);
        NewTargetSelection(&gSelectInfo_Attack);
        sub_80832C8();
        return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
    }
#endif  

    return StartUnitWeaponSelect(menu, menuItem);
}

LYN_REPLACE_CHECK(AttackMapSelect_SwitchIn);
u8 AttackMapSelect_SwitchIn(ProcPtr proc, struct SelectTarget* target) {

    struct Unit* unit = GetUnit(target->uid);

    ChangeActiveUnitFacing(target->x, target->y);

    if (target->uid == 0) {
        gActionData.xOther = target->x;
        gActionData.yOther = target->y;
        gActionData.trapType = target->extra;

        InitObstacleBattleUnit();
    }

    if (gActionData.itemSlotIndex == BU_ISLOT_BALLISTA) {
        BattleGenerateBallistaSimulation(gActiveUnit, unit, gActiveUnit->xPos, gActiveUnit->yPos);
    }
    else {
        BattleGenerateSimulation(gActiveUnit, unit, -1, -1, gActionData.itemSlotIndex);
    }

    UpdateBattleForecastContents();

    return 0;
}

extern struct ProcCmd const gProcScr_0859B630[];

LYN_REPLACE_CHECK(AttackMapSelect_Cancel);
u8 AttackMapSelect_Cancel(ProcPtr proc, struct SelectTarget* target) {
    if (EventEngineExists() == 1) {
        return 0;
    }

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    if (SkillTester(gActiveUnit, SID_UnarmedCombat))
    {
        return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6B;
    }
#endif

    Proc_Start(gProcScr_0859B630, PROC_TREE_3);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6B;
}

LYN_REPLACE_CHECK(UnitApplyWorkingMovementScript);
void UnitApplyWorkingMovementScript(struct Unit *unit, int x, int y)
{
    u8 *it = gWorkingMovementScript;

    for (;;)
    {
        gActionData.xMove = x;
        gActionData.yMove = y;

        switch (*it)
        {

        case MOVE_CMD_MOVE_UP: // up
            y--;
            break;

        case MOVE_CMD_MOVE_DOWN: // down
            y++;
            break;

        case MOVE_CMD_MOVE_LEFT: // left
            x--;
            break;

        case MOVE_CMD_MOVE_RIGHT: // right
            x++;
            break;

        } // switch (*it)

        // BUG -  https://github.com/JesterWizard/C-SkillSystem-Mokha/issues/264
        if (!(UNIT_CATTRIBUTES(unit) & (CA_THIEF | CA_FLYER | CA_ASSASSIN)))
        {
            if (gBmMapHidden[y][x] & HIDDEN_BIT_TRAP)
            {
                #ifdef CONFIG_DIRTY_FIXES
                    if (gPlaySt.chapterIndex > 1) // Everything inside this if statement is vanilla code
                    {
                        *++it = MOVE_CMD_HALT;

                        gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                        gActionData.xMove = x;
                        gActionData.yMove = y;

                        return;
                    }
                #else
                    *++it = MOVE_CMD_HALT;

                    gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                    gActionData.xMove = x;
                    gActionData.yMove = y;

                    return;
                #endif
            }
        }
        if (gBmMapHidden[y][x] & HIDDEN_BIT_UNIT)
        {
            #ifdef CONFIG_DIRTY_FIXES
                if (gPlaySt.chapterIndex > 1) // Everything inside this if statement is vanilla code
                {
                    *it++ = MOVE_CMD_BUMP;
                    *it++ = MOVE_CMD_HALT;

                #if defined(SID_Reflex) && (COMMON_SKILL_VALID(SID_Reflex))
                    if (!SkillTester(unit, SID_Reflex))
                        gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                #else
                    gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                #endif

                    return;
                }
            #else
                *it++ = MOVE_CMD_BUMP;
                *it++ = MOVE_CMD_HALT;

                #if defined(SID_Reflex) && (COMMON_SKILL_VALID(SID_Reflex))
                    if (!SkillTester(unit, SID_Reflex))
                        gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                #else
                    gActionData.unitActionType = UNIT_ACTION_TRAPPED;
                #endif

                return;
            #endif
        }
        if (*it == MOVE_CMD_HALT)
            break;

        it++;
    }
}
struct EvCheck0A
{
    u32 unk0;
    u32 script;
    u8 x;
    u8 y;
    u16 tileCommand;
};

//! FE8U = 0x08083A58
LYN_REPLACE_CHECK(EvCheck0A_SHOP);
int EvCheck0A_SHOP(struct EventInfo * info)
{
    struct EvCheck0A * listScript = (void *)info->listScript;

    int x = listScript->x;
    int y = listScript->y;

    int tileCommand = listScript->tileCommand;

#if defined(SID_Secret) && (COMMON_SKILL_VALID(SID_Secret))
    if (SkillTester(gActiveUnit, SID_Secret))
    {
        info->script = listScript->script;
        info->flag = listScript->unk0 >> 16;
        info->commandId = tileCommand;
        return 1;
    }
#endif

    if ((x == info->xPos) && (y == info->yPos))
    {
        if ((tileCommand != TILE_COMMAND_SECRET || (GetUnitItemSlot(gActiveUnit, ITEM_MEMBERCARD) != -1)))
        {
            info->script = listScript->script;
            info->flag = listScript->unk0 >> 16;
            info->commandId = tileCommand;
            return 1;
        }
    }

    return 0;
}

LYN_REPLACE_CHECK(BeginUnitPoisonDamageAnim);
void BeginUnitPoisonDamageAnim(struct Unit * unit, int damage)
{

#if (defined(SID_PoisonHeal) && COMMON_SKILL_VALID(SID_PoisonHeal))
    if (SkillTester(&gBattleActor.unit, SID_PoisonHeal))
    {
        BeginUnitHealAnim(&gBattleActor.unit, damage);
        AddUnitHp(&gBattleActor.unit, damage * 2); // A quick fix for poison damage applying, double the damage and heal it
        return;
    }
#endif

    BattleInitItemEffect(unit, -1);

    AddUnitHp(&gBattleActor.unit, -damage);

    // if (gBattleActor.unit.curHP < 0)
    // {
    //     gBattleActor.unit.curHP = 0;
    // }

    gBattleHitIterator->hpChange = gBattleActor.hpInitial - gBattleActor.unit.curHP;

    if (gBattleActor.unit.curHP == 0)
    {
        gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
    }

    BattleHitTerminate();

    BeginMapAnimForPoisonDmg();

    RenderMapForFogFadeIfUnitDied(unit);

    return;
}

LYN_REPLACE_CHECK(GenerateFireTileTrapTargets);
void GenerateFireTileTrapTargets(int x, int y, int damage)
{
    FORCE_DECLARE bool dontAddTarget = false;

#if (defined(SID_Absolve) && (COMMON_SKILL_VALID(SID_Absolve)))
    if (SkillTester(GetUnit(gBmMapUnit[y][x]), SID_Absolve))
        dontAddTarget = true;
#endif

    if (!dontAddTarget)
        AddTarget(x, y, gBmMapUnit[y][x], damage);
}

LYN_REPLACE_CHECK(GenerateArrowTrapTargets);
void GenerateArrowTrapTargets(int x, int y, int damage)
{
    FORCE_DECLARE bool dontAddTarget = false;
    int iy;

    for (iy = 0; iy < gBmMapSize.y; ++iy)
    {
        if (gBmMapUnit[iy][x])
        {
#if (defined(SID_Absolve) && (COMMON_SKILL_VALID(SID_Absolve)))
            if (SkillTester(GetUnit(gBmMapUnit[iy][x]), SID_Absolve))
                dontAddTarget = true;
#endif
            if (!dontAddTarget)
                AddTarget(x, iy, gBmMapUnit[iy][x], damage);
        }
    }
}

LYN_REPLACE_CHECK(GenerateGasTrapTargets);
void GenerateGasTrapTargets(int x, int y, int damage, int facing)
{
    FORCE_DECLARE bool dontAddTarget = false;

    int i;

    int xInc = 0;
    int yInc = 0;

    switch (facing)
    {

        case FACING_UP:
            xInc = 0;
            yInc = -1;

            break;

        case FACING_DOWN:
            xInc = 0;
            yInc = +1;

            break;

        case FACING_LEFT:
            xInc = -1;
            yInc = 0;

            break;

        case FACING_RIGHT:
            xInc = +1;
            yInc = 0;

            break;

    } // switch (facing)

    for (i = 2; i >= 0; --i)
    {
        x += xInc;
        y += yInc;

        if (gBmMapUnit[y][x])
        {
#if (defined(SID_Absolve) && (COMMON_SKILL_VALID(SID_Absolve)))
            if (SkillTester(GetUnit(gBmMapUnit[y][x]), SID_Absolve))
                dontAddTarget = true;
#endif
            if (!dontAddTarget)
                AddTarget(x, y, gBmMapUnit[y][x], damage);
        }
    }
}

LYN_REPLACE_CHECK(UnitRescue);
void UnitRescue(struct Unit* actor, struct Unit* target) {

#if defined(SID_DangerRanger) && (COMMON_SKILL_VALID(SID_DangerRanger))
    if (SkillTester(actor, SID_DangerRanger))
        gActionDataExpa.refrain_action = true;
#endif

    actor->state  |= US_RESCUING;
    target->state |= US_RESCUED | US_HIDDEN;

    actor->rescue = target->index;
    target->rescue = actor->index;

    target->xPos = actor->xPos;
    target->yPos = actor->yPos;
}

LYN_REPLACE_CHECK(HasBattleUnitGainedWeaponLevel);
s8 HasBattleUnitGainedWeaponLevel(struct BattleUnit * bu)
{

#if (defined(SID_ShadowGiftPlus) && (COMMON_SKILL_VALID(SID_ShadowGiftPlus)))
    if (BattleFastSkillTester(bu, SID_ShadowGiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_DARK)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_DARK] == 0)
                return false;
#endif

#if (defined(SID_ShadowGift) && (COMMON_SKILL_VALID(SID_ShadowGift)))
    if (BattleFastSkillTester(bu, SID_ShadowGift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_DARK)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_DARK] == 0)
                return false;
#endif

#if (defined(SID_LightGiftPlus) && (COMMON_SKILL_VALID(SID_LightGiftPlus)))
    if (BattleFastSkillTester(bu, SID_LightGiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_LIGHT)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_LIGHT] == 0)
                return false;
#endif

#if (defined(SID_StormgiftPlus) && (COMMON_SKILL_VALID(SID_StormgiftPlus)))
    if (BattleFastSkillTester(bu, SID_StormgiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_ANIMA)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_ANIMA] == 0)
                return false;
#endif

#if (defined(SID_Stormgift) && (COMMON_SKILL_VALID(SID_Stormgift)))
    if (BattleFastSkillTester(bu, SID_Stormgift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_ANIMA)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_ANIMA] == 0)
                return false;
#endif

#if (defined(SID_Gracegift) && (COMMON_SKILL_VALID(SID_Gracegift)))
    if (BattleFastSkillTester(bu, SID_Gracegift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_STAFF)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_STAFF] == 0)
                return false;
#endif

#if (defined(SID_GracegiftPlus) && (COMMON_SKILL_VALID(SID_GracegiftPlus)))
    if (BattleFastSkillTester(bu, SID_GracegiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_STAFF)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_STAFF] == 0)
                return false;
#endif

#if (defined(SID_Gracegift) && (COMMON_SKILL_VALID(SID_Gracegift)))
    if (BattleFastSkillTester(bu, SID_Gracegift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_STAFF)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_STAFF] == 0)
                return false;
#endif

#if (defined(SID_BladegiftPlus) && (COMMON_SKILL_VALID(SID_BladegiftPlus)))
    if (BattleFastSkillTester(bu, SID_BladegiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_SWORD)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_SWORD] == 0)
                return false;
#endif

#if (defined(SID_Bladegift) && (COMMON_SKILL_VALID(SID_Bladegift)))
    if (BattleFastSkillTester(bu, SID_Bladegift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_SWORD)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_SWORD] == 0)
                return false;
#endif

#if (defined(SID_PiercegiftPlus) && (COMMON_SKILL_VALID(SID_PiercegiftPlus)))
    if (BattleFastSkillTester(bu, SID_PiercegiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_LANCE)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_LANCE] == 0)
                return false;
#endif

#if (defined(SID_Piercegift) && (COMMON_SKILL_VALID(SID_Piercegift)))
    if (BattleFastSkillTester(bu, SID_Piercegift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_LANCE)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_LANCE] == 0)
                return false;
#endif

#if (defined(SID_HackgiftPlus) && (COMMON_SKILL_VALID(SID_HackgiftPlus)))
    if (BattleFastSkillTester(bu, SID_HackgiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_AXE)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_AXE] == 0)
                return false;
#endif

#if (defined(SID_Hackgift) && (COMMON_SKILL_VALID(SID_Hackgift)))
    if (BattleFastSkillTester(bu, SID_Hackgift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_AXE)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_AXE] == 0)
                return false;
#endif

#if (defined(SID_ArcgiftPlus) && (COMMON_SKILL_VALID(SID_ArcgiftPlus)))
    if (BattleFastSkillTester(bu, SID_ArcgiftPlus))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_BOW)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_BOW] == 0)
                return false;
#endif

#if (defined(SID_Arcgift) && (COMMON_SKILL_VALID(SID_Arcgift)))
    if (BattleFastSkillTester(bu, SID_Arcgift))
        if (GetItemType(GetUnit(bu->unit.index)->items[0]) == ITYPE_BOW)
            if (GetUnit(bu->unit.index)->ranks[ITYPE_BOW] == 0)
                return false;
#endif

    int oldWexp = bu->unit.ranks[bu->weaponType];
    int newWexp = GetBattleUnitUpdatedWeaponExp(bu);

    if (newWexp < 0)
        return FALSE;

    return GetWeaponLevelFromExp(oldWexp) != GetWeaponLevelFromExp(newWexp);
}

/* JESTER - This exists for SID_Upgrade for skills that use the regular SkillTester check */
bool SkillTesterPlus(struct Unit *unit, u16 sid)
{
    if (SkillTester(unit, sid))
        return true;

    /* If the unit doesn't have upgrade we won't look for the plus version of the skill */
    if (!SkillTester(unit, SID_Upgrade))
        return false;

    // O(1) lookup for base skill
    const u16 base = (sid <= MAX_SKILL_NUM) ? gSkillUpgradePlusLookup[sid] : 0;
    return base ? SkillTester(unit, base) : false;
}

LYN_REPLACE_CHECK(UnitUpdateUsedItem);
void UnitUpdateUsedItem(struct Unit * unit, int itemSlot)
{

    /**
     * If the item is a scroll, we need to check for the scroll savant skill.
     * If the unit has the skill, then we skip the check to reduce item uses.
     */
    switch (GetItemIndex(unit->items[itemSlot]))
    {
        case CONFIG_ITEM_INDEX_SKILL_SCROLL_1:
        case CONFIG_ITEM_INDEX_SKILL_SCROLL_2:
        case CONFIG_ITEM_INDEX_SKILL_SCROLL_3:
        case CONFIG_ITEM_INDEX_SKILL_SCROLL_4:
#if defined(SID_ScrollSavant) && (COMMON_SKILL_VALID(SID_ScrollSavant))
            if (SkillTester(unit, SID_ScrollSavant))
                return;
#endif
        default:
            break;
    }

    if (unit->items[itemSlot])
    {
        unit->items[itemSlot] = GetItemAfterUse(unit->items[itemSlot]);
        UnitRemoveInvalidItems(unit);
    }
}

LYN_REPLACE_CHECK(ExecVulneraryItem);
void ExecVulneraryItem(ProcPtr proc, int amount) {
    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), gActionData.itemSlotIndex);

#if (defined(SID_ItemLore) && (COMMON_SKILL_VALID(SID_ItemLore)))
    if (SkillTester(gActiveUnit, SID_ItemLore))
        amount *= 2;
#endif

    AddUnitHp(GetUnit(gActionData.subjectIndex), amount);

    gBattleHitIterator->hpChange = gBattleActor.unit.curHP - GetUnitCurrentHp(GetUnit(gActionData.subjectIndex));

    gBattleActor.unit.curHP = GetUnitCurrentHp(GetUnit(gActionData.subjectIndex));

    gBattleActor.weaponBefore = ITEM_VULNERARY;

    BattleApplyItemEffect(proc);
    BeginBattleAnimations();

    return;
}

LYN_REPLACE_CHECK(ExecElixirItem);
void ExecElixirItem(ProcPtr proc) {
    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), gActionData.itemSlotIndex);

    int amount = 80;

#if (defined(SID_ItemLore) && (COMMON_SKILL_VALID(SID_ItemLore)))
    if (SkillTester(gActiveUnit, SID_ItemLore))
        amount *= 2;
#endif

    AddUnitHp(GetUnit(gActionData.subjectIndex), amount);

    /* For the purposes of dealing with healing > 127 we employ the abs function to always return the positive value */
    gBattleHitIterator->hpChange = abs(gBattleActor.unit.curHP - GetUnitCurrentHp(GetUnit(gActionData.subjectIndex)));

    gBattleActor.unit.curHP = GetUnitCurrentHp(GetUnit(gActionData.subjectIndex));

    BattleApplyItemEffect(proc);
    BeginBattleAnimations();

    return;
}

//! FE8U = 0x0801D300
LYN_REPLACE_CHECK(RunPotentialWaitEvents);
bool RunPotentialWaitEvents(void)
{
#if defined(SID_GoodListener) && (COMMON_SKILL_VALID(SID_GoodListener))
    if (SkillTester(gActiveUnit, SID_GoodListener) && gActionData.unitActionType == UNIT_ACTION_VISIT)
    {
        AddExp_Event(10);
    }
#endif

    if (CheckForWaitEvents())
    {
        RunWaitEvents();
        return false;
    }

    return true;
}

static const u8 monsterClasses[25] = 
{
    CLASS_MANAKETE,
    CLASS_MANAKETE_MYRRH,
    CLASS_PHANTOM,
    CLASS_REVENANT,
    CLASS_ENTOMBED,
    CLASS_BONEWALKER,
    CLASS_BONEWALKER_BOW,
    CLASS_WIGHT,
    CLASS_WIGHT_BOW,
    CLASS_BAEL,
    CLASS_ELDER_BAEL,
    CLASS_CYCLOPS,
    CLASS_MAUTHEDOOG,
    CLASS_GWYLLGI,
    CLASS_TARVOS,
    CLASS_MAELDUIN,
    CLASS_MOGALL,
    CLASS_ARCH_MOGALL,
    CLASS_GORGON,
    CLASS_GORGONEGG,
    CLASS_GORGONEGG2,
    CLASS_GARGOYLE,
    CLASS_DEATHGOYLE,
    CLASS_DRACO_ZOMBIE,
    CLASS_DEMON_KING,
};

#define MONSTER_CLASS_COUNT (sizeof(monsterClasses) / sizeof(monsterClasses[0]))

bool isMonsterClass(int classId) {
    for (unsigned i = 0; i < MONSTER_CLASS_COUNT; i++) {
        if (monsterClasses[i] == classId) {
            return true;
        }
    }
    return false;
}

LYN_REPLACE_CHECK(BattleInitTargetCanCounter);
void BattleInitTargetCanCounter(void)
{

    // Target cannot counter if it is a gorgon egg
    if (UNIT_IS_GORGON_EGG(&gBattleTarget.unit))
    {
        gBattleTarget.weapon = 0;
        gBattleTarget.canCounter = FALSE;
    }

    // Target cannot counter if either units are using "uncounterable" weapons
#if defined(SID_Counterattack) && (COMMON_SKILL_VALID(SID_Counterattack))
    if (!SkillTester(&gBattleTarget.unit, SID_Counterattack))
    {
        if ((gBattleActor.weaponAttributes | gBattleTarget.weaponAttributes) & IA_UNCOUNTERABLE)
        {
            gBattleTarget.weapon = 0;
            gBattleTarget.canCounter = FALSE;
        }
    }
#else
    if ((gBattleActor.weaponAttributes | gBattleTarget.weaponAttributes) & IA_UNCOUNTERABLE)
    {
        gBattleTarget.weapon = 0;
        gBattleTarget.canCounter = FALSE;
    }
    else
    {
#if defined(SID_DualWieldPlus) && (COMMON_SKILL_VALID(SID_DualWieldPlus))
        if (SkillTester(GetUnit(gBattleActor.unit.index), SID_DualWieldPlus))
        {
            for (int i = 1; i < UNIT_MAX_INVENTORY; i++)
            {
                if (GetItemMight(gBattleActor.unit.items[i]) > 0 &&
                    CanUnitUseWeapon(GetUnit(gBattleActor.unit.index), gBattleActor.unit.items[i]))
                {
                    if (GetItemAttributes(gBattleActor.unit.items[i]) & IA_UNCOUNTERABLE)
                    {
                        gBattleTarget.weapon = 0;
                        gBattleTarget.canCounter = FALSE;
                        break;
                    }
                }
            }
        }
#endif
    }
#endif

    // Target cannot counter if a berserked player unit is attacking another player unit
    if (gBattleActor.unit.statusIndex == UNIT_STATUS_BERSERK)
    {
        if ((UNIT_FACTION(&gBattleActor.unit) == FACTION_BLUE) && (UNIT_FACTION(&gBattleTarget.unit) == FACTION_BLUE))
        {
            gBattleTarget.weapon = 0;
            gBattleTarget.canCounter = FALSE;
        }
    }
}

LYN_REPLACE_CHECK(PrepItemUseBooster_OnEnd);
void PrepItemUseBooster_OnEnd(struct ProcPrepItemUseBooster * proc)
{
    struct ProcPrepItemUse *parent = proc->proc_parent;
    int max = GetUnitItemCount(parent->unit);
    TileMap_FillRect(TILEMAP_LOCATED(gBG2TilemapBuffer, 14, 14), 14, 1, 0);

    if (max == 0) {
        Proc_Goto(parent, 0x6);
    } else {
        if (parent->slot >= max)
            parent->slot--;

        ShowSysHandCursor(0x10, parent->slot * 0x10 + 0x48, 0xB, 0x800);
    }

    DrawPrepScreenItems(
        TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9),
        &gPrepItemTexts[15],
        parent->unit, 1
    );

    DrawPrepScreenItemUseDesc(parent->unit, parent->slot);

    sub_80ACA84(0);
    EndManimLevelUpStatGainLabels();
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG2_SYNC_BIT);

#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06012000, -1);
#else
    LoadHelpBoxGfx(OBJ_VRAM1, -1);
#endif
}

LYN_REPLACE_CHECK(PrepItemUseJuna_OnEnd);
void PrepItemUseJuna_OnEnd(struct ProcPrepItemUseJunaFruit *proc)
{
    struct ProcPrepItemUse *parent = proc->proc_parent;
    int max = GetUnitItemCount(parent->unit);
    TileMap_FillRect(TILEMAP_LOCATED(gBG2TilemapBuffer, 17, 14), 12, 1, 0);

    if (max == 0) {
        Proc_Goto(parent, 0x6);
    } else {
        if (parent->slot >= max)
            parent->slot--;

        ShowSysHandCursor(0x10, parent->slot * 0x10 + 0x48, 0xB, 0x800);
    }

    DrawPrepScreenItems(
        TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9),
        &gPrepItemTexts[15],
        parent->unit, 1
    );

    DrawPrepScreenItemUseDesc(parent->unit, parent->slot);

    sub_80ACA84(0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG2_SYNC_BIT);

#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06012000, -1);
#else
    LoadHelpBoxGfx(OBJ_VRAM1, -1);
#endif
}

LYN_REPLACE_CHECK(PrepItemUse_InitDisplay);
void PrepItemUse_InitDisplay(struct ProcPrepItemUse *proc)
{
    int i;
    const char *str;

    struct Text *texts;
    struct FaceVramEntry face_config[4] = {
        {0x5800, 0x6},
        {0x6800, 0x7},
        {0x0000, 0x0},
        {0x0000, 0x0}
    };

    gLCDControlBuffer.dispcnt.mode = 0;
    SetupBackgrounds(gBgConfig_ItemUseScreen);
    SetupFaceGfxData(face_config);
    proc->unk34 = 0xFF;

    BG_Fill(BG_GetMapBuffer(BG_0), 0);
    BG_Fill(BG_GetMapBuffer(BG_1), 0);
    BG_Fill(BG_GetMapBuffer(BG_2), 0);

    gLCDControlBuffer.bg0cnt.priority = 1;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 0;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();
    ApplyUnitSpritePalettes();

    BG_SetPosition(BG_0, 0,0);
    BG_SetPosition(BG_1, 0,0);
    BG_SetPosition(BG_2, 0,0);


#ifdef CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
    LoadHelpBoxGfx((void*)0x06013000, -1);
#else
    LoadHelpBoxGfx(OBJ_VRAM1, -1);
#endif

    LoadIconPalettes(4);
    RestartMuralBackground();

    for (i = 0; i < 5; i++)
        InitTextDb(&gPrepItemTexts[0xF + i], 7);

    for (i = 0; i < 8; i++)
        InitText(&gPrepItemTexts[TEXT_PREPITEM_HP + i], 3);

    texts = gPrepItemTexts;

    InitText(&texts[TEXT_PREPITEM_CLASS], 8);
    InitText(&texts[25], 15);
    InitText(&texts[26], 15);
    InitText(&texts[29], 15);
    InitText(&texts[27], 14);
    InitText(&texts[28], 8);

    DrawPrepScreenItemUseStatLabels(proc->unit);
    DrawPrepScreenItemUseStatValues(proc->unit);
    DrawPrepScreenItemUseStatBars(proc->unit, 0);

    StartFace2(0, GetUnitPortraitId(proc->unit), 0x40, -4, 0x203);
    PutImg_PrepItemUseUnk(0x6000, 0x5);
    PutImg_PrepPopupWindow(0x800, 0xA);

    Decompress(Img_PrepItemUseScreen, (void *)BG_VRAM + 0x440);
    Decompress(Tsa_PrepItemUseScreen, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);

    StartSysBrownBox(0xD, 0xE00, 0xF, 0xC00, 0, proc);
    EnableSysBrownBox(0, -0x28, -1, 1);

    str = GetStringFromIndex(proc->unit->pCharacterData->nameTextId);
    PutDrawText(
        0, gBG0TilemapBuffer, 
        TEXT_COLOR_SYSTEM_WHITE, 
        (0x30 - GetStringTextLen(str))/2,
        6, str
    );

    /* Some hand obj API */
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    StartParallelWorker(PrepItemUseParallel_UpdateSMS, proc);

    SetWinEnable(1, 0, 0);
    SetWin0Box(0x68, 0x5E, 0xF0, 0x92);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWOutLayers(1, 1, 1, 1, 1);

    gLCDControlBuffer.wincnt.win0_enableBlend = 1;
    gLCDControlBuffer.wincnt.wout_enableBlend = 0;

    SetBlendConfig(3, 0, 0, 8);
    SetBlendTargetA(0,0,0,1,0);
    StartGreenText(proc);
    StartHelpPromptSprite(0xC0, 0x90, 9, proc);
    DrawPrepScreenItemUseDesc(proc->unit, proc->slot);

    DrawPrepScreenItems(
        TILEMAP_LOCATED(gBG0TilemapBuffer, 2, 9),
        &texts[15],
        proc->unit, 1
    );

    ShowSysHandCursor(
        (proc->slot >> 3) * 7 * 0x10 + 0x10,
        (proc->slot & 0x7) * 0x10 + 0x48,
        0xB, 0x800
    );

    UseUnitSprite(GetUnitSMSId(proc->unit));
    ForceSyncUnitSpriteSheet();
}

int GetUnitMaxMP(struct Unit * unit)
{
	if (!CheckHasBwl(UNIT_CHAR_ID(unit)))
        return 0;
    else
    {
        struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(unit));

        int maxMP = bwl->maxMP;

    #if defined(SID_ManaRush) && (COMMON_SKILL_VALID(SID_ManaRush))
        if (SkillTester(unit, SID_ManaRush))
            maxMP = maxMP * 2;
    #endif

        return maxMP;
    }
};

int GetUnitCurrentMP(struct Unit * unit)
{
	if (!CheckHasBwl(UNIT_CHAR_ID(unit)))
        return 0;
    else
    {
        struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(unit));

        int currentMP = bwl->currentMP;
        int maxMP = GetUnitMaxMP(unit);

        if (currentMP > maxMP)
        {
            bwl->currentMP = maxMP;
            currentMP = maxMP;
        }

        return currentMP;
    }
};

u8 EWRAM_DATA gSMSGfxBuffer[3][8*0x20*0x20] = {};

LYN_REPLACE_CHECK(SyncUnitSpriteSheet);
void SyncUnitSpriteSheet(void)
{
    int frame = k_umod(GetGameClock(), 72);

    if (frame == 0)
        CpuFastCopy(gSMSGfxBuffer[0], (void*)0x06011000, sizeof(gSMSGfxBuffer[0]));

    if (frame == 32)
        CpuFastCopy(gSMSGfxBuffer[1], (void*)0x06011000, sizeof(gSMSGfxBuffer[1]));

    if (frame == 36)
        CpuFastCopy(gSMSGfxBuffer[2], (void*)0x06011000, sizeof(gSMSGfxBuffer[2]));

    if (frame == 68)
        CpuFastCopy(gSMSGfxBuffer[1], (void*)0x06011000, sizeof(gSMSGfxBuffer[1]));
}

// static u8 const sMuWalkSpeedLut[2] = {
//     [UNIT_WALKSPEED_FAST] = 2,
//     [UNIT_WALKSPEED_SLOW] = 1,
// };

// LYN_REPLACE_CHECK(GetMuQ4MovementSpeed);
// u16 GetMuQ4MovementSpeed(struct MuProc * proc)
// {
// #ifdef CONFIG_SUPER_FAST_MAP_ANIMATIONS
//     return 0x200;
// #endif

//     int config = proc->moveConfig;

//     if (config & 0x80)
//         config += 0x80; // I don't really get that one

//     if (proc->fast_walk_b)
//         return 0x100;

//     if (config != 0x40)
//     {
//         if (config != 0x00)
//         {
//             int speed = config;

//             if (speed & 0x40)
//                 speed ^= 0x40;
//             else if (gPlaySt.config.gameSpeed || (gKeyStatusPtr->heldKeys & A_BUTTON))
//                 speed *= 4;

//             if (speed > 0x80)
//                 speed = 0x80;

//             return speed;
//         }

//         if (!IsFirstPlaythrough() && (gKeyStatusPtr->heldKeys & A_BUTTON))
//             return 0x80;

//         if (gPlaySt.config.gameSpeed)
//             return 0x40;
//     }

//     return 16 * sMuWalkSpeedLut[GetClassData(proc->jid)->slowWalking];
// }


//! FE8U = 0x0801DB4C
LYN_REPLACE_CHECK(TrySwitchViewedUnit);
void TrySwitchViewedUnit(int x, int y)
{
    int i;
    int unitId = gBmMapUnit[y][x];
    int factionMin, factionMax;
    
#ifdef CONFIG_L_BUTTON_SAME_FACTION_CYCLING
    // Determine faction range based on current unit
    if (unitId & 0x80)
    {
        // Enemy Faction (Red)
        factionMin = 0x80;
        factionMax = 0xBF;
    }
    else if (unitId & 0x40)
    {
        // NPC Faction (Green)
        factionMin = 0x40;
        factionMax = 0x7F;
    }
    else
    {
        // Player Faction (Blue)
        factionMin = 0x01;
        factionMax = 0x3F;
    }
#else
    // Original behavior: only cycle through Blue faction
    if ((unitId & 0xC0) != FACTION_BLUE)
    {
        unitId = 0;
    }
    factionMin = 0x01;
    factionMax = 0x3F;
#endif
    
    // Normalize unitId if outside faction range
    if (unitId < factionMin || unitId > factionMax)
    {
        unitId = factionMin - 1;
    }
    
    unitId++;
    
    // Wrap around if past end of faction
    if (unitId > factionMax)
    {
        unitId = factionMin;
    }
    
    // First loop: from current unit to end of faction
    for (i = unitId; i <= factionMax; ++i)
    {
        if (TrySetCursorOn(i))
        {
            return;
        }
    }
    
    // Second loop: wrap around from start of faction to current unit
    for (i = factionMin; i < unitId; ++i)
    {
        if (TrySetCursorOn(i))
        {
            return;
        }
    }
    
    return;
}

//! FE8U = 0x080A207C
LYN_REPLACE_CHECK(InitSupportSubScreenRemainingSupports);
void InitSupportSubScreenRemainingSupports(struct SubScreenProc* proc) {
    int i;

    if (proc->fromPrepScreen) {
#ifdef CONFIG_UNLOCK_SUPPORT_CONVO_LIMIT
        proc->remainingSupports = MAX_SIMULTANEOUS_SUPPORT_COUNT - GetTotalSupportLevel(proc->unitIdx);
#else
        proc->remainingSupports = 5 - GetTotalSupportLevel(proc->unitIdx);
#endif
    } else {
        int charId = GetSupportScreenCharIdAt(proc->unitIdx);

        proc->remainingSupports = 0;

        for (i = 0; i < proc->partnerCount; i++) {
            proc->remainingSupports += GetUnitsAverageSupportValue(charId, GetSupportScreenPartnerCharId(proc->unitIdx, i));
        }

        proc->remainingSupports -= GetTotalSupportLevel(proc->unitIdx);
    }

    return;
}

//! FE8U = 0x0801550C
LYN_REPLACE_CHECK(BmMain_StartIntroFx);
void BmMain_StartIntroFx(ProcPtr proc)
{

#ifdef CONFIG_SKIP_CHAPTER_INTROS
    return;
#endif

#ifdef CONFIG_BASE_CHAPTERS
    if (CheckFlag(GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP))
    {
        ClearFlag(GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP);
        return;
    }
#endif

    if (gPlaySt.chapterIndex == 0x38)
        return;

    if (gPlaySt.chapterIndex == 0x06 && CheckFlag(0x88))
        return;

    Proc_StartBlocking(gProcScr_ChapterIntro, proc);
}

//! FE8U = 0x080843C0
LYN_REPLACE_CHECK(RunWaitEvents);
void RunWaitEvents(void) {
    struct EventInfo* pInfo;
    struct EventInfo info;

    if (AreAnyEnemyUnitDead() == 0) {
        SetFlag(EVFLAG_DEFEAT_ALL);
        if (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH) {
            CallEndEvent();
            return;
        }
    } else {
        ClearFlag(6);
    }

#ifdef CONFIG_HOTFIXES
    // JESTER - This is sort of a hotfix since the game over screen stopped working around commit b49e56733fe5326857996e73b636abcf9473ec43
    // I'm not sure why the flag isn't being set but something is stopping it. Maybe look at PROTECTs to narrow down the source?
    struct Unit * unitA = GetUnit(gActionData.subjectIndex);
    struct Unit * unitB = GetUnit(gActionData.targetIndex);

    if (GetUnitFaction(unitA) == FACTION_BLUE && unitA->state & US_DEAD && UNIT_CATTRIBUTES(unitA) & CA_LORD)
        SetFlag(EVFLAG_GAMEOVER);
    else if (GetUnitFaction(unitB) == FACTION_BLUE && unitB->state & US_DEAD && UNIT_CATTRIBUTES(unitB) & CA_LORD)
        SetFlag(EVFLAG_GAMEOVER);
#endif

    if (CheckFlag(0x65) || (CountAvailableBlueUnits() == 0)) {
        sub_80837B0();
        return;
    }


    if (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH) {
        return;
    }

    RunTutorialEvent(TUTORIAL_EVT_TYPE_POSTACTION);

    info.listScript = GetChapterEventDataPointer(gPlaySt.chapterIndex)->miscBasedEvents;
    info.xPos = gActiveUnit->xPos;
    info.yPos = gActiveUnit->yPos;

    if (!SearchAvailableEvent(&info)) {
        return;
    }

    ClearActiveEventRegistry();

    do {
        StartEventFromInfo(&info, EV_EXEC_CUTSCENE);
        pInfo = SearchNextAvailableEvent(&info);
    } while (pInfo);

    return;
}

LYN_REPLACE_CHECK(AreUnitsAllied);
s8 AreUnitsAllied(int left, int right) {

#ifdef CONFIG_FOURTH_ALLEGIANCE
    int l = left  & 0xC0;
    int r = right & 0xC0;

    /* Collapse Player (0x00) and Green (0x40) */
    if (l == 0x40) l = 0x00;
    if (r == 0x40) r = 0x00;

    return l == r;
#else
    int a = left & 0x80;
    int b = right & 0x80;
    return (a == b);
#endif
}

static inline int CheckAltBgm(u8 base, u8 alt) {
    if (!CheckFlag(EVFLAG_BGM_CHANGE)) {
        return base;
    } else {
        return alt;
    }
}

//! FE8U = 0x08015FC8
LYN_REPLACE_CHECK(GetCurrentMapMusicIndex);
int GetCurrentMapMusicIndex(void) {
    int aliveUnits;
    u32 mapKind;

    u8 blueBgmIdx = CheckAltBgm(MAP_BGM_BLUE, MAP_BGM_BLUE_GREEN_ALT);
    u8 redBgmIdx = CheckAltBgm(MAP_BGM_RED, MAP_BGM_RED_ALT);
    u8 greenBgmIdx;

    if (!CheckFlag(EVFLAG_BGM_CHANGE)) {
        greenBgmIdx = MAP_BGM_GREEN;
        greenBgmIdx++; greenBgmIdx--;
    } else {
        greenBgmIdx = MAP_BGM_BLUE_GREEN_ALT;
    }

    switch (gPlaySt.faction) {
        case FACTION_RED:
            return GetROMChapterStruct(gPlaySt.chapterIndex)->mapBgmIds[redBgmIdx];

        case FACTION_GREEN:
            return GetROMChapterStruct(gPlaySt.chapterIndex)->mapBgmIds[greenBgmIdx];

        case FACTION_BLUE:

            if (CheckFlag(EVFLAG_BGM_CHANGE)) {
                return GetROMChapterStruct(gPlaySt.chapterIndex)->mapBgmIds[blueBgmIdx];
            }

            if ((GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH) || GetROMChapterStruct(gPlaySt.chapterIndex)->victorySongEnemyThreshold != 0) {
                aliveUnits = CountUnitsInState(FACTION_RED, US_UNAVAILABLE);
                mapKind = GetBattleMapKind();

                if ((mapKind != BATTLEMAP_KIND_SKIRMISH && aliveUnits <= GetROMChapterStruct(gPlaySt.chapterIndex)->victorySongEnemyThreshold)
                    || (mapKind == BATTLEMAP_KIND_SKIRMISH && aliveUnits <= 1))
                    return SONG_GRASP_AT_VICTORY;
            }

            return GetROMChapterStruct(gPlaySt.chapterIndex)->mapBgmIds[blueBgmIdx];

#ifdef CONFIG_FOURTH_ALLEGIANCE
        case FACTION_PURPLE:
            return GetROMChapterStruct(gPlaySt.chapterIndex)->mapBgmIds[redBgmIdx];
#endif
    }

    return 0; // JESTER - We'll never get here, but the function demands a return value;
}