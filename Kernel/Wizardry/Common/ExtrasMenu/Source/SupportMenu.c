#include "common-chax.h"
#include "bmmenu.h"
#include "jester_headers/custom-arrays.h"

LYN_REPLACE_CHECK(GetGlobalSupportListFromSave);
void GetGlobalSupportListFromSave(int unitId, u8* data, struct GlobalSaveInfo* info)
{
    struct GlobalSaveInfo local_info;
    struct SupportTalkEnt* ptr;
    int i;
    int j;

    if (gCharacterData_NEW[unitId-1].pSupportData == 0) {
        for (i = 0; i < UNIT_SUPPORT_MAX_COUNT; data++, i++)
            *data = 0;

        return;
    }

    j = 0;
    ptr = GetSupportTalkList();

    if (info == NULL) {
        info = &local_info;
        ReadGlobalSaveInfo(info);
    }

    for (; ; j++, ptr++) {
        // int tmp1, tmp2;

        if (ptr->unitA == 0xFFFF)
            break;

        if ((ptr->unitA != unitId) && (ptr->unitB != unitId))
            continue;

        // tmp1 = j >> 2;
        // tmp2 = (j & 3) << 1;

        for (i = 0; i < gCharacterData_NEW[unitId-1].pSupportData->supportCount; i++) {

            if ((ptr->unitA != gCharacterData_NEW[unitId-1].pSupportData->characters[i]) &&
                (ptr->unitB != gCharacterData_NEW[unitId-1].pSupportData->characters[i])) {
                continue;
            }

            data[i] = 3;

            break;
        }
    }

    for (i = gCharacterData_NEW[unitId-1].pSupportData->supportCount; i < UNIT_SUPPORT_MAX_COUNT; i++) {
        data[i] = 0;
    }

    return;
}

//! FE8U = 0x080AB89C
LYN_REPLACE_CHECK(InitSaveMenuChoice);
void InitSaveMenuChoice(struct SaveMenuProc * proc)
{
    int i;

    int count = 0;

    proc->unk_31 = 0;
    proc->main_options = 0;

    if (proc->unk_44 == 0x100)
        AddMainMenuOption(proc, MAIN_MENU_OPTION_RESUME);

    for (i = 0; i < 3; i++)
        if (proc->chapter_idx[i] != (u8)-1)
            count++;

    if (count > 0)
    {
        AddMainMenuOption(proc, MAIN_MENU_OPTION_RESTART);

        if (count < 3)
            AddMainMenuOption(proc, MAIN_MENU_OPTION_COPY);
        AddMainMenuOption(proc, MAIN_MENU_OPTION_ERASE);
    }

    if (count < 3)
        AddMainMenuOption(proc, MAIN_MENU_OPTION_NEW_GAME);

    proc->extra_options = 0;
    proc->max_choice = 0;

    if (IsExtraLinkArenaEnabled(0) != 0)
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_LINK_ARENA);

    if (IsExtraSoundRoomEnabled() != 0)
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_SOUND_ROOM);

#ifndef CONFIG_UNLOCK_ALL_SUPPORTS
    if (IsExtraSupportViewerEnabled() != 0)
#endif
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_SUPPORT);

    if (IsExtraFreeMapEnabled() != 0)
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_MAP);

    if (IsExtraBonusClaimEnabled() != 0)
        AddExtraMenuOption(proc, EXTRA_MENU_OPTION_BONUS_CLAIM);

    if (proc->extra_options != 0)
    {
        proc->main_options |= MAIN_MENU_OPTION_EXTRAS;
        proc->unk_31++;
    }
}

LYN_REPLACE_CHECK(GGM_IsCharacterKnown);
bool GGM_IsCharacterKnown(int index, struct GlobalSaveInfo *buf)
{
#ifdef CONFIG_UNLOCK_ALL_SUPPORTS
    return 1;
#endif
    struct GlobalSaveInfo tmp_header;
    u32 _index = index;

    if (index > 0x100)
        return 0;

    if (0 == buf) {
        buf = &tmp_header;
        ReadGlobalSaveInfo(&tmp_header);
    }

    if (1 & buf->charKnownFlags[index >> 3] >> (_index % 8))
        return 1;
    else
        return 0;
}