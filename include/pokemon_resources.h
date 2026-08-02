#ifndef GUARD_POKEMON_RESOURCES_H
#define GUARD_POKEMON_RESOURCES_H

#include "gba/types.h"

#ifdef PORTABLE
void *LoadExternalPokemonPic(const void *compiledData, u64 *sizeOut);
const u16 *GetExternalPokemonPalette(const void *compiledData);
#endif

#endif // GUARD_POKEMON_RESOURCES_H
