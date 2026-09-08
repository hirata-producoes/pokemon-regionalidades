#ifndef GUARD_PLATFORM_SAVE_FILE_H
#define GUARD_PLATFORM_SAVE_FILE_H

#include <stdbool.h>
#include <stddef.h>

bool PlatformSave_Load(const char *path, unsigned char *data, size_t size);
bool PlatformSave_Commit(const char *path, const unsigned char *data, size_t size, unsigned int recoveryCount);

#endif // GUARD_PLATFORM_SAVE_FILE_H
