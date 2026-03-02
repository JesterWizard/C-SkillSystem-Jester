//#include "config-debug.h"
#include "config-memmap.h"

// These need to remain as build time configs
#define CONFIG_FE8_REWRITE
#define CONFIG_CROP_VANILLA_MSG 			// Remove vanilla conversations and save 470KB of data, FEB will report errors but can be ignored
#define CONFIG_COMMON_PROTECTION_ENABLED 	// I'm not even sure what this does, so I ain't touching it
#define CONFIG_VERIFY_SKILLSYS_SRAM 		// This is a clever piece of protection that prevents the loading of saves created from this buildfile on incompatible copies of FE8

/**
* Append the same logic on loading skill as old skillsys, as:
* https://feuniverse.us/t/the-skill-system-and-you-maximizing-your-usage-of-fe8s-most-prolific-bundle-of-wizardry/8232/5
*/
#define CONFIG_FIT_OLD_SKILLSYS_LIST

/**
 * Install DrawMapActionAnimation
 */
// #define CONFIG_INSTALL_MAPACTIONANIM

// #define CONFIG_INSTALL_STATSCREENFX // FE7 stat screen. It causes graphical glitches with the additional pages past 4 and the extended desc textbox

#define CONFIG_INSTALL_CONVOYEXPA_AMT 200 // You'll need to adjust the EMSChunks for SaveExpaConvoy/LoadExpaConvoy in data.event to expand 

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
// #define CONFIG_VESLY_AVATAR
#define CONFIG_STAT_SCREEN_ALLEGIANCE_COLORS

#define CONFIG_RES_TERRAIN_WINDOW

#define CONFIG_TEXT_ENGINE_REWORK

#define CONFIG_SHOW_CGs_LIKE_FE7

#define CONFIG_CHAPTER_NAMES
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

#define SETH_INJURED
#define INJURED_TURN_COUNT 5

#define CONFIG_GAMEOVER_QUOTES
#ifdef CONFIG_GAMEOVER_QUOTES
	//#define CONFIG_GAMEOVER_GENERIC
	#define CONFIG_GAMEOVER_SPECIFIC
	//#define CONFIG_GAMEOVER_COMEDIC
#endif

#define CONFIG_CUSTOM_GUIDE //Enable flag 0xB4 to view (configurable in GuideTable.event)

/* Not installed because of conflicts with phase suspend and sound mixer */
// #define CONFIG_CUSTOM_CHAPTER_SCREEN

// #define CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS

// #define CONFIG_MULTIPLE_FOG_STAGES /* Still a work in progress */

#define CONFIG_MISC_UNIT_COUNTERS /* Used for Skill - Bravely Default */

#define CONFIG_UNIT_SELECTION_QUOTES // Need to keep this because of the music installer event file

#define CONFIG_VOICE_ACTED_DIALOGUE // Need to keep this because of the music installer event file

#define CONFIG_FREE_MOVEMENT

// #define CONFIG_MOKHA_AOE // This only adds one command rather than letting you define them, turn of Vesly's AOE if using this

// #define CONFIG_LIGHTS_OUT_GAME

// #define CONFIG_BREAKABLE_DOORS // This seems to turn all the walls to 50HP as well. Rewrite in C

// #define CONFIG_FE7_MODE_SELECT // Graphical errors. needs the RAM allocation in config-memmap.s at line 109 turned on

/* 
** For the portrait formatting, it only works by calling the HalfBodyFormatter.exe in my own local drive in the downloads folder,
** so will not work for anyone else unless they're using WSL (Windows Subsystem in Linux) and put the exe there or using Wine.
** For now I've put a copy in EA's "Tools" folder for WSL users to move. If I get Vesly's Python portrait formatter working I may switch.
*/
// #define CONFIG_HALF_BODY_PORTRAITS

#define CONFIG_MAX_COLOR_BACKGROUNDS