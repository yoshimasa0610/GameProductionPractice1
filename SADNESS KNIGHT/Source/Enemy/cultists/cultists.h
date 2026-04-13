#pragma once
#include "../EnemyBase.h"

// Cultist（カルトイスト）スポーン用ラッパー
// SpawnCultist を通せば EnemyBase 側で管理される敵が生成されます。
int SpawnCultist(float x, float y);
int GetCultistMaxHP();
int GetCultistAttackPower();
const EnemyConfig& GetCultistConfig();