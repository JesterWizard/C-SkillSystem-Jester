.thumb

@ r0 = palette from GetStatScreenBgPal, then resume vanilla CopyToPaletteBuffer

ldr  r3, GetPal
mov  lr, r3
.short 0xF800

mov  r1, #0xC0
lsl  r1, #1
mov  r2, #0x80
ldr  r3, =0x80885a4|1
bx   r3

.align
.ltorg

GetPal:
@POIN GetStatScreenBgPal
