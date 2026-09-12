#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/pc_save_container.h"
#include "platform/pc_inventory_state.h"
#include "platform/pc_rotomdex_state.h"
#include "platform/pc_world_state.h"

#define LEGACY_SIZE 131072u
#define HEADER_SIZE 40u
#define ENTRY_SIZE 48u

static unsigned int ReadU32(const unsigned char *data)
{
    return (unsigned int)data[0]
         | (unsigned int)data[1] << 8
         | (unsigned int)data[2] << 16
         | (unsigned int)data[3] << 24;
}

static void WriteU32(unsigned char *data, unsigned int value)
{
    data[0] = value;
    data[1] = value >> 8;
    data[2] = value >> 16;
    data[3] = value >> 24;
}

static void WriteU64(unsigned char *data, unsigned long long value)
{
    WriteU32(data, (unsigned int)value);
    WriteU32(data + 4, (unsigned int)(value >> 32));
}

static unsigned int Crc32(const unsigned char *data, size_t size)
{
    unsigned int crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < size; i++)
    {
        crc ^= data[i];
        for (unsigned int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

static int WriteFile(const char *path, const unsigned char *data, size_t size)
{
    FILE *file = fopen(path, "wb");
    int ok = file != NULL && fwrite(data, 1, size, file) == size;
    if (file != NULL)
        fclose(file);
    return ok;
}

static int CopyFileContents(const char *sourcePath, const char *destinationPath)
{
    FILE *source = fopen(sourcePath, "rb");
    unsigned char *data = NULL;
    long size;
    int ok = 0;

    if (source == NULL || fseek(source, 0, SEEK_END) != 0
     || (size = ftell(source)) < 0 || fseek(source, 0, SEEK_SET) != 0)
        goto cleanup;
    data = malloc(size == 0 ? 1 : (size_t)size);
    if (data == NULL || (size != 0 && fread(data, 1, (size_t)size, source) != (size_t)size))
        goto cleanup;
    ok = WriteFile(destinationPath, data, (size_t)size);

cleanup:
    free(data);
    if (source != NULL)
        fclose(source);
    return ok;
}

static int FlipLastByte(const char *path)
{
    FILE *file = fopen(path, "r+b");
    long size;
    int value;

    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0
     || fseek(file, size - 1, SEEK_SET) != 0 || (value = fgetc(file)) == EOF
     || fseek(file, size - 1, SEEK_SET) != 0 || fputc(value ^ 0xFF, file) == EOF)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int BreakWorldPayloadButRepairContainerChecksums(const char *path)
{
    FILE *file = fopen(path, "r+b");
    unsigned char *data = NULL;
    long fileSize;
    unsigned int chunkCount;
    int ok = 0;

    if (file == NULL || fseek(file, 0, SEEK_END) != 0
     || (fileSize = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0)
        goto cleanup;
    data = malloc((size_t)fileSize);
    if (data == NULL || fread(data, 1, (size_t)fileSize, file) != (size_t)fileSize)
        goto cleanup;

    chunkCount = ReadU32(data + 24);
    for (unsigned int i = 0; i < chunkCount; i++)
    {
        unsigned char *entry = data + HEADER_SIZE + i * ENTRY_SIZE;
        if (memcmp(entry, "WORLD\0\0\0", 8) == 0)
        {
            unsigned int offset = ReadU32(entry + 16);
            unsigned int size = ReadU32(entry + 24);
            data[offset] ^= 0x01;
            WriteU32(entry + 40, Crc32(data + offset, size));
            WriteU32(data + 32, Crc32(data + HEADER_SIZE, chunkCount * ENTRY_SIZE));
            WriteU32(data + 36, Crc32(data, 36));
            if (fseek(file, 0, SEEK_SET) == 0
             && fwrite(data, 1, (size_t)fileSize, file) == (size_t)fileSize)
                ok = 1;
            break;
        }
    }

cleanup:
    free(data);
    if (file != NULL)
        fclose(file);
    return ok;
}

static int AddOptionalFutureChunk(const char *path)
{
    static const unsigned char futureData[] = {0x10, 0x32, 0x54, 0x76};
    unsigned char *oldData = NULL;
    unsigned char *newData = NULL;
    FILE *file = fopen(path, "rb");
    long oldSize;
    size_t newSize;
    int ok = 0;

    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (oldSize = ftell(file)) <= 0
     || fseek(file, 0, SEEK_SET) != 0)
        goto cleanup;
    oldData = malloc((size_t)oldSize);
    newSize = (size_t)oldSize + ENTRY_SIZE + sizeof(futureData);
    newData = calloc(1, newSize);
    if (oldData == NULL || newData == NULL
     || fread(oldData, 1, (size_t)oldSize, file) != (size_t)oldSize)
        goto cleanup;
    fclose(file);
    file = NULL;

    memcpy(newData, oldData, HEADER_SIZE);
    WriteU32(newData + 24, 2);
    memcpy(newData + HEADER_SIZE, oldData + HEADER_SIZE, ENTRY_SIZE);
    WriteU64(newData + HEADER_SIZE + 16, HEADER_SIZE + 2 * ENTRY_SIZE);
    memcpy(newData + HEADER_SIZE + 2 * ENTRY_SIZE,
           oldData + HEADER_SIZE + ENTRY_SIZE, LEGACY_SIZE);

    unsigned char *futureEntry = newData + HEADER_SIZE + ENTRY_SIZE;
    memcpy(futureEntry, "FUTURE", 6);
    WriteU32(futureEntry + 8, 7);
    WriteU64(futureEntry + 16, HEADER_SIZE + 2 * ENTRY_SIZE + LEGACY_SIZE);
    WriteU64(futureEntry + 24, sizeof(futureData));
    WriteU64(futureEntry + 32, sizeof(futureData));
    WriteU32(futureEntry + 40, Crc32(futureData, sizeof(futureData)));
    memcpy(newData + HEADER_SIZE + 2 * ENTRY_SIZE + LEGACY_SIZE,
           futureData, sizeof(futureData));
    WriteU32(newData + 32, Crc32(newData + HEADER_SIZE, 2 * ENTRY_SIZE));
    WriteU32(newData + 36, Crc32(newData, 36));
    ok = WriteFile(path, newData, newSize);

cleanup:
    if (file != NULL)
        fclose(file);
    free(oldData);
    free(newData);
    return ok;
}

static int HasPreservedFutureChunk(const char *path)
{
    FILE *file = fopen(path, "rb");
    unsigned char header[HEADER_SIZE + 2 * ENTRY_SIZE];
    unsigned char payload[4];
    unsigned int offset;
    int ok = 0;

    if (file == NULL || fread(header, 1, sizeof(header), file) != sizeof(header)
     || ReadU32(header + 24) != 2
     || memcmp(header + HEADER_SIZE + ENTRY_SIZE, "FUTURE", 6) != 0)
        goto cleanup;
    offset = ReadU32(header + HEADER_SIZE + ENTRY_SIZE + 16);
    if (fseek(file, offset, SEEK_SET) == 0 && fread(payload, 1, sizeof(payload), file) == sizeof(payload)
     && payload[0] == 0x10 && payload[1] == 0x32 && payload[2] == 0x54 && payload[3] == 0x76)
        ok = 1;

cleanup:
    if (file != NULL)
        fclose(file);
    return ok;
}

int main(int argc, char **argv)
{
    unsigned char *expected;
    unsigned char *loaded;
    unsigned char worldData[PC_WORLD_STATE_MAX_ENCODED_SIZE];
    unsigned char malformedWorld[PC_WORLD_STATE_HEADER_SIZE] = {0};
    size_t worldSize;
    struct PcWorldProgressState worldState = {0};
    struct PcWorldProgressState decodedWorld;
    unsigned char rotomDexData[PC_ROTOMDEX_STATE_ENCODED_SIZE];
    struct PcRotomDexState rotomDexState = {0xFu};
    struct PcRotomDexState decodedRotomDex;
    unsigned char *inventoryData;
    struct PcInventoryState *inventoryState;
    struct PcInventoryState *decodedInventory;
    size_t inventorySize;
    const unsigned char *chunkData;
    size_t chunkSize;
    unsigned int chunkVersion;
    unsigned int chunkFlags;
    struct PcSaveLoadInfo info;
    char recoveryPath[1200];
    int result = 1;

    if (argc != 2 && argc != 3)
        return 2;
    worldState.storyEvents[2][0] = 0x1Fu;
    worldState.storyEvents[3][PC_WORLD_STATE_STORY_WORD_COUNT - 1] = 0x80000000u;
    worldState.globalRewards[0] = 0x5u;
    worldState.globalRewards[PC_WORLD_STATE_REWARD_WORD_COUNT - 1] = 0x40000000u;
    worldState.regionalRewards[2][1] = 0x80000000u;
    worldState.regionalRewards[0][PC_WORLD_STATE_REWARD_WORD_COUNT - 1] = 0x20000000u;
    if (!PcWorldState_Encode(&worldState, worldData, &worldSize)
     || !PcWorldState_Decode(worldData, worldSize,
                             PC_WORLD_STATE_SCHEMA_VERSION, &decodedWorld)
     || memcmp(&worldState, &decodedWorld, sizeof(worldState)) != 0)
        return 3;
    if (!PcRotomDexState_Encode(&rotomDexState, rotomDexData)
     || !PcRotomDexState_Decode(rotomDexData, sizeof(rotomDexData),
                                PC_ROTOMDEX_STATE_SCHEMA_VERSION, &decodedRotomDex)
     || decodedRotomDex.unlockedRegionMask != rotomDexState.unlockedRegionMask)
        return 4;
    inventoryData = malloc(PC_INVENTORY_MAX_ENCODED_SIZE);
    inventoryState = calloc(1, sizeof(*inventoryState));
    decodedInventory = malloc(sizeof(*decodedInventory));
    if (inventoryData == NULL || inventoryState == NULL || decodedInventory == NULL)
        return 5;
    inventoryState->authority = PC_INVENTORY_AUTHORITY_NATIVE;
    inventoryState->slots[0][0].itemId = 1;
    inventoryState->slots[0][0].quantity = 999;
    inventoryState->slots[0][1].itemId = 2;
    inventoryState->slots[0][1].quantity = 1000;
    inventoryState->slots[0][2].itemId = 3;
    inventoryState->slots[0][2].quantity = 99998;
    inventoryState->slots[0][3].itemId = 4;
    inventoryState->slots[0][3].quantity = 99999;
    inventoryState->slots[PC_INVENTORY_LOCATION_COUNT - 1][PC_INVENTORY_MAX_SLOTS_PER_LOCATION - 1].itemId = PC_INVENTORY_MAX_ITEM_ID;
    inventoryState->slots[PC_INVENTORY_LOCATION_COUNT - 1][PC_INVENTORY_MAX_SLOTS_PER_LOCATION - 1].quantity = 1;
    if (!PcInventoryState_Encode(inventoryState, inventoryData, &inventorySize)
     || !PcInventoryState_Decode(inventoryData, inventorySize,
                                 PC_INVENTORY_STATE_SCHEMA_VERSION, decodedInventory)
     || memcmp(inventoryState, decodedInventory, sizeof(*inventoryState)) != 0)
        return 6;
    inventoryState->slots[0][3].quantity = PC_INVENTORY_MAX_QUANTITY + 1;
    if (PcInventoryState_Encode(inventoryState, inventoryData, &inventorySize))
        return 7;
    free(inventoryData);
    free(inventoryState);
    free(decodedInventory);
    expected = malloc(LEGACY_SIZE);
    loaded = malloc(LEGACY_SIZE);
    if (expected == NULL || loaded == NULL)
        goto cleanup;
    snprintf(recoveryPath, sizeof(recoveryPath), "%s.recovery-1", argv[1]);
    remove(argv[1]);
    remove(recoveryPath);

    if (!PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.kind != PC_SAVE_LOAD_NEW)
        goto cleanup;
    for (size_t i = 0; i < LEGACY_SIZE; i++)
    {
        if (loaded[i] != 0xFF)
            goto cleanup;
        expected[i] = (unsigned char)(i * 37u + 11u);
    }

    if (!PcSaveContainer_Commit(argv[1], expected, LEGACY_SIZE, 3)
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.kind != PC_SAVE_LOAD_NATIVE || info.generation != 1
     || info.chunkCount != 1 || memcmp(expected, loaded, LEGACY_SIZE) != 0)
        goto cleanup;

    if (PcSaveContainer_SetChunk("FUTURE", 1, PC_SAVE_CHUNK_REQUIRED,
                                 worldData, sizeof(worldData))
     || PcSaveContainer_SetChunk("ROTOMDEX", 1, PC_SAVE_CHUNK_REQUIRED,
                                 worldData, worldSize)
     || PcSaveContainer_SetChunk("WORLD", 1, PC_SAVE_CHUNK_REQUIRED,
                                 malformedWorld, sizeof(malformedWorld))
     || !PcSaveContainer_SetChunk("WORLD", 1, PC_SAVE_CHUNK_REQUIRED,
                                  worldData, worldSize)
     || !PcSaveContainer_GetChunk("WORLD", &chunkData, &chunkSize,
                                  &chunkVersion, &chunkFlags)
     || chunkSize != worldSize || chunkVersion != 1
     || chunkFlags != PC_SAVE_CHUNK_REQUIRED
     || memcmp(chunkData, worldData, worldSize) != 0
     || !PcSaveContainer_Commit(argv[1], expected, LEGACY_SIZE, 3)
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.generation != 2 || info.chunkCount != 2
     || !PcSaveContainer_GetChunk("WORLD", &chunkData, &chunkSize,
                                  &chunkVersion, &chunkFlags)
     || chunkSize != worldSize
     || memcmp(chunkData, worldData, worldSize) != 0)
        goto cleanup;
    if (argc == 3 && !CopyFileContents(argv[1], argv[2]))
        goto cleanup;

    memset(expected, 0x5A, LEGACY_SIZE);
    if (!PcSaveContainer_Commit(argv[1], expected, LEGACY_SIZE, 3)
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.generation != 3 || info.chunkCount != 2
     || memcmp(expected, loaded, LEGACY_SIZE) != 0)
        goto cleanup;

    // A valid outer CRC is insufficient when a required WORLD schema is
    // malformed: the executable must reject it before gameplay starts.
    if (!BreakWorldPayloadButRepairContainerChecksums(argv[1])
     || PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info))
        goto cleanup;

    PcSaveContainer_Reset();
    if (!PcSaveContainer_Load(recoveryPath, loaded, LEGACY_SIZE, &info)
     || info.kind != PC_SAVE_LOAD_NATIVE || info.generation != 2)
        goto cleanup;

    // A raw Emerald save is accepted as an import source and converted only
    // when the next confirmed save is written.
    memset(expected, 0xA5, LEGACY_SIZE);
    if (!WriteFile(argv[1], expected, LEGACY_SIZE)
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.kind != PC_SAVE_LOAD_LEGACY
     || memcmp(expected, loaded, LEGACY_SIZE) != 0
     || !PcSaveContainer_Commit(argv[1], expected, LEGACY_SIZE, 3)
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.kind != PC_SAVE_LOAD_NATIVE)
        goto cleanup;

    if (!AddOptionalFutureChunk(argv[1])
     || !PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info)
     || info.chunkCount != 2)
        goto cleanup;
    loaded[12345] ^= 0x7F;
    if (!PcSaveContainer_Commit(argv[1], loaded, LEGACY_SIZE, 3)
     || !HasPreservedFutureChunk(argv[1]))
        goto cleanup;

    if (!FlipLastByte(argv[1])
     || PcSaveContainer_Load(argv[1], loaded, LEGACY_SIZE, &info))
        goto cleanup;

    result = 0;
    puts("PC_SAVE_CONTAINER_TEST_OK");

cleanup:
    PcSaveContainer_Reset();
    remove(argv[1]);
    remove(recoveryPath);
    free(expected);
    free(loaded);
    return result;
}
