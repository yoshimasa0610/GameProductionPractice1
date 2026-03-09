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
    int m_lastUsedSkillID = -1;

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

    int GetLastUsedSkillID() const { return m_lastUsedSkillID; }
    void ClearLastUsedSkillID() { m_lastUsedSkillID = -1; }

    int GetEquipSkill(int set, int slot) const;
    bool IsSkillEquipped(int skillID) const;
    int GetCurrentSet() const { return m_currentSet; }
    void SetCurrentSet(int s) { m_currentSet = s; }
    void UnequipSkill(int set, int slot);
    SkillType GetSkillTypeInSlot(int slotIndex) const;
};

extern SkillManager g_SkillManager;
