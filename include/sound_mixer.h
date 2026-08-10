#ifndef GUARD_NATIVE_SOUND_MIXER_H
#define GUARD_NATIVE_SOUND_MIXER_H

#include "music_player.h"

#define MixerSource SoundChannel
#define SoundMixerState SoundInfo

#define MIXER_UNLOCKED ID_NUMBER
#define MIXER_LOCKED (MIXER_UNLOCKED + 1)
#define MIXED_AUDIO_BUFFER_SIZE 4907

void RunMixerFrame(void);

#endif
