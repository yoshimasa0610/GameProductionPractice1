#include "Kether.h"
#include "../BigBossBase.h"

namespace
{
    const int KETHER_MAX_HP = 500;
    const int KETHER_ATTACK_POWER = 14;

    const KetherTuning KETHER_TUNING =
    {
        0.09f,   // bodyHalfWidthRatio
        0.18f,   // bodyWidthRatio
        0.62f,   // bodyHeightRatio
        0.84f,   // bodyTopOffsetRatio
        0.00f,   // flameOriginXRatio
        0.56f,   // flameOriginYRatio
        24.0f,   // flameDrawOffsetY
        -0.26f,  // bookOriginXRatio
        0.34f,   // bookOriginYRatio
        0.45f,   // phaseShockwaveDuration
        0.35f,   // transformEndHold
        18.0f,   // diedDrawOffsetY
        200.0f,  // areaMinY
        450.0f,  // areaMaxY
        94.0f,   // flameWidth
        130.0f,  // bookWidth
        100.0f,  // bookHeight
        true     // debugDraw
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
