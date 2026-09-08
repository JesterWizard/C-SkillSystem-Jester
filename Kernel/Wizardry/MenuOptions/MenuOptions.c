#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"

extern const struct GameOption gGameOptions_NEW[];
extern u8 Img_ConfigUiIcons_NEW[];
extern void SetAchievementsTo(int id);

const u8 gGameOptionsUiOrder_NEW[33] = {
    GAME_OPTION_ANIMATION, 
    GAME_OPTION_GAME_SPEED, 
    GAME_OPTION_TEXT_SPEED, 
    GAME_OPTION_TERRAIN,
    GAME_OPTION_UNIT, 
    GAME_OPTION_COMBAT, 
    GAME_OPTION_OBJECTIVE, 
    GAME_OPTION_SUBTITLE_HELP,
    GAME_OPTION_AUTOCURSOR, 
    GAME_OPTION_AUTOEND_TURNS, 
    GAME_OPTION_MUSIC, 
    GAME_OPTION_SOUND_EFFECTS,
    GAME_OPTION_WINDOW_COLOR, 

    // CUSTOM
    GAME_OPTION_SKILL_CAPACITY,
    GAME_OPTION_CASUAL_MODE,
    GAME_OPTION_TALK_ON_LEVEL_UP,
    GAME_OPTION_RESTORE_HP_ON_LEVEL_UP,
    GAME_OPTION_PROMOTE_ENEMY_ON_KILL,
    GAME_OPTION_DANGER_BONES,
    GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS,
    GAME_OPTION_EXPANDED_MAX_HP,
    GAME_OPTION_FLIPPED_ENEMY_SPRITES,
    GAME_OPTION_SUMMONS_GAIN_EXP,
    GAME_OPTION_PROMOTE_ON_MAX_LEVEL,
    GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE,
    GAME_OPTION_SEND_INVENTORY_ON_DEATH,
    GAME_OPTION_FAST_MAP_ANIMATIONS,
    GAME_OPTION_TUTORIALS,
    GAME_OPTION_SUPPORT_AFTER_BATTLE,
    GAME_OPTION_REAL_TIME_BATTLE,
    GAME_OPTION_REAL_TIME_INTERVAL,
    GAME_OPTION_SHOW_TRUE_2RN,
    GAME_OPTION_ACHIEVEMENTS,
};

static bool IsGameOptionUiVisible(u8 optionIdx)
{
    if (optionIdx == GAME_OPTION_ACHIEVEMENTS
        && gpKernelDesignerConfig->vesly_achievements != true)
        return false;

    if (optionIdx == GAME_OPTION_SHOW_TRUE_2RN
        && gpKernelDesignerConfig->show_true_2rn != true)
        return false;

    return true;
}

static u8 GetUiGameOptionAt(int visibleIdx)
{
    int i;
    int visible = 0;

    for (i = 0; i < (int)ARRAY_COUNT(gGameOptionsUiOrder_NEW); i++) {
        u8 opt = gGameOptionsUiOrder_NEW[i];

        if (!IsGameOptionUiVisible(opt))
            continue;

        if (visible == visibleIdx)
            return opt;

        visible++;
    }

    return gGameOptionsUiOrder_NEW[0];
}

static int GetGameOptionIndexCount(void)
{
    int i;
    int count = 0;

    for (i = 0; i < (int)ARRAY_COUNT(gGameOptionsUiOrder_NEW); i++) {
        if (IsGameOptionUiVisible(gGameOptionsUiOrder_NEW[i]))
            count++;
    }

    return count;
}

static int GetGameOptionRowCount(int optionIdx)
{
    int i;

    if (optionIdx == GAME_OPTION_REAL_TIME_INTERVAL)
        return 4;

    if (optionIdx >= GAME_OPTION_SKILL_CAPACITY)
        return 2;

    for (i = 0; i < 4; ++i) {
        if (gGameOptions_NEW[optionIdx].selectors[i].optionTextId == MSG_000)
            break;
    }

    return i;
}

static const char *GetGameOptionRowTitle(int optionIdx)
{
    if (optionIdx >= GAME_OPTION_SKILL_CAPACITY)
        return GetStringFromIndex(gGameOptions_NEW[optionIdx].msgId);

    return GetStringFromIndex(gGameOptions_NEW[optionIdx].msgId);
}

static const char *GetGameOptionRowHelpText(int optionIdx, int value)
{
    if (optionIdx >= GAME_OPTION_SKILL_CAPACITY)
        return GetStringFromIndex(gGameOptions_NEW[optionIdx].selectors[value].helpTextId);

    return GetStringFromIndex(gGameOptions_NEW[optionIdx].selectors[value].helpTextId);
}

static const char *GetGameOptionRowValueText(int optionIdx, int value)
{
    if (optionIdx == GAME_OPTION_REAL_TIME_INTERVAL)
        return GetStringFromIndex(gGameOptions_NEW[optionIdx].selectors[value].optionTextId);

    if (optionIdx >= GAME_OPTION_SKILL_CAPACITY)
        return value ? "OFF" : "ON";

    return GetStringFromIndex(gGameOptions_NEW[optionIdx].selectors[value].optionTextId);
}

static int GetGameOptionRowX(int optionIdx)
{
    if (optionIdx == GAME_OPTION_REAL_TIME_INTERVAL)
        return gGameOptions_NEW[optionIdx].selectors[0].xPos;

    if (optionIdx >= GAME_OPTION_SKILL_CAPACITY)
        return 112;

    return gGameOptions_NEW[optionIdx].selectors[0].xPos;
}

const struct GameOption gGameOptions_NEW[] =
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
    },
 [GAME_OPTION_SKILL_CAPACITY] =
    {
        .msgId = MSG_MENU_OPTION_SKILL_CAPACITY_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SKILL_CAPACITY_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SKILL_CAPACITY_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x22,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_CASUAL_MODE] =
    {
        .msgId = MSG_MENU_OPTION_CASUAL_MODE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_CASUAL_MODE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_CASUAL_MODE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x20,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_TALK_ON_LEVEL_UP] =
    {
        .msgId = MSG_MENU_OPTION_TALK_ON_LEVEL_UP_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_TALK_ON_LEVEL_UP_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_TALK_ON_LEVEL_UP_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x24,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_RESTORE_HP_ON_LEVEL_UP] =
    {
        .msgId = MSG_MENU_OPTION_RESTORE_HP_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_RESTORE_HP_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_RESTORE_HP_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x26,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_PROMOTE_ENEMY_ON_KILL] =
    {
        .msgId = MSG_MENU_OPTION_PROMOTE_ON_KILL_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_PROMOTE_ON_KILL_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_PROMOTE_ON_KILL_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x28,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_DANGER_BONES] =
    {
        .msgId = MSG_MENU_OPTION_DANGER_BONES_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_DANGER_BONES_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_DANGER_BONES_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x2A,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS] =
    {
        .msgId = MSG_MENU_OPTION_FAST_BATTLE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_FAST_BATTLE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_FAST_BATTLE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x2C,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_EXPANDED_MAX_HP] =
    {
        .msgId = MSG_MENU_OPTION_EXPANDED_HP_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_EXPANDED_HP_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_EXPANDED_HP_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x2E,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_FLIPPED_ENEMY_SPRITES] =
    {
        .msgId = MSG_MENU_OPTION_FLIPPED_ENEMY_SPRITES_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_FLIPPED_ENEMY_SPRITES_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_FLIPPED_ENEMY_SPRITES_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x30,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SUMMONS_GAIN_EXP] =
    {
        .msgId = MSG_MENU_OPTION_SUMMONS_GAIN_EXP_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SUMMONS_GAIN_EXP_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SUMMONS_GAIN_EXP_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x32,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_PROMOTE_ON_MAX_LEVEL] =
    {
        .msgId = MSG_MENU_OPTION_PROMOTE_ON_MAX_LEVEL_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_PROMOTE_ON_MAX_LEVEL_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_PROMOTE_ON_MAX_LEVEL_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x34,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE] =
    {
        .msgId = MSG_MENU_OPTION_SHOW_ARENA_OPPONENT_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SHOW_ARENA_OPPONENT_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SHOW_ARENA_OPPONENT_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x36,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SEND_INVENTORY_ON_DEATH] =
    {
        .msgId = MSG_MENU_OPTION_SEND_INVENTORY_ON_DEATH_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SEND_INVENTORY_ON_DEATH_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SEND_INVENTORY_ON_DEATH_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x38,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_FAST_MAP_ANIMATIONS] =
    {
        .msgId = MSG_MENU_OPTION_FAST_MAP_ANIMATIONS_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_FAST_MAP_ANIMATIONS_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_FAST_MAP_ANIMATIONS_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x3A,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_TUTORIALS] =
    {
        .msgId = MSG_MENU_OPTION_TUTORIALS_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_TUTORIALS_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_TUTORIALS_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x3C,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SUPPORT_AFTER_BATTLE] =
    {
        .msgId = MSG_MENU_OPTION_SUPPORT_AFTER_BATTLE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SUPPORT_AFTER_BATTLE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SUPPORT_AFTER_BATTLE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x3E,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_REAL_TIME_BATTLE] =
    {
        .msgId = MSG_MENU_OPTION_REAL_TIME_BATTLE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_REAL_TIME_BATTLE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_REAL_TIME_BATTLE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x40,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_REAL_TIME_INTERVAL] =
    {
        .msgId = MSG_MENU_OPTION_REAL_TIME_INTERVAL_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_REAL_TIME_INTERVAL_DESC, MSG_MENU_OPTION_REAL_TIME_1S, 112, 2 },
            { MSG_MENU_OPTION_REAL_TIME_INTERVAL_DESC, MSG_MENU_OPTION_REAL_TIME_2S, 135, 2 },
            { MSG_MENU_OPTION_REAL_TIME_INTERVAL_DESC, MSG_MENU_OPTION_REAL_TIME_3S, 158, 2 },
            { MSG_MENU_OPTION_REAL_TIME_INTERVAL_DESC, MSG_MENU_OPTION_REAL_TIME_5S, 181, 2 },
        },
        .icon = 0x42,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_ACHIEVEMENTS] =
    {
        .msgId = MSG_MENU_OPTION_ACHIEVEMENTS_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_ACHIEVEMENTS_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_ACHIEVEMENTS_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x44,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_SHOW_TRUE_2RN] =
    {
        .msgId = MSG_MENU_OPTION_SHOW_TRUE_2RN_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_SHOW_TRUE_2RN_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_SHOW_TRUE_2RN_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x14, /* reuse Combat icon */
        .func = GenericOptionChangeHandler,
    },
};

LYN_REPLACE_CHECK(SetGameOption);
void SetGameOption(u8 index, u8 newValue) {
    switch (index) {
        // Condensed switch case formatting
        case GAME_OPTION_ANIMATION:
            if (newValue == 0) gPlaySt.config.animationType = PLAY_ANIMCONF_ON;
            else if (newValue == 1) gPlaySt.config.animationType = PLAY_ANIMCONF_ON_UNIQUE_BG;
            else if (newValue == 2) gPlaySt.config.animationType = PLAY_ANIMCONF_OFF;
            else if (newValue == 3) gPlaySt.config.animationType = PLAY_ANIMCONF_SOLO_ANIM;
            return;
        case GAME_OPTION_TERRAIN:                        gPlaySt.config.disableTerrainDisplay = newValue; break;
        case GAME_OPTION_UNIT:                           gPlaySt.config.unitDisplayType = newValue; break;
        case GAME_OPTION_AUTOCURSOR:                     gPlaySt.config.autoCursor = newValue; break;
        case GAME_OPTION_TEXT_SPEED:                     gPlaySt.config.textSpeed = newValue; break;
        case GAME_OPTION_GAME_SPEED:                     gPlaySt.config.gameSpeed = newValue; break;
        case GAME_OPTION_MUSIC:                          gPlaySt.config.disableBgm = newValue; break;
        case GAME_OPTION_SOUND_EFFECTS:                  gPlaySt.config.disableSoundEffects = newValue; break;
        case GAME_OPTION_WINDOW_COLOR:                   gPlaySt.config.windowColor = newValue; break;
        case GAME_OPTION_COMBAT:                         gPlaySt.config.battleForecastType = newValue; break;
        case GAME_OPTION_SUBTITLE_HELP:                  gPlaySt.config.noSubtitleHelp = newValue; break;
        case GAME_OPTION_AUTOEND_TURNS:                  gPlaySt.config.disableAutoEndTurns = newValue; break;
        case GAME_OPTION_UNIT_COLOR:                     gPlaySt.config.unitColor = newValue; break;
        case GAME_OPTION_OBJECTIVE:                      gPlaySt.config.disableGoalDisplay = newValue; break;
        case GAME_OPTION_CONTROLLER:                     gPlaySt.config.controller = newValue; break;
        case GAME_OPTION_RANK_DISPLAY:                   gPlaySt.config.rankDisplay = newValue; break;
        case GAME_OPTION_SKILL_CAPACITY:                 gPlaySt.config.skill_capacity = newValue; break;
        case GAME_OPTION_CASUAL_MODE:                    gPlaySt.config.casual_mode = newValue; break;
        case GAME_OPTION_TALK_ON_LEVEL_UP:               gPlaySt.config.talk_on_level_up = newValue; break;
        case GAME_OPTION_RESTORE_HP_ON_LEVEL_UP:         gPlaySt.config.restore_hp_on_level_up = newValue; break;
        case GAME_OPTION_PROMOTE_ENEMY_ON_KILL:          gPlaySt.config.promote_enemy_on_kill = newValue; break;
        case GAME_OPTION_DANGER_BONES:                   gPlaySt.config.danger_bones = newValue; break;
        case GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS:  gPlaySt.config.fast_battle_animations = newValue; break;
        case GAME_OPTION_EXPANDED_MAX_HP:                gPlaySt.config.expanded_max_hp = newValue; break;
        case GAME_OPTION_FLIPPED_ENEMY_SPRITES:          gPlaySt.config.flip_enemy_sprites = newValue; break;
        case GAME_OPTION_SUMMONS_GAIN_EXP:               gPlaySt.config.summons_gain_exp = newValue; break;
        case GAME_OPTION_PROMOTE_ON_MAX_LEVEL:           gPlaySt.config.promote_unit_on_max_level = newValue; break;
        case GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE: gPlaySt.config.show_arena_opponent_in_advance = newValue; break;
        case GAME_OPTION_SEND_INVENTORY_ON_DEATH:        gPlaySt.config.send_inventory_on_death = newValue; break;
        case GAME_OPTION_FAST_MAP_ANIMATIONS:            gPlaySt.config.fast_map_animations= newValue; break;
        case GAME_OPTION_TUTORIALS:                      gPlaySt.config.show_tutorial = newValue; break;
        case GAME_OPTION_SUPPORT_AFTER_BATTLE:           gPlaySt.config.support_after_battle = newValue; break;
        case GAME_OPTION_REAL_TIME_BATTLE:               gPlaySt.config.real_time_battle = newValue; break;
        case GAME_OPTION_REAL_TIME_INTERVAL:             gPlaySt.config.real_time_interval = newValue; break;
        case GAME_OPTION_SHOW_TRUE_2RN:
            if (gpKernelDesignerConfig->show_true_2rn != true) {
                gPlaySt.config.show_true_2rn = 1; /* OFF */
                break;
            }
            gPlaySt.config.show_true_2rn = newValue;
            break;
        case GAME_OPTION_ACHIEVEMENTS:
            if (gpKernelDesignerConfig->vesly_achievements != true) {
                gPlaySt.config.achievements = 1; /* OFF */
                SetAchievementsTo(0);
                break;
            }
            gPlaySt.config.achievements = newValue;
            /* Keep Vesly Achievements flag in sync: SetAchievementsTo(1)=ON. */
            SetAchievementsTo(newValue == 0);
            break;
    }
}

//! FE8U: 0x080B1DE8
LYN_REPLACE_CHECK(GetGameOption);
u8 GetGameOption(u8 index) {
    switch (index) {
        case GAME_OPTION_ANIMATION:
            if (gPlaySt.config.animationType == PLAY_ANIMCONF_ON) return 0;
            if (gPlaySt.config.animationType == PLAY_ANIMCONF_ON_UNIQUE_BG) return 1;
            if (gPlaySt.config.animationType == PLAY_ANIMCONF_OFF) return 2;
            if (gPlaySt.config.animationType == PLAY_ANIMCONF_SOLO_ANIM) return 3;
            return 0; // Default
        case GAME_OPTION_TERRAIN:                        return gPlaySt.config.disableTerrainDisplay;
        case GAME_OPTION_UNIT:                           return gPlaySt.config.unitDisplayType;
        case GAME_OPTION_AUTOCURSOR:                     return gPlaySt.config.autoCursor;
        case GAME_OPTION_TEXT_SPEED:                     return gPlaySt.config.textSpeed;
        case GAME_OPTION_GAME_SPEED:                     return gPlaySt.config.gameSpeed;
        case GAME_OPTION_MUSIC:                          return gPlaySt.config.disableBgm;
        case GAME_OPTION_SOUND_EFFECTS:                  return gPlaySt.config.disableSoundEffects;
        case GAME_OPTION_WINDOW_COLOR:                   return gPlaySt.config.windowColor;
        case GAME_OPTION_COMBAT:                         return gPlaySt.config.battleForecastType;
        case GAME_OPTION_SUBTITLE_HELP:                  return gPlaySt.config.noSubtitleHelp;
        case GAME_OPTION_AUTOEND_TURNS:                  return gPlaySt.config.disableAutoEndTurns;
        case GAME_OPTION_UNIT_COLOR:                     return gPlaySt.config.unitColor;
        case GAME_OPTION_OBJECTIVE:                      return gPlaySt.config.disableGoalDisplay;
        case GAME_OPTION_CONTROLLER:                     return gPlaySt.config.controller;
        case GAME_OPTION_RANK_DISPLAY:                   return gPlaySt.config.rankDisplay;
        case GAME_OPTION_SKILL_CAPACITY:                 return gPlaySt.config.skill_capacity;
        case GAME_OPTION_CASUAL_MODE:                    return gPlaySt.config.casual_mode;
        case GAME_OPTION_TALK_ON_LEVEL_UP:               return gPlaySt.config.talk_on_level_up;
        case GAME_OPTION_RESTORE_HP_ON_LEVEL_UP:         return gPlaySt.config.restore_hp_on_level_up;
        case GAME_OPTION_PROMOTE_ENEMY_ON_KILL:          return gPlaySt.config.promote_enemy_on_kill;
        case GAME_OPTION_DANGER_BONES:                   return gPlaySt.config.danger_bones;
        case GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS:  return gPlaySt.config.fast_battle_animations;
        case GAME_OPTION_EXPANDED_MAX_HP:                return gPlaySt.config.expanded_max_hp;
        case GAME_OPTION_FLIPPED_ENEMY_SPRITES:          return gPlaySt.config.flip_enemy_sprites;
        case GAME_OPTION_SUMMONS_GAIN_EXP:               return gPlaySt.config.summons_gain_exp;
        case GAME_OPTION_PROMOTE_ON_MAX_LEVEL:           return gPlaySt.config.promote_unit_on_max_level;
        case GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE: return gPlaySt.config.show_arena_opponent_in_advance;
        case GAME_OPTION_SEND_INVENTORY_ON_DEATH:        return gPlaySt.config.send_inventory_on_death;
        case GAME_OPTION_FAST_MAP_ANIMATIONS:            return gPlaySt.config.fast_map_animations;
        case GAME_OPTION_TUTORIALS:                      return gPlaySt.config.show_tutorial;
        case GAME_OPTION_SUPPORT_AFTER_BATTLE:           return gPlaySt.config.support_after_battle;
        case GAME_OPTION_REAL_TIME_BATTLE:               return gPlaySt.config.real_time_battle;
        case GAME_OPTION_REAL_TIME_INTERVAL:             return gPlaySt.config.real_time_interval;
        case GAME_OPTION_SHOW_TRUE_2RN:
            if (gpKernelDesignerConfig->show_true_2rn != true)
                return 1; /* OFF */
            return gPlaySt.config.show_true_2rn;
        case GAME_OPTION_ACHIEVEMENTS:
            if (gpKernelDesignerConfig->vesly_achievements != true)
                return 1; /* OFF */
            return gPlaySt.config.achievements;
    }
    return 0;
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
    gPlaySt.config.skill_capacity = 0;
    gPlaySt.config.casual_mode = 0;
    gPlaySt.config.talk_on_level_up = 0;
    gPlaySt.config.restore_hp_on_level_up = 0;
    gPlaySt.config.promote_enemy_on_kill = 0;
    gPlaySt.config.danger_bones = 0;
    gPlaySt.config.fast_battle_animations = 0;
    gPlaySt.config.expanded_max_hp = 0;
    gPlaySt.config.flip_enemy_sprites = 0;
    gPlaySt.config.summons_gain_exp = 0;
    gPlaySt.config.skill_capacity = 0;
    gPlaySt.config.show_arena_opponent_in_advance = 0;
    gPlaySt.config.promote_unit_on_max_level = 0;
    gPlaySt.config.send_inventory_on_death = 0;
    gPlaySt.config.fast_map_animations = 0;
    gPlaySt.config.show_tutorial = 0;
    gPlaySt.config.support_after_battle = 0;
    gPlaySt.config.gameover_quotes = 0;
    /* Menu: 0 = ON. Match designer-config so new games inherit the build default. */
    gPlaySt.config.real_time_battle = gpKernelDesignerConfig->real_time_battle ? 0 : 1;
    gPlaySt.config.real_time_interval = 0; /* display hint; interval frames come from designer config */
    gPlaySt.config.show_true_2rn = 1; /* player default OFF; menu hidden if designer gate is off */
    /* Menu: 0 = ON. Match designer-config so new games inherit the build default. */
    gPlaySt.config.achievements = gpKernelDesignerConfig->vesly_achievements ? 0 : 1;
    SetAchievementsTo(gpKernelDesignerConfig->vesly_achievements ? 1 : 0);
}

void ChapterInit_SyncAchievementsOption(void)
{
    if (gpKernelDesignerConfig->vesly_achievements != true) {
        gPlaySt.config.achievements = 1; /* OFF */
        SetAchievementsTo(0);
        return;
    }

    SetAchievementsTo(gPlaySt.config.achievements == 0);
}

//! FE8U = 0x080B16DC
LYN_REPLACE_CHECK(GetSelectedOptionValue);
u8 GetSelectedOptionValue(void)
{
    return GetGameOption(GetUiGameOptionAt(gConfigUiState->selectedOptionIdx));
}

static inline int GetGameOptionIconChr(int icon)
{
    return 0x200 + (icon & 0x1f) + ((icon << 1) & 0xFFC0);
}

// ! FE8U: 0x080B1700
LYN_REPLACE_CHECK(DrawGameOptionIcon);
void DrawGameOptionIcon(int selectedIdx, int yBase)
{
    int yTop = 0x20 * ((selectedIdx * 2 + yBase) & 0x1f);
    int yBot = 0x20 * ((selectedIdx * 2 + yBase + 1) & 0x1f);
    int icon = gGameOptions_NEW[GetUiGameOptionAt(selectedIdx)].icon;
    int chr = GetGameOptionIconChr(icon);

    // Variable reuse seems to be required to match
    icon = TILEREF(chr, 4);

    // Loads the icons in quarters
    gBG1TilemapBuffer[TILEMAP_INDEX(2, 0) + yTop] = icon + 0;    // Top Left
    gBG1TilemapBuffer[TILEMAP_INDEX(3, 0) + yTop] = icon + 1;    // Top Right
    gBG1TilemapBuffer[TILEMAP_INDEX(2, 0) + yBot] = icon + 0x20; // Bottom Left
    gBG1TilemapBuffer[TILEMAP_INDEX(3, 0) + yBot] = icon + 0x21; // Bottom Right
}

//! FE8U: 0x080B1784
LYN_REPLACE_CHECK(DrawGameOptionHelpText);
void DrawGameOptionHelpText(void)
{
    const char * str;
    ClearText(&gConfigUiState->optionHelpText);
    str = GetGameOptionRowHelpText(GetUiGameOptionAt(gConfigUiState->selectedOptionIdx), GetSelectedOptionValue());
    PutDrawText(&gConfigUiState->optionHelpText, TILEMAP_LOCATED(gBG0TilemapBuffer, 4, 18), TEXT_COLOR_SYSTEM_WHITE, 0, 28, str);
}

/**
 * PutText writes bottom glyphs at dest+0x20 with no Y wrap. Option idx 13/29
 * use tile row 31; place those with an explicit wrap. All other rows use vanilla.
 */
static void PutTextYWrapped(struct Text * text, u16 * bg, int x, int y)
{
    int i;
    u16 tmScratch[0x40];
    int yTop = y & 0x1f;
    int yBot = (y + 1) & 0x1f;
    u16 * destTop;
    u16 * destBot;

    if (yTop != 0x1f)
    {
        PutText(text, bg + TILEMAP_INDEX(x, yTop));
        return;
    }

    destTop = bg + TILEMAP_INDEX(x, yTop);
    destBot = bg + TILEMAP_INDEX(x, yBot);

    CpuFill16(0, tmScratch, sizeof(tmScratch));
    PutText(text, tmScratch);

    for (i = 0; i < text->tile_width; i++)
    {
        destTop[i] = tmScratch[i];
        destBot[i] = tmScratch[0x20 + i];
    }
}

//! FE8U = 0x080B17E4
LYN_REPLACE_CHECK(DrawGameOptionText);
void DrawGameOptionText(int selectedIdx, int textIdx, int y)
{
    const char * str;

    y &= 0x1f;

    ClearText(&gConfigUiState->optionTexts[textIdx]);
    str = GetGameOptionRowTitle(GetUiGameOptionAt(selectedIdx));

    /* Match vanilla for the common case so BG1 titles stay clean. */
    if (y != 0x1f)
    {
        PutDrawText(
            &gConfigUiState->optionTexts[textIdx],
            TILEMAP_LOCATED(gBG1TilemapBuffer, 4, y),
            TEXT_COLOR_SYSTEM_WHITE, 0, 9, str);
        return;
    }

    Text_SetCursor(&gConfigUiState->optionTexts[textIdx], 0);
    Text_SetColor(&gConfigUiState->optionTexts[textIdx], TEXT_COLOR_SYSTEM_WHITE);
    Text_DrawString(&gConfigUiState->optionTexts[textIdx], str);
    PutTextYWrapped(&gConfigUiState->optionTexts[textIdx], gBG1TilemapBuffer, 4, y);
}

//! FE8U: 0x080B1850
LYN_REPLACE_CHECK(DrawOptionValueTexts);
void DrawOptionValueTexts(int selectedIdx, int textIdx, int y)
{
    int i;

    int optionIdx = GetUiGameOptionAt(selectedIdx);
    int x = GetGameOptionRowX(optionIdx) / 8;

    y &= 0x1f;

    ClearText(&gConfigUiState->valueTexts[textIdx]);

    for (i = 0; i < GetGameOptionRowCount(optionIdx); i++)
    {
        Text_InsertDrawString(
            &gConfigUiState->valueTexts[textIdx],
            gGameOptions_NEW[optionIdx].selectors[i].xPos - GetGameOptionRowX(optionIdx),
            (i == GetGameOption(optionIdx)) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY,
            GetGameOptionRowValueText(optionIdx, i));
    }

    PutTextYWrapped(&gConfigUiState->valueTexts[textIdx], gBG1TilemapBuffer, x, y);
}

//! FE8U = 0x080B1938
LYN_REPLACE_CHECK(DrawConfigUiSprites);
void DrawConfigUiSprites(void)
{
    int y;

    int optionIdx = GetUiGameOptionAt(gConfigUiState->selectedOptionIdx);

    u8 time = k_umod(GetGameClock(), 16) & 8;

    CallARM_PushToSecondaryOAM(18, 8, gSprite_ConfigurationUiHeader, OAM2_CHR(0xC0) + OAM2_PAL(2));

    // current option position on screen (cur index - top index)
    y = (gConfigUiState->selectedOptionIdx - gConfigUiState->headOptionIdx) * 16 + 40;

    DisplayFrozenUiHand(16, y);

    DisplayUiHand(gGameOptions_NEW[optionIdx].selectors[GetGameOption(optionIdx)].xPos - 2, y);

    if (!(gConfigUiState->source & CONFIG_UI_SOURCE_FROMPREP) || (PrepGetDeployedUnitAmt() != 0))
    {
        if ((GetUiGameOptionAt(gConfigUiState->selectedOptionIdx) == GAME_OPTION_ANIMATION) && (GetSelectedOptionValue() == 3))
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
    gConfigUiState->maxOption = GetGameOptionIndexCount();
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

    // Decompress(Img_ConfigUiSprites, OBJ_CHR_ADDR(0xC0));
    // Decompress(Img_ConfigUiIcons, BG_CHR_ADDR(0x200));
    Decompress(Img_ConfigUiSprites, OBJ_CHR_ADDR(0xC0));
    Decompress(Img_ConfigUiIcons_NEW, BG_CHR_ADDR(0x200));

    Decompress(Tsa_ConfigUiFrame, gGenericBuffer + 0x80);
    CallARM_FillTileRect(gBG2TilemapBuffer, gGenericBuffer + 0x80, TILEREF(0x0, 1));

    ResetTextFont();

    /* Custom helps can exceed vanilla's 22 tiles (176px). Overflow walks into
     * the next InitText slot (optionTexts[0] here) and garbles that row's
     * title — e.g. Show Tutorials help stomps Supports Gains (textIdx 0). */
    InitText(&gConfigUiState->optionHelpText, 28);

    DrawGameOptionHelpText();

    StartMenuScrollBarExt(proc, 224, 47, 0x390 * CHR_SIZE, 1);

    for (i = 0; i < 7; i++)
    {
        InitText(&gConfigUiState->optionTexts[i], 9);
        InitText(&gConfigUiState->valueTexts[i], 14);
    }

    for (i = 0; i < 6; i++)
    {
        int y = (i * 2) + 5;

        DrawGameOptionIcon(i, 5);
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

    if (GetGameOption(GetUiGameOptionAt(gConfigUiState->selectedOptionIdx)) != 0)
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
    u8 optionIdx = GetUiGameOptionAt(selectedIdx);

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
            if (selectedValue + 1 < GetGameOptionRowCount(optionIdx)) {
                selectedValue++;
                SetGameOption(optionIdx, selectedValue);

                valueChanged = true;
            }
        }

        if (valueChanged)
        {
            Proc_Start(gProcScr_RedrawConfigHelpText, proc);
            DrawOptionValueTexts(selectedIdx, k_umod(selectedIdx, 7), (selectedIdx * 2 + 5) & 0x1f);
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

            if (GetUiGameOptionAt(gConfigUiState->selectedOptionIdx) != 0)
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
            if (gGameOptions_NEW[GetUiGameOptionAt(gConfigUiState->selectedOptionIdx)].func != NULL)
            {
                gGameOptions_NEW[GetUiGameOptionAt(gConfigUiState->selectedOptionIdx)].func(proc);
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
    int yNextTmp = 0x20 * ((y + 1) & 0x1f);

    for (i = 0; i <= 26; i++)
    {
        gBG1TilemapBuffer[yTmp + 0x02 + i] = 0;
        gBG1TilemapBuffer[yNextTmp + 0x02 + i] = 0;
    }

    textIdx = k_umod(selectedIdx, 7);

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