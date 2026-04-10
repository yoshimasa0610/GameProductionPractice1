#include "../MidBossBase.h"

namespace
{
    const int GAROK_MAX_HP = 680;
    const int GAROK_ATTACK_POWER = 28;
}

int SpawnGarok(float x, float y)
{
    return SpawnMidBoss(MidBossType::Garok, x, y);
}

int GetGarokMaxHP()
{
    return GAROK_MAX_HP;
}

int GetGarokAttackPower()
{
    return GAROK_ATTACK_POWER;
}