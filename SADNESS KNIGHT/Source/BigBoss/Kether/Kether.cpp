#include "Kether.h"
#include "../BigBossBase.h"

namespace
{
    const int KETHER_MAX_HP = 500;
    const int KETHER_ATTACK_POWER = 14;

    const KetherTuning KETHER_TUNING =
    {
        0.09f,
        0.18f,
        0.62f,
        0.84f,
        0.00f,
        0.56f,
        24.0f,
        -0.26f,
        0.34f,
        0.45f,
        0.35f,
        18.0f,
        true
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
