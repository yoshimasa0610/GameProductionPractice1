#pragma once
#include "SkillData.h"
#include "../Player/Player.h"

class Skill
{
private:
    SkillData m_data;

    int m_currentCoolTime;
    int m_remainingUseCount;

    bool m_isActive;
    int m_activeTimer;

public:
    Skill(const SkillData& data);

    void Update(PlayerData* player);
    void Activate(PlayerData* player);

    bool CanUse() const;
    bool IsActive() const { return m_isActive; }

    int GetID() const { return m_data.id; }
    SkillType GetType() const { return m_data.type; }
};
