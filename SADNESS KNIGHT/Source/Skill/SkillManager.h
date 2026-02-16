#pragma once
#include <vector>
#include <memory>
#include "Skill.h"

class SkillManager
{
private:
    std::vector<std::shared_ptr<Skill>> m_ownedSkills;

    int m_equipSlots[2][3];
    int m_currentSet;

public:
    SkillManager();

    void AddSkill(const SkillData& data);
    void EquipSkill(int setIndex, int slotIndex, int skillID);

    void ChangeSet();
    void UseSkill(int slotIndex, PlayerData* player);

    void Update(PlayerData* player);
};
