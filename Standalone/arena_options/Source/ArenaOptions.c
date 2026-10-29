#include "global.h"
#include "bmunit.h"
#include "bmitem.h"
#include "bmarena.h"
#include "bmmap.h"
#include "bmlib.h"
#include "bmio.h"
#include "bmshop.h"
#include "bmudisp.h"
#include "bm.h"
#include "eventinfo.h"
#include "uimenu.h"
#include "uiutils.h"
#include "fontgrp.h"
#include "hardware.h"
#include "proc.h"
#include "scene.h"
#include "face.h"
#include "icon.h"
#include "popup.h"
#include "prepscreen.h"
#include "sysutil.h"
#include "soundwrapper.h"
#include "functions.h"
#include "variables.h"
#include "constants/items.h"
#include "constants/classes.h"
#include "constants/terrains.h"
#include "constants/characters.h"
#include "gba/syscall.h"
#include "ctc.h"

/*
 * Arena options (standalone port of the C Skill System arena module flags
 * arena_let_player_use_upgraded_weapons, arena_calculate_weapon_based_on_level,
 * arena_limits and arena_roster_menu). Each part is only reached through the
 * hooks Installer.event installs for it; see README.md.
 */

/* ------------------------------------------------------------------------- */
/* Data from Installer.event                                                 */
/* ------------------------------------------------------------------------- */

struct ArenaOptionsConfig {
	u8 weaponUpgrade;       /* 1 = one tier up, 2 = tier from level */
	u8 rosterShowOpponent;  /* draw the opponent window before the wager */
	u8 rosterFirstFlag;     /* first chapter flag used to mark beaten entries */
	u8 limitPromotedBonus;  /* levels added for promoted units by the level limit */
};

struct ArenaLimitEntry {
	u8 chapterId;  /* 0xFF ends the table */
	u8 levelLimit; /* 0 = none */
	u8 turnLimit;  /* 0 = none */
};

enum {
	ARENA_ROSTER_REWARD_DOUBLE_PAYOUT,
	ARENA_ROSTER_REWARD_TRIPLE_PAYOUT,
	ARENA_ROSTER_REWARD_ITEM,
};

struct ArenaRosterEntry {
	u8 pid;        /* 0 = generic arena opponent (class name shown) */
	u8 jid;
	u8 level;
	u8 rewardType;
	u16 item;      /* opponent weapon */
	u16 price;     /* wager */
	u16 reward;    /* item for ARENA_ROSTER_REWARD_ITEM */
};

struct ArenaRosterChapter {
	u8 chapterId;  /* 0xFF ends the table */
	u8 maxWins;    /* "cleaned out" after this many wins (0 = no limit) */
	u8 entryCount;
	u8 _pad;
	const struct ArenaRosterEntry *entries;
};

extern const struct ArenaOptionsConfig gArenaOptionsConfig;
extern const struct ArenaLimitEntry gArenaLimitTable[];
extern const struct ArenaRosterChapter gArenaRosterTable[];

/* Vanilla symbols not declared by FE-CLib */
extern struct ProcCmd gProcScr_ArenaUiResults[];
extern int sActiveMsg;
extern char sMsgString[];
extern struct Font *gActiveFont;
void WaitForFade(ProcPtr proc);
void StartMidFadeFromBlack(void);
void BMapDispResume(void);

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

/* Text control codes */
#define TC_NL "\x01"
#define TC_A  "\x03"

/* Same as vanilla StartArenaDialogue (0x080B5C04), with a string instead of a text ID */
static void StartArenaDialogueStr(const char *str, ProcPtr proc)
{
	SetInitTalkTextFont();
	ClearTalkText();

	StartTalkExt(8, 2, str, proc);
	SetTalkPrintColor(0);

	SetTalkFlag(TALK_FLAG_INSTANTSHIFT);
	SetTalkFlag(TALK_FLAG_NOBUBBLE);
	SetTalkFlag(TALK_FLAG_NOSKIP);

	SetActiveTalkFace(1);
}

static void StrCopy(char *dst, const char *src)
{
	while ((*dst++ = *src++) != 0)
		;
}

static char *StrPutNumber(char *dst, int number)
{
	char tmp[8];
	int len = 0;

	do {
		tmp[len++] = '0' + DivRem(number, 10);
		number = Div(number, 10);
	} while (number > 0 && len < 8);

	while (len > 0)
		*dst++ = tmp[--len];

	*dst = 0;
	return dst;
}

/* ------------------------------------------------------------------------- */
/* Upgraded player weapon (arena_let_player_use_upgraded_weapons and        */
/* arena_calculate_weapon_based_on_level)                                    */
/* ------------------------------------------------------------------------- */

static int ArenaGetWeaponTier(struct Unit *unit)
{
	int level = unit->level;

	if (gArenaOptionsConfig.weaponUpgrade < 2)
		return 1;

	if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
		level += 20;

	if (level <= 10)
		return 0;

	if (level <= 20)
		return 1;

	return 2;
}

/* Replaces vanilla ArenaGenerateBaseWeapons (0x08031B70) */
void ArenaGenerateBaseWeapons_Options(void)
{
	static const u8 arenaWeapons[] = {
		[ITYPE_SWORD] = ITEM_SWORD_IRON,
		[ITYPE_LANCE] = ITEM_LANCE_IRON,
		[ITYPE_AXE]   = ITEM_AXE_IRON,
		[ITYPE_BOW]   = ITEM_BOW_IRON,
		[ITYPE_STAFF] = ITEM_NONE,
		[ITYPE_ANIMA] = ITEM_ANIMA_FIRE,
		[ITYPE_LIGHT] = ITEM_LIGHT_LIGHTNING,
		[ITYPE_DARK]  = ITEM_DARK_FLUX,
	};

	u16 weapon = MakeNewItem(arenaWeapons[gArenaState.playerWpnType]);
	int tier = ArenaGetWeaponTier(gArenaState.playerUnit);

	/* Highest tier up to the target that the player can wield */
	while (tier-- > 0) {
		u16 next = ArenaGetUpgradedWeapon(weapon);

		if (GetItemIndex(next) == GetItemIndex(weapon))
			break;

		if (!CanUnitUseWeapon(gArenaState.playerUnit, next))
			break;

		weapon = next;
	}

	gArenaState.playerWeapon = weapon;
	gArenaState.opponentWeapon = MakeNewItem(arenaWeapons[gArenaState.opponentWpnType]);

	gArenaState.range = 1;

	if (gArenaState.playerWpnType == ITYPE_BOW)
		gArenaState.range = 2;

	if (gArenaState.opponentWpnType == ITYPE_BOW)
		gArenaState.range = 2;
}

/* ------------------------------------------------------------------------- */
/* Arena limits (arena_limits)                                               */
/* ------------------------------------------------------------------------- */

enum {
	ARENA_LIMIT_NONE,
	ARENA_LIMIT_CLOSED,
	ARENA_LIMIT_LEVEL,
	ARENA_LIMIT_UNPROMOTED_ONLY,
};

static const struct ArenaLimitEntry *GetArenaLimitEntry(void)
{
	const struct ArenaLimitEntry *it;

	for (it = gArenaLimitTable; it->chapterId != 0xFF; it++)
		if (it->chapterId == gPlaySt.chapterIndex)
			return it;

	return NULL;
}

static int GetArenaLimitReason(struct Unit *unit)
{
	const struct ArenaLimitEntry *entry = GetArenaLimitEntry();
	int level;

	if (!entry)
		return ARENA_LIMIT_NONE;

	if (entry->turnLimit && gPlaySt.chapterTurnNumber > entry->turnLimit)
		return ARENA_LIMIT_CLOSED;

	if (entry->levelLimit) {
		level = unit->level;

		if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
			level += gArenaOptionsConfig.limitPromotedBonus;

		if (level > entry->levelLimit) {
			if ((UNIT_CATTRIBUTES(unit) & CA_PROMOTED) && entry->levelLimit <= gArenaOptionsConfig.limitPromotedBonus)
				return ARENA_LIMIT_UNPROMOTED_ONLY;

			return ARENA_LIMIT_LEVEL;
		}
	}

	return ARENA_LIMIT_NONE;
}

/* Replaces vanilla ArenaCommandUsability (0x080240EC) */
u8 ArenaCommandUsability_Limits(const struct MenuItemDef *def, int number)
{
	(void)def;
	(void)number;

	if (gActiveUnit->pClassData->number == CLASS_PHANTOM)
		return MENU_NOTSHOWN;

	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (gBmMapTerrain[gActiveUnit->yPos][gActiveUnit->xPos] != TERRAIN_ARENA_08)
		return MENU_NOTSHOWN;

	if (!ArenaIsUnitAllowed(gActiveUnit))
		return MENU_DISABLED;

	if (GetArenaLimitReason(gActiveUnit) != ARENA_LIMIT_NONE)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

/*
 * Help box with a built-in string. The help box takes a text ID, and
 * GetStringFromIndex returns its decode buffer as-is when asked for the
 * last decoded ID: put our string there under a real ID (if anything decodes
 * another text in between, that vanilla text shows instead).
 */
#define ARENA_LIMIT_FALLBACK_MSG 0x854 /* "You can't fight, so you can't enter the arena." */

static u8 ArenaLimitHelpBox(struct MenuProc *menu, int reason)
{
	const struct ArenaLimitEntry *entry = GetArenaLimitEntry();
	char *str = sMsgString;

	switch (reason) {
	case ARENA_LIMIT_CLOSED:
		StrCopy(str, "The arena is closed for the day.");
		break;

	case ARENA_LIMIT_UNPROMOTED_ONLY:
		StrCopy(str, "Unpromoted units only.");
		break;

	default:
		StrCopy(str, "Arena level limit: ");
		str = StrPutNumber(str + 19, entry ? entry->levelLimit : 0);
		StrCopy(str, ".");
		break;
	}

	sActiveMsg = ARENA_LIMIT_FALLBACK_MSG;
	return MenuFrozenHelpBox(menu, ARENA_LIMIT_FALLBACK_MSG);
}

/* Replaces vanilla ArenaCommandEffect (0x08024144) */
u8 ArenaCommandEffect_Limits(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	if (menuItem->availability == MENU_DISABLED) {
		int reason = GetArenaLimitReason(gActiveUnit);

		if (!ArenaIsUnitAllowed(gActiveUnit) || reason == ARENA_LIMIT_NONE) {
			if (IsUnitMagicSealed(gActiveUnit))
				MenuFrozenHelpBox(menu, 0x853); /* "You can't enter the arena while Silenced." */
			else
				MenuFrozenHelpBox(menu, 0x854); /* "You can't fight, so you can't enter the arena." */
		} else {
			ArenaLimitHelpBox(menu, reason);
		}

		return MENU_ACT_SND6B;
	}

	StartArenaScreen();

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/* ------------------------------------------------------------------------- */
/* Arena roster menu (arena_roster_menu)                                     */
/* ------------------------------------------------------------------------- */

#define ARENA_ROSTER_VISIBLE_ROWS 5
#define ARENA_ROSTER_MAX_FLAG 40 /* FE8U stores chapter flags 1-40 */
#define ARENA_ROSTER_CURSOR_X 12
#define ARENA_ROSTER_CURSOR_Y 72
#define ARENA_ROSTER_ROW_HEIGHT 16
#define ARENA_ROSTER_LIST_X 1
#define ARENA_ROSTER_LIST_Y 8
#define ARENA_ROSTER_LIST_W 28
#define ARENA_ROSTER_LIST_H 12
#define ARENA_ROSTER_SCROLLBAR_X 228
#define ARENA_ROSTER_SCROLLBAR_Y 80
#define ARENA_ROSTER_TEXT_NAME_WIDTH 10
#define ARENA_ROSTER_TEXT_BUFFER_WIDTH 13
#define ARENA_ROSTER_TEXT_REWARD_WIDTH 8
#define ARENA_ROSTER_REWARD_ICON_X 19
#define ARENA_ROSTER_REWARD_TEXT_X 21

enum {
	ARENA_ROSTER_CHOICE_NONE,
	ARENA_ROSTER_CHOICE_CANCELLED,
	ARENA_ROSTER_CHOICE_SELECTED,
};

/* The arena UI proc (our copy of gProcScr_ArenaUiMain) keeps the selection */
struct ProcArenaRosterMain {
	PROC_HEADER;
	/* 29 */ u8 _pad[0x64 - 0x29];
	/* 64 */ u8 choiceState;
	/* 65 */ s8 selectedIndex;
	/* 66 */ u16 pendingRewardItem;
};

struct ProcArenaRosterSelect {
	PROC_HEADER;
	/* 2C */ int cursor;
	/* 30 */ int scrollTop;
	/* 34 */ u8 promptShown;
	/* 36 */ u16 savedChr;
};

static const struct ProcCmd ProcScr_ArenaRosterMain[];
static const struct ProcCmd ProcScr_ArenaRosterResults[];

extern struct Text gBanimText[20];

static const char sArenaRosterWelcome[] =
	"Welcome to the arena!" TC_A TC_NL
	"Pick your opponent." TC_A;

static const char sArenaRosterCleanedOut[] =
	"You again? You've already" TC_NL
	"cleaned me out. Clear off!" TC_A;

static const char sArenaRosterItemPrize[] =
	"So you won, eh? Here's" TC_NL
	"your prize." TC_A;

static struct ProcArenaRosterMain *ArenaRosterGetMain(void)
{
	return Proc_Find(ProcScr_ArenaRosterMain);
}

static const struct ArenaRosterChapter *ArenaRosterGetChapter(int chapterId)
{
	const struct ArenaRosterChapter *it;

	for (it = gArenaRosterTable; it->chapterId != 0xFF; it++)
		if (it->chapterId == chapterId)
			return it;

	return NULL;
}

static int ArenaRosterGetEntryCount(const struct ArenaRosterChapter *chapter)
{
	int count;
	int maxCount = ARENA_ROSTER_MAX_FLAG + 1 - gArenaOptionsConfig.rosterFirstFlag;

	if (!chapter || !chapter->entries)
		return 0;

	count = chapter->entryCount;

	if (count > maxCount)
		count = maxCount;

	return count < 0 ? 0 : count;
}

/* Beaten entries are chapter (temporary) flags: cleared on chapter start, kept by suspend */
static int ArenaRosterGetEntryFlag(int index)
{
	return gArenaOptionsConfig.rosterFirstFlag + index;
}

static bool ArenaRosterEntryCleared(int index)
{
	return CheckFlag(ArenaRosterGetEntryFlag(index));
}

static void ArenaRosterSetEntryCleared(int index)
{
	SetFlag(ArenaRosterGetEntryFlag(index));
}

static int ArenaRosterCountWins(const struct ArenaRosterChapter *chapter)
{
	int i;
	int wins = 0;

	for (i = 0; i < ArenaRosterGetEntryCount(chapter); i++)
		if (ArenaRosterEntryCleared(i))
			wins++;

	return wins;
}

static bool ArenaRosterHasReachedWinLimit(const struct ArenaRosterChapter *chapter)
{
	if (!chapter || chapter->maxWins == 0)
		return false;

	return ArenaRosterCountWins(chapter) >= chapter->maxWins;
}

static bool ArenaRosterEntryValid(const struct ArenaRosterEntry *entry)
{
	return entry && entry->jid && entry->item;
}

static void ArenaRosterClearSelection(void)
{
	struct ProcArenaRosterMain *main = ArenaRosterGetMain();

	if (!main)
		return;

	main->choiceState = ARENA_ROSTER_CHOICE_NONE;
	main->selectedIndex = -1;
}

static const struct ArenaRosterEntry *ArenaRosterGetSelectedEntry(void)
{
	struct ProcArenaRosterMain *main = ArenaRosterGetMain();
	const struct ArenaRosterChapter *chapter;

	if (!main || main->choiceState != ARENA_ROSTER_CHOICE_SELECTED)
		return NULL;

	chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
	if (!chapter)
		return NULL;

	if (main->selectedIndex < 0 || main->selectedIndex >= ArenaRosterGetEntryCount(chapter))
		return NULL;

	return &chapter->entries[main->selectedIndex];
}

static const char *ArenaRosterGetDisplayName(const struct ArenaRosterEntry *entry)
{
	if (entry->pid && GetCharacterData(entry->pid)->nameTextId)
		return GetStringFromIndex(GetCharacterData(entry->pid)->nameTextId);

	return GetStringFromIndex(GetClassData(entry->jid)->nameTextId);
}

static unsigned ArenaRosterGetRewardGold(const struct ArenaRosterEntry *entry)
{
	if (entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT)
		return entry->price * 3;

	if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM)
		return 0;

	return entry->price * 2;
}

static bool ArenaRosterIsDialogueOpen(ProcPtr proc)
{
	(void)proc;
	return Proc_Find(gProcScr_TalkOpen) != NULL;
}

static void ArenaRosterClearUi(void)
{
	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
	TileMap_FillRect(TILEMAP_LOCATED(gBG1TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void ArenaRosterApplyEnemySpritePalettes(void)
{
	int faction = gPlaySt.faction;

	gPlaySt.faction = FACTION_RED;
	ApplyUnitSpritePalettes();
	gPlaySt.faction = faction;
}

static void ArenaRosterDrawSpritesOnly(struct ProcArenaRosterSelect *proc, const struct ArenaRosterChapter *chapter)
{
	int row;
	int entryCount = ArenaRosterGetEntryCount(chapter);

	ClearSprites();

	for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
		int index = proc->scrollTop + row;
		const struct ArenaRosterEntry *entry;

		if (index >= entryCount)
			continue;

		entry = &chapter->entries[index];

		if (!ArenaRosterEntryValid(entry))
			continue;

		PutUnitSpriteForClassId(0, 36, ARENA_ROSTER_CURSOR_Y + row * ARENA_ROSTER_ROW_HEIGHT, 0xD800, entry->jid);
	}

	RefreshUnitSprites();
	SyncUnitSpriteSheet();

	if ((proc->cursor >= proc->scrollTop) && (proc->cursor < proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS))
		ShowSysHandCursor(ARENA_ROSTER_CURSOR_X, ARENA_ROSTER_CURSOR_Y + (proc->cursor - proc->scrollTop) * ARENA_ROSTER_ROW_HEIGHT, 0x8, 0x800);
}

static void ArenaRosterDrawList(struct ProcArenaRosterSelect *proc)
{
	const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
	int entryCount;
	int row;

	if (!chapter)
		return;

	entryCount = ArenaRosterGetEntryCount(chapter);

	TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y), ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);
	DrawUiFrame2(ARENA_ROSTER_LIST_X, ARENA_ROSTER_LIST_Y, ARENA_ROSTER_LIST_W, ARENA_ROSTER_LIST_H, 0);

	for (row = 0; row < ARENA_ROSTER_VISIBLE_ROWS; ++row) {
		int index = proc->scrollTop + row;
		struct Text *nameText = &gBanimText[row];
		struct Text *rewardText = &gBanimText[row + ARENA_ROSTER_VISIBLE_ROWS];
		const struct ArenaRosterEntry *entry;
		int color;

		ClearText(nameText);
		ClearText(rewardText);

		if (index >= entryCount)
			continue;

		entry = &chapter->entries[index];

		if (!ArenaRosterEntryValid(entry))
			continue;

		color = ArenaRosterEntryCleared(index) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;

		PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 9 + row * 2), color, index + 1);
		PutDrawText(nameText, TILEMAP_LOCATED(gBG0TilemapBuffer, 7, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_NAME_WIDTH, ArenaRosterGetDisplayName(entry));
		PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 17, 9 + row * 2), color, entry->price);

		if ((entry->rewardType == ARENA_ROSTER_REWARD_ITEM) && (entry->reward != ITEM_NONE)) {
			int icon = GetItemIconId(entry->reward);

			if (icon >= 0)
				DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_ICON_X, 9 + row * 2), icon, 0x4000);

			PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_REWARD_WIDTH, GetItemName(entry->reward));
		} else {
			PutDrawText(rewardText, TILEMAP_LOCATED(gBG0TilemapBuffer, ARENA_ROSTER_REWARD_TEXT_X - 2, 9 + row * 2), color, 0, ARENA_ROSTER_TEXT_BUFFER_WIDTH,
				entry->rewardType == ARENA_ROSTER_REWARD_TRIPLE_PAYOUT ? "Triple Payout" : "Double Payout");
		}
	}

	UpdateMenuScrollBarConfig(8, proc->scrollTop * ARENA_ROSTER_ROW_HEIGHT, entryCount < 1 ? 1 : entryCount, ARENA_ROSTER_VISIBLE_ROWS);
	ArenaRosterDrawSpritesOnly(proc, chapter);
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* Roster opponent: fixed character/class/level/weapon instead of the random one */
static void ArenaRosterGenerateOpponentUnit(const struct ArenaRosterEntry *entry)
{
	struct Unit *unit = &gArenaOpponent;
	struct UnitDefinition udef;
	int type = GetItemType(entry->item);

	CpuFill16(0, &udef, sizeof(udef));
	udef.charIndex = entry->pid ? entry->pid : CHARACTER_ARENA_OPPONENT;
	udef.classIndex = entry->jid;
	udef.level = entry->level;
	udef.autolevel = 1;
	udef.items[0] = entry->item;

	unit->index = 0x80;
	UnitInitFromDefinition(unit, &udef);
	UnitLoadStatsFromChracter(unit, unit->pCharacterData);
	unit->level = entry->level;
	UnitAutolevel(unit);

	if (type >= 0 && type < 8)
		unit->ranks[type] = 0xFB; /* S rank: the roster weapon is always usable */

	UnitCheckStatCaps(unit);
	SetUnitHp(unit, GetUnitMaxHp(unit));
}

static void ArenaRosterApplySelection(const struct ArenaRosterEntry *entry)
{
	gArenaState.opponentUnit = &gArenaOpponent;
	gArenaState.opponentClassId = entry->jid;
	gArenaState.opponentWpnType = GetItemType(entry->item);
	gArenaState.opponentIsMagic = IsWeaponMagic(gArenaState.opponentWpnType);
	gArenaState.opponentLevel = entry->level;
	gArenaState.matchupGoldValue = entry->price;

	ArenaRosterGenerateOpponentUnit(entry);
	ArenaGenerateBaseWeapons(); /* the player's weapon (goes through the upgrade hook if installed) */
	gArenaState.opponentWeapon = MakeNewItem(entry->item);

	if (gArenaState.playerWpnType == ITYPE_BOW || gArenaState.opponentWpnType == ITYPE_BOW)
		gArenaState.range = 2;
	else
		gArenaState.range = 1;

	gArenaState.playerPowerWeight = ArenaGetPowerRanking(gArenaState.playerUnit, gArenaState.opponentIsMagic);
	gArenaState.opponentPowerWeight = ArenaGetPowerRanking(gArenaState.opponentUnit, gArenaState.playerIsMagic);
	ArenaSetResult(0);
	gArenaState.unk0B = 1;
	ArenaSetFallbackWeaponsMaybe();
}

static void ArenaRosterCancelSelectionAndExit(struct ProcArenaRosterSelect *proc)
{
	struct ProcArenaRosterMain *main = proc->proc_parent;

	main->choiceState = ARENA_ROSTER_CHOICE_CANCELLED;
	main->selectedIndex = -1;
	Proc_Goto(main, 2);
	Proc_Break(proc);
}

static void ArenaRosterSelector_Init(struct ProcArenaRosterSelect *proc)
{
	int i;

	proc->cursor = 0;
	proc->scrollTop = 0;
	proc->promptShown = 0;

	SetTextFont(NULL);
	InitSystemTextFont();
	proc->savedChr = gActiveFont->chr_counter;
	StartUiCursorHand(proc);
	ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x600, 1);
	ArenaRosterApplyEnemySpritePalettes();
	LoadIconPalettes(4);
	StartMenuScrollBar(proc);
	PutMenuScrollBarAt(ARENA_ROSTER_SCROLLBAR_X, ARENA_ROSTER_SCROLLBAR_Y);
	InitMenuScrollBarImg(0x7A60, 2);

	for (i = 0; i < (ARENA_ROSTER_VISIBLE_ROWS * 2) + 2; ++i)
		InitText(&gBanimText[i], ARENA_ROSTER_TEXT_BUFFER_WIDTH);

	ArenaRosterDrawList(proc);
}

/*
 * Give back the text tiles the list used. InitSystemTextFont does not rewind
 * the allocator, so later opponent windows would otherwise run into the
 * dialogue's tiles (0x200+).
 */
static void ArenaRosterReleaseText(struct ProcArenaRosterSelect *proc)
{
	struct Font *active = gActiveFont; /* the dialogue may still be printing with its font */

	SetTextFont(NULL);
	ResetTextFont();
	gActiveFont->chr_counter = proc->savedChr;
	SetTextFont(active);
}

static void ArenaRosterSelector_End(struct ProcArenaRosterSelect *proc)
{
	ArenaRosterReleaseText(proc);
	EndMenuScrollBar();
	HideSysHandCursor();
}

static void ArenaRosterSelector_Loop(struct ProcArenaRosterSelect *proc)
{
	const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);
	struct ProcArenaRosterMain *main = proc->proc_parent;
	int entryCount;
	bool redraw = false;

	if (!chapter) {
		ArenaRosterCancelSelectionAndExit(proc);
		return;
	}

	entryCount = ArenaRosterGetEntryCount(chapter);

	if (!proc->promptShown) {
		proc->promptShown = 1;
		return;
	}

	if (ArenaRosterIsDialogueOpen(proc))
		return;

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		PlaySoundEffect(0x6B);
		ArenaRosterCancelSelectionAndExit(proc);
		return;
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP) {
		if (proc->cursor > 0) {
			proc->cursor--;

			if (proc->cursor < proc->scrollTop)
				proc->scrollTop = proc->cursor;

			redraw = true;
			PlaySoundEffect(0x66);
		}
	}

	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		if (proc->cursor < entryCount - 1) {
			proc->cursor++;

			if (proc->cursor >= proc->scrollTop + ARENA_ROSTER_VISIBLE_ROWS)
				proc->scrollTop = proc->cursor - ARENA_ROSTER_VISIBLE_ROWS + 1;

			redraw = true;
			PlaySoundEffect(0x66);
		}
	}

	if ((gKeyStatusPtr->newKeys & A_BUTTON) && entryCount > 0) {
		const struct ArenaRosterEntry *entry = &chapter->entries[proc->cursor];

		if (!ArenaRosterEntryValid(entry) || ArenaRosterEntryCleared(proc->cursor)) {
			PlaySoundEffect(0x6C);
			return;
		}

		main->choiceState = ARENA_ROSTER_CHOICE_SELECTED;
		main->selectedIndex = proc->cursor;
		ArenaRosterApplySelection(entry);

		if (gArenaOptionsConfig.rosterShowOpponent) {
			/* Vanilla symbol: Standalone/arena_show_opponent_in_advance draws its window if installed */
			ArenaRosterClearUi();
			ClearSprites();
			HideSysHandCursor();
			DrawArenaOpponentDetailsText(main);
		}

		SetTalkNumber(ArenaGetMatchupGoldValue());
		StartArenaDialogue(0x8D2, main); /* "Would you like to wager [G] gold?[Yes]" */
		PlaySoundEffect(0x6A);
		Proc_Goto(proc, 1);
		return;
	}

	if (redraw)
		ArenaRosterDrawList(proc);
	else
		ArenaRosterDrawSpritesOnly(proc, chapter);
}

static void ArenaRosterSelector_WaitForWagerChoice(struct ProcArenaRosterSelect *proc)
{
	if (!gArenaOptionsConfig.rosterShowOpponent) {
		const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);

		if (chapter)
			ArenaRosterDrawSpritesOnly(proc, chapter);
	}

	if (ArenaRosterIsDialogueOpen(proc))
		return;

	Proc_Break(proc);
}

static const struct ProcCmd ProcScr_ArenaRosterSelect[] = {
	PROC_NAME("ArenaRosterSelect"),
	PROC_CALL(ArenaRosterSelector_Init),
PROC_LABEL(0),
	PROC_REPEAT(ArenaRosterSelector_Loop),
PROC_LABEL(1),
	PROC_REPEAT(ArenaRosterSelector_WaitForWagerChoice),
	PROC_CALL(ArenaRosterSelector_End),
	PROC_END,
};

static bool ArenaRosterMenuEnabled(void)
{
	if (UNIT_FACTION(gActiveUnit) != FACTION_BLUE)
		return false;

	return ArenaRosterGetChapter(gPlaySt.chapterIndex) != NULL;
}

/* Our arena UI steps (replace the vanilla ones in our copy of the proc) */

static void ArenaRoster_WelcomeDialogue(ProcPtr proc)
{
	StartArenaDialogueStr(sArenaRosterWelcome, proc);
}

static void ArenaRoster_StartSelection(struct ProcArenaRosterMain *proc)
{
	const struct ArenaRosterChapter *chapter = ArenaRosterGetChapter(gPlaySt.chapterIndex);

	proc->choiceState = ARENA_ROSTER_CHOICE_NONE;
	proc->selectedIndex = -1;
	proc->pendingRewardItem = ITEM_NONE;

	if (ArenaRosterHasReachedWinLimit(chapter)) {
		StartArenaDialogueStr(sArenaRosterCleanedOut, proc);
		Proc_Goto(proc, 2);
		return;
	}

	Proc_StartBlocking(ProcScr_ArenaRosterSelect, proc);
}

static void ArenaRoster_CheckConfirmation(struct ProcArenaRosterMain *proc)
{
	if (proc->choiceState != ARENA_ROSTER_CHOICE_SELECTED) {
		ArenaRosterClearSelection();
		Proc_Goto(proc, 2);
		return;
	}

	if (GetTalkChoiceResult() != 1) {
		ArenaRosterClearSelection();
		StartArenaDialogue(0x8D4, proc); /* "What's that? Bah! Get outta here!" */
		Proc_Goto(proc, 2);
		return;
	}

	if (ArenaGetMatchupGoldValue() > (int)GetPartyGoldAmount()) {
		ArenaRosterClearSelection();
		StartArenaDialogue(0x8DA, proc); /* "You don't have the money! Try again later." */
		Proc_Goto(proc, 2);
	}
}

static void ArenaRoster_ConfirmWager(ProcPtr proc)
{
	SetPartyGoldAmount(GetPartyGoldAmount() - ArenaGetMatchupGoldValue());
	PlaySoundEffect(0xB9);
	DisplayGoldBoxText(TILEMAP_LOCATED(gBG0TilemapBuffer, 0x1B, 0x6));

	ArenaRosterClearUi();

	if (gArenaOptionsConfig.rosterShowOpponent)
		DrawArenaOpponentDetailsText(proc);
}

static void ArenaRoster_ResultsDialogue(struct ProcArenaRosterMain *proc)
{
	const struct ArenaRosterEntry *entry = ArenaRosterGetSelectedEntry();

	if (!entry) {
		/* Not a roster fight (should not happen): vanilla results */
		ArenaUi_ResultsDialogue(proc);
		return;
	}

	switch (ArenaGetResult()) {
	case 1: /* win */
		ArenaRosterSetEntryCleared(proc->selectedIndex);

		if (entry->rewardType == ARENA_ROSTER_REWARD_ITEM) {
			proc->pendingRewardItem = MakeNewItem(entry->reward);
			StartArenaDialogueStr(sArenaRosterItemPrize, proc);
			return;
		}

		SetTalkNumber(ArenaRosterGetRewardGold(entry));
		StartArenaDialogue(0x8D6, proc); /* "So you won, eh? Here's your prize. [G] gold." */
		SetPartyGoldAmount(GetPartyGoldAmount() + ArenaRosterGetRewardGold(entry));
		break;

	case 2: /* loss */
		StartArenaDialogue(0x8D7, proc);
		break;

	case 3: /* draw: wager back */
		StartArenaDialogue(0x8D9, proc);
		SetPartyGoldAmount(GetPartyGoldAmount() + ArenaGetMatchupGoldValue());
		break;

	case 4: /* yield */
		StartArenaDialogue(0x8D8, proc);
		break;
	}
}

/*
 * The prize item is handed over on the map, after sub_80B5970 copied the
 * battle copy of the unit (and its inventory) back: giving it on the arena
 * screen would be overwritten by that copy. Inventory full -> vanilla choice.
 */
static void ArenaRoster_GiveRewardItem(struct ProcArenaRosterMain *proc)
{
	int item = proc->pendingRewardItem;
	struct Unit *unit = gArenaState.playerUnit;

	if (item == ITEM_NONE)
		return;

	proc->pendingRewardItem = ITEM_NONE;

	if (!unit || !unit->pCharacterData)
		unit = gActiveUnit;

	NewPopup_ItemGot(proc, unit, item);
}

static void ArenaRoster_ClearSelection(ProcPtr proc)
{
	(void)proc;
	ArenaRosterClearSelection();
}

/* Copy of vanilla gProcScr_ArenaUiMain (0x08A394DC) with the roster steps */
static const struct ProcCmd ProcScr_ArenaRosterMain[] = {
	PROC_NAME("ArenaRosterMain"),
	PROC_CALL(LockGame),

	PROC_SLEEP(1),
	PROC_CALL_ARG(_FadeBgmOut, -1),
	PROC_CALL(StartMidFadeToBlack),
	PROC_REPEAT(WaitForFade),

	PROC_CALL(BMapDispSuspend),

	PROC_CALL_ARG(_StartBgm, 56),

	PROC_CALL(ArenaUi_Init),
	PROC_CALL(FadeInBlackSpeed20),
	PROC_SLEEP(1),

	PROC_CALL(ArenaRoster_WelcomeDialogue),
	PROC_SLEEP(1),

	PROC_CALL(ArenaRoster_StartSelection),
	PROC_SLEEP(1),

	PROC_CALL(ArenaRoster_CheckConfirmation),
	PROC_SLEEP(1),

	PROC_CALL(ArenaRoster_ConfirmWager),
	PROC_SLEEP(1),

	PROC_CALL(ArenaUi_InstructionsDialogue),
	PROC_SLEEP(1),

	PROC_CALL(ArenaUi_GoodLuckDialogue),
	PROC_SLEEP(1),

PROC_LABEL(0),
	PROC_CALL_ARG(_FadeBgmOut, 2),
	PROC_CALL(sub_8013F40),
	PROC_SLEEP(1),

	PROC_CALL(ArenaUi_StartArenaBattle),
	PROC_SLEEP(1),

	PROC_CALL(UnlockGame),
	PROC_CALL(BMapDispResume),

	PROC_JUMP(ProcScr_ArenaRosterResults),

PROC_LABEL(2),
	PROC_SLEEP(1),

	PROC_CALL(sub_8013F40),
	PROC_SLEEP(1),

	PROC_CALL(ArenaUi_OnEnd),

	PROC_CALL(ResetDialogueScreen),

	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL(StartMapSongBgm),

	PROC_CALL(StartMidFadeFromBlack),
	PROC_REPEAT(WaitForFade),

	PROC_CALL(UnlockGame),

	PROC_END,
};

/* Copy of vanilla gProcScr_ArenaUiResults (0x08A3963C) with the roster rewards */
static const struct ProcCmd ProcScr_ArenaRosterResults[] = {
PROC_LABEL(1),
	PROC_CALL(sub_80B5B00),

	PROC_CALL(LockGame),
	PROC_CALL(BMapDispSuspend),
	PROC_SLEEP(0),

	PROC_START_CHILD(gProcScr_ArenaUiResultBgm),

	PROC_CALL(ArenaUi_Init),

	PROC_CALL(FadeInBlackSpeed20),
	PROC_SLEEP(0),

	PROC_CALL(ArenaRoster_ResultsDialogue),
	PROC_WHILE(ArenaRosterIsDialogueOpen),
	PROC_SLEEP(0),

	PROC_CALL(ArenaUi_ShowGoldBoxOnVictoryOrDraw),
	PROC_SLEEP(0),

PROC_LABEL(2),
	PROC_SLEEP(1),

	PROC_END_EACH(gProcScr_ArenaUiResultBgm),
	PROC_SLEEP(0),

	PROC_CALL_ARG(_FadeBgmOut, 2),
	PROC_CALL(sub_8013F40),
	PROC_SLEEP(0),

	PROC_CALL(sub_80B5970),

	PROC_CALL(ArenaUi_OnEnd),

	PROC_CALL(ResetDialogueScreen),

	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL(StartMapSongBgm),

	PROC_CALL(StartMidFadeFromBlack),
	PROC_REPEAT(WaitForFade),

	PROC_CALL(ArenaRoster_GiveRewardItem),
	PROC_YIELD,
	PROC_CALL(ArenaRoster_ClearSelection),

	PROC_CALL(UnlockGame),

	PROC_END,
};

/* Replaces vanilla StartArenaScreen (0x080B576C) */
void StartArenaScreen_Roster(void)
{
	ArenaBegin(gActiveUnit);

	if (ArenaRosterMenuEnabled())
		Proc_Start(ProcScr_ArenaRosterMain, PROC_TREE_3);
	else
		Proc_Start(gProcScr_ArenaUiMain, PROC_TREE_3);
}
