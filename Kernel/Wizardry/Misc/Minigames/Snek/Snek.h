#pragma once

extern const u8 Pal_SnekPressStart[];
extern const u8 Gfx_SnekPressStart_1[];
extern const u8 Gfx_SnekPressStart_2[];
extern const u8 Gfx_SnekPressStart_3[];
extern const u8 Gfx_SnekPressStart_4[];

extern const u8 Pal_SnekGameSheet[];
extern const u8 Gfx_SnekGameSheet_1[];
extern const u8 Gfx_SnekGameSheet_2[];
extern const u8 Gfx_SnekGameSheet_3[];
extern const u8 Gfx_SnekGameSheet_4[];
extern const u8 Gfx_SnekGameSheet_5[];

extern const u8 Gfx_TopBarBackground[];
extern const u8 Gfx_SnekGameBackground[];
extern const u8 Pal_SnekGameBackground[];
extern const u8 Tsa_SnekGameBackground[];
extern u16 gSnekCoinPresent;
extern u16 gSnekCoinCoordinates[1]; // Packed x and y coordinates for the coin, with x in the upper byte and y in the lower byte

struct EventEngineProc;

void CallSnekMinigameASMC(struct EventEngineProc * proc);
