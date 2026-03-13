#include "SkillManager.h"
#include <cmath>

SkillManager g_SkillManager;

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

    // Follow弾薬消費コールバック
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
	// スキルリストに追加
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
/*
void SkillManager::EquipSkill(int setIndex, int slotIndex, int skillID)
{
    m_equipSlots[setIndex][slotIndex] = skillID;
}*/
// セット切り替え
void SkillManager::ChangeSet()
{
    m_currentSet = 1 - m_currentSet;
}
// スキル使用
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

        // 攻撃タイプ制限
        if (skill->GetType() == SkillType::Attack)
        {
            for (auto& s : m_ownedSkills)
            {
                if (s->GetType() == SkillType::Attack && s->IsActive() && s->GetID() != skillID)
                    return;
            }
        }

        // Attack / Summon は即消費
        if (skill->GetType() != SkillType::Follow)
        {
            auto it = m_remainingUses.find(skillID);
            if (it != m_remainingUses.end())
            {
                int& remain = it->second;

                if (remain == 0)
                    return;

                if (remain > 0 && !skill->IsActive())
                    remain--;
            }
        }
		// スキル発動
        skill->Activate(player);

		// 最後に使ったスキルID更新
        m_lastUsedSkillID = skillID;

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
// 装備スロットのスキルIDを取得
int SkillManager::GetEquipSkill(int set, int slot) const
{
    return m_equipSlots[set][slot];
}

bool SkillManager::IsSkillEquipped(int skillID) const
{
    for (int s = 0; s < 2; s++)
    {
        for (int i = 0; i < 3; i++)
        {
            if (m_equipSlots[s][i] == skillID)
                return true;
        }
    }

    return false;
}

// スキル外す
void SkillManager::UnequipSkill(int set, int slot)
{
    m_equipSlots[set][slot] = -1;
}
// スキル装備。重複チェックして古い方外す
void SkillManager::EquipSkill(int setIndex, int slotIndex, int skillID)
{
    // 同一セット内重複チェック
    for (int i = 0; i < 3; i++)
    {
        if (i == slotIndex) continue;

        if (m_equipSlots[setIndex][i] == skillID)
        {
            // 古い方外す
            m_equipSlots[setIndex][i] = -1;
        }
    }

    m_equipSlots[setIndex][slotIndex] = skillID;
}

SkillType SkillManager::GetSkillTypeInSlot(int slotIndex) const
{
    int skillID = m_equipSlots[m_currentSet][slotIndex];

    for (auto& s : m_ownedSkills)
    {
        if (s->GetID() == skillID)
            return s->GetType();
    }

    return SkillType::None;
}

int SkillManager::GetRemainingUses(int skillID) const
{
    auto it = m_remainingUses.find(skillID);

    if (it == m_remainingUses.end())
        return 0;

    return it->second;
}

Skill* SkillManager::GetEquippedSkill(int set, int slot) const
{
    int skillID = m_equipSlots[set][slot];

    if (skillID == -1)
        return nullptr;

    for (auto& s : m_ownedSkills)
    {
        if (s->GetID() == skillID)
            return s.get();
    }

    return nullptr;
}