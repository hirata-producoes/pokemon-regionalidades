#ifdef PORTABLE

#include <string.h>

#include "platform/pc_rotomdex_state.h"

static const unsigned char sMagic[8] = {'P', 'G', 'R', 'D', 'E', 'X', 0, 0};

static uint32_t ReadU32(const unsigned char *data)
{
    return (uint32_t)data[0]
         | (uint32_t)data[1] << 8
         | (uint32_t)data[2] << 16
         | (uint32_t)data[3] << 24;
}

static void WriteU32(unsigned char *data, uint32_t value)
{
    data[0] = value;
    data[1] = value >> 8;
    data[2] = value >> 16;
    data[3] = value >> 24;
}

bool PcRotomDexState_Encode(const struct PcRotomDexState *state,
                            unsigned char output[PC_ROTOMDEX_STATE_ENCODED_SIZE])
{
    const uint32_t validRegionMask = (1u << PC_ROTOMDEX_STATE_REGION_COUNT) - 1u;

    if (state == NULL || output == NULL
     || (state->unlockedRegionMask & ~validRegionMask) != 0)
        return false;

    memset(output, 0, PC_ROTOMDEX_STATE_ENCODED_SIZE);
    memcpy(output, sMagic, sizeof(sMagic));
    WriteU32(output + 8, PC_ROTOMDEX_STATE_SCHEMA_VERSION);
    WriteU32(output + 12, PC_ROTOMDEX_STATE_REGION_COUNT);
    WriteU32(output + 16, state->unlockedRegionMask);
    return true;
}

bool PcRotomDexState_Decode(const unsigned char *data, size_t size,
                            uint32_t schemaVersion,
                            struct PcRotomDexState *state)
{
    const uint32_t validRegionMask = (1u << PC_ROTOMDEX_STATE_REGION_COUNT) - 1u;
    uint32_t unlockedRegionMask;

    if (data == NULL || state == NULL || size != PC_ROTOMDEX_STATE_ENCODED_SIZE
     || schemaVersion != PC_ROTOMDEX_STATE_SCHEMA_VERSION
     || memcmp(data, sMagic, sizeof(sMagic)) != 0
     || ReadU32(data + 8) != PC_ROTOMDEX_STATE_SCHEMA_VERSION
     || ReadU32(data + 12) != PC_ROTOMDEX_STATE_REGION_COUNT
     || ReadU32(data + 20) != 0)
        return false;

    unlockedRegionMask = ReadU32(data + 16);
    if ((unlockedRegionMask & ~validRegionMask) != 0)
        return false;
    state->unlockedRegionMask = unlockedRegionMask;
    return true;
}

#endif // PORTABLE
