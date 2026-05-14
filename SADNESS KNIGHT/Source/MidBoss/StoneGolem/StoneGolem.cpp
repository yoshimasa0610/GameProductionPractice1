#include "../MidBossBase.h"
#include "StoneGolem.h"

namespace
{
    const int STONE_GOLEM_MAX_HP = 1500;
    const int STONE_GOLEM_ATTACK_POWER = 24;

    const StoneGolemTuning STONE_GOLEM_TUNING =
    {
        1.5f,   // shotPrepare - 通常弾の準備時間
        2.0f,   // beamPrepare - ビームの準備時間
        2.0f,   // shotRecovery - 通常弾後の硬直時間
        4.0f,   // beamRecovery - ビーム後の硬直時間
        0.5f,   // beamActive - ビームの持続時間
        1.0f,   // barrageDuration - 乱射の持続時間
        0.22f,  // barrageInterval - 乱射の弾の間隔
        6.0f,   // bulletSpeed - 弾の速度
        96.0f,  // bulletDrawSize - 弾の描画サイズ
        60.0f,  // bulletHitboxSize - 弾の当たり判定サイズ
        0.018f, // homingTurnRate - ホーミング回転速度
        0.0f,   // drawOffsetX - 描画オフセットX
        80.0f,  // drawOffsetY - 描画オフセットY
        100.0f, // beamDrawOffsetX - ビーム描画オフセットX
        28.0f,  // beamDrawOffsetY - ビーム描画オフセットY
        -70.0f  // beamMuzzleFineTuneX - ビーム発射位置微調整X
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
