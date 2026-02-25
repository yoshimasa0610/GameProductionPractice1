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
// 追従型の攻撃判定
Skill::Skill(const SkillData& data)
    : m_data(data),
    m_currentCoolTime(0),
    m_isActive(false),
    m_activeTimer(0)
{
}
// スキルが使用可能か
bool Skill::CanUse() const
{
    if (m_currentCoolTime > 0) return false;
    return true;
}
// クールタイム開始
void Skill::StartCoolTime()
{
    m_currentCoolTime = m_data.coolTime;
}
// スキルの強制終了（CTは消費する）
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
			// コンボステップ取得
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
	// 追従型の処理
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
	// 召喚型の処理
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
		// ヒット履歴リセット
        ClearHitTargets();
    }
	// スキル使用時のコールバック
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
	// 召喚型の処理
    if (m_data.type == SkillType::Summon)
    {
        for (int i = (int)m_summons.size() - 1; i >= 0; --i)
        {
            auto& s = m_summons[i];

            // ===== 寿命 =====
            s.timer--;

            if (s.timer <= 0)
            {
                if (s.collider != -1)
                    DestroyCollider(s.collider);

                if (s.attackCollider != -1)
                    DestroyCollider(s.attackCollider);

                m_summons.erase(m_summons.begin() + i);
                continue;
            }

            // ===== 本体位置更新 =====
            UpdateCollider(
                s.collider,
                s.x - 40,
                s.y - 80,
                80,
                80);

            // ===== 攻撃タイマー =====
            if (s.attackTimer > 0)
                s.attackTimer--;

            // ===== 攻撃開始 =====
            if (s.attackTimer <= 0)
            {
                // ヒット履歴リセット
                ClearHitTargets();

                if (s.attackCollider != -1)
                    DestroyCollider(s.attackCollider);

                s.attackCollider = CreateCollider(
                    ColliderTag::Other,
                    s.x - m_summonAttackWidth * 0.5f,
                    s.y - m_summonAttackHeight,
                    m_summonAttackWidth,
                    m_summonAttackHeight,
                    this);

                s.attackTimer = s.attackInterval;
                // 個体の攻撃寿命
                s.attackLife = m_summonAttackDuration;

                // 弾薬消費（Summonは攻撃した時）
                if (m_onConsumeUse)
                    m_onConsumeUse(m_data.id);
            }

            // ===== 攻撃コライダー寿命 =====
            if (s.attackCollider != -1)
            {
                s.attackLife--;

                if (s.attackLife <= 0)
                {
                    DestroyCollider(s.attackCollider);
                    s.attackCollider = -1;
                }
            }
        }
		// 召喚ユニットがいなくなったらスキル非アクティブ
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
        if (s.attackCollider != -1)
            DestroyCollider(s.attackCollider);
    }
    m_summons.clear();
}