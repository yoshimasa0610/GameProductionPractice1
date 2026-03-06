#include "cultists.h"
#include "cultists.h"
#include "../EnemyBase.h"

int SpawnCultist(float x, float y)
{
    return SpawnEnemy(EnemyType::Cultists, x, y);
}