#include "DxLib.h"
#include "UIImage.h"
#include "../Player/Player.h"
#include "../Scene/Play/Play.h"

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

    if (g_HealIcon == -1)
    {
        printfDx("heal_icon.png 読み込み失敗\n");
    }
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

    
    int barHeight = 15;

    // 最大HP（黒バー）
    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + barWidth,
        HP_POS_Y + 5 + barHeight,
        GetColor(0, 0, 0),
        TRUE
    );

    // 現在HP（赤バー）
    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + hpWidth,
        HP_POS_Y + 5 + barHeight,
        GetColor(220, 40, 40),
        TRUE
    );

    // フレーム
    DrawGraph(HP_POS_X, HP_POS_Y, g_HPBarFrame, TRUE);

    // 回復回数UI
    int healCount = GetHealCount();
    int maxHeal = 3;

    for (int i = 0; i < maxHeal; i++)
    {
        int x = HEAL_POS_X + i * 40;
        int y = HEAL_POS_Y;

        if (i < healCount)
        {
            int size = 32;

            DrawExtendGraph(
                x,
                y,
                x + size,
                y + size,
                g_HealIcon,
                TRUE
            );
        }
        else
        {
            SetDrawBright(100, 100, 100);

            int size = 32;

            DrawExtendGraph(
                x,
                y,
                x + size,
                y + size,
                g_HealIcon,
                TRUE
            );

            SetDrawBright(255, 255, 255);
        }
    }
    //DrawGraph(300, 300, g_HealIcon, TRUE);
}

void UnloadUIImage()
{
    DeleteGraph(g_HPBarFrame);
    DeleteGraph(g_HPBarFill);
    DeleteGraph(g_HealIcon);
}