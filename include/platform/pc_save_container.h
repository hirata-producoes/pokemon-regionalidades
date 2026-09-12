#ifndef GUARD_PLATFORM_PC_SAVE_CONTAINER_H
#define GUARD_PLATFORM_PC_SAVE_CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PC_SAVE_CONTAINER_VERSION 1
#define PC_SAVE_CONTAINER_MAX_SIZE (64u * 1024u * 1024u)
#define PC_SAVE_CONTAINER_MAX_CHUNKS 128u
#define PC_SAVE_CHUNK_OPTIONAL 0u
#define PC_SAVE_CHUNK_REQUIRED 1u

enum PcSaveLoadKind
{
    PC_SAVE_LOAD_NEW,
    PC_SAVE_LOAD_NATIVE,
    PC_SAVE_LOAD_LEGACY,
};

struct PcSaveLoadInfo
{
    enum PcSaveLoadKind kind;
    uint64_t generation;
    uint32_t chunkCount;
};

// Loads either a native container or a raw legacy image. A missing file is a
// valid new campaign and fills legacyData with 0xFF.
bool PcSaveContainer_Load(const char *path, unsigned char *legacyData,
                          size_t legacySize, struct PcSaveLoadInfo *info);

// Writes a native container atomically. Optional chunks loaded from a newer
// compatible file are preserved byte-for-byte.
bool PcSaveContainer_Commit(const char *path, const unsigned char *legacyData,
                            size_t legacySize, unsigned int recoveryCount);

// Chunk identifiers contain one to eight ASCII characters. The returned data
// remains owned by the container and is valid until the next load, set or reset.
bool PcSaveContainer_GetChunk(const char *id, const unsigned char **data,
                              size_t *size, uint32_t *schemaVersion,
                              uint32_t *flags);
bool PcSaveContainer_SetChunk(const char *id, uint32_t schemaVersion,
                              uint32_t flags, const unsigned char *data,
                              size_t size);

void PcSaveContainer_Reset(void);

#endif // GUARD_PLATFORM_PC_SAVE_CONTAINER_H
