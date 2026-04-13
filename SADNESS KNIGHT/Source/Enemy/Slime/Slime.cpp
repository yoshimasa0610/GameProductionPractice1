#include "Slime.h"
#include "../EnemyBase.h"

namespace
{
    const int SLIME_MAX_HP = 12;
    const int SLIME_ATTACK_POWER = 4;
    const EnemyConfig SLIME_CONFIG =
    {
        12, 4, 1.0f, 52.0f, 44.0f, 140.0f, 80.0f, 0.45f, 1.5f, false, 0.0f, "Assets/Enemies/Slime/"
    };
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

const EnemyConfig& GetSlimeConfig()
{
    return SLIME_CONFIG;
}

