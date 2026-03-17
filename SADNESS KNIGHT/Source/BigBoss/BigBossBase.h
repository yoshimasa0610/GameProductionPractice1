#pragma once

enum class BigBossType
{
    Kether,
    Count
};

void InitBigBossSystem();
void UpdateBigBosses();
void DrawBigBosses();
void ClearBigBosses();

int SpawnBigBoss(BigBossType type, float x, float y);

bool DamageBigBossByColliderId(int colliderId, int damage);
