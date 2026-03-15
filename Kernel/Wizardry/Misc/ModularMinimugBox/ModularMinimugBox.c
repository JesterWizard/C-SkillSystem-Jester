#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

extern u16 ModularMinimugBox_TileMap[];
#define MMBHeight 6
#define MMBWidth 14

static const char* fe8_names[] = {
    "Eirika", "Seth", "Franz", "Gilliam", "Moulder", "Vanessa", "Ross", "Garcia", "Neimi", "Colm", "Lute", "Artur",
    "Natasha", "Joshua", "Ephraim", "Forde", "Kyle", "Tana", "Amelia", "Duessel", "Cormag", "L'Arachel", "Dozla",
    "Ewan", "Marisa", "Tethys", "Gerik", "Rennac", "Saleh", "Knoll", "Innes", "Myrrh", "Syrene", "O'Neill", "Berguet", 
    "Bones", "Bazba", "Saar", "Novala", "Murray", "Tirado", "Binks", "Pablo", "Aias", "Carlyle", "Gheb", "Beran", 
    "Zonta", "Vigarde", "Mansel", "Klimt", "Dara", "Caellach", "Orson", "Riev", "Ismaire", "Selena", "Hayden", "Glen", 
    "Valter", "Fado", "Lyon"
};

static const s8 sMMBSlideInLut[2][4] = { {6, 10, 13, 14}, {5, 9, 11, 13} };

static void GetDigits(int val, u8* d) {
    StoreNumberStringOrDashesToSmallBuffer(val);
    for (int i = 0; i < 3; i++) d[i] = gNumberStr[i+5] - '0';
    if (val < 100) d[0] = 0;
    if (val < 10) d[1] = 0;
}

static void DrawDigits(int x, int y, int count, u8* d) {
    for (int i = 0; i < count; i++) {
        u16 tile = (count == 2 && d[0] > 0 && i < 2) ? 0x2EA : (d[i] + 0x2E0);
        CallARM_PushToSecondaryOAM(x + (i * 7), y, gObject_8x8, OAM2_CHR(tile) + OAM2_PAL(8));
    }
}

LYN_REPLACE_CHECK(ClearUnitMapUiStatus);
void ClearUnitMapUiStatus(struct PlayerInterfaceProc* proc, u16* buffer, struct Unit* unit) {
    bool alt = gpKernelDesignerConfig->mp_system;
    buffer[0] = TILEREF(0x120, 2); buffer[1] = TILEREF(0x121, 2);
    buffer[2] = buffer[3] = buffer[alt ? 4 : 5] = buffer[6] = 0;
    buffer[alt ? 5 : 4] = TILEREF(0x13E, 2);
}

LYN_REPLACE_CHECK(UnitMapUiUpdate);
void UnitMapUiUpdate(struct PlayerInterfaceProc* proc, struct Unit* unit) {
    bool alt = gpKernelDesignerConfig->mp_system;
    if (unit->statusIndex == UNIT_STATUS_RECOVER) proc->unitClock = 0;

/* Stage 2: suppress HP text, slash, and digit OAM entirely */
    if (gpKernelDesignerConfig->multiple_fog_stages == true
            && gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] == 1)
        return;

    if ((proc->unitClock & 63) == 0) {
        (proc->unitClock & 64) ? PutUnitMapUiStatus(proc->statusTm, unit) : ClearUnitMapUiStatus(proc, proc->statusTm, unit);
        BG_EnableSyncByMask(BG0_SYNC_BIT);
    }

    if (proc->hideContents || ((proc->unitClock & 64) && unit->statusIndex != UNIT_STATUS_NONE)) return;

    int xb = proc->xHp * 8, yb = proc->yHp * 8;
    u8 hp[6], mp[6];

    if (!alt) {
        // Legacy FE8 Logic: Cap at 99 visually
        GetDigits(GetUnitCurrentHp(unit) >= 100 ? 0xFF : GetUnitCurrentHp(unit), hp);
        GetDigits(GetUnitMaxHp(unit) >= 100 ? 0xFF : GetUnitMaxHp(unit), hp + 3);
        
        if (hp[1] != (u8)(' ' - '0')) CallARM_PushToSecondaryOAM(xb + 17, yb, gObject_8x8, hp[1] + OAM2_CHR(0x2E0) + OAM2_PAL(8));
        CallARM_PushToSecondaryOAM(xb + 24, yb, gObject_8x8, hp[2] + OAM2_CHR(0x2E0) + OAM2_PAL(8));
        
        if (hp[4] != (u8)(' ' - '0')) CallARM_PushToSecondaryOAM(xb + 41, yb, gObject_8x8, hp[4] + OAM2_CHR(0x2E0) + OAM2_PAL(8));
        CallARM_PushToSecondaryOAM(xb + 48, yb, gObject_8x8, hp[5] + OAM2_CHR(0x2E0) + OAM2_PAL(8));
        return;
    }

    bool exp = gpKernelDesignerConfig->expanded_hp;
    int curX = xb + (exp ? 11 : 18), maxX = xb + 41;
    
    GetDigits(GetUnitCurrentHp(unit), hp); GetDigits(GetUnitMaxHp(unit), hp + 3);
    DrawDigits(curX, yb, exp ? 3 : 2, hp + (exp ? 0 : 1)); 
    DrawDigits(maxX, yb, exp ? 3 : 2, hp + 3 + (exp ? 0 : 1));
    
    GetDigits(GetUnitCurrentMP(unit), mp); GetDigits(GetUnitMaxMP(unit), mp + 3);
    DrawDigits(curX, yb + 8, exp ? 3 : 2, mp + (exp ? 0 : 1)); 
    DrawDigits(maxX, yb + 8, exp ? 3 : 2, mp + 3 + (exp ? 0 : 1));
}

LYN_REPLACE_CHECK(DrawUnitMapUi);
void DrawUnitMapUi(struct PlayerInterfaceProc* proc, struct Unit* unit) {
    bool alt = gpKernelDesignerConfig->mp_system;
    bool exp = gpKernelDesignerConfig->expanded_hp;
    CpuFastFill(0, gUiTmScratchA, 6 * CHR_SIZE * sizeof(u16));

    if (gpKernelDesignerConfig->multiple_fog_stages == true
            && gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] == 1) {
        proc->hideContents = true;
        /* Point statusTm into the already-zeroed scratch buffer so any stale
         * proc->unitClock tick that escapes our UnitMapUiUpdate guard writes
         * to safe memory rather than leftover data from a previous unit. */
        proc->statusTm = gUiTmScratchA + TILEMAP_INDEX(5, 3);
        proc->unitClock = 0;
        CallARM_FillTileRect(gUiTmScratchB, alt ? ModularMinimugBox_TileMap : gTSA_MinimugBox, TILEREF(0x0, 3));
        ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);
        return;
    }

    char* str = GetStringFromIndex(unit->pCharacterData->nameTextId);
#if (defined(SID_IdentityProblems) && (COMMON_SKILL_VALID(SID_IdentityProblems)))
    if (SkillTester(unit, SID_IdentityProblems)) str = (char*)fe8_names[NextRN_N(62)];
#endif

    ClearText(proc->texts); // Fix for garbled name
    Text_SetParams(proc->texts, GetStringTextCenteredPos(56, str), alt ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_BLACK);
    Text_DrawString(proc->texts, str);
    PutText(proc->texts, gUiTmScratchA + TILEMAP_INDEX(5, 1));
    PutFaceChibi(GetUnitMiniPortraitId(unit) + (unit->state & US_BIT23 ? 1 : 0), gUiTmScratchA + TILEMAP_INDEX(1, 1), 0xF0, 4, 0);

    if (alt) {
        gUiTmScratchA[TILEMAP_INDEX(5, 3)] = TILEREF(0x120, 2); gUiTmScratchA[TILEMAP_INDEX(6, 3)] = TILEREF(0x121, 2);
        gUiTmScratchA[TILEMAP_INDEX(5, 4)] = TILEREF(0x160, 2); gUiTmScratchA[TILEMAP_INDEX(6, 4)] = TILEREF(0x161, 2);
        gUiTmScratchA[TILEMAP_INDEX(exp ? 10 : 9, 3)] = gUiTmScratchA[TILEMAP_INDEX(exp ? 10 : 9, 4)] = TILEREF(0x13E, 2);
    }

    proc->statusTm = gUiTmScratchA + TILEMAP_INDEX(5, 3);
    proc->unitClock = 0;
    proc->xHp = (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0) ? (alt && exp ? 6 : 5) : 24;
    proc->yHp = (sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0) ? 3 : 17;

    UnitMapUiUpdate(proc, unit);
    if (!alt) DrawHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1));
    CallARM_FillTileRect(gUiTmScratchB, alt ? ModularMinimugBox_TileMap : gTSA_MinimugBox, TILEREF(0x0, 3));
    ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);
}

static void MMB_Slide_Common(struct PlayerInterfaceProc* proc, bool out) {
    bool alt = gpKernelDesignerConfig->mp_system;
    bool exp = gpKernelDesignerConfig->expanded_hp;
    int adj = (!alt || exp) ? 0 : 1;
    int y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;
    int xOff = sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0 ? 0 : 17;
    int width = out ? sMMBSlideOutWidthLut[proc->showHideClock] : sMMBSlideInLut[!exp][proc->showHideClock];

    if (out) proc->hideContents = true;
    TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(xOff, y), MMBWidth - adj, MMBHeight, 0);
    TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(xOff, y), MMBWidth - adj, MMBHeight, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    u16 *srcA = gUiTmScratchA, *srcB = gUiTmScratchB;
    int destX = (xOff == 0) ? 0 : 30 - width;
    if (xOff == 0) { srcA += (MMBWidth - width - adj); srcB += (MMBWidth - width - adj); }

    TileMap_CopyRect(srcA, gBG0TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMBHeight);
    TileMap_CopyRect(srcB, gBG1TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMBHeight);

    if (++proc->showHideClock == (out ? 3 : 4)) {
        proc->showHideClock = 0;
        if (out) { proc->isRetracting = false; proc->windowQuadrant = -1; } 
        else { proc->hideContents = false; UnitMapUiUpdate(proc, GetUnit(gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x])); }
        Proc_Break(proc);
    }
}

LYN_REPLACE_CHECK(MMB_Loop_SlideIn);
void MMB_Loop_SlideIn(struct PlayerInterfaceProc* proc) { MMB_Slide_Common(proc, false); }

LYN_REPLACE_CHECK(MMB_Loop_SlideOut);
void MMB_Loop_SlideOut(struct PlayerInterfaceProc* proc) { MMB_Slide_Common(proc, true); }