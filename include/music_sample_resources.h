#ifndef GUARD_MUSIC_SAMPLE_RESOURCES_H
#define GUARD_MUSIC_SAMPLE_RESOURCES_H

#include "gba/m4a_internal.h"

#ifdef PORTABLE
struct WaveData *ResolveMusicSample(struct WaveData *compiledData);
#else
#define ResolveMusicSample(data) (data)
#endif

#endif // GUARD_MUSIC_SAMPLE_RESOURCES_H
