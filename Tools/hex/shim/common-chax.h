/*
 * Host stand-in for the repo's common-chax.h (the GBA types only). Tools/hex/run_hex_tests.sh puts this directory first on
 * the include path, so Kernel/Wizardry/Minigames/Hex/Source/HexCore.c and include/kernel/hex-core.h build on the host.
 */
#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint32_t bool32;
