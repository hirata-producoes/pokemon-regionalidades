#ifdef PORTABLE

#include <string.h>

#include "platform/pc_inventory_state.h"

static const unsigned char sMagic[8] = {'P', 'G', 'R', 'I', 'N', 'V', 0, 0};

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

bool PcInventoryState_Encode(const struct PcInventoryState *state,
                             unsigned char output[PC_INVENTORY_MAX_ENCODED_SIZE],
                             size_t *outputSize)
{
    uint32_t entryCount = 0;
    size_t offset = PC_INVENTORY_HEADER_SIZE;

    if (state == NULL || output == NULL || outputSize == NULL)
        return false;
    if (state->authority > PC_INVENTORY_AUTHORITY_NATIVE)
        return false;

    memset(output, 0, PC_INVENTORY_HEADER_SIZE);
    for (uint32_t location = 0; location < PC_INVENTORY_LOCATION_COUNT; location++)
    {
        for (uint32_t slot = 0; slot < PC_INVENTORY_MAX_SLOTS_PER_LOCATION; slot++)
        {
            const struct PcInventorySlot *entry = &state->slots[location][slot];

            if (entry->itemId == 0 && entry->quantity == 0)
                continue;
            if (entry->itemId == 0 || entry->itemId > PC_INVENTORY_MAX_ITEM_ID
             || entry->quantity == 0 || entry->quantity > PC_INVENTORY_MAX_QUANTITY)
                return false;
            output[offset] = (unsigned char)location;
            WriteU32(output + offset + 4, slot);
            WriteU32(output + offset + 8, entry->itemId);
            WriteU32(output + offset + 12, entry->quantity);
            offset += PC_INVENTORY_ENTRY_SIZE;
            entryCount++;
        }
    }

    memcpy(output, sMagic, sizeof(sMagic));
    WriteU32(output + 8, PC_INVENTORY_STATE_SCHEMA_VERSION);
    WriteU32(output + 12, entryCount);
    WriteU32(output + 16, PC_INVENTORY_ENTRY_SIZE);
    WriteU32(output + 20, PC_INVENTORY_LOCATION_COUNT);
    WriteU32(output + 24, PC_INVENTORY_MAX_SLOTS_PER_LOCATION);
    WriteU32(output + 28, state->authority);
    *outputSize = offset;
    return true;
}

bool PcInventoryState_Decode(const unsigned char *data, size_t size,
                             uint32_t schemaVersion,
                             struct PcInventoryState *state)
{
    uint32_t entryCount;

    if (data == NULL || state == NULL || size < PC_INVENTORY_HEADER_SIZE
     || schemaVersion != PC_INVENTORY_STATE_SCHEMA_VERSION
     || memcmp(data, sMagic, sizeof(sMagic)) != 0
     || ReadU32(data + 8) != PC_INVENTORY_STATE_SCHEMA_VERSION
     || ReadU32(data + 16) != PC_INVENTORY_ENTRY_SIZE
     || ReadU32(data + 20) != PC_INVENTORY_LOCATION_COUNT
     || ReadU32(data + 24) != PC_INVENTORY_MAX_SLOTS_PER_LOCATION
     || ReadU32(data + 28) > PC_INVENTORY_AUTHORITY_NATIVE)
        return false;

    entryCount = ReadU32(data + 12);
    if (entryCount > PC_INVENTORY_MAX_ENTRIES
     || size != PC_INVENTORY_HEADER_SIZE + (size_t)entryCount * PC_INVENTORY_ENTRY_SIZE)
        return false;

    memset(state, 0, sizeof(*state));
    state->authority = ReadU32(data + 28);
    for (uint32_t i = 0; i < entryCount; i++)
    {
        size_t offset = PC_INVENTORY_HEADER_SIZE + (size_t)i * PC_INVENTORY_ENTRY_SIZE;
        uint32_t location = data[offset];
        uint32_t slot = ReadU32(data + offset + 4);
        uint32_t itemId = ReadU32(data + offset + 8);
        uint32_t quantity = ReadU32(data + offset + 12);

        if (data[offset + 1] != 0 || data[offset + 2] != 0 || data[offset + 3] != 0
         || location >= PC_INVENTORY_LOCATION_COUNT
         || slot >= PC_INVENTORY_MAX_SLOTS_PER_LOCATION
         || itemId == 0 || itemId > PC_INVENTORY_MAX_ITEM_ID
         || quantity == 0 || quantity > PC_INVENTORY_MAX_QUANTITY
         || state->slots[location][slot].itemId != 0)
            return false;
        state->slots[location][slot].itemId = itemId;
        state->slots[location][slot].quantity = quantity;
    }
    return true;
}

#endif // PORTABLE
