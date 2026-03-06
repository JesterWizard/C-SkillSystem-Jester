#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "jester_headers/custom-structs.h"


LYN_REPLACE_CHECK(InitPlayConfig);
void InitPlayConfig(int isDifficult, s8 unk) {
    CpuFill16(0, &gPlaySt, sizeof(gPlaySt));

    gPlaySt.chapterIndex = 0;

    if (isDifficult)
        gPlaySt.chapterStateBits |= PLAY_FLAG_HARD;

    // TODO: WHAT ARE THOSE
    gPlaySt.config.controller = unk;
    gPlaySt.config.animationType = 0;
    gPlaySt.config.disableTerrainDisplay = 0;
    gPlaySt.config.unitDisplayType = 0;
    gPlaySt.config.autoCursor = 0;
    gPlaySt.config.textSpeed = 1; // TODO: (DEFAULT?) TEXT SPEED DEFINITIONS
    gPlaySt.config.gameSpeed = 0;
    gPlaySt.config.disableBgm = 0;
    gPlaySt.config.disableSoundEffects = 0;
    gPlaySt.config.windowColor = 0;
    gPlaySt.config.disableAutoEndTurns = 0;
    gPlaySt.config.noSubtitleHelp = 0;
    gPlaySt.config.battleForecastType = 0;
    gPlaySt.config.debugControlRed = 0;
    gPlaySt.config.debugControlGreen = 0;
    gPlaySt.config.unitColor = 0;
    gPlaySt.config.unk41_5 = 0;
    gPlaySt.config.skill_capacity = 0;
    gPlaySt.config.casual_mode = 0;
    gPlaySt.config.talk_on_level_up = 0;
    gPlaySt.config.restore_hp_on_level_up = 0;
    gPlaySt.config.promote_enemy_on_kill = 0;
    gPlaySt.config.danger_bones = 0;
    gPlaySt.config.fast_battle_animatons = 0;
    gPlaySt.config.expanded_max_hp = 0;
    gPlaySt.config.flip_enemy_sprites = 0;
    gPlaySt.config.summons_gain_exp = 0;
    gPlaySt.config.skill_capacity = 0;
    gPlaySt.config.show_arena_opponent_in_advance = 0;
}

//! FE8U = 0x080B16DC
LYN_REPLACE_CHECK(GetSelectedOptionValue);
u8 GetSelectedOptionValue(void)
{
    return GetGameOption(gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx]);
}

static inline int GetGameOptionIconChr(int icon)
{
    return 0x200 + (icon & 0x1f) + ((icon << 1) & 0xFFC0);
}

// ! FE8U: 0x080B1700
LYN_REPLACE_CHECK(DrawGameOptionIcon);
void DrawGameOptionIcon(int selectedIdx, int yBase)
{
    int y = 0x20 * ((selectedIdx * 2 + yBase) & 0x1f);
    int icon = gGameOptions_NEW[gGameOptionsUiOrder_NEW[selectedIdx]].icon;
    int chr = GetGameOptionIconChr(icon);

    // Variable reuse seems to be required to match
    icon = TILEREF(chr, 4);

    // Loads the icons in quarters
    gBG1TilemapBuffer[TILEMAP_INDEX(2, 0) + y] = icon + 0;    // Top Left
    gBG1TilemapBuffer[TILEMAP_INDEX(3, 0) + y] = icon + 1;    // Top Right
    gBG1TilemapBuffer[TILEMAP_INDEX(2, 1) + y] = icon + 0x20; // Bottom Right
    gBG1TilemapBuffer[TILEMAP_INDEX(3, 1) + y] = icon + 0x21; // Bottom Left
}

//! FE8U: 0x080B1784
LYN_REPLACE_CHECK(DrawGameOptionHelpText);
void DrawGameOptionHelpText(void)
{
    const char * str;
    ClearText(&gConfigUiState->optionHelpText);
    str = GetStringFromIndex(gGameOptions_NEW[gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx]].selectors[GetSelectedOptionValue()].helpTextId);
    PutDrawText(&gConfigUiState->optionHelpText, TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 18), TEXT_COLOR_SYSTEM_WHITE, 0, 22, str);
}

//! FE8U = 0x080B17E4
LYN_REPLACE_CHECK(DrawGameOptionText);
void DrawGameOptionText(int selectedIdx, int textIdx, int y)
{
    const char * str;
    ClearText(&gConfigUiState->optionTexts[textIdx]);
    str = GetStringFromIndex(gGameOptions_NEW[gGameOptionsUiOrder_NEW[selectedIdx]].msgId);
    PutDrawText(&gConfigUiState->optionTexts[textIdx], TILEMAP_LOCATED(gBG1TilemapBuffer, 4, y), TEXT_COLOR_SYSTEM_WHITE, 0, 9, str);
}

//! FE8U: 0x080B1850
LYN_REPLACE_CHECK(DrawOptionValueTexts);
void DrawOptionValueTexts(int selectedIdx, int textIdx, int y)
{
    int i;

    int optionIdx = gGameOptionsUiOrder_NEW[selectedIdx];

    int x = gGameOptions_NEW[optionIdx].selectors[0].xPos / 8;

    ClearText(&gConfigUiState->valueTexts[textIdx]);

    for (i = 0; i < 4; i++)
    {
        if (gGameOptions_NEW[optionIdx].selectors[i].optionTextId == MSG_000)
        {
            break;
        }

        Text_InsertDrawString(
            &gConfigUiState->valueTexts[textIdx], gGameOptions_NEW[optionIdx].selectors[i].xPos - 112,
            (i == GetGameOption(optionIdx)) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY,
            GetStringFromIndex(gGameOptions_NEW[optionIdx].selectors[i].optionTextId));
    }

    PutText(&gConfigUiState->valueTexts[textIdx], TILEMAP_LOCATED(gBG1TilemapBuffer, x, y));

    return;
}

//! FE8U = 0x080B1938
LYN_REPLACE_CHECK(DrawConfigUiSprites);
void DrawConfigUiSprites(void)
{
    int y;

    int optionIdx = gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx];

    u8 time = (k_umod(GetGameClock(), 16)) & 8;

    CallARM_PushToSecondaryOAM(18, 8, gSprite_ConfigurationUiHeader, OAM2_CHR(0xC0) + OAM2_PAL(2));

    // current option position on screen (cur index - top index)
    y = (gConfigUiState->selectedOptionIdx - gConfigUiState->headOptionIdx) * 16 + 40;

    DisplayFrozenUiHand(16, y);

    DisplayUiHand(gGameOptions_NEW[optionIdx].selectors[GetGameOption(optionIdx)].xPos - 2, y);

    if (!(gConfigUiState->source & CONFIG_UI_SOURCE_FROMPREP) || (PrepGetDeployedUnitAmt() != 0))
    {
        if ((GetSelectedGameOption() == GAME_OPTION_ANIMATION) && (GetSelectedOptionValue() == 3))
        {
            // Draw sprite for blinking "A Press" prompt
            CallARM_PushToSecondaryOAM(192, 40, gObject_16x16, (time != 0) ? OAM2_CHR(0xCE) + OAM2_PAL(2) : OAM2_CHR(0xCC) + OAM2_PAL(2));
        }
    }

    UpdateMenuScrollBarConfig(10, gConfigUiState->bg1YOffset, gConfigUiState->maxOption, 6);

    return;
}

//! FE8U = 0x080B1A08
LYN_REPLACE_CHECK(Config_Init);
void Config_Init(struct ConfigProc * proc)
{
    int i;

    // clang-format off
    u16 bgConfig[12] =
    {
        0x0000, 0x6000, 0,
        0x0000, 0x6800, 0,
        0x0000, 0x7000, 0,
        0x8000, 0x7800, 0,
    };
    // clang-format on

    i = 0;

    SetupBackgrounds(bgConfig);

    gConfigUiState->unk_32 = 0;
    gConfigUiState->maxOption = ARRAY_COUNT(gGameOptionsUiOrder_NEW);
    gConfigUiState->selectedOptionIdx = 0;
    gConfigUiState->headOptionIdx = 0;
    gConfigUiState->bg1YOffset = 0;

    proc->moving = CONFIG_MOVE_NONE;
    proc->loadSoloAnimScreen = false;

    gConfigUiState->source &= ~CONFIG_UI_SOURCE_FROMPREP;
    gConfigUiState->source &= ~CONFIG_UI_SOURCE_FROMWM;

    ResetText();

    ApplySystemObjectsPalettes();
    LoadUiFrameGraphics();

    SetDispEnable(1, 1, 1, 1, 1);

    BG_SetPosition(BG_0, 0, 0);
   // BG_SetPosition(BG_1, 0, gConfigUiState->bg1YOffset);
    BG_SetPosition(BG_1, 0, (gConfigUiState->bg1YOffset - 40) & 0xFF);
    BG_SetPosition(BG_2, 0, 0);
    BG_SetPosition(BG_3, 0, 0);

    SetWinEnable(1, 0, 0);

    SetWin0Box(0, 40, DISPLAY_WIDTH, 136);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWOutLayers(1, 0, 1, 1, 1);

    SetBlendAlpha(14, 4);
    SetBlendTargetA(0, 0, 1, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_Fill(gBG3TilemapBuffer, 0);

    ApplyPalette(Pal_ConfigUiSprites, 4);
    ApplyPalette(Pal_ConfigUiSprites, 18);

    Decompress(Img_ConfigUiSprites, OBJ_CHR_ADDR(0xC0));
    Decompress(Img_ConfigUiIcons_NEW, BG_CHR_ADDR(0x200));

    Decompress(Tsa_ConfigUiFrame, gGenericBuffer + 0x80);
    CallARM_FillTileRect(gBG2TilemapBuffer, gGenericBuffer + 0x80, TILEREF(0x0, 1));

    ResetTextFont();

    InitText(&gConfigUiState->optionHelpText, 22);

    DrawGameOptionHelpText();

    StartMenuScrollBarExt(proc, 224, 47, 0x390 * CHR_SIZE, 1);

    InitText(&gConfigUiState->text_68, 9);
    InitText(&gConfigUiState->text_a0, 14);

    for (; i < 6; i++)
    {
        // int y = (i * 2) + 5;
        int y = (i * 2);

        // DrawGameOptionIcon(i, 5);
        DrawGameOptionIcon(i, 0);

        InitText(&gConfigUiState->optionTexts[i], 9);
        InitText(&gConfigUiState->valueTexts[i], 14);

        DrawGameOptionText(i, i, y);
        DrawOptionValueTexts(i, i, y);
    }

    StartMuralBackgroundExt(proc, NULL, 18, 2, 0);

    Proc_Start(gProcScr_DrawConfigUiSprites, proc);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);

    return;
}

//! FE8U: 0x080B1CAC
LYN_REPLACE_CHECK(MusicOptionChangeHandler);
bool MusicOptionChangeHandler(ProcPtr proc)
{
    if (GenericOptionChangeHandler(proc) == 0)
        return false;

    if (GetGameOption(gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx]) != 0)
    {
        sub_8002AC8();
        return false;
    }

    if (gConfigUiState->source & CONFIG_UI_SOURCE_FROMPREP)
    {
        StartBgm(SONG_COMBAT_PREPARATION, NULL);
        return false;
    }

    if (gConfigUiState->source & CONFIG_UI_SOURCE_FROMWM)
        UpdateWorldMapBgm();
    else
        StartMapSongBgm();

    return false;
}

//! FE8U: 0x080B1D14
LYN_REPLACE_CHECK(GenericOptionChangeHandler);
bool GenericOptionChangeHandler(ProcPtr proc)
{
    int valueChanged = false;

    int selectedIdx = gConfigUiState->selectedOptionIdx;
    u8 optionIdx = gGameOptionsUiOrder_NEW[selectedIdx];

    u8 selectedValue = GetSelectedOptionValue();

    if (gKeyStatusPtr->repeatedKeys & (DPAD_LEFT | DPAD_RIGHT))
    {
        if (gKeyStatusPtr->repeatedKeys & (DPAD_LEFT))
        {
            if (selectedValue != 0)
            {
                selectedValue--;
                SetGameOption(optionIdx, selectedValue);

                valueChanged = true;
            }
        }
        else // if (gKeyStatusPtr->repeatedKeys & (DPAD_RIGHT))
        {
            if (gGameOptions_NEW[optionIdx].selectors[selectedValue + 1].optionTextId != 0)
            {
                if (selectedValue < 3)
                {
                    selectedValue++;
                    SetGameOption(optionIdx, selectedValue);

                    valueChanged = true;
                }
            }
        }

        if (valueChanged)
        {
            Proc_Start(gProcScr_RedrawConfigHelpText, proc);
            DrawOptionValueTexts(selectedIdx, k_umod(selectedIdx, 7), selectedIdx * 2);
            // DrawOptionValueTexts(selectedIdx, k_umod(selectedIdx, 7), selectedIdx * 2 + 5);
            BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
        }
    }

    return valueChanged;
}

//! FE8U: 0x080B220C
LYN_REPLACE_CHECK(Config_Loop_KeyHandler);
void Config_Loop_KeyHandler(struct ConfigProc * proc)
{
    bool valueChanged = false;

    switch (proc->moving)
    {
    case CONFIG_MOVE_NONE:
        if (gKeyStatusPtr->newKeys & (B_BUTTON))
        {
            PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
            Proc_Break(proc);

            break;
        }
        else if (gKeyStatusPtr->newKeys & (A_BUTTON))
        {
            if ((gConfigUiState->source & CONFIG_UI_SOURCE_FROMPREP) && (PrepGetDeployedUnitAmt() == 0))
            {
                break;
            }

            if (gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx] != 0)
            {
                break;
            }

            if (GetGameOption(GAME_OPTION_ANIMATION) != 3)
            {
                break;
            }

            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
            proc->loadSoloAnimScreen = true;
            Proc_Break(proc);

            break;
        }
        else if (gKeyStatusPtr->repeatedKeys & (DPAD_UP | DPAD_DOWN))
        {
            if (gKeyStatusPtr->repeatedKeys & (DPAD_UP))
            {
                if (gConfigUiState->selectedOptionIdx != 0)
                {
                    gConfigUiState->selectedOptionIdx--;

                    if ((gConfigUiState->selectedOptionIdx - gConfigUiState->headOptionIdx < 1) && (gConfigUiState->headOptionIdx != 0))
                    {
                        gConfigUiState->headOptionIdx--;

                        PutGameOptionRow(proc, gConfigUiState->selectedOptionIdx - 1, 0);

                        gConfigUiState->bg1YOffset -= 4;
                        proc->moving = CONFIG_MOVE_UP;
                    }

                    valueChanged = true;
                }
            }
            else // if (gKeyStatusPtr->repeatedKeys & (DPAD_DOWN))
            {
                if (gConfigUiState->selectedOptionIdx < gConfigUiState->maxOption - 1)
                {
                    gConfigUiState->selectedOptionIdx++;

                    if ((gConfigUiState->selectedOptionIdx - gConfigUiState->headOptionIdx > 4) &&
                        (gConfigUiState->selectedOptionIdx < gConfigUiState->maxOption - 1))
                    {
                        gConfigUiState->headOptionIdx++;

                        PutGameOptionRow(proc, gConfigUiState->selectedOptionIdx + 1, 320);

                        gConfigUiState->bg1YOffset += 4;
                        proc->moving = CONFIG_MOVE_DOWN;
                    }

                    valueChanged = true;
                }
            }

            if (valueChanged)
            {
                Proc_Start(gProcScr_RedrawConfigHelpText, proc);
                BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

                break;
            }
        }

        if (gKeyStatusPtr->newKeys & (DPAD_LEFT | DPAD_RIGHT))
        {
            if (gGameOptions_NEW[gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx]].func != NULL)
            {
                gGameOptions_NEW[gGameOptionsUiOrder_NEW[gConfigUiState->selectedOptionIdx]].func(proc);
            }
        }

        break;

    case CONFIG_MOVE_UP:
    case CONFIG_MOVE_UP_FRAME_2:
    case CONFIG_MOVE_UP_FRAME_3:
        // Moving up (duration of 3 frames)

        gConfigUiState->bg1YOffset -= 4;

        if (proc->moving == CONFIG_MOVE_UP_FRAME_3)
        {
            proc->moving = CONFIG_MOVE_NONE;
        }
        else
        {
            proc->moving++;
        }

        break;

    case CONFIG_MOVE_DOWN:
    case CONFIG_MOVE_DOWN_FRAME_2:
    case CONFIG_MOVE_DOWN_FRAME_3:
        // Moving down (duration of 3 frames)

        gConfigUiState->bg1YOffset += 4;

        if (proc->moving == CONFIG_MOVE_DOWN_FRAME_3)
        {
            proc->moving = CONFIG_MOVE_NONE;
        }
        else
        {
            proc->moving++;
        }

        break;
    }

    // BG_SetPosition(BG_1, 0, gConfigUiState->bg1YOffset);
    BG_SetPosition(BG_1, 0, (gConfigUiState->bg1YOffset - 40) & 0xFF);

    return;
}

//! FE8U: 0x080B2188
LYN_REPLACE_CHECK(PutGameOptionRow);
void PutGameOptionRow(ProcPtr proc, int selectedIdx, int c)
{
    int i;
    int textIdx;

    // int y = ((selectedIdx * 2) + 5) & 0x1f;
    int y = (selectedIdx * 2) & 0x1f;

    int yTmp = 0x20 * y;

    for (i = 0; i <= 26; i++)
    {
        gBG1TilemapBuffer[yTmp + 0x02 + i] = 0;
        gBG1TilemapBuffer[yTmp + 0x22 + i] = 0;
    }

    textIdx = k_umod(selectedIdx, 7);
    //textIdx = selectedIdx % 7; // Fucking modulo!!

    // DrawGameOptionIcon(selectedIdx, 5);
    DrawGameOptionIcon(selectedIdx, 0);
    DrawGameOptionText(selectedIdx, textIdx, y);
    DrawOptionValueTexts(selectedIdx, textIdx, y);

    for (i = 0; i <= 26; i++)
    {
        gBG0TilemapBuffer[c + 0x62 + i] = 0;
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    return;
}