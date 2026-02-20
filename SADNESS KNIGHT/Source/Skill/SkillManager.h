#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Skill.h"

class SkillManager
{
private:
    std::vector<std::shared_ptr<Skill>> m_ownedSkills;

    // スキルID → 残り使用回数
    std::unordered_map<int, int> m_remainingUses;

    int m_equipSlots[2][3];
    int m_currentSet;

public:
    SkillManager();

    void AddSkill(const SkillData& data, PlayerData* player);
    void EquipSkill(int setIndex, int slotIndex, int skillID);

    void ChangeSet();
    void UseSkill(int slotIndex, PlayerData* player);

    void Update(PlayerData* player);
    // 再計算
    void RecalculateUses(PlayerData* player);

    const std::vector<std::shared_ptr<Skill>>& GetSkills() const
    {
        return m_ownedSkills;
    }
};
