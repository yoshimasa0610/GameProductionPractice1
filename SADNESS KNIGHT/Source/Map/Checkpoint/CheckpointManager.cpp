#include "CheckpointManager.h"

static SitState s_SitState = SitState::None;

void SetSitState(SitState state)
{
    s_SitState = state;
}

SitState GetSitState()
{
    return s_SitState;
}

bool IsPlayerSitting()
{
    return s_SitState == SitState::Sitting;
}