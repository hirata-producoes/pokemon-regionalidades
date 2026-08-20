#ifndef GUARD_TILESET_ANIM_RESOURCES_H
#define GUARD_TILESET_ANIM_RESOURCES_H

#include "gba/types.h"

#ifdef PORTABLE
const u16 *ResolveTilesetAnimFrame(const u16 *compiledData, u16 requiredSize);
#else
#define ResolveTilesetAnimFrame(data, requiredSize) (data)
#endif

#endif // GUARD_TILESET_ANIM_RESOURCES_H
