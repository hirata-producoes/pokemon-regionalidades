#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon_regionalidades_progress.h"
#include "constants/flags.h"
#ifdef PORTABLE
#include "platform/pc_save_container.h"
#include "platform/pc_world_state.h"
#endif

#define PGR_PROGRESS_MAGIC   0xB47E
#define PGR_PROGRESS_VERSION 2

#ifdef PORTABLE
STATIC_ASSERT(PGR_WORLD_REGION_COUNT == PC_WORLD_STATE_REGION_COUNT, NativeWorldRegionCountMismatch);
STATIC_ASSERT(PGR_STORY_EVENT_WORD_COUNT <= PC_WORLD_STATE_STORY_WORD_COUNT, NativeWorldStoryCapacityTooSmall);
STATIC_ASSERT(PGR_UNIQUE_REWARD_WORD_COUNT <= PC_WORLD_STATE_REWARD_WORD_COUNT, NativeWorldRewardCapacityTooSmall);

static struct PcWorldProgressState sNativeWorldState;
static unsigned char sNativeWorldEncoded[PC_WORLD_STATE_MAX_ENCODED_SIZE];
static bool32 sNativeWorldReady;
#endif

static bool32 IsValidRegion(enum PgwStartingRegion region)
{
    return (u32)region < PGR_WORLD_REGION_COUNT;
}

static bool32 IsValidStoryEvent(u16 eventId)
{
#ifdef PORTABLE
    return eventId < PC_WORLD_STATE_STORY_WORD_COUNT * 32u;
#else
    return eventId < PGR_STORY_EVENT_COUNT;
#endif
}

static bool32 IsValidReward(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId)
{
    if (scope != PGR_REWARD_GLOBAL && scope != PGR_REWARD_REGIONAL)
        return FALSE;
#ifdef PORTABLE
    if (rewardId >= PC_WORLD_STATE_REWARD_WORD_COUNT * 32u)
        return FALSE;
#else
    if (rewardId >= PGR_UNIQUE_REWARD_COUNT)
        return FALSE;
#endif
    return scope == PGR_REWARD_GLOBAL || IsValidRegion(region);
}

static void ResetLegacyProgress(void)
{
    memset(&gSaveBlock3Ptr->regionalidadesProgress, 0, sizeof(gSaveBlock3Ptr->regionalidadesProgress));
    gSaveBlock3Ptr->regionalidadesProgress.magic = PGR_PROGRESS_MAGIC;
    gSaveBlock3Ptr->regionalidadesProgress.version = PGR_PROGRESS_VERSION;
}

static void SetLegacyStoryEvent(enum PgwStartingRegion region, u16 eventId)
{
    if (eventId < PGR_STORY_EVENT_COUNT)
        gSaveBlock3Ptr->regionalidadesProgress.storyEvents[region][eventId / 32] |= 1u << (eventId % 32);
}

static void ImportLegacyHoennProgress(void)
{
    // Import in dependency order. A later canonical flag implies that the
    // earlier steps of Emerald's linear opening were completed as well.
    if (FlagGet(FLAG_VISITED_LITTLEROOT_TOWN)
     || FlagGet(FLAG_RESCUED_BIRCH)
     || FlagGet(FLAG_DEFEATED_RIVAL_ROUTE103)
     || FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ARRIVED_LITTLEROOT);

    if (FlagGet(FLAG_RESCUED_BIRCH)
     || FlagGet(FLAG_DEFEATED_RIVAL_ROUTE103)
     || FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RESCUED_BIRCH);

    if (FlagGet(FLAG_DEFEATED_RIVAL_ROUTE103)
     || FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_103);

    if (FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH))
    {
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_POKEDEX);
        gSaveBlock3Ptr->regionalidadesProgress.globalRewards[0] |= 1u << PGR_REWARD_ROTOMDEX_DEVICE;
    }

    if (FlagGet(FLAG_RECEIVED_RUNNING_SHOES) && VarGet(VAR_LITTLEROOT_TOWN_STATE) >= 4)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES);

}

static void EnsureLegacyProgressValid(void)
{
    if (gSaveBlock3Ptr->regionalidadesProgress.magic == PGR_PROGRESS_MAGIC
     && gSaveBlock3Ptr->regionalidadesProgress.version == 1)
    {
        // Repair the old ability-based Hoenn inference, preserving other
        // regions and reward records.
        gSaveBlock3Ptr->regionalidadesProgress.version = PGR_PROGRESS_VERSION;
        memset(gSaveBlock3Ptr->regionalidadesProgress.storyEvents[PGW_START_HOENN], 0,
               sizeof(gSaveBlock3Ptr->regionalidadesProgress.storyEvents[PGW_START_HOENN]));
        ImportLegacyHoennProgress();
        return;
    }
    if (gSaveBlock3Ptr->regionalidadesProgress.magic != PGR_PROGRESS_MAGIC
     || gSaveBlock3Ptr->regionalidadesProgress.version != PGR_PROGRESS_VERSION)
    {
        ResetLegacyProgress();
        ImportLegacyHoennProgress();
    }
}

#ifdef PORTABLE
static void MergeLegacyIntoNative(void)
{
    for (u32 region = 0; region < PGR_WORLD_REGION_COUNT; region++)
    {
        for (u32 word = 0; word < PGR_STORY_EVENT_WORD_COUNT; word++)
            sNativeWorldState.storyEvents[region][word] |= gSaveBlock3Ptr->regionalidadesProgress.storyEvents[region][word];
        for (u32 word = 0; word < PGR_UNIQUE_REWARD_WORD_COUNT; word++)
            sNativeWorldState.regionalRewards[region][word] |= gSaveBlock3Ptr->regionalidadesProgress.regionalRewards[region][word];
    }
    for (u32 word = 0; word < PGR_UNIQUE_REWARD_WORD_COUNT; word++)
        sNativeWorldState.globalRewards[word] |= gSaveBlock3Ptr->regionalidadesProgress.globalRewards[word];
}

static void MirrorNativeIntoLegacy(void)
{
    for (u32 region = 0; region < PGR_WORLD_REGION_COUNT; region++)
    {
        for (u32 word = 0; word < PGR_STORY_EVENT_WORD_COUNT; word++)
            gSaveBlock3Ptr->regionalidadesProgress.storyEvents[region][word] |= sNativeWorldState.storyEvents[region][word];
        for (u32 word = 0; word < PGR_UNIQUE_REWARD_WORD_COUNT; word++)
            gSaveBlock3Ptr->regionalidadesProgress.regionalRewards[region][word] |= sNativeWorldState.regionalRewards[region][word];
    }
    for (u32 word = 0; word < PGR_UNIQUE_REWARD_WORD_COUNT; word++)
        gSaveBlock3Ptr->regionalidadesProgress.globalRewards[word] |= sNativeWorldState.globalRewards[word];
}

static bool32 InitializeNativeWorld(void)
{
    const unsigned char *data;
    size_t size;
    u32 schemaVersion;
    u32 flags;
    struct PcWorldProgressState loadedState;

    if (sNativeWorldReady)
        return TRUE;

    memset(&sNativeWorldState, 0, sizeof(sNativeWorldState));
    MergeLegacyIntoNative();
    if (PcSaveContainer_GetChunk("WORLD", &data, &size, &schemaVersion, &flags))
    {
        if (flags != PC_SAVE_CHUNK_REQUIRED
         || !PcWorldState_Decode(data, size, schemaVersion, &loadedState))
            return FALSE;
        for (u32 region = 0; region < PC_WORLD_STATE_REGION_COUNT; region++)
        {
            for (u32 word = 0; word < PC_WORLD_STATE_STORY_WORD_COUNT; word++)
                sNativeWorldState.storyEvents[region][word] |= loadedState.storyEvents[region][word];
            for (u32 word = 0; word < PC_WORLD_STATE_REWARD_WORD_COUNT; word++)
                sNativeWorldState.regionalRewards[region][word] |= loadedState.regionalRewards[region][word];
        }
        for (u32 word = 0; word < PC_WORLD_STATE_REWARD_WORD_COUNT; word++)
            sNativeWorldState.globalRewards[word] |= loadedState.globalRewards[word];
    }
    MirrorNativeIntoLegacy();
    sNativeWorldReady = TRUE;
    return TRUE;
}
#endif

void PgrProgress_Reset(void)
{
    ResetLegacyProgress();
#ifdef PORTABLE
    memset(&sNativeWorldState, 0, sizeof(sNativeWorldState));
    sNativeWorldReady = TRUE;
#endif
}

void PgrProgress_EnsureValid(void)
{
    EnsureLegacyProgressValid();
#ifdef PORTABLE
    InitializeNativeWorld();
#endif
}

static void SetStoryEvent(enum PgwStartingRegion region, u16 eventId)
{
    u32 mask = 1u << (eventId % 32);

#ifdef PORTABLE
    sNativeWorldState.storyEvents[region][eventId / 32] |= mask;
    if (eventId < PGR_STORY_EVENT_COUNT)
#endif
        gSaveBlock3Ptr->regionalidadesProgress.storyEvents[region][eventId / 32] |= mask;
}

bool32 PgrProgress_OnSaveLoaded(void)
{
    EnsureLegacyProgressValid();
#ifdef PORTABLE
    sNativeWorldReady = FALSE;
    return InitializeNativeWorld();
#endif
    return TRUE;
}

bool32 PgrProgress_StageNativeWorld(void)
{
#ifdef PORTABLE
    size_t encodedSize;

    PgrProgress_EnsureValid();
    if (!sNativeWorldReady
     || !PcWorldState_Encode(&sNativeWorldState, sNativeWorldEncoded, &encodedSize))
        return FALSE;
    return PcSaveContainer_SetChunk("WORLD", PC_WORLD_STATE_SCHEMA_VERSION,
                                    PC_SAVE_CHUNK_REQUIRED, sNativeWorldEncoded,
                                    encodedSize);
#else
    return TRUE;
#endif
}

bool32 PgrProgress_IsStoryEventComplete(enum PgwStartingRegion region, u16 eventId)
{
    PgrProgress_EnsureValid();
    if (!IsValidRegion(region) || !IsValidStoryEvent(eventId))
        return FALSE;
#ifdef PORTABLE
    return (sNativeWorldState.storyEvents[region][eventId / 32] & (1u << (eventId % 32))) != 0;
#else
    return (gSaveBlock3Ptr->regionalidadesProgress.storyEvents[region][eventId / 32] & (1u << (eventId % 32))) != 0;
#endif
}

bool32 PgrProgress_AreRequirementsMet(const struct PgrStoryRequirement *requirements, u32 count)
{
    u32 i;

    if (count != 0 && requirements == NULL)
        return FALSE;
    for (i = 0; i < count; i++)
    {
        if (!PgrProgress_IsStoryEventComplete(requirements[i].region, requirements[i].eventId))
            return FALSE;
    }
    return TRUE;
}

enum PgrProgressResult PgrProgress_TryCompleteStoryEvent(enum PgwStartingRegion region, u16 eventId, const struct PgrStoryRequirement *requirements, u32 count)
{
    PgrProgress_EnsureValid();
    if (!IsValidRegion(region) || !IsValidStoryEvent(eventId) || (count != 0 && requirements == NULL))
        return PGR_PROGRESS_INVALID;
    if (PgrProgress_IsStoryEventComplete(region, eventId))
        return PGR_PROGRESS_ALREADY_COMPLETE;
    if (!PgrProgress_AreRequirementsMet(requirements, count))
        return PGR_PROGRESS_REQUIREMENTS_NOT_MET;

    SetStoryEvent(region, eventId);
    return PGR_PROGRESS_COMPLETED;
}

static bool32 GetRegisteredRequirements(enum PgwStartingRegion region, u16 eventId, const struct PgrStoryRequirement **requirements, u32 *count)
{
    static const struct PgrStoryRequirement sHoennRescue[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_ARRIVED_LITTLEROOT },
    };
    static const struct PgrStoryRequirement sHoennRival[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RESCUED_BIRCH },
    };
    static const struct PgrStoryRequirement sHoennPokedex[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_103 },
    };
    static const struct PgrStoryRequirement sHoennShoes[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_POKEDEX },
    };

    *requirements = NULL;
    *count = 0;
    if (region != PGW_START_HOENN)
        return FALSE;

    switch (eventId)
    {
    case PGR_HOENN_STORY_ARRIVED_LITTLEROOT:
        return TRUE;
    case PGR_HOENN_STORY_RESCUED_BIRCH:
        *requirements = sHoennRescue;
        *count = ARRAY_COUNT(sHoennRescue);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_103:
        *requirements = sHoennRival;
        *count = ARRAY_COUNT(sHoennRival);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_POKEDEX:
        *requirements = sHoennPokedex;
        *count = ARRAY_COUNT(sHoennPokedex);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES:
        *requirements = sHoennShoes;
        *count = ARRAY_COUNT(sHoennShoes);
        return TRUE;
    default:
        return FALSE;
    }
}

bool32 PgrProgress_CanStartRegisteredStoryEvent(enum PgwStartingRegion region, u16 eventId)
{
    const struct PgrStoryRequirement *requirements;
    u32 count;

    if (!GetRegisteredRequirements(region, eventId, &requirements, &count))
        return FALSE;
    if (PgrProgress_IsStoryEventComplete(region, eventId))
        return FALSE;
    return PgrProgress_AreRequirementsMet(requirements, count);
}

enum PgrProgressResult PgrProgress_TryCompleteRegisteredStoryEvent(enum PgwStartingRegion region, u16 eventId)
{
    const struct PgrStoryRequirement *requirements;
    u32 count;

    if (!GetRegisteredRequirements(region, eventId, &requirements, &count))
        return PGR_PROGRESS_INVALID;
    return PgrProgress_TryCompleteStoryEvent(region, eventId, requirements, count);
}

bool32 PgrProgress_IsRewardClaimed(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId)
{
    u32 mask;

    PgrProgress_EnsureValid();
    if (!IsValidReward(scope, region, rewardId))
        return FALSE;

    mask = 1u << (rewardId % 32);
#ifdef PORTABLE
    if (scope == PGR_REWARD_GLOBAL)
        return (sNativeWorldState.globalRewards[rewardId / 32] & mask) != 0;
    return (sNativeWorldState.regionalRewards[region][rewardId / 32] & mask) != 0;
#else
    if (scope == PGR_REWARD_GLOBAL)
        return (gSaveBlock3Ptr->regionalidadesProgress.globalRewards[rewardId / 32] & mask) != 0;
    return (gSaveBlock3Ptr->regionalidadesProgress.regionalRewards[region][rewardId / 32] & mask) != 0;
#endif
}

bool32 PgrProgress_MarkRewardClaimed(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId)
{
    u32 mask;

    PgrProgress_EnsureValid();
    if (!IsValidReward(scope, region, rewardId))
        return FALSE;

    mask = 1u << (rewardId % 32);
#ifdef PORTABLE
    if (scope == PGR_REWARD_GLOBAL)
        sNativeWorldState.globalRewards[rewardId / 32] |= mask;
    else
        sNativeWorldState.regionalRewards[region][rewardId / 32] |= mask;

    if (rewardId < PGR_UNIQUE_REWARD_COUNT)
    {
#endif
        if (scope == PGR_REWARD_GLOBAL)
            gSaveBlock3Ptr->regionalidadesProgress.globalRewards[rewardId / 32] |= mask;
        else
            gSaveBlock3Ptr->regionalidadesProgress.regionalRewards[region][rewardId / 32] |= mask;
#ifdef PORTABLE
    }
#endif
    return TRUE;
}

enum PgrRewardResult PgrProgress_TryGiveUniqueItem(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId, enum Item itemId, u16 count)
{
    if (!IsValidReward(scope, region, rewardId) || itemId == ITEM_NONE || count == 0)
        return PGR_REWARD_RESULT_INVALID;
    if (PgrProgress_IsRewardClaimed(scope, region, rewardId))
        return PGR_REWARD_RESULT_ALREADY_CLAIMED;

    // Old saves may already own the item without the new ledger. Adopt that
    // state instead of duplicating it when a regional script is repeated.
    if (CheckBagHasItem(itemId, count))
    {
        PgrProgress_MarkRewardClaimed(scope, region, rewardId);
        return PGR_REWARD_RESULT_ALREADY_CLAIMED;
    }
    if (!AddBagItem(itemId, count))
        return PGR_REWARD_RESULT_NO_ROOM;

    PgrProgress_MarkRewardClaimed(scope, region, rewardId);
    return PGR_REWARD_RESULT_GRANTED;
}

void PgrProgress_OnLegacyFlagSet(u16 flagId)
{
    enum PgrProgressResult result;

    switch (flagId)
    {
    case FLAG_VISITED_LITTLEROOT_TOWN:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ARRIVED_LITTLEROOT);
        break;
    case FLAG_RESCUED_BIRCH:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RESCUED_BIRCH);
        break;
    case FLAG_DEFEATED_RIVAL_ROUTE103:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_103);
        break;
    case FLAG_RECEIVED_POKEDEX_FROM_BIRCH:
        result = PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_POKEDEX);
        if (result == PGR_PROGRESS_COMPLETED || result == PGR_PROGRESS_ALREADY_COMPLETE)
            PgrProgress_MarkRewardClaimed(PGR_REWARD_GLOBAL, PGW_START_HOENN, PGR_REWARD_ROTOMDEX_DEVICE);
        break;
    case FLAG_RECEIVED_RUNNING_SHOES:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES);
        break;
    }
}

void PgrProgress_ScriptCheckHoennPokedex(void)
{
    gSpecialVar_Result = FlagGet(FLAG_DEFEATED_RIVAL_ROUTE103)
        && !FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH)
        && PgrProgress_CanStartRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_POKEDEX);
}

void PgrProgress_ScriptCheckHoennShoes(void)
{
    gSpecialVar_Result = FlagGet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH)
        && PgrProgress_CanStartRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES);
}

void PgrProgress_ScriptCanStartStoryEvent(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void PgrProgress_ScriptCompleteStoryEvent(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void PgrProgress_ScriptTryGiveUniqueItem(void)
{
    gSpecialVar_Result = PgrProgress_TryGiveUniqueItem(gSpecialVar_0x8006,
                                                       gSpecialVar_0x8004,
                                                       gSpecialVar_0x8005,
                                                       gSpecialVar_0x8007,
                                                       gSpecialVar_0x8008);
}
