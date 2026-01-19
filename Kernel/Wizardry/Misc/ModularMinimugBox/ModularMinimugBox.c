#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

extern u16 ModularMinimugBox_TileMap[];

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
    buffer[4] = 0;
    buffer[5] = TILEREF(0x13E, 2);
    buffer[6] = 0;

    return;
}

//! FE8U = 0x0808C45C
static inline void GetThreeDigits(int value, u8 *hundreds, u8 *tens, u8 *ones, bool forceHundredsZero, bool forceTensZero)
{
    StoreNumberStringOrDashesToSmallBuffer(value);

    *hundreds = gNumberStr[5] - '0';
    *tens     = gNumberStr[6] - '0';
    *ones     = gNumberStr[7] - '0';

    if (forceHundredsZero && value < 100)
        *hundreds = 0;

    if (forceTensZero && value < 10)
        *tens = 0;
}

static inline void DrawThreeDigits(int x, int y, u8 h, u8 t, u8 o)
{
    if (h != (u8)(' ' - '0'))
        CallARM_PushToSecondaryOAM(
            x, y, gObject_8x8,
            h + OAM2_CHR(0x2E0) + OAM2_PAL(8));

    if (t != (u8)(' ' - '0'))
        CallARM_PushToSecondaryOAM(
            x + 7, y, gObject_8x8,
            t + OAM2_CHR(0x2E0) + OAM2_PAL(8));

    CallARM_PushToSecondaryOAM(
        x + 14, y, gObject_8x8,
        o + OAM2_CHR(0x2E0) + OAM2_PAL(8));
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
            ClearUnitMapUiStatus(proc, proc->statusTm, unit);

        BG_EnableSyncByMask(BG0_SYNC_BIT);
    }

    /* Hide check */
    if (proc->hideContents ||
        ((frameCount & 64) && unit->statusIndex != UNIT_STATUS_NONE))
        return;

    int xBase = proc->xHp * 8;
    int yBase = proc->yHp * 8;

    /* ---------------- HP ---------------- */
    u8 hpCurH, hpCurT, hpCurO;
    u8 hpMaxH, hpMaxT, hpMaxO;

    GetThreeDigits(GetUnitCurrentHp(unit),
        &hpCurH, &hpCurT, &hpCurO,
        true,   /* force 0 if < 100 */
        true);  /* force 0 if < 10 */

    GetThreeDigits(GetUnitMaxHp(unit),
        &hpMaxH, &hpMaxT, &hpMaxO,
        true,
        true);

    DrawThreeDigits(xBase + 11, yBase, hpCurH, hpCurT, hpCurO);
    DrawThreeDigits(xBase + 41, yBase, hpMaxH, hpMaxT, hpMaxO);

    /* ---------------- MP ---------------- */
    int mpY = yBase + 8;
    u8 mpCurH, mpCurT, mpCurO;
    u8 mpMaxH, mpMaxT, mpMaxO;

    GetThreeDigits(GetUnitCurrentMP(unit),
        &mpCurH, &mpCurT, &mpCurO,
        true,   /* force 0 if < 100 */
        true);  /* force 0 if < 10 */

    GetThreeDigits(GetUnitMaxMP(unit),
        &mpMaxH, &mpMaxT, &mpMaxO,
        true,
        true);

    DrawThreeDigits(xBase + 11, mpY, mpCurH, mpCurT, mpCurO);
    DrawThreeDigits(xBase + 41, mpY, mpMaxH, mpMaxT, mpMaxO);
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
        faceId = faceId + 1;

    PutFaceChibi(faceId, gUiTmScratchA + TILEMAP_INDEX(1, 1), 0xF0, 4, 0);

    /* Display HP slash graphic in MMB */
    proc->statusTm = gUiTmScratchA + TILEMAP_INDEX(5, 3);
    proc->unitClock = 0;

    /* Add slash for MP one tile row down from HP */
    gUiTmScratchA[TILEMAP_INDEX(10, 4)] = TILEREF(0x13E, 2);
    /* ensure MP icon (same icon used for HP) is present one tile-row below */
    gUiTmScratchA[TILEMAP_INDEX(5, 4)] = TILEREF(0x160, 2);
    gUiTmScratchA[TILEMAP_INDEX(6, 4)] = TILEREF(0x161, 2);

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
        proc->xHp = 6;
    else
        proc->xHp = 24;

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0)
        proc->yHp = 3;
    else
        proc->yHp = 17;

    UnitMapUiUpdate(proc, unit);
    //DrawHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1));

    // CallARM_FillTileRect(gUiTmScratchB, gTSA_MinimugBox, TILEREF(0x0, 3));
    CallARM_FillTileRect(gUiTmScratchB, ModularMinimugBox_TileMap, TILEREF(0x0, 3));
    ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);

    return;
}

#define MMBHeight 6
#define MMBWidth 14

static const s8 sMMBSlideInWidthLut_NEW[4] =
{
    6, 10, 13, MMBWidth
};

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

        TileMap_FillRect(gBG0TilemapBuffer + tmIndex, MMBWidth, MMBHeight, 0);
        TileMap_FillRect(gBG1TilemapBuffer + tmIndex, MMBWidth, MMBHeight, 0);
    }
    else
    {
        tmIndex = TILEMAP_INDEX(0, y);

        TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, MMBWidth, MMBHeight, 0);
        TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, MMBWidth, MMBHeight, 0);
    }

    tmIndex = TILEMAP_INDEX(0, y);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    width = sMMBSlideInWidthLut_NEW[proc->showHideClock];

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        TileMap_CopyRect(gUiTmScratchA + (MMBWidth - width), gBG0TilemapBuffer + tmIndex, width, MMBHeight);
        TileMap_CopyRect(gUiTmScratchB + (MMBWidth - width), gBG1TilemapBuffer + tmIndex, width, MMBHeight);
    }
    else
    {
        TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, MMBHeight);
        TileMap_CopyRect(gUiTmScratchB, gBG1TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, MMBHeight);
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

//! FE8U = 0x0808BE70
LYN_REPLACE_CHECK(MMB_Loop_SlideOut);
void MMB_Loop_SlideOut(struct PlayerInterfaceProc * proc)
{
    int tmIndex;
    int width;

    int y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;

    proc->hideContents = true;

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        tmIndex = TILEMAP_INDEX(0, y);

        TileMap_FillRect(gBG0TilemapBuffer + tmIndex, MMBWidth, MMBHeight, 0);
        TileMap_FillRect(gBG1TilemapBuffer + tmIndex, MMBWidth, MMBHeight, 0);
    }
    else
    {
        tmIndex = TILEMAP_INDEX(0, y);

        TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, MMBWidth, MMBHeight, 0);
        TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(17, 0) + tmIndex, MMBWidth, MMBHeight, 0);
    }

    tmIndex = TILEMAP_INDEX(0, y);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    width = sMMBSlideOutWidthLut[proc->showHideClock];

    if (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0)
    {
        TileMap_CopyRect(gUiTmScratchA + (MMBWidth - width), gBG0TilemapBuffer + tmIndex, width, MMBHeight);
        TileMap_CopyRect(gUiTmScratchB + (MMBWidth - width), gBG1TilemapBuffer + tmIndex, width, MMBHeight);
    }
    else
    {
        TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, MMBHeight);
        TileMap_CopyRect(gUiTmScratchB, gBG1TilemapBuffer + TILEMAP_INDEX(30 - width, y), width, MMBHeight);
    }

    proc->showHideClock++;

    if (proc->showHideClock == 3)
    {
        proc->isRetracting = false;
        proc->showHideClock = 0;
        proc->windowQuadrant = -1;

        Proc_Break(proc);
    }

    return;
}