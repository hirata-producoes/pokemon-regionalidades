#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform/save_file.h"

#define SAVE_SIZE 131072

static int CheckFile(const char *path, unsigned char expected)
{
    unsigned char buffer[4096];
    FILE *file = fopen(path, "rb");
    size_t total = 0;

    if (file == NULL)
        return 0;
    while (!feof(file))
    {
        size_t count = fread(buffer, 1, sizeof(buffer), file);
        for (size_t i = 0; i < count; i++)
        {
            if (buffer[i] != expected)
            {
                fclose(file);
                return 0;
            }
        }
        total += count;
    }
    fclose(file);
    return total == SAVE_SIZE;
}

int main(int argc, char **argv)
{
    unsigned char *data;
    char recoveryPath[1200];

    if (argc != 2)
        return 2;
    data = malloc(SAVE_SIZE);
    if (data == NULL)
        return 3;

    for (unsigned char generation = 1; generation <= 5; generation++)
    {
        memset(data, generation, SAVE_SIZE);
        if (!PlatformSave_Commit(argv[1], data, SAVE_SIZE, 3))
            return 10 + generation;
        if (!CheckFile(argv[1], generation))
            return 20 + generation;
    }

    snprintf(recoveryPath, sizeof(recoveryPath), "%s.recovery-1", argv[1]);
    if (!CheckFile(recoveryPath, 4))
        return 31;
    snprintf(recoveryPath, sizeof(recoveryPath), "%s.recovery-2", argv[1]);
    if (!CheckFile(recoveryPath, 3))
        return 32;
    snprintf(recoveryPath, sizeof(recoveryPath), "%s.recovery-3", argv[1]);
    if (!CheckFile(recoveryPath, 2))
        return 33;

    if (!PlatformSave_Commit(argv[1], data, SAVE_SIZE, 3))
        return 40;
    snprintf(recoveryPath, sizeof(recoveryPath), "%s.recovery-1", argv[1]);
    if (!CheckFile(recoveryPath, 4))
        return 41;

    memset(data, 0, SAVE_SIZE);
    if (!PlatformSave_Load(argv[1], data, SAVE_SIZE)
     || data[0] != 5 || data[SAVE_SIZE - 1] != 5)
        return 50;

    free(data);
    puts("SAVE_FILE_TEST_OK");
    return 0;
}
