#include "global.h"
#include "event_data.h"
#include "pokedex.h"
#include "pokemon_go_world.h"
#include "pokemon_regionalidades_dex.h"
#include "constants/pokedex.h"
#include "constants/pokemon_go_world.h"
#include "constants/flags.h"
#ifdef PORTABLE
#include "platform/pc_rotomdex_state.h"
#include "platform/pc_save_container.h"
#endif

#define PGW_DEX_STATE_MAGIC   0xD38A
#define PGW_DEX_STATE_VERSION 1

#ifdef PORTABLE
static struct PcRotomDexState sNativeDexState;
static bool32 sNativeDexReady;
#endif

static void EnsureLegacyResearchState(void)
{
    if (gSaveBlock3Ptr->regionalidadesDex.magic != PGW_DEX_STATE_MAGIC
     || gSaveBlock3Ptr->regionalidadesDex.version != PGW_DEX_STATE_VERSION)
    {
        gSaveBlock3Ptr->regionalidadesDex.magic = PGW_DEX_STATE_MAGIC;
        gSaveBlock3Ptr->regionalidadesDex.version = PGW_DEX_STATE_VERSION;
        gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask = 0;
        gSaveBlock3Ptr->regionalidadesDex.reserved = 0;
        // Migration for saves made before regional research existed.
        if (FlagGet(FLAG_SYS_POKEDEX_GET))
            gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask = 1 << Pgw_GetStartingRegion();
    }
}

#ifdef PORTABLE
static bool32 InitializeNativeResearchState(void)
{
    const unsigned char *data;
    size_t size;
    u32 schemaVersion;
    u32 flags;
    struct PcRotomDexState loadedState;

    if (sNativeDexReady)
        return TRUE;

    sNativeDexState.unlockedRegionMask = gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask;
    if (PcSaveContainer_GetChunk("ROTOMDEX", &data, &size, &schemaVersion, &flags))
    {
        if (flags != PC_SAVE_CHUNK_REQUIRED
         || !PcRotomDexState_Decode(data, size, schemaVersion, &loadedState))
            return FALSE;
        sNativeDexState.unlockedRegionMask |= loadedState.unlockedRegionMask;
    }
    gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask |= sNativeDexState.unlockedRegionMask;
    sNativeDexReady = TRUE;
    return TRUE;
}
#endif

static void EnsureResearchState(void)
{
    EnsureLegacyResearchState();
#ifdef PORTABLE
    InitializeNativeResearchState();
#endif
}

void PgwDex_ResetResearch(void)
{
    gSaveBlock3Ptr->regionalidadesDex.magic = PGW_DEX_STATE_MAGIC;
    gSaveBlock3Ptr->regionalidadesDex.version = PGW_DEX_STATE_VERSION;
    gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask = 0;
    gSaveBlock3Ptr->regionalidadesDex.reserved = 0;
#ifdef PORTABLE
    sNativeDexState.unlockedRegionMask = 0;
    sNativeDexReady = TRUE;
#endif
}

bool32 PgwDex_OnSaveLoaded(void)
{
    EnsureLegacyResearchState();
#ifdef PORTABLE
    sNativeDexReady = FALSE;
    return InitializeNativeResearchState();
#endif
    return TRUE;
}

bool32 PgwDex_StageNativeState(void)
{
#ifdef PORTABLE
    unsigned char encoded[PC_ROTOMDEX_STATE_ENCODED_SIZE];

    EnsureResearchState();
    if (!sNativeDexReady || !PcRotomDexState_Encode(&sNativeDexState, encoded))
        return FALSE;
    return PcSaveContainer_SetChunk("ROTOMDEX", PC_ROTOMDEX_STATE_SCHEMA_VERSION,
                                    PC_SAVE_CHUNK_REQUIRED, encoded, sizeof(encoded));
#else
    return TRUE;
#endif
}

void PgwDex_UnlockRegion(enum PgwStartingRegion region)
{
    EnsureResearchState();
    if ((u32)region < PGW_START_REGION_COUNT)
    {
        gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask |= 1 << region;
#ifdef PORTABLE
        sNativeDexState.unlockedRegionMask |= 1u << region;
#endif
    }
}

bool32 PgwDex_IsRegionUnlocked(enum PgwStartingRegion region)
{
    EnsureResearchState();
    if ((u32)region >= PGW_START_REGION_COUNT)
        return FALSE;
#ifdef PORTABLE
    return (sNativeDexState.unlockedRegionMask & (1u << region)) != 0;
#else
    return (gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask & (1 << region)) != 0;
#endif
}

u16 PgwDex_GetUnlockedRegionMask(void)
{
    EnsureResearchState();
#ifdef PORTABLE
    return sNativeDexState.unlockedRegionMask;
#else
    return gSaveBlock3Ptr->regionalidadesDex.unlockedRegionMask;
#endif
}

enum PgwDexObservation PgwDex_GetObservation(enum NationalDexOrder nationalNum)
{
    if (nationalNum <= NATIONAL_DEX_NONE || nationalNum > NATIONAL_DEX_COUNT)
        return PGW_DEX_NOT_OBSERVED;
    if (GetSetPokedexFlag(nationalNum, FLAG_GET_CAUGHT))
        return PGW_DEX_CAUGHT;
    if (GetSetPokedexFlag(nationalNum, FLAG_GET_SEEN))
        return PGW_DEX_SEEN;
    return PGW_DEX_NOT_OBSERVED;
}
