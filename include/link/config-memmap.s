.macro SET_DATA name, value
    .global \name
    .type \name, object
    .set \name, \value
.endm

.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm


SET_DATA FreeRamSpaceTop,    0x02026E30
SET_DATA FreeRamSpaceBottom, 0x02028E58
SET_DATA UsedFreeRamSpaceTop, FreeRamSpaceBottom

.macro _kernel_malloc name, size
    .set UsedFreeRamSpaceTop, UsedFreeRamSpaceTop - \size
    SET_DATA \name, UsedFreeRamSpaceTop
.endm

SET_DATA FreeDemoRamSpaceTop,    0x0203F150 @ see GetLoadUnitsAmount
SET_DATA FreeDemoRamSpaceBottom, 0x02040000
SET_DATA UsedFreeDemoRamSpaceTop, FreeDemoRamSpaceBottom

.macro _kernel_malloc_demo name, size
    .set UsedFreeDemoRamSpaceTop, UsedFreeDemoRamSpaceTop - \size
    SET_DATA \name, UsedFreeDemoRamSpaceTop
.endm

SET_DATA FreeRamSpace2Top,    0x0203AAA4
SET_DATA FreeRamSpace2Bottom, 0x0203DDE0
SET_DATA UsedFreeRamSpace2Bottom, FreeRamSpace2Top

.macro _kernel_malloc2 name, size
    SET_DATA \name, UsedFreeRamSpace2Bottom
    .set UsedFreeRamSpace2Bottom, UsedFreeRamSpace2Bottom + \size
.endm

SET_DATA FreeRamSpace3Top,    0x02026AD0
SET_DATA FreeRamSpace3Bottom, 0x02026E30
SET_DATA UsedFreeRamSpace3Top, FreeRamSpace3Bottom

.macro _kernel_malloc3 name, size
    .set UsedFreeRamSpace3Top, UsedFreeRamSpace3Top - \size
    SET_DATA \name, UsedFreeRamSpace3Top
.endm

SET_DATA EwramOverlay0_FreeRamSpaceTop,    0x201F200
SET_DATA EwramOverlay0_FreeRamSpaceBottom, 0x2020188
SET_DATA EwramOverlay0_UsedFreeRamSpaceTop, EwramOverlay0_FreeRamSpaceBottom

.macro _kernel_malloc_overlay0 name, size
    .set EwramOverlay0_UsedFreeRamSpaceTop, EwramOverlay0_UsedFreeRamSpaceTop - \size
    SET_DATA \name, EwramOverlay0_UsedFreeRamSpaceTop
.endm

/* From the bottom to the top */
_kernel_malloc mgba_print_level, 4
_kernel_malloc __stdio_FILEs, 0x10
_kernel_malloc sSkillList, 0x50 * 3
_kernel_malloc sSkillFastList, 0x100
_kernel_malloc sLearnedSkillPLists, 51 * 0x20
_kernel_malloc sEfxSkillRoundData, 8 * 0x21
_kernel_malloc sEfxCombatArtRoundData,  0x30
_kernel_malloc gBattleActorGlobalFlag, 0x10
_kernel_malloc gBattleTargetGlobalFlag, 0x10
_kernel_malloc gCombatArtStatus, 0x10
_kernel_malloc sRandSeedsC, 8
_kernel_malloc gBanimSyncHandler, 8
_kernel_malloc gComboAtkList, 0x14
_kernel_malloc sKTutorialBits, 0xC
_kernel_malloc sKTutorialBitsHistory, 0xC
_kernel_malloc gBattleTargetPositionBackup, 0x4
_kernel_malloc gActionDataExpa, 0x10
_kernel_malloc sStatDebuffStatusAlly, 40 * 16 // Was originally 51 but reduced to 40 for 4th allegiance
_kernel_malloc sStatDebuffStatusEnemy, 51 * 16
_kernel_malloc sStatDebuffStatusNpc, 8 * 16
_kernel_malloc sStatDebuffStatusFourth, 10 * 16 // CONFIG_FOURTH_ALLEGIANCE
_kernel_malloc sStatDebuffStatusBattleUnit, 2 * 16
_kernel_malloc sStatDebuffMsgBuf, 0x2C * 9 // JESTER - Expanded from 7 to 9 to account for curHP and maxHP
_kernel_malloc sExpaConvoyItemCount, 4
_kernel_malloc sExpaConvoyItemArray, 2 * 200
_kernel_malloc sGaidenMagicListObj, 0x24
_kernel_malloc gpActorShileInfo, 4
_kernel_malloc gpTargetShileInfo, 4
_kernel_malloc sShileldInfoCache, 0x14 * 4
_kernel_malloc sPopupSkillStack, 0x10
_kernel_malloc gPlayStExpa, 0x10
_kernel_malloc sShileldInfoNext, 1
_kernel_malloc GenericBufferUsedFlag, 1
_kernel_malloc sKernelHookSkippingFlag, 1
_kernel_malloc sAnimNumberSlot, 1
_kernel_malloc sStatDebuffMsgBufNext, 1
_kernel_malloc gKonamiComboStep, 1
_kernel_malloc sEfxNosferatuEfxIndexCacheMagic, 1
_kernel_malloc sEfxNosferatuEfxIndexCacheData, 1
_kernel_malloc gSkillDbgList, 20 * 4
_kernel_malloc gForgedItemRam, 50 * 2
_kernel_malloc gDeadUnits, 50
_kernel_malloc sAumDeadUnit, 1
_kernel_malloc gChapterTimerSeconds, 2
_kernel_malloc firstVisibleIndex, 1
_kernel_malloc gInfuseMenuArray, 6
_kernel_malloc gTopVisibleListIndex, 2
_kernel_malloc gBEXP_State, 1
_kernel_malloc gBEXP_Applied, 1
_kernel_malloc gBEXP_Total, 2
_kernel_malloc gBEXP_MapGain, 2
_kernel_malloc gBexpFromWorldMap, 2
_kernel_malloc gList_Total, 2
_kernel_malloc gPrepMenuVisibleTableSlots, 12
_kernel_malloc gBaseConversations_Flags, 10
_kernel_malloc gCharacterBiographyPage, 1
_kernel_malloc gCharacterBiographyListNumber, 1
_kernel_malloc gEventReplay_SelectedChapter, 1
_kernel_malloc gEventReplay_ChapterScrollIndex, 1
_kernel_malloc gGameOptionsUiOrder_NEW, 32
_kernel_malloc sWmManageSkillsEmpty, 2
_kernel_malloc sWmManageSkillsMode, 2
_kernel_malloc sArenaRosterRuntimeState, 4
_kernel_malloc sArenaRosterSuspendState, 34
_kernel_malloc sSkillStaffMenuState, 16
_kernel_malloc sSkillStaffSuspendState, 68
_kernel_malloc sTextEngineWaveOffsets, 0x280
_kernel_malloc sTextEngineWaveActiveBuffer, 2
_kernel_malloc sTextEngineNameplateState, 0xA0
_kernel_malloc gTonicChapterState, 2
_kernel_malloc gUnitTonicState, 50
_kernel_malloc gStartMapEffectsUnlockMask, 2
_kernel_malloc sStartMapEffectsTexts, 5 * 8
_kernel_malloc sStartMapEffectsSuspendState, 4
_kernel_malloc sPhoenixMenuActive, 2
_kernel_malloc sDeadUnitCount, 2
_kernel_malloc gSnekCoinPresent, 2
_kernel_malloc gSnekCoinCoordinates, 2
_kernel_malloc gSnekSnakeState, 6
_kernel_malloc gSnekSnakeX, 2
_kernel_malloc gSnekSnakeY, 2
_kernel_malloc gSnekSnakeBodyX, 32
_kernel_malloc gSnekSnakeBodyY, 32
_kernel_malloc gSnekSnakeBodyLength, 2
_kernel_malloc gSnekCurrentScore, 2
_kernel_malloc gSnekHighScore, 2
_kernel_malloc gSnekHighScoreMagic, 2
_kernel_malloc gSnekLastTravelDirection, 2
_kernel_malloc gSnekLastPressedDirection, 2
_kernel_malloc gSavedWorldMapUnitId, 2
_kernel_malloc sCameraScrollCounter, 2
_kernel_malloc gSavedWorldMapXCoordiate, 1
_kernel_malloc gSavedWorldMapYCoordiate, 1
_kernel_malloc gAchMenuSaveSt, 4

/* Skill-tree screen state: seven bytes of flags/cursor data plus HelpBoxInfo. */
_kernel_malloc gSkillTreeCursor, 0x24
SET_DATA gSkillTreeCursorPid, gSkillTreeCursor + 1
SET_DATA gSkillTreeLastPage, gSkillTreeCursor + 2
SET_DATA gSkillTreeConfirming, gSkillTreeCursor + 3
SET_DATA gSkillTreeConfirmChoice, gSkillTreeCursor + 4
SET_DATA gSkillTreeMessage, gSkillTreeCursor + 5
SET_DATA gSkillTreePageDrawn, gSkillTreeCursor + 6
SET_DATA gSkillTreeHelp, gSkillTreeCursor + 8

// u32[7], one bit per unit index, set when a unit actually performs an AI action this phase
_kernel_malloc sAiPhasePerformedBits, 28

/**
 * Real-time battle scheduler state.
 * Size kept even: active/paused/gateOwner/pad(4) + timers(4)
 * + nextEnemyIndex/pad(2) + enemyCooldown(52) = 62.
 */
_kernel_malloc gRealtimeBattleState, 62

/* MokhaAOE transient target list + EXP/selection state */
_kernel_malloc sGambitTargetSaveBuf, 0x42
_kernel_malloc sGambitExpAccum, 2
_kernel_malloc sGambitSelectedAttack, 2

// JESTER - Warning, do not assign any even numbered amount of bytes if the next address would be odd.
// Ensure that if you assign a byte for something, you include an addition one to keep the next available address even

_kernel_malloc gUndeployedUnitCount, 32

_kernel_malloc gBattleFlagExt, 0x10
_kernel_malloc BanimSwitcherBuf, 0x40
_kernel_malloc BanimSwitcherAnimDef, 0x8

// For Eebit's FE7 Mode Select port
// Keep the two byte-sized blend state values in one even-sized allocation.
_kernel_malloc gUnk_ModeSelect_02000000, 2
SET_DATA gUnk_ModeSelect_02000001, gUnk_ModeSelect_02000000 + 1
// The menu can display three characters at once.
_kernel_malloc gUnk_0201E8D4, 0x38 * 3 // struct AnimBuffer[3]
_kernel_malloc gUnk_0201E97C, 0x28 * 3 // struct AnimMagicFxBuffer[3]
_kernel_malloc gUnk_020000A4, 0x50 // struct Font plus struct Text[7]
_kernel_malloc gUnk_0201E9F4, 0x5A // u16[3][15]


/* CONFIG */

@ _kernel_malloc _kernel_malloc_align4_pad, 3

/**
 * Free space allocated from icon display
 */
_kernel_malloc3 gBattleHitArrayRe, 4 * 0x21
_kernel_malloc3 gExtBattleHitArray, 4 * 0x21
_kernel_malloc3 gAnimRoundDataRe, 2 * 0x21 + 2
_kernel_malloc3 gEfxHpLutRe, 2 * 0x61 + 2

/**
 * These part of space is allocated from `ewram_overlay_0`
 * For more space, place refer to decomp ldscript.
 *
 * Note that since this space is in a time-sharing
 * relationship with banim, it is risky to use this area rashly.
 */
_kernel_malloc_overlay0 sPrepEquipSkillList, 0x120
_kernel_malloc_overlay0 UnitMenuSkills, 16
_kernel_malloc_overlay0 sEfxSkillQueue, 32
_kernel_malloc_overlay0 gBattleTemporaryFlag, 0x10
_kernel_malloc_overlay0 sCombatArtList, 0x20
_kernel_malloc_overlay0 sSelectedComatArtIndex, 4
_kernel_malloc_overlay0 sCombatArtBKSELfxTimer, 4
_kernel_malloc_overlay0 sHelpBoxType, 4
_kernel_malloc_overlay0 BattleOrderSkills, 8
_kernel_malloc_overlay0 gComboMapAnimBattleUnit, 0x80
_kernel_malloc_overlay0 KernelMoveMapFlags, 4
_kernel_malloc_overlay0 KernelExtMoveBarrierMap, 4
_kernel_malloc_overlay0 KernelExtMovePioneerMap, 4
_kernel_malloc_overlay0 MapTaskVec, 4
_kernel_malloc_overlay0 gStatScreenStExpa, 4
_kernel_malloc_overlay0 BattleRoundInfoBak, 0x100
_kernel_malloc_overlay0 gDmg, 40
_kernel_malloc_overlay0 BattleSysBattleStatusBackup, 32
_kernel_malloc_overlay0 sAiSimuSlotBuf, 0x100
_kernel_malloc_overlay0 gItemPageList, 0x28
_kernel_malloc_overlay0 gPostActionCommonBuffer, 0x20
_kernel_malloc_overlay0 sTmpMovCostTable, 0x44
_kernel_malloc_overlay0 gWtaStatus_act, 0x38
_kernel_malloc_overlay0 gWtaStatus_tar, 0x38
_kernel_malloc_overlay0 gActorBaseDmg,  0x0C
_kernel_malloc_overlay0 gTargetBaseDmg, 0x0C
_kernel_malloc_overlay0 sOverrideState, 0x48

/* Dedicated map storage for enemy fog-of-war AI. */
_kernel_malloc2 gBmMapEnemyVisionBuffer, 0x800
_kernel_malloc2 gBmMapEnemyVision, 4

/* Chapter talk chatlog (ring buffer + UI scratch). SUS-persisted state is sChatLogState only. */
_kernel_malloc2 sChatLogState, 0x444
_kernel_malloc2 sChatlogUiState, 0x600

// _kernel_malloc_overlay0 _kernel_malloc_overlay0_align4_pad, 3

/**
 * Usage of memory on IWRAM for arm-functions
 *
 * part     function name       start           end             max size    real size
 * [a]      ARM_MapFloodCoreRe  0x03003CAC      0x03003F94      0x2E8       0x2E8
 * [a]      ARM_MapTask         0x03003F94      0x03003FF0      0x05C       0x05C
 * [a]      ARM_SkillTester     0x03003FF0      0x03004150      0x138       0x160
 *
 * [b]      __free__            end_of_irq      0x03004838      ---         ---
 * [b]      ARM_UnitList        0x03004838      0x03004924      0x0EC       0x0EC
 * [b]      ARM_SkillList       0x03004924      0x03004960      0x03C       0x03C
 *
 * Note on part[a]:
 * In vanilla, RAM func left a ram space at: 0x03003F48 - 0x03004150
 * But since MovementSkill has rewriten function: MapFloodCore/MapFloodCoreStep
 * So we get antother space as 0x03003CAC - 0x03003F48
 * Now this part of free IWRAM space is: [0x03003CAC - 0x03004150]
 */
dat 0x03003CAC, ARM_MapFloodCoreRe
dat 0x03003F94, ARM_MapFloodCoreReEnd
dat 0x03003F94, ARM_MapTask
dat 0x03003FF0, ARM_MapTaskEnd
dat 0x03003FF0, ARM_SkillTester
dat 0x03004150, ARM_SkillTesterEnd

dat 0x03004838, ARM_UnitList
dat 0x03004924, ARM_UnitListEnd
dat 0x03004924, ARM_SkillList
dat 0x03004960, ARM_SkillListEnd
