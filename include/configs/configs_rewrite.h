// #include "config-memmap.h"

// ///////////////////////////////////////////////
// // 1. Core Engine / Build Configuration
// ///////////////////////////////////////////////

// #define CONFIG_CROP_VANILLA_MSG
// #define CONFIG_COMMON_PROTECTION_ENABLED
// #define CONFIG_BANIM_SWITCHER_EN
// #define CONFIG_VERIFY_SKILLSYS_SRAM
// #define CONFIG_USE_UTF8_GLYPH
// #define CONFIG_KTUT_LEVEL 0
// #define CONFIG_UNLOCK_ALLY_MHP_LIMIT

// #define CONFIG_LVUP_MODE_TUTORIAL 0
// #define CONFIG_LVUP_MODE_NORMAL 0
// #define CONFIG_LVUP_MODE_HARD 0

// #define CONFIG_BATTLE_MAX_DAMAGE 127
// #define CONFIG_INSTALL_NON_KERNEL_PATCH

// #define CONFIG_UNIT_AMT_ENEMY 50
// #define CONFIG_UNIT_AMT_NPC   8

// ///////////////////////////////////////////////
// // 2. Debugging / Developer Tools
// ///////////////////////////////////////////////

// #define CONFIG_VESLY_DEBUGGER

// ///////////////////////////////////////////////
// // 3. Battle Engine & Combat Behavior
// ///////////////////////////////////////////////

// #define CONFIG_USE_COMBO_ATTACK
// #define CONFIG_USE_CHAR_CUSTOM_ANIM
// // #define CONFIG_BATTLE_SURROUND
// // #define CONFIG_FAKE_OLD_ANIMNUMBER_PATCH
// #define CONFIG_USE_GAIDEN_MAGIC
// #ifdef CONFIG_USE_GAIDEN_MAGIC
// 	#define CONFIG_GAIDEN_MAGIC_MUST_BE_MAGIC 0
// 	#define CONFIG_GAIDEN_MAGIC_REQUIRES_WRANK 0
// 	#define CONFIG_GAIDEN_MAGIC_AI_EN 1
// 	#define CONFIG_GAIDEN_EXT_CONF_EN 1
// #endif
// #define CONFIG_STOP_COUNTER_ENABLED
// #define CONFIG_EFX_STATUS_DISPLAY_ON_EXTHIT
// #define CONFIG_C03_NOT_FLUSH_EFXSTATUS
// // #define CONFIG_LEADER_SHIP_EN
// // #define CONFIG_REMOVE_MOVE_PATH
// #define CONFIG_INSTALL_KERNEL_SHIELD
// // #define CONFIG_INSTALL_BOW2DECREASE
// #define CONFIG_UNIT_ACTION_AMT 0x30
// #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 0x23
// #define CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat 0x24
// #define CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff 0x25
// #define CONFIG_VESLY_SUPPORT_POST_BATTLE
// #ifdef  CONFIG_VESLY_SUPPORT_POST_BATTLE
// 	#define SUPPORT_RATE_KILL 100
// 	#define SUPPORT_RATE_COMBAT 100
// 	#define SUPPORT_RATE_STAFF 100
// 	#define SUPPORT_RATE_DANCE 100
// #endif
// #define CONFIG_VESLY_ANIMS_FAST_FORWARD
// #define CONFIG_VESLY_RECLASS
// #define CONFIG_INSTANT_LEVEL_UP
// #define CONFIG_MIRROR_MAP_SPRITES
// #define CONFIG_AUTO_REPAIR_WEAPONS
// #define CONFIG_S_RANK_NO_WEAPON_WEIGHT
// // #define CONFIG_RESTORE_HP_ON_LEVEL_UP
// #define CONFIG_RESET_BWL_STATS_EACH_CHAPTER
// #define CONFIG_MULTIPLE_DEATH_QUOTES
// #define CONFIG_GAMEOVER_QUOTES
// #ifdef CONFIG_GAMEOVER_QUOTES
// 	//#define CONFIG_GAMEOVER_GENERIC
// 	#define CONFIG_GAMEOVER_SPECIFIC
// 	//#define CONFIG_GAMEOVER_COMEDIC
// #endif
// #define CONFIG_DEATH_DANCE
// // #define CONFIG_2RN_GROWTHS
// // #define CONFIG_ANIMA_WEAPON_TRIANGLE

// ///////////////////////////////////////////////
// // 4. UI / Visual Enhancements
// ///////////////////////////////////////////////

// #define CONFIG_PAGE1_WITH_BWL 0
// #define CONFIG_PAGE1_WITH_LEADERSHIP 1
// #define CONFIG_PAGE4_MOKHA_PLAN_A 0
// #define CONFIG_PAGE4_MOKHA_PLAN_B 1
// // #define CONFIG_INSTALL_MAPACTIONANIM
// // #define CONFIG_INSTALL_STATSCREENFX
// #define CONFIG_VESLY_DRAW_ANIMATIONS
// #define CONFIG_VESLY_UI
// #define CONFIG_STAT_SCREEN_ALLEGIANCE_COLORS
// #define CONFIG_MODULAR_MINIMUG_BOX
// #define CONFIG_TEXT_ENGINE_REWORK
// #define CONFIG_GREEN_BONUS_GROWTHS

// #ifndef CONFIG_GREEN_BONUS_GROWTHS
// // #define CONFIG_GROWTHS_AS_LETTERS
// #endif

// /* Not installed because of conflicts with phase suspend and sound mixer */
// // #define CONFIG_CUSTOM_CHAPTER_SCREEN

// #define CONFIG_STAT_PAGE_PERSONAL_INFO
// #define CONFIG_STAT_PAGE_PROMOTIONS
// // #define CONFIG_FE7_MODE_SELECT
// // #define CONFIG_HALF_BODY_PORTRAITS

// ///////////////////////////////////////////////
// // 5. Quality‑of‑Life Features
// ///////////////////////////////////////////////

// #define CONFIG_AUTO_NARROW_FONT
// #define CONFIG_PERFORMANCE_OPTIMIZATION
// #define CONFIG_INSTALL_CONVOYEXPA
// #define CONFIG_INSTALL_CONVOYEXPA_AMT 200
// #define CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
// #define CONFIG_RES_TERRAIN_WINDOW
// #define CONFIG_SHOW_HEAL_AMOUNT
// #define CONFIG_SHOW_CGs_LIKE_FE7
// //#define CONFIG_SEND_INVENTORY_ON_DEATH
// #define CONFIG_NO_WAIT_AFTER_TRADING
// #define CONFIG_CHAPTER_NAMES
// #define CONFIG_PREPS_AUGURY
// #define CONFIG_PROMOTION_ON_MAX_LEVEL
// // #define CONFIG_SKIP_INTRO
// // #define CONFIG_SKIP_CHAPTER_INTROS
// #define CONFIG_QUALITY_OF_LIFE_SHORTEN_AVOID_TEXT
// #define CONFIG_QUALITY_OF_LIFE_UNIT_NAME_DROP
// #define CONFIG_QUALITY_OF_LIFE_EPILOGUE_FADE
// #define CONFIG_QUALITY_OF_LIFE_CAP_CRIT_100
// #define CONFIG_QUALITY_OF_LIFE_AI_TRADE_FIX
// #define CONFIG_QUALITY_OF_LIFE_WEAPON_EXP_HELPBOX
// #define CONFIG_QUALITY_OF_LIFE_WEAPON_STAT_BONUSES
// #define CONFIG_STAT_SCREEN_TERRAIN_BONUS
// #define CONFIG_CUSTOM_STAFF_ACCURACY 100

// #define CONFIG_PROMOTE_ENEMIES_IF_KILLED_UNIT
// #ifdef CONFIG_PROMOTE_ENEMIES_IF_KILLED_UNIT
// 	#define CONFIG_ENEMY_PROMOTION_BOOST 3
// #endif

// #define CONFIG_MODULAR_STAFF_EXP
// #define CONFIG_EXPANDED_PREP_MENU_OPTIONS
// // #define CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS
// #define CONFIG_ARENA_SHOW_OPPONENT_IN_ADVANCE
// #define CONFIG_ARENA_LET_PLAYER_USE_UPGRADED_WEAPONS
// #define CONFIG_ARENA_CALCULATE_WEAPON_BASED_ON_LEVEL

// ///////////////////////////////////////////////
// // 6. AI & Action Logic
// ///////////////////////////////////////////////

// #define CONFIG_AUTO_DETECT_EFXRESIRE_WEAPON
// #define CONFIG_NO_SUS_IN_AI_PHASE
// #define CONFIG_AI_ACTION_AMT 20
// #define CONFIG_AI_ACTION_EXPA_Teleportation 14

// ///////////////////////////////////////////////
// // 7. Items, Icons & Inventory
// ///////////////////////////////////////////////

// #define CONFIG_IER_EN
// #define CONFIG_FEB_SKILL_SCROLL_OVERFLOW_HOTFIX 
// #define CONFIG_MULTIPLE_BOOST_STAVES

// #define CONFIG_PR_ITEM_ICON  0x5926F4
// #define CONFIG_ICON_INDEX_MAG_BOOSTER 0xCA
// #define CONFIG_ICON_INDEX_STAR 0xCB
// #define CONFIG_ICON_INDEX_SKILL_STEALER 0xCC
// #define CONFIG_ICON_INDEX_ARMS_SCROLL 0xCD

// #ifdef CONFIG_MULTIPLE_BOOST_STAVES
// 	#define CONFIG_ICON_INDEX_FORCE_STAFF 0xCE
// 	#define CONFIG_ICON_INDEX_ACUITY_STAFF 0xCF
// 	#define CONFIG_ICON_INDEX_FORTUNE_STAFF 0xDE
// 	#define CONFIG_ICON_INDEX_IRON_STAFF 0xDF
// 	#define CONFIG_ICON_INDEX_SPRINT_STAFF 0xAD
// 	#define CONFIG_ICON_INDEX_TEMPEST_STAFF 0xAE
// 	#define CONFIG_ICON_INDEX_OMNI_STAFF 0x9F
// #endif

// #define CONFIG_ICON_INDEX_RUNE_STAFF 0xAF
// #define CONFIG_ICON_INDEX_MINE_STAFF 0x6F
// #define CONFIG_ICON_INDEX_SLOW_STAFF 0x78
// #define CONFIG_ICON_INDEX_FORGE_STAFF 0xAB
// #define CONFIG_ICON_INDEX_REWARP_STAFF 0xAC
// #define CONFIG_ICON_INDEX_POISON_STAFF 0xA0
// #define CONFIG_ICON_INDEX_DELAY_STAFF 0xA1
// #define CONFIG_ICON_INDEX_ENTRAP_STAFF 0xA2
// #define CONFIG_ICON_INDEX_QUICKEN_STAFF 0xA3
// #define CONFIG_ICON_INDEX_HIDE_STAFF 0xA4
// #define CONFIG_ICON_INDEX_PROVOKE_STAFF 0xA5
// #define CONFIG_ICON_INDEX_PETRIFY_STAFF 0xA6
// #define CONFIG_ICON_INDEX_SOOTH_STAFF 0xA7
// #define CONFIG_ICON_INDEX_ENFEEBLE_STAFF 0xA7
// #define CONFIG_ICON_INDEX_INVEST_STAFF 0xA8

// #define CONFIG_PR_ITEM_TABLE 0x809B10
// #define CONFIG_ITEM_INDEX_MAG_BOOSTER 0xBC
// // #define CONFIG_ITEM_INDEX_SKILL_SCROLL 0xBD
// #define CONFIG_ITEM_INDEX_SKILL_SCROLL_FEB 0xFF

// #define CONFIG_ITEM_INDEX_SKILL_STEALER 0xBE
// #define CONFIG_ITEM_INDEX_ARMS_SCROLL 0xCC

// #ifdef CONFIG_MULTIPLE_BOOST_STAVES
// 	#define CONFIG_ITEM_INDEX_FORCE_STAFF 0xC0
// 	#define CONFIG_ITEM_INDEX_ACUITY_STAFF 0xC1
// 	#define CONFIG_ITEM_INDEX_FORTUNE_STAFF 0xC2
// 	#define CONFIG_ITEM_INDEX_IRON_STAFF 0xC3
// 	#define CONFIG_ITEM_INDEX_SPRINT_STAFF 0xC4
// 	#define CONFIG_ITEM_INDEX_TEMPEST_STAFF 0xC5
// 	#define CONFIG_ITEM_INDEX_OMNI_STAFF 0xCE
// #endif

// #define CONFIG_ITEM_INDEX_RUNE_STAFF 0xC6
// #define CONFIG_ITEM_INDEX_MINE_STAFF 0xC7
// #define CONFIG_ITEM_INDEX_SLOW_STAFF 0xC8
// #define CONFIG_ITEM_INDEX_FORGE_STAFF 0xCB
// #define CONFIG_ITEM_INDEX_REWARP_STAFF 0xCD
// #define CONFIG_ITEM_INDEX_POISON_STAFF 0xD0
// #define CONFIG_ITEM_INDEX_DELAY_STAFF 0xD1
// #define CONFIG_ITEM_INDEX_ENTRAP_STAFF 0xD2
// #define CONFIG_ITEM_INDEX_QUICKEN_STAFF 0xD3
// #define CONFIG_ITEM_INDEX_HIDE_STAFF 0xD4
// #define CONFIG_ITEM_INDEX_PROVOKE_STAFF 0xD5
// #define CONFIG_ITEM_INDEX_PETRIFY_STAFF 0xD6
// #define CONFIG_ITEM_INDEX_SOOTH_STAFF 0xD7
// #define CONFIG_ITEM_INDEX_ENFEEBLE_STAFF 0xD8
// #define CONFIG_ITEM_INDEX_INVEST_STAFF 0xD9

// #ifdef CONFIG_ITEM_INDEX_FORGE_STAFF
// 	#define CONFIG_FORGE_CHECKER 5000
// #endif

// ///////////////////////////////////////////////
// // 8. Skills & Skill System
// ///////////////////////////////////////////////

// #define CONFIG_FIT_OLD_SKILLSYS_LIST
// #define CONFIG_MENU_SKILL_NOT_IN_UPPER
// #define CONFIG_MENU_SKILL_DISP_MSG_EN_N 1

// #define CONFIG_TURN_ON_ALL_SKILLS

// #ifdef CONFIG_TURN_ON_ALL_SKILLS
// 	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0x0A
// 	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_2 0xBD
// 	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_3 0xC9
// 	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_4 0xCA
// #else
// 	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0xBD
// #endif

// #define CONFIG_TELLIUS_CAPACITY_SYSTEM
// #define CONFIG_TELLIUS_CAPACITY_BASE 50
// #define CONFIG_TELLIUS_CAPACITY_PROMOTED 25

// ///////////////////////////////////////////////
// // 9. Map, Movement & Terrain
// ///////////////////////////////////////////////

// #define CONFIG_FASTER_MAP_RANGE
// #define CONFIG_DISPLAY_TALK_ICON
// #define CONFIG_DISPLAY_DROPPABLE_ITEM_ICON
// #define CONFIG_BREAKABLE_DOORS
// // #define CONFIG_SUPER_FAST_MAP_ANIMATIONS
// #define CONFIG_L_BUTTON_SAME_FACTION_CYCLING

// ///////////////////////////////////////////////
// // 10. Support / Relationships
// ///////////////////////////////////////////////

// #define CONFIG_UNLOCK_ALL_SUPPORTS
// // #define CONFIG_SUPPORT_REWARDS
// #define CONFIG_CUSTOM_SUPPORT_CONVOS
// #define CONFIG_UNLOCK_SUPPORT_CONVO_LIMIT 10

// ///////////////////////////////////////////////
// // 11. Death & Game‑Over Systems
// ///////////////////////////////////////////////

// // #define CONFIG_CASUAL_MODE

// ///////////////////////////////////////////////
// // 12. Experimental Features
// ///////////////////////////////////////////////

// #define CONFIG_USE_KONAMI_CODE_BONUS
// #define CONFIG_FOURTH_ALLEGIANCE  // Run a full make clean every time you toggle this
// #ifdef CONFIG_FOURTH_ALLEGIANCE
//  	#define CONFIG_UNIT_AMT_ALLY  41
// 	#define CONFIG_UNIT_AMT_FOURTH 10
// #else
// 	#define CONFIG_UNIT_AMT_ALLY  51
// 	#define CONFIG_UNIT_AMT_FOURTH 0
// #endif
// #define CONFIG_VESLY_CREDITS_SEQUENCE
// // #define CONFIG_VESLY_DANGER_BONES
// // #define CONFIG_VESLY_NOTIFICATION_SYSTEM
// // #define CONFIG_VESLY_AOE
// #define CONFIG_LIMITED_SHOP_STOCK
// // #define CONFIG_DENY_STAT_SCREEN
// #define CONFIG_ARENA_LIMITS
// #define CONFIG_QUINTESSANCE_EFFECT
// #define SETH_INJURED
// #define INJURED_TURN_COUNT 5
// // #define CONFIG_TALK_LEVEL_UP
// #define CONFIG_CUSTOM_GUIDE
// #define CONFIG_REFUGE_FEATURE
// // #define CONFIG_LAGUZ_BARS
// // #define CONFIG_FORGING
// // #ifdef CONFIG_FORGING
// // 	#define CONFIG_FE4_CRIT_BONUS_ON_KILL
// // #endif
// #define CONFIG_MP_SYSTEM
// #define CONFIG_MODULAR_FOG_UNIT_SIGHT
// // #define CONFIG_MULTIPLE_FOG_STAGES
// #define CONFIG_MISC_UNIT_COUNTERS
// // #define CONFIG_UNIT_SELECTION_QUOTES 
// #define CONFIG_VOICE_ACTED_DIALOGUE
// #define CONFIG_FREE_MOVEMENT
// // #define CONFIG_BIORHYTHM
// // #define CONFIG_MOKHA_AOE 
// // #define CONFIG_LIGHTS_OUT_GAME
// #define CONFIG_BASE_CHAPTERS
// // #define CONFIG_SKILL_POINTS_ENGAGE
// #define CONFIG_KILL_REWARDS

// #define CONFIG_FE8_REWRITE

// ///////////////////////////////////////////////
// // 13. Hotfixes
// ///////////////////////////////////////////////

// #define CONFIG_HOTFIXES