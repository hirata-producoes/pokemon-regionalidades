#include "global.h"
#include "fake_rtc.h"
#include "pokemon_go_world.h"
#include "test/test.h"

TEST("Pokemon Regionalidades World Clock advances at three times real time")
{
    struct Time internalElapsed;

    EXPECT_EQ(FakeRtc_GetSecondsRatio(), PGW_WORLD_CLOCK_REALTIME_MULTIPLIER);
    EXPECT_EQ(FakeRtc_GetSecondsRatio(), 3);

    Pgw_CalculateInternalElapsed(1, &internalElapsed);
    EXPECT_EQ(internalElapsed.days, 0);
    EXPECT_EQ(internalElapsed.hours, 0);
    EXPECT_EQ(internalElapsed.minutes, 0);
    EXPECT_EQ(internalElapsed.seconds, 3);

    Pgw_CalculateInternalElapsed(30, &internalElapsed);
    EXPECT_EQ(internalElapsed.minutes, 1);
    EXPECT_EQ(internalElapsed.seconds, 30);

    Pgw_CalculateInternalElapsed(10 * 24 * 60 * 60, &internalElapsed);
    EXPECT_EQ(internalElapsed.days, 30);
    EXPECT_EQ(internalElapsed.hours, 0);
    EXPECT_EQ(internalElapsed.minutes, 0);
    EXPECT_EQ(internalElapsed.seconds, 0);
}

TEST("Pokemon Regionalidades explicitly pauses the World Clock in menus")
{
    struct SiiRtcInfo *rtc;

    FakeRtc_ManuallySetTime(0, 0, 0, 0);
    FakeRtc_SetMenuPaused(TRUE);
    EXPECT(FakeRtc_IsMenuPaused());
    FakeRtc_TickTimeForward();
    rtc = FakeRtc_GetCurrentTime();
    EXPECT_EQ(rtc->second, 0);

    FakeRtc_SetMenuPaused(FALSE);
    EXPECT(!FakeRtc_IsMenuPaused());
    FakeRtc_TickTimeForward();
    EXPECT_EQ(rtc->second, PGW_WORLD_CLOCK_REALTIME_MULTIPLIER);
}

TEST("Pokemon Regionalidades decodes the physical RTC independently from the fake clock")
{
    struct SiiRtcInfo rtc =
    {
        .year = 0x26,
        .month = 0x08,
        .day = 0x30,
        .hour = 0x12,
        .minute = 0x34,
        .second = 0x56,
        .status = SIIRTCINFO_24HOUR,
    };
    u32 first;
    u32 second;

    EXPECT(Pgw_TryConvertRealRtcToSeconds(&rtc, &first));
    rtc.second = 0x57;
    EXPECT(Pgw_TryConvertRealRtcToSeconds(&rtc, &second));
    EXPECT_EQ(second - first, 1);

    rtc.month = 0x13;
    EXPECT(!Pgw_TryConvertRealRtcToSeconds(&rtc, &second));
}

TEST("Pokemon Regionalidades season calendar crosses its 30-day boundary")
{
    enum PgwSeason season;
    u16 seasonDay;

    Pgw_CalculateSeasonAfterDays(PGW_SEASON_SPRING, 28, 1, &season, &seasonDay);
    EXPECT_EQ(season, PGW_SEASON_SPRING);
    EXPECT_EQ(seasonDay, 29);

    Pgw_CalculateSeasonAfterDays(PGW_SEASON_SPRING, 30, 1, &season, &seasonDay);
    EXPECT_EQ(season, PGW_SEASON_SUMMER);
    EXPECT_EQ(seasonDay, 1);

    Pgw_CalculateSeasonAfterDays(PGW_SEASON_WINTER, 30, 1, &season, &seasonDay);
    EXPECT_EQ(season, PGW_SEASON_SPRING);
    EXPECT_EQ(seasonDay, 1);
}

TEST("Pokemon Regionalidades season transition spans days 29-30 and 1-2")
{
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(28), PGW_SEASON_PHASE_ESTABLISHED);
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(29), PGW_SEASON_PHASE_TRANSITION_OUT);
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(30), PGW_SEASON_PHASE_TRANSITION_OUT);
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(1), PGW_SEASON_PHASE_TRANSITION_IN);
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(2), PGW_SEASON_PHASE_TRANSITION_IN);
    EXPECT_EQ(Pgw_GetSeasonPhaseForDay(3), PGW_SEASON_PHASE_ESTABLISHED);
}

TEST("Pokemon Regionalidades exposes stable season names for the environmental popup")
{
    EXPECT_NE(Pgw_GetSeasonName(PGW_SEASON_SPRING), Pgw_GetSeasonName(PGW_SEASON_SUMMER));
    EXPECT_NE(Pgw_GetSeasonName(PGW_SEASON_SUMMER), Pgw_GetSeasonName(PGW_SEASON_AUTUMN));
    EXPECT_NE(Pgw_GetSeasonName(PGW_SEASON_AUTUMN), Pgw_GetSeasonName(PGW_SEASON_WINTER));
    EXPECT_EQ(Pgw_GetSeasonName(PGW_SEASON_COUNT), Pgw_GetSeasonName(PGW_SEASON_SPRING));
}
