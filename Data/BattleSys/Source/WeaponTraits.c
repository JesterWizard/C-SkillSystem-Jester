#include "common-chax.h"
#include "battle-system.h"

/**
 * Weapon traits (Documentation/Features/WeaponTraits.md): which classes spears and
 * magic are effective against, while the designer toggle weapon_traits is on.
 * Cavalry is vanilla's Horseslayer list; armour is vanilla's Armorslayer list plus
 * the Wyvern Lords and the Marshall (the General's third tier).
 */
const u8 gWeaponTraitJConf[0x100] = {
	[CLASS_TROUBADOUR]           = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_VALKYRIE]             = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_CAVALIER]             = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_CAVALIER_F]           = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_PALADIN]              = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_PALADIN_F]            = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_RANGER]               = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_RANGER_F]             = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_MAGE_KNIGHT]          = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_MAGE_KNIGHT_F]        = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_TARVOS]               = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_MAELDUIN]             = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_EPHRAIM_MASTER_LORD]  = WEAPON_TRAIT_CLASS_CAVALRY,
	[CLASS_EIRIKA_MASTER_LORD]   = WEAPON_TRAIT_CLASS_CAVALRY,

	[CLASS_GREAT_KNIGHT]         = WEAPON_TRAIT_CLASS_CAVALRY | WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_GREAT_KNIGHT_F]       = WEAPON_TRAIT_CLASS_CAVALRY | WEAPON_TRAIT_CLASS_ARMOR,

	[CLASS_ARMOR_KNIGHT]         = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_ARMOR_KNIGHT_F]       = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_GENERAL]              = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_GENERAL_F]            = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_MARSHALL]             = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_WYVERN_LORD]          = WEAPON_TRAIT_CLASS_ARMOR,
	[CLASS_WYVERN_LORD_F]        = WEAPON_TRAIT_CLASS_ARMOR,
};
