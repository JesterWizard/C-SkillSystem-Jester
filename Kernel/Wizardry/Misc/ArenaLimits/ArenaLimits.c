#include "gbafe.h"
#include "common-chax.h"
#include "kernel-lib.h"
#include "save-data.h"
#include "skill-system.h"
#include "constants/texts.h"
#include "constants/skills.h"
#include "event-rework.h"
#include "bmarena.h"

// Arena Limits. Hack by Zeta/Gilgamesh
// Requires FE-CLIB
// Free to use/modify

#define ARENA_WIN 0x1
#define ARENA_LOSS 0x2
#define ARENA_OK 0x1
#define CANT_ARENA 0x3
#define CENTERTUTORIALTEXTBOX SVAL(0xB,0xFFFFFFFF)

#define ARENA_ROSTER_VISIBLE_ROWS 5
#define ARENA_ROSTER_MAX_TRACKED_ENTRIES 32
#define ARENA_ROSTER_CURSOR_X 12
#define ARENA_ROSTER_CURSOR_Y 72
#define ARENA_ROSTER_ROW_HEIGHT 16
#define ARENA_ROSTER_LIST_X 1
#define ARENA_ROSTER_LIST_Y 8
#define ARENA_ROSTER_LIST_W 28
#define ARENA_ROSTER_LIST_H 12
#define ARENA_ROSTER_SCROLLBAR_X 228
#define ARENA_ROSTER_SCROLLBAR_Y 80
#define ARENA_ROSTER_TEXT_NAME_WIDTH 10
#define ARENA_ROSTER_TEXT_BUFFER_WIDTH 13
#define ARENA_ROSTER_TEXT_REWARD_WIDTH 8
#define ARENA_ROSTER_REWARD_ICON_X 19
#define ARENA_ROSTER_REWARD_TEXT_X 21
#define ARENA_ROSTER_STATE_RESERVED_BYTES 25

enum ArenaRosterChoiceState {
    ARENA_ROSTER_CHOICE_NONE,
    ARENA_ROSTER_CHOICE_CANCELLED,
    ARENA_ROSTER_CHOICE_SELECTED,
};

#define ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND (1 << 7)

enum ArenaRosterRewardType {
    ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
    ARENA_ROSTER_REWARD_TRIPLE_PAYOUT,
    ARENA_ROSTER_REWARD_ITEM,
};

struct _ArenaLimitTable {
    u8 ChapterID;
    u8 LevelLimit;
    u8 TurnLimit;
};

struct ArenaRosterEntry {
    u8 pid;
    u8 jid;
    u8 level;
    u8 rewardType;
    u16 item;
    u16 price;
    u16 reward;
};

struct ArenaRosterChapter {
    u8 chapterId;
    u8 maxWins;
    u8 entryCount;
    u8 _pad;
    const struct ArenaRosterEntry *entries;
};

struct ArenaRosterSuspendState {
    u8 chapterId;
    u8 wins;
    u8 _pad[2];
    u8 clearedFlags[4];
    u8 reserved[ARENA_ROSTER_STATE_RESERVED_BYTES];
};

struct ProcArenaRosterSelect {
    PROC_HEADER;
    int cursor;
    int scrollTop;
    u8 promptShown;
    u8 _pad[3];
};

struct ArenaRosterRuntimeState {
    u8 choiceState;
    s8 selectedIndex;
    u16 pendingRewardItem;
};

extern struct Text gBanimText[20];

STATIC_DECLAR const EventScr EventScr_ArenaClosed[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaClosedString)
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_ArenaLevelLimit[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaLevelLimitString)
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_UnpromotedOnly[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_UnpromotedOnlyString)
    TEXTEND
    NOFADE
    ENDA
};

extern struct _ArenaLimitTable ArenaLimitTable[];
extern u8 NonLethalArena;
extern u8 MaxLevel;
extern struct ArenaRosterSuspendState sArenaRosterSuspendState;
extern struct ArenaRosterRuntimeState sArenaRosterRuntimeState;

STATIC_DECLAR const struct ArenaRosterEntry sArenaRosterChapter6Entries[] = {
    {
        .pid = CHARACTER_JOSHUA,
        .jid = CLASS_MYRMIDON,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_ITEM,
        .item = ITEM_SWORD_KILLER,
        .price = 900,
        .reward = ITEM_SWORD_AUDHULMA,
    },
    {
        .pid = 0x00,
        .jid = CLASS_MERCENARY,
        .level = 6,
        .rewardType = ARENA_ROSTER_REWARD_TRIPLE_PAYOUT,
        .item = 0x01,
        .price = 500,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_FIGHTER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x1F,
        .price = 480,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_ARCHER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_ITEM,
        .item = ITEM_BOW_IRON,
        .price = 460,
        .reward = ITEM_BOW_IRON,
    },
    {
        .pid = 0x00,
        .jid = CLASS_SOLDIER,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x14,
        .price = 440,
        .reward = 0,
    },
    {
        .pid = 0x00,
        .jid = CLASS_PALADIN,
        .level = 5,
        .rewardType = ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
        .item = 0x14,
        .price = 440,
        .reward = 0,
    },
};

const struct ArenaRosterChapter gArenaRosterTable[] = {
    {
        .chapterId = 6,
        .maxWins = 5,
        .entryCount = sizeof(sArenaRosterChapter6Entries) / sizeof(sArenaRosterChapter6Entries[0]),
        .entries = sArenaRosterChapter6Entries,
    },
    {
        .chapterId = 0xFF,
    },
};

STATIC_DECLAR void ArenaRosterSelector_Init(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR void ArenaRosterSelector_End(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR void ArenaRosterSelector_Loop(struct ProcArenaRosterSelect *proc);
STATIC_DECLAR int ArenaRosterGetEntryCount(const struct ArenaRosterChapter *chapter);
STATIC_DECLAR int ArenaRosterGetWagerMultiplier(void);
STATIC_DECLAR unsigned ArenaRosterGetRewardGold(const struct ArenaRosterEntry *entry);
STATIC_DECLAR bool ArenaRosterHasReachedWinLimit(const struct ArenaRosterChapter *chapter);
STATIC_DECLAR void ArenaRosterGrantWinReward(const struct ArenaRosterEntry *entry);
STATIC_DECLAR void ArenaRosterApplyEnemySpritePalettes(void);
STATIC_DECLAR void ArenaRosterUpdateScrollBar(struct ProcArenaRosterSelect *proc, int entryCount);
STATIC_DECLAR void ArenaRosterDrawSpritesOnly(struct ProcArenaRosterSelect *proc, const struct ArenaRosterChapter *chapter);

STATIC_DECLAR const struct ProcCmd ProcScr_ArenaRosterSelect[] = {
    PROC_CALL(ArenaRosterSelector_Init),
    PROC_REPEAT(ArenaRosterSelector_Loop),
    PROC_CALL(ArenaRosterSelector_End),
    PROC_END,
};

STATIC_DECLAR const struct ArenaRosterChapter *ArenaRosterGetChapter(int chapter_id)
{
    int index;

    for (index = 0; gArenaRosterTable[index].chapterId != 0xFF; ++index)
        if (gArenaRosterTable[index].chapterId == chapter_id)
            return &gArenaRosterTable[index];

    return NULL;
}

STATIC_DECLAR void ArenaRosterClearProgress(void)
{
    sArenaRosterSuspendState.clearedFlags[0] = 0;
    sArenaRosterSuspendState.clearedFlags[1] = 0;
    sArenaRosterSuspendState.clearedFlags[2] = 0;
    sArenaRosterSuspendState.clearedFlags[3] = 0;
    sArenaRosterSuspendState.wins = 0;
    sArenaRosterSuspendState._pad[0] = 0;
    sArenaRosterSuspendState._pad[1] = 0;
    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;
}

STATIC_DECLAR void ArenaRosterEnsureSuspendStateCurrent(void)
{
    if (sArenaRosterSuspendState.chapterId == gPlaySt.chapterIndex)
        return;

    sArenaRosterSuspendState.chapterId = gPlaySt.chapterIndex;
    ArenaRosterClearProgress();
}

STATIC_DECLAR int ArenaRosterGetWinLimit(const struct ArenaRosterChapter *chapter)
{
    if (!chapter)
        return 0;

    return chapter->maxWins;
}

STATIC_DECLAR int ArenaRosterGetEntryCount(const struct ArenaRosterChapter *chapter)
{
    if (!chapter || !chapter->entries)
        return 0;

    if (chapter->entryCount > ARENA_ROSTER_MAX_TRACKED_ENTRIES)
        return ARENA_ROSTER_MAX_TRACKED_ENTRIES;

    return chapter->entryCount;
}

STATIC_DECLAR bool ArenaRosterHasReachedWinLimit(const struct ArenaRosterChapter *chapter)
{
    int limit = ArenaRosterGetWinLimit(chapter);

    if (limit == 0)
        return false;

    ArenaRosterEnsureSuspendStateCurrent();
    return sArenaRosterSuspendState.wins >= limit;
}

STATIC_DECLAR bool ArenaRosterEntryValid(const struct ArenaRosterEntry *entry)
{
    return entry && entry->jid && entry->item;
}

STATIC_DECLAR bool ArenaRosterEntryCleared(int entry_idx)
{
    int byteIndex;
    int bitIndex;

    ArenaRosterEnsureSuspendStateCurrent();

    if ((entry_idx < 0) || (entry_idx >= ARENA_ROSTER_MAX_TRACKED_ENTRIES))
        return false;

    byteIndex = entry_idx >> 3;
    bitIndex = entry_idx & 7;

    return (sArenaRosterSuspendState.clearedFlags[byteIndex] & (1u << bitIndex)) != 0;
}

STATIC_DECLAR void ArenaRosterSetEntryCleared(int entry_idx)
{
    int byteIndex;
    int bitIndex;

    ArenaRosterEnsureSuspendStateCurrent();

    if ((entry_idx < 0) || (entry_idx >= ARENA_ROSTER_MAX_TRACKED_ENTRIES))
        return;

    if (ArenaRosterEntryCleared(entry_idx))
        return;

    byteIndex = entry_idx >> 3;
    bitIndex = entry_idx & 7;

    sArenaRosterSuspendState.clearedFlags[byteIndex] |= (1u << bitIndex);
    sArenaRosterSuspendState.wins++;
}

STATIC_DECLAR const struct ArenaRosterEntry *ArenaRosterGetSelectedEntry(void)
{
    const struct ArenaRosterChapter *chapter;

    if ((sArenaRosterRuntimeState.choiceState & 0x7F) != ARENA_ROSTER_CHOICE_SELECTED)
        return NULL;
    chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    if (!chapter)
        return NULL;

    if ((sArenaRosterRuntimeState.selectedIndex < 0) || (sArenaRosterRuntimeState.selectedIndex >= ArenaRosterGetEntryCount(chapter)))
        return NULL;

    return &chapter->entries[sArenaRosterRuntimeState.selectedIndex];
}

STATIC_DECLAR char const *ArenaRosterGetDisplayName(const struct ArenaRosterEntry *entry)
{
    if (entry->pid && GetCharacterData(entry->pid)->nameTextId)
        return GetStringFromIndex(GetCharacterData(entry->pid)->nameTextId);

    return GetStringFromIndex(GetClassData(entry->jid)->nameTextId);
}

STATIC_DECLAR int ArenaRosterGetWagerMultiplier(void)
{
    int multiplier = 1;

#if defined(SID_Ludopathy) && (COMMON_SKILL_VALID(SID_Ludopathy))
    if (SkillTester(gActiveUnit, SID_Ludopathy))
        multiplier = 2;
#endif

    return multiplier;
}

STATIC_DECLAR unsigned ArenaRosterGetRewardGold(const struct ArenaRosterEntry *entry)
{
    if (!entry)
        return 0;

    if (entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT)
        return entry->price * 3;

    if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM)
        return 0;

    return entry->price * 2;
}

STATIC_DECLAR bool ArenaRosterIsDialogueOpen(void)
{
    return Proc_Find(gProcScr_TalkOpen) != NULL;
}

STATIC_DECLAR void ArenaRosterGrantWinReward(const struct ArenaRosterEntry *entry)
{
    int item;

    if (!entry)
        return;

    if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM) {
        item = MakeNewItem(entry->reward);

        AddItemToConvoy(item);

        return;
    }

    SetPartyGoldAmount(GetPartyGoldAmount() + ArenaRosterGetRewardGold(entry));
}

STATIC_DECLAR void ArenaRosterApplyEnemySpritePalettes(void)
{
    int faction = gPlaySt.faction;

    gPlaySt.faction = FACTION_RED;
    ApplyUnitSpritePalettes();
    gPlaySt.faction = faction;
}

STATIC_DECLAR void ArenaRosterUpdateScrollBar(struct ProcArenaRosterSelect *proc, int entryCount)
{
    if (entryCount < 1)
        entryCount = 1;

    UpdateMenuScrollBarConfig(8, proc->scrollTop * ARENA_ROSTER_ROW_HEIGHT, entryCount, ARENA_ROSTER_VISIBLE_ROWS);
}

STATIC_DECLAR void ArenaRosterDrawSpritesOnly(struct ProcArenaRosterSelect *proc, const struct ArenaRosterChapter *chapter)
{
    int row;
    int entryCount;

    if (!chapter)
        return;

    entryCount = ArenaRosterGetEntryCount(chapter);

    ClearSprites();

    for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
        int index = proc->scrollTop + row;
        const struct ArenaRosterEntry *entry;

        if (index >= entryCount)
            continue;

        entry = &chapter->entries[index];

        if (!ArenaRosterEntryValid(entry))
            continue;

        PutUnitSpriteForClassId(0, 36, ARENA_ROSTER_CURSOR_Y + row * ARENA_ROSTER_ROW_HEIGHT, 0xD800, entry->jid);
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();

    if ((proc->cursor >= proc->scrollTop) && (proc->cursor < proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS))
        ShowSysHandCursor(ARENA_ROSTER_CURSOR_X, ARENA_ROSTER_CURSOR_Y + (proc->cursor - proc->scrollTop) * ARENA_ROSTER_ROW_HEIGHT, 0x8, 0x800);
}

STATIC_DECLAR void ArenaRosterDrawList(struct ProcArenaRosterSelect *proc)
{
    const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    int entryCount;
    int row;

    if (!chapter)
        return;

    entryCount = ArenaRosterGetEntryCount(chapter);

    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
    DrawUiFrame2(ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y, ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);

    for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
        int index = proc->scrollTop + row;
        char payoutLabel[14];
        struct Text *nameText = &gBanimText[row];
        struct Text *rewardText = &gBanimText[row + ARENA_ROSTER_VISIBLE_ROWS];

        ClearText(nameText);
        ClearText(rewardText);

        if (index >= entryCount)
            continue;

        {
            const struct ArenaRosterEntry *entry = &chapter->entries[index];
            int color = ArenaRosterEntryCleared(index) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;

            if (!ArenaRosterEntryValid(entry))
                continue;

            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 9 + row * 2), color, index + 1);
            PutDrawText(nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_NAME_WIDTH, ArenaRosterGetDisplayName(entry));
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 17, 9 + row * 2), color, entry->price);

            if ((entry->rewardType == ARENA_ROSTER_REWARD_ITEM) && (entry->reward != ITEM_NONE)) {
                int icon = GetItemIconId(entry->reward);

                if (icon >= 0)
                    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_ICON_X, 9 + row * 2), icon, 0x4000);

                PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_REWARD_WIDTH, GetItemName(entry->reward));
            } else {
                if (entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT) {
                    payoutLabel[0] = 'T';
                    payoutLabel[1] = 'r';
                    payoutLabel[2] = 'i';
                    payoutLabel[3] = 'p';
                    payoutLabel[4] = 'l';
                    payoutLabel[5] = 'e';
                } else {
                    payoutLabel[0] = 'D';
                    payoutLabel[1] = 'o';
                    payoutLabel[2] = 'u';
                    payoutLabel[3] = 'b';
                    payoutLabel[4] = 'l';
                    payoutLabel[5] = 'e';
                }

                payoutLabel[6] = ' ';
                payoutLabel[7] = 'P';
                payoutLabel[8] = 'a';
                payoutLabel[9] = 'y';
                payoutLabel[10] = 'o';
                payoutLabel[11] = 'u';
                payoutLabel[12] = 't';
                payoutLabel[13] = 0;

                PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X - 2, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_BUFFER_WIDTH, payoutLabel);
            }
        }
    }

    ArenaRosterUpdateScrollBar(proc, entryCount);
    ArenaRosterDrawSpritesOnly(proc, chapter);
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

STATIC_DECLAR void ArenaRosterApplySelection(const struct ArenaRosterEntry *entry)
{
    gArenaState.opponentUnit = &gArenaOpponent;
    gArenaState.opponentClassId = entry->jid;
    gArenaState.opponentWpnType = GetItemType(entry->item);
    gArenaState.opponentIsMagic = IsWeaponMagic(gArenaState.opponentWpnType);
    gArenaState.opponentLevel = entry->level;
    gArenaState.matchupGoldValue = entry->price;

    ArenaGenerateOpponentUnit();
    ArenaGenerateBaseWeapons();
    gArenaState.playerPowerWeight = ArenaGetPowerRanking(gArenaState.playerUnit, gArenaState.opponentIsMagic);
    gArenaState.opponentPowerWeight = ArenaGetPowerRanking(gArenaState.opponentUnit, gArenaState.playerIsMagic);
    ArenaSetResult(0);
    gArenaState.unk0B = 1;
    ArenaSetFallbackWeaponsMaybe();
}

STATIC_DECLAR void ArenaRosterSelector_Init(struct ProcArenaRosterSelect *proc)
{
    unsigned index;

    proc->cursor = 0;
    proc->scrollTop = 0;
    proc->promptShown = 0;
    SetTextFont(0);
    InitSystemTextFont();
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ArenaRosterApplyEnemySpritePalettes();
    LoadIconPalettes(4);
    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(ARENA_ROSTER_SCROLLBAR_X, ARENA_ROSTER_SCROLLBAR_Y);
    InitMenuScrollBarImg(0x7A60, 2);

    for (index = 0; index < (ARENA_ROSTER_VISIBLE_ROWS * 2) + 2; ++index)
        InitText(&gBanimText[index], ARENA_ROSTER_TEXT_BUFFER_WIDTH);

    ArenaRosterDrawList(proc);
}

STATIC_DECLAR void ArenaRosterSelector_End(struct ProcArenaRosterSelect *proc)
{
    (void) proc;

    EndMenuScrollBar();
    HideSysHandCursor();
}

STATIC_DECLAR void ArenaRosterSelector_Loop(struct ProcArenaRosterSelect *proc)
{
    const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
    int entryCount;
    ProcPtr parent = proc->proc_parent ? proc->proc_parent : proc;
    bool redraw = false;

    if (!chapter) {
        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_CANCELLED;
        sArenaRosterRuntimeState.selectedIndex = -1;
        Proc_Break(proc);
        return;
    }

    entryCount = ArenaRosterGetEntryCount(chapter);

    if (!proc->promptShown) {
        proc->promptShown = 1;
       // StartArenaDialogue(MSG_ArenaPickOpponentString, parent);
        return;
    }

    if (ArenaRosterIsDialogueOpen())
        return;

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_CANCELLED;
        sArenaRosterRuntimeState.selectedIndex = -1;
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        Proc_Break(proc);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->cursor > 0) {
            proc->cursor--;

            if (proc->cursor < proc->scrollTop)
                proc->scrollTop = proc->cursor;

            redraw = true;
        }

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->cursor < entryCount - 1) {
            proc->cursor++;

            if (proc->cursor >= proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS)
                proc->scrollTop = proc->cursor - ARENA_ROSTER_VISIBLE_ROWS + 1;

            redraw = true;
        }

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
    }

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        const struct ArenaRosterEntry *entry = &chapter->entries[proc->cursor];

        if (!ArenaRosterEntryValid(entry) || ArenaRosterEntryCleared(proc->cursor)) {
            PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
            ArenaRosterDrawList(proc);
            return;
        }

        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_SELECTED;
        sArenaRosterRuntimeState.selectedIndex = proc->cursor;
        ArenaRosterApplySelection(entry);

        if (gpKernelDesignerConfig->arena_show_opponent_in_advance == true) {
            DrawUiFrame2(7, 9, 0x10, 8, 0);
            DrawArenaOpponentDetailsText(parent);
        }

        SetTalkNumber(ArenaGetMatchupGoldValue() * ArenaRosterGetWagerMultiplier());
        StartArenaDialogue(0x8D2, parent);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Break(proc);
        return;
    }

    if (redraw)
        ArenaRosterDrawList(proc);
    else
        ArenaRosterDrawSpritesOnly(proc, chapter);
}

bool ArenaRosterMenuEnabled(void)
{
    if (gpKernelDesignerConfig->arena_roster_menu != true)
        return false;

    if (UNIT_FACTION(gActiveUnit) != FACTION_BLUE)
        return false;

    return ArenaRosterGetChapter(gPlaySt.chapterIndex) != NULL;
}

bool ArenaRosterStartSelection(ProcPtr proc)
{
    const struct ArenaRosterChapter *chapter;

    if (!ArenaRosterMenuEnabled())
        return false;

    ArenaRosterEnsureSuspendStateCurrent();
    chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);

    if (ArenaRosterHasReachedWinLimit(chapter)) {
        StartArenaDialogue(MSG_ArenaCleanedOutString, proc);
        Proc_Goto(proc, 2);
        return true;
    }

    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
    sArenaRosterRuntimeState.selectedIndex = -1;
    Proc_StartBlocking(ProcScr_ArenaRosterSelect, proc);
    return true;
}

bool ArenaRosterHandleConfirmation(ProcPtr proc)
{
    if (!ArenaRosterMenuEnabled())
        return false;

    if ((sArenaRosterRuntimeState.choiceState & 0x7F) != ARENA_ROSTER_CHOICE_CANCELLED)
        return false;

    ArenaRosterClearSelection();
    Proc_Goto(proc, 2);
    return true;
}

bool ArenaRosterHasConfiguredOpponent(void)
{
    return ArenaRosterGetSelectedEntry() != NULL;
}

void ArenaRosterFlushDeferredPopup(void)
{
    extern struct ProcCmd gProcScr_ArenaUiMain[];
    extern struct ProcCmd gProcScr_ArenaUiResults[];

    ProcPtr proc;
    int item;

    item = sArenaRosterRuntimeState.pendingRewardItem;

    if (item == ITEM_NONE)
        return;

    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;
    proc = Proc_Find(gProcScr_ArenaUiMain);

    if (!proc)
        proc = Proc_Find(gProcScr_ArenaUiResults);

    if (!proc)
        proc = Proc_Find(gProcScr_PlayerPhase);

    if (!proc)
        proc = Proc_Find(gProc_BMapMain);

    if (AddItemToConvoy(item) >= 0)
        NewPopup2_SendItem(proc, item);
    else
        NewPopup2_DropItem(proc, item);

    ArenaRosterClearSelection();
}

bool ArenaRosterHandleResultsDialogue(ProcPtr proc)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();
    int selectedIndex = sArenaRosterRuntimeState.selectedIndex;

    if (!entry)
        return false;

    switch (ArenaGetResult()) {
    case ARENA_WIN:
        if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM) {
            ArenaRosterSetEntryCleared(selectedIndex);
            sArenaRosterRuntimeState.pendingRewardItem = MakeNewItem(entry->reward);
            StartArenaDialogue(MSG_ArenaItemPrizeString, proc);
            return true;
        }

        ArenaRosterSetEntryCleared(selectedIndex);

        SetTalkNumber(ArenaRosterGetRewardGold(entry));
        StartArenaDialogue(0x8D6, proc);
        ArenaRosterGrantWinReward(entry);
        break;

    case ARENA_LOSS:
        StartArenaDialogue(0x8D7, proc);
        break;

    case 3:
        StartArenaDialogue(0x8D9, proc);
        SetPartyGoldAmount(GetPartyGoldAmount() + ArenaGetMatchupGoldValue());
        break;

    case 4:
        StartArenaDialogue(0x8D8, proc);
        break;
    }

    ArenaRosterClearSelection();
    return true;
}

bool ArenaRosterGenerateOpponentUnit(struct Unit *unit)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();
    struct UnitDefinition udef;

    if (!entry)
        return false;

    CpuFill16(0, &udef, sizeof(udef));
    udef.charIndex = entry->pid ? entry->pid : CHARACTER_ARENA_OPPONENT;
    udef.classIndex = entry->jid;
    udef.level = entry->level;
    udef.autolevel = 1;
    udef.items[0] = entry->item;

    unit->index = 0x80;
    UnitInitFromDefinition(unit, &udef);
    UnitLoadStatsFromChracter(unit, unit->pCharacterData);
    unit->level = entry->level;
    UnitAutolevel(unit);

    if (GetItemType(entry->item) < 8)
        unit->ranks[GetItemType(entry->item)] = 0xFB;

    UnitCheckStatCaps(unit);
    SetUnitHp(unit, GetUnitMaxHp(unit));
    return true;
}

u16 ArenaRosterGetSelectedWeapon(void)
{
    const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();

    if (!entry)
        return ITEM_NONE;

    return entry->item;
}

void ArenaRosterClearSelection(void)
{
    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
    sArenaRosterRuntimeState.selectedIndex = -1;
}

void ChapterInit_ResetArenaRosterState(ProcPtr proc)
{
    (void) proc;

    if (sArenaRosterRuntimeState.choiceState & ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND) {
        sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_CHOICE_NONE;
        ArenaRosterClearSelection();
        return;
    }

    sArenaRosterSuspendState.chapterId = gPlaySt.chapterIndex;
    ArenaRosterClearProgress();
    ArenaRosterClearSelection();
}

void SaveArenaRosterProgress(u8 *dst, const u32 size)
{
    if (size < sizeof(sArenaRosterSuspendState))
        return;

    ArenaRosterEnsureSuspendStateCurrent();
    WriteAndVerifySramFast(&sArenaRosterSuspendState, dst, sizeof(sArenaRosterSuspendState));
}

void LoadArenaRosterProgress(u8 *src, const u32 size)
{
    if (size < sizeof(sArenaRosterSuspendState))
        return;

    ReadSramFast(src, &sArenaRosterSuspendState, sizeof(sArenaRosterSuspendState));
    sArenaRosterRuntimeState.choiceState = ARENA_ROSTER_RUNTIME_FLAG_LOADED_FROM_SUSPEND;
    sArenaRosterRuntimeState.selectedIndex = -1;
    sArenaRosterRuntimeState.pendingRewardItem = ITEM_NONE;
    ArenaRosterEnsureSuspendStateCurrent();
}

u8 CheckArenaLimits(void)
{
    ProcPtr playerPhaseProc;
    int x;

    if (gpKernelDesignerConfig->arena_limits != true)
        return ARENA_OK;

    playerPhaseProc = Proc_Find(gProcScr_PlayerPhase);

    for (x = 0; ArenaLimitTable[x].ChapterID != 0xFF; x++) {
        if (ArenaLimitTable[x].ChapterID != gPlaySt.chapterIndex)
            continue;

        if (ArenaLimitTable[x].TurnLimit) {
            if (gPlaySt.chapterTurnNumber > ArenaLimitTable[x].TurnLimit) {
                gEventSlots[EVT_SLOT_7] = 1;
                KernelCallEvent(EventScr_ArenaClosed, EV_EXEC_CUTSCENE, playerPhaseProc);
                return CANT_ARENA;
            }
        }

        if (ArenaLimitTable[x].LevelLimit) {
            u8 temp_level = gActiveUnit->level;

            if (gActiveUnit->pClassData->attributes & CA_PROMOTED)
                temp_level += MaxLevel;

            if (temp_level > ArenaLimitTable[x].LevelLimit) {
                if (temp_level > MaxLevel && MaxLevel > 20)
                    gEventSlots[EVT_SLOT_7] = 3;
                else if (temp_level > MaxLevel)
                    gEventSlots[EVT_SLOT_7] = 2;
                else
                    return ARENA_OK;

                if (gEventSlots[EVT_SLOT_7] == 2)
                    KernelCallEvent(EventScr_ArenaLevelLimit, EV_EXEC_CUTSCENE, playerPhaseProc);
                else if (gEventSlots[EVT_SLOT_7] == 4)
                    KernelCallEvent(EventScr_UnpromotedOnly, EV_EXEC_CUTSCENE, playerPhaseProc);

                return CANT_ARENA;
            }
        }

        return ARENA_OK;
    }

    return ARENA_OK;
}

void KillUnitIfNoHealth(struct Unit *unit)
{
    if (GetUnitCurrentHp(unit) == 0)
        if (gArenaState.result == ARENA_LOSS && NonLethalArena)
            unit->curHP = 1;
}

LYN_REPLACE_CHECK(DidUnitDie);
bool DidUnitDie(struct Unit *unit)
{
    if (GetUnitCurrentHp(unit) == 0) {
        if (gArenaState.result == ARENA_LOSS && NonLethalArena) {
            unit->curHP = 1;
            return TRUE;
        }

        return FALSE;
    }

    return TRUE;
}