#include "Skeleton.h"
#include "Skeleton.h"
#include "../EnemyBase.h"

int SpawnSkeleton(float x, float y)
{
    return SpawnEnemy(EnemyType::Skeleton, x, y);
}