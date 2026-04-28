#include "Tiphereth.h"
#include "../BigBossBase.h"

namespace
{
    const int TIPHERETH_MAX_HP = 960;
    const int TIPHERETH_ATTACK_POWER = 24;

    const TipherethTuning TIPHERETH_TUNING =
    {
        0.15f,   // bodyHalfWidthRatio
        0.30f,   // bodyWidthRatio
        0.62f,   // bodyHeightRatio
        0.58f,   // bodyTopOffsetRatio
        0.08f,   // bodyFacingOffsetRatio
        300.0f,  // tentacleWidth
        170.0f,  // tentacleHeight
        218.0f,  // tentacleFrontOffset
        -42.0f,  // tentacleCenterYOffset
        250.0f,  // biteWidth
        190.0f,  // biteHeight
        245.0f,  // biteFrontOffset
        -32.0f,  // biteCenterYOffset
        300.0f,  // chargeWidth
        180.0f,  // chargeHeight
        126.0f,  // chargeFrontOffset
        -26.0f,  // chargeCenterYOffset
        500.0f,  // stompRange
        72.0f,   // stompHitboxYOffset
        520.0f,  // chargePriorityRange
        5.0f,    // chargeSpeed
        220.0f,  // chargeThroughDistance
        1.0f,    // prepareDuration
        3.0f,    // stompPrepareDuration
        -24.0f,  // drawOffsetX
        80.0f,   // drawOffsetY
        true,    // reverseDrawFacing
        true     // debugDraw
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