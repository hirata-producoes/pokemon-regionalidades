#ifndef GUARD_VOICEGROUP_RESOURCES_H
#define GUARD_VOICEGROUP_RESOURCES_H

#include "gba/m4a_internal.h"

#ifdef PORTABLE
struct ToneData *ResolveVoicegroupByHash(u64 symbolHash);
#endif

#endif // GUARD_VOICEGROUP_RESOURCES_H
