#include "Slime.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// スライム：低HP、近接でゆっくり
int SpawnSlime(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    e->width = 24.0f;
    e->height = 20.0f;
    e->maxHP = 12;
    e->currentHP = e->maxHP;
    e->attackPower = 4;
    e->detectRange = 140.0f;
    e->attackRange = 18.0f;
    e->attackCooldown = 1.2f;

    std::printf("Spawned Slime at %.1f, %.1f (idx=%d)\n", x, y, idx);
    return idx;
}