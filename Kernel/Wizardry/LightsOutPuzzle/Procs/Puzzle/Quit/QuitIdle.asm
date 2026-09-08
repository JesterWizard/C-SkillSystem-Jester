@ Ends Quit screen or quits if certain input is given.
.thumb

push  {r4-r5, r14}
mov   r5, r0


@ Draw sprites
bl    QuitLoopSprites


@ Check input, A
ldr   r1, =KeyStatusBuffer
ldrh  r4, [r1, #0x8]
mov   r0, #0x1
tst   r0, r4
beq   CheckB

  @ Quit
  mov   r0, r5
  ldr   r4, =Break6CLoop
  bl    GOTO_R4
  ldr   r0, [r5, #0x14]
  mov   r1, #0x0
  ldr   r4, =Goto6CLabel                  @ Go beyond victory message
  bl    GOTO_R4
  
  @ Return 0 to indicate no reward
  ldr   r0, =SlotC
  mov   r1, #0x0
  str   r1, [r0]
  
  @ Fade out
  mov   r0, #0x10
  ldr   r4, =NewFadeOut
  bl    GOTO_R4
  
  @ Play sfx
  mov   r0, #0x59                         @ YUGIOH place card sfx
  bl    PlaySFX
  
  @ Fade out music
  mov   r0, #0x7F
  lsl   r0, #0x8
  add   r0, #0xFF                         @ SongID, silence
  mov   r1, #0x5                          @ Fadeout & Fadein rate?
  mov   r2, #0x0                          @ No idea
  ldr   r4, =Switch_BGM
  bl    GOTO_R4
  
  b     Return

    CheckB:
    mov   r0, #0x2
    tst   r0, r4
    beq   Return
    
      @ Return to puzzle
      mov   r0, r5
      ldr   r4, =Break6CLoop
      bl    GOTO_R4
      
      @ Play sfx
      mov   r0, #0x59                 @ YUGIOH place card sfx
      bl    PlaySFX


Return:
pop   {r4-r5}
pop   {r0}
bx    r0
GOTO_R4:
bx    r4
