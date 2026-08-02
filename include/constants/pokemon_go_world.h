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

enum PgwSeason
{
    PGW_SEASON_SPRING,
    PGW_SEASON_SUMMER,
    PGW_SEASON_AUTUMN,
    PGW_SEASON_WINTER,
    PGW_SEASON_COUNT,
};

#define PGW_DEFAULT_STARTING_REGION PGW_START_HOENN
#define PGW_DEFAULT_SEASON          PGW_SEASON_SPRING
#define PGW_DAYS_PER_SEASON         28

#endif // GUARD_CONSTANTS_POKEMON_GO_WORLD_H
