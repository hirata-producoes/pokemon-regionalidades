#ifndef GUARD_MP2K_COMMON_H
#define GUARD_MP2K_COMMON_H

#include "gba/types.h"

typedef s8 sf8;
typedef u8 uf8;
typedef s16 sf16;
typedef u16 uf16;

#ifndef __has_builtin
#define __has_builtin(x) defined(__GNUC__)
#endif

#if ((-1 >> 1) == -1) && __has_builtin(__builtin_ctz)
#define FLOOR_DIV_POW2(a, b) ((a) >> __builtin_ctz(b))
#else
#define FLOOR_DIV_POW2(a, b) ((a) > 0 ? (a) / (b) : (((a) + 1 - (b)) / (b)))
#endif

#define NOT_GBA_BIOS
#define NOT_GBA
#define POKEMON_EXTENSIONS

#endif
