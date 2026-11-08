#include "common-chax.h"
#include "kernel/unit-icon.h"

/*
 * Standing map sprite table, repointed from vanilla unit_icon_wait_table
 * (0x088AF880). The vanilla table has only 0x6B entries: gSpellAssocData
 * follows it, so new sprites cannot be written past its end.
 *
 * The custom images are installed by Kernel/Wizardry/TrapData/TrapData_Installer.event.
 */

extern char Gfx_Unlit_Torch[];
extern char Gfx_Teleport_Tile[];
extern char Gfx_Grass_Tile[];
extern char Gfx_Boulder_Tile[];
extern char Gfx_Spin_Tile_RIGHT[];
extern char Gfx_Spin_Tile_LEFT[];
extern char Gfx_Spin_Tile_UP[];
extern char Gfx_Spin_Tile_DOWN[];
extern char Gfx_Hellscape_Void_Tile[];
extern char Gfx_Caltrops_Tile[];
extern char Gfx_Floor_Intact_Tile[];
extern char Gfx_Floor_Cracked_Tile[];
extern char Gfx_Floor_Hole_Tile[];
extern char Gfx_Heal_Tile[];
extern char Gfx_Dragon_Vein_Tile[];
extern char Gfx_Lit_Torch[];

#define TRAP_SMS(_sheet) { 2, UNIT_ICON_SIZE_16x16, (_sheet) }

const UnitIconWait gUnitIconWaitTable[UNIT_ICON_WAIT_AMT] = {
	/* Vanilla sprites */
	[0x00] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Ephraim_Lord_sheet },
	[0x01] = { 0, UNIT_ICON_SIZE_16x16, unit_icon_wait_Eirika_Lord_sheet },
	[0x02] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Ephraim_Master_Lord_sheet },
	[0x03] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Eirika_Master_Lord_sheet },
	[0x04] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Cavalier_sheet },
	[0x05] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Paladin_sheet },
	[0x06] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Armor_Knight_sheet },
	[0x07] = { 0, UNIT_ICON_SIZE_16x32, unit_icon_wait_General_sheet },
	[0x08] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Thief_sheet },
	[0x09] = { 0, UNIT_ICON_SIZE_16x16, unit_icon_wait_Blank_sheet },
	[0x0A] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Mercenary_sheet },
	[0x0B] = { 2, UNIT_ICON_SIZE_32x32, unit_icon_wait_Mercenary_F_sheet },
	[0x0C] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Hero_sheet },
	[0x0D] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Hero_F_sheet },
	[0x0E] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Myrmidon_sheet },
	[0x0F] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Myrmidon_F_sheet },
	[0x10] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Swordmaster_sheet },
	[0x11] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Swordmaster_F_sheet },
	[0x12] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Assassin_sheet },
	[0x13] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Assassin_F_sheet },
	[0x14] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Archer_sheet },
	[0x15] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Archer_F_sheet },
	[0x16] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Sniper_sheet },
	[0x17] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Sniper_F_sheet },
	[0x18] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Ranger_sheet },
	[0x19] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Wyvern_Rider_sheet },
	[0x1A] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Wyvern_Lord_sheet },
	[0x1B] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Wyvern_Lord_F_sheet },
	[0x1C] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Wyvern_Knight_sheet },
	[0x1D] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Wyvern_Knight_F_sheet },
	[0x1E] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Mage_sheet },
	[0x1F] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Mage_F_sheet },
	[0x20] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Sage_sheet },
	[0x21] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Sage_F_sheet },
	[0x22] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Mage_Knight_sheet },
	[0x23] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Mage_Knight_F_sheet },
	[0x24] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Bishop_sheet },
	[0x25] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Bishop_F_sheet },
	[0x26] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Shaman_sheet },
	[0x27] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Druid_sheet },
	[0x28] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Summoner_sheet },
	[0x29] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Rogue_sheet },
	[0x2A] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Gorgonegg2_sheet },
	[0x2B] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Great_Knight_sheet },
	[0x2C] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Recruit_T1_sheet },
	[0x2D] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Journeyman_T2_sheet },
	[0x2E] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Pupil_T2_sheet },
	[0x2F] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Manakete_2_sheet },
	[0x30] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Manakete_Myrrh_sheet },
	[0x31] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Fighter_sheet },
	[0x32] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Warrior_sheet },
	[0x33] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Brigand_sheet },
	[0x34] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Pirate_sheet },
	[0x35] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Berserker_sheet },
	[0x36] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Monk_sheet },
	[0x37] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Priest_sheet },
	[0x38] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Bard_sheet },
	[0x39] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Pegasus_Knight_sheet },
	[0x3A] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Falcon_Knight_sheet },
	[0x3B] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Cleric_sheet },
	[0x3C] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Troubadour_sheet },
	[0x3D] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Valkyrie_sheet },
	[0x3E] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Dancer_sheet },
	[0x3F] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Soldier_sheet },
	[0x40] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Necromancer_sheet },
	[0x41] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Fleet_sheet },
	[0x42] = { 2, UNIT_ICON_SIZE_32x32, unit_icon_wait_Phantom_sheet },
	[0x43] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Revenant_sheet },
	[0x44] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Entoumbed_sheet },
	[0x45] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Bonewalker_sheet },
	[0x46] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Bonewalker_Bow_sheet },
	[0x47] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Wight_sheet },
	[0x48] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Wight_Bow_sheet },
	[0x49] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Bael_sheet },
	[0x4A] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Elder_Bael_sheet },
	[0x4B] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Cyclops_sheet },
	[0x4C] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Mauthedoog_sheet },
	[0x4D] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Gwyllgi_sheet },
	[0x4E] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Tarvos_sheet },
	[0x4F] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Maelduin_sheet },
	[0x50] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Mogall_sheet },
	[0x51] = { 0, UNIT_ICON_SIZE_16x32, unit_icon_wait_Arch_Mogall_sheet },
	[0x52] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Gorgon_sheet },
	[0x53] = { 3, UNIT_ICON_SIZE_16x16, unit_icon_wait_Gorgonegg_sheet },
	[0x54] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Gargoyle_sheet },
	[0x55] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Deathgoyle_sheet },
	[0x56] = { 3, UNIT_ICON_SIZE_16x32, unit_icon_wait_Draco_Zombie_sheet },
	[0x57] = { 3, UNIT_ICON_SIZE_32x32, unit_icon_wait_Demon_King_sheet },
	[0x58] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Regular_Used_sheet },
	[0x59] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Long_Used_sheet },
	[0x5A] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Killer_Used_sheet },
	[0x5B] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Long_Empty_sheet },
	[0x5C] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Used_Empty_sheet },
	[0x5D] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Blst_Killer_Empty_sheet },
	[0x5E] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Civilian_M1_sheet },
	[0x5F] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Civilian_F1_sheet },
	[0x60] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Civilian_M2_sheet },
	[0x61] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Civilian_F2_sheet },
	[0x62] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Peer_sheet },
	[0x63] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Queen_sheet },
	[0x64] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Prince_sheet },
	[0x65] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Queen_2_sheet },
	[0x66] = { 0, UNIT_ICON_SIZE_16x16, unit_icon_wait_Unk77_sheet },
	[0x67] = { 2, UNIT_ICON_SIZE_16x16, unit_icon_wait_Fallen_Prince_sheet },
	[0x68] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Tent_sheet },
	[0x69] = { 2, UNIT_ICON_SIZE_16x32, unit_icon_wait_Pontifex_sheet },
	[0x6A] = { 0, UNIT_ICON_SIZE_16x16, unit_icon_wait_Fallen_Peer_sheet },

	/* Trap sprites */
	[SMS_ID_UNLIT_TORCH] = TRAP_SMS(Gfx_Unlit_Torch),
	[SMS_ID_TELEPORT_TILE] = TRAP_SMS(Gfx_Teleport_Tile),
	[SMS_ID_GRASS_TILE] = TRAP_SMS(Gfx_Grass_Tile),
	[SMS_ID_BOULDER_TILE] = TRAP_SMS(Gfx_Boulder_Tile),
	[SMS_ID_SPIN_TILE_RIGHT] = TRAP_SMS(Gfx_Spin_Tile_RIGHT),
	[SMS_ID_SPIN_TILE_LEFT] = TRAP_SMS(Gfx_Spin_Tile_LEFT),
	[SMS_ID_SPIN_TILE_UP] = TRAP_SMS(Gfx_Spin_Tile_UP),
	[SMS_ID_SPIN_TILE_DOWN] = TRAP_SMS(Gfx_Spin_Tile_DOWN),
	[SMS_ID_HELLSCAPE_VOID] = TRAP_SMS(Gfx_Hellscape_Void_Tile),
	[SMS_ID_CALTROPS] = TRAP_SMS(Gfx_Caltrops_Tile),
	[SMS_ID_COLLAPSING_FLOOR + COLLAPSING_FLOOR_INTACT] = TRAP_SMS(Gfx_Floor_Intact_Tile),
	[SMS_ID_COLLAPSING_FLOOR + COLLAPSING_FLOOR_CRACKED] = TRAP_SMS(Gfx_Floor_Cracked_Tile),
	[SMS_ID_COLLAPSING_FLOOR + COLLAPSING_FLOOR_HOLE] = TRAP_SMS(Gfx_Floor_Hole_Tile),
	[SMS_ID_HEAL_TILE] = TRAP_SMS(Gfx_Heal_Tile),
	[SMS_ID_DRAGON_VEIN] = TRAP_SMS(Gfx_Dragon_Vein_Tile),
	[SMS_ID_LIT_TORCH] = TRAP_SMS(Gfx_Lit_Torch),
};
