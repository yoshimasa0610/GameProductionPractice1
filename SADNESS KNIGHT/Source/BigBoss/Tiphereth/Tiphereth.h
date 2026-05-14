#pragma once

struct TipherethTuning
{
    float bodyHalfWidthRatio;       // 本体半幅比率
    float bodyWidthRatio;           // 本体幅比率
    float bodyHeightRatio;          // 本体高さ比率
    float bodyTopOffsetRatio;       // 本体上部オフセット比率
    float bodyFacingOffsetRatio;    // 本体向きオフセット比率
    float tentacleWidth;            // 触手の幅
    float tentacleHeight;           // 触手の高さ
    float tentacleFrontOffset;      // 触手前方オフセット
    float tentacleCenterYOffset;    // 触手中心Yオフセット
    float biteWidth;                // 噪みつきの幅
    float biteHeight;               // 噪みつきの高さ
    float biteFrontOffset;          // 噪みつき前方オフセット
    float biteCenterYOffset;        // 噪みつき中心Yオフセット
    float chargeWidth;              // 突進の幅
    float chargeHeight;             // 突進の高さ
    float chargeFrontOffset;        // 突進前方オフセット
    float chargeCenterYOffset;      // 突進中心Yオフセット
    float stompRange;               // 踏みつけ範囲
    float stompHitboxYOffset;       // 踏みつけ当たり判定Yオフセット
    float chargePriorityRange;      // 突進優先範囲
    float chargeSpeed;              // 突進速度
    float chargeThroughDistance;    // 突進貫通距離
    float prepareDuration;          // 準備時間
    float stompPrepareDuration;     // 踏みつけ準備時間
    float drawOffsetX;              // 描画オフセットX
    float drawOffsetY;              // 描画オフセットY
    bool reverseDrawFacing;         // 描画向き反転
    bool debugDraw;                 // デバッグ描画
};

int SpawnTiphereth(float x, float y);
int GetTipherethMaxHP();
int GetTipherethAttackPower();
const TipherethTuning& GetTipherethTuning();