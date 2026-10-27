#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
	#define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Takeover) && (COMMON_SKILL_VALID(SID_Takeover))

/* Keypad keys besides the digits. */
enum {
	TAKEOVER_KEY_DEL = 10,
	TAKEOVER_KEY_OK = 11,
	TAKEOVER_MAX_DIGITS = 3,

	/* Popup frame and text layout, in tiles. */
	TAKEOVER_PAD_X = 7,
	TAKEOVER_PAD_Y = 4,
	TAKEOVER_PAD_W = 17,
	TAKEOVER_PAD_H = 10,
	TAKEOVER_TEXT_X = 8,
	TAKEOVER_TEXT_W = 15,
};

/*
 * 1 2 3   0
 * 4 5 6   DEL
 * 7 8 9   OK
 */
static const u8 sTakeoverKeys[3][4] = {
	{ 1, 2, 3, 0 },
	{ 4, 5, 6, TAKEOVER_KEY_DEL },
	{ 7, 8, 9, TAKEOVER_KEY_OK },
};

/* Pixel offset of each keypad column inside a key row's text. */
static const u8 sTakeoverKeyColX[4] = { 12, 36, 60, 96 };

struct TakeoverProc {
	PROC_HEADER;
	/* 2C */ int amount; /* damage typed on the keypad, 0 if cancelled */
	/* 30 */ u8 row, col, len;
	/* 33 */ char digits[TAKEOVER_MAX_DIGITS + 1];
	/* 38 */ struct Text field;
	/* 40 */ struct Text keys[3];
};

_Static_assert(sizeof(struct TakeoverProc) <= 0x6C, "TakeoverProc too big");

/* Gold per point of damage against this target. */
static int GetTakeoverCost(struct Unit *target)
{
	if (UNIT_CATTRIBUTES(target) & CA_BOSS)
		return SKILL_EFF1(SID_Takeover) * 100;

	return SKILL_EFF0(SID_Takeover) * 100;
}

u8 Takeover_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentEnemies))
		return MENU_NOTSHOWN;

	if ((int)GetPartyGoldAmount() < SKILL_EFF0(SID_Takeover) * 100)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

static u8 Takeover_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.targetIndex = target->uid;

	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	gActionData.unk08 = SID_Takeover;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Takeover_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED)
	{
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	MakeTargetListForAdjacentEnemies(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Takeover_OnSelectTarget),
		GetStringFromIndex(MSG_SKILL_Common_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1), -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void ShowActiveUnit(ProcPtr proc)
{
	ShowUnitSprite(gActiveUnit);
}

static void Takeover_KillCheck(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (target->curHP <= 0)
		UnitKill(target);
}

static void callback_exec(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	EndAllMus();
	ShowUnitSprite(gActiveUnit);

	CallMapAnim_HurtExt(proc, target, gActionData.unk0A, ShowActiveUnit, Takeover_KillCheck);
}

/* Writes n (>= 0) in decimal into buf. */
static void TakeoverNumberToString(int n, char *buf)
{
	char tmp[12];
	int i = 0;

	do {
		tmp[i++] = '0' + DivRem(n, 10);
		n = Div(n, 10);
	} while (n > 0);

	while (i > 0)
		*buf++ = tmp[--i];

	*buf = '\0';
}

static int TakeoverTypedAmount(struct TakeoverProc *proc)
{
	int i, amount = 0;

	for (i = 0; i < proc->len; i++)
		amount = amount * 10 + (proc->digits[i] - '0');

	return amount;
}

/* Entry line: the typed damage, and on the right what it costs. */
static void TakeoverPad_DrawField(struct TakeoverProc *proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);
	char cost[12];
	int len;

	ClearText(&proc->field);
	Text_InsertDrawString(&proc->field, 8, TEXT_COLOR_SYSTEM_BLUE, proc->len ? proc->digits : "-");

	TakeoverNumberToString(TakeoverTypedAmount(proc) * GetTakeoverCost(target), cost);
	len = 0;
	while (cost[len] != '\0')
		len++;
	cost[len] = 'G';
	cost[len + 1] = '\0';
	Text_InsertDrawString(&proc->field, 64, TEXT_COLOR_SYSTEM_GOLD, cost);

	PutText(&proc->field, gBG0TilemapBuffer + TILEMAP_INDEX(TAKEOVER_TEXT_X, TAKEOVER_PAD_Y + 1));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void TakeoverPad_Init(struct TakeoverProc *proc)
{
	int row, col;

	proc->amount = 0;
	proc->row = 0;
	proc->col = 0;
	proc->len = 0;
	proc->digits[0] = '\0';

	ResetTextFont();
	SetTextFontGlyphs(0);
	ResetText();
	ClearBg0Bg1();

	DrawUiFrame(BG_GetMapBuffer(1), TAKEOVER_PAD_X, TAKEOVER_PAD_Y, TAKEOVER_PAD_W, TAKEOVER_PAD_H, TILEREF(0, 0), 0);

	InitText(&proc->field, TAKEOVER_TEXT_W);

	for (row = 0; row < 3; row++) {
		InitText(&proc->keys[row], TAKEOVER_TEXT_W);

		for (col = 0; col < 4; col++) {
			int key = sTakeoverKeys[row][col];
			char str[2] = { '0' + key, '\0' };

			if (key == TAKEOVER_KEY_DEL)
				Text_InsertDrawString(&proc->keys[row], sTakeoverKeyColX[col], TEXT_COLOR_SYSTEM_WHITE, "DEL");
			else if (key == TAKEOVER_KEY_OK)
				Text_InsertDrawString(&proc->keys[row], sTakeoverKeyColX[col], TEXT_COLOR_SYSTEM_WHITE, "OK");
			else
				Text_InsertDrawString(&proc->keys[row], sTakeoverKeyColX[col], TEXT_COLOR_SYSTEM_WHITE, str);
		}

		PutText(&proc->keys[row], gBG0TilemapBuffer + TILEMAP_INDEX(TAKEOVER_TEXT_X, TAKEOVER_PAD_Y + 3 + row * 2));
	}

	TakeoverPad_DrawField(proc);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void TakeoverPad_Press(struct TakeoverProc *proc, int key)
{
	if (key == TAKEOVER_KEY_OK) {
		PlaySoundEffect(0x6A);
		proc->amount = TakeoverTypedAmount(proc);
		Proc_Break(proc);
		return;
	}

	if (key == TAKEOVER_KEY_DEL) {
		if (proc->len == 0) {
			PlaySoundEffect(0x6C);
			return;
		}
		PlaySoundEffect(0x6B);
		proc->digits[--proc->len] = '\0';
		TakeoverPad_DrawField(proc);
		return;
	}

	if (proc->len >= TAKEOVER_MAX_DIGITS) {
		PlaySoundEffect(0x6C);
		return;
	}

	PlaySoundEffect(0x6A);
	proc->digits[proc->len++] = '0' + key;
	proc->digits[proc->len] = '\0';
	TakeoverPad_DrawField(proc);
}

static void TakeoverPad_Loop(struct TakeoverProc *proc)
{
	u16 repeated = gKeyStatusPtr->repeatedKeys;
	u16 pressed = gKeyStatusPtr->newKeys;

	if (repeated & DPAD_UP) {
		proc->row = proc->row == 0 ? 2 : proc->row - 1;
		PlaySoundEffect(0x66);
	}
	if (repeated & DPAD_DOWN) {
		proc->row = proc->row == 2 ? 0 : proc->row + 1;
		PlaySoundEffect(0x66);
	}
	if (repeated & DPAD_LEFT) {
		proc->col = proc->col == 0 ? 3 : proc->col - 1;
		PlaySoundEffect(0x66);
	}
	if (repeated & DPAD_RIGHT) {
		proc->col = proc->col == 3 ? 0 : proc->col + 1;
		PlaySoundEffect(0x66);
	}

	if (pressed & START_BUTTON) {
		proc->row = 2;
		proc->col = 3;
	}

	if (pressed & A_BUTTON) {
		TakeoverPad_Press(proc, sTakeoverKeys[proc->row][proc->col]);
		if (proc->proc_scrCur == NULL)
			return;
	} else if (pressed & B_BUTTON) {
		/* B deletes the last digit, or cancels when nothing is typed. */
		if (proc->len == 0) {
			PlaySoundEffect(0x6B);
			proc->amount = 0;
			Proc_Break(proc);
			return;
		}
		TakeoverPad_Press(proc, TAKEOVER_KEY_DEL);
	}

	DisplayUiHand(TAKEOVER_TEXT_X * 8 + sTakeoverKeyColX[proc->col] - 12, (TAKEOVER_PAD_Y + 3 + proc->row * 2) * 8);
}

static void TakeoverPad_End(struct TakeoverProc *proc)
{
	ClearBg0Bg1();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Takeover_Apply(struct TakeoverProc *proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);
	int cost = GetTakeoverCost(target);
	int gold = GetPartyGoldAmount();
	int damage = proc->amount;

	if (damage > target->curHP)
		damage = target->curHP;

	if (damage > Div(gold, cost))
		damage = Div(gold, cost);

	if (damage <= 0)
		return;

	SetPartyGoldAmount(gold - damage * cost);
	gActionData.unk0A = damage;

	NewMuSkillAnimOnActiveUnit(SID_Takeover, callback_anim, callback_exec);
}

static const struct ProcCmd ProcScr_Takeover[] = {
	PROC_NAME("Takeover"),
	PROC_CALL(EndAllMus),
	PROC_SLEEP(1),

	PROC_CALL(TakeoverPad_Init),
	PROC_REPEAT(TakeoverPad_Loop),
	PROC_CALL(TakeoverPad_End),

	PROC_CALL(Takeover_Apply),
	PROC_END,
};

bool Action_Takeover(ProcPtr parent)
{
	ShowUnitSprite(gActiveUnit);
	Proc_StartBlocking(ProcScr_Takeover, parent);
	return false;
}

#endif
