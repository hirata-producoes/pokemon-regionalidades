#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/pc_save_container.h"
#include "platform/pc_inventory_state.h"
#include "platform/pc_rotomdex_state.h"
#include "platform/save_file.h"
#include "platform/pc_world_state.h"

#define HEADER_SIZE 40u
#define DIRECTORY_ENTRY_SIZE 48u
static const unsigned char sContainerMagic[8] = {'P', 'G', 'R', 'S', 'A', 'V', 'E', 0};
static const unsigned char sLegacyChunkId[8] = {'L', 'E', 'G', 'A', 'C', 'Y', 0, 0};
static const unsigned char sKnownChunkIds[][8] =
{
    {'L', 'E', 'G', 'A', 'C', 'Y', 0, 0},
    {'W', 'O', 'R', 'L', 'D', 0, 0, 0},
    {'R', 'O', 'T', 'O', 'M', 'D', 'E', 'X'},
    {'I', 'N', 'V', 'E', 'N', 'T', 0, 0},
    {'G', 'E', 'A', 'R', 0, 0, 0, 0},
    {'T', 'I', 'M', 'E', 0, 0, 0, 0},
};

struct PendingChunk
{
    bool used;
    unsigned char id[8];
    uint32_t schemaVersion;
    uint32_t flags;
    unsigned char *data;
    size_t size;
};

static unsigned char *sLoadedContainer;
static uint64_t sLoadedGeneration;
static struct PendingChunk sPendingChunks[PC_SAVE_CONTAINER_MAX_CHUNKS];

static uint32_t ReadU32(const unsigned char *data)
{
    return (uint32_t)data[0]
         | (uint32_t)data[1] << 8
         | (uint32_t)data[2] << 16
         | (uint32_t)data[3] << 24;
}

static uint64_t ReadU64(const unsigned char *data)
{
    return (uint64_t)ReadU32(data) | (uint64_t)ReadU32(data + 4) << 32;
}

static void WriteU32(unsigned char *data, uint32_t value)
{
    data[0] = value;
    data[1] = value >> 8;
    data[2] = value >> 16;
    data[3] = value >> 24;
}

static void WriteU64(unsigned char *data, uint64_t value)
{
    WriteU32(data, (uint32_t)value);
    WriteU32(data + 4, (uint32_t)(value >> 32));
}

static uint32_t CalculateCrc32(const unsigned char *data, size_t size)
{
    uint32_t crc = 0xFFFFFFFFu;

    for (size_t i = 0; i < size; i++)
    {
        crc ^= data[i];
        for (unsigned int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

enum FileReadResult
{
    FILE_READ_OK,
    FILE_READ_MISSING,
    FILE_READ_ERROR,
};

static enum FileReadResult ReadWholeFile(const char *path, unsigned char **data, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long fileSize;
    unsigned char *buffer;

    if (file == NULL)
        return errno == ENOENT ? FILE_READ_MISSING : FILE_READ_ERROR;
    if (fseek(file, 0, SEEK_END) != 0
     || (fileSize = ftell(file)) < 0
     || (uint64_t)fileSize > PC_SAVE_CONTAINER_MAX_SIZE
     || fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return FILE_READ_ERROR;
    }

    buffer = malloc(fileSize == 0 ? 1 : (size_t)fileSize);
    if (buffer == NULL)
    {
        fclose(file);
        return FILE_READ_ERROR;
    }
    if (fileSize != 0 && fread(buffer, 1, (size_t)fileSize, file) != (size_t)fileSize)
    {
        free(buffer);
        fclose(file);
        return FILE_READ_ERROR;
    }
    fclose(file);
    *data = buffer;
    *size = (size_t)fileSize;
    return FILE_READ_OK;
}

static bool IsKnownChunk(const unsigned char *id)
{
    for (size_t i = 0; i < sizeof(sKnownChunkIds) / sizeof(sKnownChunkIds[0]); i++)
    {
        if (memcmp(id, sKnownChunkIds[i], 8) == 0)
            return true;
    }
    return false;
}

static bool IsSupportedRequiredChunk(const unsigned char *id,
                                     uint32_t schemaVersion,
                                     const unsigned char *payload,
                                     size_t payloadSize)
{
    if (memcmp(id, sLegacyChunkId, sizeof(sLegacyChunkId)) == 0)
        return schemaVersion == 1;
    if (memcmp(id, "WORLD\0\0\0", 8) == 0)
    {
        struct PcWorldProgressState state;
        return PcWorldState_Decode(payload, payloadSize, schemaVersion, &state);
    }
    if (memcmp(id, "ROTOMDEX", 8) == 0)
    {
        struct PcRotomDexState state;
        return PcRotomDexState_Decode(payload, payloadSize, schemaVersion, &state);
    }
    if (memcmp(id, "INVENT\0\0", 8) == 0)
    {
        struct PcInventoryState *state = malloc(sizeof(*state));
        bool valid = state != NULL
                  && PcInventoryState_Decode(payload, payloadSize, schemaVersion, state);
        free(state);
        return valid;
    }

    // Reserved identifiers become required only when this executable gains
    // their schema implementation. Until then they may exist as optional
    // forward-compatible data and will be preserved byte-for-byte.
    return false;
}

static bool NormalizeChunkId(const char *source, unsigned char destination[8])
{
    size_t length;

    if (source == NULL || source[0] == '\0')
        return false;
    length = strlen(source);
    if (length > 8)
        return false;
    memset(destination, 0, 8);
    memcpy(destination, source, length);
    return true;
}

static void ClearPendingChunks(void)
{
    for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
    {
        free(sPendingChunks[i].data);
        memset(&sPendingChunks[i], 0, sizeof(sPendingChunks[i]));
    }
}

static bool ValidateContainer(const unsigned char *fileData, size_t fileSize,
                              size_t legacySize, size_t *legacyOffset,
                              uint64_t *generation, uint32_t *chunkCount)
{
    uint32_t version;
    uint32_t headerSize;
    uint32_t count;
    uint32_t entrySize;
    size_t directorySize;
    size_t directoryEnd;
    bool foundLegacy = false;

    if (fileSize < HEADER_SIZE || memcmp(fileData, sContainerMagic, sizeof(sContainerMagic)) != 0)
        return false;

    version = ReadU32(fileData + 8);
    headerSize = ReadU32(fileData + 12);
    count = ReadU32(fileData + 24);
    entrySize = ReadU32(fileData + 28);
    if (version != PC_SAVE_CONTAINER_VERSION || headerSize != HEADER_SIZE
     || entrySize != DIRECTORY_ENTRY_SIZE || count == 0
     || count > PC_SAVE_CONTAINER_MAX_CHUNKS)
        return false;
    if (ReadU32(fileData + 36) != CalculateCrc32(fileData, 36))
        return false;

    directorySize = (size_t)count * DIRECTORY_ENTRY_SIZE;
    if (directorySize > fileSize - HEADER_SIZE)
        return false;
    directoryEnd = HEADER_SIZE + directorySize;
    if (ReadU32(fileData + 32) != CalculateCrc32(fileData + HEADER_SIZE, directorySize))
        return false;

    for (uint32_t i = 0; i < count; i++)
    {
        const unsigned char *entry = fileData + HEADER_SIZE + i * DIRECTORY_ENTRY_SIZE;
        uint32_t schemaVersion = ReadU32(entry + 8);
        uint32_t flags = ReadU32(entry + 12);
        uint64_t offset = ReadU64(entry + 16);
        uint64_t storedSize = ReadU64(entry + 24);
        uint64_t logicalSize = ReadU64(entry + 32);
        uint32_t expectedCrc = ReadU32(entry + 40);

        if ((flags & ~PC_SAVE_CHUNK_REQUIRED) != 0
         || offset < directoryEnd || offset > fileSize || storedSize > fileSize - (size_t)offset
         || logicalSize != storedSize
         || expectedCrc != CalculateCrc32(fileData + (size_t)offset, (size_t)storedSize))
            return false;

        if (memcmp(entry, sLegacyChunkId, sizeof(sLegacyChunkId)) == 0)
        {
            if (foundLegacy || schemaVersion != 1 || storedSize != legacySize)
                return false;
            foundLegacy = true;
            *legacyOffset = (size_t)offset;
        }
        else if ((flags & PC_SAVE_CHUNK_REQUIRED) != 0
              && (!IsKnownChunk(entry)
               || !IsSupportedRequiredChunk(entry, schemaVersion,
                                             fileData + (size_t)offset,
                                             (size_t)storedSize)))
        {
            return false;
        }

        for (uint32_t previous = 0; previous < i; previous++)
        {
            const unsigned char *previousEntry = fileData + HEADER_SIZE + previous * DIRECTORY_ENTRY_SIZE;
            uint64_t previousOffset = ReadU64(previousEntry + 16);
            uint64_t previousSize = ReadU64(previousEntry + 24);
            uint64_t end = offset + storedSize;
            uint64_t previousEnd = previousOffset + previousSize;

            if (memcmp(entry, previousEntry, 8) == 0
             || (storedSize != 0 && previousSize != 0
              && offset < previousEnd && previousOffset < end))
                return false;
        }
    }

    if (!foundLegacy)
        return false;
    *generation = ReadU64(fileData + 16);
    *chunkCount = count;
    return true;
}

void PcSaveContainer_Reset(void)
{
    free(sLoadedContainer);
    sLoadedContainer = NULL;
    sLoadedGeneration = 0;
    ClearPendingChunks();
}

bool PcSaveContainer_Load(const char *path, unsigned char *legacyData,
                          size_t legacySize, struct PcSaveLoadInfo *info)
{
    unsigned char *fileData = NULL;
    size_t fileSize = 0;
    size_t legacyOffset = 0;
    uint64_t generation = 0;
    uint32_t chunkCount = 0;
    enum FileReadResult readResult;

    PcSaveContainer_Reset();
    memset(legacyData, 0xFF, legacySize);
    if (info != NULL)
    {
        info->kind = PC_SAVE_LOAD_NEW;
        info->generation = 0;
        info->chunkCount = 0;
    }

    readResult = ReadWholeFile(path, &fileData, &fileSize);
    if (readResult != FILE_READ_OK)
        return readResult == FILE_READ_MISSING;

    if (fileSize == legacySize && memcmp(fileData, sContainerMagic, sizeof(sContainerMagic)) != 0)
    {
        memcpy(legacyData, fileData, legacySize);
        free(fileData);
        if (info != NULL)
            info->kind = PC_SAVE_LOAD_LEGACY;
        return true;
    }

    if (!ValidateContainer(fileData, fileSize, legacySize, &legacyOffset, &generation, &chunkCount))
    {
        free(fileData);
        return false;
    }

    memcpy(legacyData, fileData + legacyOffset, legacySize);
    sLoadedContainer = fileData;
    sLoadedGeneration = generation;
    if (info != NULL)
    {
        info->kind = PC_SAVE_LOAD_NATIVE;
        info->generation = generation;
        info->chunkCount = chunkCount;
    }
    return true;
}

static struct PendingChunk *FindPendingChunk(const unsigned char id[8])
{
    for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
    {
        if (sPendingChunks[i].used && memcmp(sPendingChunks[i].id, id, 8) == 0)
            return &sPendingChunks[i];
    }
    return NULL;
}

bool PcSaveContainer_GetChunk(const char *id, const unsigned char **data,
                              size_t *size, uint32_t *schemaVersion,
                              uint32_t *flags)
{
    unsigned char normalizedId[8];
    struct PendingChunk *pending;

    if (!NormalizeChunkId(id, normalizedId) || data == NULL || size == NULL)
        return false;
    pending = FindPendingChunk(normalizedId);
    if (pending != NULL)
    {
        *data = pending->data;
        *size = pending->size;
        if (schemaVersion != NULL)
            *schemaVersion = pending->schemaVersion;
        if (flags != NULL)
            *flags = pending->flags;
        return true;
    }

    if (sLoadedContainer != NULL)
    {
        uint32_t count = ReadU32(sLoadedContainer + 24);
        for (uint32_t i = 0; i < count; i++)
        {
            const unsigned char *entry = sLoadedContainer + HEADER_SIZE + i * DIRECTORY_ENTRY_SIZE;
            if (memcmp(entry, normalizedId, 8) == 0)
            {
                *data = sLoadedContainer + (size_t)ReadU64(entry + 16);
                *size = (size_t)ReadU64(entry + 24);
                if (schemaVersion != NULL)
                    *schemaVersion = ReadU32(entry + 8);
                if (flags != NULL)
                    *flags = ReadU32(entry + 12);
                return true;
            }
        }
    }
    return false;
}

bool PcSaveContainer_SetChunk(const char *id, uint32_t schemaVersion,
                              uint32_t flags, const unsigned char *data,
                              size_t size)
{
    unsigned char normalizedId[8];
    struct PendingChunk *pending;
    unsigned char *copy = NULL;

    if (!NormalizeChunkId(id, normalizedId)
     || memcmp(normalizedId, sLegacyChunkId, 8) == 0
     || schemaVersion == 0 || (flags & ~PC_SAVE_CHUNK_REQUIRED) != 0
     || ((flags & PC_SAVE_CHUNK_REQUIRED) != 0
      && (!IsKnownChunk(normalizedId)
       || !IsSupportedRequiredChunk(normalizedId, schemaVersion, data, size)))
     || (size != 0 && data == NULL) || size > PC_SAVE_CONTAINER_MAX_SIZE)
        return false;
    if (size != 0)
    {
        copy = malloc(size);
        if (copy == NULL)
            return false;
        memcpy(copy, data, size);
    }

    pending = FindPendingChunk(normalizedId);
    if (pending == NULL)
    {
        for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
        {
            if (!sPendingChunks[i].used)
            {
                pending = &sPendingChunks[i];
                break;
            }
        }
    }
    if (pending == NULL)
    {
        free(copy);
        return false;
    }

    free(pending->data);
    pending->used = true;
    memcpy(pending->id, normalizedId, 8);
    pending->schemaVersion = schemaVersion;
    pending->flags = flags;
    pending->data = copy;
    pending->size = size;
    return true;
}

static bool IsChunkOverridden(const unsigned char id[8])
{
    return FindPendingChunk(id) != NULL;
}

static uint32_t CountOutputChunks(void)
{
    uint32_t count = 1;

    if (sLoadedContainer != NULL)
    {
        uint32_t loadedCount = ReadU32(sLoadedContainer + 24);
        for (uint32_t i = 0; i < loadedCount; i++)
        {
            const unsigned char *entry = sLoadedContainer + HEADER_SIZE + i * DIRECTORY_ENTRY_SIZE;
            if (memcmp(entry, sLegacyChunkId, 8) != 0 && !IsChunkOverridden(entry))
                count++;
        }
    }
    for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
    {
        if (sPendingChunks[i].used)
            count++;
    }
    return count;
}

bool PcSaveContainer_Commit(const char *path, const unsigned char *legacyData,
                            size_t legacySize, unsigned int recoveryCount)
{
    uint32_t chunkCount = CountOutputChunks();
    size_t directorySize = (size_t)chunkCount * DIRECTORY_ENTRY_SIZE;
    size_t totalSize = HEADER_SIZE + directorySize + legacySize;
    unsigned char *output;
    unsigned char *legacyEntry;
    size_t payloadOffset;
    uint32_t outputIndex = 1;

    if (chunkCount > PC_SAVE_CONTAINER_MAX_CHUNKS
     || directorySize > PC_SAVE_CONTAINER_MAX_SIZE - HEADER_SIZE
     || legacySize > PC_SAVE_CONTAINER_MAX_SIZE - HEADER_SIZE - directorySize)
        return false;

    if (sLoadedContainer != NULL)
    {
        uint32_t oldCount = ReadU32(sLoadedContainer + 24);
        for (uint32_t i = 0; i < oldCount; i++)
        {
            const unsigned char *entry = sLoadedContainer + HEADER_SIZE + i * DIRECTORY_ENTRY_SIZE;
            if (memcmp(entry, sLegacyChunkId, sizeof(sLegacyChunkId)) != 0
             && !IsChunkOverridden(entry))
            {
                size_t chunkSize = (size_t)ReadU64(entry + 24);
                if (chunkSize > PC_SAVE_CONTAINER_MAX_SIZE - totalSize)
                    return false;
                totalSize += chunkSize;
            }
        }
    }
    for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
    {
        if (sPendingChunks[i].used)
        {
            if (sPendingChunks[i].size > PC_SAVE_CONTAINER_MAX_SIZE - totalSize)
                return false;
            totalSize += sPendingChunks[i].size;
        }
    }
    if (totalSize > PC_SAVE_CONTAINER_MAX_SIZE)
        return false;

    output = calloc(1, totalSize);
    if (output == NULL)
        return false;
    memcpy(output, sContainerMagic, sizeof(sContainerMagic));
    WriteU32(output + 8, PC_SAVE_CONTAINER_VERSION);
    WriteU32(output + 12, HEADER_SIZE);
    WriteU64(output + 16, sLoadedGeneration + 1);
    WriteU32(output + 24, chunkCount);
    WriteU32(output + 28, DIRECTORY_ENTRY_SIZE);

    payloadOffset = HEADER_SIZE + directorySize;
    legacyEntry = output + HEADER_SIZE;
    memcpy(legacyEntry, sLegacyChunkId, sizeof(sLegacyChunkId));
    WriteU32(legacyEntry + 8, 1);
    WriteU32(legacyEntry + 12, PC_SAVE_CHUNK_REQUIRED);
    WriteU64(legacyEntry + 16, payloadOffset);
    WriteU64(legacyEntry + 24, legacySize);
    WriteU64(legacyEntry + 32, legacySize);
    WriteU32(legacyEntry + 40, CalculateCrc32(legacyData, legacySize));
    memcpy(output + payloadOffset, legacyData, legacySize);
    payloadOffset += legacySize;

    if (sLoadedContainer != NULL)
    {
        uint32_t oldCount = ReadU32(sLoadedContainer + 24);
        for (uint32_t i = 0; i < oldCount; i++)
        {
            const unsigned char *oldEntry = sLoadedContainer + HEADER_SIZE + i * DIRECTORY_ENTRY_SIZE;
            size_t oldOffset;
            size_t oldSize;
            unsigned char *newEntry;

            if (memcmp(oldEntry, sLegacyChunkId, sizeof(sLegacyChunkId)) == 0
             || IsChunkOverridden(oldEntry))
                continue;
            oldOffset = (size_t)ReadU64(oldEntry + 16);
            oldSize = (size_t)ReadU64(oldEntry + 24);
            newEntry = output + HEADER_SIZE + outputIndex * DIRECTORY_ENTRY_SIZE;
            memcpy(newEntry, oldEntry, DIRECTORY_ENTRY_SIZE);
            WriteU64(newEntry + 16, payloadOffset);
            memcpy(output + payloadOffset, sLoadedContainer + oldOffset, oldSize);
            payloadOffset += oldSize;
            outputIndex++;
        }
    }

    for (size_t i = 0; i < PC_SAVE_CONTAINER_MAX_CHUNKS; i++)
    {
        const struct PendingChunk *pending = &sPendingChunks[i];
        unsigned char *newEntry;

        if (!pending->used)
            continue;
        newEntry = output + HEADER_SIZE + outputIndex * DIRECTORY_ENTRY_SIZE;
        memcpy(newEntry, pending->id, 8);
        WriteU32(newEntry + 8, pending->schemaVersion);
        WriteU32(newEntry + 12, pending->flags);
        WriteU64(newEntry + 16, payloadOffset);
        WriteU64(newEntry + 24, pending->size);
        WriteU64(newEntry + 32, pending->size);
        WriteU32(newEntry + 40, CalculateCrc32(pending->data, pending->size));
        if (pending->size != 0)
            memcpy(output + payloadOffset, pending->data, pending->size);
        payloadOffset += pending->size;
        outputIndex++;
    }

    WriteU32(output + 32, CalculateCrc32(output + HEADER_SIZE, directorySize));
    WriteU32(output + 36, CalculateCrc32(output, 36));
    if (!PlatformSave_Commit(path, output, totalSize, recoveryCount))
    {
        free(output);
        return false;
    }

    free(sLoadedContainer);
    sLoadedContainer = output;
    sLoadedGeneration++;
    ClearPendingChunks();
    return true;
}
