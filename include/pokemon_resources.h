#ifndef GUARD_POKEMON_RESOURCES_H
#define GUARD_POKEMON_RESOURCES_H

#include "gba/types.h"

#ifdef PORTABLE
void *LoadExternalPokemonPic(const void *compiledData, u64 *sizeOut);
#endif

#endif // GUARD_POKEMON_RESOURCES_H
