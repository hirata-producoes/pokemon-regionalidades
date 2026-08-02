#ifdef PORTABLE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/types.h>
#endif

#include "global.h"
#include "resource_pack.h"

#define RESOURCE_PACK_VERSION 1
#define RESOURCE_PACK_HEADER_SIZE 48
#define RESOURCE_PACK_ENTRY_SIZE 40
#define RESOURCE_PACK_MAX_ENTRIES 1000000
#define RESOURCE_PACK_MAX_NAME_LENGTH 4096

static const u8 sResourcePackMagic[8] = {'P', 'G', 'W', 'P', 'A', 'C', 'K', 0};

struct ResourcePackEntry
{
    u64 hash;
    u64 offset;
    u64 size;
    u32 checksum;
    char *name;
    void *data;
};

static FILE *sResourcePackFile;
static struct ResourcePackEntry *sResourcePackEntries;
static u32 sResourcePackEntryCount;
static u64 sResourcePackFileSize;

static u32 ReadU32Le(const u8 *data)
{
    return (u32)data[0]
         | (u32)data[1] << 8
         | (u32)data[2] << 16
         | (u32)data[3] << 24;
}

static u64 ReadU64Le(const u8 *data)
{
    return (u64)ReadU32Le(data) | (u64)ReadU32Le(data + 4) << 32;
}

static u64 HashName(const char *name)
{
    const u8 *cursor = (const u8 *)name;
    u64 hash = UINT64_C(14695981039346656037);

    while (*cursor != 0)
    {
        hash ^= *cursor++;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static u32 CalculateCrc32(const void *data, size_t size)
{
    const u8 *cursor = data;
    u32 crc = UINT32_C(0xFFFFFFFF);
    size_t i;

    while (size-- != 0)
    {
        crc ^= *cursor++;
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (UINT32_C(0xEDB88320) & (u32)-(s32)(crc & 1));
    }
    return ~crc;
}

static bool32 RangeIsValid(u64 offset, u64 size)
{
    return offset <= sResourcePackFileSize && size <= sResourcePackFileSize - offset;
}

static bool32 SeekPack(u64 offset)
{
#ifdef _WIN32
    return _fseeki64(sResourcePackFile, (__int64)offset, SEEK_SET) == 0;
#else
    return fseeko(sResourcePackFile, (off_t)offset, SEEK_SET) == 0;
#endif
}

static u64 GetFileSize(FILE *file)
{
#ifdef _WIN32
    __int64 size;
    if (_fseeki64(file, 0, SEEK_END) != 0)
        return 0;
    size = _ftelli64(file);
    if (size < 0 || _fseeki64(file, 0, SEEK_SET) != 0)
        return 0;
    return (u64)size;
#else
    off_t size;
    if (fseeko(file, 0, SEEK_END) != 0)
        return 0;
    size = ftello(file);
    if (size < 0 || fseeko(file, 0, SEEK_SET) != 0)
        return 0;
    return (u64)size;
#endif
}

void ResourcePack_Close(void)
{
    u32 i;

    if (sResourcePackEntries != NULL)
    {
        for (i = 0; i < sResourcePackEntryCount; i++)
        {
            free(sResourcePackEntries[i].name);
            free(sResourcePackEntries[i].data);
        }
        free(sResourcePackEntries);
    }
    if (sResourcePackFile != NULL)
        fclose(sResourcePackFile);

    sResourcePackFile = NULL;
    sResourcePackEntries = NULL;
    sResourcePackEntryCount = 0;
    sResourcePackFileSize = 0;
}

bool32 ResourcePack_Open(const char *path)
{
    u8 header[RESOURCE_PACK_HEADER_SIZE];
    u64 indexOffset;
    u64 stringOffset;
    u64 dataOffset;
    u64 declaredFileSize;
    u64 indexSize;
    u32 i;

    ResourcePack_Close();
    sResourcePackFile = fopen(path, "rb");
    if (sResourcePackFile == NULL)
    {
        DBGPRINTF("Resource pack: %s unavailable (%s); resources will use their configured fallback\n",
                  path, strerror(errno));
        return FALSE;
    }

    sResourcePackFileSize = GetFileSize(sResourcePackFile);
    if (sResourcePackFileSize < RESOURCE_PACK_HEADER_SIZE
     || fread(header, 1, sizeof(header), sResourcePackFile) != sizeof(header)
     || memcmp(header, sResourcePackMagic, sizeof(sResourcePackMagic)) != 0)
        goto invalid;

    if (ReadU32Le(header + 8) != RESOURCE_PACK_VERSION)
        goto invalid;
    sResourcePackEntryCount = ReadU32Le(header + 12);
    indexOffset = ReadU64Le(header + 16);
    stringOffset = ReadU64Le(header + 24);
    dataOffset = ReadU64Le(header + 32);
    declaredFileSize = ReadU64Le(header + 40);

    if (sResourcePackEntryCount > RESOURCE_PACK_MAX_ENTRIES
     || declaredFileSize != sResourcePackFileSize
     || indexOffset < RESOURCE_PACK_HEADER_SIZE
     || stringOffset < indexOffset
     || dataOffset < stringOffset)
        goto invalid;

    indexSize = (u64)sResourcePackEntryCount * RESOURCE_PACK_ENTRY_SIZE;
    if (!RangeIsValid(indexOffset, indexSize) || indexOffset + indexSize > stringOffset)
        goto invalid;

    sResourcePackEntries = calloc(sResourcePackEntryCount, sizeof(*sResourcePackEntries));
    if (sResourcePackEntryCount != 0 && sResourcePackEntries == NULL)
        goto invalid;

    for (i = 0; i < sResourcePackEntryCount; i++)
    {
        u8 rawEntry[RESOURCE_PACK_ENTRY_SIZE];
        struct ResourcePackEntry *entry = &sResourcePackEntries[i];
        u64 nameOffset;
        u32 nameLength;

        if (!SeekPack(indexOffset + (u64)i * RESOURCE_PACK_ENTRY_SIZE)
         || fread(rawEntry, 1, sizeof(rawEntry), sResourcePackFile) != sizeof(rawEntry))
            goto invalid;

        entry->hash = ReadU64Le(rawEntry);
        entry->offset = ReadU64Le(rawEntry + 8);
        entry->size = ReadU64Le(rawEntry + 16);
        nameOffset = ReadU64Le(rawEntry + 24);
        nameLength = ReadU32Le(rawEntry + 32);
        entry->checksum = ReadU32Le(rawEntry + 36);

        if (nameLength == 0
         || nameLength > RESOURCE_PACK_MAX_NAME_LENGTH
         || nameOffset < stringOffset
         || !RangeIsValid(nameOffset, nameLength)
         || nameOffset + nameLength > dataOffset
         || entry->offset < dataOffset
         || !RangeIsValid(entry->offset, entry->size))
            goto invalid;

        entry->name = malloc((size_t)nameLength + 1);
        if (entry->name == NULL
         || !SeekPack(nameOffset)
         || fread(entry->name, 1, nameLength, sResourcePackFile) != nameLength)
            goto invalid;
        entry->name[nameLength] = '\0';

        if (HashName(entry->name) != entry->hash)
            goto invalid;
        if (i != 0)
        {
            struct ResourcePackEntry *previous = &sResourcePackEntries[i - 1];
            // Hash-only lookups require hashes to be unique. The pack builder
            // enforces this too, but validate the invariant for arbitrary files.
            if (previous->hash >= entry->hash)
                goto invalid;
        }
    }

    DBGPRINTF("Resource pack: loaded %s (%u entries, %llu bytes)\n",
              path, sResourcePackEntryCount, (unsigned long long)sResourcePackFileSize);
    return TRUE;

invalid:
    DBGPRINTF("Resource pack: %s is invalid; resources will use their configured fallback\n", path);
    ResourcePack_Close();
    return FALSE;
}

bool32 ResourcePack_IsOpen(void)
{
    return sResourcePackFile != NULL;
}

static struct ResourcePackEntry *FindFirstEntryByHash(u64 hash)
{
    u32 left = 0;
    u32 right = sResourcePackEntryCount;

    while (left < right)
    {
        u32 middle = left + (right - left) / 2;
        if (sResourcePackEntries[middle].hash < hash)
            left = middle + 1;
        else
            right = middle;
    }

    if (left < sResourcePackEntryCount && sResourcePackEntries[left].hash == hash)
        return &sResourcePackEntries[left];
    return NULL;
}

static struct ResourcePackEntry *FindEntry(const char *name)
{
    u64 hash = HashName(name);
    struct ResourcePackEntry *entry = FindFirstEntryByHash(hash);
    u32 index;

    if (entry == NULL)
        return NULL;
    index = (u32)(entry - sResourcePackEntries);
    for (; index < sResourcePackEntryCount && sResourcePackEntries[index].hash == hash; index++)
    {
        struct ResourcePackEntry *entry = &sResourcePackEntries[index];
        if (strcmp(entry->name, name) == 0)
            return entry;
    }
    return NULL;
}

static void *LoadEntryData(struct ResourcePackEntry *entry)
{
    void *data;

    if (entry == NULL || entry->size > SIZE_MAX)
        return NULL;
    data = malloc(entry->size == 0 ? 1 : (size_t)entry->size);
    if (data == NULL)
        return NULL;
    if (entry->size != 0
     && (!SeekPack(entry->offset)
      || fread(data, 1, (size_t)entry->size, sResourcePackFile) != (size_t)entry->size))
    {
        free(data);
        return NULL;
    }
    if (CalculateCrc32(data, (size_t)entry->size) != entry->checksum)
    {
        free(data);
        return NULL;
    }
    return data;
}

static const void *GetEntryData(struct ResourcePackEntry *entry, u64 *sizeOut)
{
    if (sizeOut != NULL)
        *sizeOut = 0;
    if (entry == NULL)
        return NULL;
    if (entry->data == NULL)
    {
        entry->data = LoadEntryData(entry);
        if (entry->data == NULL)
            return NULL;
        DBGPRINTF("Resource pack: cached %s (%llu bytes)\n",
                  entry->name, (unsigned long long)entry->size);
    }
    if (sizeOut != NULL)
        *sizeOut = entry->size;
    return entry->data;
}

const void *ResourcePack_Get(const char *name, const void *fallback, u64 fallbackSize, u64 *sizeOut)
{
    struct ResourcePackEntry *entry = FindEntry(name);

    if (sizeOut != NULL)
        *sizeOut = fallbackSize;

    if (entry != NULL)
    {
        const void *data = GetEntryData(entry, sizeOut);
        if (data != NULL)
            return data;
    }

    if (fallback != NULL)
        DBGPRINTF("Resource pack: %s missing or unreadable; using compiled fallback\n", name);
    else
        DBGPRINTF("Resource pack: %s missing or unreadable; continuing without this resource\n", name);
    if (sizeOut != NULL)
        *sizeOut = fallbackSize;
    return fallback;
}

const void *ResourcePack_GetByHash(u64 hash, u64 *sizeOut)
{
    return GetEntryData(FindFirstEntryByHash(hash), sizeOut);
}

void *ResourcePack_Load(const char *name, u64 *sizeOut)
{
    struct ResourcePackEntry *entry = FindEntry(name);
    void *data;

    if (sizeOut != NULL)
        *sizeOut = 0;
    data = LoadEntryData(entry);
    if (data == NULL)
    {
        DBGPRINTF("Resource pack: %s missing or unreadable\n", name);
        return NULL;
    }
    if (sizeOut != NULL)
        *sizeOut = entry->size;
    DBGPRINTF("Resource pack: loaded transient %s (%llu bytes)\n",
              name, (unsigned long long)entry->size);
    return data;
}

void *ResourcePack_LoadByHash(u64 hash, u64 *sizeOut)
{
    struct ResourcePackEntry *entry = FindFirstEntryByHash(hash);
    void *data;

    if (sizeOut != NULL)
        *sizeOut = 0;
    data = LoadEntryData(entry);
    if (data == NULL)
        return NULL;
    if (sizeOut != NULL)
        *sizeOut = entry->size;
    DBGPRINTF("Resource pack: loaded transient %s (%llu bytes)\n",
              entry->name, (unsigned long long)entry->size);
    return data;
}

void ResourcePack_Free(void *data)
{
    free(data);
}

#endif // PORTABLE
