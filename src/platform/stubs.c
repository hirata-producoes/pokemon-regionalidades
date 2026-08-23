#include "global.h"
#include "gba/m4a_internal.h"
#include "libgcnmultiboot.h"
#include "multiboot.h"

// These GBA-only entry points have no native-PC equivalent. Keeping their
// behavior isolated here lets the gameplay code link while each subsystem is
// replaced by a real platform implementation in a later milestone.

u32 IntrMain[1];

bool8 HandleLinkConnection(void)
{
    return FALSE;
}

void RegisterRamReset(u32 resetFlags)
{
}

void ReInitializeEWRAM(void)
{
}

void GameCubeMultiBoot_Main(struct GcmbStruct *state)
{
}

void GameCubeMultiBoot_ExecuteProgram(struct GcmbStruct *state)
{
}

void GameCubeMultiBoot_Init(struct GcmbStruct *state)
{
    // The native target does not execute GBA/GameCube multiboot payloads.
    // Keep the state deterministically inactive so callers never attempt to
    // access the zero-length payload symbols used by the PC build.
    memset(state, 0, sizeof(*state));
}

void GameCubeMultiBoot_HandleSerialInterrupt(struct GcmbStruct *state)
{
}

void GameCubeMultiBoot_Quit(void)
{
}

void MultiBootInit(struct MultiBootParam *params)
{
}

int MultiBootMain(struct MultiBootParam *params)
{
    return 0;
}

void MultiBootStartProbe(struct MultiBootParam *params)
{
}

void MultiBootStartMaster(struct MultiBootParam *params, const u8 *src, int length, u8 paletteColor, s8 paletteSpeed)
{
}

int MultiBootCheckComplete(struct MultiBootParam *params)
{
    return 0;
}
