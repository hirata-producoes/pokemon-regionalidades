#include "global.h"
#include "gba/m4a_internal.h"
#include "libgcnmultiboot.h"
#include "multiboot.h"

// These GBA-only entry points have no native-PC equivalent. Keeping their
// behavior isolated here lets the gameplay code link while each subsystem is
// replaced by a real platform implementation in a later milestone.

u32 IntrMain[1];

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

u32 umul3232H32(u32 multiplier, u32 multiplicand)
{
    return ((u64)multiplier * multiplicand) >> 32;
}

void SoundMain(void)
{
}

void SoundMainBTM(void)
{
}

void TrackStop(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track)
{
}

void MPlayMain(struct MusicPlayerInfo *player)
{
}

void RealClearChain(void *channel)
{
}

extern void *const gMPlayJumpTableTemplate[];

void MPlayJumpTableCopy(MPlayFunc *jumpTable)
{
    u32 i;

    for (i = 0; i < 36; i++)
        jumpTable[i] = (MPlayFunc)gMPlayJumpTableTemplate[i];
}

#define DEFINE_PLAYER_EVENT(name) \
    void name(struct MusicPlayerInfo *player, struct MusicPlayerTrack *track) { }

DEFINE_PLAYER_EVENT(ply_fine)
DEFINE_PLAYER_EVENT(ply_goto)
DEFINE_PLAYER_EVENT(ply_patt)
DEFINE_PLAYER_EVENT(ply_pend)
DEFINE_PLAYER_EVENT(ply_rept)
DEFINE_PLAYER_EVENT(ply_prio)
DEFINE_PLAYER_EVENT(ply_tempo)
DEFINE_PLAYER_EVENT(ply_keysh)
DEFINE_PLAYER_EVENT(ply_voice)
DEFINE_PLAYER_EVENT(ply_vol)
DEFINE_PLAYER_EVENT(ply_pan)
DEFINE_PLAYER_EVENT(ply_bend)
DEFINE_PLAYER_EVENT(ply_bendr)
DEFINE_PLAYER_EVENT(ply_lfos)
DEFINE_PLAYER_EVENT(ply_lfodl)
DEFINE_PLAYER_EVENT(ply_mod)
DEFINE_PLAYER_EVENT(ply_modt)
DEFINE_PLAYER_EVENT(ply_tune)
DEFINE_PLAYER_EVENT(ply_port)
DEFINE_PLAYER_EVENT(ply_endtie)

void ply_note(u32 noteCmd, struct MusicPlayerInfo *player, struct MusicPlayerTrack *track)
{
}

void m4aSoundVSync(void)
{
}

