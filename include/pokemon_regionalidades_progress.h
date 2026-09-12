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

#endif // GUARD_POKEMON_REGIONALIDADES_PROGRESS_H
