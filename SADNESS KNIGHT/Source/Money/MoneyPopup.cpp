#include "MoneyPopup.h"
#include "DxLib.h"

static const int MAX_POPUPS = 8;
static MoneyPopup g_popups[MAX_POPUPS];

void InitMoneyPopup()
{
    for (int i = 0; i < MAX_POPUPS; ++i)
        g_popups[i].active = false;
}

void AddMoneyPopup(int amount)
{
    for (int i = 0; i < MAX_POPUPS; ++i)
    {
        if (!g_popups[i].active)
        {
            g_popups[i].active = true;
            g_popups[i].amount = amount;
            g_popups[i].x = 120;   // ŠŽ‹àUI•t‹ß
            g_popups[i].y = 50;
            g_popups[i].timer = 1.0f;
            return;
        }
    }
}

void UpdateMoneyPopup(float deltaTime)
{
    for (int i = 0; i < MAX_POPUPS; ++i)
    {
        MoneyPopup& p = g_popups[i];
        if (!p.active) continue;

        p.y -= 20.0f * deltaTime;
        p.timer -= deltaTime;

        if (p.timer <= 0.0f)
            p.active = false;
    }
}

void DrawMoneyPopup()
{
    for (int i = 0; i < MAX_POPUPS; ++i)
    {
        MoneyPopup& p = g_popups[i];
        if (!p.active) continue;

        int alpha = (int)(255 * p.timer);
        int color = GetColor(255, 255, 0);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        DrawFormatString((int)p.x, (int)p.y, color, "+%d G", p.amount);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}