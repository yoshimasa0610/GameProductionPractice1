#include "Big Quartist.h"
#include "Big Quartist.h"
#include "../EnemyBase.h"

namespace
{
    const int BIG_QUARTIST_MAX_HP = 250;
    const int BIG_QUARTIST_ATTACK_POWER = 30;
}

int SpawnBigQuartist(float x, float y)
{
    return SpawnEnemy(EnemyType::BigQuartist, x, y);
}

int GetBigQuartistMaxHP()
{
    return BIG_QUARTIST_MAX_HP;
}

int GetBigQuartistAttackPower()
{
    return BIG_QUARTIST_ATTACK_POWER;
}