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

    Slash->coolTime = 60;
    Slash->maxUseCount = -1;

    // 通常値（未使用でもOK）
    Slash->attackRate = 1.1f;
    Slash->duration = 30;

    // ===== コンボ定義 =====
    Slash->comboSteps =
    {//左から威力、モーションの時間、当たり判定の開始、当たり判定の終了
        { 0.95f, 20, 5, 10 },   // 1段目
        { 1.0f, 22, 6, 12 },   // 2段目
        { 1.1, 30, 8, 16 }    // 3段目
    };

    Slash->iconSmallPath = "Data/Skill/Icon_Small_Slash.png";
    Slash->iconLargePath = "Data/Skill/Icon_Large_Slash.png";

    Slash->systemDesc =
    {
        "前方に対して近接攻撃を行う。\n",
        "連続して攻撃を行うとコンボ攻撃に派生。\n"
        "徐々にダメージ画高くなる\n"
        "攻撃力25 / CT120"
    };

    Slash->flavorDesc =
    {
        "騎士の剣。",
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
        "かつて王に仕えた影の戦士。",
        "主を守るため戦い続ける。"
    };

    g_SkillDatabase.push_back(std::move(shadow));
}
