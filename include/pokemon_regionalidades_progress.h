#ifndef GUARD_POKEMON_REGIONALIDADES_PROGRESS_H
#define GUARD_POKEMON_REGIONALIDADES_PROGRESS_H

#include "global.h"

enum PgrProgressResult
{
    PGR_PROGRESS_INVALID,
    PGR_PROGRESS_REQUIREMENTS_NOT_MET,
    PGR_PROGRESS_ALREADY_COMPLETE,
    PGR_PROGRESS_COMPLETED,
};

enum PgrRewardScope
{
    PGR_REWARD_GLOBAL,
    PGR_REWARD_REGIONAL,
};

enum PgrRewardResult
{
    PGR_REWARD_RESULT_INVALID,
    PGR_REWARD_RESULT_ALREADY_CLAIMED,
    PGR_REWARD_RESULT_NO_ROOM,
    PGR_REWARD_RESULT_GRANTED,
};

// Stable milestone identifiers for the existing Hoenn campaign. Other
// regions will define their own identifiers in the same 0..127 space.
enum PgrHoennStoryEvent
{
    PGR_HOENN_STORY_ARRIVED_LITTLEROOT,
    PGR_HOENN_STORY_RESCUED_BIRCH,
    PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_103,
    PGR_HOENN_STORY_RECEIVED_POKEDEX,
    PGR_HOENN_STORY_RECEIVED_RUNNING_SHOES,
    PGR_HOENN_STORY_DEFEATED_ROXANNE,
    PGR_HOENN_STORY_DEVON_GOODS_STOLEN,
    PGR_HOENN_STORY_RECOVERED_DEVON_GOODS,
    PGR_HOENN_STORY_RETURNED_DEVON_GOODS,
    PGR_HOENN_STORY_RECEIVED_DEVON_COMMISSIONS,
    PGR_HOENN_STORY_DELIVERED_STEVEN_LETTER,
    PGR_HOENN_STORY_DOCK_DIRECTED_TO_STERN,
    PGR_HOENN_STORY_DELIVERED_DEVON_GOODS,
    PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_110,
    PGR_HOENN_STORY_DEFEATED_WALLY_MAUVILLE,
    PGR_HOENN_STORY_DEFEATED_WATTSON,
    PGR_HOENN_STORY_WITNESSED_METEORITE_THEFT,
    PGR_HOENN_STORY_DEFEATED_MAXIE_MT_CHIMNEY,
    PGR_HOENN_STORY_DEFEATED_FLANNERY,
    PGR_HOENN_STORY_RECEIVED_GO_GOGGLES,
    PGR_HOENN_STORY_DEFEATED_NORMAN,
    PGR_HOENN_STORY_MET_STEVEN_ROUTE_118,
    PGR_HOENN_STORY_CLEARED_WEATHER_INSTITUTE,
    PGR_HOENN_STORY_DEFEATED_RIVAL_ROUTE_119,
    PGR_HOENN_STORY_RECEIVED_DEVON_SCOPE,
    PGR_HOENN_STORY_CLEARED_FORTREE_GYM_PATH,
    PGR_HOENN_STORY_DEFEATED_WINONA,
    PGR_HOENN_STORY_WITNESSED_MT_PYRE_ORB_THEFT,
    PGR_HOENN_STORY_AWAKENED_GROUDON_MAGMA_HIDEOUT,
    PGR_HOENN_STORY_WITNESSED_SUBMARINE_THEFT,
    PGR_HOENN_STORY_CLEARED_AQUA_HIDEOUT,
    PGR_HOENN_STORY_AWAKENED_KYOGRE_SEAFLOOR_CAVERN,
    PGR_HOENN_STORY_WITNESSED_SOOTOPOLIS_CRISIS,
    PGR_HOENN_STORY_SENT_WALLACE_TO_SKY_PILLAR,
    PGR_HOENN_STORY_OPENED_SKY_PILLAR,
    PGR_HOENN_STORY_AWAKENED_RAYQUAZA,
    PGR_HOENN_STORY_RESOLVED_SOOTOPOLIS_CRISIS,
    // Appended in v16 to keep every previously persisted identifier stable.
    PGR_HOENN_STORY_DEFEATED_TATE_LIZA,
    PGR_HOENN_STORY_CLEARED_MOSSDEEP_SPACE_CENTER,
    PGR_HOENN_STORY_RECEIVED_DIVE_FROM_STEVEN,
    // Appended in v17; do not reorder persisted identifiers.
    PGR_HOENN_STORY_RECEIVED_WATERFALL_FROM_WALLACE,
    PGR_HOENN_STORY_DEFEATED_JUAN,
    PGR_HOENN_STORY_DEFEATED_WALLY_VICTORY_ROAD,
    PGR_HOENN_STORY_ENTERED_POKEMON_LEAGUE,
    PGR_HOENN_STORY_DEFEATED_SIDNEY,
    PGR_HOENN_STORY_DEFEATED_PHOEBE,
    PGR_HOENN_STORY_DEFEATED_GLACIA,
    PGR_HOENN_STORY_DEFEATED_DRAKE,
    PGR_HOENN_STORY_BECAME_CHAMPION,
    // Appended in v18; Brawly is a flexible branch, not a linear insertion.
    PGR_HOENN_STORY_DEFEATED_BRAWLY,
    // Appended in v19 after the complete Hoenn audit found an opening gap.
    PGR_HOENN_STORY_COMPLETED_WALLY_CATCHING_TUTORIAL,
    // Appended in v20 so the optional Mossdeep meeting cannot repeat on reload.
    PGR_HOENN_STORY_MET_SCOTT_MOSSDEEP,
    // Appended in v21; this is a Hoenn research update, not a second Pokédex.
    PGR_HOENN_STORY_COMPLETED_POSTGAME_RESEARCH_UPDATE,
    // Appended in v22; keep the postgame ferry chain regional and ordered.
    PGR_HOENN_STORY_RECEIVED_SS_TICKET,
    PGR_HOENN_STORY_MET_SCOTT_SS_TIDAL,
    PGR_HOENN_STORY_ENTERED_BATTLE_FRONTIER,
    // Appended in v23; Scott's house reward follows the reception ceremony.
    PGR_HOENN_STORY_RECEIVED_SCOTT_FRONTIER_WELCOME,
    // Appended in v24; Steven's optional challenge exists only after the title.
    PGR_HOENN_STORY_DEFEATED_STEVEN_METEOR_FALLS,
};

// Reward identifiers describe the acquisition, not the concrete item. This
// lets a later regional professor give an upgrade or alternative reward
// without duplicating a unique device.
enum PgrUniqueReward
{
    PGR_REWARD_ROTOMDEX_DEVICE,
    PGR_REWARD_EXP_SHARE,
    PGR_REWARD_BICYCLE_ACCESS,
};

struct PgrStoryRequirement
{
    enum PgwStartingRegion region;
    u16 eventId;
};

void PgrProgress_Reset(void);
void PgrProgress_EnsureValid(void);
bool32 PgrProgress_OnSaveLoaded(void);
bool32 PgrProgress_StageNativeWorld(void);
void PgrProgress_OnLegacyFlagSet(u16 flagId);
bool32 PgrProgress_IsStoryEventComplete(enum PgwStartingRegion region, u16 eventId);
bool32 PgrProgress_AreRequirementsMet(const struct PgrStoryRequirement *requirements, u32 count);
enum PgrProgressResult PgrProgress_TryCompleteStoryEvent(enum PgwStartingRegion region, u16 eventId, const struct PgrStoryRequirement *requirements, u32 count);
bool32 PgrProgress_CanStartRegisteredStoryEvent(enum PgwStartingRegion region, u16 eventId);
enum PgrProgressResult PgrProgress_TryCompleteRegisteredStoryEvent(enum PgwStartingRegion region, u16 eventId);

bool32 PgrProgress_IsRewardClaimed(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId);
bool32 PgrProgress_MarkRewardClaimed(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId);
enum PgrRewardResult PgrProgress_TryGiveUniqueItem(enum PgrRewardScope scope, enum PgwStartingRegion region, u16 rewardId, enum Item itemId, u16 count);

// Script bridge: 0x8004=region, 0x8005=event/reward, 0x8006=scope,
// 0x8007=item and 0x8008=quantity. Results are returned in VAR_RESULT.
void PgrProgress_ScriptCanStartStoryEvent(void);
void PgrProgress_ScriptCompleteStoryEvent(void);
void PgrProgress_ScriptTryGiveUniqueItem(void);
void PgrProgress_ScriptCheckHoennPokedex(void);
void PgrProgress_ScriptCheckHoennShoes(void);
void PgrProgress_ScriptCheckHoennWallyCatchingTutorial(void);
void PgrProgress_ScriptCompleteHoennWallyCatchingTutorial(void);
void PgrProgress_ScriptGetHoennPetalburgScottOpportunity(void);
void PgrProgress_ScriptShouldShowHoennRustboroSchoolScott(void);
void PgrProgress_ScriptCheckHoennRoxanne(void);
void PgrProgress_ScriptCheckHoennBrawly(void);
void PgrProgress_ScriptCheckHoennDevonGoodsTheft(void);
void PgrProgress_ScriptCheckHoennDevonGoodsRecovery(void);
void PgrProgress_ScriptCheckHoennDevonGoodsReturn(void);
void PgrProgress_ScriptShouldShowHoennRustboroRival(void);
void PgrProgress_ScriptCanMeetHoennRoute104Rival(void);
void PgrProgress_ScriptCheckHoennStevenLetter(void);
void PgrProgress_ScriptCheckHoennDock(void);
void PgrProgress_ScriptCheckHoennStern(void);
void PgrProgress_ScriptGetHoennSlateportScottOpportunity(void);
void PgrProgress_ScriptCheckHoennRoute110Rival(void);
void PgrProgress_ScriptCompleteHoennRoute110Rival(void);
void PgrProgress_ScriptCheckHoennWallyMauville(void);
void PgrProgress_ScriptCheckHoennWattson(void);
void PgrProgress_ScriptShouldShowHoennVerdanturfScott(void);
void PgrProgress_ScriptCanMoveHoennScottToFallarbor(void);
void PgrProgress_ScriptShouldShowHoennFallarborScott(void);
void PgrProgress_ScriptCheckHoennMeteoriteTheft(void);
void PgrProgress_ScriptCheckHoennMaxieMtChimney(void);
void PgrProgress_ScriptCheckHoennFlannery(void);
void PgrProgress_ScriptCheckHoennGoGoggles(void);
void PgrProgress_ScriptCheckHoennNorman(void);
void PgrProgress_ScriptCheckHoennStevenRoute118(void);
void PgrProgress_ScriptCompleteHoennStevenRoute118(void);
void PgrProgress_ScriptCheckHoennWeatherInstitute(void);
void PgrProgress_ScriptCompleteHoennWeatherInstitute(void);
void PgrProgress_ScriptCheckHoennRoute119Rival(void);
void PgrProgress_ScriptCompleteHoennRoute119Rival(void);
void PgrProgress_ScriptShouldShowHoennLilycoveRival(void);
void PgrProgress_ScriptShouldShowHoennLilycoveScott(void);
void PgrProgress_ScriptCheckHoennDevonScope(void);
void PgrProgress_ScriptCompleteHoennDevonScope(void);
void PgrProgress_ScriptCheckHoennFortreeGymPath(void);
void PgrProgress_ScriptCheckHoennWinona(void);
void PgrProgress_ScriptCheckHoennMtPyreOrbTheft(void);
void PgrProgress_ScriptCompleteHoennMtPyreOrbTheft(void);
void PgrProgress_ScriptCheckHoennMagmaHideout(void);
void PgrProgress_ScriptCompleteHoennMagmaHideout(void);
void PgrProgress_ScriptCheckHoennSubmarineTheft(void);
void PgrProgress_ScriptCompleteHoennSubmarineTheft(void);
void PgrProgress_ScriptCheckHoennAquaHideout(void);
void PgrProgress_ScriptCompleteHoennAquaHideout(void);
void PgrProgress_ScriptCheckHoennTateLiza(void);
void PgrProgress_ScriptShouldShowHoennMossdeepScott(void);
void PgrProgress_ScriptCompleteHoennMossdeepScott(void);
void PgrProgress_ScriptCheckHoennMossdeepSpaceCenter(void);
void PgrProgress_ScriptCompleteHoennMossdeepSpaceCenter(void);
void PgrProgress_ScriptCheckHoennDiveFromSteven(void);
void PgrProgress_ScriptCompleteHoennDiveFromSteven(void);
void PgrProgress_ScriptCheckHoennSeafloorCavern(void);
void PgrProgress_ScriptCompleteHoennSeafloorCavern(void);
void PgrProgress_ScriptCheckHoennSootopolisCrisis(void);
void PgrProgress_ScriptCompleteHoennSootopolisCrisis(void);
void PgrProgress_ScriptCheckHoennWallaceRayquaza(void);
void PgrProgress_ScriptCompleteHoennWallaceRayquaza(void);
void PgrProgress_ScriptCheckHoennSkyPillarOpening(void);
void PgrProgress_ScriptCompleteHoennSkyPillarOpening(void);
void PgrProgress_ScriptCheckHoennRayquazaAwakening(void);
void PgrProgress_ScriptCompleteHoennRayquazaAwakening(void);
void PgrProgress_ScriptCheckHoennSootopolisResolution(void);
void PgrProgress_ScriptCompleteHoennSootopolisResolution(void);
void PgrProgress_ScriptCheckHoennWaterfallFromWallace(void);
void PgrProgress_ScriptCompleteHoennWaterfallFromWallace(void);
void PgrProgress_ScriptCheckHoennJuan(void);
void PgrProgress_ScriptCheckHoennWallyVictoryRoad(void);
void PgrProgress_ScriptShouldShowHoennEverGrandeScott(void);
void PgrProgress_ScriptCheckHoennLeagueAccess(void);
void PgrProgress_ScriptCompleteHoennLeagueEntry(void);
void PgrProgress_ScriptCheckHoennSidney(void);
void PgrProgress_ScriptCheckHoennPhoebe(void);
void PgrProgress_ScriptCheckHoennGlacia(void);
void PgrProgress_ScriptCheckHoennDrake(void);
void PgrProgress_ScriptCheckHoennChampion(void);
void PgrProgress_ScriptGetHoennPostgameResearchOpportunity(void);
void PgrProgress_ScriptCompleteHoennPostgameResearchUpdate(void);
void PgrProgress_ScriptCompleteHoennSSTicket(void);
void PgrProgress_ScriptCanUseHoennSSTidal(void);
void PgrProgress_ScriptGetHoennSSTidalScottOpportunity(void);
void PgrProgress_ScriptCompleteHoennSSTidalScott(void);
void PgrProgress_ScriptCanStartHoennBattleFrontierReception(void);
void PgrProgress_ScriptCompleteHoennBattleFrontierReception(void);
void PgrProgress_ScriptHasEnteredHoennBattleFrontier(void);
void PgrProgress_ScriptGetHoennScottHouseOpportunity(void);
void PgrProgress_ScriptCompleteHoennScottHouseWelcome(void);
void PgrProgress_ScriptGetHoennMeteorFallsStevenOpportunity(void);
void PgrProgress_ScriptCompleteHoennMeteorFallsSteven(void);

#endif // GUARD_POKEMON_REGIONALIDADES_PROGRESS_H
