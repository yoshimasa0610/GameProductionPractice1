#include "Assassin Cultist.h"
#include "Assassin Cultist.h"
#include "../EnemyBase.h"

namespace
{
    const int ASSASSIN_CULTIST_MAX_HP = 30;
    const int ASSASSIN_CULTIST_ATTACK_POWER = 8;
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