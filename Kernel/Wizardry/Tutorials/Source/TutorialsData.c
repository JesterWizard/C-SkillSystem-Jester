#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "bmtrick.h"
#include "tutorials.h"
#include "constants/texts.h"
#include "constants/skills.h"
#include "TutorialTitles.h"
#include "jester_headers/maps.h"
#include "jester_headers/macros.h"
#include "jester_headers/custom-arrays.h"
#include "kernel/traps.h"
#include "functions.h"

/*
 * Extras > Tutorials' list (Documentation/Features/Tutorials.md): the README's gameplay features that are on in this
 * build, in the README's order. Each has a title (its message, and its image, which Tools/tutorial-titles/titles.py
 * draws from the message: up to 128 pixels), a summary for the box (four lines at most), the explanation shown on its
 * map, and its map: TUTORIAL_MAP with only the units and set-up it needs, or Port Kiris (with only the tutorial's spot
 * or people) for the districts and their games. The
 * texts are at the end of Contents/Texts/Source/texts/Districts.txt.
 *
 * TUTORIAL_MAP is vanilla Chapter 4's map (15x15): the players start round (4, 5), the enemies round (9-11, 4-6).
 */

void EnableFreeMovementASMC(void);
void StartSpawnClassFx(ProcPtr parent, int x, int y, int classId);
int HexNpc_FindDeck(int chapter, int pid);

#define BLUE(pid, jid, lv, x, y, ...) \
	UNIT_ENTRY(pid, jid, FACTION_ID_BLUE, NO_ITEM_DROP, NO_AUTOLEVEL, lv, x, y, 0, NULL, 0, 0, 0, 0, __VA_ARGS__)
/* An enemy that attacks what it can reach and moves towards the player's units */
#define RED(pid, jid, lv, x, y, ...) \
	UNIT_ENTRY(pid, jid, FACTION_ID_RED, NO_ITEM_DROP, AUTOLEVEL, lv, x, y, 0, NULL, 0, 0, 9, 0, __VA_ARGS__)
/* An enemy that stays where it is and attacks what comes into reach */
#define RED_STILL(pid, jid, lv, x, y, ...) \
	UNIT_ENTRY(pid, jid, FACTION_ID_RED, NO_ITEM_DROP, AUTOLEVEL, lv, x, y, 0, NULL, 0, 3, 9, 0, __VA_ARGS__)

#define EIRIKA(x, y, ...)   BLUE(CHARACTER_EIRIKA, CLASS_EIRIKA_LORD, 5, x, y, __VA_ARGS__)
#define SOLDIER(x, y)       RED(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, x, y, ITEM_LANCE_IRON)

/* title, summary, title image, explanation, then the map's units, set-up and after-explanation step */
#define MAP_TUTORIAL(title, text, img, intro, units, setup, after) \
	{ title, text, img, intro, TUTORIAL_MAP, units, setup, after }
/* The districts and their games: Port Kiris, which has every game (and Snek) */
#define TOWN_TUTORIAL(title, text, img, intro, setup) \
	{ title, text, img, intro, DISTRICT_PORT_KIRIS, NULL, setup, NULL }

/* ---- Set-up ---- */

static struct Unit *TutUnit(int pid)
{
	return GetUnitFromCharId(pid);
}

/* The unit on a tile (units are on the map by now: TutorialMap_Setup refreshes the map first) */
static struct Unit *TutUnitAt(int x, int y)
{
	int uid = gBmMapUnit[y][x];

	return uid ? GetUnit(uid) : NULL;
}

static void Setup_Cutin(void)
{
#if defined(SID_Crit) && (COMMON_SKILL_VALID(SID_Crit))
	AddSkill(TutUnit(CHARACTER_JOSHUA), SID_Crit);
#endif
}

static void Setup_HP(void)
{
	struct Unit *unit = TutUnit(CHARACTER_GILLIAM);

	if (unit) {
		unit->maxHP = 120;
		unit->curHP = 120;
	}
}

static void Setup_Seal(void)
{
#if defined(SID_MagicSeal) && (COMMON_SKILL_VALID(SID_MagicSeal))
	struct Unit *unit = TutUnitAt(9, 5);

	if (unit)
		AddSkill(unit, SID_MagicSeal);
#endif
}

/* Natasha knows Heal (Gaiden Magic): she heals Eirika with W.Mag */
static void Setup_StaffExp(void)
{
	struct Unit *unit = TutUnit(CHARACTER_EIRIKA);

	if (unit)
		unit->curHP = 5;
}

static void Setup_Rescue(void)
{
	struct Unit *unit = TutUnitAt(10, 5);

	if (unit)
		unit->curHP = 2;
}

/* Fog with a vision of 2, drawn at once (as the fog-vision event, Event29_SetFogVision, does) */
static void Setup_Fog(void)
{
	UpdateMapViewWithFog(2);
}

static void After_Teleport(ProcPtr event)
{
	static const struct UnitDefinition assassin[] = {
		RED(CHARACTER_SOLDIER_80, CLASS_ASSASSIN, 1, 9, 5, ITEM_SWORD_IRON),
		{},
	};

	LoadUnit(assassin);
	RefreshEntityBmMaps();
	RefreshUnitSprites();
	StartSpawnClassFx(event, 9 * 16 - gBmSt.camera.x, 5 * 16 - gBmSt.camera.y, CLASS_ASSASSIN);
}

static void Setup_FreeMove(void)
{
	EnableFreeMovementASMC();
}

static void Setup_Traps(void)
{
	AddHealTile(5, 4, 10, 99, TRAP_MAPSPRITE_PAL_DEFAULT);
	AddTeleportTilePair(3, 6, 12, 3);
}

static void Setup_Threat(void)
{
	gPlaySt.config.threat_arrows_on = 1;
}

/* A tonic's kind is its item's second byte (UnitDefinition items are one byte) */
static void Setup_Tonic(void)
{
	struct Unit *unit = TutUnit(CHARACTER_EIRIKA);

	if (unit) {
		unit->items[1] = (1 << 8) | ITEM_TONIC;
		unit->items[2] = (2 << 8) | ITEM_TONIC;
	}
}

static void Setup_Trainee(void)
{
	struct Unit *unit = TutUnit(CHARACTER_ROSS);

	if (unit)
		unit->exp = 99;
}

/* Lute is hurt, so Natasha's W.Mag has someone to heal */
static void Setup_Gaiden(void)
{
	struct Unit *unit = TutUnit(CHARACTER_LUTE);

	if (unit)
		unit->curHP = unit->maxHP / 2;
}

/*
 * The towns' tutorials: Port Kiris with only the tutorial's spot or people. Run by its opening before its NPCs start
 * walking (TutorialMap_TownSetup); the gate stays, to leave by.
 */
static bool TutTown_NotHexPlayer(struct Unit *unit)
{
	return HexNpc_FindDeck(gPlaySt.chapterIndex, UNIT_CHAR_ID(unit)) < 0;
}

static bool TutTown_NotPiedPiper(struct Unit *unit)
{
	return UNIT_CHAR_ID(unit) != 0x7E;
}

static bool TutTown_Everyone(struct Unit *unit)
{
	return true;
}

static void TutTown_Keep(int trapType, bool (*npcGoes)(struct Unit *unit))
{
	struct Trap *trap = GetTrap(0);
	int i;

	while (trap->type != TRAP_NONE) {
		if (trap->type != TRAP_DISTRICT_GATE && trap->type != trapType)
			RemoveTrap(trap);  /* the ones after it move down */
		else
			trap++;
	}

	for (i = FACTION_GREEN + 1; i < FACTION_GREEN + 0x40; i++) {
		struct Unit *unit = GetUnit(i);

		if (UNIT_IS_VALID(unit) && npcGoes(unit))
			ClearUnit(unit);
	}
	RefreshEntityBmMaps();
	RefreshUnitSprites();
}

static void Town_Fishing(void)
{
	TutTown_Keep(TRAP_FISHING_SPOT, TutTown_Everyone);
}

static void Town_Hex(void)
{
	TutTown_Keep(TRAP_NONE, TutTown_NotHexPlayer);
}

static void Town_Crossing(void)
{
	TutTown_Keep(TRAP_CROSSING_SPOT, TutTown_Everyone);
}

static void Town_Snek(void)
{
	TutTown_Keep(TRAP_NONE, TutTown_NotPiedPiper);
}

static void Town_Tower(void)
{
	TutTown_Keep(TRAP_TOWER_SPOT, TutTown_Everyone);
}

static void Town_Guild(void)
{
	TutTown_Keep(TRAP_GUILD_BOARD, TutTown_Everyone);
}

/* ---- Units: only what the tutorial needs ---- */

#define FIGHTER(x, y)       RED(CHARACTER_SOLDIER_80, CLASS_FIGHTER, 3, x, y, ITEM_AXE_IRON)
#define LUTE(x, y)          BLUE(CHARACTER_LUTE, CLASS_MAGE_F, 8, x, y)
#define NATASHA(x, y)       BLUE(CHARACTER_NATASHA, CLASS_CLERIC, 5, x, y)
#define COLM(x, y)          BLUE(CHARACTER_COLM, CLASS_THIEF, 5, x, y, ITEM_SWORD_IRON)

static const struct UnitDefinition TutUnits_Arts[] = {
	EIRIKA(4, 5, ITEM_SWORD_SLIM),
	SOLDIER(9, 5),
	{},
};

static const struct UnitDefinition TutUnits_Cutin[] = {
	BLUE(CHARACTER_JOSHUA, CLASS_MYRMIDON, 10, 5, 5, ITEM_SWORD_KILLER),
	FIGHTER(9, 5),
	{},
};

static const struct UnitDefinition TutUnits_Slots[] = {
	COLM(5, 5),
	{},
};

static const struct UnitDefinition TutUnits_HP[] = {
	BLUE(CHARACTER_GILLIAM, CLASS_GENERAL, 10, 5, 5, ITEM_LANCE_SILVER),
	SOLDIER(9, 5),
	{},
};

static const struct UnitDefinition TutUnits_Gaiden[] = {
	LUTE(5, 5),
	NATASHA(4, 5),
	SOLDIER(9, 5),
	{},
};

static const struct UnitDefinition TutUnits_MP[] = {
	LUTE(5, 5),
	SOLDIER(9, 5),
	{},
};

static const struct UnitDefinition TutUnits_Reward[] = {
	BLUE(CHARACTER_ARTUR, CLASS_MONK, 10, 5, 5),
	RED_STILL(CHARACTER_NOVALA, CLASS_SHAMAN, 1, 9, 5, ITEM_DARK_FLUX),
	{},
};

static const struct UnitDefinition TutUnits_Seal[] = {
	LUTE(5, 5),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_FIGHTER, 1, 9, 5, ITEM_AXE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Eirika[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	{},
};

static const struct UnitDefinition TutUnits_StaffExp[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	NATASHA(5, 5),
	{},
};

static const struct UnitDefinition TutUnits_Rescue[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 10, 5, ITEM_LANCE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_PALADIN, 1, 11, 5, ITEM_LANCE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Promote[] = {
	BLUE(CHARACTER_ROSS, CLASS_JOURNEYMAN, 1, 7, 5, ITEM_AXE_IRON),
	RED(CHARACTER_SOLDIER_80, CLASS_FIGHTER, 10, 9, 5, ITEM_AXE_STEEL),
	{},
};

static const struct UnitDefinition TutUnits_Fog[] = {
	COLM(4, 5),
	SOLDIER(10, 4),
	SOLDIER(11, 6),
	{},
};

static const struct UnitDefinition TutUnits_FogVision[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	COLM(4, 6),
	SOLDIER(10, 4),
	SOLDIER(11, 6),
	{},
};

static const struct UnitDefinition TutUnits_Threat[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	SOLDIER(10, 5),
	SOLDIER(11, 6),
	{},
};

static const struct UnitDefinition TutUnits_Prestige[] = {
	BLUE(CHARACTER_FRANZ, CLASS_CAVALIER, 10, 5, 5, ITEM_SWORD_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Trainee[] = {
	BLUE(CHARACTER_ROSS, CLASS_JOURNEYMAN, 9, 7, 5, ITEM_AXE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 8, 5, ITEM_LANCE_IRON),
	{},
};

/* ---- The list ---- */

const struct Tutorial gTutorials[] = {
	/* Battle & Combat */
	MAP_TUTORIAL(MSG_TUT_ARTS_TITLE, MSG_TUT_ARTS, Img_TutTitle_ARTS, MSG_TUT_ARTS_MAP,
		     TutUnits_Arts, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_CUTIN_TITLE, MSG_TUT_CUTIN, Img_TutTitle_CUTIN, MSG_TUT_CUTIN_MAP,
		     TutUnits_Cutin, Setup_Cutin, NULL),
	MAP_TUTORIAL(MSG_TUT_SLOTS_TITLE, MSG_TUT_SLOTS, Img_TutTitle_SLOTS, MSG_TUT_SLOTS_MAP,
		     TutUnits_Slots, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_HP_TITLE, MSG_TUT_HP, Img_TutTitle_HP, MSG_TUT_HP_MAP,
		     TutUnits_HP, Setup_HP, NULL),
	MAP_TUTORIAL(MSG_TUT_GAIDEN_TITLE, MSG_TUT_GAIDEN, Img_TutTitle_GAIDEN, MSG_TUT_GAIDEN_MAP,
		     TutUnits_Gaiden, Setup_Gaiden, NULL),
	MAP_TUTORIAL(MSG_TUT_MP_TITLE, MSG_TUT_MP, Img_TutTitle_MP, MSG_TUT_MP_MAP,
		     TutUnits_MP, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_REWARD_TITLE, MSG_TUT_REWARD, Img_TutTitle_REWARD, MSG_TUT_REWARD_MAP,
		     TutUnits_Reward, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_SEAL_TITLE, MSG_TUT_SEAL, Img_TutTitle_SEAL, MSG_TUT_SEAL_MAP,
		     TutUnits_Seal, Setup_Seal, NULL),
	MAP_TUTORIAL(MSG_TUT_TONIC_TITLE, MSG_TUT_TONIC, Img_TutTitle_TONIC, MSG_TUT_TONIC_MAP,
		     TutUnits_Eirika, Setup_Tonic, NULL),

	/*
	 * Staves. The Arbalest, Aum, Phoenix and Skill staves are left out: with Three Houses magic no unit keeps a staff
	 * item (PrePhase_RemoveThreeHousesMagicItems), and no unit learns them as spells.
	 */
	MAP_TUTORIAL(MSG_TUT_STAFFEXP_TITLE, MSG_TUT_STAFFEXP, Img_TutTitle_STAFFEXP, MSG_TUT_STAFFEXP_MAP,
		     TutUnits_StaffExp, Setup_StaffExp, NULL),

	/* Maps, Fog & AI */
	MAP_TUTORIAL(MSG_TUT_RESCUE_TITLE, MSG_TUT_RESCUE, Img_TutTitle_RESCUE, MSG_TUT_RESCUE_MAP,
		     TutUnits_Rescue, Setup_Rescue, NULL),
	MAP_TUTORIAL(MSG_TUT_PROMOTE_TITLE, MSG_TUT_PROMOTE, Img_TutTitle_PROMOTE, MSG_TUT_PROMOTE_MAP,
		     TutUnits_Promote, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_EFOG_TITLE, MSG_TUT_EFOG, Img_TutTitle_EFOG, MSG_TUT_EFOG_MAP,
		     TutUnits_Fog, Setup_Fog, NULL),
	MAP_TUTORIAL(MSG_TUT_FOGSTAGES_TITLE, MSG_TUT_FOGSTAGES, Img_TutTitle_FOGSTAGES, MSG_TUT_FOGSTAGES_MAP,
		     TutUnits_Fog, Setup_Fog, NULL),
	MAP_TUTORIAL(MSG_TUT_FOGVISION_TITLE, MSG_TUT_FOGVISION, Img_TutTitle_FOGVISION, MSG_TUT_FOGVISION_MAP,
		     TutUnits_FogVision, Setup_Fog, NULL),
	MAP_TUTORIAL(MSG_TUT_TELEPORT_TITLE, MSG_TUT_TELEPORT, Img_TutTitle_TELEPORT, MSG_TUT_TELEPORT_MAP,
		     TutUnits_Eirika, NULL, After_Teleport),
	MAP_TUTORIAL(MSG_TUT_FREEMOVE_TITLE, MSG_TUT_FREEMOVE, Img_TutTitle_FREEMOVE, MSG_TUT_FREEMOVE_MAP,
		     TutUnits_Eirika, Setup_FreeMove, NULL),
	MAP_TUTORIAL(MSG_TUT_GHOST_TITLE, MSG_TUT_GHOST, Img_TutTitle_GHOST, MSG_TUT_GHOST_MAP,
		     TutUnits_Eirika, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_TRAPS_TITLE, MSG_TUT_TRAPS, Img_TutTitle_TRAPS, MSG_TUT_TRAPS_MAP,
		     TutUnits_Eirika, Setup_Traps, NULL),
	MAP_TUTORIAL(MSG_TUT_THREAT_TITLE, MSG_TUT_THREAT, Img_TutTitle_THREAT, MSG_TUT_THREAT_MAP,
		     TutUnits_Threat, Setup_Threat, NULL),

	/* Growth & Progression */
	MAP_TUTORIAL(MSG_TUT_PRESTIGE_TITLE, MSG_TUT_PRESTIGE, Img_TutTitle_PRESTIGE, MSG_TUT_PRESTIGE_MAP,
		     TutUnits_Prestige, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_CAPACITY_TITLE, MSG_TUT_CAPACITY, Img_TutTitle_CAPACITY, MSG_TUT_CAPACITY_MAP,
		     TutUnits_Eirika, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_TRAINEE_TITLE, MSG_TUT_TRAINEE, Img_TutTitle_TRAINEE, MSG_TUT_TRAINEE_MAP,
		     TutUnits_Trainee, Setup_Trainee, NULL),

	/* Towns & Minigames */
	TOWN_TUTORIAL(MSG_TUT_DISTRICTS_TITLE, MSG_TUT_DISTRICTS, Img_TutTitle_DISTRICTS, MSG_TUT_DISTRICTS_MAP,
		      NULL),
	TOWN_TUTORIAL(MSG_TUT_FISHING_TITLE, MSG_TUT_FISHING, Img_TutTitle_FISHING, MSG_TUT_FISHING_MAP,
		      Town_Fishing),
	TOWN_TUTORIAL(MSG_TUT_HEX_TITLE, MSG_TUT_HEX, Img_TutTitle_HEX, MSG_TUT_HEX_MAP,
		      Town_Hex),
	TOWN_TUTORIAL(MSG_TUT_CROSSING_TITLE, MSG_TUT_CROSSING, Img_TutTitle_CROSSING, MSG_TUT_CROSSING_MAP,
		      Town_Crossing),
	TOWN_TUTORIAL(MSG_TUT_SNEK_TITLE, MSG_TUT_SNEK, Img_TutTitle_SNEK, MSG_TUT_SNEK_MAP,
		      Town_Snek),
	TOWN_TUTORIAL(MSG_TUT_TOWER_TITLE, MSG_TUT_TOWER, Img_TutTitle_TOWER, MSG_TUT_TOWER_MAP,
		      Town_Tower),
	TOWN_TUTORIAL(MSG_TUT_GUILD_TITLE, MSG_TUT_GUILD, Img_TutTitle_GUILD, MSG_TUT_GUILD_MAP,
		      Town_Guild),

	{ 0 },
};
