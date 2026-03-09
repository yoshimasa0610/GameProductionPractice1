#include "DxLib.h"
#include "UIImage.h"
#include "../Player/Player.h"

// HPバー画像
static int g_HPBarFrame = -1;
static int g_HPBarFill = -1;

// 回復アイコン
static int g_HealIcon = -1;

// UI位置
static const int HP_POS_X = 30;
static const int HP_POS_Y = 30;

static const int HEAL_POS_X = 30;
static const int HEAL_POS_Y = 80;

void LoadUIImage()
{
    g_HPBarFrame = LoadGraph("Data/UI/hp_frame.png");
    g_HPBarFill = LoadGraph("Data/UI/hp_fill.png");
    g_HealIcon = LoadGraph("Data/UI/heal_icon.png");
}

void UpdateUIImage()
{

}

void DrawUIImage()
{
    int currentHP = GetPlayerHP();
    int maxHP = GetPlayerMaxHP();

    // HP割合
    float hpRate = (float)currentHP / (float)maxHP;

    // HPバー描画
    int barWidth = 200;
    int hpWidth = (int)(barWidth * hpRate);

    DrawGraph(HP_POS_X, HP_POS_Y, g_HPBarFrame, TRUE);

    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + hpWidth,
        HP_POS_Y + 20,
        GetColor(200, 0, 0),
        TRUE
    );

    // 回復回数UI
    int healCount = GetHealCount();
    int maxHeal = GetMaxHealCount();

    for (int i = 0; i < maxHeal; i++)
    {
        int x = HEAL_POS_X + i * 40;
        int y = HEAL_POS_Y;

        if (i < healCount)
        {
            DrawGraph(x, y, g_HealIcon, TRUE);
        }
        else
        {
            SetDrawBright(100, 100, 100);
            DrawGraph(x, y, g_HealIcon, TRUE);
            SetDrawBright(255, 255, 255);
        }
    }
}

void UnloadUIImage()
{
    DeleteGraph(g_HPBarFrame);
    DeleteGraph(g_HPBarFill);
    DeleteGraph(g_HealIcon);
}