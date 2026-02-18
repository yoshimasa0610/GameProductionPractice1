#include "Twisted Caltis.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// Twisted Caltist: 変則的なカルト教団兵。素早さ・HP・攻撃特性を設定するラッパー。
int SpawnTwistedCultist(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    // 個別パラメータ調整
    e->width = 34.0f;
    e->height = 52.0f;
    e->maxHP = 90;
    e->currentHP = e->maxHP;
    e->attackPower = 16;
    e->detectRange = 320.0f;
    e->attackRange = 44.0f;
    e->attackCooldown = 1.2f;

    std::printf("Spawned Twisted Caltist at %.1f, %.1f (idx=%d)\n", x, y, idx);
    return idx;
}