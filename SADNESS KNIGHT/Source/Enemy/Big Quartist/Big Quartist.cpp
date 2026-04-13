#include "Big Quartist.h"
#include "../EnemyBase.h"

namespace
{
    const int BIG_QUARTIST_MAX_HP = 250;
    const int BIG_QUARTIST_ATTACK_POWER = 30;
    const EnemyConfig BIG_QUARTIST_CONFIG =
    {
        250, 30, 0.8f, 150.0f, 150.0f, 360.0f, 88.0f, 0.75f, 1.8f, false, 0.0f, "Assets/Enemies/BigQuartist/"
    };
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

const EnemyConfig& GetBigQuartistConfig()
{
    return BIG_QUARTIST_CONFIG;
}