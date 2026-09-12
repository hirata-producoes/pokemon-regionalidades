#include <string.h>

#include "platform/pc_world_state.h"

static const unsigned char sWorldMagic[8] = {'P', 'G', 'R', 'W', 'O', 'R', 'L', 'D'};

enum PcWorldEntryType
{
    PC_WORLD_ENTRY_STORY_EVENT = 1,
    PC_WORLD_ENTRY_GLOBAL_REWARD = 2,
    PC_WORLD_ENTRY_REGIONAL_REWARD = 3,
};

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

bool PcWorldState_Encode(const struct PcWorldProgressState *state,
                         unsigned char output[PC_WORLD_STATE_MAX_ENCODED_SIZE],
                         size_t *outputSize)
{
    size_t offset = PC_WORLD_STATE_HEADER_SIZE;
    uint32_t entryCount = 0;

    if (state == NULL || output == NULL || outputSize == NULL)
        return false;

    memset(output, 0, PC_WORLD_STATE_MAX_ENCODED_SIZE);
    memcpy(output, sWorldMagic, sizeof(sWorldMagic));
    WriteU32(output + 8, PC_WORLD_STATE_SCHEMA_VERSION);
    WriteU32(output + 16, PC_WORLD_STATE_ENTRY_SIZE);
    WriteU32(output + 20, PC_WORLD_STATE_REGION_COUNT);

    for (size_t region = 0; region < PC_WORLD_STATE_REGION_COUNT; region++)
    {
        for (size_t word = 0; word < PC_WORLD_STATE_STORY_WORD_COUNT; word++)
        {
            for (size_t bit = 0; bit < 32; bit++)
            {
                if ((state->storyEvents[region][word] & (1u << bit)) == 0)
                    continue;
                output[offset] = PC_WORLD_ENTRY_STORY_EVENT;
                output[offset + 1] = (unsigned char)region;
                WriteU32(output + offset + 4, (uint32_t)(word * 32 + bit));
                offset += PC_WORLD_STATE_ENTRY_SIZE;
                entryCount++;
            }
        }
    }
    for (size_t word = 0; word < PC_WORLD_STATE_REWARD_WORD_COUNT; word++)
    {
        for (size_t bit = 0; bit < 32; bit++)
        {
            if ((state->globalRewards[word] & (1u << bit)) == 0)
                continue;
            output[offset] = PC_WORLD_ENTRY_GLOBAL_REWARD;
            output[offset + 1] = 0xFF;
            WriteU32(output + offset + 4, (uint32_t)(word * 32 + bit));
            offset += PC_WORLD_STATE_ENTRY_SIZE;
            entryCount++;
        }
    }
    for (size_t region = 0; region < PC_WORLD_STATE_REGION_COUNT; region++)
    {
        for (size_t word = 0; word < PC_WORLD_STATE_REWARD_WORD_COUNT; word++)
        {
            for (size_t bit = 0; bit < 32; bit++)
            {
                if ((state->regionalRewards[region][word] & (1u << bit)) == 0)
                    continue;
                output[offset] = PC_WORLD_ENTRY_REGIONAL_REWARD;
                output[offset + 1] = (unsigned char)region;
                WriteU32(output + offset + 4, (uint32_t)(word * 32 + bit));
                offset += PC_WORLD_STATE_ENTRY_SIZE;
                entryCount++;
            }
        }
    }
    WriteU32(output + 12, entryCount);
    *outputSize = offset;
    return offset <= PC_WORLD_STATE_MAX_ENCODED_SIZE;
}

bool PcWorldState_Decode(const unsigned char *data, size_t size,
                         uint32_t schemaVersion,
                         struct PcWorldProgressState *state)
{
    uint32_t entryCount;
    size_t offset = PC_WORLD_STATE_HEADER_SIZE;

    if (data == NULL || state == NULL || size < PC_WORLD_STATE_HEADER_SIZE
     || schemaVersion != PC_WORLD_STATE_SCHEMA_VERSION
     || memcmp(data, sWorldMagic, sizeof(sWorldMagic)) != 0
     || ReadU32(data + 8) != PC_WORLD_STATE_SCHEMA_VERSION
     || ReadU32(data + 16) != PC_WORLD_STATE_ENTRY_SIZE
     || ReadU32(data + 20) != PC_WORLD_STATE_REGION_COUNT
     || ReadU32(data + 24) != 0 || ReadU32(data + 28) != 0)
        return false;

    entryCount = ReadU32(data + 12);
    if (entryCount > PC_WORLD_STATE_MAX_ENTRIES
     || size != PC_WORLD_STATE_HEADER_SIZE + (size_t)entryCount * PC_WORLD_STATE_ENTRY_SIZE)
        return false;

    memset(state, 0, sizeof(*state));
    for (uint32_t entryIndex = 0; entryIndex < entryCount; entryIndex++)
    {
        uint32_t type = data[offset];
        uint32_t region = data[offset + 1];
        uint32_t id = ReadU32(data + offset + 4);
        uint32_t *word;
        uint32_t mask;

        if (data[offset + 2] != 0 || data[offset + 3] != 0)
            return false;
        if (type == PC_WORLD_ENTRY_STORY_EVENT
         && region < PC_WORLD_STATE_REGION_COUNT
         && id < PC_WORLD_STATE_STORY_WORD_COUNT * 32u)
            word = &state->storyEvents[region][id / 32];
        else if (type == PC_WORLD_ENTRY_GLOBAL_REWARD
              && region == 0xFF
              && id < PC_WORLD_STATE_REWARD_WORD_COUNT * 32u)
            word = &state->globalRewards[id / 32];
        else if (type == PC_WORLD_ENTRY_REGIONAL_REWARD
              && region < PC_WORLD_STATE_REGION_COUNT
              && id < PC_WORLD_STATE_REWARD_WORD_COUNT * 32u)
            word = &state->regionalRewards[region][id / 32];
        else
            return false;

        mask = 1u << (id % 32);
        if ((*word & mask) != 0)
            return false;
        *word |= mask;
        offset += PC_WORLD_STATE_ENTRY_SIZE;
    }
    return offset == size;
}
