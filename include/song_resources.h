#ifndef GUARD_SONG_RESOURCES_H
#define GUARD_SONG_RESOURCES_H

#include "gba/m4a_internal.h"

#ifdef PORTABLE
struct SongHeader *ResolveSongHeader(struct SongHeader *compiledData);
#else
#define ResolveSongHeader(data) (data)
#endif

#endif // GUARD_SONG_RESOURCES_H
