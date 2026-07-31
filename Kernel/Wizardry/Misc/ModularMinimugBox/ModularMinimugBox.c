#include "common-chax.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

extern u16 ModularMinimugBox_TileMap[];
extern const u16 gUiFramePaletteA[];
extern const u16 MenuTilesPalette_Gamma[];
extern const u16 MenuTilesPalette_Pikmin[];

int GetUIPalID(void);

/* OBJ palette 15 is reserved by the engine for grayed map sprites.
 * Keep the minimug digits on a free palette bank so acted player units
 * continue using the vanilla gray palette correctly. */
#define MMB_NUMBER_OBJ_PAL 11
#define MMBHeight 6
#define MMBWidth 16

static const char* fe8_names[] = {
    "Eirika", "Seth", "Franz", "Gilliam", "Moulder", "Vanessa", "Ross", "Garcia", "Neimi", "Colm", "Lute", "Artur",
    "Natasha", "Joshua", "Ephraim", "Forde", "Kyle", "Tana", "Amelia", "Duessel", "Cormag", "L'Arachel", "Dozla",
    "Ewan", "Marisa", "Tethys", "Gerik", "Rennac", "Saleh", "Knoll", "Innes", "Myrrh", "Syrene", "O'Neill", "Berguet", 
    "Bones", "Bazba", "Saar", "Novala", "Murray", "Tirado", "Binks", "Pablo", "Aias", "Carlyle", "Gheb", "Beran", 
    "Zonta", "Vigarde", "Mansel", "Klimt", "Dara", "Caellach", "Orson", "Riev", "Ismaire", "Selena", "Hayden", "Glen", 
    "Valter", "Fado", "Lyon"
};

static const s8 sMMBSlideInLut[2][4] = { {8, 12, 14, 16}, {7, 11, 14, 16} };
static const s8 sMMBSlideOutLut[3] = { 12, 8, 4 };

static void GetDigits(int val, u8* d) {
    StoreNumberStringOrDashesToSmallBuffer(val);
    for (int i = 0; i < 3; i++) d[i] = gNumberStr[i+5] - '0';
    if (val < 100) d[0] = 0;
    if (val < 10) d[1] = 0;
}

static void DrawDigits(int x, int y, int count, u8* d) {
    for (int i = 0; i < count; i++) {
        u16 tile = (count == 2 && d[0] > 0 && i < 2) ? 0x2EA : (d[i] + 0x2E0);
        CallARM_PushToSecondaryOAM(x + (i * 7), y, gObject_8x8, OAM2_CHR(tile) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
    }
}

static void ApplyMMBNumberPalette(void) {
    ApplyPalette(Pal_Text, 0x10 + MMB_NUMBER_OBJ_PAL);
}

static const u16 *GetMMBWindowPaletteRow(int uiPalId, int windowColor) {
    switch (uiPalId) {
    case 2:
        return &MenuTilesPalette_Gamma[windowColor * 0x10];

    case 3:
        return &MenuTilesPalette_Pikmin[windowColor * 0x10];

    default:
        return &gUiFramePaletteA[windowColor * 0x10];
    }
}

static int GetMMBUnitWindowColor(struct Unit *unit) {
    switch (UNIT_FACTION(unit)) {
    case FACTION_RED:
        return 0;

    case FACTION_BLUE:
        return 1;

    case FACTION_GREEN:
        return 2;

    case FACTION_PURPLE:
        return 3;

    default:
        return gPlaySt.config.windowColor;
    }
}

static void ApplyMMBFramePalette(struct Unit *unit, bool alt) {
    if (alt) {
        if (gpKernelDesignerConfig->vesly_custom_ui) {
            const u16 *basePalette = GetMMBWindowPaletteRow(GetUIPalID(), gPlaySt.config.windowColor);
            const u16 *fillPalette = GetMMBWindowPaletteRow(GetUIPalID(), GetMMBUnitWindowColor(unit));
            u16 blendedPalette[0x10];

            CpuFastCopy(basePalette, blendedPalette, 0x20);

            /* Keep the frame border entries from the active window color and only
             * retint the interior shades for the unit's allegiance. */
            for (int i = 5; i < 0x10; ++i)
                blendedPalette[i] = fillPalette[i];

            ApplyPalette(blendedPalette, 3);
            return;
        }

        UnpackUiFramePalette(3);
        return;
    }

    ApplyUnitMapUiFramePal(UNIT_FACTION(unit), 3);
}

static int GetMMBBaseX(struct PlayerInterfaceProc *proc) {
    return (sPlayerInterfaceConfigLut[proc->cursorQuadrant].xMinimug < 0) ? 0 : (30 - MMBWidth);
}

static void DrawLongHpBar(u16 *buffer, struct Unit *unit, int tileBase, int length)
{
    int i;
    int hpCurrent = GetUnitCurrentHp(unit);
    int hpMax = GetUnitMaxHp(unit);
    int hpPercent;
    int middleHp;
    int middleCount;

    if (length <= 0) {
        DrawHpBar(buffer, unit, tileBase);
        return;
    }

    if (hpCurrent < 0)
        hpCurrent = 0;
    if (hpMax <= 0)
        hpPercent = 0;
    else {
        if (hpCurrent > hpMax)
            hpCurrent = hpMax;
        hpPercent = 50 * hpCurrent / hpMax;
    }

    if (hpPercent > 50)
        hpPercent = 50;

    /* The vanilla bar reserves five percentage points for each cap. */
    buffer[0] = tileBase + (hpPercent > 5 ? 5 : hpPercent);

    /*
     * The modular layout has seven middle tiles.  Distribute the vanilla
     * middle range (5..45) across them instead of copying overlapping tiles;
     * this keeps the visual fill proportional at every HP value.
     */
    middleCount = 4 + length;
    middleHp = hpPercent - 5;
    if (middleHp < 0)
        middleHp = 0;
    if (middleHp > 40)
        middleHp = 40;

    for (i = 0; i < middleCount; ++i) {
        int fill = ((middleHp * middleCount - 40 * i) * 8) / 40;

        if (fill <= 0)
            buffer[1 + i] = tileBase + 6;
        else if (fill >= 8)
            buffer[1 + i] = tileBase + 14;
        else
            buffer[1 + i] = tileBase + 6 + fill;
    }

    hpPercent -= 45;
    if (hpPercent < 0)
        hpPercent = 0;
    if (hpPercent > 5)
        hpPercent = 5;
    buffer[middleCount + 1] = tileBase + 15 + hpPercent;
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
    // bool showMpNumbers = IsStatScreenPageAvailable(PAGE_GAIDEN_MAGIC);
    if (!UNIT_IS_VALID(unit))
        return;

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
    u8 hp[6];

    ApplyMMBNumberPalette();

    if (!alt) {
        // Legacy FE8 Logic: Cap at 99 visually
        GetDigits(GetUnitCurrentHp(unit) >= 100 ? 0xFF : GetUnitCurrentHp(unit), hp);
        GetDigits(GetUnitMaxHp(unit) >= 100 ? 0xFF : GetUnitMaxHp(unit), hp + 3);
        
        if (hp[1] != (u8)(' ' - '0')) CallARM_PushToSecondaryOAM(xb + 17, yb, gObject_8x8, hp[1] + OAM2_CHR(0x2E0) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
        CallARM_PushToSecondaryOAM(xb + 24, yb, gObject_8x8, hp[2] + OAM2_CHR(0x2E0) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
        
        if (hp[4] != (u8)(' ' - '0')) CallARM_PushToSecondaryOAM(xb + 41, yb, gObject_8x8, hp[4] + OAM2_CHR(0x2E0) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
        CallARM_PushToSecondaryOAM(xb + 48, yb, gObject_8x8, hp[5] + OAM2_CHR(0x2E0) + OAM2_PAL(MMB_NUMBER_OBJ_PAL));
        return;
    }

    bool exp = gpKernelDesignerConfig->expanded_hp;
    int curX = xb + (exp ? 11 : 18), maxX = xb + 41;
    
    GetDigits(GetUnitCurrentHp(unit), hp); GetDigits(GetUnitMaxHp(unit), hp + 3);
    DrawDigits(curX, yb, exp ? 3 : 2, hp + (exp ? 0 : 1)); 
    DrawDigits(maxX, yb, exp ? 3 : 2, hp + 3 + (exp ? 0 : 1));

    // if (!showMpNumbers)
    //     return;
    //
    // GetDigits(GetUnitCurrentMP(unit), mp); GetDigits(GetUnitMaxMP(unit), mp + 3);
    // DrawDigits(curX, yb + 8, exp ? 3 : 2, mp + (exp ? 0 : 1));
    // DrawDigits(maxX, yb + 8, exp ? 3 : 2, mp + 3 + (exp ? 0 : 1));
}

LYN_REPLACE_CHECK(DrawUnitMapUi);
void DrawUnitMapUi(struct PlayerInterfaceProc* proc, struct Unit* unit) {
    bool alt = gpKernelDesignerConfig->mp_system;
    // bool showMpNumbers = IsStatScreenPageAvailable(PAGE_GAIDEN_MAGIC);
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
        ApplyMMBFramePalette(unit, alt);
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
    // if (!alt || !showMpNumbers)
    //     DrawLongHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1), 3);
    DrawLongHpBar(gUiTmScratchA + TILEMAP_INDEX(5, 4), unit, TILEREF(0x140, 1), 3);
    CallARM_FillTileRect(gUiTmScratchB, alt ? ModularMinimugBox_TileMap : gTSA_MinimugBox, TILEREF(0x0, 3));
    ApplyMMBFramePalette(unit, alt);
}

static bool IsUnitFogStage2(struct Unit* cu) {
    return UNIT_IS_VALID(cu) && gpKernelDesignerConfig->multiple_fog_stages
        && gPlaySt.chapterVisionRange && gBmMapFog[cu->yPos][cu->xPos] == 1;
}

static struct Unit *GetMMBCursorUnit(void) {
    int unitId = gBmMapUnit[gBmSt.playerCursor.y][gBmSt.playerCursor.x];
    return unitId ? GetUnit(unitId) : NULL;
}

static void MMB_Slide_Common(struct PlayerInterfaceProc* proc, bool out) {
    bool exp = gpKernelDesignerConfig->expanded_hp;
    int y = sPlayerInterfaceConfigLut[proc->cursorQuadrant].yMinimug < 0 ? 0 : 14;
    int baseX = GetMMBBaseX(proc);
    bool left = (baseX == 0);
    int width = out ? sMMBSlideOutLut[proc->showHideClock] : sMMBSlideInLut[!exp][proc->showHideClock];

    if (out) {
        proc->hideContents = true;
    } else if (proc->showHideClock == 0) {
        /* On the first slide-in frame, sync hideContents to the incoming unit's fog
         * state.  The slide-out always leaves hideContents=true; without this reset,
         * visible units keep it true through the whole slide (Bug 2: HP/MP never
         * shown during the slide-in after a fog→visible switch).  Conversely, fog
         * units must keep it true so the OAM digit guard in UnitMapUiUpdate has a
         * second line of defence (Bug 1: HP/slash bleeding onto fog unit boxes). */
        struct Unit* cu = GetMMBCursorUnit();
        proc->hideContents = IsUnitFogStage2(cu);
    }

    TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(baseX, y), MMBWidth, MMBHeight, 0);
    TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(baseX, y), MMBWidth, MMBHeight, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    u16 *srcA = gUiTmScratchA, *srcB = gUiTmScratchB;
    int destX = left ? 0 : 30 - width;
    if (left) { srcA += (MMBWidth - width); srcB += (MMBWidth - width); }

    TileMap_CopyRect(srcA, gBG0TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMBHeight);
    TileMap_CopyRect(srcB, gBG1TilemapBuffer + TILEMAP_INDEX(destX, y), width, MMBHeight);

    if (++proc->showHideClock == (out ? 3 : 4)) {
        proc->showHideClock = 0;
        if (out) {
            TileMap_FillRect(gBG0TilemapBuffer + TILEMAP_INDEX(baseX, y), MMBWidth, MMBHeight, 0);
            TileMap_FillRect(gBG1TilemapBuffer + TILEMAP_INDEX(baseX, y), MMBWidth, MMBHeight, 0);
            BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
            proc->isRetracting = false;
            proc->windowQuadrant = -1;
        }
        else {
            struct Unit* cu = GetMMBCursorUnit();
            proc->hideContents = IsUnitFogStage2(cu);
            UnitMapUiUpdate(proc, cu);
        }
        Proc_Break(proc);
    } else if (!out && !proc->hideContents) {
        /* Push HP/MP OAM digits on each intermediate slide-in frame so the values
         * are visible throughout the slide, not only on the final frame. */
        UnitMapUiUpdate(proc, GetMMBCursorUnit());
    }
}

LYN_REPLACE_CHECK(MMB_Loop_SlideIn);
void MMB_Loop_SlideIn(struct PlayerInterfaceProc* proc) { MMB_Slide_Common(proc, false); }

LYN_REPLACE_CHECK(MMB_Loop_SlideOut);
void MMB_Loop_SlideOut(struct PlayerInterfaceProc* proc) { MMB_Slide_Common(proc, true); }
