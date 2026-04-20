#include "cultists.h"
#include "../EnemyBase.h"

namespace
{
    const int CULTIST_MAX_HP = 350;
    const int CULTIST_ATTACK_POWER = 12;
    const EnemyConfig CULTIST_CONFIG =
    {
        40, 12, 1.2f, 50.0f, 50.0f, 320.0f, 96.0f, 0.35f, 1.0f, false, 0.0f, "Assets/Enemies/Cultists/"
    };
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

const EnemyConfig& GetCultistConfig()
{
    return CULTIST_CONFIG;
}