.macro dat value, name
    .global \name
    .type \name, object
    .set \name, \value
.endm

/* EWRAM state for the conversation log (TEXT_ENGINE_CHATLOG), 0x600 bytes
 * in all, separate from the text engine's own block in RamAlloc.s.
 *
 * Default placement: FreeRamSpace2 on clean FE8U (0x0203AAA4 .. 0x0203DDE0),
 * after the text engine's 0x0203AB00 .. 0x0203AC9F.
 * Occupies 0x0203B000 .. 0x0203B5FF.
 *
 * Change the base below and run make if another hack already uses this range.
 * Keep each symbol 4-byte aligned.
 */
.set ChatlogRamBase, 0x0203B000

/* Ring buffer of logged lines: struct ChatLogState (0x444) */
dat ChatlogRamBase + 0x000, sChatLogState
/* Chapter the log belongs to: struct ChatLogStamp (8) */
dat ChatlogRamBase + 0x444, sChatLogStamp
/* Drawing scratch while the log is open: struct ChatlogUiState (0x1AC, up to 0x1B4) */
dat ChatlogRamBase + 0x44C, sChatlogUiState
