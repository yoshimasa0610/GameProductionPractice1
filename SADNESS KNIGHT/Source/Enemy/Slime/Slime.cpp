#include "Slime.h"
#include "../EnemyBase.h"

namespace
{
    const int SLIME_MAX_HP = 12;
    const int SLIME_ATTACK_POWER = 4;
}

//============================================================
// スライムを配置
//============================================================
// 内部的には SpawnEnemy(EnemyType::Slime, x, y) を呼んでいる
// パラメータは EnemyBase.cpp の設定テーブルで管理されている
//============================================================

int SpawnSlime(float x, float y)
{
    return SpawnEnemy(EnemyType::Slime, x, y);
}

int GetSlimeMaxHP()
{
    return SLIME_MAX_HP;
}

int GetSlimeAttackPower()
{
    return SLIME_ATTACK_POWER;
}

