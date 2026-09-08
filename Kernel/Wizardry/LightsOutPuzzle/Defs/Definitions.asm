@ Definitions

@ Functions
.global FadeOutExists
.type   FadeOutExists, function
.set    FadeOutExists, 0x80AE2B9

.global NewFadeOut
.type   NewFadeOut, function
.set    NewFadeOut, 0x80AE2F5

.global PushToSecondaryOAM
.type   PushToSecondaryOAM, function
.set    PushToSecondaryOAM, 0x8002BB9

.global SetBackgroundTileDataOffset
.type   SetBackgroundTileDataOffset, function
.set    SetBackgroundTileDataOffset, 0x8000FDD

.global Font_InitForUIDefault
.type   Font_InitForUIDefault, function
.set    Font_InitForUIDefault, 0x8003C95

.global Text_Init
.type   Text_Init, function
.set    Text_Init, 0x8003D5D

.global Text_Clear
.type   Text_Clear, function
.set    Text_Clear, 0x8003DC9

.global Text_Draw
.type   Text_Draw, function
.set    Text_Draw, 0x8003E71

.global Text_InsertString
.type   Text_InsertString, function
.set    Text_InsertString, 0x8004481

.global GetStringFromIndex
.type   GetStringFromIndex, function
.set    GetStringFromIndex, 0x800A241

.global FilterSomeTextFromStandardBuffer
.type   FilterSomeTextFromStandardBuffer, function
.set    FilterSomeTextFromStandardBuffer, 0x800A3B9

.global UnLZ77Decompress
.type   UnLZ77Decompress, function
.set    UnLZ77Decompress, 0x8012F51

.global EnableBackgroundSyncById
.type   EnableBackgroundSyncById, function
.set    EnableBackgroundSyncById, 0x8001FBD

.global EnableBackgroundSyncByMask
.type   EnableBackgroundSyncByMask, function
.set    EnableBackgroundSyncByMask, 0x8001FAD

.global CopyToPaletteBuffer
.type   CopyToPaletteBuffer, function
.set    CopyToPaletteBuffer, 0x8000DB9

.global SetSpecialColourEffectsParam
.type   SetSpecialColourEffectsParam, function
.set    SetSpecialColourEffectsParam, 0x8001EA1

.global SetDefaultColourEffects
.type   SetDefaultColourEffects, function
.set    SetDefaultColourEffects, 0x8001F81

.global New6C
.type   New6C, function
.set    New6C, 0x8002C7D

.global NewBlocking6C
.type   NewBlocking6C, function
.set    NewBlocking6C, 0x8002CE1

.global Break6CLoop
.type   Break6CLoop, function
.set    Break6CLoop, 0x8002E95

.global GetGameClock
.type   GetGameClock, function
.set    GetGameClock, 0x8000D29

.global CpuFastSet
.type   CpuFastSet, function
.set    CpuFastSet, 0x80D1675

.global EnablePaletteSync
.type   EnablePaletteSync, function
.set    EnablePaletteSync, 0x8001F95

.global m4aSongNumStart
.type   m4aSongNumStart, function
.set    m4aSongNumStart, 0x80D01FD

.global Switch_BGM
.type   Switch_BGM, function
.set    Switch_BGM, 0x80024E5

.global Goto6CLabel
.type   Goto6CLabel, function
.set    Goto6CLabel, 0x8002F25

.global NextRN_N
.type   NextRN_N, function
.set    NextRN_N, 0x8000C81

.global BG_Fill
.type   BG_Fill, function
.set    BG_Fill, 0x8001221

.global ValniRuinsCongrats
.type   ValniRuinsCongrats function
.set    ValniRuinsCongrats, 0x8085C71

.global CallEventEngine
.type   CallEventEngine function
.set    CallEventEngine, 0x800D07D

.global GetAvailableLocaCommandAt
.type   GetAvailableLocaCommandAt function
.set    GetAvailableLocaCommandAt, 0x8084079

.global CheckFlag
.type   CheckFlag function
.set    CheckFlag, 0x8083DA9

.global SetFlag
.type   SetFlag function
.set    SetFlag, 0x8083D81

.global GetChapterEvents
.type   GetChapterEvents function
.set    GetChapterEvents, 0x80346B1

.global GetUnitStruct
.type   GetUnitStruct, function
.set    GetUnitStruct, 0x8019431

.global GetItemIndex
.type   GetItemIndex, function
.set    GetItemIndex, 0x80174ED

.global CopyTileGfxForObj
.type   CopyTileGfxForObj, function
.set    CopyTileGfxForObj, 0x8013021

.global LoadObjUIGfx
.type   LoadObjUIGfx, function
.set    LoadObjUIGfx, 0x8015681


@ RAM locations
.global Slot1,
.set		Slot1, 0x30004BC

.global Slot2,
.set		Slot2, 0x30004C0

.global Slot3,
.set		Slot3, 0x30004C4

.global Slot4,
.set		Slot4, 0x30004C8

.global SlotA,
.set		SlotA, 0x30004E0

.global SlotC,
.set		SlotC, 0x30004E8

.global gpDISPCNTbuffer
.set    gpDISPCNTbuffer, 0x3003080

.global gpBG0MapBuffer
.set    gpBG0MapBuffer, 0x2022CA8

.global gpBG1MapBuffer
.set    gpBG1MapBuffer, 0x20234A8

.global gpBG2MapBuffer
.set    gpBG2MapBuffer, 0x2023CA8

.global gpBG3MapBuffer
.set    gpBG3MapBuffer, 0x20244A8

.global gGenericBuffer
.set    gGenericBuffer, 0x2020188

.global whole_OAM_buffer
.set    whole_OAM_buffer, 0x3003140

.global LocScreenBlockBG0
.set    LocScreenBlockBG0, 0x2024CA8

.global LocScreenBlockBG1
.set    LocScreenBlockBG1, 0x2024CAC

.global LocScreenBlockBG2
.set    LocScreenBlockBG2, 0x2024CB0

.global LocScreenBlockBG3
.set    LocScreenBlockBG3, 0x2024CB4

.global KeyStatusBuffer
.set    KeyStatusBuffer, 0x2024CC0

.global ActionData
.set    ActionData, 0x203A958

.global ActiveUnit
.set    ActiveUnit, 0x3004E50

.global ChapterData
.set    ChapterData, 0x202BCF0


@ ROM locations, I'm assuming these aren't repointed. So make changes if you have repointed these!
.global OAM16x16
.set    OAM16x16, 0x8590F4C

.global OAM32x16
.set    OAM32x16, 0x8590F8C

.global RInfoOAM
.set    RInfoOAM, 0x8A00B10


@ Graphics
.global ItemPalette
.set    ItemPalette, 0x85996F4
