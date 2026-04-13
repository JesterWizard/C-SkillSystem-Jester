#pragma once

enum {
	SNEK_STATE_X,
	SNEK_STATE_Y,
	SNEK_STATE_DIR,
	SNEK_STATE_TIMER,
};

enum {
	SNEK_DIR_LEFT,
	SNEK_DIR_RIGHT,
	SNEK_DIR_UP,
	SNEK_DIR_DOWN,
};

extern const u8 Pal_SnekPressStart[];
extern const u8 Gfx_SnekPressStart[];

extern const u8 Pal_SnekGameSheet[];
extern const u8 Gfx_SnekGameSheet[];

extern const u8 Gfx_TopBarBackground[];
extern const u8 Gfx_SnekGameBackground[];
extern const u8 Pal_SnekGameBackground[];
extern const u8 Tsa_SnekGameBackground[];
extern u16 gSnekCoinPresent;
extern u16 gSnekCoinCoordinates[1]; // Packed x and y coordinates for the coin, with x in the upper byte and y in the lower byte
extern u8 gSnekSnakeState[4]; // Packed snake state: x, y, direction, frame counter
extern s16 gSnekSnakeX;
extern s16 gSnekSnakeY;
extern u16 gSnekCurrentScore;
extern u16 gSnekHighScore;
extern u16 gSnekHighScoreMagic;
extern u8 gSnekSnakeBodyX[32];
extern u8 gSnekSnakeBodyY[32];
extern u16 gSnekSnakeBodyLength;
extern u16 gSnekLastTravelDirection;
extern u16 gSnekLastPressedDirection;

struct EventEngineProc;

void CallSnekMinigameASMC(struct EventEngineProc * proc);
