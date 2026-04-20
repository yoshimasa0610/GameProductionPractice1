#include "Assassin Cultist.h"
#include "../EnemyBase.h"

namespace
{
    const int ASSASSIN_CULTIST_MAX_HP = 300;
    const int ASSASSIN_CULTIST_ATTACK_POWER = 8;
    const EnemyConfig ASSASSIN_CULTIST_CONFIG =
    {
        30, 8, 2.0f, 50.0f, 50.0f, 176.0f, 44.0f, 0.22f, 1.5f, true, 9.0f, "Assets/Enemies/AssassinCultist/"
    };
}

int SpawnAssassinCultist(float x, float y)
{
    return SpawnEnemy(EnemyType::AssassinCultist, x, y);
}

int GetAssassinCultistMaxHP()
{
    return ASSASSIN_CULTIST_MAX_HP;
}

int GetAssassinCultistAttackPower()
{
    return ASSASSIN_CULTIST_ATTACK_POWER;
}

const EnemyConfig& GetAssassinCultistConfig()
{
    return ASSASSIN_CULTIST_CONFIG;
}