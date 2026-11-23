#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "bmtrick.h"
#include "tutorials.h"
#include "gaiden-magic.h"
#include "strmag.h"
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
void StartSpawnClassFxFaction(ProcPtr parent, int x, int y, int classId, int faction);

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

/*
 * title, summary, title image, explanation, then the map's units, set-up and after-explanation step, and its guided
 * steps (TutorialGuides.c: EventScr_TutG_<guide>), or NULL for a lesson with nothing to do but watch
 */
#define MAP_TUTORIAL(title, text, img, intro, units, setup, after, guide) \
	{ title, text, img, intro, TUTORIAL_MAP, units, setup, after, guide }
/* The districts and their games: Port Kiris, which has every game (and Snek); no guided steps, they are walks */
#define TOWN_TUTORIAL(title, text, img, intro, setup) \
	{ title, text, img, intro, DISTRICT_PORT_KIRIS, NULL, setup, NULL, NULL }

extern const EventScr EventScr_TutG_Arts[], EventScr_TutG_Cutin[], EventScr_TutG_Slots[], EventScr_TutG_HP[];
extern const EventScr EventScr_TutG_Gaiden[], EventScr_TutG_MP[], EventScr_TutG_Reward[], EventScr_TutG_Seal[];
extern const EventScr EventScr_TutG_Tonic[], EventScr_TutG_StaffExp[], EventScr_TutG_EnemyFirst[];
extern const EventScr EventScr_TutG_Turn[];
extern const EventScr EventScr_TutG_Promote[], EventScr_TutG_Ghost[], EventScr_TutG_Threat[];
extern const EventScr EventScr_TutG_Prestige[], EventScr_TutG_Trainee[];

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

/* The fighter's seal (10 tiles) reaches over most of the map: in a corner, its edge shows between Lute and it */
static void Setup_Seal(void)
{
#if defined(SID_MagicSeal) && (COMMON_SKILL_VALID(SID_MagicSeal))
	struct Unit *unit = TutUnitAt(13, 9);

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

/* Artur (a level 10 unit with his base stats) defeats Novala in one fight, doubling her */
static void Setup_Reward(void)
{
	struct Unit *unit = TutUnit(CHARACTER_ARTUR);

	if (unit) {
		UNIT_MAG(unit) = 20;
		unit->skl = 15;
		unit->spd = 15;
		unit->maxHP = 30;
		unit->curHP = 30;
	}
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
	StartSpawnClassFxFaction(event, 9 * 16 - gBmSt.camera.x, 5 * 16 - gBmSt.camera.y, CLASS_ASSASSIN, FACTION_RED);
}

/*
 * Free movement on, and its things to try (TutorialMap/events/events.c): the village at (8, 2), the armory at (1, 11),
 * Garcia to talk to. No end of its own: SELECT leaves, as its explanation says.
 */
static void Setup_FreeMove(void)
{
	ClearFlag(TUTMAP_FLAG_VILLAGE);
	SetPartyGoldAmount(1000);   /* to buy at the armory (a tutorial's new game has none) */
	EnableFreeMovementASMC();
}

/*
 * Traps: every trap a map can place (TrapData.c), round Eirika (4, 5), to try in any order; no end of its own (SELECT
 * leaves). Left out: the dragon vein (its command is not wired) and the Hellscape void (a skill places it). Eirika has
 * 60 HP: the fire, the gas and the arrows go off between turns. The house is the Free Movement tutorial's village
 * (TutorialMap/events), whose visit gives EXP once here (a repeat house).
 */
static void Setup_Traps(void)
{
	struct Unit *unit = TutUnit(CHARACTER_EIRIKA);

	if (unit) {
		unit->maxHP = 60;
		unit->curHP = 60;
	}

	ClearFlag(TUTMAP_FLAG_VILLAGE);
	AddRepeatHouse(8, 2, 0);
	AddHealTile(5, 4, 10, 99, TRAP_MAPSPRITE_PAL_DEFAULT);
	AddTeleportTilePair(3, 6, 12, 3);
	AddToggleTorch(6, 4, 3, true, TRAP_MAPSPRITE_PAL_NPC);
	AddLightRune(2, 4, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);
	AddSpinTile(5, 6, SPIN_TILE_DIR_RIGHT);
	AddGrassTile(6, 6, 0);
	AddHiddenCaltrops(6, 5);
	AddBoulderTile(8, 5);
	AddCollapsingFloor(8, 6);
	AddTrap(9, 6, TRAP_MINE, 0);
	AddFireTile(10, 5, 1, 2);
	AddGasTrap(11, 6, FACING_LEFT, 1, 2);
	AddArrowTrap(7, 1, 2);
}

u16 MakeSkillScrollItem(u16 sid);

/*
 * Skill Capacity: Eirika's equipped skills are five stat bonuses (rank C, 8 each: 40 of her 50), and her Items two scrolls: Astra
 * (rank S, 20) is more than she has left, Mercy (rank E, 3) fits. Saleh then teaches her Luna (rank A, 15) in a talk:
 * learned, but not equipped (AddSkillWithinCapacity, as after a level-up or a promotion).
 */
static void Setup_Capacity(void)
{
	struct Unit *unit = TutUnit(CHARACTER_EIRIKA);
	int i;

	if (!unit)
		return;

	for (i = 0; i < UNIT_RAM_SKILLS_LEN; i++)
		SET_SKILL(unit, i, 0);
	ResetSkillLists();

#if defined(SID_HpBonus) && (COMMON_SKILL_VALID(SID_HpBonus))
	AddSkill(unit, SID_HpBonus);
#endif
#if defined(SID_StrBonus) && (COMMON_SKILL_VALID(SID_StrBonus))
	AddSkill(unit, SID_StrBonus);
#endif
#if defined(SID_SpdBonus) && (COMMON_SKILL_VALID(SID_SpdBonus))
	AddSkill(unit, SID_SpdBonus);
#endif
#if defined(SID_DefBonus) && (COMMON_SKILL_VALID(SID_DefBonus))
	AddSkill(unit, SID_DefBonus);
#endif
#if defined(SID_SklBonus) && (COMMON_SKILL_VALID(SID_SklBonus))
	AddSkill(unit, SID_SklBonus);
#endif
#if defined(SID_Astra) && (COMMON_SKILL_VALID(SID_Astra))
	unit->items[1] = MakeSkillScrollItem(SID_Astra);
#endif
#if defined(SID_Mercy) && (COMMON_SKILL_VALID(SID_Mercy))
	unit->items[2] = MakeSkillScrollItem(SID_Mercy);
#endif
	ClearFlag(TUTMAP_FLAG_SALEH);
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

/* Enemy Promotion: the fighter's first blow defeats Ross (1 HP, no speed or luck to dodge it), and it promotes */
static void Setup_Promote(void)
{
	struct Unit *ross = TutUnit(CHARACTER_ROSS);
	struct Unit *fighter = TutUnitAt(9, 5);

	if (ross) {
		ross->curHP = 1;
		ross->spd = 0;
		ross->lck = 0;
	}

	if (fighter)
		fighter->skl = 20;
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
	BLUE(CHARACTER_GILLIAM, CLASS_GENERAL, 10, 5, 5, ITEM_LANCE_IRON),  /* a Silver Lance is over his rank */
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

/*
 * A soldier next to Lute's destination (7, 6), both inside the seal (10 tiles from the fighter): a target in her
 * spells' reach, and still no B.Mag in her menu
 */
static const struct UnitDefinition TutUnits_Seal[] = {
	LUTE(3, 6),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_FIGHTER, 1, 13, 9, ITEM_AXE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 8, 6, ITEM_LANCE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Eirika[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	{},
};

/*
 * Tonics: Franz beside Eirika, so a unit is left to act after her tonic and the player phase goes on: its next step is
 * R on her stats (with her alone the turn ended, and the stat step came at a phase change)
 */
static const struct UnitDefinition TutUnits_Tonic[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	BLUE(CHARACTER_FRANZ, CLASS_CAVALIER, 1, 3, 6, ITEM_SWORD_IRON),
	{},
};

/* Free Movement: Garcia (green) to talk to, between Eirika and the village at (8, 2) */
static const struct UnitDefinition TutUnits_FreeMove[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	UNIT_ENTRY(CHARACTER_GARCIA, CLASS_FIGHTER, FACTION_ID_GREEN, NO_ITEM_DROP, NO_AUTOLEVEL, 5, 6, 3, 0, NULL, 0, 3, 0, 0,
		   ITEM_AXE_IRON),
	{},
};

/* Skill Capacity: Saleh (green) beside Eirika, to talk to: he teaches her Luna (TutorialMap/events/events.c) */
static const struct UnitDefinition TutUnits_Capacity[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	UNIT_ENTRY(CHARACTER_SALEH, CLASS_SAGE, FACTION_ID_GREEN, NO_ITEM_DROP, NO_AUTOLEVEL, 1, 5, 5, 0, NULL, 0, 3, 0, 0,
		   ITEM_ANIMA_FIRE),
	{},
};

static const struct UnitDefinition TutUnits_StaffExp[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	NATASHA(5, 5),
	{},
};

/* The paladin first: the enemy phase's first action is its rescue (the soldier, unarmed, never charges first) */
static const struct UnitDefinition TutUnits_Rescue[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_PALADIN, 1, 11, 5, ITEM_LANCE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 10, 5),
	{},
};

/*
 * Eirika on Ross's row, out of the fighter's reach (7 tiles from it: it moves 5 and hits at 1), so the camera stays put
 * from her (the leader) to Ross and the fighter. With Ross the only player unit, his defeat was a game over, which
 * ended the map during the promotion's effect, before its popup.
 */
static const struct UnitDefinition TutUnits_Promote[] = {
	BLUE(CHARACTER_ROSS, CLASS_JOURNEYMAN, 1, 7, 5, ITEM_AXE_IRON),
	EIRIKA(2, 5, ITEM_SWORD_RAPIER),
	RED(CHARACTER_SOLDIER_80, CLASS_FIGHTER, 10, 9, 5, ITEM_AXE_STEEL),
	{},
};

static const struct UnitDefinition TutUnits_Fog[] = {
	COLM(4, 5),
	SOLDIER(10, 4),
	SOLDIER(11, 6),
	{},
};

/*
 * Fog sight (FogVision.c): 2, an archer 4, a thief 7. Colm saw every enemy here, so the fog showed nothing. Fog Stages:
 * Eirika (sight 2) and soldiers 3, 4 and 6 tiles off: each stage of the fog (seen but unreadable, a shape, hidden). Fog Vision: Neimi the archer
 * (sight 4) sees the soldier at (7, 6), 3 tiles from her, which Eirika beside her (4 tiles off) does not.
 */
static const struct UnitDefinition TutUnits_FogStages[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 7, 5, ITEM_LANCE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 8, 5, ITEM_LANCE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 10, 5, ITEM_LANCE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_FogVision[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	BLUE(CHARACTER_NEIMI, CLASS_ARCHER_F, 1, 4, 6, ITEM_BOW_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 7, 6, ITEM_LANCE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 10, 5, ITEM_LANCE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Threat[] = {
	EIRIKA(4, 5, ITEM_SWORD_RAPIER),
	SOLDIER(10, 5),
	SOLDIER(11, 6),
	{},
};

/* Vanessa: a unit left to act after Franz's Prestige, so the player phase goes on to its growths step */
static const struct UnitDefinition TutUnits_Prestige[] = {
	BLUE(CHARACTER_FRANZ, CLASS_CAVALIER, 10, 5, 5, ITEM_SWORD_IRON),
	BLUE(CHARACTER_VANESSA, CLASS_PEGASUS_KNIGHT, 1, 3, 6, ITEM_LANCE_IRON),
	{},
};

static const struct UnitDefinition TutUnits_Trainee[] = {
	BLUE(CHARACTER_ROSS, CLASS_JOURNEYMAN, 9, 7, 5, ITEM_AXE_IRON),
	RED_STILL(CHARACTER_SOLDIER_80, CLASS_SOLDIER, 1, 8, 5, ITEM_LANCE_IRON),
	{},
};

/* ---- The list ---- */

/*
 * MP and Three Houses magic are two ways to pay for a spell, never both in one game: with Three Houses magic (uses a
 * chapter) Gaiden Magic's tutorial is shown and MP System's is not; with the MP system alone, only MP System's.
 */
static bool Tutorial_InThisGame(int index)
{
	int title = gTutorials[index].title;

	if (title == MSG_TUT_MP_TITLE)
		return gpKernelDesignerConfig->mp_system && !IsGaidenMagicUsesSystem();
	if (title == MSG_TUT_GAIDEN_TITLE)
		return IsGaidenMagicUsesSystem();
	return true;
}

int Tutorials_Count(void)
{
	int i, n = 0;

	for (i = 0; gTutorials[i].title != 0; i++)
		if (Tutorial_InThisGame(i))
			n++;
	return n;
}

int Tutorials_Index(int shown)
{
	int i;

	for (i = 0; gTutorials[i].title != 0; i++)
		if (Tutorial_InThisGame(i) && shown-- == 0)
			return i;
	return 0;
}

int Tutorials_Shown(int index)
{
	int i, n = 0;

	for (i = 0; gTutorials[i].title != 0; i++) {
		if (!Tutorial_InThisGame(i))
			continue;
		if (i == index)
			return n;
		n++;
	}
	return -1;
}

const struct Tutorial gTutorials[] = {
	/* Battle & Combat */
	MAP_TUTORIAL(MSG_TUT_ARTS_TITLE, MSG_TUT_ARTS, Img_TutTitle_ARTS, MSG_TUT_ARTS_MAP,
		     TutUnits_Arts, NULL, NULL, EventScr_TutG_Arts),
	MAP_TUTORIAL(MSG_TUT_CUTIN_TITLE, MSG_TUT_CUTIN, Img_TutTitle_CUTIN, MSG_TUT_CUTIN_MAP,
		     TutUnits_Cutin, Setup_Cutin, NULL, EventScr_TutG_Cutin),
	MAP_TUTORIAL(MSG_TUT_SLOTS_TITLE, MSG_TUT_SLOTS, Img_TutTitle_SLOTS, MSG_TUT_SLOTS_MAP,
		     TutUnits_Slots, NULL, NULL, EventScr_TutG_Slots),
	MAP_TUTORIAL(MSG_TUT_HP_TITLE, MSG_TUT_HP, Img_TutTitle_HP, MSG_TUT_HP_MAP,
		     TutUnits_HP, Setup_HP, NULL, EventScr_TutG_HP),
	MAP_TUTORIAL(MSG_TUT_GAIDEN_TITLE, MSG_TUT_GAIDEN, Img_TutTitle_GAIDEN, MSG_TUT_GAIDEN_MAP,
		     TutUnits_Gaiden, Setup_Gaiden, NULL, EventScr_TutG_Gaiden),
	MAP_TUTORIAL(MSG_TUT_MP_TITLE, MSG_TUT_MP, Img_TutTitle_MP, MSG_TUT_MP_MAP,
		     TutUnits_MP, NULL, NULL, EventScr_TutG_MP),
	MAP_TUTORIAL(MSG_TUT_REWARD_TITLE, MSG_TUT_REWARD, Img_TutTitle_REWARD, MSG_TUT_REWARD_MAP,
		     TutUnits_Reward, Setup_Reward, NULL, EventScr_TutG_Reward),
	MAP_TUTORIAL(MSG_TUT_SEAL_TITLE, MSG_TUT_SEAL, Img_TutTitle_SEAL, MSG_TUT_SEAL_MAP,
		     TutUnits_Seal, Setup_Seal, NULL, EventScr_TutG_Seal),
	MAP_TUTORIAL(MSG_TUT_TONIC_TITLE, MSG_TUT_TONIC, Img_TutTitle_TONIC, MSG_TUT_TONIC_MAP,
		     TutUnits_Tonic, Setup_Tonic, NULL, EventScr_TutG_Tonic),

	/*
	 * Staves. The Arbalest, Aum, Phoenix and Skill staves are left out: with Three Houses magic no unit keeps a staff
	 * item (PrePhase_RemoveThreeHousesMagicItems), and no unit learns them as spells.
	 */
	MAP_TUTORIAL(MSG_TUT_STAFFEXP_TITLE, MSG_TUT_STAFFEXP, Img_TutTitle_STAFFEXP, MSG_TUT_STAFFEXP_MAP,
		     TutUnits_StaffExp, Setup_StaffExp, NULL, EventScr_TutG_StaffExp),

	/* Maps, Fog & AI */
	MAP_TUTORIAL(MSG_TUT_RESCUE_TITLE, MSG_TUT_RESCUE, Img_TutTitle_RESCUE, MSG_TUT_RESCUE_MAP,
		     TutUnits_Rescue, Setup_Rescue, NULL, EventScr_TutG_EnemyFirst),
	MAP_TUTORIAL(MSG_TUT_PROMOTE_TITLE, MSG_TUT_PROMOTE, Img_TutTitle_PROMOTE, MSG_TUT_PROMOTE_MAP,
		     TutUnits_Promote, Setup_Promote, NULL, EventScr_TutG_Promote),
	MAP_TUTORIAL(MSG_TUT_EFOG_TITLE, MSG_TUT_EFOG, Img_TutTitle_EFOG, MSG_TUT_EFOG_MAP,
		     TutUnits_Fog, Setup_Fog, NULL, EventScr_TutG_Turn),
	MAP_TUTORIAL(MSG_TUT_FOGSTAGES_TITLE, MSG_TUT_FOGSTAGES, Img_TutTitle_FOGSTAGES, MSG_TUT_FOGSTAGES_MAP,
		     TutUnits_FogStages, Setup_Fog, NULL, EventScr_TutG_Turn),
	MAP_TUTORIAL(MSG_TUT_FOGVISION_TITLE, MSG_TUT_FOGVISION, Img_TutTitle_FOGVISION, MSG_TUT_FOGVISION_MAP,
		     TutUnits_FogVision, Setup_Fog, NULL, EventScr_TutG_Turn),
	MAP_TUTORIAL(MSG_TUT_TELEPORT_TITLE, MSG_TUT_TELEPORT, Img_TutTitle_TELEPORT, MSG_TUT_TELEPORT_MAP,
		     TutUnits_Eirika, NULL, After_Teleport, EventScr_TutG_Turn),
	MAP_TUTORIAL(MSG_TUT_FREEMOVE_TITLE, MSG_TUT_FREEMOVE, Img_TutTitle_FREEMOVE, MSG_TUT_FREEMOVE_MAP,
		     TutUnits_FreeMove, Setup_FreeMove, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_GHOST_TITLE, MSG_TUT_GHOST, Img_TutTitle_GHOST, MSG_TUT_GHOST_MAP,
		     TutUnits_Eirika, NULL, NULL, EventScr_TutG_Ghost),
	MAP_TUTORIAL(MSG_TUT_TRAPS_TITLE, MSG_TUT_TRAPS, Img_TutTitle_TRAPS, MSG_TUT_TRAPS_MAP,
		     TutUnits_Eirika, Setup_Traps, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_THREAT_TITLE, MSG_TUT_THREAT, Img_TutTitle_THREAT, MSG_TUT_THREAT_MAP,
		     TutUnits_Threat, Setup_Threat, NULL, EventScr_TutG_Threat),

	/* Growth & Progression */
	MAP_TUTORIAL(MSG_TUT_PRESTIGE_TITLE, MSG_TUT_PRESTIGE, Img_TutTitle_PRESTIGE, MSG_TUT_PRESTIGE_MAP,
		     TutUnits_Prestige, NULL, NULL, EventScr_TutG_Prestige),
	MAP_TUTORIAL(MSG_TUT_CAPACITY_TITLE, MSG_TUT_CAPACITY, Img_TutTitle_CAPACITY, MSG_TUT_CAPACITY_MAP,
		     TutUnits_Capacity, Setup_Capacity, NULL, NULL),
	MAP_TUTORIAL(MSG_TUT_TRAINEE_TITLE, MSG_TUT_TRAINEE, Img_TutTitle_TRAINEE, MSG_TUT_TRAINEE_MAP,
		     TutUnits_Trainee, Setup_Trainee, NULL, EventScr_TutG_Trainee),

	/* Towns & Minigames */
	TOWN_TUTORIAL(MSG_TUT_DISTRICTS_TITLE, MSG_TUT_DISTRICTS, Img_TutTitle_DISTRICTS, MSG_TUT_DISTRICTS_MAP,
		      NULL),
	/* Every district game in one: Port Kiris has them all (fishing, Hex, River Crossing, Snek, Tower Stack, the board) */
	TOWN_TUTORIAL(MSG_TUT_MINIGAMES_TITLE, MSG_TUT_MINIGAMES, Img_TutTitle_MINIGAMES, MSG_TUT_MINIGAMES_MAP, NULL),

	{ 0 },
};
