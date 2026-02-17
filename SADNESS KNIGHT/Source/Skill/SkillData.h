#pragma once
#include <string>
#include "DxLib.h"
#include <vector>

enum class SkillType
{
    Attack,
    Follow,
    Summon
};

struct ComboStep
{
    int power;          // この段階の威力
    int duration;       // モーション時間
    int hitStartFrame;  // 当たり判定開始
    int hitEndFrame;    // 当たり判定終了
};

struct SkillData
{
    int id;                     // スキルID
    std::string name;           // スキル名
    SkillType type;             // 種別

    int coolTime;               // CT（フレーム）
    int maxUseCount;            // 最大使用回数（-1なら無限）

    int power;                  // 威力
    int duration;               // 持続時間（召喚・追従用）

    // コンボ用
    std::vector<ComboStep> comboSteps;
    // 画像
    std::string iconSmallPath;
    std::string iconLargePath;

    int iconSmallHandle = -1;
    int iconLargeHandle = -1;

    std::vector<std::string> systemDesc;  // 性能説明
    std::vector<std::string> flavorDesc;  // 世界観説明

    // 将来的にエフェクトや画像ハンドル追加可能
};
const SkillData& GetSkillData(int skillID);
