#include "common-chax.h"
#include "combat-art.h"
#include "battle-system.h"
#include "constants/texts.h"
#include "skill-system.h"
#include "constants/skills.h"

enum CombatArtBKSELfxConfig {
	// Real VRAM Offset to uncompress: OBJ_VRAM0 + OBJ_MOKHA_VRAMOFF
	OBJ_ARROW_VRAMOFF = 0x0320,

	// Real Palette index: OBJ_ARROW_PAL + 0x10
	OBJ_ARROW_PAL = 0x3,

	// Obj config
	OBJ_ARROW_L_XPOS = -1,
	OBJ_ARROW_R_XPOS = 0x4B,
	OBJ_ARROW_YPOS = 0x16A,
};

STATIC_DECLAR void BKSELfxUpdateAoeSprits(struct BattleForecastProc *proc)
{
	const u8 Y_SYNC_OFFs[] = {0, 1, 2, 3, 2, 1};
	const u8 Y_SYNC_TIMEs[] = {0, 0, 1, 2, 2};
	const s8 X_TILEs[] = {-16, 0, 0, 0, 16};
	const s8 Y_TILEs[] = {0, 16, 0, -16, 0};

	int i;
	u8 cid = GetCombatArtByTargetSelIndex();

	if (!COMBART_VALID(cid) || !GetCombatArtInfo(cid)->aoe_debuff)
		return;

	for (i = 0; i < 5; i++) {
		int ix = gBattleTarget.unit.xPos * 16 - gBmSt.camera.x + X_TILEs[i];
		int iy = gBattleTarget.unit.yPos * 16 - gBmSt.camera.y + Y_TILEs[i];

		if (ix > 0xF0 || iy > 0xA0)
			continue;

		PutSprite(
			0,
			0x1FF & (ix + 0x204),
			0xFF  & (iy + 0x103 + Y_SYNC_OFFs[sCombatArtBKSELfxTimer + Y_SYNC_TIMEs[i]]),
			gObject_8x8,
			OAM2_PAL(OBJ_ARROW_PAL) + OAM2_LAYER(0b00) + OAM2_CHR(OBJ_ARROW_VRAMOFF / 0x20)
		);
	}
}

STATIC_DECLAR void BKSELfxUpdateSprits(struct BattleForecastProc *proc)
{
	const int X = proc->side	  == -1 ? 0 : 0x9F;
	const int Y = proc->frameKind ==  1 ? 0 : 0x20;
	const int VOFF = (OBJ_ARROW_VRAMOFF / 0x20) + 1 + sCombatArtBKSELfxTimer;

	if (!(GetGameClock() & 0x7)) {
		sCombatArtBKSELfxTimer++;
		if (sCombatArtBKSELfxTimer >= 6)
			sCombatArtBKSELfxTimer = 0;
	}

	if (!CombatArtSelectTargetExist())
		return;

	/* Put arrow */
	PutSprite(
		0,
		OBJ_ARROW_L_XPOS + X,
		OBJ_ARROW_YPOS + Y,
		gObject_8x16,
		OAM2_PAL(OBJ_ARROW_PAL) + OAM2_LAYER(0b00) + OAM2_CHR(VOFF)
	);

	PutSprite(
		0,
		OBJ_ARROW_R_XPOS + X,
		OBJ_ARROW_YPOS + Y,
		gObject_8x16_HFlipped,
		OAM2_PAL(OBJ_ARROW_PAL) + OAM2_LAYER(0b00) + OAM2_CHR(VOFF)
	);

	BKSELfxUpdateAoeSprits(proc);
}

STATIC_DECLAR bool CanDisplayCombartArrow(void)
{
	switch (gPlaySt.chapterWeatherId) {
	case WEATHER_SNOWSTORM:
	case WEATHER_SANDSTORM:
	case WEATHER_FLAMES:
		return false;

	default:
		break;
	}

	return true;
}

/* External jump */
void BattleForecast_InitRework(struct BattleForecastProc *proc)
{
	/* Vanilla */
	BattleForecast_Init(proc);
	StartGreenText(proc);

	if (!CanDisplayCombartArrow())
		return;

	if (CanUnitPlayCombatArt(gActiveUnit, GetItemFromSlot(gActiveUnit, gActionData.itemSlotIndex))) {
		sCombatArtBKSELfxTimer = 0;

		Decompress(gpImg_BkselObjArrow, gGenericBuffer);
		Copy2dChr(gGenericBuffer, OBJ_VRAM0 + OBJ_ARROW_VRAMOFF, 7, 2);

		ApplyPalette(gpPal_BkselObjArrow, OBJ_ARROW_PAL + 0x10);

		StartParallelWorker(BKSELfxUpdateSprits, proc);
	}
}

LYN_REPLACE_CHECK(DrawBattleForecastContentsStandard);
void DrawBattleForecastContentsStandard(struct BattleForecastProc * proc)
{
    int damage;
    int critRate;

    CallARM_FillTileRect(gUiTmScratchB, gTSA_BattleForecastStandard, 0x1000);

    TileMap_FillRect(gUiTmScratchA, 10, 15, 0);

    PutBattleForecastUnitName(gUiTmScratchA + 0x23, &proc->unitNameTextA, &gBattleActor.unit);

	/* Only show the target's full identity at Stage 0 (full visibility) */
	if (!gpKernelDesignerConfig->multiple_fog_stages
			|| !gPlaySt.chapterVisionRange
			|| gBmMapFog[gBattleTarget.unit.yPos][gBattleTarget.unit.xPos] >= 3)
	{
		PutBattleForecastUnitName(gUiTmScratchA + 0x161, &proc->unitNameTextA, &gBattleTarget.unit);
		PutBattleForecastItemName(gUiTmScratchA + 0x1A1, &proc->itemNameText, gBattleTarget.weaponBefore);
	}
	else
	{
		char* str = "N/A";
		int position = GetStringTextCenteredPos(48, str);
		ClearText(&proc->unitNameTextA);
		PutDrawText(&proc->unitNameTextA, gUiTmScratchA + 0x161, 0, position, 0, str);
	}

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    if (SkillTester(GetUnit(gBattleActor.unit.index), SID_UnarmedCombat))
    {
        if (gBattleActor.weapon == 0)
        {
            int actorAccuracy = gBattleActor.unit.skl * 2;
            int targetAvoid = gBattleTarget.battleAvoidRate;
            int calculatedHit = SKILL_EFF0(SID_UnarmedCombat) + actorAccuracy - targetAvoid;
            gBattleActor.battleEffectiveHitRate = calculatedHit > 100 ? 100 : calculatedHit;
        }
    }
    if (SkillTester(GetUnit(gBattleTarget.unit.index), SID_UnarmedCombat))
    {
        if (gBattleTarget.weapon == 0)
        {
            int targetAccuracy = gBattleTarget.unit.skl * 2;
            int actorAvoid = gBattleActor.battleAvoidRate;
            int calculatedHit = SKILL_EFF0(SID_UnarmedCombat) + targetAccuracy - actorAvoid;
            gBattleTarget.battleEffectiveHitRate = calculatedHit > 100 ? 100 : calculatedHit;
        }
    }
#endif

    if ((gBattleTarget.weapon == 0) && (gBattleTarget.weaponBroke == 0)) {
        damage = -1;

        gBattleTarget.battleEffectiveHitRate = 0xFF;
        gBattleTarget.battleEffectiveCritRate = 0xFF;
    } else {
        damage = gBattleTarget.battleAttack - gBattleActor.battleDefense;

        if (damage < 0) {
            damage = 0;
        }
    }

	if (gpKernelDesignerConfig->multiple_fog_stages == true
			&& gPlaySt.chapterVisionRange
			&& gBmMapFog[gBattleTarget.unit.yPos][gBattleTarget.unit.xPos] < 3)
	{
		PutNumberTwoChr(gUiTmScratchA + 0x62, 2, 0xFF);
		PutNumberTwoChr(gUiTmScratchA + 0xA2, 2, 0xFF);
		PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x40, 2, 0xFF);
		PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x80, 2, 0xFF);
	}
	else
	{
		if (gBattleTarget.hpInitial > 99) {
			PutNumberTwoChr(gUiTmScratchA + 0x62, 2, 0xFF);
		} else {
			PutNumberTwoChr(gUiTmScratchA + 0x62, 2, gBattleTarget.hpInitial);
		}

		PutNumberTwoChr(gUiTmScratchA + 0xA2, 2, damage);
		PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x40, 2, gBattleTarget.battleEffectiveHitRate);
		PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x80, 2, gBattleTarget.battleEffectiveCritRate);
	}

    damage = gBattleActor.battleAttack - gBattleTarget.battleDefense;

    if (GetItemIndex(gBattleActor.weapon) == ITEM_MONSTER_STONE) {
        damage = 0xFF;
    }

    if (damage < 0) {
        damage = 0;
    }

    critRate = gBattleActor.battleEffectiveCritRate;

    if (GetItemIndex(gBattleActor.weapon) == ITEM_MONSTER_STONE) {
        critRate = 0xFF;
    }

    if (critRate < 0) {
        critRate = 0;
    }

    if (gBattleActor.hpInitial > 99) {
        PutNumberTwoChr(gUiTmScratchA + 0xA8 - 0x40, 2, 0xFF);
    } else {
        PutNumberTwoChr(gUiTmScratchA + 0xA8 - 0x40, 2, gBattleActor.hpInitial);
    }

    PutNumberTwoChr(gUiTmScratchA + 0xA8, 2, damage);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x40, 2, gBattleActor.battleEffectiveHitRate);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x80, 2, critRate);

    PutTwoSpecialChar(gUiTmScratchA + 0xA8 - 0x44, TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_HP_A, TEXT_SPECIAL_HP_B);

    PutText(gaBattleForecastTextStructs, gUiTmScratchA  + 0xA8 - 5);
    PutText(gaBattleForecastTextStructs + 1, gUiTmScratchA  + 0xA8 + 0x3B);
    PutText(gaBattleForecastTextStructs + 2, gUiTmScratchA  + 0xA8 + 0x7B);

    DrawIcon(gUiTmScratchA + 0xA8 + 0xBF, GetItemIconId(gBattleTarget.weaponBefore), 0x4000);

    DrawIcon(gUiTmScratchA + 0xA8 - 0x87, GetItemIconId(gBattleActor.weaponBefore), 0x3000);
}

LYN_REPLACE_CHECK(DrawBattleForecastContentsExtended);
void DrawBattleForecastContentsExtended(struct BattleForecastProc * proc)
{
    CallARM_FillTileRect(gUiTmScratchB, gTSA_BattleForecastExtended, 0x1000);

    TileMap_FillRect(gUiTmScratchA, 10, 19, 0);

    PutBattleForecastUnitName(gUiTmScratchA + 0x23, &proc->unitNameTextA, &gBattleActor.unit);
    PutBattleForecastUnitName(gUiTmScratchA + 0x1E1, &proc->unitNameTextA, &gBattleTarget.unit);

    PutBattleForecastItemName(gUiTmScratchA + 0x221, &proc->itemNameText, gBattleTarget.weaponBefore);

    if ((gBattleTarget.weapon == 0) && (!gBattleTarget.weaponBroke)) {
        gBattleTarget.battleAttack = 0xFF;
        gBattleTarget.battleEffectiveHitRate = 0xFF;
        gBattleTarget.battleEffectiveCritRate = 0xFF;
    }

    if (gBattleTarget.hpInitial > 99) {
        PutNumberTwoChr(gUiTmScratchA + 0x62, 2, 0xFF);
    } else {
        PutNumberTwoChr(gUiTmScratchA + 0x62, 2, gBattleTarget.hpInitial);
    }

    PutNumberTwoChr(gUiTmScratchA + 0xA2, 2, gBattleTarget.battleAttack);
    PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x40, 2, gBattleTarget.battleDefense);
    PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x80, 2, gBattleTarget.battleEffectiveHitRate);
    PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0xC0, 2, gBattleTarget.battleEffectiveCritRate);
    PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x100, 2, gBattleTarget.battleSpeed);

    if (gBattleActor.hpInitial > 99) {
        PutNumberTwoChr(gUiTmScratchA + 0xA2 - 0x3A, 2, 0xFF);
    } else {
        PutNumberTwoChr(gUiTmScratchA + 0xA2 - 0x3A, 2, gBattleActor.hpInitial);
    }

    PutNumberTwoChr(gUiTmScratchA + 0xA8, 2, gBattleActor.battleAttack);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x40, 2, gBattleActor.battleDefense);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x80, 2, gBattleActor.battleEffectiveHitRate);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0xC0, 2, gBattleActor.battleEffectiveCritRate);
    PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x100, 2, gBattleActor.battleSpeed);

    PutTwoSpecialChar(gUiTmScratchA + 0xA8 - 0x44, TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_HP_A, TEXT_SPECIAL_HP_B);

    PutText(gaBattleForecastTextStructs + 3, gUiTmScratchA + 0xA8 - 5);
    PutText(gaBattleForecastTextStructs + 4, gUiTmScratchA + 0xA8 + 0x3B);
    PutText(gaBattleForecastTextStructs + 1, gUiTmScratchA + 0xA8 + 0x7B);
    PutText(gaBattleForecastTextStructs + 2, gUiTmScratchA + 0xA8 + 0xBB);
    PutText(gaBattleForecastTextStructs + 5, gUiTmScratchA + 0xA8 + 0xFB);

    DrawIcon(gUiTmScratchA + 0xA8 + 0x13F, GetItemIconId(gBattleTarget.weaponBefore), 0x4000);

    DrawIcon(gUiTmScratchA + 0xA8 - 0x87, GetItemIconId(gBattleActor.weaponBefore), 0x3000);
}

STATIC_DECLAR void DrawBattleForecastContentsStandardRework(struct BattleForecastProc *proc)
{
	DrawBattleForecastContentsStandard(proc);

	if (CanUnitPlayCombatArt(gActiveUnit, GetItemFromSlot(gActiveUnit, gActionData.itemSlotIndex)))
		CallARM_FillTileRect(gUiTmScratchB, gpTsa_BkselStandardScreen, 0x1000);
}

STATIC_DECLAR void DrawBattleForecastContentsExtendedRework(struct BattleForecastProc *proc)
{
	DrawBattleForecastContentsExtended(proc);

	if (CanUnitPlayCombatArt(gActiveUnit, GetItemFromSlot(gActiveUnit, gActionData.itemSlotIndex)))
		CallARM_FillTileRect(gUiTmScratchB, gpTsa_BkselDetailedScreen, 0x1000);
}

STATIC_DECLAR void DrawBattleForecastContentsVanilla(struct BattleForecastProc *proc)
{
	proc->unk_2C = 0;
	proc->needContentUpdate = 0;

	switch (proc->frameKind) {
	case 1:
		InitBattleForecastBattleStats(proc);
		DrawBattleForecastContentsStandardRework(proc);
		break;

	case 2:
		InitBattleForecastBattleStats(proc);
		DrawBattleForecastContentsExtendedRework(proc);
		break;
	}
}

LYN_REPLACE_CHECK(DrawBattleForecastContents);
void DrawBattleForecastContents(struct BattleForecastProc *proc)
{
	u8 cid;
	char *str;
	struct Text *text = &proc->itemNameText;
	const int Y = proc->frameKind ==  1 ? 0x0D : 0x11;

	/* Vanilla */
	DrawBattleForecastContentsVanilla(proc);

	ClearText(text);

	if (!CanUnitPlayCombatArt(gActiveUnit, GetItemFromSlot(gActiveUnit, gActionData.itemSlotIndex))) {
		/* If no arts, draw item name */
		str = GetItemName(gBattleTarget.weaponBefore);

		PutDrawText(
			text,
			TILEMAP_LOCATED(gUiTmScratchA, 0x1, Y),
			TEXT_COLOR_SYSTEM_WHITE,
			GetStringTextCenteredPos(0x40, str),
			0, str
		);
		return;
	}

	cid = GetCombatArtByTargetSelIndex();

	if (!COMBART_VALID(cid)) {
		str = GetStringFromIndex(MSG_COMBATART_BKSEL_NO_ARTS);

		PutDrawText(
			text,
			TILEMAP_LOCATED(gUiTmScratchA, 0x1, Y),
			TEXT_COLOR_SYSTEM_WHITE,
			GetStringTextCenteredPos(0x40, str),
			0, str
		);
	} else {
		str = GetStringFromIndex(GetCombatArtName(cid));

		PutDrawText(
			text,
			TILEMAP_LOCATED(gUiTmScratchA, 0x1, Y),
			TEXT_COLOR_SYSTEM_GREEN,
			GetStringTextCenteredPos(0x40, str),
			0, str
		);
	}
}
