#include "common-chax.h"
#include "constants/texts.h"
#include "kernel/help-box.h"
#include "kernel-lib.h"
#include "status-getter.h"
#include "jester_headers/custom-arrays.h"

enum StartMapEffectKind {
    START_MAP_EFFECT_KIND_STAT_PERCENT = 0,
    START_MAP_EFFECT_KIND_STAT_FLAT = 1,
    START_MAP_EFFECT_KIND_MOV_DELTA = 2,
};

enum StartMapEffectPromptState {
    START_MAP_EFFECT_PROMPT_IDLE = 0,
    START_MAP_EFFECT_PROMPT_OPEN = 1,
    START_MAP_EFFECT_PROMPT_DONE = 2,
};

enum {
    START_MAP_EFFECT_NONE = 0xFF,
    START_MAP_EFFECT_DEFAULT_UNLOCK_MASK = 0x01,
    START_MAP_EFFECT_VISIBLE_ROWS = 5,
};

enum {
    START_MAP_EFFECT_FRAME_X = 7,
    START_MAP_EFFECT_FRAME_Y = 5,
    START_MAP_EFFECT_FRAME_W = 18,
    START_MAP_EFFECT_FRAME_H = 12,
    START_MAP_EFFECT_SCROLLBAR_X = ((START_MAP_EFFECT_FRAME_X + START_MAP_EFFECT_FRAME_W) * 8) - 7,
    START_MAP_EFFECT_SCROLLBAR_Y = ((START_MAP_EFFECT_FRAME_Y + 1) * 8) + 7,
};

struct StartMapEffectDef {
    const char *label;
    u16 rTextMsgId;
    u8 kind;
    s8 value;
    u8 turns;
    u8 targetFaction;
};

struct StartMapEffectsSuspendState {
    u8 chapterIndex;
    u8 promptState;
    u8 activeEffectId;
    u8 turnsRemaining;
};

struct StartMapEffectsPromptProc {
    PROC_HEADER;
    u8 curIndex;
    u8 topVisibleIndex;
    u8 itemCount;
    u8 helpOpen;
    u8 bg0Priority;
    u8 bg1Priority;
    u8 bg2Priority;
};

extern u8 gStartMapEffectsUnlockMask;
extern struct Text sStartMapEffectsTexts[START_MAP_EFFECT_VISIBLE_ROWS];
extern struct StartMapEffectsSuspendState sStartMapEffectsSuspendState;
static const char sStartMapEffectsNoEffectLabel[] = "No Effect";

#define START_MAP_EFFECT_ENTRY(label, rTextMsgId, kind, value, turns, targetFaction) \
    { label, rTextMsgId, kind, value, turns, targetFaction }

static const struct StartMapEffectDef sStartMapEffectDefs[] = {
    START_MAP_EFFECT_ENTRY("Rally", MSG_StartMapEffects_Rally_DESC, START_MAP_EFFECT_KIND_STAT_PERCENT, 10, 2, FACTION_BLUE),
    START_MAP_EFFECT_ENTRY("Drag", MSG_StartMapEffects_Drag_DESC, START_MAP_EFFECT_KIND_MOV_DELTA, -2, 2, FACTION_RED),
    START_MAP_EFFECT_ENTRY("Advance", MSG_StartMapEffects_Advance_DESC, START_MAP_EFFECT_KIND_MOV_DELTA, 1, 3, FACTION_BLUE),
    START_MAP_EFFECT_ENTRY("Pressure", MSG_StartMapEffects_Pressure_DESC, START_MAP_EFFECT_KIND_STAT_PERCENT, -10, 2, FACTION_RED),
    START_MAP_EFFECT_ENTRY("Bulwark", MSG_StartMapEffects_Bulwark_DESC, START_MAP_EFFECT_KIND_STAT_FLAT, 2, 1, FACTION_BLUE),
    START_MAP_EFFECT_ENTRY("Collapse", MSG_StartMapEffects_Collapse_DESC, START_MAP_EFFECT_KIND_STAT_FLAT, -2, 2, FACTION_RED),
};

#undef START_MAP_EFFECT_ENTRY

static void StartMapEffects_ClearSuspendState(void)
{
    sStartMapEffectsSuspendState.chapterIndex = 0xFF;
    sStartMapEffectsSuspendState.promptState = START_MAP_EFFECT_PROMPT_IDLE;
    sStartMapEffectsSuspendState.activeEffectId = START_MAP_EFFECT_NONE;
    sStartMapEffectsSuspendState.turnsRemaining = 0;
}

static void StartMapEffects_ResetChapterState(void)
{
    if (sStartMapEffectsSuspendState.chapterIndex == gPlaySt.chapterIndex)
        return;

    StartMapEffects_ClearSuspendState();
    sStartMapEffectsSuspendState.chapterIndex = gPlaySt.chapterIndex;
}

static inline bool StartMapEffectsEffectValid(int effectId)
{
    return effectId >= 0 && effectId < (int)ARRAY_COUNT(sStartMapEffectDefs);
}

static inline const struct StartMapEffectDef *GetStartMapEffectDef(int effectId)
{
    if (!StartMapEffectsEffectValid(effectId))
        return NULL;

    return &sStartMapEffectDefs[effectId];
}

static inline int StartMapEffects_GetEffectCount(void)
{
    return (int)ARRAY_COUNT(sStartMapEffectDefs);
}

static inline bool StartMapEffectsUnitMatches(const struct Unit *unit, const struct StartMapEffectDef *def)
{
    return unit && UNIT_FACTION(unit) == def->targetFaction;
}

static inline int StartMapEffects_GetItemCount(void)
{
    return StartMapEffects_GetEffectCount() + 1;
}

static inline const char *StartMapEffects_GetLabel(int itemNumber)
{
    if (itemNumber == 0)
        return sStartMapEffectsNoEffectLabel;

    return sStartMapEffectDefs[itemNumber - 1].label;
}

static inline int StartMapEffects_GetRText(int itemNumber)
{
    if (itemNumber == 0)
        return MSG_StartMapEffects_NoEffect_DESC;

    return sStartMapEffectDefs[itemNumber - 1].rTextMsgId;
}

static inline int StartMapEffects_GetTextColor(int itemNumber)
{
    if (itemNumber == 0)
        return TEXT_COLOR_SYSTEM_WHITE;

    return TEXT_COLOR_SYSTEM_WHITE;
}

static inline int StartMapEffects_GetSelectionColor(int itemNumber, int currentIndex)
{
    if (itemNumber == currentIndex)
        return TEXT_COLOR_SYSTEM_BLUE;

    return StartMapEffects_GetTextColor(itemNumber);
}

static void StartMapEffectsPrompt_UpdateCursorHand(struct StartMapEffectsPromptProc *proc)
{
    ShowSysHandCursor(
        START_MAP_EFFECT_FRAME_X * 8,
        (START_MAP_EFFECT_FRAME_Y * 8) + 8 + ((proc->curIndex - proc->topVisibleIndex) * 16),
        15,
        0x800
    );
}

static void StartMapEffectsPrompt_DrawFrame(void)
{
    DrawUiFrame(gBG2TilemapBuffer, START_MAP_EFFECT_FRAME_X - 1, START_MAP_EFFECT_FRAME_Y, START_MAP_EFFECT_FRAME_W, START_MAP_EFFECT_FRAME_H, 0, 0);
}

static void StartMapEffectsPrompt_LoadHelpBoxGfx(void)
{
    LoadHelpBoxGfx(NULL, 0xE);
}

static bool StartMapEffectsPrompt_IsHelpBoxOpen(void)
{
    return Proc_Find(ProcScr_Helpbox_bug_08A01678) || Proc_Find(gProcScr_HelpBox);
}

static void StartMapEffectsPrompt_UpdateHelpBox(struct StartMapEffectsPromptProc *proc)
{
    int rowY = (START_MAP_EFFECT_FRAME_Y * 8) + 8 + ((proc->curIndex - proc->topVisibleIndex) * 16);

    StartMapEffectsPrompt_LoadHelpBoxGfx();
    StartHelpBox((START_MAP_EFFECT_FRAME_X + 1) * 8 + 4, rowY, StartMapEffects_GetRText(proc->curIndex));
}

static void StartMapEffectsPrompt_ClearUi(void)
{
    TileMap_FillRect(
        TILEMAP_LOCATED(gBG0TilemapBuffer, START_MAP_EFFECT_FRAME_X, START_MAP_EFFECT_FRAME_Y),
        START_MAP_EFFECT_FRAME_W,
        START_MAP_EFFECT_FRAME_H,
        0
    );

    TileMap_FillRect(
        TILEMAP_LOCATED(gBG2TilemapBuffer, START_MAP_EFFECT_FRAME_X, START_MAP_EFFECT_FRAME_Y),
        START_MAP_EFFECT_FRAME_W,
        START_MAP_EFFECT_FRAME_H,
        0
    );
}

static void StartMapEffectsPrompt_ApplyBgPriority(struct StartMapEffectsPromptProc *proc)
{
    proc->bg0Priority = gLCDControlBuffer.bg0cnt.priority;
    proc->bg1Priority = gLCDControlBuffer.bg1cnt.priority;
    proc->bg2Priority = gLCDControlBuffer.bg2cnt.priority;

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 1;
    gLCDControlBuffer.bg2cnt.priority = 2;
}

static void StartMapEffectsPrompt_RestoreBgPriority(struct StartMapEffectsPromptProc *proc)
{
    gLCDControlBuffer.bg0cnt.priority = proc->bg0Priority;
    gLCDControlBuffer.bg1cnt.priority = proc->bg1Priority;
    gLCDControlBuffer.bg2cnt.priority = proc->bg2Priority;
}

static void StartMapEffectsPrompt_HideUnitSprites(void)
{
    int i;

    for (i = 1; i < 0xC0; ++i) {
        struct Unit *unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        unit->state |= US_HIDDEN;
        HideUnitSprite(unit);
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();
}

static void StartMapEffectsPrompt_ShowUnitSprites(void)
{
    int i;

    for (i = 1; i < 0xC0; ++i) {
        struct Unit *unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        unit->state &= ~US_HIDDEN;
        ShowUnitSprite(unit);
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();
}

static int StartMapEffects_ApplyStatEffect(int status, struct Unit *unit)
{
    if (gpKernelDesignerConfig->start_map_effects != true)
        return status;

    const struct StartMapEffectDef *def = GetStartMapEffectDef(sStartMapEffectsSuspendState.activeEffectId);

    if (!def || !StartMapEffectsUnitMatches(unit, def))
        return status;

    switch (def->kind) {
    case START_MAP_EFFECT_KIND_STAT_PERCENT:
        status += Div(status * def->value, 100);
        break;

    case START_MAP_EFFECT_KIND_STAT_FLAT:
        status += def->value;
        break;

    default:
        return status;
    }

    if (status < 0)
        status = 0;

    return status;
}

static int StartMapEffects_ApplyMovEffect(int status, struct Unit *unit)
{
    if (gpKernelDesignerConfig->start_map_effects != true)
        return status;

    const struct StartMapEffectDef *def = GetStartMapEffectDef(sStartMapEffectsSuspendState.activeEffectId);

    if (!def || def->kind != START_MAP_EFFECT_KIND_MOV_DELTA || !StartMapEffectsUnitMatches(unit, def))
        return status;

    status += def->value;

    if (status < 0)
        status = 0;

    return status;
}

static void StartMapEffectsPrompt_Finish(struct StartMapEffectsPromptProc *proc, int effectId)
{
    if (effectId < 0 || !StartMapEffectsEffectValid(effectId)) {
        sStartMapEffectsSuspendState.activeEffectId = START_MAP_EFFECT_NONE;
        sStartMapEffectsSuspendState.turnsRemaining = 0;
    } else {
        sStartMapEffectsSuspendState.activeEffectId = effectId;
        sStartMapEffectsSuspendState.turnsRemaining = sStartMapEffectDefs[effectId].turns;
    }

    sStartMapEffectsSuspendState.promptState = START_MAP_EFFECT_PROMPT_DONE;
    sStartMapEffectsSuspendState.chapterIndex = gPlaySt.chapterIndex;

    StartMapEffectsPrompt_ClearUi();
    EndMenuScrollBar();
    StartMapEffectsPrompt_RestoreBgPriority(proc);
    StartMapEffectsPrompt_ShowUnitSprites();
    CloseHelpBox();
    HideSysHandCursor();
    BG_Fill(gBG2TilemapBuffer, 0);
    LoadObjUIGfx();
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

static void StartMapEffectsPrompt_Draw(struct StartMapEffectsPromptProc *proc)
{
    int i;

    for (i = 0; i < START_MAP_EFFECT_VISIBLE_ROWS; ++i) {
        int itemNumber = proc->topVisibleIndex + i;
        int tileY = START_MAP_EFFECT_FRAME_Y + 1 + (i * 2);

        TileMap_FillRect(
            TILEMAP_LOCATED(gBG0TilemapBuffer, START_MAP_EFFECT_FRAME_X + 1, tileY),
            START_MAP_EFFECT_FRAME_W - 3,
            1,
            0
        );

        ClearText(&sStartMapEffectsTexts[i]);

        if (itemNumber >= proc->itemCount)
            continue;

        sStartMapEffectsTexts[i].colorId = StartMapEffects_GetSelectionColor(itemNumber, proc->curIndex);
        Text_DrawString(&sStartMapEffectsTexts[i], StartMapEffects_GetLabel(itemNumber));

        PutText(&sStartMapEffectsTexts[i], TILEMAP_LOCATED(gBG0TilemapBuffer, START_MAP_EFFECT_FRAME_X + 1, tileY));
    }

    UpdateMenuScrollBarConfig(8, (u16)proc->topVisibleIndex * 16, (u16)proc->itemCount, START_MAP_EFFECT_VISIBLE_ROWS);
    if (proc->helpOpen)
        StartMapEffectsPrompt_UpdateHelpBox(proc);
    StartMapEffectsPrompt_UpdateCursorHand(proc);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

void StartMapEffectsPrompt_OnInit(struct Proc *proc_)
{
    if (gpKernelDesignerConfig->start_map_effects != true)
        return;

    struct StartMapEffectsPromptProc *proc = (struct StartMapEffectsPromptProc *)proc_;
    int i;

    proc->curIndex = 0;
    proc->topVisibleIndex = 0;
    proc->itemCount = StartMapEffects_GetItemCount();
    proc->helpOpen = 0;

    SetTextFont(0);
    InitSystemTextFont();
    ResetText();
    BG_SetPosition(BG_0, 0, 0);
    BG_SetPosition(BG_1, 0, 0);

    StartMapEffectsPrompt_ClearUi();
    StartMapEffectsPrompt_ApplyBgPriority(proc);
    StartMapEffectsPrompt_HideUnitSprites();
    LoadUiFrameGraphics();
    LoadObjUIGfx();
    StartMapEffectsPrompt_LoadHelpBoxGfx();
    ApplyPalette(gUiFramePaletteD, 2);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    for (i = 0; i < START_MAP_EFFECT_VISIBLE_ROWS; ++i)
        InitText(&sStartMapEffectsTexts[i], START_MAP_EFFECT_FRAME_W - 3);

    StartMapEffectsPrompt_DrawFrame();
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);

    StartMenuScrollBarExt((ProcPtr)proc, START_MAP_EFFECT_SCROLLBAR_X, START_MAP_EFFECT_SCROLLBAR_Y, 0x200, START_MAP_EFFECT_VISIBLE_ROWS);

    sStartMapEffectsSuspendState.promptState = START_MAP_EFFECT_PROMPT_OPEN;
    StartMapEffectsPrompt_Draw(proc);
}

static void StartMapEffectsPrompt_HandleInput(struct StartMapEffectsPromptProc *proc)
{
    int previousIndex = proc->curIndex;
    int previousTopVisibleIndex = proc->topVisibleIndex;

    proc->helpOpen = StartMapEffectsPrompt_IsHelpBoxOpen();

    if (DPAD_UP & gKeyStatusPtr->repeatedKeys) {
        if (proc->curIndex > 0) {
            proc->curIndex--;
        } else if (DPAD_UP & gKeyStatusPtr->newKeys) {
            proc->curIndex = proc->itemCount - 1;
        }
    }

    if (DPAD_DOWN & gKeyStatusPtr->repeatedKeys) {
        if (proc->curIndex + 1 < proc->itemCount) {
            proc->curIndex++;
        } else if (DPAD_DOWN & gKeyStatusPtr->newKeys) {
            proc->curIndex = 0;
        }
    }

    if (proc->curIndex < proc->topVisibleIndex)
        proc->topVisibleIndex = proc->curIndex;

    if (proc->curIndex >= proc->topVisibleIndex + START_MAP_EFFECT_VISIBLE_ROWS)
        proc->topVisibleIndex = proc->curIndex - START_MAP_EFFECT_VISIBLE_ROWS + 1;

    if (previousIndex != proc->curIndex)
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

    if (R_BUTTON & gKeyStatusPtr->newKeys) {
        StartMapEffectsPrompt_UpdateHelpBox(proc);
        proc->helpOpen = StartMapEffectsPrompt_IsHelpBoxOpen();

        return;
    }

    if (A_BUTTON & gKeyStatusPtr->newKeys) {
        if (proc->curIndex == 0 || StartMapEffects_IsUnlocked(proc->curIndex - 1)) {
            StartMapEffectsPrompt_Finish(proc, proc->curIndex - 1);
            return;
        }

        PlaySoundEffect(SONG_6C);
        return;
    }

    if (B_BUTTON & gKeyStatusPtr->newKeys) {
        if (StartMapEffectsPrompt_IsHelpBoxOpen()) {
            CloseHelpBox();
            proc->helpOpen = 0;
            return;
        }

        StartMapEffectsPrompt_Finish(proc, START_MAP_EFFECT_NONE);
        return;
    }

    if ((previousIndex != proc->curIndex || previousTopVisibleIndex != proc->topVisibleIndex) && proc->helpOpen)
        StartMapEffectsPrompt_UpdateHelpBox(proc);

    if (previousIndex != proc->curIndex || previousTopVisibleIndex != proc->topVisibleIndex)
        StartMapEffectsPrompt_Draw(proc);
}

void StartMapEffectsPrompt_OnLoop(struct Proc *proc_)
{
    struct StartMapEffectsPromptProc *proc = (struct StartMapEffectsPromptProc *)proc_;

    if (sStartMapEffectsSuspendState.promptState == START_MAP_EFFECT_PROMPT_DONE) {
        Proc_Break(proc);
        return;
    }

    StartMapEffectsPrompt_HandleInput(proc);
}

static const struct ProcCmd sStartMapEffectsPromptProc[] = {
    PROC_NAME("StartMapEffectsPrompt"),
    PROC_YIELD,
    PROC_CALL(StartMapEffectsPrompt_OnInit),
    PROC_REPEAT(StartMapEffectsPrompt_OnLoop),
    PROC_END,
};

void StartMapEffects_UnlockEffect(int effectId)
{
    if (!StartMapEffectsEffectValid(effectId))
        return;

    gStartMapEffectsUnlockMask |= (1u << effectId);
}

bool StartMapEffects_IsUnlocked(int effectId)
{
    if (!StartMapEffectsEffectValid(effectId))
        return false;

    return true;
}

void StartMapEffects_ResetAll(void)
{
    gStartMapEffectsUnlockMask = START_MAP_EFFECT_DEFAULT_UNLOCK_MASK;

    StartMapEffects_ClearSuspendState();
}

void ChapterInit_ResetStartMapEffects(void)
{
    StartMapEffects_ClearSuspendState();
}

bool StartMapEffects_PrePhaseHook(ProcPtr proc)
{
    if (gpKernelDesignerConfig->start_map_effects != true)
        return false;

    StartMapEffects_ResetChapterState();

    if (sStartMapEffectsSuspendState.promptState == START_MAP_EFFECT_PROMPT_IDLE) {
        Proc_StartBlocking(sStartMapEffectsPromptProc, proc);
        return true;
    }

    if (sStartMapEffectsSuspendState.promptState == START_MAP_EFFECT_PROMPT_DONE &&
        sStartMapEffectsSuspendState.activeEffectId != START_MAP_EFFECT_NONE) {
        if (sStartMapEffectsSuspendState.turnsRemaining > 0)
            sStartMapEffectsSuspendState.turnsRemaining--;

        if (sStartMapEffectsSuspendState.turnsRemaining == 0)
            sStartMapEffectsSuspendState.activeEffectId = START_MAP_EFFECT_NONE;
    }

    return false;
}

void SaveStartMapEffectsUnlockMask(u8 *dst, const u32 size)
{
    if (size < sizeof(gStartMapEffectsUnlockMask))
        return;

    WriteAndVerifySramFast(&gStartMapEffectsUnlockMask, dst, sizeof(gStartMapEffectsUnlockMask));
}

void LoadStartMapEffectsUnlockMask(u8 *src, const u32 size)
{
    if (size < sizeof(gStartMapEffectsUnlockMask))
        return;

    ReadSramFast(src, &gStartMapEffectsUnlockMask, sizeof(gStartMapEffectsUnlockMask));
}

void SaveStartMapEffectsSuspendState(u8 *dst, const u32 size)
{
    if (size < sizeof(sStartMapEffectsSuspendState))
        return;

    WriteAndVerifySramFast(&sStartMapEffectsSuspendState, dst, sizeof(sStartMapEffectsSuspendState));
}

void LoadStartMapEffectsSuspendState(u8 *src, const u32 size)
{
    if (size < sizeof(sStartMapEffectsSuspendState))
        return;

    ReadSramFast(src, &sStartMapEffectsSuspendState, sizeof(sStartMapEffectsSuspendState));
}

int PowGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int MagGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int SklGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int SpdGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int LckGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int DefGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int ResGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyStatEffect(status, unit);
}

int MovGetterStartMapEffects(int status, struct Unit *unit)
{
    return StartMapEffects_ApplyMovEffect(status, unit);
}