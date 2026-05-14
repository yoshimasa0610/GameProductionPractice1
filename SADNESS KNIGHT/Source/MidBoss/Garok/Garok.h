#pragma once

struct GarokTuning
{
    float prepareTime;          // 攻撃準備時間
    float attackDuration1;      // 攻撃時間1
    float attackDuration2;      // 攻撃時間2
    float attackDuration3;      // 攻撃時間3
    float cooldown1;            // クールダウン1
    float cooldown2;            // クールダウン2
    float cooldown3;            // クールダウン3
    float barrageDuration;      // 乱射持続時間
    float barrageCooldown;      // 乱射クールダウン
    float dashDuration;         // ダッシュ時間
    float dashSpeed;            // ダッシュ速度
    bool enableJump;            // ジャンプ有効化
    float jumpInterval;         // ジャンプ間隔
    float jumpHeight;           // ジャンプ高さ
    float hitboxWidthRatio;     // 当たり判定幅比率
    float hitboxHeightRatio;    // 当たり判定高さ比率
    float hitboxFrontRatio;     // 当たり判定前方比率
    float contactYRatio;        // 接触Y比率
    float facingDeadzone;       // 向き判定デッドゾーン
    float arenaLeft;            // アリーナ左端
    float arenaRight;           // アリーナ右端
    float attackCheckRange;     // 攻撃判定範囲
    float attackRange;          // 攻撃射程
    float gravity;              // 重力
    float maxFallSpeed;         // 最大落下速度
    float drawOffsetX;          // 描画オフセットX
    float drawOffsetY;          // 描画オフセットY
    float drawWidthScale;       // 描画幅スケール
    bool debugDraw;             // デバッグ描画
};

int SpawnGarok(float x, float y);
int GetGarokMaxHP();
int GetGarokAttackPower();
const GarokTuning& GetGarokTuning();