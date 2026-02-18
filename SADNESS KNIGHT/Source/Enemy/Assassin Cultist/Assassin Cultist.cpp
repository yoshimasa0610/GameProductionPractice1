#include "Assassin Cultist.h"
#include "../../Enemy/EnemyBase.h"
#include <cstdio>

// 小型で素早く、近接攻撃を行う敵
int SpawnAssassinCultist(float x, float y)
{
    int idx = SpawnEnemy(x, y);
    if (idx < 0) return -1;

    EnemyData* e = GetEnemy(idx);
    if (!e) return -1;

    // 個別パラメータ
    e->width = 28.0f;
    e->height = 44.0f;
    e->maxHP = 30;
    e->currentHP = e->maxHP;
    e->attackPower = 8;
    e->detectRange = 220.0f;
    e->attackRange = 36.0f;
    e->attackCooldown = 0.8f;

    // コライダー owner をセット可能なら将来ここで設定する（現状は CreateCollider の owner が nullptr）
    // 例: UpdateColliderOwner(e->colliderId, e); // 未実装

    std::printf("Spawned Assassin Cultist at %.1f, %.1f (idx=%d)\n", x, y, idx);
    return idx;
}