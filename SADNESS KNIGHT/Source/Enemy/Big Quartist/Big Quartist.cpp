#include "Big Quartist.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// ‘åŒ^‚Ì“GAƒŠ[ƒ`‚Æ‘Ï‹v‚ğd‹
int SpawnBigQuartist(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    e->width = 64.0f;
    e->height = 96.0f;
    e->maxHP = 250;
    e->currentHP = e->maxHP;
    e->attackPower = 30;
    e->detectRange = 360.0f;
    e->attackRange = 100.0f;
    e->attackCooldown = 1.6f;

    std::printf("Spawned Big Quartist at %.1f, %.1f (idx=%d)\n", x, y, idx);
    return idx;
}