#include "Slime.h"
#include "../EnemyBase.h"

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

