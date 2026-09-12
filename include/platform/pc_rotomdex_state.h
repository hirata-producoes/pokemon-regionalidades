#ifndef GUARD_PLATFORM_PC_ROTOMDEX_STATE_H
#define GUARD_PLATFORM_PC_ROTOMDEX_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PC_ROTOMDEX_STATE_SCHEMA_VERSION 1u
#define PC_ROTOMDEX_STATE_REGION_COUNT 4u
#define PC_ROTOMDEX_STATE_ENCODED_SIZE 24u

struct PcRotomDexState
{
    uint32_t unlockedRegionMask;
};

bool PcRotomDexState_Encode(const struct PcRotomDexState *state,
                            unsigned char output[PC_ROTOMDEX_STATE_ENCODED_SIZE]);
bool PcRotomDexState_Decode(const unsigned char *data, size_t size,
                            uint32_t schemaVersion,
                            struct PcRotomDexState *state);

#endif // GUARD_PLATFORM_PC_ROTOMDEX_STATE_H
