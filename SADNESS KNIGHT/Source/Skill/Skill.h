#pragma once
#include "SkillData.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include <unordered_set>
#include <functional>

class Skill
{
private:
    SkillData m_data;

    int m_currentCoolTime;
    bool m_isActive;
    int m_activeTimer;
    int m_comboIndex = 0;
    int m_comboTimer = 0;
    // Follow用
    float m_followOffsetX = 60.0f;
    int m_followAttackInterval = 60;
    int m_followAttackTimer = 0;
    ColliderId m_followCollider = -1;
    //　Summon型の処理
    float m_summonX = 0;
    float m_summonY = 0;
    ColliderId m_summonCollider = -1;
    ColliderId m_attackCollider = -1;

    // 敵ごとのヒット履歴
    std::unordered_set<void*> m_hitTargets;
    std::function<void(int)> m_onConsumeUse;
public:
    Skill(const SkillData& data);

    void Update(PlayerData* player);
    void Activate(PlayerData* player);

    bool CanUse() const;
    bool IsActive() const { return m_isActive; }

    int GetID() const { return m_data.id; }
    SkillType GetType() const { return m_data.type; }
    int GetBaseUseCount() const { return m_data.maxUseCount; }
    float GetCurrentAttackRate() const;

    // ヒット管理
    bool RegisterHit(void* target);
    void ClearHitTargets();
    void StartCoolTime();
    // 強制終了（上限の使用回数まで消費したから）
    void ForceEnd();

    void SetConsumeCallback(std::function<void(int)> cb)
    {
        m_onConsumeUse = cb;
    }
};
