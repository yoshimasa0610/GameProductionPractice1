#include "StoneGolem.h"
#include "../MidBossBase.h"

int SpawnStoneGolem(float x, float y)
{
    return SpawnMidBoss(MidBossType::StoneGolem, x, y);
}
