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
    m_isActive(false),
    m_activeTimer(0)
{
}

bool Skill::CanUse() const
{
    if (m_currentCoolTime > 0) return false;
    return true;
}

void Skill::StartCoolTime()
{
    m_currentCoolTime = m_data.coolTime;
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

    if (m_data.type == SkillType::Follow)
    {
        m_isActive = true;
        m_activeTimer = m_data.duration;
        m_followAttackTimer = 0;

        float offset = player->isFacingRight ? m_followOffsetX : -m_followOffsetX;

        m_followPosX = player->posX + offset;
        m_followPosY = player->posY - PLAYER_HEIGHT;

        m_followCollider = CreateCollider(
            ColliderTag::Other,
            m_followPosX,
            m_followPosY,
            60,
            60,
            this);
    }

    if (m_data.type == SkillType::Summon)
    {
        // 同時数制限
        if ((int)m_summons.size() >= m_maxSummons)
            return;

        SummonUnit unit;

        unit.x = player->posX;
        unit.y = player->posY;
        unit.timer = m_data.duration;

        unit.collider = CreateCollider(
            ColliderTag::Other,
            unit.x - 40,
            unit.y - 80,
            80,
            80,
            this);

        m_summons.push_back(unit);

        m_isActive = true;

        ClearHitTargets();
    }

    StartCoolTime();
}

void Skill::Update(PlayerData* player)
{
    if (m_currentCoolTime > 0)
        m_currentCoolTime--;

    if (m_isActive && m_data.type == SkillType::Attack)
    {
        if (m_activeTimer > 0)
            m_activeTimer--;

        if (m_activeTimer <= 0)
        {
            m_isActive = false;

            // 攻撃コライダー破棄
            if (m_attackCollider != -1)
            {
                DestroyCollider(m_attackCollider);
                m_attackCollider = -1;
            }
        }
    }

    // コンボの受付時間の減少
    if (m_comboTimer > 0)
        m_comboTimer--;

    if (!m_isActive && m_comboTimer <= 0)
    {
        m_comboIndex = 0;
    }

    if (m_isActive && m_data.type == SkillType::Attack)
    {
        m_activeTimer--;
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

    // 追従型の処理
    if (m_data.type == SkillType::Follow && m_isActive)
    {
        m_activeTimer--;

        if (m_activeTimer <= 0)
        {
            m_isActive = false;
            if (m_followCollider != -1)
            {
                DestroyCollider(m_followCollider);
                m_followCollider = -1;
            }
            return;
        }

        // プレイヤーに追従
        float offset = player->isFacingRight ? m_followOffsetX : -m_followOffsetX;

        float targetX = player->posX + offset;
        float targetY = player->posY - PLAYER_HEIGHT;

        // Lerp
        m_followPosX += (targetX - m_followPosX) * m_followLerpSpeed;
        m_followPosY += (targetY - m_followPosY) * m_followLerpSpeed;

        UpdateCollider(m_followCollider, m_followPosX, m_followPosY, 60, 60);

        // 攻撃間隔
        if (m_followAttackTimer > 0)
            m_followAttackTimer--;

        if (m_followAttackTimer <= 0)
        {
            ClearHitTargets();
            m_followAttackTimer = m_followAttackInterval;
            if (m_onConsumeUse)
                m_onConsumeUse(m_data.id);
        }
    }

    if (m_data.type == SkillType::Summon)
    {
        for (int i = (int)m_summons.size() - 1; i >= 0; --i)
        {
            auto& s = m_summons[i];

            s.timer--;

            if (s.timer <= 0)
            {
                if (s.collider != -1)
                    DestroyCollider(s.collider);

                m_summons.erase(m_summons.begin() + i);
                continue;
            }

            // 位置更新（固定設置）
            UpdateCollider(
                s.collider,
                s.x - 40,
                s.y - 80,
                80,
                80);
        }

        if (m_summons.empty())
            m_isActive = false;
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

void Skill::ForceEnd()
{
    m_isActive = false;

    if (m_followCollider != -1)
    {
        DestroyCollider(m_followCollider);
        m_followCollider = -1;
    }

    if (m_summonCollider != -1)
    {
        DestroyCollider(m_summonCollider);
        m_summonCollider = -1;
    }

    for (auto& s : m_summons)
    {
        if (s.collider != -1)
            DestroyCollider(s.collider);
    }
    m_summons.clear();
}