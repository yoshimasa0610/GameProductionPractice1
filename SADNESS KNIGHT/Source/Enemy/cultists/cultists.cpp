#include "cultists.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// Cultist：中型の汎用近接敵（プレイヤーを感知して近づき攻撃）
int SpawnCultist(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    // 個別パラメータ
    e->width = 30.0f;
    e->height = 56.0f;
    e->maxHP = 40;
    e->currentHP = e->maxHP;
    e->attackPower = 12;
    e->detectRange = 260.0f;
    e->attackRange = 40.0f;
    e->attackCooldown = 1.0f;

    // 出力（デバッグ）: printfDx を使わず標準出力へ
    std::printf("Spawned Cultist at %.1f, %.1f (idx=%d)\n", x, y, idx);

    return idx;
}