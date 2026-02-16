#pragma once
#include <stdint.h>

#define MAX_MONEY_DROPS 128

enum MoneyVisualType
{
    MONEY_VISUAL_SMALL,
    MONEY_VISUAL_MIDDLE,
    MONEY_VISUAL_LARGE,
};

struct MoneyDrop
{
    bool active;
    float x, y;
    float vx, vy;
    int value;       // このコインの価値（例：5G, 12G）
    int radius;      // プレイヤーとの接触用
    MoneyVisualType visual;
};

void InitMoneyDrops();
void UpdateMoneyDrops(float playerX, float playerY);
void DrawMoneyDrops();
void SpawnMoneyDrops(float x, float y, int totalMoney);
// 石像破壊時のドロップ金額を決定
int DecideStatueDropMoney();