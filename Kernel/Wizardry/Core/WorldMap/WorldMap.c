#include "common-chax.h"
#include "kernel/no-cash-gba.h"

// static const u8 WmMonsterGenerateRatesIdx_EirikaMode[11] = {
//     /* chapter idx */
//     0x0A,
//     0x0B,
//     0x3D,
//     0x0D,
//     0x0E,
//     0x0F,
//     0x10,
//     0x11,
//     0x12,
//     0x13,
//     0x14,
// };

// static const u8 WmMonsterGenerateRatesIdx_EphraimMode[11] = {
//     /* chapter idx */
//     0x17,
//     0x18,
//     0x3E,
//     0x1A,
//     0x1B,
//     0x1C,
//     0x1D,
//     0x1E,
//     0x1F,
//     0x20,
//     0x21,
// };

// static const u8 WmMonsterGenerateRates_EirikaMode[WM_MON_LOC_MAX * 10] = {
//     40, 60,  0,  0,  0,  0,  0,  0,  0,
//     60, 40,  0,  0,  0,  0,  0,  0,  0,
//     55, 35, 10,  0,  0,  0,  0,  0,  0,
//     30, 40, 30,  0,  0,  0,  0,  0,  0,
//     30, 30, 30, 10,  0,  0,  0,  0,  0,
//      5,  5, 10, 20,  0,  0,  0,  0,  0,
//      5,  5, 15, 15, 25, 35,  0,  0,  0,
//      5,  5, 10, 20, 30, 30,  0,  0,  0,
//      5,  5, 15, 20, 15, 20, 10,  0,  0,
//      5,  5, 15, 20, 10, 20, 20,  5,  0,
// };

// static const u8 WmMonsterGenerateRates_XmapEirika[WM_MON_LOC_MAX] = {
//      5,  5, 15, 10, 15, 15, 15,  5, 15,
// };

// static const u8 WmMonsterGenerateRates_EphraimMode[WM_MON_LOC_MAX * 10] = {
//     40, 60,  0,  0,  0,  0,  0,  0,  0,
//     60, 40,  0,  0,  0,  0,  0,  0,  0,
//     55, 35,  0,  0, 10,  0,  0,  0,  0,
//     35, 45,  0,  0, 20,  0,  0,  0,  0,
//     30, 30,  0,  0, 30, 10,  0,  0,  0,
//      5,  5,  0,  0, 35, 25,  0,  0,  0,
//      5,  5, 15, 15, 25, 35,  0,  0,  0,
//      5,  5, 10, 20, 30, 30,  0,  0,  0,
//      5,  5, 15, 20, 15, 20, 10,  0,  0,
//      5,  5, 15, 20, 10, 20, 20,  5,  0,
// };

// static const u8 WmMonsterGenerateRates_XmapEphraim[WM_MON_LOC_MAX] = {
//      5,  5, 15, 10, 15, 15, 15,  5, 15,
// };

// static const u8 gWMMonsterSpawnLocations[WM_MON_LOC_MAX] = {
//     [WM_MON_LOC_0] = WM_NODE_ZahaWoods,
//     [WM_MON_LOC_1] = WM_NODE_AdlasPlains,
//     [WM_MON_LOC_2] = WM_NODE_TerazPlateau,
//     [WM_MON_LOC_3] = WM_NODE_HamillCanyon,
//     [WM_MON_LOC_4] = WM_NODE_Bethroen,
//     [WM_MON_LOC_5] = WM_NODE_ZaalbulMarsh,
//     [WM_MON_LOC_6] = WM_NODE_NarubeRiver,
//     [WM_MON_LOC_7] = WM_NODE_NelerasPeak,
//     [WM_MON_LOC_8] = WM_NODE_MelkaenCoast,
// };

// // static const u8 gWMMonsterSpawnsSize = WM_MON_LOC_MAX;

// //! FE8U = 0x080B9D14
// LYN_REPLACE_CHECK(WorldMap_GenerateRandomMonsters);
// void WorldMap_GenerateRandomMonsters(ProcPtr proc)
// {
//     int i;
//     int monster_amt;

//     s8 flag = 0;

//     if (!(gGMData.state.bits.monster_merged))
//         flag = 1;
//     else
//     {
//         if (gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME)
//         {
//             for (i = WM_MONS_UID_ENTRY; i < WM_MONS_UID_END; i++)
//                 if (gGMData.units[i].id != 0)
//                     break;

//             if (i == 7)
//                 flag = 1;
//         }
//         else
//         {
//             if (gGMData.units[0].location[gWMNodeData].placementFlag == GMAP_NODE_PLACEMENT_DUNGEON)
//             {
//                 for (i = WM_MONS_UID_ENTRY; i < WM_MONS_UID_END; i++)
//                     if (gGMData.units[i].id != 0)
//                         break;

//                 if (i == WM_MONS_UID_END)
//                     flag = 1;
//             }
//         }
//     }

//     if (flag)
//     {
//         NewGmapTimeMons(NULL, &monster_amt);
//         if (monster_amt > 0)
//             Proc_Goto(proc, 2);
//     }
//     WmShowMonsters();
// }

LYN_REPLACE_CHECK(GmapTimeMons_Init);
void GmapTimeMons_Init(struct ProcGmapTimeMons * proc)
{
    int ret;

    proc->trigger = false;
    ret = GenerateRandomonsterMergeConf(GetNextUnclearedChapter(), proc->confs);
    proc->monster_amt = ret;

    if (!(u8)ret)
        Proc_Goto(proc, 0);
}

LYN_REPLACE_CHECK(GmapTimeMons_ExecMonsterMergeMu);
void GmapTimeMons_ExecMonsterMergeMu(struct ProcGmapTimeMons * proc)
{
    int i, wm_uid;
    if (proc->trigger != false)
    {
        PlaySoundEffect(SONG_312);
        for (i = 0; i < proc->monster_amt; i++)
        {
            s16 x, y;
            s16 x1, y1, x2, y2;

            *&x1 = proc->confs[i].node[gWMNodeData].x;
            *&y1 = proc->confs[i].node[gWMNodeData].y;

            *&x2 = GM_SCREEN->x;
            *&y2 = GM_SCREEN->y;
        
            x = x1 - x2;
            y = y1 - y2 + 8;

            if ((y >= 0 && y < 0xB0) && (x >= 0 && x < 0xF0))
                proc->ap_procs[i] = APProc_Create(Sprite_08A97AEC, (s16)x, (s16)y, 0x3880, 0, 7); // Puff of smoke map sprite for skirmishes

            wm_uid = i + 4;
            SetGmClassUnit(wm_uid, proc->confs[i].jid, WM_FACTION_RED, proc->confs[i].node);
            gGMData.units[wm_uid].state |= GM_UNIT_STATE_B0;
            GmShowMuUnit(GM_MU, wm_uid);
        }
    }

    Proc_Break(proc); // Moved to the bottom to prevent crashes
    return;
}

// LYN_REPLACE_CHECK(GenerateRandomonsterMergeConf);
// int GenerateRandomonsterMergeConf(int chapter, struct GmapTimeMonsConf * out)
// {
//     // int r0; // Not used?
//     int cnt;
//     int rn;
// #ifdef NONMATCHING
//     int node;
//     u16 * ptr;
//     u8 * zeromus;
// #else
//     register int node asm("r5");
//     register u16 * ptr asm("r5");
//     register u8 * zeromus asm("r0");
// #endif
//     int i;
//     u32 idx;
//     u8 array[WM_MON_LOC_MAX];
//     u8 list[WM_MONS_AMT];
//     u16 seeds[WM_MONS_AMT];
//     const u8 * lut1;
//     const u32 rateCount = ARRAY_COUNT(WmMonsterGenerateRatesIdx_EirikaMode);

//     if (chapter >= 0)
//     {
//         if (chapter < 10 || chapter == 0x38)
//             return 0;

//         if (chapter < 0)
//             goto handle_xmap;

//         switch (gPlaySt.chapterModeIndex) {
//         case CHAPTER_MODE_COMMON:
//             for (idx = 0; idx < rateCount; ++idx)
//             {
//                 if (WmMonsterGenerateRatesIdx_EirikaMode[idx] == chapter)
//                     break;
//             }

//             if (idx < rateCount)
//             {
//                 lut1 = WmMonsterGenerateRates_EirikaMode + idx * WM_MON_LOC_MAX;
//                 break;
//             }

//             for (idx = 0; idx < rateCount; ++idx)
//             {
//                 if (WmMonsterGenerateRatesIdx_EphraimMode[idx] == chapter)
//                     break;
//             }

//             if (idx >= rateCount)
//                 return 0;

//             lut1 = WmMonsterGenerateRates_EphraimMode + idx * WM_MON_LOC_MAX;
//             break;

//         case CHAPTER_MODE_EIRIKA:
//         default:
//             for (idx = 0; idx < rateCount; ++idx)
//             {
//                 if (WmMonsterGenerateRatesIdx_EirikaMode[idx] == chapter)
//                     break;
//             }
//             // Defensive check to prevent the world map from crashing if no chapter indexes match
//             if (idx >= rateCount)
//                 return 0;

//             lut1 = WmMonsterGenerateRates_EirikaMode + idx * WM_MON_LOC_MAX;
//             break;

//         case CHAPTER_MODE_EPHRAIM:
//             for (idx = 0; idx < rateCount; ++idx) {
//                 if (WmMonsterGenerateRatesIdx_EphraimMode[idx] == chapter)
//                     break;
//             }
//             // Defensive check to prevent the world map from crashing if no chapter indexes match
//             if (idx >= rateCount)
//                 return 0;

//             lut1 = WmMonsterGenerateRates_EphraimMode + idx * WM_MON_LOC_MAX;
//             break;
//         }
//         cnt = GetWmMonsterGenAmount(idx);

//         if (cnt <= 0)
//             return 0;
//     }
//     else
//     {
//     /* xmap? */
//     handle_xmap:

//         switch (gPlaySt.chapterModeIndex) {
//         case CHAPTER_MODE_COMMON:
//             lut1 = WmMonsterGenerateRates_XmapEirika;
//             break;

//         case CHAPTER_MODE_EIRIKA:
//         default:
//             lut1 = WmMonsterGenerateRates_XmapEirika;
//             break;

//         case CHAPTER_MODE_EPHRAIM:
//             lut1 = WmMonsterGenerateRates_XmapEphraim;
//             break;
//         }
//         cnt = 3;
//     }

//     memcpy(array, lut1, sizeof(array));
//     for (i = 0; i < WM_MON_LOC_MAX; i++)
//     {
//         /* Monster will not generate at unit location */
//         if (gWMMonsterSpawnLocations[i] == gGMData.units[0].location)
//             array[i] = 0;
//     }
//     StoreRNState(seeds);
//     ptr = gGmMonsterRnState;
//     LoadRNState(ptr);

//     for (i = 0; i < cnt; i++)
//     {
//         node = GenerateRandomonsterMergeNode(array, WM_MON_LOC_MAX);
//         if (node < 0)
//             return i;
//         out[i].node = gWMMonsterSpawnLocations[node];

//         // Use the progression chapter that selected the spawn table.
//         // Node chapter IDs can be non-story placeholders in common mode.
//         GetChapterSkirmishLeaderClasses(chapter, list);
//         rn = NextRN_N(sizeof(list));
//         out[i].jid = list[rn];
//         out[i].unk2 = 0;
//         gGMData.unk_c9[i] = rn;
//         zeromus = array + node;
//         *zeromus = 0;
//     }
//     StoreRNState(gGmMonsterRnState);
//     LoadRNState(seeds);
//     return cnt;
// }

// //! FE8U = 0x080BD048
// LYN_REPLACE_CHECK(GetNextUnclearedChapter);
// u32 GetNextUnclearedChapter(void)
// {
//     int nodeId = GetNextUnclearedNode(&gGMData);

//     if (nodeId < 0)
//         return -1;

//     return WMLoc_GetChapterId(nodeId);
// }

// //! FE8U = 0x080BD014
// LYN_REPLACE_CHECK(GetNextUnclearedNode);
// int GetNextUnclearedNode(struct GMapData * worldMapData)
// {
//     int i;

//     for (i = 0; i < NODE_MAX; i++)
//     {
//         if (!(worldMapData->nodes[i].state & GM_NODE_STATE_VALID))
//         {
//             continue;
//         }

//         if (!(worldMapData->nodes[i].state & GM_NODE_STATE_CLEARED))
//         {
//             continue;
//         }

//         return i;
//     }

//     return -1;
// }

// //! FE8U = 0x080153D4
// LYN_REPLACE_CHECK(CallBeginningEvents);
// int CallBeginningEvents(void)
// {
//     const struct ChapterEventGroup* pChapterEvents = GetChapterEventDataPointer(gPlaySt.chapterIndex);

//     if (gPlaySt.chapterIndex < 0 || pChapterEvents == NULL)
//         return 0;

//     if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
//     {
//         if (pChapterEvents->beginningSceneEvents == NULL)
//             return 0;

//         CallEvent(pChapterEvents->beginningSceneEvents, 1);
//     }
//     else
//         CallEvent((u16 *)EventScr_SkirmishCommonBeginning, 1);

//     return 0;
// }

// //! FE8U = 0x080BB5E4
// LYN_REPLACE_CHECK(WMLoc_GetNextLocId);
// int WMLoc_GetNextLocId(int idx)
// {
//     const s8 * unk_08;

//     const struct GMapNodeData * node = &idx[gWMNodeData];

//     if (CheckFlag(node->unk_06))
//     {
//         unk_08 = node->unk_08 + 2;
//     }
//     else
//     {
//         unk_08 = node->unk_08;
//     }

//     switch (gPlaySt.chapterModeIndex)
//     {
//         case CHAPTER_MODE_EIRIKA:
//         default:
//             return unk_08[0];

//         case CHAPTER_MODE_EPHRAIM:
//             return unk_08[1];
//     }
// }

LYN_REPLACE_CHECK(WmMergeMonsters);
void WmMergeMonsters(void)
{
    struct ProcGmapTimeMons * proc;
    proc = Proc_Find(ProcScr_GmapTimeMons);
    if (proc)
    {
        proc->trigger = true;
        NoCashGBAPrint("ProcScr_GmapTimeMons triggered");
    }
    else
        NoCashGBAPrint("ProcScr_GmapTimeMons has not been created");
}

LYN_REPLACE_CHECK(NewGmapTimeMons);
ProcPtr NewGmapTimeMons(ProcPtr parent, int * out)
{
    struct ProcGmapTimeMons * proc;
    if (!parent)
        proc = Proc_Start(ProcScr_GmapTimeMons, PROC_TREE_3);
    else
        proc = Proc_StartBlocking(ProcScr_GmapTimeMons, parent);

    if (out)
        *out = proc->monster_amt;

    return proc;
}