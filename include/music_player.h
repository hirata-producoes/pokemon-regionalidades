#ifndef GUARD_NATIVE_MUSIC_PLAYER_H
#define GUARD_NATIVE_MUSIC_PLAYER_H

#include "gba/m4a_internal.h"

// The native mixer was written with descriptive MP2K type names. The
// Expansion keeps the original matching-layout names, so use aliases rather
// than maintaining a second copy of every live player and track structure.
#define WaveData2 WaveData
#define MP2KInstrument ToneData
#define MP2KTrack MusicPlayerTrack
#define MP2KPlayerState MusicPlayerInfo
#define MP2KSongHeader SongHeader

#define PLAYER_UNLOCKED ID_NUMBER
#define PLAYER_LOCKED (PLAYER_UNLOCKED + 1)

void clear_modM(struct MusicPlayerInfo *unused, struct MusicPlayerTrack *track);

#endif
