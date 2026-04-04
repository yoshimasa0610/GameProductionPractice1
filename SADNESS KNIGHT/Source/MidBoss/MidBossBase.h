#pragma once

enum class MidBossType
{
    StoneGolem,
    Count
};

void InitMidBossSystem();
void UpdateMidBosses();
void DrawMidBosses();
void ClearMidBosses();

int SpawnMidBoss(MidBossType type, float x, float y);

bool DamageMidBossByColliderId(int colliderId, int damage);
bool IsMidBossAlive();
int GetMidBossHP();
int GetMidBossMaxHP();
