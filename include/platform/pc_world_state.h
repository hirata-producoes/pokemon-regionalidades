#ifndef GUARD_PLATFORM_PC_WORLD_STATE_H
#define GUARD_PLATFORM_PC_WORLD_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PC_WORLD_STATE_SCHEMA_VERSION 1u
#define PC_WORLD_STATE_REGION_COUNT 4u
// Native capacity is deliberately independent from SaveBlock3. Records are
// sparse on disk, so unused capacity costs only a few KiB while running and no
// bytes in the campaign file.
#define PC_WORLD_STATE_STORY_WORD_COUNT 128u
#define PC_WORLD_STATE_REWARD_WORD_COUNT 32u
#define PC_WORLD_STATE_HEADER_SIZE 32u
#define PC_WORLD_STATE_ENTRY_SIZE 8u
#define PC_WORLD_STATE_MAX_ENTRIES \
    (PC_WORLD_STATE_REGION_COUNT * PC_WORLD_STATE_STORY_WORD_COUNT * 32u \
   + PC_WORLD_STATE_REWARD_WORD_COUNT * 32u \
   + PC_WORLD_STATE_REGION_COUNT * PC_WORLD_STATE_REWARD_WORD_COUNT * 32u)
#define PC_WORLD_STATE_MAX_ENCODED_SIZE \
    (PC_WORLD_STATE_HEADER_SIZE + PC_WORLD_STATE_MAX_ENTRIES * PC_WORLD_STATE_ENTRY_SIZE)

// This is the host-side representation of the first WORLD schema. The file
// representation is encoded explicitly, so compiler padding and host byte
// order can never change an existing campaign.
struct PcWorldProgressState
{
    uint32_t storyEvents[PC_WORLD_STATE_REGION_COUNT][PC_WORLD_STATE_STORY_WORD_COUNT];
    uint32_t globalRewards[PC_WORLD_STATE_REWARD_WORD_COUNT];
    uint32_t regionalRewards[PC_WORLD_STATE_REGION_COUNT][PC_WORLD_STATE_REWARD_WORD_COUNT];
};

bool PcWorldState_Encode(const struct PcWorldProgressState *state,
                         unsigned char output[PC_WORLD_STATE_MAX_ENCODED_SIZE],
                         size_t *outputSize);
bool PcWorldState_Decode(const unsigned char *data, size_t size,
                         uint32_t schemaVersion,
                         struct PcWorldProgressState *state);

#endif // GUARD_PLATFORM_PC_WORLD_STATE_H
