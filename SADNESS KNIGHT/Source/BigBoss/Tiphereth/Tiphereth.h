#pragma once

struct TipherethTuning
{
    float bodyHalfWidthRatio;
    float bodyWidthRatio;
    float bodyHeightRatio;
    float bodyTopOffsetRatio;
    float bodyFacingOffsetRatio;
    float tentacleWidth;
    float tentacleHeight;
    float tentacleFrontOffset;
    float tentacleCenterYOffset;
    float biteWidth;
    float biteHeight;
    float biteFrontOffset;
    float biteCenterYOffset;
    float chargeWidth;
    float chargeHeight;
    float chargeFrontOffset;
    float chargeCenterYOffset;
    float stompRange;
    float stompHitboxYOffset;
    float chargePriorityRange;
    float chargeSpeed;
    float chargeThroughDistance;
    float prepareDuration;
    float stompPrepareDuration;
    float drawOffsetX;
    float drawOffsetY;
    bool reverseDrawFacing;
    bool debugDraw;
};

int SpawnTiphereth(float x, float y);
int GetTipherethMaxHP();
int GetTipherethAttackPower();
const TipherethTuning& GetTipherethTuning();