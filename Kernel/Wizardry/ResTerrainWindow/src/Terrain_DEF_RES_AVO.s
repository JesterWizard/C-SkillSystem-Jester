.thumb

.equ TerrainResBonusTable, 0x880c4fb
.equ DebugIntToString, 0x80038E1

.equ TerrainAvoBonusTable, 0x880c479

@Hooks at 0x8CA1C
TerrainDef:
    ldr   r0, =0x8013139
    mov   r12, r0
    mov   r0, #0x85 // Def terrain bonus to load
    lsl   r0, #0x1
    add   r0, r8
    mov   r1, r4
    mov   r2, r5
    mov   r3, #0x2
    bl    BXR12

TerrainAvo:
    ldr   r0, =TerrainAvoBonusTable
    add   r0, r7
    ldrb  r0, [r0]
    ldr   r1, =DebugIntToString
    mov   r12, r1
    bl    BXR12
    ldr   r0, =0x8013139
    mov   r12, r0
    mov   r0, #0xC5 // Avo terrain bonus to load
    lsl   r0, #0x1
    add   r0, r8
    mov   r1, r4
    mov   r2, r5
    mov   r3, #0x2
    bl    BXR12

TerrainRes:
    ldr   r0, =TerrainResBonusTable
    add   r0, r7
    ldrb  r0, [r0]
    ldr   r1, =DebugIntToString
    mov   r12, r1
    bl    BXR12
    ldr   r0, =0x8013139
    mov   r12, r0
    mov   r0, #0xA5 // Res terrain bonus to load
    lsl   r0, #0x1
    add   r0, r8
    mov   r1, r4
    mov   r2, r5
    mov   r3, #0x2
    bl    BXR12

    ldr   r0, =0x808CA3A|1
    bx    r0

BXR12:
    bx r12
