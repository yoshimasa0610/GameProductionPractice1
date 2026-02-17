#include "CheckpointManager.h"

// 非公開の静的フラグ
static bool s_PlayerSitting = false;

void SetPlayerSitting(bool sitting)
{
    s_PlayerSitting = sitting;
}

bool IsPlayerSitting()
{
    return s_PlayerSitting;
}