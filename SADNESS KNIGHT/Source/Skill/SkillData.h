#pragma once
#include <string>
#include "DxLib.h"

enum class SkillType
{
    Attack,
    Follow,
    Summon
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

    // 将来的にエフェクトや画像ハンドル追加可能
};
