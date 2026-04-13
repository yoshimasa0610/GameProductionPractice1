#include "Twisted Caltis.h"
#include "../EnemyBase.h"

namespace
{
    const int TWISTED_CALTIS_MAX_HP = 90;
    const int TWISTED_CALTIS_ATTACK_POWER = 16;
    const EnemyConfig TWISTED_CALTIS_CONFIG =
    {
        90, 16, 1.8f, 50.0f, 50.0f, 320.0f, 128.0f, 0.50f, 1.2f, true, 10.0f, "Assets/Enemies/TwistedCaltis/"
    };
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

const EnemyConfig& GetTwistedCaltisConfig()
{
    return TWISTED_CALTIS_CONFIG;
}
