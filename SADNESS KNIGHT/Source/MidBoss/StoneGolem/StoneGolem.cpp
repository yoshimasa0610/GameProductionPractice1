#include "../MidBossBase.h"
#include "StoneGolem.h"

namespace
{
    const int STONE_GOLEM_MAX_HP = 420;
    const int STONE_GOLEM_ATTACK_POWER = 24;

    const StoneGolemTuning STONE_GOLEM_TUNING =
    {
        1.5f,   // shotPrepare
        2.0f,   // beamPrepare
        2.0f,   // shotRecovery
        4.0f,   // beamRecovery
        1.0f,   // beamActive
        3.0f,   // barrageDuration
        0.22f,  // barrageInterval
        6.0f,   // bulletSpeed
        96.0f,  // bulletDrawSize
        60.0f,  // bulletHitboxSize
        0.0f,   // drawOffsetX
        80.0f,  // drawOffsetY
        100.0f, // beamDrawOffsetX
        28.0f,  // beamDrawOffsetY
        -70.0f  // beamMuzzleFineTuneX
    };
}

int SpawnStoneGolem(float x, float y)
{
    return SpawnMidBoss(MidBossType::StoneGolem, x, y);
}

int GetStoneGolemMaxHP()
{
    return STONE_GOLEM_MAX_HP;
}

int GetStoneGolemAttackPower()
{
    return STONE_GOLEM_ATTACK_POWER;
}

const StoneGolemTuning& GetStoneGolemTuning()
{
    return STONE_GOLEM_TUNING;
}
