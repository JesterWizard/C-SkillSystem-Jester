#include "common-chax.h"

extern u8 gPlayStChapterBits[4];
extern u8 gPlayStChapterMode[4];
extern struct PlaySt_OptionBits gPlayStOptionBits[4];

/*
 * Vanilla GMapData occupies 0xD0 bytes (its GMNode entries are 4 bytes),
 * while the C declaration in worldmap.h is compact. Every vanilla routine
 * used here reads and writes the vanilla layout, so keep the raw buffer.
 */
#define WM_SAVE_DATA_SIZE 0xD0

/* GMapData::units[0].location in the vanilla layout. */
#define WM_SAVE_UNIT0_LOCATION 0x11

static int GetLatestChapterFromSavedWorldMap(
    const u8 * worldMapData,
    const struct PlaySt * playSt)
{
    int chapterMode = gPlaySt.chapterModeIndex;
    int nodeId;
    int chapter = -1;

    /*
     * WMLoc_GetChapterId uses the global route, while the save slot may
     * belong to a different route than the currently loaded game.
     */
    gPlaySt.chapterModeIndex = playSt->chapterModeIndex;

    /*
     * LoadSavedWMStuff only reconstructs nodes, paths, units and a few
     * state bits from the packed GMapSaveInfo; trailing fields such as
     * current_node are never serialized. GetNextUnclearedNode and the
     * unit location are the only usable destination hints.
     */
    nodeId = GetNextUnclearedNode((struct GMapData *)worldMapData);

    /*
     * Only override when the saved world-map unit is away from the next
     * story node, i.e. the save was made on a skirmish or dungeon detour.
     * When the unit stands on the destination node, the saved chapter
     * index already names the chapter about to be played.
     */
    if ((nodeId >= 0) && (worldMapData[WM_SAVE_UNIT0_LOCATION] != nodeId))
        chapter = WMLoc_GetChapterId(nodeId);

    gPlaySt.chapterModeIndex = chapterMode;

    return chapter;
}

LYN_REPLACE_CHECK(SaveMenu_SetLcdChapterIdx);
void SaveMenu_SetLcdChapterIdx(void)
{
    /*
     * The vanilla preamble rewrites gPlaySt.chapterIndex for the menu title.
     * That value is also serialized by the next save, so leave it untouched;
     * SaveMenuInitSaveSlotData resolves world-map destination titles locally.
     */
    InitSaveMenuHelpTextSt();

    SetupBackgrounds(gBgConfig_SaveMenu);
    SetDispEnable(0, 0, 0, 0, 0);
    gLCDControlBuffer.dispcnt.mode = DISPCNT_MODE_0;
    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 1;
    gLCDControlBuffer.bg2cnt.priority = 2;
    gLCDControlBuffer.bg3cnt.priority = 3;
    SetBlendTargetA(0, 0, 1, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);
    SetBlendBackdropA(0);
    SetBlendBackdropB(0);
    SetBlendConfig(1, 6, 0x10, 0);
}

LYN_REPLACE_CHECK(SaveMenuInitSaveSlotData);
void SaveMenuInitSaveSlotData(u8 slot, struct SaveMenuProc * proc)
{
    struct PlaySt playSt;
    u8 worldMapData[WM_SAVE_DATA_SIZE];

    if (slot < 3)
    {
        if (IsSaveValid(slot))
        {
            int chIndex;
            int ch_idx;

            ReadGameSavePlaySt(slot, &playSt);

            ch_idx = playSt.chapterIndex;

            /*
             * World-map-backed saves, including skirmish saves, display the
             * chapter at the saved world-map destination rather than the
             * chapter whose map is loaded on resume.
             */
            if (!(playSt.chapterStateBits & PLAY_FLAG_COMPLETE))
            {
                int latestChapter;

                LoadSavedWMStuff(slot, (struct GMapData *)worldMapData);

                latestChapter = GetLatestChapterFromSavedWorldMap(
                    worldMapData,
                    &playSt);

                if (latestChapter >= 0)
                    ch_idx = latestChapter;
            }

            chIndex = playSt.chapterIndex;
            playSt.chapterIndex = ch_idx;
            proc->chapter_idx[slot] = GetChapterTitleExtra(&playSt);
            playSt.chapterIndex = chIndex;

            proc->played_time[slot] = playSt.time_saved;
            proc->unk_3a[slot] = 0;

            if (IsGameNotFirstChapter((struct PlaySt *)(uintptr_t)slot) != 0)
                proc->unk_3a[slot] |= 1;

            if (LoadSavedEid8A(slot) != 0)
                proc->unk_3a[slot] |= 2;

            if (playSt.chapterStateBits & PLAY_FLAG_COMPLETE)
                proc->unk_3a[slot] |= 4;

            gPlayStChapterBits[slot] = playSt.chapterStateBits;
            gPlayStChapterMode[slot] = playSt.chapterModeIndex;
            memcpy(&gPlayStOptionBits[slot], &playSt.config, 8);
        }
        else
        {
            proc->chapter_idx[slot] = (u8)-1;
            proc->unk_3a[slot] = 0;
            proc->played_time[slot] = 0;

            gPlayStChapterBits[slot] = 0;
            gPlayStChapterMode[slot] = 0;

            memset(&gPlayStOptionBits[slot], 0, 8);
        }
    }
    else if (proc->unk_44 == 0x100)
    {
        if (IsValidSuspendSave(3))
        {
            ReadSuspendSavePlaySt(3, &playSt);
            proc->sus_slot_cur = playSt.gameSaveSlot;
            proc->total_time = playSt.time_saved;
        }
        else
        {
            proc->unk_44 = 0xf0;
        }
    }
}
