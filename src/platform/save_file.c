#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "platform/save_file.h"

#define SAVE_PATH_CAPACITY 1200

static bool BuildAuxiliaryPath(char *destination, size_t capacity, const char *path, const char *suffix)
{
    int length = snprintf(destination, capacity, "%s%s", path, suffix);
    return length > 0 && (size_t)length < capacity;
}

static bool QuerySaveFileSize(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long fileSize;

    if (file == NULL)
        return false;
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return false;
    }
    fileSize = ftell(file);
    fclose(file);
    if (fileSize < 0)
        return false;
    *size = (size_t)fileSize;
    return true;
}

static bool FileHasSize(const char *path, size_t expectedSize)
{
    size_t size;
    return QuerySaveFileSize(path, &size) && size == expectedSize;
}

static bool FileMatchesBuffer(const char *path, const unsigned char *data, size_t size)
{
    unsigned char buffer[4096];
    FILE *file;
    size_t offset = 0;

    if (!FileHasSize(path, size))
        return false;
    file = fopen(path, "rb");
    if (file == NULL)
        return false;

    while (offset < size)
    {
        size_t chunkSize = size - offset;
        if (chunkSize > sizeof(buffer))
            chunkSize = sizeof(buffer);
        if (fread(buffer, 1, chunkSize, file) != chunkSize
         || memcmp(buffer, data + offset, chunkSize) != 0)
        {
            fclose(file);
            return false;
        }
        offset += chunkSize;
    }

    fclose(file);
    return true;
}

static bool ReplaceFileAtomically(const char *temporaryPath, const char *destinationPath)
{
#ifdef _WIN32
    return MoveFileExA(temporaryPath, destinationPath,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(temporaryPath, destinationPath) == 0;
#endif
}

static bool WriteBufferFile(const char *path, const unsigned char *data, size_t size)
{
    FILE *file = fopen(path, "wb");
    bool success;

    if (file == NULL)
        return false;
    success = fwrite(data, 1, size, file) == size;
    if (fflush(file) != 0)
        success = false;
    if (fclose(file) != 0)
        success = false;
    if (!success)
    {
        remove(path);
        return false;
    }
    return FileHasSize(path, size);
}

static bool CopyFileAtomically(const char *sourcePath, const char *destinationPath, size_t expectedSize)
{
    unsigned char buffer[4096];
    char temporaryPath[SAVE_PATH_CAPACITY];
    FILE *source;
    FILE *destination;
    bool success = true;

    if (!BuildAuxiliaryPath(temporaryPath, sizeof(temporaryPath), destinationPath, ".tmp"))
        return false;
    source = fopen(sourcePath, "rb");
    destination = fopen(temporaryPath, "wb");
    if (source == NULL || destination == NULL)
    {
        if (source != NULL)
            fclose(source);
        if (destination != NULL)
            fclose(destination);
        remove(temporaryPath);
        return false;
    }

    for (;;)
    {
        size_t bytesRead = fread(buffer, 1, sizeof(buffer), source);
        if (bytesRead != 0 && fwrite(buffer, 1, bytesRead, destination) != bytesRead)
        {
            success = false;
            break;
        }
        if (bytesRead < sizeof(buffer))
        {
            if (ferror(source))
                success = false;
            break;
        }
    }

    fclose(source);
    if (fflush(destination) != 0 || fclose(destination) != 0)
        success = false;
    if (!success || !FileHasSize(temporaryPath, expectedSize)
     || !ReplaceFileAtomically(temporaryPath, destinationPath))
    {
        remove(temporaryPath);
        return false;
    }
    return true;
}

static bool RotateRecoveryFiles(const char *path, unsigned int recoveryCount)
{
    char sourcePath[SAVE_PATH_CAPACITY];
    char destinationPath[SAVE_PATH_CAPACITY];

    for (unsigned int index = recoveryCount; index > 1; index--)
    {
        char sourceSuffix[32];
        char destinationSuffix[32];

        snprintf(sourceSuffix, sizeof(sourceSuffix), ".recovery-%u", index - 1);
        snprintf(destinationSuffix, sizeof(destinationSuffix), ".recovery-%u", index);
        if (!BuildAuxiliaryPath(sourcePath, sizeof(sourcePath), path, sourceSuffix)
         || !BuildAuxiliaryPath(destinationPath, sizeof(destinationPath), path, destinationSuffix))
            return false;

        size_t sourceSize;
        if (QuerySaveFileSize(sourcePath, &sourceSize))
        {
            if (!CopyFileAtomically(sourcePath, destinationPath, sourceSize))
                return false;
        }
        else
        {
            remove(destinationPath);
        }
    }

    if (!BuildAuxiliaryPath(destinationPath, sizeof(destinationPath), path, ".recovery-1"))
        return false;
    size_t activeSize;
    return QuerySaveFileSize(path, &activeSize)
        && CopyFileAtomically(path, destinationPath, activeSize);
}

bool PlatformSave_Load(const char *path, unsigned char *data, size_t size)
{
    FILE *file = fopen(path, "rb");
    size_t bytesRead;

    if (file == NULL)
    {
        memset(data, 0xFF, size);
        return errno == ENOENT;
    }

    bytesRead = fread(data, 1, size, file);
    if (ferror(file))
    {
        fclose(file);
        memset(data, 0xFF, size);
        return false;
    }
    fclose(file);
    if (bytesRead < size)
        memset(data + bytesRead, 0xFF, size - bytesRead);
    return true;
}

bool PlatformSave_Commit(const char *path, const unsigned char *data, size_t size, unsigned int recoveryCount)
{
    char pendingPath[SAVE_PATH_CAPACITY];
    size_t previousSize;
    bool hasPreviousSave = QuerySaveFileSize(path, &previousSize);

    if (!BuildAuxiliaryPath(pendingPath, sizeof(pendingPath), path, ".pending")
     || !WriteBufferFile(pendingPath, data, size))
        return false;

    if (hasPreviousSave && FileMatchesBuffer(path, data, size))
    {
        remove(pendingPath);
        return true;
    }

    if (hasPreviousSave && recoveryCount != 0
     && !RotateRecoveryFiles(path, recoveryCount))
    {
        remove(pendingPath);
        return false;
    }

    if (!ReplaceFileAtomically(pendingPath, path))
    {
        remove(pendingPath);
        return false;
    }
    return true;
}
