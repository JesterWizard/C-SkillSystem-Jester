    .INCLUDE "macro.inc"
    .SYNTAX UNIFIED
    .section .rodata

THUMB_FUNC_START SkillTester
SkillTester:
    ldr r3, _SkillTester
    bx r3
.global _SkillTester
.align 2, 0
_SkillTester:
    .4byte ARM_SkillTester

    .arm
    .section .rodata

.global _ARM_SkillTester_CopyStart
_ARM_SkillTester_CopyStart:
    push {r4, r5, lr}

     @ UNIT_IS_VALID
    cmp r0, #0
    beq .Lend_false
    ldr r2, [r0]
    cmp r2, #0
    beq .Lend_false

    mov r5, r0

    /**
     * Here we may not judge:
     * 1. COMMON_SKILL_VALID(sid)
     * 2. SKILL_INDEX_LIST(sid) valid
     */
    mov r2, r1
    //lsr r3, r1, #8          // r3 = sid_list
    mov r3, #0                 // Force return a 0 so that the jump table always uses the generic skill tester
    add pc, pc, r3, lsl #2

.4byte 0

    /**
     * 1. Only skill index range from 0x01~0xFE can be found in RAM list (CONFIG_TURN_ON_ALL_SKILLS expands this to 0x01-0x400)
     * 2. person/job list aims to support all 0x400 skills
     * 3. item list is limited to judge only skill index 0x300~0x400 on consideration of performance issue 
     */

.Ltesters:
    b _SkillTester_Generic
    b _SkillTester_COMMON
    b _SkillTester_COMMON
    b _SkillTester_IInfo

.Lend_true:
    mov r0, #1
    pop {r4, r5, lr}
    bx lr

.Lend_false:
    mov r0, #0
    pop {r4, r5, lr}
    bx lr

@ Used for 1023 skills in RAM
_SkillTester_Generic:
    push    {r4, r11, r12, lr}   @ Save scratch registers
    add     r1, r0, #0x32        @ r1 = &unit->supports[0]
    mov     r3, #0               @ r3 = skill slot index (0-4 for 5 slots)
    
.Lskill_loop:
    @ Calculate bit offset: r4 = r3 * 10
    mov     r4, r3
    mov     r12, #10             @ Use r12 as temp for multiply
    mul     r4, r12, r4          @ r4 = bit offset (0, 10, 20, 30, 40)
    
    @ Calculate which byte to start reading from: r12 = r4 / 8
    lsr     r12, r4, #3          @ r12 = byte offset (0, 1, 2, 3, 5)
    
    @ Calculate bit offset within the starting byte: r4 = r4 % 8
    and     r4, r4, #7           @ r4 = bit offset within byte (0, 2, 4, 6, 0)
    
    @ Load the skill value (need up to 2 bytes to get 10 bits)
    ldrb    r11, [r1, r12]       @ Load first byte
    cmp     r12, #6              @ Check if we can safely load next byte
    bge     .Lsingle_byte        @ If offset >= 6, only use single byte
    
    add     r12, r12, #1         @ r12 = next byte offset
    ldrb    r0, [r1, r12]        @ Load next byte (using r0 as temp)
    orr     r11, r11, r0, lsl #8 @ Combine into 16-bit value
    b       .Lextract_skill

.Lsingle_byte:
    @ For the last skill, we might only have partial data
    @ This handles edge cases where skill spans beyond available data
    
.Lextract_skill:
    @ Extract 10-bit skill from the combined value
    lsr     r11, r11, r4         @ Shift right by bit offset
    mov     r12, #0xFF           @ Lower 8 bits of mask
    orr     r12, r12, #0x300     @ r12 = 0x3FF (10-bit mask)
    and     r11, r11, r12        @ Extract 10-bit skill ID
    
    @ Compare with target skill
    cmp     r11, r2
    popeq   {r4, r11, r12, lr}   @ Restore registers if found
    beq     .Lend_true
    
    @ Move to next skill slot
    add     r3, r3, #1
    cmp     r3, #5               @ Check if we've processed all 5 slots
    blt     .Lskill_loop
    
    @ Not found, continue to other skill sources
    pop     {r4, r11, r12, lr}   @ Restore registers
    b       _SkillTester_COMMON
@ end of code used for 1023 skills in RAM

_SkillTester_COMMON:
_SkillTester_PInfo:
    ldr r4, .LgpConstSkillTable_Person
    ldr r0, [r5]
    // adr lr, .Lend_false
    adr lr, _SkillTester_JInfo
    ldrb r0, [r0, #4]

.L_Table:
    ldr r1, [r4]
    add r1, r1, r0, lsl #2
    ldrh r0, [r1], #2
    cmp r2, r0
    beq .Lend_true
    ldrh r0, [r1]
    cmp r2, r0
    beq .Lend_true
    mov pc, lr

_SkillTester_JInfo:
    ldr r4, .LgpConstSkillTable_Job
    ldr r0, [r5, #4]
    ldrb r0, [r0, #4]
    adr lr, .Lend_false
    b .L_Table

_SkillTester_IInfo:
    add r3, r5, #0x1E
    ldr r4, .LgpConstSkillTable_Item
    ldrb r0, [r3], #2
    bl .L_Table
    ldrb r0, [r3], #2
    bl .L_Table
    ldrb r0, [r3], #2
    bl .L_Table
    ldrb r0, [r3], #2
    bl .L_Table
    ldrb r0, [r3], #2
    // bl .L_Table

    adr lr, _SkillTester_COMMON
    // adr lr, .Lend_false
    b .L_Table

.LgpConstSkillTable_Person:
    .4byte gpConstSkillTable_Person
.LgpConstSkillTable_Job:
    .4byte gpConstSkillTable_Job
.LgpConstSkillTable_Item:
    .4byte gpConstSkillTable_Item

.LgBattleActor:
    .4byte gBattleActor

.global _ARM_SkillTester_CopyEnd
_ARM_SkillTester_CopyEnd:
