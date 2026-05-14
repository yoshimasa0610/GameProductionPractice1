#pragma once

struct KetherTuning
{
    float bodyHalfWidthRatio;       // 本体半幅比率
    float bodyWidthRatio;           // 本体幅比率
    float bodyHeightRatio;          // 本体高さ比率
    float bodyTopOffsetRatio;       // 本体上部オフセット比率
    float flameOriginXRatio;        // 炎発射位置X比率
    float flameOriginYRatio;        // 炎発射位置Y比率
    float flameDrawOffsetY;         // 炎描画オフセットY
    float bookOriginXRatio;         // 本発射位置X比率
    float bookOriginYRatio;         // 本発射位置Y比率
    float phaseShockwaveDuration;   // フェーズ衝撃波持続時間
    float transformEndHold;         // 変身終了ホールド時間
    float diedDrawOffsetY;          // 死亡時描画オフセットY
    float areaMinY;                 // エリア最小Y
    float areaMaxY;                 // エリア最大Y
    float flameWidth;               // 炎の幅
    float bookWidth;                // 本の幅
    float bookHeight;               // 本の高さ
    bool debugDraw;                 // デバッグ描画
};

int SpawnKether(float x, float y);
int GetKetherMaxHP();
int GetKetherAttackPower();
const KetherTuning& GetKetherTuning();
