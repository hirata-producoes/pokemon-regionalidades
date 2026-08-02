#include "global.h"
#include "gba/gba.h"
#include "gba/flash_internal.h"

static const u16 sDummyMaxTime[] =
{
      10, 65469, TIMER_ENABLE | TIMER_INTR_ENABLE | TIMER_256CLK,
      10, 65469, TIMER_ENABLE | TIMER_INTR_ENABLE | TIMER_256CLK,
    2000, 65469, TIMER_ENABLE | TIMER_INTR_ENABLE | TIMER_256CLK,
    2000, 65469, TIMER_ENABLE | TIMER_INTR_ENABLE | TIMER_256CLK,
};

const struct FlashSetupInfo DUMMY_SAVE =
{
    ProgramFlashByte_DUMMY,
    ProgramFlashSector_DUMMY,
    EraseFlashChip_DUMMY,
    EraseFlashSector_DUMMY,
    WaitForFlashWrite_DUMMY,
    sDummyMaxTime,
    {
        131072,
        { 4096, 12, 32, 0 },
        { 3, 1 },
        { { 0xCC, 0xCC } }
    }
};

u16 WaitForFlashWrite_DUMMY(u8 phase, u8 *addr, u8 lastData)
{
    return 0;
}

u16 EraseFlashChip_DUMMY(void)
{
    memset(FLASH_BASE, 0xFF, sizeof(FLASH_BASE));
    return 0;
}

u16 EraseFlashSector_DUMMY(u16 sectorNum)
{
    memset(&FLASH_BASE[sectorNum << 12], 0xFF, 0x1000);
    return 0;
}

u16 ProgramFlashByte_DUMMY(u16 sectorNum, u32 offset, u8 data)
{
    FLASH_BASE[(sectorNum << gFlash->sector.shift) + offset] = data;
    return 0;
}

u16 ProgramFlashSector_DUMMY(u16 sectorNum, u8 *src)
{
    memcpy(&FLASH_BASE[sectorNum << gFlash->sector.shift], src, 0x1000);
    return 0;
}
