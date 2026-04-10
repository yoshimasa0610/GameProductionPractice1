#include "cultists.h"
#include "cultists.h"
#include "../EnemyBase.h"

namespace
{
    const int CULTIST_MAX_HP = 40;
    const int CULTIST_ATTACK_POWER = 12;
}

int SpawnCultist(float x, float y)
{
    return SpawnEnemy(EnemyType::Cultists, x, y);
}

int GetCultistMaxHP()
{
    return CULTIST_MAX_HP;
}

int GetCultistAttackPower()
{
    return CULTIST_ATTACK_POWER;
}