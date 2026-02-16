#pragma once

struct MoneyPopup
{
    bool active;
    int amount;
    float x, y;
    float timer;
};

void InitMoneyPopup();
void AddMoneyPopup(int amount);
void UpdateMoneyPopup(float deltaTime);
void DrawMoneyPopup();