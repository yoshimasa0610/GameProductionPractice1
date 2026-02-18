#include "Skeleton.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// スケルトン：中堅型、やや遠距離寄り（近接攻撃主体だがリーチがある想定）
int SpawnSkeleton(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    e->width = 30.0f;
    e->height = 56.0f;
    e->maxHP = 60;
    e->currentHP = e->maxHP;
    e->attackPower = 12;
    e->detectRange = 280.0f;
    e->attackRange = 48.0f;
    e->attackCooldown = 1.0f;

    std::printf("Spawned Skeleton at %.1f, %.1f (idx=%d)\n", x, y, idx);
    return idx;
}