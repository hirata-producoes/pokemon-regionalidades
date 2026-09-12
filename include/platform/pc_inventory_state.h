#ifndef GUARD_PLATFORM_PC_INVENTORY_STATE_H
#define GUARD_PLATFORM_PC_INVENTORY_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PC_INVENTORY_STATE_SCHEMA_VERSION 1u
#define PC_INVENTORY_LOCATION_COUNT 6u
#define PC_INVENTORY_MAX_SLOTS_PER_LOCATION 4096u
#define PC_INVENTORY_MAX_ITEM_ID 4095u
#define PC_INVENTORY_MAX_QUANTITY 99999u
#define PC_INVENTORY_HEADER_SIZE 32u
#define PC_INVENTORY_ENTRY_SIZE 16u
#define PC_INVENTORY_MAX_ENTRIES \
    (PC_INVENTORY_LOCATION_COUNT * PC_INVENTORY_MAX_SLOTS_PER_LOCATION)
#define PC_INVENTORY_MAX_ENCODED_SIZE \
    (PC_INVENTORY_HEADER_SIZE + PC_INVENTORY_MAX_ENTRIES * PC_INVENTORY_ENTRY_SIZE)

enum PcInventoryAuthority
{
    PC_INVENTORY_AUTHORITY_LEGACY_MIRROR,
    PC_INVENTORY_AUTHORITY_NATIVE,
};

struct PcInventorySlot
{
    uint32_t itemId;
    uint32_t quantity;
};

struct PcInventoryState
{
    uint32_t authority;
    struct PcInventorySlot slots[PC_INVENTORY_LOCATION_COUNT][PC_INVENTORY_MAX_SLOTS_PER_LOCATION];
};

bool PcInventoryState_Encode(const struct PcInventoryState *state,
                             unsigned char output[PC_INVENTORY_MAX_ENCODED_SIZE],
                             size_t *outputSize);
bool PcInventoryState_Decode(const unsigned char *data, size_t size,
                             uint32_t schemaVersion,
                             struct PcInventoryState *state);

#endif // GUARD_PLATFORM_PC_INVENTORY_STATE_H
