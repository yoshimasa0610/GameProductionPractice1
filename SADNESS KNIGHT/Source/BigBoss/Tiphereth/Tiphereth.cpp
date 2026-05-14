#include "Tiphereth.h"
#include "../BigBossBase.h"

namespace
{
    const int TIPHERETH_MAX_HP = 2500;
    const int TIPHERETH_ATTACK_POWER = 24;

    const TipherethTuning TIPHERETH_TUNING =
    {
        0.15f,   // bodyHalfWidthRatio - 本体半幅比率
        0.30f,   // bodyWidthRatio - 本体幅比率
        0.62f,   // bodyHeightRatio - 本体高さ比率
        0.58f,   // bodyTopOffsetRatio - 本体上部オフセット比率
        0.08f,   // bodyFacingOffsetRatio - 本体向きオフセット比率
        300.0f,  // tentacleWidth - 触手の幅
        170.0f,  // tentacleHeight - 触手の高さ
        218.0f,  // tentacleFrontOffset - 触手前方オフセット
        -42.0f,  // tentacleCenterYOffset - 触手中心Yオフセット
        250.0f,  // biteWidth - 噪みつきの幅
        190.0f,  // biteHeight - 噪みつきの高さ
        245.0f,  // biteFrontOffset - 噪みつき前方オフセット
        -32.0f,  // biteCenterYOffset - 噪みつき中心Yオフセット
        300.0f,  // chargeWidth - 突進の幅
        180.0f,  // chargeHeight - 突進の高さ
        126.0f,  // chargeFrontOffset - 突進前方オフセット
        -26.0f,  // chargeCenterYOffset - 突進中心Yオフセット
        500.0f,  // stompRange - 踏みつけ範囲
        72.0f,   // stompHitboxYOffset - 踏みつけ当たり判定Yオフセット
        520.0f,  // chargePriorityRange - 突進優先範囲
        5.0f,    // chargeSpeed - 突進速度
        220.0f,  // chargeThroughDistance - 突進貫通距離
        1.0f,    // prepareDuration - 準備時間
        3.0f,    // stompPrepareDuration - 踏みつけ準備時間
        -24.0f,  // drawOffsetX - 描画オフセットX
        80.0f,   // drawOffsetY - 描画オフセットY
        true,    // reverseDrawFacing - 描画向き反転
        false    // debugDraw - デバッグ描画
    };
}

int SpawnTiphereth(float x, float y)
{
    return SpawnBigBoss(BigBossType::Tiphereth, x, y);
}

int GetTipherethMaxHP()
{
    return TIPHERETH_MAX_HP;
}

int GetTipherethAttackPower()
{
    return TIPHERETH_ATTACK_POWER;
}

const TipherethTuning& GetTipherethTuning()
{
    return TIPHERETH_TUNING;
}