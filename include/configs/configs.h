//#include "config-debug.h"
#include "config-memmap.h"

// These need to remain as build time configs due to predominately using event/asm files
#define CONFIG_FE8_REWRITE
#define CONFIG_CROP_VANILLA_MSG 			   // Remove vanilla conversations and save 470KB of data, FEB will report errors but can be ignored
#define CONFIG_COMMON_PROTECTION_ENABLED 	   // I'm not even sure what this does, so I ain't touching it
#define CONFIG_VERIFY_SKILLSYS_SRAM 		   // This is a clever piece of protection that prevents the loading of saves created from this buildfile on incompatible copies of FE8
// #define CONFIG_INSTALL_STATSCREENFX 		   // FE7 stat screen. It causes graphical glitches with the additional pages past 4 and the extended desc textbox
#define CONFIG_INSTALL_CONVOYEXPA_AMT 200      // You'll need to adjust the EMSChunks for SaveExpaConvoy/LoadExpaConvoy in data.event to expand 
#define CONFIG_AI_ACTION_EXPA_Teleportation 14 // Needs to remain for now as it's injected in Kernel/Wizardry/Misc/SkillEffects/AiSkills/Teleportation.event
#define CONFIG_AI_ACTION_AMT 20				   // Ai action expansion
#define CONFIG_UNIT_ACTION_EXPA_ExecSkill 0x23
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat 0x24
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff 0x25
#define CONFIG_UNIT_ACTION_AMT 0x30           // Unit action expansion
#define CONFIG_CUSTOM_GUIDE //Enable flag 0xB4 to view (configurable in GuideTable.event)
#define CONFIG_VESLY_DRAW_ANIMATIONS // Installation costs about 200KB
// #define CONFIG_VESLY_RECLASS // Ovewrites the juna fruit. Maybe look into not using an item at all?
#define CONFIG_CHAPTER_NAMES         // All ASM, so easier to use build time config
#define CONFIG_STAT_SCREEN_ALLEGIANCE_COLORS // All ASM, so easier to use build time config
#define CONFIG_TEXT_ENGINE_REWORK    // All ASM, so easier to use build time config
#define CONFIG_SHOW_CGs_LIKE_FE7     // All ASM, so easier to use build time config
#define CONFIG_UNIT_SELECTION_QUOTES // Installs assets so needs a build time config
#define CONFIG_VOICE_ACTED_DIALOGUE  // Installs assets so needs a build time config
#define CONFIG_VESLY_SHOOT_ARROW     // All ASM, so easier to use build time config
#define CONFIG_RES_TERRAIN_WINDOW    // All ASM, so easier to use build time config
#define CONFIG_MAX_COLOR_BACKGROUNDS // All ASM, so easier to use build time config
// #define CONFIG_LIGHTS_OUT_GAME    // All ASM, so easier to use build time config
#define CONFIG_MISC_UNIT_COUNTERS    // Used for Skill - Bravely Default (It also edits the unit struct bits) 

/**
 * Icon config
 */
#define CONFIG_PR_ITEM_ICON  0x5926F4
#include "../constants/item-icons.h"

/**
* Item config
*/
#define CONFIG_PR_ITEM_TABLE 0x809B10
#define CONFIG_ITEM_INDEX_MAG_BOOSTER 0xBC
// #define CONFIG_ITEM_INDEX_SKILL_SCROLL 0xBD

#define CONFIG_ITEM_INDEX_SKILL_STEALER 0xBE
#define CONFIG_ITEM_INDEX_ARMS_SCROLL 0xDA

#define CONFIG_FORGE_CHECKER 5000

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

// #define CONFIG_VESLY_NOTIFICATION_SYSTEM /* Has issues with setting custom notifications and displaying UTF8 text */
// #define CONFIG_VESLY_AOE /* This needs to be updated with Vesly's latest fixes as it's affecting the item menu and minimug palettes */
// #define CONFIG_VESLY_AVATAR

#define CONFIG_TURN_ON_ALL_SKILLS

#ifdef CONFIG_TURN_ON_ALL_SKILLS
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0x0A
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_2 0xBD
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_3 0xC9
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_4 0xCA
#else
	#define CONFIG_ITEM_INDEX_SKILL_SCROLL_1 0xBD
#endif

/* Not installed because of conflicts with phase suspend and sound mixer */
// #define CONFIG_CUSTOM_CHAPTER_SCREEN

// #define CONFIG_MOKHA_AOE // This only adds one command rather than letting you define them, turn of Vesly's AOE if using this

// #define CONFIG_FE7_MODE_SELECT // Graphical errors. needs the RAM allocation in config-memmap.s at line 109 turned on

/* 
** For the portrait formatting, it only works by calling the HalfBodyFormatter.exe in my own local drive in the downloads folder,
** so will not work for anyone else unless they're using WSL (Windows Subsystem in Linux) and put the exe there or using Wine.
** For now I've put a copy in EA's "Tools" folder for WSL users to move. If I get Vesly's Python portrait formatter working I may switch.
*/
// #define CONFIG_HALF_BODY_PORTRAITS