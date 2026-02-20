#include "SkillManager.h"
#include <cmath>

SkillManager::SkillManager()
    : m_currentSet(0)
{
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            m_equipSlots[i][j] = -1;
}

// スキルの発動回数計算して登録
void SkillManager::AddSkill(const SkillData& data, PlayerData* player)
{
    auto skill = std::make_shared<Skill>(data);

    // ★Follow弾薬消費コールバック
    skill->SetConsumeCallback(
        [this](int id)
        {
            auto it = m_remainingUses.find(id);
            if (it == m_remainingUses.end())
                return;

            int& remain = it->second;

            if (remain > 0)
                remain--;

            // 0になったら消滅
            if (remain == 0)
            {
                for (auto& s : m_ownedSkills)
                {
                    if (s->GetID() == id)
                        s->ForceEnd();
                }
            }
        });

    m_ownedSkills.push_back(skill);

    int base = data.maxUseCount;

    if (base < 0)
    {
        m_remainingUses[data.id] = -1;
        return;
    }

    float rate = 1.0f + player->skillCountRate;
    m_remainingUses[data.id] = (int)ceil(base * rate);
}

void SkillManager::EquipSkill(int setIndex, int slotIndex, int skillID)
{
    m_equipSlots[setIndex][slotIndex] = skillID;
}

void SkillManager::ChangeSet()
{
    m_currentSet = 1 - m_currentSet;
}

void SkillManager::UseSkill(int slotIndex, PlayerData* player)
{
    int skillID = m_equipSlots[m_currentSet][slotIndex];
    if (skillID == -1) return;

    for (auto& skill : m_ownedSkills)
    {
        if (skill->GetID() != skillID)
            continue;

        // 回数チェック
        auto it = m_remainingUses.find(skillID);
        if (it == m_remainingUses.end()) return;

        int& remain = it->second;
        if (remain == 0)
            return;

        // 攻撃型排他
        if (skill->GetType() == SkillType::Attack)
        {
            for (auto& s : m_ownedSkills)
            {
                if (s->GetType() == SkillType::Attack && s->IsActive())
                    return;
            }
        }

        // Attack / Summonだけ即消費
        if (skill->GetType() != SkillType::Follow)
        {
            auto it = m_remainingUses.find(skillID);
            if (it != m_remainingUses.end())
            {
                int& remain = it->second;

                if (remain == 0)
                    return;

                if (remain > 0)
                    remain--;
            }
        }

        skill->Activate(player);

        return;
    }
}

void SkillManager::Update(PlayerData* player)
{
    for (auto& skill : m_ownedSkills)
    {
        skill->Update(player);
    }
}

// 装備・アイテム変更時に呼ぶ
void SkillManager::RecalculateUses(PlayerData* player)
{
    for (auto& skill : m_ownedSkills)
    {
        int id = skill->GetID();
        int base = skill->GetBaseUseCount();

        if (base < 0)
        {
            m_remainingUses[id] = -1;
            continue;
        }

        float rate = 1.0f + player->skillCountRate;
        m_remainingUses[id] = (int)ceil(base * rate);
    }
}
