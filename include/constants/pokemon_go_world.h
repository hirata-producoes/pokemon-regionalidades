#ifndef GUARD_CONSTANTS_POKEMON_GO_WORLD_H
#define GUARD_CONSTANTS_POKEMON_GO_WORLD_H

// Regions that can be selected when starting a new adventure.
// Orange Islands is separate because it is a custom world region rather than
// one of the core-series regions from constants/regions.h.
enum PgwStartingRegion
{
    PGW_START_KANTO,
    PGW_START_JOHTO,
    PGW_START_HOENN,
    PGW_START_SINNOH,
    PGW_START_UNOVA,
    PGW_START_KALOS,
    PGW_START_ALOLA,
    PGW_START_GALAR,
    PGW_START_HISUI,
    PGW_START_PALDEA,
    PGW_START_ORANGE_ISLANDS,
    PGW_START_REGION_COUNT,
};

// The first four identifiers form the connected world planned for V1.
// Later identifiers remain available as content references, but do not own
// regional story state until they are promoted into the playable world.
#define PGR_WORLD_REGION_COUNT 4

// Persistent progress capacity. Event and reward identifiers are deliberately
// local to this project instead of consuming Emerald's shared flag namespace.
#define PGR_STORY_EVENT_COUNT          128
#define PGR_STORY_EVENT_WORD_COUNT     (PGR_STORY_EVENT_COUNT / 32)
#define PGR_UNIQUE_REWARD_COUNT         64
#define PGR_UNIQUE_REWARD_WORD_COUNT   (PGR_UNIQUE_REWARD_COUNT / 32)

enum PgwSeason
{
    PGW_SEASON_SPRING,
    PGW_SEASON_SUMMER,
    PGW_SEASON_AUTUMN,
    PGW_SEASON_WINTER,
    PGW_SEASON_COUNT,
};

// A four-day transition crosses the boundary between two seasons: the final
// two days of the old season and the first two days of the new one.
enum PgwSeasonPhase
{
    PGW_SEASON_PHASE_ESTABLISHED,
    PGW_SEASON_PHASE_TRANSITION_OUT,
    PGW_SEASON_PHASE_TRANSITION_IN,
};

// Logical climate states are deliberately independent from field-weather
// effects. A biome profile may translate the same state differently (for
// example, precipitation can become rain or snow) without conflating climate,
// season and biome.
enum PgwClimate
{
    PGW_CLIMATE_CLEAR,
    PGW_CLIMATE_CLOUDY,
    PGW_CLIMATE_RAIN,
    PGW_CLIMATE_STORM,
    PGW_CLIMATE_FOG,
    PGW_CLIMATE_WIND,
    PGW_CLIMATE_COUNT,
};

#define PGW_DEFAULT_STARTING_REGION PGW_START_HOENN
#define PGW_DEFAULT_SEASON          PGW_SEASON_SPRING
#define PGW_WORLD_CLOCK_REALTIME_MULTIPLIER 3
#define PGW_DAYS_PER_SEASON                 30
#define PGW_SEASON_TRANSITION_HALF_DAYS      2
#define PGW_CLIMATE_SLOT_HOURS                6
#define PGW_CLIMATE_FORECAST_HOURS           24

#endif // GUARD_CONSTANTS_POKEMON_GO_WORLD_H
