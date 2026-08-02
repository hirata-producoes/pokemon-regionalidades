#include "global.h"
#include "event_data.h"
#include "pokemon_go_world.h"

static u16 AdvanceWeatherSeed(u16 seed)
{
    return seed * 25173 + 13849;
}

void Pgw_InitWorldState(void)
{
    u16 weatherSeed = gSaveBlock1Ptr->dailySeed ^ (gSaveBlock1Ptr->dailySeed >> 16);

    if (weatherSeed == 0)
        weatherSeed = 1;

    VarSet(VAR_PGW_STARTING_REGION, PGW_DEFAULT_STARTING_REGION);
    VarSet(VAR_PGW_CURRENT_REGION, PGW_DEFAULT_STARTING_REGION);
    VarSet(VAR_PGW_SEASON, PGW_DEFAULT_SEASON);
    VarSet(VAR_PGW_SEASON_DAY, 1);
    VarSet(VAR_PGW_WEATHER_SEED, weatherSeed);
    VarSet(VAR_PGW_WORLD_LEVEL, 0);
}

void Pgw_AdvanceWorldDays(u16 days)
{
    u32 elapsedDays;
    u16 season;
    u16 seasonDay;
    u16 seed;

    if (days == 0)
        return;

    season = Pgw_GetSeason();
    seasonDay = VarGet(VAR_PGW_SEASON_DAY);
    if (seasonDay == 0 || seasonDay > PGW_DAYS_PER_SEASON)
        seasonDay = 1;
    elapsedDays = seasonDay - 1 + days;
    season = (season + elapsedDays / PGW_DAYS_PER_SEASON) % PGW_SEASON_COUNT;

    VarSet(VAR_PGW_SEASON, season);
    VarSet(VAR_PGW_SEASON_DAY, elapsedDays % PGW_DAYS_PER_SEASON + 1);

    seed = VarGet(VAR_PGW_WEATHER_SEED);
    while (days--)
        seed = AdvanceWeatherSeed(seed);
    VarSet(VAR_PGW_WEATHER_SEED, seed);
}

enum PgwStartingRegion Pgw_GetStartingRegion(void)
{
    u16 region = VarGet(VAR_PGW_STARTING_REGION);

    if (region >= PGW_START_REGION_COUNT)
        return PGW_DEFAULT_STARTING_REGION;
    return region;
}

void Pgw_SetStartingRegion(enum PgwStartingRegion region)
{
    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;

    VarSet(VAR_PGW_STARTING_REGION, region);
    Pgw_SetCurrentRegion(region);
}

enum PgwStartingRegion Pgw_GetCurrentRegion(void)
{
    u16 region = VarGet(VAR_PGW_CURRENT_REGION);

    if (region >= PGW_START_REGION_COUNT)
        return PGW_DEFAULT_STARTING_REGION;
    return region;
}

void Pgw_SetCurrentRegion(enum PgwStartingRegion region)
{
    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;
    VarSet(VAR_PGW_CURRENT_REGION, region);
}

enum PgwSeason Pgw_GetSeason(void)
{
    u16 season = VarGet(VAR_PGW_SEASON);

    if (season >= PGW_SEASON_COUNT)
        return PGW_DEFAULT_SEASON;
    return season;
}

void Pgw_ScriptSetStartingRegion(void)
{
    Pgw_SetStartingRegion(gSpecialVar_0x8004);
    gSpecialVar_Result = Pgw_GetStartingRegion();
}

void Pgw_ScriptSetCurrentRegion(void)
{
    Pgw_SetCurrentRegion(gSpecialVar_0x8004);
    gSpecialVar_Result = Pgw_GetCurrentRegion();
}

void Pgw_ScriptGetStartingRegion(void)
{
    gSpecialVar_Result = Pgw_GetStartingRegion();
}

void Pgw_ScriptGetSeason(void)
{
    gSpecialVar_Result = Pgw_GetSeason();
}
