#include "Twisted Caltis.h"
#include "Twisted Caltis.h"
#include "../EnemyBase.h"

namespace
{
    const int TWISTED_CALTIS_MAX_HP = 90;
    const int TWISTED_CALTIS_ATTACK_POWER = 16;
}

int SpawnTwistedCultist(float x, float y)
{
    return SpawnEnemy(EnemyType::TwistedCaltis, x, y);
}

int GetTwistedCaltisMaxHP()
{
    return TWISTED_CALTIS_MAX_HP;
}

int GetTwistedCaltisAttackPower()
{
    return TWISTED_CALTIS_ATTACK_POWER;
}
