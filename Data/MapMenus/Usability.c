#include "common-chax.h"
#include "constants/texts.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "jester_headers/custom-functions.h"

int MapMenu_AchievementsCommandDraw(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
    Text_DrawString(&menuItem->text, Utf8ToNarrowFonts(GetStringFromIndex(menuItem->def->nameMsgId)));
    PutText(&menuItem->text, BG_GetMapBuffer(menu->frontBg) + TILEMAP_INDEX(menuItem->xTile, menuItem->yTile));
    return 0;
}

const struct MenuItemDef gMapMenuItems_NEW[] = {
    {"　部隊", 0x69A, 0x6DF, 0, 0x6e, MenuAlwaysEnabled, 0, MapMenu_UnitCommand, 0, 0, 0}, // Unit >
    {"　状況", 0x690, 0x6E0, 0, 0x6f, MenuAlwaysEnabled, 0, MapMenu_StatusCommand, 0, 0, 0}, // Status >
    {"　辞書", 0x69C, 0x6E5, 4, 0x74, MapMenu_IsGuideCommandAvailable, MapMenu_GuideCommandDraw, MapMenu_GuideCommand, 0, 0, 0}, // Guide
    {"Goals", MSG_MAP_MENU_GOALS_TITLE, MSG_MAP_MENU_GOALS_DESC, 4, 0x75, MapMenu_IsAchievementsCommandAvailable, MapMenu_AchievementsCommandDraw, MapMenu_AchievementsCommand, 0, 0, 0}, // Goals
    {"　辞書", MSG_MAP_MENU_BIOGRAPHY_TITLE, MSG_MAP_MENU_BIOGRAPHY_DESC, 4, 0x74, MapMenu_IsBiographyCommandAvailable, MapMenu_BiographyCommandDraw, MapMenu_BiographyCommand, 0, 0, 0}, // Bios
    {"　戦績", 0x69E, 0x6E3, 0, 0x70, MapMenu_IsRecordsCommandAvailable, 0, MapMenu_RecordsCommand, 0, 0, 0}, // Records
    {"　設定", 0x69B, 0x6E1, 0, 0x71, MenuAlwaysEnabled, 0, MapMenu_OptionsCommand, 0, 0, 0}, // Options
    {"　退却", 0x69D, 0x6E2, 0, 0x72, MapMenu_IsRetreatCommandAvailable, 0, MapMenu_RetreatCommand, 0, 0, 0}, // Retreat
    {"　中断", 0x69F, 0x6E4, 0, 0x73, MapMenu_IsSuspendCommandAvailable, 0, MapMenu_SuspendCommand, 0, 0, 0}, // Suspend
    {"　終了", 0x6A0, 0x6E6, 0, 0x78, MenuAlwaysEnabled, 0, CommandEffectEndPlayerPhase, 0, 0, 0}, // End Phase
    MenuItemsEnd
};

const struct MenuDef gMapMenuDef_NEW = {
    {1, 2, 7, 0},
    0,
    gMapMenuItems_NEW,
    0, 0, 0,
    MenuCancelSelect,
    MenuAutoHelpBoxSelect,
    MenuStdHelpBox
};

//! FE8U = 0x0801C940
LYN_REPLACE_CHECK(PlayerPhase_MainIdle);
void PlayerPhase_MainIdle(ProcPtr proc)
{
    HandlePlayerCursorMovement();

    if (gKeyStatusPtr->newKeys & L_BUTTON)
    {
        TrySwitchViewedUnit(gBmSt.playerCursor.x, gBmSt.playerCursor.y);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
    }
    else if (!DoesBMXFADEExist())
    {
        if ((gKeyStatusPtr->newKeys & R_BUTTON) && (gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x] != 0))
        {
            if (CanShowUnitStatScreen(GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x])))
            {

                EndAllMus();

                EndPlayerPhaseSideWindows();
                SetStatScreenConfig(
                    STATSCREEN_CONFIG_NONDEAD | STATSCREEN_CONFIG_NONBENCHED | STATSCREEN_CONFIG_NONUNK9 |
                    STATSCREEN_CONFIG_NONROOFED | STATSCREEN_CONFIG_NONUNK16);

                StartStatScreen(GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x]), proc);

                Proc_Goto(proc, 5);

                return;
            }
        }

        /* This is for Vesly's debugger */
        if (gKeyStatusPtr->newKeys & B_BUTTON)
        {
            if (gpKernelDesignerConfig->vesly_debugger == true)
                StartDebuggerProc(proc);
        }

        if (gKeyStatusPtr->newKeys & A_BUTTON)
        {
            struct Unit * unit = GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x]);

            switch (GetPlayerSelectKind(unit))
            {
                case PLAYER_SELECT_NOUNIT:
                case PLAYER_SELECT_TURNENDED:
                    EndPlayerPhaseSideWindows();

                    gPlaySt.xCursor = gBmSt.playerCursor.x;
                    gPlaySt.yCursor = gBmSt.playerCursor.y;

                    if (unit)
                    {
                        EndAllMus();
                        ShowUnitSprite(unit);
                    }

                    StartOrphanMenuAdjusted(&gMapMenuDef_NEW, gBmSt.cursorTarget.x - gBmSt.camera.x, 1, 0x17);
                    sub_80832CC();

                    Proc_Goto(proc, 9);

                    return;

                case PLAYER_SELECT_CONTROL:
                    UnitBeginAction(unit);
                    PidStatsAddActAmt(gActiveUnit->pCharacterData->number);

                    Proc_Break(proc);

                    break;

                case PLAYER_SELECT_NOCONTROL:
                    UnitBeginAction(unit);
                    gBmSt.swapActionRangeCount = 0;

                    Proc_Goto(proc, 11);

                    break;

                default:
                    goto else_stmt;
            }
        }
        else
        {
else_stmt:
            if ((gKeyStatusPtr->newKeys & START_BUTTON) && !(gKeyStatusPtr->heldKeys & SELECT_BUTTON))
            {
                struct Unit * unit = GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x]);

                if (unit)
                {
                    EndAllMus();
                    ShowUnitSprite(unit);
                }

                EndPlayerPhaseSideWindows();
                StartMinimapPlayerPhase();

                Proc_Goto(proc, 9);

                return;
            }
        }


    }

    UnitSpriteHoverUpdate();

    PutMapCursor(
        gBmSt.playerCursorDisplay.x, gBmSt.playerCursorDisplay.y,
        IsUnitSpriteHoverEnabledAt(gBmSt.playerCursor.x, gBmSt.playerCursor.y) ? 3 : 0);

    return;
}