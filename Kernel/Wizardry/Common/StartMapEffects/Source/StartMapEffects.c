#include "common-chax.h"
#include "kernel-lib.h"
#include "status-getter.h"

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
    START_MAP_EFFECT_FRAME_X = 5,
    START_MAP_EFFECT_FRAME_Y = 4,
    START_MAP_EFFECT_FRAME_W = 18,
    START_MAP_EFFECT_FRAME_H = 12,
    START_MAP_EFFECT_SCROLLBAR_X = ((START_MAP_EFFECT_FRAME_X + START_MAP_EFFECT_FRAME_W) * 8) - 7,
    START_MAP_EFFECT_SCROLLBAR_Y = ((START_MAP_EFFECT_FRAME_Y + 1) * 8) + 7,
};

struct StartMapEffectDef {
    const char *name;
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
    u8 bg0Priority;
    u8 bg1Priority;
    u8 bg2Priority;
};

extern u8 gStartMapEffectsUnlockMask;
extern struct Text sStartMapEffectsTexts[START_MAP_EFFECT_VISIBLE_ROWS];
extern struct StartMapEffectsSuspendState sStartMapEffectsSuspendState;
static const char sStartMapEffectsNoEffectLabel[] = "No effect";

static const struct StartMapEffectDef sStartMapEffectDefs[] = {
    { "P Rally +10% / 2T", START_MAP_EFFECT_KIND_STAT_PERCENT, 10, 2, FACTION_BLUE },
    { "E Drag -2 MOV / 2T", START_MAP_EFFECT_KIND_MOV_DELTA, -2, 2, FACTION_RED },
    { "P Advance +1 MOV / 3T", START_MAP_EFFECT_KIND_MOV_DELTA, 1, 3, FACTION_BLUE },
    { "E Pressure -10% / 2T", START_MAP_EFFECT_KIND_STAT_PERCENT, -10, 2, FACTION_RED },
    { "P Bulwark +2 ALL / 1T", START_MAP_EFFECT_KIND_STAT_FLAT, 2, 1, FACTION_BLUE },
    { "E Collapse -2 ALL / 2T", START_MAP_EFFECT_KIND_STAT_FLAT, -2, 2, FACTION_RED },
};

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

static inline bool StartMapEffectsUnitMatches(const struct Unit *unit, const struct StartMapEffectDef *def)
{
    return unit && UNIT_FACTION(unit) == def->targetFaction;
}

static inline int StartMapEffects_GetItemCount(void)
{
    return (int)ARRAY_COUNT(sStartMapEffectDefs) + 1;
}

static inline const char *StartMapEffects_GetLabel(int itemNumber)
{
    if (itemNumber == 0)
        return sStartMapEffectsNoEffectLabel;

    return sStartMapEffectDefs[itemNumber - 1].name;
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

static void StartMapEffectsPrompt_DrawFrame(void)
{
    DrawUiFrame(gBG1TilemapBuffer, START_MAP_EFFECT_FRAME_X, START_MAP_EFFECT_FRAME_Y, START_MAP_EFFECT_FRAME_W, START_MAP_EFFECT_FRAME_H, 0, 0);
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
        TILEMAP_LOCATED(gBG1TilemapBuffer, START_MAP_EFFECT_FRAME_X, START_MAP_EFFECT_FRAME_Y),
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

static int StartMapEffects_ApplyStatEffect(int status, struct Unit *unit)
{
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
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
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
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

void StartMapEffectsPrompt_OnInit(struct Proc *proc_)
{
    struct StartMapEffectsPromptProc *proc = (struct StartMapEffectsPromptProc *)proc_;
    int i;

    proc->curIndex = 0;
    proc->topVisibleIndex = 0;
    proc->itemCount = StartMapEffects_GetItemCount();

    SetTextFont(0);
    InitSystemTextFont();
    ResetText();
    BG_SetPosition(BG_0, 0, 0);
    BG_SetPosition(BG_1, 0, 0);

    StartMapEffectsPrompt_ClearUi();
    StartMapEffectsPrompt_ApplyBgPriority(proc);
    LoadUiFrameGraphics();
    ApplyPalette(gUiFramePaletteD, 2);

    for (i = 0; i < START_MAP_EFFECT_VISIBLE_ROWS; ++i)
        InitText(&sStartMapEffectsTexts[i], START_MAP_EFFECT_FRAME_W - 3);

    StartMapEffectsPrompt_DrawFrame();
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    StartMenuScrollBarExt((ProcPtr)proc, START_MAP_EFFECT_SCROLLBAR_X, START_MAP_EFFECT_SCROLLBAR_Y, 0x200, START_MAP_EFFECT_VISIBLE_ROWS);

    sStartMapEffectsSuspendState.promptState = START_MAP_EFFECT_PROMPT_OPEN;
    StartMapEffectsPrompt_Draw(proc);
}

static void StartMapEffectsPrompt_HandleInput(struct StartMapEffectsPromptProc *proc)
{
    int previousIndex = proc->curIndex;
    int previousTopVisibleIndex = proc->topVisibleIndex;

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

    if (A_BUTTON & gKeyStatusPtr->newKeys) {
        if (proc->curIndex == 0 || StartMapEffects_IsUnlocked(proc->curIndex - 1)) {
            StartMapEffectsPrompt_Finish(proc, proc->curIndex - 1);
            return;
        }

        PlaySoundEffect(SONG_6C);
        return;
    }

    if (B_BUTTON & gKeyStatusPtr->newKeys) {
        StartMapEffectsPrompt_Finish(proc, START_MAP_EFFECT_NONE);
        return;
    }

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