#pragma once

struct StoneGolemTuning
{
    float shotPrepare;
    float beamPrepare;
    float shotRecovery;
    float beamRecovery;
    float beamActive;
    float barrageDuration;
    float barrageInterval;
    float bulletSpeed;
    float bulletDrawSize;
    float bulletHitboxSize;
    float drawOffsetX;
    float drawOffsetY;
    float beamDrawOffsetX;
    float beamDrawOffsetY;
    float beamMuzzleFineTuneX;
};

int SpawnStoneGolem(float x, float y);
int GetStoneGolemMaxHP();
int GetStoneGolemAttackPower();
const StoneGolemTuning& GetStoneGolemTuning();
