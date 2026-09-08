#include "common-chax.h"
#include "kernel-lib.h"
#include "C_Code.h"

/* lynjump-detector Round 2 greps this .c for checks matching ORG $ hooks in C_Code.lyn.event */
LYN_REPLACE_CHECK(EquipUnitItemSlot);
LYN_REPLACE_CHECK(UnitChangeFaction);
LYN_REPLACE_CHECK(IsExtraBonusClaimEnabled);
LYN_REPLACE_CHECK(sub_80AA550);
LYN_REPLACE_CHECK(sub_80B0674);
LYN_REPLACE_CHECK(sub_80B06FC);
LYN_REPLACE_CHECK(InitBonusClaimData);
LYN_REPLACE_CHECK(DrawBonusClaimItemText);
LYN_REPLACE_CHECK(SetBonusItemClaimed);
LYN_REPLACE_CHECK(SetupBonusClaimTargets);
LYN_REPLACE_CHECK(BonusClaim_Init);
LYN_REPLACE_CHECK(BonusClaim_Loop_MainKeyHandler);
LYN_REPLACE_CHECK(BonusClaim_DrawTargetUnitSprites);
LYN_REPLACE_CHECK(sub_80B1008);
LYN_REPLACE_CHECK(TryClaimBonusItem);
LYN_REPLACE_CHECK(BonusClaim_Loop_SelectTargetKeyHandler);
LYN_REPLACE_CHECK(BonusClaim_EndSelectTargetSubMenu);
LYN_REPLACE_CHECK(BonusClaim_DrawItemSentPopup);
LYN_REPLACE_CHECK(BonusClaim_Loop_PopupDisplayTimer);
LYN_REPLACE_CHECK(BonusClaim_ClearItemSentPopup);
LYN_REPLACE_CHECK(BonusClaim_OnEnd);
LYN_REPLACE_CHECK(StartBonusClaimScreen);
