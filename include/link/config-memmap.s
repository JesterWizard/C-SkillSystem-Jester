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


SET_DATA FreeRamSpaceTop,    0x02026FC8 @ after the five new purple unit slots at 0x02026E30 (FourthAllegiance/NewUnitRAMPointerSetter.event)
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
/*
 * The learned skills were here (51 * 0x20 bytes; now gLearnedSkills, below). Kept reserved: FreeMovement's iFRAM
 * (0x2028924, cFreeMU/src/_include/MokhaRAMSpace.s) is inside it, and freeing it moves every allocation after it
 * onto addresses that LimitedShopStock (0x202774C) and FreeMovement hardcode.
 */
_kernel_malloc sReservedOldLearnedSkills, 51 * 0x20
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
_kernel_malloc sStatDebuffStatusAllyOldSlot, 40 * 16 // Unused: too small for 41 allies (the 41st overlapped gActionDataExpa); sStatDebuffStatusAlly is in the kernel_malloc2 region
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
_kernel_malloc sWRankPopupStack, 0x20
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
_kernel_malloc gChapterTimerSeconds, 2
_kernel_malloc sAumDeadUnit, 1
_kernel_malloc firstVisibleIndex, 1
_kernel_malloc gInfuseMenuArray, 6
_kernel_malloc gTopVisibleListIndex, 2
_kernel_malloc gBEXP_State, 1
_kernel_malloc gBEXP_Applied, 1
_kernel_malloc gBEXP_Total, 2
_kernel_malloc gBEXP_MapGain, 2
_kernel_malloc gBexpFromWorldMap, 2
_kernel_malloc gInfuseFromWorldMap, 2
_kernel_malloc gList_Total, 2
_kernel_malloc gPrepMenuVisibleTableSlots, 13
_kernel_malloc gBaseConversations_Flags, 10
_kernel_malloc gCharacterBiographyPage, 1
_kernel_malloc gCharacterBiographyListNumber, 1
_kernel_malloc gEventReplay_SelectedChapter, 1
_kernel_malloc gEventReplay_ChapterScrollIndex, 1
_kernel_malloc gGameOptionsUiOrder_NEW, 32
_kernel_malloc sWmManageSkillsEmpty, 2
_kernel_malloc sWmManageSkillsMode, 2
_kernel_malloc sArenaRosterRuntimeStateOldSlot, 4
_kernel_malloc sArenaRosterSuspendState, 34
_kernel_malloc sSkillStaffMenuStateOldSlot, 16
_kernel_malloc sSkillStaffSuspendStateOldSlot, 68
_kernel_malloc sTextEngineWaveOffsets, 0xF0
_kernel_malloc sTextEngineEarthquakeOffset, 2
_kernel_malloc sTextEngineNameplateStateOldSlot, 0xA0
_kernel_malloc gTonicChapterStateOldSlot, 2
_kernel_malloc gUnitTonicState, 50
_kernel_malloc gStartMapEffectsUnlockMask, 2
/* Old odd-address slot of sStartMapEffectsTexts, kept so the addresses below do not move; the texts are in _kernel_malloc2 */
_kernel_malloc sStartMapEffectsTextsOldSlot, 5 * 8
_kernel_malloc sStartMapEffectsSuspendState, 4
_kernel_malloc sPhoenixMenuActive, 2
_kernel_malloc sSkillScrollReplaceMenuActive, 2
_kernel_malloc sDeadUnitCount, 2
_kernel_malloc gSnekCoinPresentOldSlot, 2
_kernel_malloc gSnekCoinCoordinatesOldSlot, 2
_kernel_malloc gSnekSnakeState, 6
_kernel_malloc gSnekSnakeXOldSlot, 2
_kernel_malloc gSnekSnakeYOldSlot, 2
_kernel_malloc gSnekSnakeBodyX, 32
_kernel_malloc gSnekSnakeBodyY, 32
_kernel_malloc gSnekSnakeBodyLengthOldSlot, 2
_kernel_malloc gSnekCurrentScoreOldSlot, 2
_kernel_malloc gSnekHighScoreOldSlot, 2
_kernel_malloc gSnekHighScoreMagicOldSlot, 2
_kernel_malloc gSnekLastTravelDirectionOldSlot, 2
_kernel_malloc gSnekLastPressedDirectionOldSlot, 2
_kernel_malloc gSavedWorldMapUnitId, 2
_kernel_malloc sCameraScrollCounter, 2
_kernel_malloc gSavedWorldMapXCoordiate, 1
_kernel_malloc gSavedWorldMapYCoordiate, 1
_kernel_malloc gAchMenuSaveStOldSlot, 4

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
_kernel_malloc sAiPhasePerformedBitsOldSlot, 28

/* Real-time battle clock state, see struct RealtimeBattleState */
_kernel_malloc gRealtimeBattleStateOldSlot, 16

/* MokhaAOE transient target list + EXP/selection state */
_kernel_malloc sGambitTargetSaveBuf, 0x42
_kernel_malloc sGambitExpAccumOldSlot, 2
_kernel_malloc sGambitSelectedAttack, 2

// JESTER - Warning, do not assign any even numbered amount of bytes if the next address would be odd.
// Ensure that if you assign a byte for something, you include an addition one to keep the next available address even

_kernel_malloc gUndeployedUnitCount, 32

_kernel_malloc gBattleFlagExt, 0x10
_kernel_malloc BanimSwitcherBufOldSlot, 0x40
_kernel_malloc BanimSwitcherAnimDefOldSlot, 0x8

// For Eebit's FE7 Mode Select port
// Keep the two byte-sized blend state values in one even-sized allocation.
_kernel_malloc gUnk_ModeSelect_02000000, 2
SET_DATA gUnk_ModeSelect_02000001, gUnk_ModeSelect_02000000 + 1
// The menu can display three characters at once.
_kernel_malloc gUnk_0201E8D4OldSlot, 0x38 * 3 // struct AnimBuffer[3]
_kernel_malloc gUnk_0201E97COldSlot, 0x28 * 3 // struct AnimMagicFxBuffer[3]
_kernel_malloc gUnk_020000A4OldSlot, 0x50 // struct Font plus struct Text[7]
_kernel_malloc gUnk_0201E9F4OldSlot, 0x5A // u16[3][15]

/* Pathfinding ghost: AP obj list copy (count + up to 8 sprites * 3 attrs) */
_kernel_malloc sPathfindingGhostObjBufOldSlot, 50

/* Text engine scene layout: enabled, boxShown, instantOpen, savedNameplate */
_kernel_malloc sTextEngineSceneLayoutState, 4
/* Text engine big text: enabled, lineBelow, savedPrintDelay, printDelayForced */
_kernel_malloc sTextEngineBigTextState, 4
/* Text engine auto-advance: frames before every [A] moves on by itself */
_kernel_malloc sTextEngineAutoAdvanceState, 4
/* Class growth preview text for the promotion screen description box */
_kernel_malloc sClassGrowthPreviewText, 0xA0

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
_kernel_malloc_overlay0 sHelpBoxPageState, 8
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
_kernel_malloc_overlay0 sAiTargetList, 0x80

/* Dedicated map storage for enemy fog-of-war AI. */
_kernel_malloc2 gBmMapEnemyVisionBuffer, 0x800
_kernel_malloc2 gBmMapEnemyVision, 4

/* Talk chatlog (ring buffer + UI scratch), cleared at the start of each conversation. RAM only: not kept in any save. */
_kernel_malloc2 sChatLogState, 0x444
_kernel_malloc2 sChatlogUiState, 0x600

/**
 * EnterTown district-entry handshake. Consumed once by WMLoc_GetChapterId and
 * WorldMap_Destruct respectively.
 *
 * These previously lived at 0x03005268 and 0x03005266, which are bytes 24 and 22
 * of gPermanentFlagBits, so event IDs 192-199 hijacked the node-to-chapter lookup
 * and event IDs 176-183 forced a map reload, and reading either wiped those flags.
 * Keep them out of flag storage. Paired so the reservation stays even-sized.
 */
_kernel_malloc2 sWmChapterIdOverride, 1
_kernel_malloc2 sWmMapReloadPending, 1
/* Word-align sPortrait32State. Pointers in that block need 4-byte alignment. */
_kernel_malloc2 sPortrait32AlignPad, 2

/**
 * 32-color portrait overlay state. Four face-slot records (paired-bank pal
 * backup). Must stay 4-byte aligned. Even-sized; not suspend-persisted.
 * Slot = 4+32+2+1+1 = 40; four slots + magic = 0xA4.
 */
_kernel_malloc2 sPortrait32State, 0xA4

/* Boot language selector choice: lang + 1, 0 = not read from SRAM yet */
_kernel_malloc2 gKernelLanguage, 4

/* Three Houses-style gaiden magic: struct GaidenMagicUsesEnt[64], SUS-persisted */
_kernel_malloc2 sGaidenMagicUsesPool, 3 * 64

/* Spells learned on rank-up waiting for their popup: count + uid[4] + iid[4], padded to keep the next block word-aligned */
_kernel_malloc2 sGaidenMagicLearnPopups, 12

/* Magic/Physical Seal source positions, rebuilt at most once per frame (see MiscGetter.c) */
_kernel_malloc2 sSealSourceCache, 0x48

/* Undo skill: MP of both battle units when the real battle starts (struct UndoSnapshot) */
_kernel_malloc2 gUndoSnapshot, 4

/* Zenkai: EXP waiting to be granted after the staff action ([0] = unit index, [1] = EXP) */
_kernel_malloc2 gZenkaiPending, 4

/* World map story chapter: the last NEXT_CHAPTER_WITH_MAP target (see WmStoryChapter.c), padded to a word */
_kernel_malloc2 gWmStoryChapter, 4

/* Hex minigame screen state (struct HexScreenState in HexScreen.c), only used while the Hex screen is open */
_kernel_malloc2 gHexScreenState, 0x280

/* Hex save data (struct HexSaveData in hex.h): the card binder and district claims, saved with the game and suspend saves */
_kernel_malloc2 gHexSave, 0x54

/* Hex shop (vanilla shop screen in Hex mode): the item list of the open shop (struct HexShopRam in hex.h), not saved */
_kernel_malloc2 gHexShopRam, 0x20

/* Start map effects prompt: struct Text[5]. Halfword fields, so it must not sit at an odd address like the old _kernel_malloc slot */
_kernel_malloc2 sStartMapEffectsTexts, 5 * 8

/* Variables moved here from _kernel_malloc because an odd-size allocation above them left them at an
 * odd or unaligned address (halfword/word fields there corrupt their neighbours). Their old slots are kept
 * as <name>OldSlot so no other address moves. Sizes are padded to a word. */
_kernel_malloc2 sPathfindingGhostObjBuf, 52
_kernel_malloc2 gUnk_0201E9F4, 92
_kernel_malloc2 gUnk_020000A4, 80
_kernel_malloc2 gUnk_0201E97C, 120
_kernel_malloc2 gUnk_0201E8D4, 168
_kernel_malloc2 BanimSwitcherAnimDef, 8
_kernel_malloc2 BanimSwitcherBuf, 64
_kernel_malloc2 sGambitExpAccum, 4
_kernel_malloc2 gRealtimeBattleState, 16
_kernel_malloc2 sAiPhasePerformedBits, 28
_kernel_malloc2 gAchMenuSaveSt, 4
_kernel_malloc2 gSnekLastPressedDirection, 4
_kernel_malloc2 gSnekLastTravelDirection, 4
_kernel_malloc2 gSnekHighScoreMagic, 4
_kernel_malloc2 gSnekHighScore, 4
_kernel_malloc2 gSnekCurrentScore, 4
_kernel_malloc2 gSnekSnakeBodyLength, 4
_kernel_malloc2 gSnekSnakeY, 4
_kernel_malloc2 gSnekSnakeX, 4
_kernel_malloc2 gSnekCoinCoordinates, 4
_kernel_malloc2 gSnekCoinPresent, 4
_kernel_malloc2 gTonicChapterState, 4
_kernel_malloc2 sTextEngineNameplateState, 160
_kernel_malloc2 sSkillStaffSuspendState, 68
_kernel_malloc2 sSkillStaffMenuState, 16
_kernel_malloc2 sArenaRosterRuntimeState, 4

/* District fishing (include/kernel/fishing.h): the screen state (RAM only) and the saved records (a save chunk). */
_kernel_malloc2 gFishingState, 0x1A0
_kernel_malloc2 gFishingRecordsV1, 0x20 /* the prize table's records: unused since the journal, kept so nothing moves */

/* District River Crossing (include/kernel/crossing.h): the game state (RAM only) and the saved best scores (a save chunk). */
_kernel_malloc2 gCrossingState, 0x100
_kernel_malloc2 gCrossingRecords, 0x0C

/* District Tower Stack (include/kernel/tower.h): the game state (RAM only) and the saved best score (a save chunk). */
_kernel_malloc2 gTowerState, 0x100
_kernel_malloc2 gTowerRecords, 4
/* Tower Stack's collapse (struct TowerDebris[TOWER_DEBRIS]), after the others so no address above moves. RAM only. */
_kernel_malloc2 gTowerDebris, 0x80
/* The fish journal (struct FishingRecords, a save chunk), after the others so no address above moves. */
_kernel_malloc2 gFishingRecords, 0x2C
/* The allies' stat buffs and debuffs (struct StatDebuffStatus, include/kernel/debuff.h): 51 slots, enough for
   CONFIG_UNIT_AMT_ALLY with or without the fourth allegiance. Moved here from the kernel_malloc region (OldSlot). */
_kernel_malloc2 sStatDebuffStatusAlly, 51 * 16
/* The angler's progress (struct FishingProgress, include/kernel/fishing.h): rank, legends, bottles, requests. Saved. */
_kernel_malloc2 gFishingProgress, 0x20
/* River Crossing's twists, pickups and records screen (struct CrossingExtra, include/kernel/crossing.h). RAM only. */
_kernel_malloc2 gCrossingExtra, 0x80
/* River Crossing's saved progress (struct CrossingProgress): fastest clears, milestones, challenges. Saved. */
_kernel_malloc2 gCrossingProgress, 0x10
/* The guild board (include/kernel/guild.h): the saved tokens, quests and prizes (a save chunk), and its screen (RAM only). */
_kernel_malloc2 gGuildSave, 0x10
_kernel_malloc2 gGuildScreen, 0x80
/* Chapter awards (include/kernel/chapter-awards.h): this chapter's counts (a suspend-save chunk) and the screen (RAM only). */
_kernel_malloc2 gChapterAwards, 0x64
_kernel_malloc2 gChapterAwardsScreen, 0x60
/* Portrait expression frame sets (include/kernel/portrait-expressions.h): the set each face slot shows. RAM only. */
_kernel_malloc2 gPortraitFrameSlots, 0x14
/* 2x2 units (include/kernel/big-unit.h): the groups of corners (struct BigUnitGroup[8]). Saved in the suspend save. */
_kernel_malloc2 gBigUnitGroups, 0x30

/* Learned skills (struct LearnedSkills, include/kernel/skill-system.h): the whole army's, sorted */
_kernel_malloc2 gLearnedSkills, 0x7AC

/* New Game+: the carried skills (struct NewGamePlusCarry) and the save picked on the title */
_kernel_malloc2 gNewGamePlus, 0x1D8
_kernel_malloc2 gNewGamePlusArmed, 4

/* The Gallery screen (struct GalleryScreen, Kernel/Wizardry/Gallery/Source/GalleryScreen.c). RAM only. */
_kernel_malloc2 gGalleryScreen, 0xA0

/* The chapter the last NEXT_CHAPTER_WITH_MAP named (Kernel/Wizardry/WorldMap/WmStoryChapter.c). RAM only. */
_kernel_malloc2 gWmChapterNamed, 4

/* The Bestiary: the characters and classes this game has met (struct BestiaryBits). Kept in the saves' tails. */
_kernel_malloc2 gBestiarySeen, 0x44
/* The Skill Codex: the skills this game has met (struct SkillCodexBits). Kept in the saves' tails. */
_kernel_malloc2 gSkillCodexSeen, 0x84
/* The Bestiary screen (struct BestiaryScreen, Kernel/Wizardry/Bestiary/Source/BestiaryScreen.c), and the Skill Codex's
   (struct SkillCodexScreen, SkillCodex/Source/SkillCodexScreen.c): the Extras screens never run together. RAM only. */
_kernel_malloc2 gBestiaryScreen, 0x160

/* A popup is being drawn in a battle animation (Kernel/Wizardry/PopupRework/Source/EfxPopup.c). RAM only. */
_kernel_malloc2 gPopupInEfx, 4

/* What the level-up quote on the battle-animation level-up borrows, to put back (Lvupfx/MapLvup.c). RAM only. */
_kernel_malloc2 gEkrLvupQuoteSave, 0xF0

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

/* The regions must not run out (the kernel_malloc region would grow into the purple units' slots below it) */
.if UsedFreeRamSpaceTop < FreeRamSpaceTop
.error "The kernel_malloc region is full: it would overlap the purple units' slots"
.endif
.if UsedFreeRamSpace2Bottom > FreeRamSpace2Bottom
.error "The kernel_malloc2 region is full"
.endif
