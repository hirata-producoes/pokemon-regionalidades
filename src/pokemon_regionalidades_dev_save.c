#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "platform.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_regionalidades_dev_save.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/pokedex.h"
#include "constants/species.h"

#if defined(PORTABLE) && defined(PLATFORM_SDL2)

struct PgrDevelopmentMon
{
    enum Species species;
    enum Move moves[MAX_MON_MOVES];
};

static const struct PgrDevelopmentMon sMobilityParty[] =
{
    { SPECIES_TROPIUS, { MOVE_CUT,  MOVE_FLY,       MOVE_STRENGTH, MOVE_ROCK_SMASH } },
    { SPECIES_PELIPPER,{ MOVE_FLY,  MOVE_SURF,      MOVE_PROTECT,  MOVE_AERIAL_ACE } },
    { SPECIES_WAILORD, { MOVE_SURF, MOVE_WATERFALL, MOVE_DIVE,     MOVE_STRENGTH } },
    { SPECIES_LANTURN, { MOVE_SURF, MOVE_DIVE,      MOVE_WATERFALL,MOVE_FLASH } },
    { SPECIES_BRELOOM, { MOVE_CUT,  MOVE_STRENGTH,  MOVE_ROCK_SMASH, MOVE_FLASH } },
};

static const u16 sHoennGymLeaders[] =
{
    TRAINER_ROXANNE_1,
    TRAINER_BRAWLY_1,
    TRAINER_WATTSON_1,
    TRAINER_FLANNERY_1,
    TRAINER_NORMAN_1,
    TRAINER_WINONA_1,
    TRAINER_TATE_AND_LIZA_1,
    TRAINER_JUAN_1,
};

#define PGR_TEST_BALL_MIN_QUANTITY 100

// Ordinary capture balls that can be used from the normal Bag interface.
// Safari, Sport, Park, Beast and Cherish Balls are deliberately excluded:
// some require a special encounter context and the current gameplay view
// still exposes the original 16 slots in the Poke Ball pocket.
static const enum Item sTestProfilePokeBalls[] =
{
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_MASTER_BALL,
    ITEM_PREMIER_BALL,
    ITEM_HEAL_BALL,
    ITEM_NET_BALL,
    ITEM_NEST_BALL,
    ITEM_DIVE_BALL,
    ITEM_DUSK_BALL,
    ITEM_TIMER_BALL,
    ITEM_QUICK_BALL,
    ITEM_REPEAT_BALL,
    ITEM_LUXURY_BALL,
};

bool32 Pgr_IsMobilityProfileRequested(void)
{
    return Platform_GetEnvironmentFlag("POKEMON_REGIONALIDADES_DEV_SESSION")
        && Platform_GetEnvironmentFlag("POKEMON_REGIONALIDADES_APPLY_MOBILITY_PROFILE");
}

bool32 Pgr_IsTechnicalMobilityProfile(void)
{
    return Platform_GetEnvironmentFlag("POKEMON_REGIONALIDADES_TECHNICAL_MOBILITY");
}

static bool32 RestoreNarrativeBadges(void)
{
    bool32 changed = FALSE;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sHoennGymLeaders); i++)
    {
        u16 badgeFlag = FLAG_BADGE01_GET + i;
        bool32 shouldHaveBadge = HasTrainerBeenFought(sHoennGymLeaders[i]);

        if (FlagGet(badgeFlag) != shouldHaveBadge)
        {
            if (shouldHaveBadge)
                FlagSet(badgeFlag);
            else
                FlagClear(badgeFlag);
            changed = TRUE;
        }
    }

    if (changed)
        DBGPRINTF("Development save: restored story badges from defeated Hoenn leaders\n");
    return changed;
}

u8 Pgr_GetNarrativeBadgeCount(void)
{
    u8 count = 0;
    u32 i;

    if (Pgr_IsTechnicalMobilityProfile() && !IS_FRLG)
    {
        for (i = 0; i < ARRAY_COUNT(sHoennGymLeaders); i++)
        {
            if (HasTrainerBeenFought(sHoennGymLeaders[i]))
                count++;
        }
    }
    else
    {
        for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        {
            if (FlagGet(i))
                count++;
        }
    }
    return count;
}

static bool32 PrepareMobilityParty(void)
{
    u32 monIndex;
    u32 moveIndex;
    bool32 addParty = FALSE;

    CalculatePlayerPartyCount();
    if (GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES) != SPECIES_MUDKIP)
    {
        DBGPRINTF("Development save: refused: expected Mudkip in slot 1\n");
        return FALSE;
    }

    if (gPartiesCount[B_TRAINER_PLAYER] == 1)
    {
        addParty = TRUE;
    }
    else if (gPartiesCount[B_TRAINER_PLAYER] == PARTY_SIZE)
    {
        for (monIndex = 0; monIndex < ARRAY_COUNT(sMobilityParty); monIndex++)
        {
            if (GetMonData(&gParties[B_TRAINER_PLAYER][monIndex + 1], MON_DATA_SPECIES)
             != sMobilityParty[monIndex].species)
            {
                DBGPRINTF("Development save: refused: slot %u is not the mobility party\n", monIndex + 2);
                return FALSE;
            }
        }
    }
    else
    {
        DBGPRINTF("Development save: refused: expected 1 or 6 Pokemon, found %u\n",
            gPartiesCount[B_TRAINER_PLAYER]);
        return FALSE;
    }

    for (monIndex = 0; monIndex < ARRAY_COUNT(sMobilityParty); monIndex++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][monIndex + 1];

        if (addParty)
        {
            CreateMon(mon,
                      sMobilityParty[monIndex].species,
                      75,
                      0x52454700 + monIndex,
                      OTID_STRUCT_PLAYER_ID);
            for (moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
                SetMonMoveSlot(mon, sMobilityParty[monIndex].moves[moveIndex], moveIndex);
            CalculateMonStats(mon);
            DBGPRINTF("Development save: added species=%u level=%u maxHP=%u\n",
                sMobilityParty[monIndex].species, GetMonData(mon, MON_DATA_LEVEL), GetMonData(mon, MON_DATA_MAX_HP));
        }

        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sMobilityParty[monIndex].species), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sMobilityParty[monIndex].species), FLAG_SET_CAUGHT);
    }
    CalculatePlayerPartyCount();
    DBGPRINTF("Development save: preserved starter species=%u level=%u; party=%u; trainer=%02X\n",
        GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES),
        GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL),
        gPartiesCount[B_TRAINER_PLAYER], gSaveBlock2Ptr->playerName[0]);
    return TRUE;
}

static bool32 PrepareTestResources(void)
{
    bool32 changed = FALSE;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sTestProfilePokeBalls); i++)
    {
        enum Item itemId = sTestProfilePokeBalls[i];
        u32 owned = CountTotalItemQuantityInBag(itemId);

        if (owned >= PGR_TEST_BALL_MIN_QUANTITY)
            continue;
        if (AddBagItem(itemId, PGR_TEST_BALL_MIN_QUANTITY - owned))
        {
            changed = TRUE;
        }
        else
        {
            DBGPRINTF("Development save: no room for test ball item=%u\n", itemId);
        }
    }

    if (GetMoney(&gSaveBlock1Ptr->money) < MAX_MONEY)
    {
        SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
        changed = TRUE;
    }

    if (changed)
        DBGPRINTF("Development save: test resources prepared (14 ball types x100 minimum, money=%u)\n",
            GetMoney(&gSaveBlock1Ptr->money));
    return changed;
}

bool32 Pgr_ApplyMobilityProfileIfRequested(void)
{
    static bool32 sAppliedThisProcess;
    u32 flag;
    bool32 changed = FALSE;

    if (sAppliedThisProcess || IS_FRLG)
        return FALSE;

    if (!Pgr_IsMobilityProfileRequested() && !Pgr_IsTechnicalMobilityProfile())
        return FALSE;

    sAppliedThisProcess = TRUE;
    if (Pgr_IsTechnicalMobilityProfile())
    {
        changed |= RestoreNarrativeBadges();
        changed |= PrepareTestResources();
    }

    if (!Pgr_IsMobilityProfileRequested())
        return changed;

    if (!PrepareMobilityParty())
        return changed;

    // These are development capabilities, not proof of narrative progress.
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_B_DASH);

    for (flag = FLAG_VISITED_LITTLEROOT_TOWN; flag <= FLAG_VISITED_EVER_GRANDE_CITY; flag++)
        FlagSet(flag);

    FlagSet(FLAG_LANDMARK_POKEMON_LEAGUE);
    FlagSet(FLAG_LANDMARK_BATTLE_FRONTIER);

    DBGPRINTF("Development save: mobility profile applied (5 level-75 Pokemon, all HMs, all Hoenn Fly destinations, no artificial badges)\n");
    return TRUE;
}

#else

bool32 Pgr_IsMobilityProfileRequested(void)
{
    return FALSE;
}

bool32 Pgr_IsTechnicalMobilityProfile(void)
{
    return FALSE;
}

u8 Pgr_GetNarrativeBadgeCount(void)
{
    u8 count = 0;
    u32 flag;

    for (flag = FLAG_BADGE01_GET; flag <= FLAG_BADGE08_GET; flag++)
    {
        if (FlagGet(flag))
            count++;
    }
    return count;
}

bool32 Pgr_ApplyMobilityProfileIfRequested(void)
{
    return FALSE;
}

#endif
