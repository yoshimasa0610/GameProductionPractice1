#include "Kether.h"
#include "../BigBossBase.h"

namespace
{
    const int KETHER_MAX_HP = 500;
    const int KETHER_ATTACK_POWER = 14;
}

int SpawnKether(float x, float y)
{
    return SpawnBigBoss(BigBossType::Kether, x, y);
}

int GetKetherMaxHP()
{
    return KETHER_MAX_HP;
}

int GetKetherAttackPower()
{
    return KETHER_ATTACK_POWER;
}
