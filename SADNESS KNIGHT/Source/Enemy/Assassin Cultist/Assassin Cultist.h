#pragma once
#include "../EnemyBase.h"

// Assassin Cultist ラッパー
// EnemyBase の SpawnEnemy を呼び出して個別パラメータを設定するヘルパーを提供します。

int SpawnAssassinCultist(float x, float y);
int GetAssassinCultistMaxHP();
int GetAssassinCultistAttackPower();
const EnemyConfig& GetAssassinCultistConfig();