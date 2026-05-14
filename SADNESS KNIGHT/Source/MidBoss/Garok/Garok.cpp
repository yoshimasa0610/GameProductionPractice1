#include "../MidBossBase.h"
#include "Garok.h"

namespace
{
    const int GAROK_MAX_HP = 680;
    const int GAROK_ATTACK_POWER = 28;

    const GarokTuning GAROK_TUNING =
    {
        0.85f,       // prepareTime
        0.45f,       // attackDuration1
        0.78f,       // attackDuration2
        1.10f,       // attackDuration3
        2.0f,        // cooldown1
        3.0f,        // cooldown2
        4.0f,        // cooldown3
        3.0f,        // barrageDuration
        5.0f,        // barrageCooldown
        2.0f,        // dashDuration
        11.0f,       // dashSpeed
        false,       // enableJump
        8.0f,        // jumpInterval
        210.0f,      // jumpHeight
        1.00f,       // hitboxWidthRatio
        0.64f,       // hitboxHeightRatio
        0.70f,       // hitboxFrontRatio
        0.45f,       // contactYRatio
        24.0f,       // facingDeadzone
        768.0f,      // arenaLeft
        3072.0f,     // arenaRight
        32.0f * 8.0f,// attackCheckRange
        32.0f * 8.0f,// attackRange
        0.35f,       // gravity
        9.0f,        // maxFallSpeed
        0.0f,        // drawOffsetX
        25.0f,       // drawOffsetY
        4.8f,        // drawWidthScale
        false        // debugDraw
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