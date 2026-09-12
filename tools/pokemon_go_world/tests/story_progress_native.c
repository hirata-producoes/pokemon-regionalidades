// Standalone contract tests: real progress implementation, in-memory save and
// flag/item adapters. Does not open or modify player profiles.
#include "global.h"
#include "event_data.h"
#include "pokemon_regionalidades_progress.h"
#include <stdio.h>

static struct SaveBlock3 sSave;
struct SaveBlock3 *gSaveBlock3Ptr = &sSave;
static u8 sFlags[65536];
static u16 sVars[65536];
u16 gSpecialVar_Result;
u16 gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006;
u16 gSpecialVar_0x8007, gSpecialVar_0x8008;
bool8 FlagGet(u16 id) { return sFlags[id]; }
u16 VarGet(u16 id) { return sVars[id]; }
bool32 CheckBagHasItem(enum Item item, u32 count) { return FALSE; }
bool32 AddBagItem(enum Item item, u32 count) { return FALSE; }
#define CHECK(expr) do { if (!(expr)) { printf("FAIL line %d: %s\n", __LINE__, #expr); return 1; } } while (0)

int main(void)
{
    // Artificial movement permissions must not fabricate the opening story.
    sFlags[FLAG_SYS_B_DASH] = TRUE;
    sFlags[FLAG_RECEIVED_RUNNING_SHOES] = TRUE;
    PgrProgress_EnsureValid();
    CHECK(!PgrProgress_IsStoryEventComplete(PGW_START_HOENN, PGR_HOENN_STORY_RECEIVED_POKEDEX));
    PgrProgress_ScriptCheckHoennPokedex();
    CHECK(!gSpecialVar_Result);
    sFlags[FLAG_VISITED_LITTLEROOT_TOWN] = TRUE;
    PgrProgress_OnLegacyFlagSet(FLAG_VISITED_LITTLEROOT_TOWN);
    sFlags[FLAG_RESCUED_BIRCH] = TRUE;
    PgrProgress_OnLegacyFlagSet(FLAG_RESCUED_BIRCH);
    sFlags[FLAG_DEFEATED_RIVAL_ROUTE103] = TRUE;
    PgrProgress_OnLegacyFlagSet(FLAG_DEFEATED_RIVAL_ROUTE103);
    PgrProgress_ScriptCheckHoennPokedex();
    CHECK(gSpecialVar_Result);
    sFlags[FLAG_RECEIVED_POKEDEX_FROM_BIRCH] = TRUE;
    PgrProgress_OnLegacyFlagSet(FLAG_RECEIVED_POKEDEX_FROM_BIRCH);
    PgrProgress_ScriptCheckHoennPokedex();
    CHECK(!gSpecialVar_Result);
    PgrProgress_ScriptCheckHoennShoes();
    CHECK(gSpecialVar_Result);
    PgrProgress_OnLegacyFlagSet(FLAG_RECEIVED_RUNNING_SHOES);
    PgrProgress_ScriptCheckHoennShoes();
    CHECK(!gSpecialVar_Result);
    CHECK(!PgrProgress_IsStoryEventComplete((enum PgwStartingRegion)-1, 0));
    puts("PASS: Hoenn prerequisites, technical mobility, repeated scenes and legacy migration");
    return 0;
}
