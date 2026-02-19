#include "Skill.h"

// スキルの攻撃の判定
static void CreateAttackCollider(
    ColliderId& collider,
    const ComboStep& step,
    PlayerData* player,
    Skill* owner)
{
    float left =
        player->posX
        + (player->isFacingRight ? step.offsetX : -step.offsetX - step.hitWidth);

    float top =
        player->posY - PLAYER_HEIGHT + step.offsetY;

    collider = CreateCollider(
        ColliderTag::Other,
        left,
        top,
        step.hitWidth,
        step.hitHeight,
        owner);
}

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

            const ComboStep& step = m_data.comboSteps[m_comboIndex];

            m_activeTimer = step.duration;

            // ヒット履歴リセット
            ClearHitTargets();

            // コライダー生成
            if (m_attackCollider != -1)
            {
                DestroyCollider(m_attackCollider);
                m_attackCollider = -1;
            }
            // 攻撃コライダー生成
            CreateAttackCollider(m_attackCollider, step, player, this);
        }
        else
        {
            m_activeTimer = m_data.duration;
            ClearHitTargets();
        }

        m_comboTimer = 20; // 次段受付時間
        m_isActive = true;
    }

    m_currentCoolTime = m_data.coolTime;

    if (m_remainingUseCount > 0)
        m_remainingUseCount--;
}

void Skill::Update(PlayerData* player)
{
    if (m_currentCoolTime > 0)
        m_currentCoolTime--;

    if (m_activeTimer <= 0)
    {
        m_isActive = false;
        player->state = PlayerState::Idle;

        // 攻撃コライダー破棄
        if (m_attackCollider != -1)
        {
            DestroyCollider(m_attackCollider);
            m_attackCollider = -1;
        }
    }

    // コンボの受付時間の減少
    if (m_comboTimer > 0)
        m_comboTimer--;

    if (!m_isActive && m_comboTimer <= 0)
    {
        m_comboIndex = 0;
    }

    // 攻撃コライダー更新
    if (m_attackCollider != -1 && !m_data.comboSteps.empty())
    {
        const ComboStep& step = m_data.comboSteps[m_comboIndex];

        float left =
            player->posX
            + (player->isFacingRight ? step.offsetX : -step.offsetX - step.hitWidth);

        float top = player->posY - PLAYER_HEIGHT + step.offsetY;

        UpdateCollider(
            m_attackCollider,
            left,
            top,
            step.hitWidth,
            step.hitHeight);
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

// 敵に攻撃がヒットしたことを記録（多段ヒット対策）
bool Skill::RegisterHit(void* target)
{
    if (m_hitTargets.find(target) != m_hitTargets.end())
        return false;

    m_hitTargets.insert(target);
    return true;
}

// ヒット履歴クリア
void Skill::ClearHitTargets()
{
    m_hitTargets.clear();
}