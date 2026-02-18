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

    if (m_data.type == SkillType::Attack)
    {
        // コンボがある場合
        if (!m_data.comboSteps.empty())
        {
            if (m_comboTimer > 0)
            {
                m_comboIndex++;
                if (m_comboIndex >= m_data.comboSteps.size())
                    m_comboIndex = 0;
            }
            else
            {
                m_comboIndex = 0;
            }

            m_activeTimer = m_data.comboSteps[m_comboIndex].duration;
        }
        else
        {
            m_activeTimer = m_data.duration;
        }

        m_comboTimer = 20; // 次段受付時間
        m_isActive = true;
        m_hasHitThisStep = false;
    }

    m_currentCoolTime = m_data.coolTime;

    if (m_remainingUseCount > 0)
        m_remainingUseCount--;
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
    // コンボの受付時間の減少
    if (m_comboTimer > 0)
        m_comboTimer--;

    if (!m_isActive && m_comboTimer <= 0)
    {
        m_comboIndex = 0;
    }
}

// スキルの攻撃倍率の取得
float Skill::GetCurrentAttackRate() const
{
    if (m_data.type != SkillType::Attack)
        return 0.0f;

    if (!m_data.comboSteps.empty())
    {
        return m_data.comboSteps[m_comboIndex].attackRate;
    }

    return m_data.attackRate;
}

// ヒットフレーム判定
bool Skill::CanHit()
{
    if (!m_isActive) return false;
    if (m_hasHitThisStep) return false;
    if (m_data.comboSteps.empty()) return false;

    const ComboStep& step = m_data.comboSteps[m_comboIndex];

    int currentFrame = step.duration - m_activeTimer;

    if (currentFrame >= step.hitStartFrame &&
        currentFrame <= step.hitEndFrame)
    {
        m_hasHitThisStep = true;  // 1回だけ
        return true;
    }

    return false;
}