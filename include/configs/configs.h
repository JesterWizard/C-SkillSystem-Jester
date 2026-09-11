//#include "config-debug.h"
#include "config-memmap.h"

// These need to remain as build time configs due to predominately using event/asm files
#define CONFIG_FE8_REWRITE
#define CONFIG_CROP_VANILLA_MSG 			   // Remove vanilla conversations and save 470KB of data, FEB will report errors but can be ignored
#define CONFIG_COMMON_PROTECTION_ENABLED 	   // I'm not even sure what this does, so I ain't touching it
#define CONFIG_VERIFY_SKILLSYS_SRAM 		   // This is a clever piece of protection that prevents the loading of saves created from this buildfile on incompatible copies of FE8
// #define CONFIG_INSTALL_STATSCREENFX 		   // FE7 stat screen. It causes graphical glitches with the additional pages past 4 and the extended desc textbox
#define CONFIG_INSTALL_CONVOYEXPA_AMT 200      // You'll need to adjust the EMSChunks for SaveExpaConvoy/LoadExpaConvoy in data.event to expand 
#define CONFIG_AI_ACTION_EXPA_Teleportation 14 // Needs to remain for now as it's injected in Kernel/Wizardry/SkillEffects/AiSkills/Teleportation.event
#define CONFIG_AI_ACTION_EXPA_MenuSkill 15      // Generic AI menu-skill execution
#define CONFIG_AI_ACTION_EXPA_Rescue 16         // AI rescue action for adjacent low-HP allies
#define CONFIG_AI_ACTION_EXPA_Drop 17           // AI drop action after reaching a safe tile
#define CONFIG_AI_ACTION_AMT 20				   // Ai action expansion
#define CONFIG_UNIT_ACTION_EXPA_ExecSkill 0x23
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat 0x24
#define CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff 0x25
#define CONFIG_UNIT_ACTION_EXPA_Gambit 0x26
#define CONFIG_UNIT_ACTION_AMT 0x30           // Unit action expansion
#define CONFIG_VESLY_SHOOT_ARROW     // All ASM, so easier to use build time config
#define CONFIG_MISC_UNIT_COUNTERS    // Used for Skill - Bravely Default (It also edits the unit struct bits) 

/**
 * Icon config
 */
#define CONFIG_PR_ITEM_ICON  0x5926F4
#include "../constants/item-icons.h"

/**
 * UI / dialogue glyph set. Define exactly one, then rebuild fonts.
 * Viable vs rejected: Documentation/Features/Fonts.md
 * Event Assembler only understands #ifdef, so do not use #if/#error here.
 */
// #define CONFIG_FONT_VANILLA
// #define CONFIG_FONT_POKE_EMERALD
// #define CONFIG_FONT_ADVANCE_WARS_2
// #define CONFIG_FONT_SUPER_STAR_SAGA
// #define CONFIG_FONT_MOTHER_3
#define CONFIG_FONT_RIVIERA

/**
* Item config
*/
#define CONFIG_PR_ITEM_TABLE 0x809B10
#define CONFIG_ITEM_INDEX_MAG_BOOSTER 0xBC
// #define CONFIG_ITEM_INDEX_SKILL_SCROLL 0xBD

#define CONFIG_ITEM_INDEX_SKILL_STEALER 0xBE
#define CONFIG_ITEM_INDEX_ARMS_SCROLL 0xDA

#define CONFIG_FORGE_CHECKER 5000

// Unit counts and save layout stay expanded. Gameplay is gated by
// KernelDesigerConfig::fourth_allegiance. Run a full make clean if you
// ever undefine this.
#define CONFIG_FOURTH_ALLEGIANCE

#ifdef CONFIG_FOURTH_ALLEGIANCE
 	#define CONFIG_UNIT_AMT_ALLY  41
	#define CONFIG_UNIT_AMT_FOURTH 10
#else
	#define CONFIG_UNIT_AMT_ALLY  51
	#define CONFIG_UNIT_AMT_FOURTH 0
#endif

#define CONFIG_UNIT_AMT_ENEMY 50
#define CONFIG_UNIT_AMT_NPC   8

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

// Voiced unit-select clips are gated by KernelDesigerConfig::unit_selection_quotes.
// FE7-style text chapter titles are gated by KernelDesigerConfig::chapter_names.
// FE7 difficulty select is gated by KernelDesigerConfig::fe7_mode_select.
// Heart Seal reclass is gated by KernelDesigerConfig::vesly_reclass.
// Notification toasts are gated by KernelDesigerConfig::vesly_notification_window.
// Extra map-action animations are gated by KernelDesigerConfig::vesly_draw_animations.