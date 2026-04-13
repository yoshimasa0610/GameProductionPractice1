#pragma once

struct GarokTuning
{
    float prepareTime;
    float attackDuration1;
    float attackDuration2;
    float attackDuration3;
    float cooldown1;
    float cooldown2;
    float cooldown3;
    float barrageDuration;
    float barrageCooldown;
    float dashDuration;
    float dashSpeed;
    bool enableJump;
    float jumpInterval;
    float jumpHeight;
    float hitboxWidthRatio;
    float hitboxHeightRatio;
    float hitboxFrontRatio;
    float contactYRatio;
    float facingDeadzone;
    float arenaLeft;
    float arenaRight;
    float attackTriggerRange;
    float gravity;
    float maxFallSpeed;
    float drawOffsetX;
    float drawOffsetY;
    float drawWidthScale;
    bool debugDraw;
};

int SpawnGarok(float x, float y);
int GetGarokMaxHP();
int GetGarokAttackPower();
const GarokTuning& GetGarokTuning();