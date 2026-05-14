#include "../MidBossBase.h"
#include "Garok.h"

namespace
{
    const int GAROK_MAX_HP = 2500;
    const int GAROK_ATTACK_POWER = 28;

    const GarokTuning GAROK_TUNING =
    {
        0.85f,       // prepareTime - 攻撃準備時間
        0.45f,       // attackDuration1 - 攻撃時間1
        0.78f,       // attackDuration2 - 攻撃時間2
        1.10f,       // attackDuration3 - 攻撃時間3
        2.0f,        // cooldown1 - クールダウン1
        3.0f,        // cooldown2 - クールダウン2
        4.0f,        // cooldown3 - クールダウン3
        3.0f,        // barrageDuration - 乱射持続時間
        5.0f,        // barrageCooldown - 乱射クールダウン
        2.0f,        // dashDuration - ダッシュ時間
        11.0f,       // dashSpeed - ダッシュ速度
        false,       // enableJump - ジャンプ有効化
        8.0f,        // jumpInterval - ジャンプ間隔
        210.0f,      // jumpHeight - ジャンプ高さ
        1.00f,       // hitboxWidthRatio - 当たり判定幅比率
        0.64f,       // hitboxHeightRatio - 当たり判定高さ比率
        0.70f,       // hitboxFrontRatio - 当たり判定前方比率
        0.45f,       // contactYRatio - 接触Y比率
        24.0f,       // facingDeadzone - 向き判定デッドゾーン
        768.0f,      // arenaLeft - アリーナ左端
        3072.0f,     // arenaRight - アリーナ右端
        32.0f * 8.0f,// attackCheckRange - 攻撃判定範囲
        32.0f * 8.0f,// attackRange - 攻撃射程
        0.35f,       // gravity - 重力
        9.0f,        // maxFallSpeed - 最大落下速度
        0.0f,        // drawOffsetX - 描画オフセットX
        25.0f,       // drawOffsetY - 描画オフセットY
        4.8f,        // drawWidthScale - 描画幅スケール
        false        // debugDraw - デバッグ描画
    };
}

int SpawnGarok(float x, float y)
{
    return SpawnMidBoss(MidBossType::Garok, x, y);
}

int GetGarokMaxHP()
{
    return GAROK_MAX_HP;
}

int GetGarokAttackPower()
{
    return GAROK_ATTACK_POWER;
}

const GarokTuning& GetGarokTuning()
{
    return GAROK_TUNING;
}