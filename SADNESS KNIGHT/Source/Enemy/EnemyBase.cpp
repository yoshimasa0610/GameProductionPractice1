#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include "DxLib.h"
#include <cmath>
#include <cstdio>

namespace
{
    std::vector<EnemyData> g_enemies;

    // デフォルトパラメータ
    const float DEFAULT_WIDTH = 32.0f;
    const float DEFAULT_HEIGHT = 48.0f;
    const float DEFAULT_DETECT_RANGE = 300.0f;
    const float DEFAULT_ATTACK_RANGE = 48.0f;
    const float DEFAULT_ATTACK_DURATION = 0.15f; // 秒
    const float DEFAULT_ATTACK_COOLDOWN = 1.0f;  // 秒
    const int   DEFAULT_ATTACK_POWER = 10;
}

// 初期化
void InitEnemySystem()
{
    g_enemies.clear();
}

// 敵生成
int SpawnEnemy(float x, float y)
{
    EnemyData e{};
    e.active = true;
    e.posX = x;
    e.posY = y;
    e.velocityX = 0.0f;
    e.velocityY = 0.0f;
    e.isFacingRight = true;
    e.isAggro = false;
    e.maxHP = 50;
    e.currentHP = e.maxHP;
    e.attackPower = DEFAULT_ATTACK_POWER;
    e.detectRange = DEFAULT_DETECT_RANGE;
    e.attackRange = DEFAULT_ATTACK_RANGE;
    e.attackCooldown = DEFAULT_ATTACK_COOLDOWN;
    e.attackTimer = 0.0f;
    e.cooldownTimer = 0.0f;
    e.width = DEFAULT_WIDTH;
    e.height = DEFAULT_HEIGHT;

    // コライダー作成（左上座標基準）
    float left = e.posX - (e.width * 0.5f);
    float top = e.posY - e.height;
    e.colliderId = CreateCollider(ColliderTag::Enemy, left, top, e.width, e.height, nullptr);

    g_enemies.push_back(e);
    return static_cast<int>(g_enemies.size() - 1);
}

// 敵破棄
void DespawnEnemy(int index)
{
    if (index < 0 || index >= (int)g_enemies.size()) return;
    EnemyData& e = g_enemies[index];
    if (e.colliderId != -1)
    {
        DestroyCollider(e.colliderId);
        e.colliderId = -1;
    }
    e.active = false;
}

// 全削除
void ClearEnemies()
{
    for (auto& e : g_enemies)
    {
        if (e.colliderId != -1)
        {
            DestroyCollider(e.colliderId);
            e.colliderId = -1;
        }
    }
    g_enemies.clear();
}

// 敵更新（AI）
void UpdateEnemies()
{
    PlayerData& player = GetPlayerData();

    for (auto& e : g_enemies)
    {
        if (!e.active) continue;

        // 簡易死亡処理
        if (e.currentHP <= 0)
        {
            // コライダー破棄して非活性化
            if (e.colliderId != -1)
            {
                DestroyCollider(e.colliderId);
                e.colliderId = -1;
            }
            e.active = false;
            continue;
        }

        // 距離計算（2D）
        float dx = player.posX - e.posX;
        float dy = player.posY - e.posY;
        float dist = std::sqrt(dx * dx + dy * dy);

        // 感知
        if (dist <= e.detectRange)
        {
            e.isAggro = true;
            e.isFacingRight = (dx >= 0.0f);
        }
        else
        {
            e.isAggro = false;
        }

        // 攻撃クール管理（秒をフレーム換算する必要があるが、DxLib のフレーム時間取得がないため
        // ここでは固定フレームレート想定（60FPS）で進める。必要なら実時間 delta を渡す形に変更してください）
        const float FRAME_TIME = 1.0f / 60.0f;
        if (e.cooldownTimer > 0.0f) e.cooldownTimer -= FRAME_TIME;
        if (e.attackTimer > 0.0f) e.attackTimer -= FRAME_TIME;

        // 追従（簡易）
        if (e.isAggro)
        {
            // 水平方向の単純移動（必要なら移動速度パラメータを追加）
            const float MOVE_SPEED = 1.2f;
            if (std::fabs(dx) > e.attackRange * 0.5f)
            {
                e.velocityX = (dx > 0.0f) ? MOVE_SPEED : -MOVE_SPEED;
            }
            else
            {
                e.velocityX = 0.0f;
            }
        }
        else
        {
            e.velocityX = 0.0f;
        }

        // 位置更新
        e.posX += e.velocityX;
        e.posY += e.velocityY;

        // 攻撃判定（X方向のみのシンプル判定）
        if (e.cooldownTimer <= 0.0f && std::fabs(dx) <= e.attackRange && std::fabs(dy) <= (e.height * 0.8f))
        {
            // 攻撃開始：攻撃時間中は当たり範囲を拡張してコライダーで判定させる
            e.attackTimer = DEFAULT_ATTACK_DURATION;
            e.cooldownTimer = e.attackCooldown;

            // 拡張コライダー（攻撃範囲）：一時的に UpdateCollider で広げる
            if (e.colliderId != -1)
            {
                float atkW = e.attackRange;
                float atkH = e.height;
                float atkLeft = e.posX + ((e.isFacingRight) ? (e.width * 0.5f) : -atkW - (e.width * 0.5f));
                float atkTop = e.posY - atkH;
                UpdateCollider(e.colliderId, atkLeft, atkTop, atkW, atkH);
            }
        }

        // 攻撃終了後は通常コライダーサイズへ戻す
        if (e.attackTimer <= 0.0f)
        {
            if (e.colliderId != -1)
            {
                float left = e.posX - (e.width * 0.5f);
                float top = e.posY - e.height;
                UpdateCollider(e.colliderId, left, top, e.width, e.height);
            }
        }
        else
        {
            // 攻撃中は位置同期（攻撃コライダーも移動する）
            if (e.colliderId != -1)
            {
                // 再計算して UpdateCollider（上と同様）
                float atkW = e.attackRange;
                float atkH = e.height;
                float atkLeft = e.posX + ((e.isFacingRight) ? (e.width * 0.5f) : -atkW - (e.width * 0.5f));
                float atkTop = e.posY - atkH;
                UpdateCollider(e.colliderId, atkLeft, atkTop, atkW, atkH);
            }
        }

        // 通常コライダー位置更新（攻撃していないとき）
        if (e.attackTimer <= 0.0f && e.colliderId != -1)
        {
            float left = e.posX - (e.width * 0.5f);
            float top = e.posY - e.height;
            UpdateCollider(e.colliderId, left, top, e.width, e.height);
        }
    }
}

// 簡易描画（色で向きを表現）
void DrawEnemies()
{
    for (const auto& e : g_enemies)
    {
        if (!e.active) continue;

        int drawX = static_cast<int>(e.posX);
        int drawY = static_cast<int>(e.posY);

        int halfW = static_cast<int>(e.width * 0.5f);
        int h = static_cast<int>(e.height);

        unsigned int color = GetColor(200, 50, 50);
        DrawBox(drawX - halfW, drawY - h, drawX + halfW, drawY, color, TRUE);

        // 向き矢印
        if (e.isFacingRight)
        {
            DrawTriangle(drawX + halfW, drawY - h / 2, drawX + halfW - 8, drawY - h / 2 - 6, drawX + halfW - 8, drawY - h / 2 + 6, GetColor(255, 255, 0), TRUE);
        }
        else
        {
            DrawTriangle(drawX - halfW, drawY - h / 2, drawX - halfW + 8, drawY - h / 2 - 6, drawX - halfW + 8, drawY - h / 2 + 6, GetColor(255, 255, 0), TRUE);
        }
    }
}

// アクセサ
EnemyData* GetEnemy(int index)
{
    if (index < 0 || index >= (int)g_enemies.size()) return nullptr;
    return &g_enemies[index];
}

int GetEnemyCount()
{
    return static_cast<int>(g_enemies.size());
}