#ifndef GUARD_RESOURCE_PACK_H
#define GUARD_RESOURCE_PACK_H

#include "gba/types.h"

bool32 ResourcePack_Open(const char *path);
void ResourcePack_Close(void);
bool32 ResourcePack_IsOpen(void);
const void *ResourcePack_Get(const char *name, const void *fallback, u64 fallbackSize, u64 *sizeOut);
const void *ResourcePack_GetByHash(u64 hash, u64 *sizeOut);
void *ResourcePack_Load(const char *name, u64 *sizeOut);
void *ResourcePack_LoadByHash(u64 hash, u64 *sizeOut);
void ResourcePack_Free(void *data);

#endif // GUARD_RESOURCE_PACK_H
