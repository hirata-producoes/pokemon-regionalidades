#ifndef GUARD_TILESET_RESOURCES_H
#define GUARD_TILESET_RESOURCES_H

#include "gba/types.h"

#ifdef PORTABLE
const u32 *ResolveTilesetTiles(const u32 *compiledData);
const u16 (*ResolveTilesetPalettes(const u16 (*compiledData)[16]))[16];
const u16 *ResolveTilesetMetatiles(const u16 *compiledData);
const u16 *ResolveTilesetAttributes(const u16 *compiledData);
#else
#define ResolveTilesetTiles(data) (data)
#define ResolveTilesetPalettes(data) (data)
#define ResolveTilesetMetatiles(data) (data)
#define ResolveTilesetAttributes(data) (data)
#endif

#endif // GUARD_TILESET_RESOURCES_H
