#include "StoneGolem.h"
#include "../MidBossBase.h"

namespace
{
    const int STONE_GOLEM_MAX_HP = 420;
    const int STONE_GOLEM_ATTACK_POWER = 24;
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
