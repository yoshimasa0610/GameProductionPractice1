#include "SkillManager.h"

SkillManager::SkillManager()
    : m_currentSet(0)
{
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            m_equipSlots[i][j] = -1;
}

void SkillManager::AddSkill(const SkillData& data)
{
    m_ownedSkills.push_back(std::make_shared<Skill>(data));
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
        if (skill->GetID() == skillID)
        {
            skill->Activate(player);
        }
    }
}

void SkillManager::Update(PlayerData* player)
{
    for (auto& skill : m_ownedSkills)
    {
        skill->Update(player);
    }
}
