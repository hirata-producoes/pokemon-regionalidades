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
#define PGR_PROGRESS_VERSION 24

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

    if (FlagGet(FLAG_RECEIVED_EXP_SHARE))
        gSaveBlock3Ptr->regionalidadesProgress.globalRewards[0] |= 1u << PGR_REWARD_EXP_SHARE;

    if (FlagGet(FLAG_RECEIVED_RUNNING_SHOES) && VarGet(VAR_LITTLEROOT_TOWN_STATE) >= 4)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES);

    if (VarGet(VAR_PETALBURG_GYM_STATE) >= 2
     || FlagGet(FLAG_DEFEATED_RUSTBORO_GYM)
     || FlagGet(FLAG_DEFEATED_DEWFORD_GYM)
     || FlagGet(FLAG_DEVON_GOODS_STOLEN))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL);

    if (FlagGet(FLAG_DEFEATED_RUSTBORO_GYM)
     || FlagGet(FLAG_DEVON_GOODS_STOLEN)
     || FlagGet(FLAG_RECOVERED_DEVON_GOODS)
     || FlagGet(FLAG_RETURNED_DEVON_GOODS)
     || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_ROXANNE);

    if (FlagGet(FLAG_DEVON_GOODS_STOLEN)
     || FlagGet(FLAG_RECOVERED_DEVON_GOODS)
     || FlagGet(FLAG_RETURNED_DEVON_GOODS)
     || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEVON_GOODS_STOLEN);

    if (FlagGet(FLAG_RECOVERED_DEVON_GOODS)
     || FlagGet(FLAG_RETURNED_DEVON_GOODS)
     || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECOVERED_DEVON_GOODS);

    if (FlagGet(FLAG_RETURNED_DEVON_GOODS) || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RETURNED_DEVON_GOODS);

    if (FlagGet(FLAG_RECEIVED_POKENAV)
     || FlagGet(FLAG_DELIVERED_STEVEN_LETTER)
     || FlagGet(FLAG_DOCK_REJECTED_DEVON_GOODS)
     || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS);

    if (FlagGet(FLAG_DELIVERED_STEVEN_LETTER)
     || FlagGet(FLAG_DOCK_REJECTED_DEVON_GOODS)
     || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER);

    if (FlagGet(FLAG_DOCK_REJECTED_DEVON_GOODS) || FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN);

    if (FlagGet(FLAG_DELIVERED_DEVON_GOODS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS);

    if (VarGet(VAR_ROUTE110_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110);

    if (FlagGet(FLAG_DEFEATED_WALLY_MAUVILLE) || FlagGet(FLAG_DEFEATED_MAUVILLE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE);

    if (FlagGet(FLAG_DEFEATED_MAUVILLE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON);

    if (FlagGet(FLAG_MET_ARCHIE_METEOR_FALLS)
     || FlagGet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY)
     || FlagGet(FLAG_DEFEATED_LAVARIDGE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT);

    if (FlagGet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY) || FlagGet(FLAG_DEFEATED_LAVARIDGE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY);

    if (FlagGet(FLAG_DEFEATED_LAVARIDGE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_FLANNERY);

    if (FlagGet(FLAG_DEFEATED_DEWFORD_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_BRAWLY);

    if (FlagGet(FLAG_RECEIVED_GO_GOGGLES) || FlagGet(FLAG_DEFEATED_PETALBURG_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_GO_GOGGLES);

    if (FlagGet(FLAG_DEFEATED_PETALBURG_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_NORMAN);

    if (VarGet(VAR_ROUTE118_STATE) >= 1
     || VarGet(VAR_WEATHER_INSTITUTE_STATE) >= 1
     || FlagGet(FLAG_RECEIVED_CASTFORM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_MET_STEVEN_ROUTE_118);

    if (VarGet(VAR_WEATHER_INSTITUTE_STATE) >= 1 || FlagGet(FLAG_RECEIVED_CASTFORM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE);

    if (VarGet(VAR_ROUTE119_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119);

    if (FlagGet(FLAG_RECEIVED_DEVON_SCOPE))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE);

    if (FlagGet(FLAG_KECLEON_FLED_FORTREE) || FlagGet(FLAG_DEFEATED_FORTREE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH);

    if (FlagGet(FLAG_DEFEATED_FORTREE_GYM))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WINONA);

    if (VarGet(VAR_MT_PYRE_STATE) >= 1 || FlagGet(FLAG_RECEIVED_RED_OR_BLUE_ORB))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT);

    if (FlagGet(FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT);

    if (FlagGet(FLAG_MET_TEAM_AQUA_HARBOR) || VarGet(VAR_SLATEPORT_HARBOR_STATE) >= 2)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT);

    if (FlagGet(FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT);

    if (FlagGet(FLAG_DEFEATED_MOSSDEEP_GYM)
     || FlagGet(FLAG_DEFEATED_MAGMA_SPACE_CENTER)
     || (FlagGet(FLAG_RECEIVED_HM_DIVE) && VarGet(VAR_STEVENS_HOUSE_STATE) >= 2)
     || FlagGet(FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN)
     || VarGet(VAR_SEAFLOOR_CAVERN_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_TATE_LIZA);

    if (FlagGet(FLAG_DEFEATED_MAGMA_SPACE_CENTER)
     || (FlagGet(FLAG_RECEIVED_HM_DIVE) && VarGet(VAR_STEVENS_HOUSE_STATE) >= 2)
     || FlagGet(FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN)
     || VarGet(VAR_SEAFLOOR_CAVERN_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER);

    if ((FlagGet(FLAG_RECEIVED_HM_DIVE) && VarGet(VAR_STEVENS_HOUSE_STATE) >= 2)
     || FlagGet(FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN)
     || VarGet(VAR_SEAFLOOR_CAVERN_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN);

    if (FlagGet(FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN) || VarGet(VAR_SEAFLOOR_CAVERN_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN);

    if (VarGet(VAR_SOOTOPOLIS_CITY_STATE) >= 2
     || FlagGet(FLAG_STEVEN_GUIDES_TO_CAVE_OF_ORIGIN)
     || FlagGet(FLAG_WALLACE_GOES_TO_SKY_PILLAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS);

    if (FlagGet(FLAG_WALLACE_GOES_TO_SKY_PILLAR) || VarGet(VAR_SOOTOPOLIS_CITY_STATE) >= 3)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR);

    if (VarGet(VAR_SOOTOPOLIS_CITY_STATE) >= 4)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_OPENED_SKY_PILLAR);

    if (VarGet(VAR_SOOTOPOLIS_CITY_STATE) >= 5 || VarGet(VAR_SKY_PILLAR_STATE) >= 1)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_RAYQUAZA);

    if (VarGet(VAR_SKY_PILLAR_STATE) >= 2)
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS);

    if ((FlagGet(FLAG_RECEIVED_HM_WATERFALL) && FlagGet(FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE))
     || FlagGet(FLAG_DEFEATED_SOOTOPOLIS_GYM)
     || FlagGet(FLAG_DEFEATED_WALLY_VICTORY_ROAD)
     || FlagGet(FLAG_ENTERED_ELITE_FOUR)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE);

    if (FlagGet(FLAG_DEFEATED_SOOTOPOLIS_GYM)
     || FlagGet(FLAG_DEFEATED_WALLY_VICTORY_ROAD)
     || FlagGet(FLAG_ENTERED_ELITE_FOUR)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_JUAN);

    if (FlagGet(FLAG_DEFEATED_WALLY_VICTORY_ROAD)
     || FlagGet(FLAG_ENTERED_ELITE_FOUR)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD);

    if (FlagGet(FLAG_ENTERED_ELITE_FOUR)
     || FlagGet(FLAG_DEFEATED_ELITE_4_SIDNEY)
     || FlagGet(FLAG_DEFEATED_ELITE_4_PHOEBE)
     || FlagGet(FLAG_DEFEATED_ELITE_4_GLACIA)
     || FlagGet(FLAG_DEFEATED_ELITE_4_DRAKE)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE);

    if (FlagGet(FLAG_DEFEATED_ELITE_4_SIDNEY)
     || FlagGet(FLAG_DEFEATED_ELITE_4_PHOEBE)
     || FlagGet(FLAG_DEFEATED_ELITE_4_GLACIA)
     || FlagGet(FLAG_DEFEATED_ELITE_4_DRAKE)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_SIDNEY);

    if (FlagGet(FLAG_DEFEATED_ELITE_4_PHOEBE)
     || FlagGet(FLAG_DEFEATED_ELITE_4_GLACIA)
     || FlagGet(FLAG_DEFEATED_ELITE_4_DRAKE)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_PHOEBE);

    if (FlagGet(FLAG_DEFEATED_ELITE_4_GLACIA)
     || FlagGet(FLAG_DEFEATED_ELITE_4_DRAKE)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_GLACIA);

    if (FlagGet(FLAG_DEFEATED_ELITE_4_DRAKE)
     || FlagGet(FLAG_IS_CHAMPION)
     || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_DRAKE);

    if (FlagGet(FLAG_IS_CHAMPION) || FlagGet(FLAG_SYS_GAME_CLEAR))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION);

    if ((VarGet(VAR_DEX_UPGRADE_JOHTO_STARTER_STATE) >= 2
      || FlagGet(FLAG_SYS_NATIONAL_DEX))
     && (FlagGet(FLAG_IS_CHAMPION) || FlagGet(FLAG_SYS_GAME_CLEAR)))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE);

    if (FlagGet(FLAG_RECEIVED_SS_TICKET))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SS_TICKET);

    if (FlagGet(FLAG_MET_SCOTT_ON_SS_TIDAL))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL);

    if (FlagGet(FLAG_SYS_FRONTIER_PASS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER);

    if (FlagGet(FLAG_SCOTT_GIVES_BATTLE_POINTS))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME);

    if (FlagGet(FLAG_DEFEATED_METEOR_FALLS_STEVEN))
        SetLegacyStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS);
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
    if (gSaveBlock3Ptr->regionalidadesProgress.magic == PGR_PROGRESS_MAGIC
     && gSaveBlock3Ptr->regionalidadesProgress.version >= 2
     && gSaveBlock3Ptr->regionalidadesProgress.version < PGR_PROGRESS_VERSION)
    {
        // Later versions add Hoenn arcs without changing the bitset layout.
        // Preserve every bit already owned by all regions and merge only
        // canonical Hoenn state.
        gSaveBlock3Ptr->regionalidadesProgress.version = PGR_PROGRESS_VERSION;
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
    static const struct PgrStoryRequirement sHoennRoxanne[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL },
    };
    static const struct PgrStoryRequirement sHoennBrawly[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL },
    };
    static const struct PgrStoryRequirement sHoennWallyCatchingTutorial[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES },
    };
    static const struct PgrStoryRequirement sHoennDevonTheft[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_ROXANNE },
    };
    static const struct PgrStoryRequirement sHoennDevonRecovery[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEVON_GOODS_STOLEN },
    };
    static const struct PgrStoryRequirement sHoennDevonReturn[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECOVERED_DEVON_GOODS },
    };
    static const struct PgrStoryRequirement sHoennDevonCommissions[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RETURNED_DEVON_GOODS },
    };
    static const struct PgrStoryRequirement sHoennStevenLetter[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS },
    };
    static const struct PgrStoryRequirement sHoennDock[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER },
    };
    static const struct PgrStoryRequirement sHoennStern[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN },
    };
    static const struct PgrStoryRequirement sHoennRoute110Rival[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS },
    };
    static const struct PgrStoryRequirement sHoennWallyMauville[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110 },
    };
    static const struct PgrStoryRequirement sHoennWattson[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE },
    };
    static const struct PgrStoryRequirement sHoennMeteoriteTheft[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON },
    };
    static const struct PgrStoryRequirement sHoennMaxieMtChimney[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT },
    };
    static const struct PgrStoryRequirement sHoennFlannery[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY },
    };
    static const struct PgrStoryRequirement sHoennGoGoggles[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_FLANNERY },
    };
    static const struct PgrStoryRequirement sHoennNorman[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_GO_GOGGLES },
    };
    static const struct PgrStoryRequirement sHoennStevenRoute118[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_NORMAN },
    };
    static const struct PgrStoryRequirement sHoennWeatherInstitute[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_MET_STEVEN_ROUTE_118 },
    };
    static const struct PgrStoryRequirement sHoennRoute119Rival[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE },
    };
    static const struct PgrStoryRequirement sHoennDevonScope[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119 },
    };
    static const struct PgrStoryRequirement sHoennFortreeGymPath[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE },
    };
    static const struct PgrStoryRequirement sHoennWinona[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH },
    };
    static const struct PgrStoryRequirement sHoennMtPyreOrbTheft[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WINONA },
    };
    static const struct PgrStoryRequirement sHoennMagmaHideout[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT },
    };
    static const struct PgrStoryRequirement sHoennSubmarineTheft[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT },
    };
    static const struct PgrStoryRequirement sHoennAquaHideout[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT },
    };
    static const struct PgrStoryRequirement sHoennTateLiza[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT },
    };
    static const struct PgrStoryRequirement sHoennMossdeepSpaceCenter[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_TATE_LIZA },
    };
    static const struct PgrStoryRequirement sHoennMossdeepScott[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_TATE_LIZA },
    };
    static const struct PgrStoryRequirement sHoennDiveFromSteven[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER },
    };
    static const struct PgrStoryRequirement sHoennSeafloorCavern[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN },
    };
    static const struct PgrStoryRequirement sHoennSootopolisCrisis[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN },
    };
    static const struct PgrStoryRequirement sHoennWallaceRayquaza[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS },
    };
    static const struct PgrStoryRequirement sHoennSkyPillarOpening[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR },
    };
    static const struct PgrStoryRequirement sHoennRayquazaAwakening[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_OPENED_SKY_PILLAR },
    };
    static const struct PgrStoryRequirement sHoennSootopolisResolution[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_RAYQUAZA },
    };
    static const struct PgrStoryRequirement sHoennWaterfallFromWallace[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS },
    };
    static const struct PgrStoryRequirement sHoennJuan[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE },
    };
    static const struct PgrStoryRequirement sHoennWallyVictoryRoad[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_JUAN },
    };
    static const struct PgrStoryRequirement sHoennLeagueEntry[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD },
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_BRAWLY },
    };
    static const struct PgrStoryRequirement sHoennSidney[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE },
    };
    static const struct PgrStoryRequirement sHoennPhoebe[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_SIDNEY },
    };
    static const struct PgrStoryRequirement sHoennGlacia[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_PHOEBE },
    };
    static const struct PgrStoryRequirement sHoennDrake[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_GLACIA },
    };
    static const struct PgrStoryRequirement sHoennChampion[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_DRAKE },
    };
    static const struct PgrStoryRequirement sHoennPostgameResearchUpdate[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION },
    };
    static const struct PgrStoryRequirement sHoennSSTicket[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION },
    };
    static const struct PgrStoryRequirement sHoennSSTidalScott[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE },
        { PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SS_TICKET },
    };
    static const struct PgrStoryRequirement sHoennBattleFrontierReception[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL },
    };
    static const struct PgrStoryRequirement sHoennScottFrontierWelcome[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER },
    };
    static const struct PgrStoryRequirement sHoennMeteorFallsSteven[] =
    {
        { PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION },
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
    case PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL:
        *requirements = sHoennWallyCatchingTutorial;
        *count = ARRAY_COUNT(sHoennWallyCatchingTutorial);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_ROXANNE:
        *requirements = sHoennRoxanne;
        *count = ARRAY_COUNT(sHoennRoxanne);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_BRAWLY:
        *requirements = sHoennBrawly;
        *count = ARRAY_COUNT(sHoennBrawly);
        return TRUE;
    case PGR_HOENN_STORY_DEVON_GOODS_STOLEN:
        *requirements = sHoennDevonTheft;
        *count = ARRAY_COUNT(sHoennDevonTheft);
        return TRUE;
    case PGR_HOENN_STORY_RECOVERED_DEVON_GOODS:
        *requirements = sHoennDevonRecovery;
        *count = ARRAY_COUNT(sHoennDevonRecovery);
        return TRUE;
    case PGR_HOENN_STORY_RETURNED_DEVON_GOODS:
        *requirements = sHoennDevonReturn;
        *count = ARRAY_COUNT(sHoennDevonReturn);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS:
        *requirements = sHoennDevonCommissions;
        *count = ARRAY_COUNT(sHoennDevonCommissions);
        return TRUE;
    case PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER:
        *requirements = sHoennStevenLetter;
        *count = ARRAY_COUNT(sHoennStevenLetter);
        return TRUE;
    case PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN:
        *requirements = sHoennDock;
        *count = ARRAY_COUNT(sHoennDock);
        return TRUE;
    case PGR_HOENN_STORY_DELIVERED_DEVON_GOODS:
        *requirements = sHoennStern;
        *count = ARRAY_COUNT(sHoennStern);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110:
        *requirements = sHoennRoute110Rival;
        *count = ARRAY_COUNT(sHoennRoute110Rival);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE:
        *requirements = sHoennWallyMauville;
        *count = ARRAY_COUNT(sHoennWallyMauville);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_WATTSON:
        *requirements = sHoennWattson;
        *count = ARRAY_COUNT(sHoennWattson);
        return TRUE;
    case PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT:
        *requirements = sHoennMeteoriteTheft;
        *count = ARRAY_COUNT(sHoennMeteoriteTheft);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY:
        *requirements = sHoennMaxieMtChimney;
        *count = ARRAY_COUNT(sHoennMaxieMtChimney);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_FLANNERY:
        *requirements = sHoennFlannery;
        *count = ARRAY_COUNT(sHoennFlannery);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_GO_GOGGLES:
        *requirements = sHoennGoGoggles;
        *count = ARRAY_COUNT(sHoennGoGoggles);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_NORMAN:
        *requirements = sHoennNorman;
        *count = ARRAY_COUNT(sHoennNorman);
        return TRUE;
    case PGR_HOENN_STORY_MET_STEVEN_ROUTE_118:
        *requirements = sHoennStevenRoute118;
        *count = ARRAY_COUNT(sHoennStevenRoute118);
        return TRUE;
    case PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE:
        *requirements = sHoennWeatherInstitute;
        *count = ARRAY_COUNT(sHoennWeatherInstitute);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119:
        *requirements = sHoennRoute119Rival;
        *count = ARRAY_COUNT(sHoennRoute119Rival);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE:
        *requirements = sHoennDevonScope;
        *count = ARRAY_COUNT(sHoennDevonScope);
        return TRUE;
    case PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH:
        *requirements = sHoennFortreeGymPath;
        *count = ARRAY_COUNT(sHoennFortreeGymPath);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_WINONA:
        *requirements = sHoennWinona;
        *count = ARRAY_COUNT(sHoennWinona);
        return TRUE;
    case PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT:
        *requirements = sHoennMtPyreOrbTheft;
        *count = ARRAY_COUNT(sHoennMtPyreOrbTheft);
        return TRUE;
    case PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT:
        *requirements = sHoennMagmaHideout;
        *count = ARRAY_COUNT(sHoennMagmaHideout);
        return TRUE;
    case PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT:
        *requirements = sHoennSubmarineTheft;
        *count = ARRAY_COUNT(sHoennSubmarineTheft);
        return TRUE;
    case PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT:
        *requirements = sHoennAquaHideout;
        *count = ARRAY_COUNT(sHoennAquaHideout);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_TATE_LIZA:
        *requirements = sHoennTateLiza;
        *count = ARRAY_COUNT(sHoennTateLiza);
        return TRUE;
    case PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER:
        *requirements = sHoennMossdeepSpaceCenter;
        *count = ARRAY_COUNT(sHoennMossdeepSpaceCenter);
        return TRUE;
    case PGR_HOENN_STORY_MET_SCOTT_MOSSDEEP:
        *requirements = sHoennMossdeepScott;
        *count = ARRAY_COUNT(sHoennMossdeepScott);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN:
        *requirements = sHoennDiveFromSteven;
        *count = ARRAY_COUNT(sHoennDiveFromSteven);
        return TRUE;
    case PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN:
        *requirements = sHoennSeafloorCavern;
        *count = ARRAY_COUNT(sHoennSeafloorCavern);
        return TRUE;
    case PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS:
        *requirements = sHoennSootopolisCrisis;
        *count = ARRAY_COUNT(sHoennSootopolisCrisis);
        return TRUE;
    case PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR:
        *requirements = sHoennWallaceRayquaza;
        *count = ARRAY_COUNT(sHoennWallaceRayquaza);
        return TRUE;
    case PGR_HOENN_STORY_OPENED_SKY_PILLAR:
        *requirements = sHoennSkyPillarOpening;
        *count = ARRAY_COUNT(sHoennSkyPillarOpening);
        return TRUE;
    case PGR_HOENN_STORY_AWAKENED_RAYQUAZA:
        *requirements = sHoennRayquazaAwakening;
        *count = ARRAY_COUNT(sHoennRayquazaAwakening);
        return TRUE;
    case PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS:
        *requirements = sHoennSootopolisResolution;
        *count = ARRAY_COUNT(sHoennSootopolisResolution);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE:
        *requirements = sHoennWaterfallFromWallace;
        *count = ARRAY_COUNT(sHoennWaterfallFromWallace);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_JUAN:
        *requirements = sHoennJuan;
        *count = ARRAY_COUNT(sHoennJuan);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD:
        *requirements = sHoennWallyVictoryRoad;
        *count = ARRAY_COUNT(sHoennWallyVictoryRoad);
        return TRUE;
    case PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE:
        *requirements = sHoennLeagueEntry;
        *count = ARRAY_COUNT(sHoennLeagueEntry);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_SIDNEY:
        *requirements = sHoennSidney;
        *count = ARRAY_COUNT(sHoennSidney);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_PHOEBE:
        *requirements = sHoennPhoebe;
        *count = ARRAY_COUNT(sHoennPhoebe);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_GLACIA:
        *requirements = sHoennGlacia;
        *count = ARRAY_COUNT(sHoennGlacia);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_DRAKE:
        *requirements = sHoennDrake;
        *count = ARRAY_COUNT(sHoennDrake);
        return TRUE;
    case PGR_HOENN_STORY_BECAME_CHAMPION:
        *requirements = sHoennChampion;
        *count = ARRAY_COUNT(sHoennChampion);
        return TRUE;
    case PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE:
        *requirements = sHoennPostgameResearchUpdate;
        *count = ARRAY_COUNT(sHoennPostgameResearchUpdate);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_SS_TICKET:
        *requirements = sHoennSSTicket;
        *count = ARRAY_COUNT(sHoennSSTicket);
        return TRUE;
    case PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL:
        *requirements = sHoennSSTidalScott;
        *count = ARRAY_COUNT(sHoennSSTidalScott);
        return TRUE;
    case PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER:
        *requirements = sHoennBattleFrontierReception;
        *count = ARRAY_COUNT(sHoennBattleFrontierReception);
        return TRUE;
    case PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME:
        *requirements = sHoennScottFrontierWelcome;
        *count = ARRAY_COUNT(sHoennScottFrontierWelcome);
        return TRUE;
    case PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS:
        *requirements = sHoennMeteorFallsSteven;
        *count = ARRAY_COUNT(sHoennMeteorFallsSteven);
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
    case FLAG_RECEIVED_EXP_SHARE:
        PgrProgress_MarkRewardClaimed(PGR_REWARD_GLOBAL, PGW_START_HOENN, PGR_REWARD_EXP_SHARE);
        break;
    case FLAG_DEFEATED_RUSTBORO_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_ROXANNE);
        break;
    case FLAG_DEFEATED_DEWFORD_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_BRAWLY);
        break;
    case FLAG_DEVON_GOODS_STOLEN:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEVON_GOODS_STOLEN);
        break;
    case FLAG_RECOVERED_DEVON_GOODS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECOVERED_DEVON_GOODS);
        break;
    case FLAG_RETURNED_DEVON_GOODS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RETURNED_DEVON_GOODS);
        break;
    case FLAG_RECEIVED_POKENAV:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS);
        break;
    case FLAG_DELIVERED_STEVEN_LETTER:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER);
        break;
    case FLAG_DOCK_REJECTED_DEVON_GOODS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN);
        break;
    case FLAG_DELIVERED_DEVON_GOODS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS);
        break;
    case FLAG_DEFEATED_WALLY_MAUVILLE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE);
        break;
    case FLAG_DEFEATED_MAUVILLE_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON);
        break;
    case FLAG_MET_ARCHIE_METEOR_FALLS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT);
        break;
    case FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY);
        break;
    case FLAG_DEFEATED_LAVARIDGE_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_FLANNERY);
        break;
    case FLAG_RECEIVED_GO_GOGGLES:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_GO_GOGGLES);
        break;
    case FLAG_DEFEATED_PETALBURG_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_NORMAN);
        break;
    case FLAG_RECEIVED_DEVON_SCOPE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE);
        break;
    case FLAG_KECLEON_FLED_FORTREE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH);
        break;
    case FLAG_DEFEATED_FORTREE_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WINONA);
        break;
    case FLAG_RECEIVED_RED_OR_BLUE_ORB:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT);
        break;
    case FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT);
        break;
    case FLAG_MET_TEAM_AQUA_HARBOR:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT);
        break;
    case FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT);
        break;
    case FLAG_DEFEATED_MOSSDEEP_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_TATE_LIZA);
        break;
    case FLAG_DEFEATED_MAGMA_SPACE_CENTER:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER);
        break;
    case FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN);
        break;
    case FLAG_DEFEATED_SOOTOPOLIS_GYM:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_JUAN);
        break;
    case FLAG_DEFEATED_WALLY_VICTORY_ROAD:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD);
        break;
    case FLAG_ENTERED_ELITE_FOUR:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE);
        break;
    case FLAG_DEFEATED_ELITE_4_SIDNEY:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_SIDNEY);
        break;
    case FLAG_DEFEATED_ELITE_4_PHOEBE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_PHOEBE);
        break;
    case FLAG_DEFEATED_ELITE_4_GLACIA:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_GLACIA);
        break;
    case FLAG_DEFEATED_ELITE_4_DRAKE:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_DRAKE);
        break;
    case FLAG_IS_CHAMPION:
    case FLAG_SYS_GAME_CLEAR:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION);
        break;
    case FLAG_RECEIVED_SS_TICKET:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SS_TICKET);
        break;
    case FLAG_MET_SCOTT_ON_SS_TIDAL:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL);
        break;
    case FLAG_SYS_FRONTIER_PASS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER);
        break;
    case FLAG_SCOTT_GIVES_BATTLE_POINTS:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME);
        break;
    case FLAG_DEFEATED_METEOR_FALLS_STEVEN:
        PgrProgress_TryCompleteRegisteredStoryEvent(PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS);
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

void PgrProgress_ScriptCheckHoennWallyCatchingTutorial(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL);
}

void PgrProgress_ScriptCompleteHoennWallyCatchingTutorial(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL);
}

void PgrProgress_ScriptGetHoennPetalburgScottOpportunity(void)
{
    if (VarGet(VAR_SCOTT_PETALBURG_ENCOUNTER) != 0
     || PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_ROXANNE))
        gSpecialVar_Result = 2; // Expired or already completed.
    else if (PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL))
        gSpecialVar_Result = 1; // Available.
    else
        gSpecialVar_Result = 0; // Not available yet.
}

void PgrProgress_ScriptShouldShowHoennRustboroSchoolScott(void)
{
    gSpecialVar_Result = VarGet(VAR_SCOTT_PETALBURG_ENCOUNTER) != 0
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS);
}

void PgrProgress_ScriptCheckHoennRoxanne(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_ROXANNE);
}

void PgrProgress_ScriptCheckHoennBrawly(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_BRAWLY);
}

void PgrProgress_ScriptCheckHoennDevonGoodsTheft(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEVON_GOODS_STOLEN);
}

void PgrProgress_ScriptCheckHoennDevonGoodsRecovery(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECOVERED_DEVON_GOODS);
}

void PgrProgress_ScriptCheckHoennDevonGoodsReturn(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RETURNED_DEVON_GOODS);
}

void PgrProgress_ScriptShouldShowHoennRustboroRival(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS)
        && VarGet(VAR_ROUTE104_STATE) < 2
        && VarGet(VAR_BOARD_BRINEY_BOAT_STATE) < 1;
}

void PgrProgress_ScriptCanMeetHoennRoute104Rival(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS)
        && VarGet(VAR_BOARD_BRINEY_BOAT_STATE) < 1;
}

void PgrProgress_ScriptCheckHoennStevenLetter(void)
{
    gSpecialVar_Result = CheckBagHasItem(ITEM_LETTER, 1)
        && PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER);
}

void PgrProgress_ScriptCheckHoennDock(void)
{
    gSpecialVar_Result = CheckBagHasItem(ITEM_DEVON_PARTS, 1)
        && PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN);
}

void PgrProgress_ScriptCheckHoennStern(void)
{
    gSpecialVar_Result = CheckBagHasItem(ITEM_DEVON_PARTS, 1)
        && PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS);
}

void PgrProgress_ScriptGetHoennSlateportScottOpportunity(void)
{
    u16 state = VarGet(VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE);

    if (state >= 3)
    {
        gSpecialVar_Result = 3; // Completed or expired.
    }
    else if (!PgrProgress_IsStoryEventComplete(
                 PGW_START_HOENN, PGR_HOENN_STORY_DELIVERED_DEVON_GOODS))
    {
        gSpecialVar_Result = 0; // Not available yet.
    }
    else if (state == 1)
    {
        gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
                PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110)
            ? 3 : 1; // Museum scene.
    }
    else if (state == 2)
    {
        gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
                PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE)
            ? 3 : 2; // Battle Tent scene.
    }
    else
    {
        gSpecialVar_Result = 0;
    }
}

void PgrProgress_ScriptCheckHoennRoute110Rival(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110);
}

void PgrProgress_ScriptCompleteHoennRoute110Rival(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110);
}

void PgrProgress_ScriptCheckHoennWallyMauville(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE);
}

void PgrProgress_ScriptCheckHoennWattson(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON);
}

void PgrProgress_ScriptShouldShowHoennVerdanturfScott(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT)
        && FlagGet(FLAG_HIDE_FALLARBOR_TOWN_BATTLE_TENT_SCOTT);
}

void PgrProgress_ScriptCanMoveHoennScottToFallarbor(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT);
}

void PgrProgress_ScriptShouldShowHoennFallarborScott(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WATTSON)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT)
        && !FlagGet(FLAG_HIDE_FALLARBOR_TOWN_BATTLE_TENT_SCOTT);
}

void PgrProgress_ScriptCheckHoennMeteoriteTheft(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT);
}

void PgrProgress_ScriptCheckHoennMaxieMtChimney(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY);
}

void PgrProgress_ScriptCheckHoennFlannery(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_FLANNERY);
}

void PgrProgress_ScriptCheckHoennGoGoggles(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_GO_GOGGLES);
}

void PgrProgress_ScriptCheckHoennNorman(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_NORMAN);
}

void PgrProgress_ScriptCheckHoennStevenRoute118(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_MET_STEVEN_ROUTE_118);
}

void PgrProgress_ScriptCompleteHoennStevenRoute118(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_MET_STEVEN_ROUTE_118);
}

void PgrProgress_ScriptCheckHoennWeatherInstitute(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE);
}

void PgrProgress_ScriptCompleteHoennWeatherInstitute(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE);
}

void PgrProgress_ScriptCheckHoennRoute119Rival(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119);
}

void PgrProgress_ScriptCompleteHoennRoute119Rival(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119);
}

void PgrProgress_ScriptShouldShowHoennLilycoveRival(void)
{
    gSpecialVar_Result = !FlagGet(FLAG_MET_RIVAL_LILYCOVE)
        && PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT);
}

void PgrProgress_ScriptShouldShowHoennLilycoveScott(void)
{
    gSpecialVar_Result = !FlagGet(FLAG_MET_SCOTT_IN_LILYCOVE)
        && PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT);
}

void PgrProgress_ScriptCheckHoennDevonScope(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE);
}

void PgrProgress_ScriptCompleteHoennDevonScope(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE);
}

void PgrProgress_ScriptCheckHoennFortreeGymPath(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH);
}

void PgrProgress_ScriptCheckHoennWinona(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WINONA);
}

void PgrProgress_ScriptCheckHoennMtPyreOrbTheft(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT);
}

void PgrProgress_ScriptCompleteHoennMtPyreOrbTheft(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT);
}

void PgrProgress_ScriptCheckHoennMagmaHideout(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT);
}

void PgrProgress_ScriptCompleteHoennMagmaHideout(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT);
}

void PgrProgress_ScriptCheckHoennSubmarineTheft(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT);
}

void PgrProgress_ScriptCompleteHoennSubmarineTheft(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT);
}

void PgrProgress_ScriptCheckHoennAquaHideout(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT);
}

void PgrProgress_ScriptCompleteHoennAquaHideout(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT);
}

void PgrProgress_ScriptCheckHoennTateLiza(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_TATE_LIZA);
}

void PgrProgress_ScriptShouldShowHoennMossdeepScott(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_MOSSDEEP)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER);
}

void PgrProgress_ScriptCompleteHoennMossdeepScott(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_MOSSDEEP);
}

void PgrProgress_ScriptCheckHoennMossdeepSpaceCenter(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER);
}

void PgrProgress_ScriptCompleteHoennMossdeepSpaceCenter(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER);
}

void PgrProgress_ScriptCheckHoennDiveFromSteven(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN);
}

void PgrProgress_ScriptCompleteHoennDiveFromSteven(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN);
}

void PgrProgress_ScriptCheckHoennSeafloorCavern(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN);
}

void PgrProgress_ScriptCompleteHoennSeafloorCavern(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN);
}

void PgrProgress_ScriptCheckHoennSootopolisCrisis(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS);
}

void PgrProgress_ScriptCompleteHoennSootopolisCrisis(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS);
}

void PgrProgress_ScriptCheckHoennWallaceRayquaza(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR);
}

void PgrProgress_ScriptCompleteHoennWallaceRayquaza(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR);
}

void PgrProgress_ScriptCheckHoennSkyPillarOpening(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_OPENED_SKY_PILLAR);
}

void PgrProgress_ScriptCompleteHoennSkyPillarOpening(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_OPENED_SKY_PILLAR);
}

void PgrProgress_ScriptCheckHoennRayquazaAwakening(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_RAYQUAZA);
}

void PgrProgress_ScriptCompleteHoennRayquazaAwakening(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_AWAKENED_RAYQUAZA);
}

void PgrProgress_ScriptCheckHoennSootopolisResolution(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS);
}

void PgrProgress_ScriptCompleteHoennSootopolisResolution(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS);
}

void PgrProgress_ScriptCheckHoennWaterfallFromWallace(void)
{
    gSpecialVar_Result = FlagGet(FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE)
        && PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE);
}

void PgrProgress_ScriptCompleteHoennWaterfallFromWallace(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE);
}

void PgrProgress_ScriptCheckHoennJuan(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_JUAN);
}

void PgrProgress_ScriptCheckHoennWallyVictoryRoad(void)
{
    gSpecialVar_Result = PgrProgress_CanStartRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD);
}

void PgrProgress_ScriptShouldShowHoennEverGrandeScott(void)
{
    gSpecialVar_Result = !FlagGet(FLAG_MET_SCOTT_IN_EVERGRANDE)
        && PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_JUAN)
        && !PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE);
}

void PgrProgress_ScriptCheckHoennLeagueAccess(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD)
        && PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_BRAWLY);
}

void PgrProgress_ScriptCompleteHoennLeagueEntry(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE);
}

void PgrProgress_ScriptCheckHoennSidney(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE);
}

void PgrProgress_ScriptCheckHoennPhoebe(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_SIDNEY);
}

void PgrProgress_ScriptCheckHoennGlacia(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_PHOEBE);
}

void PgrProgress_ScriptCheckHoennDrake(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_GLACIA);
}

void PgrProgress_ScriptCheckHoennChampion(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_DRAKE);
}

void PgrProgress_ScriptGetHoennPostgameResearchOpportunity(void)
{
    if (PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE))
    {
        gSpecialVar_Result = 2; // Already completed.
    }
    else if (PgrProgress_CanStartRegisteredStoryEvent(
                 PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE))
    {
        gSpecialVar_Result = 1; // Available after becoming Hoenn Champion.
    }
    else
    {
        gSpecialVar_Result = 0; // Not available yet.
    }
}

void PgrProgress_ScriptCompleteHoennPostgameResearchUpdate(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE);
}

void PgrProgress_ScriptCompleteHoennSSTicket(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SS_TICKET);
}

void PgrProgress_ScriptCanUseHoennSSTidal(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SS_TICKET)
        && PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_BECAME_CHAMPION)
        && FlagGet(FLAG_RECEIVED_SS_TICKET)
        && CheckBagHasItem(ITEM_SS_TICKET, 1);
}

void PgrProgress_ScriptGetHoennSSTidalScottOpportunity(void)
{
    if (PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL))
    {
        gSpecialVar_Result = 2; // Already completed.
    }
    else if (PgrProgress_CanStartRegisteredStoryEvent(
                 PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL)
          && FlagGet(FLAG_RECEIVED_SS_TICKET)
          && CheckBagHasItem(ITEM_SS_TICKET, 1))
    {
        gSpecialVar_Result = 1; // Available.
    }
    else
    {
        gSpecialVar_Result = 0; // Not available yet.
    }
}

void PgrProgress_ScriptCompleteHoennSSTidalScott(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL);
}

void PgrProgress_ScriptCanStartHoennBattleFrontierReception(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER)
        || PgrProgress_CanStartRegisteredStoryEvent(
            PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER);
}

void PgrProgress_ScriptCompleteHoennBattleFrontierReception(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER);
}

void PgrProgress_ScriptHasEnteredHoennBattleFrontier(void)
{
    gSpecialVar_Result = PgrProgress_IsStoryEventComplete(
        PGW_START_HOENN, PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER);
}

void PgrProgress_ScriptGetHoennScottHouseOpportunity(void)
{
    if (PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME))
    {
        gSpecialVar_Result = 2; // Initial Battle Points already received.
    }
    else if (PgrProgress_CanStartRegisteredStoryEvent(
                 PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME))
    {
        gSpecialVar_Result = 1; // First house conversation is available.
    }
    else
    {
        gSpecialVar_Result = 0; // Reception has not been completed.
    }
}

void PgrProgress_ScriptCompleteHoennScottHouseWelcome(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME);
}

void PgrProgress_ScriptGetHoennMeteorFallsStevenOpportunity(void)
{
    if (PgrProgress_IsStoryEventComplete(
            PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS))
    {
        gSpecialVar_Result = 2; // Battle already won.
    }
    else if (PgrProgress_CanStartRegisteredStoryEvent(
                 PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS))
    {
        gSpecialVar_Result = 1; // Postgame challenge is available.
    }
    else
    {
        gSpecialVar_Result = 0; // Not Hoenn Champion yet.
    }
}

void PgrProgress_ScriptCompleteHoennMeteorFallsSteven(void)
{
    gSpecialVar_Result = PgrProgress_TryCompleteRegisteredStoryEvent(
        PGW_START_HOENN, PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS);
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
