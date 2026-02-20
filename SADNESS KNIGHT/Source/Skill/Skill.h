#pragma once
#include "SkillData.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include <unordered_set>

class Skill
{
private:
    SkillData m_data;

    int m_currentCoolTime;
    int m_remainingUseCount;
    bool m_isActive;
    int m_activeTimer;
    int m_comboIndex = 0;
    int m_comboTimer = 0;
    // Follow—p
    float m_followOffsetX = 60.0f;
    int m_followAttackInterval = 60;
    int m_followAttackTimer = 0;
    ColliderId m_followCollider = -1;
    //@SummonŒ^‚Ìˆ—
    float m_summonX = 0;
    float m_summonY = 0;
    ColliderId m_summonCollider = -1;
    ColliderId m_attackCollider = -1;

    // “G‚²‚Æ‚Ìƒqƒbƒg—š—ğ
    std::unordered_set<void*> m_hitTargets;

public:
    Skill(const SkillData& data);

    void Update(PlayerData* player);
    void Activate(PlayerData* player);

    bool CanUse() const;
    bool IsActive() const { return m_isActive; }

    int GetID() const { return m_data.id; }
    SkillType GetType() const { return m_data.type; }

    float GetCurrentAttackRate() const;

    // ƒqƒbƒgŠÇ—
    bool RegisterHit(void* target);
    void ClearHitTargets();
};
