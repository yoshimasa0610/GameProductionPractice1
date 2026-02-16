#include "Skill.h"

Skill::Skill(const SkillData& data)
    : m_data(data),
    m_currentCoolTime(0),
    m_remainingUseCount(data.maxUseCount),
    m_isActive(false),
    m_activeTimer(0)
{
}

bool Skill::CanUse() const
{
    if (m_currentCoolTime > 0) return false;
    if (m_remainingUseCount == 0) return false;
    return true;
}

void Skill::Activate(PlayerData* player)
{
    if (!CanUse()) return;

    m_currentCoolTime = m_data.coolTime;

    if (m_remainingUseCount > 0)
        m_remainingUseCount--;

    if (m_data.type == SkillType::Attack)
    {
        player->state = PlayerState::UsingSkill;
        m_isActive = true;
        m_activeTimer = m_data.duration;
    }
}

void Skill::Update(PlayerData* player)
{
    if (m_currentCoolTime > 0)
        m_currentCoolTime--;

    if (m_isActive)
    {
        m_activeTimer--;

        if (m_activeTimer <= 0)
        {
            m_isActive = false;
            player->state = PlayerState::Idle;
        }
    }
}
