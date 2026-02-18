#pragma once
#include <vector>

// 敵のデータ（ベース）
struct EnemyData
{
    bool active;

    // 位置・速度
    float posX;
    float posY;
    float velocityX;
    float velocityY;

    // 向き / 行動
    bool isFacingRight;
    bool isAggro;

    // ステータス
    int maxHP;
    int currentHP;
    int attackPower;

    // AI パラメータ
    float detectRange;    // プレイヤー検知距離
    float attackRange;    // 攻撃判定距離（X 方向）
    float attackCooldown; // 攻撃のクールダウン（秒）
    float attackTimer;    // 攻撃残り時間（秒）
    float cooldownTimer;  // クールダウン残り（秒）

    // 描画 / 当たり
    float width;
    float height;

    // コライダーID（Collision モジュールと連携）
    int colliderId;
};

//
// 敵システム API
//
void InitEnemySystem();
void UpdateEnemies();
void DrawEnemies();
void ClearEnemies();

// 敵生成 / 破棄
int SpawnEnemy(float x, float y); // 成功時はインデックス >= 0 を返す
void DespawnEnemy(int index);

// 敵データアクセス
EnemyData* GetEnemy(int index);
int GetEnemyCount();