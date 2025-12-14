#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

static char * fe8_characters[62] = {
    // Main Story Characters
    "Eirika", "Seth", "Franz", "Gilliam", "Moulder", "Vanessa", "Ross", "Garcia", "Neimi", "Colm", "Lute", "Artur",
    "Natasha", "Joshua", "Ephraim", "Forde", "Kyle", "Tana", "Amelia", "Duessel", "Cormag", "L'Arachel", "Dozla",
    "Ewan", "Marisa", "Tethys", "Gerik", "Rennac", "Saleh", "Knoll", "Innes", "Myrrh", "Syrene",

    // Bosses
    "O'Neill", "Berguet", "Bones", "Bazba", "Saar", "Novala", "Murray", "Tirado", "Binks", "Pablo", "Aias", "Carlyle",
    "Gheb", "Beran", "Zonta", "Vigarde",

    // Extras
    "Mansel", "Klimt", "Dara",

    // Post-Game Unlockable Characters
    "Caellach", "Orson", "Riev", "Ismaire", "Selena", "Hayden", "Glen", "Valter", "Fado", "Lyon"
};

/* Controls the drawing of the HP and / symbols in the minimug box */
LYN_REPLACE_CHECK(ClearUnitMapUiStatus);
void ClearUnitMapUiStatus(struct PlayerInterfaceProc* proc, u16* buffer, struct Unit* unit) {

    buffer[0] = TILEREF(0x120, 2);
    buffer[1] = TILEREF(0x121, 2);
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = TILEREF(0x13E, 2);
    buffer[5] = 0;
    buffer[6] = 0;

    return;
}

//! FE8U = 0x0808C45C
static inline void GetTwoDigits(int value, u8 *hi, u8 *lo, bool forceLeadingZero)
{
    if (value >= 100)
        StoreNumberStringOrDashesToSmallBuffer(0xFF);
    else
        StoreNumberStringOrDashesToSmallBuffer(value);

    *hi = gNumberStr[6] - '0';
    *lo = gNumberStr[7] - '0';

    if (forceLeadingZero && value < 10)
        *hi = 0;
}

static inline void DrawTwoDigits(int x, int y, u8 hi, u8 lo)
{
    if (hi != (u8)(' ' - '0'))
        CallARM_PushToSecondaryOAM(
            x, y, gObject_8x8,
            hi + OAM2_CHR(0x2E0) + OAM2_PAL(8));

    CallARM_PushToSecondaryOAM(
        x + 7, y, gObject_8x8,
        lo + OAM2_CHR(0x2E0) + OAM2_PAL(8));
}

LYN_REPLACE_CHECK(UnitMapUiUpdate);
void UnitMapUiUpdate(struct PlayerInterfaceProc *proc, struct Unit *unit)
{
    s16 frameCount = proc->unitClock;

    if (unit->statusIndex == UNIT_STATUS_RECOVER)
        frameCount = 0;

    /* Status flashing */
    if ((frameCount & 63) == 0)
    {
        if (frameCount & 64)
            PutUnitMapUiStatus(proc->statusTm, unit);
        else
        {
            GetTwoDigits(GetUnitCurrentHp(unit), &proc->hpCurHi, &proc->hpCurLo, false);
            GetTwoDigits(GetUnitMaxHp(unit),     &proc->hpMaxHi, &proc->hpMaxLo, false);
            ClearUnitMapUiStatus(proc, proc->statusTm, unit);
        }

        BG_EnableSyncByMask(BG0_SYNC_BIT);
    }

    /* Hide check */
    if (proc->hideContents ||
        ((frameCount & 64) && unit->statusIndex != UNIT_STATUS_NONE))
        return;

    /* Base positions */
    int xBase = proc->xHp * 8;
    int yBase = proc->yHp * 8;

    /* HP row */
    DrawTwoDigits(xBase + 17, yBase, proc->hpCurHi, proc->hpCurLo);
    DrawTwoDigits(xBase + 41, yBase, proc->hpMaxHi, proc->hpMaxLo);

    /* MP row (one tile below) */
    if (GetUnitMaxMP(unit) > 0)
    {
        u8 mpCurHi, mpCurLo, mpMaxHi, mpMaxLo;
        int mpY = yBase + 8;

        GetTwoDigits(GetUnitCurrentMP(unit), &mpCurHi, &mpCurLo, false);
        GetTwoDigits(GetUnitMaxMP(unit),     &mpMaxHi, &mpMaxLo, false);

        DrawTwoDigits(xBase + 17, mpY, mpCurHi, mpCurLo);
        DrawTwoDigits(xBase + 41, mpY, mpMaxHi, mpMaxLo);
    }
}

// ! FE8U = 0x0808C5D0
LYN_REPLACE_CHECK(DrawUnitMapUi);
void DrawUnitMapUi(struct PlayerInterfaceProc * proc, struct Unit * unit)
{
    char * str;
    int pos;
    int faceId;

    CpuFastFill(0, gUiTmScratchA, 6 * CHR_SIZE * sizeof(u16));

    str = GetStringFromIndex(unit->pCharacterData->nameTextId);

#if (defined(SID_IdentityProblems) && (COMMON_SKILL_VALID(SID_IdentityProblems)))
    if (SkillTester(unit, SID_IdentityProblems))
        str = fe8_characters[NextRN_N(sizeof(fe8_characters) / sizeof((fe8_characters)[0]))];
#endif

    pos = GetStringTextCenteredPos(56, str);

    ClearText(proc->texts);
    Text_SetParams(proc->texts, pos, TEXT_COLOR_SYSTEM_WHITE);
    Text_DrawString(proc->texts, str);
    PutText(proc->texts, gUiTmScratchA + TILEMAP_INDEX(5, 1));

    faceId = GetUnitMiniPortraitId(unit);

    if (unit->state & US_BIT23)
    {
        faceId = faceId + 1;
    }

    PutFaceChibi(faceId, gUiTmScratchA + TILEMAP_INDEX(1, 1), 0xF0, 4, 0);

    /* Display HP graphic in MMB */
    proc->statusTm = gUiTmScratchA + TILEMAP_INDEX(5, 3);
    proc->unitClock = 0;

    /* Add slash for MP one tile row down from HP */
    gUiTmScratchA[TILEMAP_INDEX(9, 4)] = TILEREF(0x13E, 2);
    /* ensure MP icon (same icon used for HP) is present one tile-row below */
    gUiTmScratchA[TILEMAP_INDEX(5, 4)] = TILEREF(0x160, 2);
    gUiTmScratchA[TILEMAP_INDEX(6, 4)] = TILEREF(0x161, 2);

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        proc->xHp = 5;
    }
    else
    {
        proc->xHp = 23;
    }

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0)
    {
        proc->yHp = 3;
    }
    else
    {
        proc->yHp = 17;
    }

    UnitMapUiUpdate(proc, unit);
    //DrawHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1));

    CallARM_FillTileRect(gUiTmScratchB, gTSA_MinimugBox, TILEREF(0x0, 3));
    ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);

    return;
}

//! FE8U = 0x0808BCF8
LYN_REPLACE_CHECK(MMB_Loop_SlideIn);
void MMB_Loop_SlideIn(struct PlayerInterfaceProc * proc)
{
    int tmIndex;
    int width;

    int y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        tmIndex = TILEMAP_INDEX(0, y);

        TileMap_FillRect(gBG0TilemapBuffer + tmIndex, 13, 6, 0);
        TileMap_FillRect(gBG1TilemapBuffer + tmIndex, 13, 6, 0);
    }
    else
    {
        tmIndex = TILEMAP_INDEX(0, y);

        TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, 13, 6, 0);
        TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, 13, 6, 0);
    }

    tmIndex = TILEMAP_INDEX(0, y);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    width = sMMBSlideInWidthLut[proc->showHideClock];

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        TileMap_CopyRect(gUiTmScratchA + (13 - width), gBG0TilemapBuffer + tmIndex, width, 6);
        TileMap_CopyRect(gUiTmScratchB + (13 - width), gBG1TilemapBuffer + tmIndex, width, 6);
    }
    else
    {
        TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, 6);
        TileMap_CopyRect(gUiTmScratchB, gBG1TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, 6);
    }

    proc->showHideClock++;

    if (proc->showHideClock == 4)
    {
        proc->hideContents = false;
        proc->showHideClock = 0;

        Proc_Break(proc);

        UnitMapUiUpdate(proc, GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x]));
    }

    return;
}