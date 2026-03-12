.thumb
.align

.global NewAddBreakables
.type NewAddBreakables, %function

.global NewTryAddTrapsToTargetList
.type NewTryAddTrapsToTargetList, %function

.macro blh to, reg=r3
  ldr \reg, =\to
  mov lr, \reg
  .short 0xf800
.endm

.equ AddTrap,0x802E2B8
.equ GetChapterDefinition,0x8034618
.equ gChapterData,0x202BCF0
.equ gMapSize,0x202E4D4
.equ gMapTerrain,0x202E4DC



NewAddBreakables: @r3 hook at 2E3A8
push {r4-r6,r14}

ldr r0,=gMapSize
mov r1,#2
ldsh r0,[r0,r1]
sub r5,r0,#1
cmp r5,#0
blt AddBreakables_GoBack

AddBreakables_ResetXLoop:
ldr r0,=gMapSize
mov r1,#0
ldsh r0,[r0,r1]
sub r4,r0,#1
sub r6,r5,#1
cmp r4,#0
blt AddBreakables_GoBack

AddBreakables_LoopStart:
ldr r0,=gMapTerrain
ldr r1,[r0]
lsl r0,r5,#2
add r1,r0
ldr r0,[r1]
add r0,r4
ldrb r0,[r0]

cmp r0,#0x1B @breakable wall
beq AddBreakables_MakeWall

cmp r0,#0x33 @breakable snag
beq AddBreakables_MakeSnag

cmp r0,#0x1E @door
beq AddBreakables_MakeDoor

b AddBreakables_LoopRestart


AddBreakables_MakeWall:
sub r0,r1,#4
ldr r0,[r0]
add r0,r4
ldrb r0,[r0]
cmp r0,#0x1B
beq AddBreakables_LoopRestart

ldr r0,=gChapterData
ldrb r0,[r0,#0xE] @chapter ID
blh GetChapterDefinition @returns pointer to chapter data

add r0,#0x2C 
ldrb r3,[r0] @wall HP for current chapter
mov r0,r4 @x coord
mov r1,r5 @y coord
ldr r2,=AddTrap
mov r14,r2
mov r2,#2 @trap ID
.short 0xF800

b AddBreakables_LoopRestart


AddBreakables_MakeSnag:
mov r0,r4  @x coord
mov r1,r5  @y coord
mov r2,#2  @trap ID
ldr r3,=AddTrap
mov r14,r3
mov r3,#20 @snag HP
.short 0xF800

b AddBreakables_LoopRestart


AddBreakables_MakeDoor: @doors always have 40HP in tellius as far as I can tell
mov r0,r4 @x coord
mov r1,r5 @y coord
mov r2,#2 @trap ID
ldr r3,=AddTrap
mov r14,r3
mov r3,#40 @door HP
.short 0xF800


AddBreakables_LoopRestart:
sub r4,#1
cmp r4,#0
bge AddBreakables_LoopStart

mov r5,r6
cmp r5,#0
bge AddBreakables_ResetXLoop


AddBreakables_GoBack:
pop {r4-r6}
pop {r0}
bx r0

.ltorg
.align






.equ GetTrap,0x802EB8C
.equ AddTarget,0x804F8BC
.equ gMapRange,0x202E4E4

NewTryAddTrapsToTargetList: @r3 hook at 250BC
push {r4-r6,r14}

@this is a way it just gets the start of traps but only sometimes?
@other times it just loads the trap array location directly
mov r0,#0
blh GetTrap 

mov r4,r0
ldrb r0,[r4,#2]
cmp r0,#0
beq TargetTraps_GoBack

ldr r6,=gMapTerrain
ldr r5,=gMapRange

TargetTraps_StartLoop:
cmp r0,#2
bne TargetTraps_RestartLoop

ldrb r1,[r4,#1]
ldr r0,[r6]
lsl r3,r1,#2
add r0,r3,r0
ldrb r2,[r4]
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
cmp r0,#0x1B @breakable wall
bne TargetTraps_AddIfWall2

ldr r0,[r5]
add r0,r3,r0
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
lsl r0,r0,#24
asr r0,r0,#24
cmp r0,#0
beq TargetTraps_AddIfWall2

ldrb r3,[r4,#3]
mov r0,r2
ldr r2,=AddTarget
mov r14,r2
mov r2,#0
.short 0xF800

TargetTraps_AddIfWall2:
ldrb r1,[r4,#1]
ldr r0,[r6]
lsl r3,r1,#2
add r0,r3,r0
ldrb r2,[r4]
ldr r0,[r0,#4]
add r0,r2
ldrb r0,[r0]
cmp r0,#0x1B @breakable wall 
bne TargetTraps_AddIfSnag

ldr r0,[r5]
add r0,r3,r0
ldr r0,[r0,#4]
add r0,r2
ldrb r0,[r0]
cmp r0,#0
beq TargetTraps_AddIfSnag

add r1,#1
ldrb r3,[r4,#3]
mov r0,r2
ldr r2,=AddTarget
mov r14,r2
mov r2,#0
.short 0xF800


TargetTraps_AddIfSnag:
ldrb r1,[r4,#1]
ldr r0,[r6]
lsl r3,r1,#2
add r0,r3,r0
ldrb r2,[r4]
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
cmp r0,#0x33 @breakable snag
bne TargetTraps_NewDoorCheck

ldr r0,[r5]
add r0,r3,r0
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
cmp r0,#0
beq TargetTraps_NewDoorCheck

ldrb r3,[r4,#3]
mov r0,r2
ldr r2,=AddTarget
mov r14,r2
mov r2,#0
.short 0xF800

TargetTraps_NewDoorCheck:
ldrb r1,[r4,#1]
ldr r0,[r6]
lsl r3,r1,#2
add r0,r3,r0
ldrb r2,[r4]
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
cmp r0,#0x1E @door
bne TargetTraps_RestartLoop
ldr r0,[r5]
add r0,r3,r0
ldr r0,[r0]
add r0,r2
ldrb r0,[r0]
cmp r0,#0
beq TargetTraps_RestartLoop

ldrb r3,[r4,#3]
mov r0,r2
ldr r2,=AddTarget
mov r14,r2
mov r2,#0
.short 0xF800

TargetTraps_RestartLoop:
add r4,#8
ldrb r0,[r4,#2]
cmp r0,#0
bne TargetTraps_StartLoop

TargetTraps_GoBack:
pop {r4-r6}
pop {r0}		
bx r0

.ltorg
.align



.global MakeSnagTargetAdder
.type MakeSnagTargetAdder, %function

.equ GetCharacterData,0x8019464


MakeSnagTargetAdder: @hooks at 2C910
cmp r0,#0x1B
beq MakeTarget_IsWall
cmp r0,#0x33
beq MakeTarget_IsSnag
cmp r0,#0x1E
beq MakeTarget_IsDoor
b MakeTarget_GoBack


MakeTarget_IsWall:
mov r0,#0xFE
blh GetCharacterData
str r0,[r4]
b MakeTarget_GoBack


MakeTarget_IsSnag:
mov r0,#0xFF
blh GetCharacterData
str r0,[r4]
mov r0,#0x14
strb r0,[r4,#0x12]
b MakeTarget_GoBack


MakeTarget_IsDoor:
mov r0,#0xFC @0xFD is reserved for arena opponent
blh GetCharacterData
str r0,[r4]
mov r0,#40 @door max HP
strb r0,[r4,#0x12]

MakeTarget_GoBack:
pop {r4}
pop {r0}
bx r0

.ltorg
.align





.global DoNewBreakAnim
.type DoNewBreakAnim, %function

.equ ReturnPoint_BreakAnim1,0x8081793
.equ ReturnPoint_BreakAnim2,0x80817CB

DoNewBreakAnim: @r3 hook at 81784
mov r3,#0 @we overwrote this, it's a constant
@r0 = terrain ID

cmp r0,#0x1B @breakable wall
beq DoNewBreakAnim_RetTrue

cmp r0,#0x33 @breakable snag
beq DoNewBreakAnim_RetTrue

cmp r0,#0x1E @door
bne DoNewBreakAnim_RetValue

DoNewBreakAnim_RetTrue:
mov r3,#1

DoNewBreakAnim_RetValue:
cmp r3,#0
beq DoNewBreakAnim_RetFalse
ldr r3,=ReturnPoint_BreakAnim1
b DoNewBreakAnim_GoBack

DoNewBreakAnim_RetFalse:
ldr r3,=ReturnPoint_BreakAnim2

DoNewBreakAnim_GoBack:
bx r3

.ltorg
.align



.global DoNewHideMapSprite
.type DoNewHideMapSprite, %function

.equ ReturnPoint_HideMapSprite1,0x807B37F @do hide
.equ ReturnPoint_HideMapSprite2,0x807B385 @do not hide

DoNewHideMapSprite: @r3 hook at 7B374
ldrb r0,[r0]
@r0 = terrain ID

cmp r0,#0x1B @breakable wall
beq DoNewHideMapSprite_IsTrue

cmp r0,#0x33 @breakable snag
beq DoNewHideMapSprite_IsTrue

cmp r0,#0x1E @door
bne DoNewHideMapSprite_IsFalse

DoNewHideMapSprite_IsTrue:
ldr r0,=ReturnPoint_HideMapSprite1
b DoNewHideMapSprite_GoBack

DoNewHideMapSprite_IsFalse:
ldr r0,=ReturnPoint_HideMapSprite2

DoNewHideMapSprite_GoBack:
bx r0

.ltorg
.align



.global NewForceMapAnimsForTrap
.type NewForceMapAnimsForTrap, %function

.equ ReturnPoint_ForceAnims1,0x8057C73
.equ ReturnPoint_ForceAnims2,0x8057C29

NewForceMapAnimsForTrap: @r3 hook at 57C20
@r0 = terrain ID

cmp r0,#0x1B @breakable wall
beq ForceAnims_RetTrue

cmp r0,#0x33 @breakable snag
beq ForceAnims_RetTrue

cmp r0,#0x1E @door
bne ForceAnims_RetFalse


ForceAnims_RetTrue:
ldr r0,=ReturnPoint_ForceAnims1
b ForceAnims_GoBack

ForceAnims_RetFalse:
ldr r0,=ReturnPoint_ForceAnims2

ForceAnims_GoBack:
bx r0

.ltorg
.align




.global NewTerrainHPDisplay
.type NewTerrainHPDisplay, %function

.equ ReturnPoint_TerrainWeirdCheck,0x808CAD9
.equ ReturnPoint_TerrainDisplayHP,0x808CA75
.equ ReturnPoint_TerrainNormalCase,0x808CB07

NewTerrainHPDisplay: @r3 hook at 8CA3C
@r7 = terrain ID

@weird vanilla thing on ruins and cliff terrain
cmp r7,#0x27
beq TerrainDisplay_RetWeird
cmp r7,#0x28
beq TerrainDisplay_RetWeird
cmp r7,#0x29
beq TerrainDisplay_RetWeird


@now we do our HP terrain checks

cmp r7,#0x1B @breakable wall
beq TerrainDisplay_RetHP
cmp r7,#0x33 @breakable snag
beq TerrainDisplay_RetHP
cmp r7,#0x1E @door
bne TerrainDisplay_RetNormal

TerrainDisplay_RetHP:
ldr r1,=ReturnPoint_TerrainDisplayHP
b TerrainDisplay_GoBack

TerrainDisplay_RetWeird:
ldr r1,=ReturnPoint_TerrainWeirdCheck
b TerrainDisplay_GoBack

TerrainDisplay_RetNormal:
ldr r1,=ReturnPoint_TerrainNormalCase

TerrainDisplay_GoBack:
bx r1


.ltorg
.align









