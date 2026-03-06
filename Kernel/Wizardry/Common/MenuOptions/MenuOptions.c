#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "jester_headers/custom-structs.h"

// Combined elements into a single comma-separated block
const u8 gGameOptionsUiOrder_NEW[30] = {
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
    GAME_OPTION_ANIMA_WEAPON_TRIANGLE,
    GAME_OPTION_SUPPORT_AFTER_BATTLE,
    GAME_OPTION_GAMEOVER_QUOTES,
};

// Collapsed designated initializers to a single line per option
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
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
        .icon = 0x22,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_ANIMA_WEAPON_TRIANGLE] =
    {
        .msgId = MSG_MENU_OPTION_ANIMA_WEAPON_TRIANGLE_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_ANIMA_WEAPON_TRIANGLE_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_ANIMA_WEAPON_TRIANGLE_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x22,
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
        .icon = 0x22,
        .func = GenericOptionChangeHandler,
    },

    [GAME_OPTION_GAMEOVER_QUOTES] =
    {
        .msgId = MSG_MENU_OPTION_GAMEOVER_QUOTES_TITLE,
        .selectors =
        {
            { MSG_MENU_OPTION_GAMEOVER_QUOTES_DESC, MSG_MENU_OPTION_ON,  112, 2 },
            { MSG_MENU_OPTION_GAMEOVER_QUOTES_DESC, MSG_MENU_OPTION_OFF, 135, 2 },
            { MSG_000,  MSG_000,  190, 0 },
            { MSG_000,  MSG_000,  189, 0 },
        },
        .icon = 0x22,
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
        case GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS:  gPlaySt.config.fast_battle_animatons = newValue; break;
        case GAME_OPTION_EXPANDED_MAX_HP:                gPlaySt.config.expanded_max_hp = newValue; break;
        case GAME_OPTION_FLIPPED_ENEMY_SPRITES:          gPlaySt.config.flip_enemy_sprites = newValue; break;
        case GAME_OPTION_SUMMONS_GAIN_EXP:               gPlaySt.config.summons_gain_exp = newValue; break;
        case GAME_OPTION_PROMOTE_ON_MAX_LEVEL:           gPlaySt.config.promote_unit_on_max_level = newValue; break;
        case GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE: gPlaySt.config.show_arena_opponent_in_advance = newValue; break;
        case GAME_OPTION_SEND_INVENTORY_ON_DEATH:        gPlaySt.config.send_inventory_on_death = newValue; break;
        case GAME_OPTION_FAST_MAP_ANIMATIONS:            gPlaySt.config.fast_map_animations= newValue; break;
        case GAME_OPTION_ANIMA_WEAPON_TRIANGLE:          gPlaySt.config.anima_weapon_triangle = newValue; break;
        case GAME_OPTION_SUPPORT_AFTER_BATTLE:           gPlaySt.config.support_after_battle = newValue; break;
        case GAME_OPTION_GAMEOVER_QUOTES:                gPlaySt.config.gameover_quotes = newValue; break;
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
        case GAME_OPTION_FAST_FOWARD_BATTLE_ANIMATIONS:  return gPlaySt.config.fast_battle_animatons;
        case GAME_OPTION_EXPANDED_MAX_HP:                return gPlaySt.config.expanded_max_hp;
        case GAME_OPTION_FLIPPED_ENEMY_SPRITES:          return gPlaySt.config.flip_enemy_sprites;
        case GAME_OPTION_SUMMONS_GAIN_EXP:               return gPlaySt.config.summons_gain_exp;
        case GAME_OPTION_PROMOTE_ON_MAX_LEVEL:           return gPlaySt.config.promote_unit_on_max_level;
        case GAME_OPTION_SHOW_ARENA_OPPONENT_IN_ADVANCE: return gPlaySt.config.show_arena_opponent_in_advance;
        case GAME_OPTION_SEND_INVENTORY_ON_DEATH:        return gPlaySt.config.send_inventory_on_death;
        case GAME_OPTION_FAST_MAP_ANIMATIONS:            return gPlaySt.config.fast_map_animations;
        case GAME_OPTION_ANIMA_WEAPON_TRIANGLE:          return gPlaySt.config.anima_weapon_triangle;
        case GAME_OPTION_SUPPORT_AFTER_BATTLE:           return gPlaySt.config.support_after_battle;
        case GAME_OPTION_GAMEOVER_QUOTES:                return gPlaySt.config.gameover_quotes;
    }
    return 0;
}