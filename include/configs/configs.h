#ifndef CONFIG_INSTALLED
#define CONFIG_INSTALLED

//#include "config-debug.h"
#include "config-memmap.h"

#define CONFIG_FE8_REWRITE

/**
 * If uncomment, kernel may remove vanilla dislogue text
 * so that we can save ~470KByte ROM space in DEMO.
 *
 * Note that enabling this config will make FEB think there are something wrong in
 * ROM and report warnning. But it will be okay since this will not actually
 * introduce any bug.
 *
 * If you're planning to develop a new game instead of making minor
 * changes on vanilla, it is recommanded to uncomment this config to save space.
 */
#define CONFIG_CROP_VANILLA_MSG

 /* CommonProtection.event */
#define CONFIG_COMMON_PROTECTION_ENABLED

/* If comment, CHAX may not verify on FESKILL magic work in SRAM */
#define CONFIG_VERIFY_SKILLSYS_SRAM

/* If uncomment, use utf-8 characters */
#define CONFIG_USE_UTF8_GLYPH

/* Engage style combo attack */
#define CONFIG_USE_COMBO_ATTACK

/**
 * Surround:
 * When a unit is attacked and adjacent to the enemy,
 * each side with enemy may cause unit avoid -10%
 *
 * If completely surrounded (each side stands an enemy),
 * unit may cause def-5 additionally.
 *
 * Flyer in outdoor environments are not affected by this effect
 */
// #define CONFIG_BATTLE_SURROUND

/**
 * 0: Not show any kernel tutorial
 * 1: Only show kernel tutorial in tutorial mode
 * 2: show kernel tutorial at anytime
 */
#define CONFIG_KTUT_LEVEL 0

/**
 * If uncommented, enable patch
 * FEB: Character Custom Animation
 */
#define CONFIG_USE_CHAR_CUSTOM_ANIM

/**
* If uncommented, fasten function GenerateUnitCompleteAttackRange
* with lower accuracy.
*/
#define CONFIG_FASTER_MAP_RANGE

/**
 * Try narrow font on skill-name/skill-desc/menu-disp
 */
#define CONFIG_AUTO_NARROW_FONT

/**
 * Inject fake FEB old AnimNumber patch hooks.
 * This config may cause two functions suffer from ugly ASM hooks:
 *      - StartBattleAnimHitEffect
 *      - NewEfxHpBar
 */
// #define CONFIG_FAKE_OLD_ANIMNUMBER_PATCH

/**
 * Auto detect on EfxResire weapon judgement
 */
#define CONFIG_AUTO_DETECT_EFXRESIRE_WEAPON

/**
* Enable patch: BanimSwitch
*/
#define CONFIG_BANIM_SWITCHER_EN

/**
 * If uncommented, enable Gaiden style magic system
 *
 * Sub:
 * MUST_BE_MAGIC: if set as 0, then all weapons will be okay (rather than magic wepaon only)
 * REQUIRES_WRANK: if set as 0, then unit can directly use the weapon regardless vanilla judgement
 * AI_EN: if set as 1, AI can also use gaiden B.Mag
 * EXT_CONF_EN: enbale gGaidenChaxConfigs
 */
#define CONFIG_USE_GAIDEN_MAGIC

#ifdef CONFIG_USE_GAIDEN_MAGIC
#define CONFIG_GAIDEN_MAGIC_MUST_BE_MAGIC 0 // 0: all weapons are okay
#define CONFIG_GAIDEN_MAGIC_REQUIRES_WRANK 0 // 0: not requires unit ability to equip that weapon
#define CONFIG_GAIDEN_MAGIC_AI_EN 1
#define CONFIG_GAIDEN_EXT_CONF_EN 1
#endif /* USE_GAIDEN_MAGIC */

/**
 * 1. Petrify should not stop battle
 * 2. Petrified/sleeped unit should not attack
 */
#define CONFIG_STOP_COUNTER_ENABLED

/**
 * In banim, unit status flashing is desicded by ext-hit rather than ANIM_ROUND_POISON
 */
#define CONFIG_EFX_STATUS_DISPLAY_ON_EXTHIT

/**
* C03 not flush efx-status
*/
#define CONFIG_C03_NOT_FLUSH_EFXSTATUS

/**
 * Enable leader-ship features
 */
// #define CONFIG_LEADER_SHIP_EN

/**
* If uncommented, this will disable move-path calculation and display.
* Since move-path can display no more than 20 steps in vanilla,
* with the increase of unit's mobility, this limitation may cause
* risk of overflow error. Thus we directly remove the move-path to avoid overflow.
*/
// #define CONFIG_REMOVE_MOVE_PATH

/**
* Install FEB patch: SetHPClassLimit
*/
#define CONFIG_UNLOCK_ALLY_MHP_LIMIT

/**
* Append the same logic on loading skill as old skillsys, as:
* https://feuniverse.us/t/the-skill-system-and-you-maximizing-your-usage-of-fe8s-most-prolific-bundle-of-wizardry/8232/5
*/
#define CONFIG_FIT_OLD_SKILLSYS_LIST

/**
 * Enable shield system
 */
#define CONFIG_INSTALL_KERNEL_SHIELD

/**
 * Unit menu skill item displayed in upper menu or standalone sub-menu.
 */
#define CONFIG_MENU_SKILL_NOT_IN_UPPER

/**
 * Levelup mode
 * 0: vanilla
 * 1: uncontrollable
 * 2: fixed
 * 3: 100%
 * 4: 0%
 */
#define CONFIG_LVUP_MODE_TUTORIAL 0
#define CONFIG_LVUP_MODE_NORMAL 0
#define CONFIG_LVUP_MODE_HARD 0

/**
 * Page4 styles
 */
#define CONFIG_PAGE4_MOKHA_PLAN_A 0
#define CONFIG_PAGE4_MOKHA_PLAN_B 1

/**
 * Install some performance hooks
 */
#define CONFIG_PERFORMANCE_OPTIMIZATION

/**
 * Install DrawMapActionAnimation
 */
// #define CONFIG_INSTALL_MAPACTIONANIM

/**
* Install StatScreenfx
*/
// #define CONFIG_INSTALL_STATSCREENFX

/**
* Battle max damage
*/
#define CONFIG_BATTLE_MAX_DAMAGE 127

/**
 * Install Bow2Decrease
 */
// #define CONFIG_INSTALL_BOW2DECREASE

/**
* Install none-kernel related patches (not necessary)
*/
#define CONFIG_INSTALL_NON_KERNEL_PATCH

/**
* When display menu skills, directly display skill name, rather than MenuItemDef::nameMsgId
*/
#define CONFIG_MENU_SKILL_DISP_MSG_EN_N 1

/**
 * Install ConvoyExpa
 */
#define CONFIG_INSTALL_CONVOYEXPA
#define CONFIG_INSTALL_CONVOYEXPA_AMT 200 // better not to touch this since it will affact on size of gExpaConvoyItemArray

/**
 * If uncommented, suspend data may only saved in play-phase starting.
 * This may also overwrite PlaySt::disableTerrainDisplay config for player configuration in runtime.
 */
#define CONFIG_NO_SUS_IN_AI_PHASE

/**
 * If you press ↑↑↓↓←→←→BA at titlescreen, then...
 */
#define CONFIG_USE_KONAMI_CODE_BONUS

/**
* IER
*/
#define CONFIG_IER_EN
#define CONFIG_FEB_SKILL_SCROLL_OVERFLOW_HOTFIX // If item index is overflowed, modify to skill scroll index

/**
 * Ai action expansion
 */
#define CONFIG_AI_ACTION_AMT 20
#define CONFIG_AI_ACTION_EXPA_Teleportation 14

/**
 * Unit action expansion
 */
#define CONFIG_UNIT_ACTION_AMT 0x30
#define CONFIG_UNIT_ACTION_EXPA_ExecSkill 0x23
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat 0x24
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff 0x25

#define CONFIG_MULTIPLE_BOOST_STAVES

/**
 * Icon config
 */
#define CONFIG_PR_ITEM_ICON  0x5926F4
#define CONFIG_ICON_INDEX_MAG_BOOSTER 0xCA
#define CONFIG_ICON_INDEX_STAR 0xCB
#define CONFIG_ICON_INDEX_SKILL_STEALER 0xCC
#define CONFIG_ICON_INDEX_ARMS_SCROLL 0xCD

#ifdef CONFIG_MULTIPLE_BOOST_STAVES
	#define CONFIG_ICON_INDEX_FORCE_STAFF 0xCE
	#define CONFIG_ICON_INDEX_ACUITY_STAFF 0xCF
	#define CONFIG_ICON_INDEX_FORTUNE_STAFF 0xDE
	#define CONFIG_ICON_INDEX_IRON_STAFF 0xDF
	#define CONFIG_ICON_INDEX_SPRINT_STAFF 0xAD
	#define CONFIG_ICON_INDEX_TEMPEST_STAFF 0xAE
	#define CONFIG_ICON_INDEX_OMNI_STAFF 0x9F // Originally the Sol Katti
#endif

#define CONFIG_ICON_INDEX_RUNE_STAFF 0xAF
#define CONFIG_ICON_INDEX_MINE_STAFF 0x6F // Literally no clue what this was originally, a necklace?
#define CONFIG_ICON_INDEX_SLOW_STAFF 0x78 // Originally Echesacks
#define CONFIG_ICON_INDEX_FORGE_STAFF 0xAB // Originally a musical note, for play?
#define CONFIG_ICON_INDEX_REWARP_STAFF 0xAC // Originally a musical note, for play?
#define CONFIG_ICON_INDEX_POISON_STAFF 0xA0 // Originally Armedes or the other FE7 axe?
#define CONFIG_ICON_INDEX_DELAY_STAFF 0xA1
#define CONFIG_ICON_INDEX_ENTRAP_STAFF 0xA2
#define CONFIG_ICON_INDEX_QUICKEN_STAFF 0xA3
#define CONFIG_ICON_INDEX_HIDE_STAFF 0xA4
#define CONFIG_ICON_INDEX_PROVOKE_STAFF 0xA5
#define CONFIG_ICON_INDEX_PETRIFY_STAFF 0xA6
#define CONFIG_ICON_INDEX_SOOTH_STAFF 0x97 // Durandel from FE7?
#define CONFIG_ICON_INDEX_ENFEEBLE_STAFF 0xA7
#define CONFIG_ICON_INDEX_INVEST_STAFF 0xA8

/**
* Item config
*/
#define CONFIG_PR_ITEM_TABLE 0x809B10
#define CONFIG_ITEM_INDEX_MAG_BOOSTER 0xBC
// #define CONFIG_ITEM_INDEX_SKILL_SCROLL 0xBD
#define CONFIG_ITEM_INDEX_SKILL_SCROLL_FEB 0xFF

#define CONFIG_ITEM_INDEX_SKILL_STEALER 0xBE
#define CONFIG_ITEM_INDEX_ARMS_SCROLL 0xDA

#ifdef CONFIG_MULTIPLE_BOOST_STAVES
	#define CONFIG_ITEM_INDEX_FORCE_STAFF 0xC0
	#define CONFIG_ITEM_INDEX_ACUITY_STAFF 0xC1
	#define CONFIG_ITEM_INDEX_FORTUNE_STAFF 0xC2
	#define CONFIG_ITEM_INDEX_IRON_STAFF 0xC3
	#define CONFIG_ITEM_INDEX_SPRINT_STAFF 0xC4
	#define CONFIG_ITEM_INDEX_TEMPEST_STAFF 0xC5
	#define CONFIG_ITEM_INDEX_OMNI_STAFF 0xCE
#endif

#define CONFIG_ITEM_INDEX_RUNE_STAFF 0xC6
#define CONFIG_ITEM_INDEX_MINE_STAFF 0xC7
#define CONFIG_ITEM_INDEX_SLOW_STAFF 0xC8
#define CONFIG_ITEM_INDEX_FORGE_STAFF 0xCB
#define CONFIG_ITEM_INDEX_REWARP_STAFF 0xCD
#define CONFIG_ITEM_INDEX_POISON_STAFF 0xD0
#define CONFIG_ITEM_INDEX_DELAY_STAFF 0xD1
#define CONFIG_ITEM_INDEX_ENTRAP_STAFF 0xD2
#define CONFIG_ITEM_INDEX_QUICKEN_STAFF 0xD3
#define CONFIG_ITEM_INDEX_HIDE_STAFF 0xD4
#define CONFIG_ITEM_INDEX_PROVOKE_STAFF 0xD5
#define CONFIG_ITEM_INDEX_PETRIFY_STAFF 0xD6
#define CONFIG_ITEM_INDEX_SOOTH_STAFF 0xD7
#define CONFIG_ITEM_INDEX_ENFEEBLE_STAFF 0xD8
#define CONFIG_ITEM_INDEX_INVEST_STAFF 0xD9

#ifdef CONFIG_ITEM_INDEX_FORGE_STAFF
	#define CONFIG_FORGE_CHECKER 5000
#endif


/**
 * Unit amount, since it is hard to modify, it is recommanded not change this value
 */

// For now don't turn it off, it'll break the game
#define CONFIG_FOURTH_ALLEGIANCE // Run a full make clean every time you toggle this

#ifdef CONFIG_FOURTH_ALLEGIANCE
 	#define CONFIG_UNIT_AMT_ALLY  41
	#define CONFIG_UNIT_AMT_FOURTH 10
#else
	#define CONFIG_UNIT_AMT_ALLY  51
	#define CONFIG_UNIT_AMT_FOURTH 0
#endif

#define CONFIG_UNIT_AMT_ENEMY 50
#define CONFIG_UNIT_AMT_NPC   8

#define CONFIG_VESLY_DEBUGGER
#define CONFIG_VESLY_DANGER_BONES 
#define CONFIG_VESLY_SUPPORT_POST_BATTLE
#ifdef  CONFIG_VESLY_SUPPORT_POST_BATTLE
	#define SUPPORT_RATE_KILL 100
	#define SUPPORT_RATE_COMBAT 100
	#define SUPPORT_RATE_STAFF 100
	#define SUPPORT_RATE_DANCE 100
#endif

// #define CONFIG_VESLY_ANIMS_FAST_FORWARD
#define CONFIG_VESLY_DRAW_ANIMATIONS // Installation costs about 200KB
#define CONFIG_VESLY_RECLASS
// #define CONFIG_VESLY_UI //Conflicts graphically with RES_TERRAIN_WINDOW
#define CONFIG_VESLY_EXTENDED_ITEM_DESCRIPTIONS
#define CONFIG_VESLY_CREDITS_SEQUENCE
// #define CONFIG_VESLY_NOTIFICATION_SYSTEM /* Has issues with setting custom notifications and displaying UTF8 text */
// #define CONFIG_VESLY_AOE /* This needs to be updated with Vesly's latest fixes as it's affecting the item menu and minimug palettes */
#define CONFIG_VESLY_SHOOT_ARROW
#define CONFIG_STAT_SCREEN_ALLEGIANCE_COLORS
#define CONFIG_CASUAL_MODE // Doesn't seem to work right now, look into remaking in C
#define CONFIG_RES_TERRAIN_WINDOW
#define CONFIG_MODULAR_MINIMUG_BOX
#define CONFIG_INSTANT_LEVEL_UP
#define CONFIG_MIRROR_MAP_SPRITES
#define CONFIG_TEXT_ENGINE_REWORK

#define CONFIG_SHOW_CGs_LIKE_FE7
//#define CONFIG_SEND_INVENTORY_ON_DEATH
#define CONFIG_NO_WAIT_AFTER_TRADING
#define CONFIG_LIMITED_SHOP_STOCK
// #define CONFIG_DENY_STAT_SCREEN
#define CONFIG_CHAPTER_NAMES
#define CONFIG_PREPS_AUGURY
// #define CONFIG_ARENA_LIMITS
#define CONFIG_QUINTESSANCE_EFFECT
#define CONFIG_TURN_ON_ALL_SKILLS

#ifdef CONFIG_TURN_ON_ALL_SKILLS
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0x0A
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_2 0xBD
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_3 0xC9
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_4 0xCA
#else
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0xBD
#endif

#define CONFIG_PROMOTION_ON_MAX_LEVEL
#define CONFIG_GREEN_BONUS_GROWTHS // Must disable CONFIG_GROWTHS_AS_LETTERS
// #define CONFIG_GROWTHS_AS_LETTERS  // Must disable CONFIG_GREEN_BONUS_GROWTHS
// #define CONFIG_SKIP_INTRO
#define SETH_INJURED
#define INJURED_TURN_COUNT 5

#define CONFIG_QUALITY_OF_LIFE_FIXES

#ifdef CONFIG_QUALITY_OF_LIFE_FIXES
	#define CONFIG_QUALITY_OF_LIFE_SHORTEN_AVOID_TEXT
	#define CONFIG_QUALITY_OF_LIFE_UNIT_NAME_DROP
	#define CONFIG_QUALITY_OF_LIFE_EPILOGUE_FADE
	#define CONFIG_QUALITY_OF_LIFE_CAP_CRIT_100
	#define CONFIG_QUALITY_OF_LIFE_AI_TRADE_FIX
	#define CONFIG_QUALITY_OF_LIFE_WEAPON_EXP_HELPBOX
	#define CONFIG_QUALITY_OF_LIFE_WEAPON_STAT_BONUSES
	#define CONFIG_QUALITY_OF_LIFE_DISPLAY_SUPPORT_LEVEL_IN_POPUP
#endif

// #define CONFIG_AUTO_REPAIR_WEAPONS

// #define CONFIG_RESTORE_HP_ON_LEVEL_UP
#define CONFIG_RESET_BWL_STATS_EACH_CHAPTER

/* Don't turn this on until I have defeat quotes for everyone as MC dying without a quote doesn't trigger a game over screen */
#define CONFIG_MULTIPLE_DEATH_QUOTES
#define CONFIG_GAMEOVER_QUOTES
#ifdef CONFIG_GAMEOVER_QUOTES
	//#define CONFIG_GAMEOVER_GENERIC
	#define CONFIG_GAMEOVER_SPECIFIC
	//#define CONFIG_GAMEOVER_COMEDIC
#endif

#define CONFIG_CUSTOM_STAFF_ACCURACY 100

// #define CONFIG_SUPPORT_REWARDS // Go to BwlSupport.c in StartSupportTalk to adjust items gained
#define CONFIG_CUSTOM_GUIDE //Enable flag 0xB4 to view (configurable in GuideTable.event)

#define CONFIG_PROMOTE_ENEMIES_IF_KILLED_UNIT

#ifdef CONFIG_PROMOTE_ENEMIES_IF_KILLED_UNIT
	#define CONFIG_ENEMY_PROMOTION_BOOST 3
#endif

#define CONFIG_REFUGE_FEATURE
/* Not installed because of conflicts with phase suspend and sound mixer */
// #define CONFIG_CUSTOM_CHAPTER_SCREEN
#define CONFIG_EXPANDED_PREP_MENU_OPTIONS
// #define CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS
#define CONFIG_TELLIUS_CAPACITY_SYSTEM
#define CONFIG_TELLIUS_CAPACITY_BASE 50
#define CONFIG_TELLIUS_CAPACITY_PROMOTED 25
// #define CONFIG_LAGUZ_BARS // Run make clean after turning this off

// 	#define CONFIG_FE4_CRIT_BONUS_ON_KILL

#define CONFIG_CUSTOM_SUPPORT_CONVOS

#define CONFIG_MP_SYSTEM
#define CONFIG_MODULAR_FOG_UNIT_SIGHT
// #define CONFIG_MULTIPLE_FOG_STAGES /* Still a work in progress */

#define CONFIG_MISC_UNIT_COUNTERS /* Used for Skill - Bravely Default */

#define CONFIG_UNIT_SELECTION_QUOTES // Need to keep this because of the music installer event file

#define CONFIG_VOICE_ACTED_DIALOGUE // Need to keep this because of the music installer event file

#define CONFIG_FREE_MOVEMENT

// #define CONFIG_BIORHYTHM

// #define CONFIG_MOKHA_AOE // This only adds one command rather than letting you define them, turn of Vesly's AOE if using this

// #define CONFIG_LIGHTS_OUT_GAME

// #define CONFIG_2RN_GROWTHS

#define CONFIG_BASE_CHAPTERS

#define CONFIG_STAT_PAGE_PERSONAL_INFO
#define CONFIG_STAT_PAGE_PROMOTIONS

#define CONFIG_DISPLAY_TALK_ICON

#define CONFIG_DISPLAY_DROPPABLE_ITEM_ICON

// #define CONFIG_BREAKABLE_DOORS // This seems to turn all the walls to 50HP as well. Rewrite in C

// #define CONFIG_SUPER_FAST_MAP_ANIMATIONS

#define CONFIG_L_BUTTON_SAME_FACTION_CYCLING

// #define CONFIG_ANIMA_WEAPON_TRIANGLE

#define CONFIG_UNLOCK_SUPPORT_CONVO_LIMIT 10

// #define CONFIG_SKILL_POINTS_ENGAGE

// #define CONFIG_SKIP_CHAPTER_INTROS

// #define CONFIG_FE7_MODE_SELECT // Graphical errors. needs the RAM allocation in config-memmap.s at line 109 turned on

/* 
** For the portrait formatting, it only works by calling the HalfBodyFormatter.exe in my own local drive in the downloads folder,
** so will not work for anyone else unless they're using WSL (Windows Subsystem in Linux) and put the exe there.
** For now I've put a copy in EA's "Tools" folder for WSL users to move. If I get Vesly's Python portrait formatter working I may switch.
*/
// #define CONFIG_HALF_BODY_PORTRAITS

#define CONFIG_KILL_REWARDS

#define CONFIG_HOTFIXES

/* Very much not complete, still a WIP */
// #define CONFIG_PREPS_INFUSE

#define CONFIG_MAX_COLOR_BACKGROUNDS

// #define CONFIG_VARIABLE_UNIT_DESCRIPTIONS

#define CONFIG_FE7_TELEPORT

#endif /* CONFIG_INSTALLED */