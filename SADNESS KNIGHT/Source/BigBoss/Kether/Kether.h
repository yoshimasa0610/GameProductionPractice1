#pragma once

struct KetherTuning
{
    float bodyHalfWidthRatio;
    float bodyWidthRatio;
    float bodyHeightRatio;
    float bodyTopOffsetRatio;
    float flameOriginXRatio;
    float flameOriginYRatio;
    float flameDrawOffsetY;
    float bookOriginXRatio;
    float bookOriginYRatio;
    float phaseShockwaveDuration;
    float transformEndHold;
    float diedDrawOffsetY;
    float areaMinY;
    float areaMaxY;
    float flameWidth;
    float bookWidth;
    float bookHeight;
    bool debugDraw;
};

int SpawnKether(float x, float y);
int GetKetherMaxHP();
int GetKetherAttackPower();
const KetherTuning& GetKetherTuning();
