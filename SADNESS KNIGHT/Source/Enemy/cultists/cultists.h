#pragma once

// Cultist（カルトツイスト）スポーン用ラッパー
// SpawnCultist を呼ぶと EnemyBase 側で管理される敵が生成されます。
int SpawnCultist(float x, float y);
int GetCultistMaxHP();
int GetCultistAttackPower();