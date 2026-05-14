#include "Kether.h"
#include "../BigBossBase.h"

namespace
{
    const int KETHER_MAX_HP = 2500;
    const int KETHER_ATTACK_POWER = 14;

    const KetherTuning KETHER_TUNING =
    {
        0.09f,   // bodyHalfWidthRatio - 本体半幅比率
        0.18f,   // bodyWidthRatio - 本体幅比率
        0.62f,   // bodyHeightRatio - 本体高さ比率
        0.84f,   // bodyTopOffsetRatio - 本体上部オフセット比率
        0.00f,   // flameOriginXRatio - 炎発射位置X比率
        0.56f,   // flameOriginYRatio - 炎発射位置Y比率
        24.0f,   // flameDrawOffsetY - 炎描画オフセットY
        -0.26f,  // bookOriginXRatio - 本発射位置X比率
        0.34f,   // bookOriginYRatio - 本発射位置Y比率
        0.45f,   // phaseShockwaveDuration - フェーズ衝撃波持続時間
        0.35f,   // transformEndHold - 変身終了ホールド時間
        18.0f,   // diedDrawOffsetY - 死亡時描画オフセットY
        200.0f,  // areaMinY - エリア最小Y
        450.0f,  // areaMaxY - エリア最大Y
        94.0f,   // flameWidth - 炎の幅
        130.0f,  // bookWidth - 本の幅
        100.0f,  // bookHeight - 本の高さ
        false    // debugDraw - デバッグ描画
    };
}

int SpawnKether(float x, float y)
{
    return SpawnBigBoss(BigBossType::Kether, x, y);
}

int GetKetherMaxHP()
{
    return KETHER_MAX_HP;
}

int GetKetherAttackPower()
{
    return KETHER_ATTACK_POWER;
}

const KetherTuning& GetKetherTuning()
{
    return KETHER_TUNING;
}
