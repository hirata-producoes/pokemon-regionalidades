#include "global.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "pokemon_go_world.h"
#include "pokemon_regionalidades_dex.h"
#include "pokemon_regionalidades_progress.h"
#include "rtc.h"

static const u8 sText_SeasonSpring[] = _("PRIMAVERA");
static const u8 sText_SeasonSummer[] = _("VERAO");
static const u8 sText_SeasonAutumn[] = _("OUTONO");
static const u8 sText_SeasonWinter[] = _("INVERNO");
static const u8 sText_ClimateClear[] = _("ABERTO");
static const u8 sText_ClimateCloudy[] = _("NUBLADO");
static const u8 sText_ClimateRain[] = _("CHUVA");
static const u8 sText_ClimateStorm[] = _("TEMPESTADE");
static const u8 sText_ClimateFog[] = _("NEBLINA");
static const u8 sText_ClimateWind[] = _("VENTO");

static const u8 *const sSeasonNames[PGW_SEASON_COUNT] =
{
    [PGW_SEASON_SPRING] = sText_SeasonSpring,
    [PGW_SEASON_SUMMER] = sText_SeasonSummer,
    [PGW_SEASON_AUTUMN] = sText_SeasonAutumn,
    [PGW_SEASON_WINTER] = sText_SeasonWinter,
};

static const u8 *const sClimateNames[PGW_CLIMATE_COUNT] =
{
    [PGW_CLIMATE_CLEAR] = sText_ClimateClear,
    [PGW_CLIMATE_CLOUDY] = sText_ClimateCloudy,
    [PGW_CLIMATE_RAIN] = sText_ClimateRain,
    [PGW_CLIMATE_STORM] = sText_ClimateStorm,
    [PGW_CLIMATE_FOG] = sText_ClimateFog,
    [PGW_CLIMATE_WIND] = sText_ClimateWind,
};

// Keep this zero-initialized so it lives in BSS on the GBA target.
// Zero means no explicit selection; stored selections use region + 1.
static u8 sNewGameStartingRegionPlusOne;

static bool32 GetRealTimeSeconds(u32 *seconds)
{
    struct SiiRtcInfo rtc = {0};

    RtcGetRawInfo(&rtc);
    return Pgw_TryConvertRealRtcToSeconds(&rtc, seconds);
}

static u32 GetRealTimeAnchor(void)
{
    return (u32)VarGet(VAR_PGW_REAL_TIME_ANCHOR_LO)
         | ((u32)VarGet(VAR_PGW_REAL_TIME_ANCHOR_HI) << 16);
}

static void SetRealTimeAnchor(u32 seconds)
{
    VarSet(VAR_PGW_REAL_TIME_ANCHOR_LO, (u16)seconds);
    VarSet(VAR_PGW_REAL_TIME_ANCHOR_HI, (u16)(seconds >> 16));
}

void Pgw_InitWorldState(void)
{
    u16 weatherSeed = gSaveBlock1Ptr->dailySeed ^ (gSaveBlock1Ptr->dailySeed >> 16);
    enum PgwStartingRegion startingRegion = PGW_DEFAULT_STARTING_REGION;

    if (weatherSeed == 0)
        weatherSeed = 1;

    if (sNewGameStartingRegionPlusOne != 0)
        startingRegion = sNewGameStartingRegionPlusOne - 1;

    VarSet(VAR_PGW_STARTING_REGION, startingRegion);
    VarSet(VAR_PGW_CURRENT_REGION, startingRegion);
    sNewGameStartingRegionPlusOne = 0;
    VarSet(VAR_PGW_SEASON, PGW_DEFAULT_SEASON);
    VarSet(VAR_PGW_SEASON_DAY, 1);
    VarSet(VAR_PGW_WEATHER_SEED, weatherSeed);
    VarSet(VAR_PGW_WORLD_LEVEL, 0);
    PgwDex_ResetResearch();
    PgrProgress_Reset();
    Pgw_SnapshotWorldClockRealTime();
}

void Pgw_SelectStartingRegionForNewGame(enum PgwStartingRegion region)
{
    if (region >= PGW_START_REGION_COUNT)
        region = PGW_DEFAULT_STARTING_REGION;
    sNewGameStartingRegionPlusOne = region + 1;
}

enum PgwStartingRegion Pgw_GetSelectedStartingRegionForNewGame(void)
{
    if (sNewGameStartingRegionPlusOne == 0)
        return PGW_DEFAULT_STARTING_REGION;
    return sNewGameStartingRegionPlusOne - 1;
}

void Pgw_SnapshotWorldClockRealTime(void)
{
    u32 realSeconds;

    if (GetRealTimeSeconds(&realSeconds))
        SetRealTimeAnchor(realSeconds);
}

void Pgw_ApplyOfflineWorldClock(void)
{
    struct Time internalElapsed;
    u32 previousSeconds = GetRealTimeAnchor();
    u32 currentSeconds;

    if (!GetRealTimeSeconds(&currentSeconds))
        return;

    SetRealTimeAnchor(currentSeconds);
    if (previousSeconds == 0 || currentSeconds <= previousSeconds)
        return;

    Pgw_CalculateInternalElapsed(currentSeconds - previousSeconds, &internalElapsed);
    FakeRtc_AdvanceTimeBy(internalElapsed.days,
                          internalElapsed.hours,
                          internalElapsed.minutes,
                          internalElapsed.seconds);
}

void Pgw_AdvanceWorldDays(u16 days)
{
    enum PgwSeason season;
    u16 seasonDay;
    u16 seed;

    if (days == 0)
        return;

    season = Pgw_GetSeason();
    seasonDay = Pgw_GetSeasonDay();
    Pgw_CalculateSeasonAfterDays(season, seasonDay, days, &season, &seasonDay);

    VarSet(VAR_PGW_SEASON, season);
    VarSet(VAR_PGW_SEASON_DAY, seasonDay);

    seed = Pgw_CalculateWeatherSeedAfterDays(VarGet(VAR_PGW_WEATHER_SEED), days);
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

const u8 *Pgw_GetSeasonName(enum PgwSeason season)
{
    if (season >= PGW_SEASON_COUNT)
        season = PGW_DEFAULT_SEASON;
    return sSeasonNames[season];
}

u16 Pgw_GetSeasonDay(void)
{
    u16 seasonDay = VarGet(VAR_PGW_SEASON_DAY);

    if (seasonDay == 0 || seasonDay > PGW_DAYS_PER_SEASON)
        return 1;
    return seasonDay;
}

enum PgwSeasonPhase Pgw_GetSeasonPhase(void)
{
    return Pgw_GetSeasonPhaseForDay(Pgw_GetSeasonDay());
}

enum PgwClimate Pgw_GetCurrentClimate(u16 locationId)
{
    RtcCalcLocalTime();
    return Pgw_CalculateClimate(VarGet(VAR_PGW_WEATHER_SEED),
                                Pgw_GetSeason(),
                                Pgw_GetSeasonDay(),
                                Pgw_GetCurrentRegion(),
                                locationId,
                                gLocalTime.hours,
                                0);
}

const u8 *Pgw_GetClimateName(enum PgwClimate climate)
{
    if (climate >= PGW_CLIMATE_COUNT)
        climate = PGW_CLIMATE_CLEAR;
    return sClimateNames[climate];
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
