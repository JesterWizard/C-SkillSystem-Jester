#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"

static const u8 gGameOptionsUiOrder_NEW[13] =
{
    [ 0] = GAME_OPTION_ANIMATION,
    [ 1] = GAME_OPTION_GAME_SPEED,
    [ 2] = GAME_OPTION_TEXT_SPEED,
    [ 3] = GAME_OPTION_TERRAIN,
    [ 4] = GAME_OPTION_UNIT,
    [ 5] = GAME_OPTION_COMBAT,
    [ 6] = GAME_OPTION_OBJECTIVE,
    [ 7] = GAME_OPTION_SUBTITLE_HELP,
    [ 8] = GAME_OPTION_AUTOCURSOR,
    [ 9] = GAME_OPTION_AUTOEND_TURNS,
    [10] = GAME_OPTION_MUSIC,
    [11] = GAME_OPTION_SOUND_EFFECTS,
    [12] = GAME_OPTION_WINDOW_COLOR,

    // [13] = GAME_OPTION_CUSTOM_1,
};

static const struct GameOption gGameOptions_NEW[] =
{
    [GAME_OPTION_ANIMATION] =
    {
        .msgId = MSG_0090, // Animation[.]
        .selectors =
        {
            { MSG_00A1, MSG_00BF, 112, 1 },
            { MSG_00A2, MSG_00C0, 127, 1 },
            { MSG_00A3, MSG_00BE, 142, 2 },
            { MSG_00A4, MSG_00C7, 165, 2 },
        },
        .icon = 0x0,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_TERRAIN] =
    {
        .msgId = MSG_0091, // Terrain[.]
        .selectors =
        {
            { MSG_00AB, MSG_00BD, 112, 2 },
            { MSG_00AB, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x02,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_UNIT] =
    {
        .msgId = MSG_0092, // Unit
        .selectors =
        {
            { MSG_00AC, MSG_00CA, 112, 3 },
            { MSG_00AD, MSG_00CB, 143, 4 },
            { MSG_00AE, MSG_00BE, 182, 2 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x04,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_AUTOCURSOR] =
    {
        .msgId = MSG_0095, // Autocursor
        .selectors =
        {
            { MSG_00B3, MSG_00BD, 112, 2 },
            { MSG_00B3, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x06,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_TEXT_SPEED] =
    {
        .msgId = MSG_0096, // Text Speed
        .selectors =
        {
            { MSG_00A7, MSG_00C3, 112, 3 },
            { MSG_00A8, MSG_00C4, 143, 3 },
            { MSG_00A9, MSG_00C5, 174, 3 },
            { MSG_00AA, MSG_00C6, 205, 2 },
        },
        .icon = 0x08,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_GAME_SPEED] =
    {
        .msgId = MSG_0097, // Game Speed
        .selectors =
        {
            { MSG_00A5, MSG_00C4, 112, 3 },
            { MSG_00A6, MSG_00C5, 143, 3 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x0a,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_MUSIC] =
    {
        .msgId = MSG_0098, // Music[.]
        .selectors =
        {
            { MSG_00B5, MSG_00BD, 112, 2 },
            { MSG_00B5, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x0c,
        .func = MusicOptionChangeHandler,
    },

    [GAME_OPTION_SOUND_EFFECTS] =
    {
        .msgId = MSG_0099, // Sound Effects[.]
        .selectors =
        {
            { MSG_00B6, MSG_00BD, 112, 2 },
            { MSG_00B6, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x0e,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_WINDOW_COLOR] =
    {
        .msgId = MSG_009A, // Window Color
        .selectors =
        {
            { MSG_00B7, MSG_00BF, 112, 1 },
            { MSG_00B7, MSG_00C0, 127, 1 },
            { MSG_00B7, MSG_00C1, 142, 1 },
            { MSG_00B7, MSG_00C2, 157, 1 },
        },
        .icon = 0x10,
        .func = WindowColorOptionChangeHandler,
    },

    [GAME_OPTION_CPU_LEVEL] =
    {
        .msgId = MSG_009B, // CPU Level[.]
        .selectors =
        {
            { MSG_00B8, MSG_00BF, 112, 1 },
            { MSG_00B8, MSG_00C0, 127, 1 },
            { MSG_00B8, MSG_00C1, 142, 1 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x12,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_COMBAT] =
    {
        .msgId = MSG_0093, // Combat
        .selectors =
        {
            { MSG_00AF, MSG_00C8, 112, 3 },
            { MSG_00B0, MSG_00C9, 143, 3 },
            { MSG_00B1, MSG_00BE, 182, 2 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x14,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SUBTITLE_HELP] =
    {
        .msgId = MSG_0094, // Subtitle Help[.]
        .selectors =
        {
            { MSG_00B2, MSG_00BD, 112, 2 },
            { MSG_00B2, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x16,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_AUTOEND_TURNS] =
    {
        .msgId = MSG_009C, // Autoend Turns[.]
        .selectors =
        {
            { MSG_00B4, MSG_00BD, 112, 2 },
            { MSG_00B4, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x18,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_UNIT_COLOR] =
    {
        .msgId = MSG_009D, // Unit Color
        .selectors =
        {
            { MSG_00B9, MSG_00BD, 112, 2 },
            { MSG_00B9, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x1a,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_OBJECTIVE] =
    {
        .msgId = MSG_009E, // Show Objective
        .selectors =
        {
            { MSG_00BA, MSG_00BD, 112, 2 },
            { MSG_00BA, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x1c,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_CONTROLLER] =
    {
        .msgId = MSG_009F, // Controller
        .selectors =
        {
            { MSG_00BB, MSG_00BD, 112, 2 },
            { MSG_00BB, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x1e,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_RANK_DISPLAY] =
    {
        .msgId = MSG_00A0, // Rank Display
        .selectors =
        {
            { MSG_00BC, MSG_00BD, 112, 2 },
            { MSG_00BC, MSG_00BE, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x20,
        .func = GenericOptionChangeHandler,
    }
};

LYN_REPLACE_CHECK(SetGameOption);
void SetGameOption(u8 index, u8 newValue)
{
    switch (index)
    {
    case GAME_OPTION_ANIMATION:
        switch (newValue)
        {
        case 0:
            gPlaySt.config.animationType = PLAY_ANIMCONF_ON;
            return;

        case 1:
            gPlaySt.config.animationType = PLAY_ANIMCONF_ON_UNIQUE_BG;
            return;

        case 2:
            gPlaySt.config.animationType = PLAY_ANIMCONF_OFF;
            return;

        case 3:
            gPlaySt.config.animationType = PLAY_ANIMCONF_SOLO_ANIM;
            return;
        }

        // fallthrough

    case GAME_OPTION_TERRAIN:
        gPlaySt.config.disableTerrainDisplay = newValue;

        break;

    case GAME_OPTION_UNIT:
        gPlaySt.config.unitDisplayType = newValue;

        break;

    case GAME_OPTION_AUTOCURSOR:
        gPlaySt.config.autoCursor = newValue;

        break;

    case GAME_OPTION_TEXT_SPEED:
        gPlaySt.config.textSpeed = newValue;

        break;

    case GAME_OPTION_GAME_SPEED:
        gPlaySt.config.gameSpeed = newValue;

        break;

    case GAME_OPTION_MUSIC:
        gPlaySt.config.disableBgm = newValue;

        break;

    case GAME_OPTION_SOUND_EFFECTS:
        gPlaySt.config.disableSoundEffects = newValue;

        break;

    case GAME_OPTION_WINDOW_COLOR:
        gPlaySt.config.windowColor = newValue;

        break;

    case GAME_OPTION_COMBAT:
        gPlaySt.config.battleForecastType = newValue;

        break;

    case GAME_OPTION_SUBTITLE_HELP:
        gPlaySt.config.noSubtitleHelp = newValue;

        break;

    case GAME_OPTION_AUTOEND_TURNS:
        gPlaySt.config.disableAutoEndTurns = newValue;

        break;

    case GAME_OPTION_UNIT_COLOR:
        gPlaySt.config.unitColor = newValue;

        break;

    case GAME_OPTION_OBJECTIVE:
        gPlaySt.config.disableGoalDisplay = newValue;

        break;

    case GAME_OPTION_CONTROLLER:
        gPlaySt.config.controller = newValue;

        break;

    case GAME_OPTION_RANK_DISPLAY:
        gPlaySt.config.rankDisplay = newValue;

        break;

    // case GAME_OPTION_CUSTOM_1:
    //     gPlaySt.config.custom_option_1 = newValue;
    //     break;

    }

    return;
}

//! FE8U: 0x080B1DE8
LYN_REPLACE_CHECK(GetGameOption);
u8 GetGameOption(u8 index)
{
    int value = 0;

    switch (index)
    {
    case GAME_OPTION_ANIMATION:
        switch (gPlaySt.config.animationType)
        {
        case PLAY_ANIMCONF_ON:
            return 0;
        case PLAY_ANIMCONF_ON_UNIQUE_BG:
            return 1;
        case PLAY_ANIMCONF_OFF:
            return 2;
        case PLAY_ANIMCONF_SOLO_ANIM:
            return 3;
        }

        // fallthrough

    case GAME_OPTION_TERRAIN:
        value = gPlaySt.config.disableTerrainDisplay;

        break;

    case GAME_OPTION_UNIT:
        value = gPlaySt.config.unitDisplayType;

        break;

    case GAME_OPTION_AUTOCURSOR:
        value = gPlaySt.config.autoCursor;

        break;

    case GAME_OPTION_TEXT_SPEED:
        value = gPlaySt.config.textSpeed;

        break;

    case GAME_OPTION_GAME_SPEED:
        value = gPlaySt.config.gameSpeed;

        break;

    case GAME_OPTION_MUSIC:
        value = gPlaySt.config.disableBgm;

        break;

    case GAME_OPTION_SOUND_EFFECTS:
        value = gPlaySt.config.disableSoundEffects;

        break;

    case GAME_OPTION_WINDOW_COLOR:
        value = gPlaySt.config.windowColor;

        break;

    case GAME_OPTION_COMBAT:
        value = gPlaySt.config.battleForecastType;

        break;

    case GAME_OPTION_SUBTITLE_HELP:
        value = gPlaySt.config.noSubtitleHelp;

        break;

    case GAME_OPTION_AUTOEND_TURNS:
        value = gPlaySt.config.disableAutoEndTurns;

        break;

    case GAME_OPTION_UNIT_COLOR:
        value = gPlaySt.config.unitColor;

        break;

    case GAME_OPTION_OBJECTIVE:
        value = gPlaySt.config.disableGoalDisplay;

        break;

    case GAME_OPTION_CONTROLLER:
        value = gPlaySt.config.controller;

        break;

    case GAME_OPTION_RANK_DISPLAY:
        value = gPlaySt.config.rankDisplay;

        break;

    // case GAME_OPTION_CUSTOM_1:
    //     value = gPlaySt.config.custom_option_1;

    //     break;
    }

    return value;
}

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

    u8 time = (GetGameClock() % 16) & 8;

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
    BG_SetPosition(BG_1, 0, gConfigUiState->bg1YOffset);
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
    Decompress(Img_ConfigUiIcons, BG_CHR_ADDR(0x200));

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
        int y = (i * 2) + 5;

        DrawGameOptionIcon(i, 5);

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
            DrawOptionValueTexts(selectedIdx, selectedIdx % 7, selectedIdx * 2 + 5);
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

            if (gGameOptionsUiOrder[gConfigUiState->selectedOptionIdx] != 0)
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
            if (gGameOptions[gGameOptionsUiOrder[gConfigUiState->selectedOptionIdx]].func != NULL)
            {
                gGameOptions[gGameOptionsUiOrder[gConfigUiState->selectedOptionIdx]].func(proc);
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

    BG_SetPosition(BG_1, 0, gConfigUiState->bg1YOffset);

    return;
}

//! FE8U: 0x080B2188
LYN_REPLACE_CHECK(PutGameOptionRow);
void PutGameOptionRow(ProcPtr proc, int selectedIdx, int c)
{
    int i;
    int textIdx;

    int y = ((selectedIdx * 2) + 5) & 0x1f;

    int yTmp = 0x20 * y;

    for (i = 0; i <= 26; i++)
    {
        gBG1TilemapBuffer[yTmp + 0x02 + i] = 0;
        gBG1TilemapBuffer[yTmp + 0x22 + i] = 0;
    }

    textIdx = k_umod(selectedIdx, 7);
    //textIdx = selectedIdx % 7; // Fucking modulo!!

    DrawGameOptionIcon(selectedIdx, 5);
    DrawGameOptionText(selectedIdx, textIdx, y);
    DrawOptionValueTexts(selectedIdx, textIdx, y);

    for (i = 0; i <= 26; i++)
    {
        gBG0TilemapBuffer[c + 0x62 + i] = 0;
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    return;
}