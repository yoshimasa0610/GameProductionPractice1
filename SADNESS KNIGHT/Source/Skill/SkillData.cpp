#include "SkillData.h"
#include <vector>
#include <memory>

static std::vector<std::unique_ptr<SkillData>> g_SkillDatabase;

void RegisterSkills()
{
    // ===============================
    // Slash
    auto Slash = std::make_unique<SkillData>();

    Slash->id = 1;
    Slash->name = "Slash";
    Slash->type = SkillType::Attack;

    Slash->coolTime = 10;
    Slash->maxUseCount = -1;

    // 通常値（未使用でもOK）
    Slash->attackRate = 1.1f;
    Slash->duration = 30;

    // ===== コンボ定義 =====
    Slash->comboSteps =
    {//左から威力、モーションの時間、当たり判定の開始、当たり判定の終了、幅、高さ、前方オフセット、Yオフセット
        { 0.95f, 27, 8, 14, 96.0f, 92.0f, 36.0f, -24.0f },   // 1段目
        { 1.0f, 11, 4, 7, 100.0f, 92.0f, 40.0f, -24.0f },    // 2段目
        { 1.1f, 16, 6, 11, 108.0f, 98.0f, 46.0f, -26.0f }    // 3段目
    };

    Slash->iconSmallPath = "Data/Skill/Icon_Small_Slash.png";
    Slash->iconLargePath = "Data/Skill/Icon_Large_Slash.png";

    Slash->systemDesc =
    {
        "前方に対して近接攻撃を行う。\n",
        "連続して攻撃を行うとコンボ攻撃に派生。\n"
        "徐々にダメージが高くなる\n"
        "攻撃力25 / CT120"
    };

    Slash->flavorDesc =
    {
        "\n\n騎士の剣。",
    };

    g_SkillDatabase.push_back(std::move(Slash));


    // ===============================
    // Shadow Follower
    auto shadow = std::make_unique<SkillData>();

    shadow->id = 2;
    shadow->name = "Shadow Follower";
    shadow->type = SkillType::Follow;

    shadow->coolTime = 300;
    shadow->maxUseCount = 50;

    shadow->duration = 600;

    shadow->iconSmallPath = "Data/Skill/Icon_Small_Shadow.png";
    shadow->iconLargePath = "Data/Skill/Icon_Large_Shadow.png";

    shadow->systemDesc =
    {
        "影の騎士を召喚する。",
        "一定時間自動攻撃を行う。"
    };

    shadow->flavorDesc =
    {
        "",
        ""
    };

    g_SkillDatabase.push_back(std::move(shadow));


    // ===============================
    // 画像ロード
    // ===============================
    for (auto& s : g_SkillDatabase)
    {
        if (!s->iconSmallPath.empty())
        {
            s->iconSmallHandle = LoadGraph(s->iconSmallPath.c_str());
        }

        if (!s->iconLargePath.empty())
        {
            s->iconLargeHandle = LoadGraph(s->iconLargePath.c_str());
        }
    }
}

const SkillData& GetSkillData(int skillID)
{
    for (auto& s : g_SkillDatabase)
    {
        if (s->id == skillID)
        {
            return *s;
        }
    }

    static SkillData dummy{};
    return dummy;
}

