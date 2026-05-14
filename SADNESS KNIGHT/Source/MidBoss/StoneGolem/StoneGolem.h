#pragma once

struct StoneGolemTuning
{
    float shotPrepare;          // 通常弾の準備時間
    float beamPrepare;          // ビームの準備時間
    float shotRecovery;         // 通常弾後の硬直時間
    float beamRecovery;         // ビーム後の硬直時間
    float beamActive;           // ビームの持続時間
    float barrageDuration;      // 乱射の持続時間
    float barrageInterval;      // 乱射の弾の間隔
    float bulletSpeed;          // 弾の速度
    float bulletDrawSize;       // 弾の描画サイズ
    float bulletHitboxSize;     // 弾の当たり判定サイズ
    float homingTurnRate;       // ホーミング回転速度
    float drawOffsetX;          // 描画オフセットX
    float drawOffsetY;          // 描画オフセットY
    float beamDrawOffsetX;      // ビーム描画オフセットX
    float beamDrawOffsetY;      // ビーム描画オフセットY
    float beamMuzzleFineTuneX;  // ビーム発射位置微調整X
};

int SpawnStoneGolem(float x, float y);
int GetStoneGolemMaxHP();
int GetStoneGolemAttackPower();
const StoneGolemTuning& GetStoneGolemTuning();
