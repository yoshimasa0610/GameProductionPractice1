#include "DxLib.h"
#include "UIImage.h"
#include "../Player/Player.h"
#include "../Scene/Play/Play.h"
#include "../Skill/Skill.h"
#include "../Skill/SkillData.h"
#include "../Skill/SkillManager.h"

// HPバー画像
static int g_HPBarFrame = -1;
static int g_HPBarFill = -1;

// 遅れて減るHPゲージ用
static float g_DamageHP = 0.0f;
static int g_DamageDelayTimer = 0;

static const int DAMAGE_DELAY = 30;     // 減り始めるまでの時間
static const float DAMAGE_SPEED = 0.02f; // 減る速度

// 回復アイコン
static int g_HealIcon = -1;

// UI位置
static const int HP_POS_X = 30;
static const int HP_POS_Y = 30;

static const int HEAL_POS_X = 30;
static const int HEAL_POS_Y = 80;

static const int SKILL_UI_X = 30;
static const int SKILL_UI_Y = 800;

static const int SKILL_ICON_SIZE = 40;
static const int SKILL_SPACING = 55;

void LoadUIImage()
{
    g_HPBarFrame = LoadGraph("Data/UI/hp_frame.png");
    g_HPBarFill = LoadGraph("Data/UI/hp_fill.png");
    g_HealIcon = LoadGraph("Data/UI/heal_icon.png");

    if (g_HealIcon == -1)
    {
        printfDx("heal_icon.png 読み込み失敗\n");
    }

    g_DamageHP = (float)GetPlayerHP();
}

void UpdateUIImage()
{
    int currentHP = GetPlayerHP();

    // ダメージを受けた瞬間
    if (currentHP < g_DamageHP)
    {
        g_DamageDelayTimer = DAMAGE_DELAY;
    }

    // 待機時間
    if (g_DamageDelayTimer > 0)
    {
        g_DamageDelayTimer--;
    }
    else
    {
        // 赤ゲージをゆっくり減らす
        g_DamageHP += (currentHP - g_DamageHP) * DAMAGE_SPEED;
    }

    // 回復した場合
    if (currentHP > g_DamageHP)
    {
        g_DamageHP = (float)currentHP;
    }
}

void DrawUIImage()
{
    int currentHP = GetPlayerHP();
    int maxHP = GetPlayerMaxHP();

    float hpRate = (float)currentHP / (float)maxHP;
    float damageRate = g_DamageHP / (float)maxHP;

    int barWidth = 200;
    int barHeight = 15;

    int hpWidth = (int)(barWidth * hpRate);
    int damageWidth = (int)(barWidth * damageRate);

    // 最大HP（黒バー）
    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + barWidth,
        HP_POS_Y + 5 + barHeight,
        GetColor(0, 0, 0),
        TRUE
    );

    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + damageWidth,
        HP_POS_Y + 5 + barHeight,
        GetColor(220, 40, 40),
        TRUE
    );

    // 現在HP（白バー）
    DrawBox(
        HP_POS_X + 5,
        HP_POS_Y + 5,
        HP_POS_X + 5 + hpWidth,
        HP_POS_Y + 5 + barHeight,
        GetColor(225, 255, 255),
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

    // =====================
// スキルUI
// =====================

    int currentSet = g_SkillManager.GetCurrentSet();

    for (int i = 0; i < 3; i++)
    {
        int skillID = g_SkillManager.GetEquipSkill(currentSet, i);

        if (skillID < 0)
            continue;

        const SkillData& data = GetSkillData(skillID);

        int x = SKILL_UI_X + i * SKILL_SPACING;
        int y = SKILL_UI_Y;

        // アイコン
        DrawExtendGraph(
            x,
            y,
            x + SKILL_ICON_SIZE,
            y + SKILL_ICON_SIZE,
            data.iconSmallHandle,
            TRUE
        );

        // 残り回数
        int remain = g_SkillManager.GetRemainingUses(skillID);

        if (remain >= 0)
        {
            DrawFormatString(
                x,
                y + SKILL_ICON_SIZE + 2,
                GetColor(255, 255, 255),
                "%d",
                remain
            );
        }
        else
        {
            DrawFormatString(
                x,
                y + SKILL_ICON_SIZE + 2,
                GetColor(255, 255, 255),
                "∞"
            );
        }
    }
}

void UnloadUIImage()
{
    DeleteGraph(g_HPBarFrame);
    DeleteGraph(g_HPBarFill);
    DeleteGraph(g_HealIcon);
}