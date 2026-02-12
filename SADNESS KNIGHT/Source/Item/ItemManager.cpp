#include "ItemManager.h"
#include "Item.h"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <algorithm>

ItemManager g_ItemManager;

ItemManager::ItemManager() {}

void ItemManager::AddItem(std::unique_ptr<Item> item)
{
    m_items.push_back(std::move(item));
}

bool ItemManager::UnlockItem(int itemId)
{
    for (auto& it : m_items)
    {
        if (it->id == itemId)
        {
            if (it->type == ItemType::Passive)
            {
                it->ownedCount++;  // パッシブは重複可
            }
            else
            {
                it->ownedCount = 1; // Equipは1個のみ
            }
            return true;
        }
    }
    return false;
}

bool ItemManager::EquipItem(int itemId, PlayerData* player)
{
    // 該当アイテムを見つける
    Item* found = nullptr;
    for (auto& it : m_items)
    {
        if (it->id == itemId)
        {
            found = it.get();
            break;
        }
    }
    if (!found) return false;
    if (found->ownedCount <= 0) return false;
    if (found->isEquipped) return false;

    // スロットチェック：playerが渡されている場合のみ正確にチェックする
    if (player)
    {
        int used = GetUsedSlots();
        int maxSlots = GetPlayerMaxSlots(player);
        if (used + found->slotCost > maxSlots) {
            // スロット不足
            return false;
        }
    }
    else
    {
        // player == nullptr の場合（セーブ復元など）は
        // スロットチェックはスキップしてフラグだけ立てる（後で起動時に ApplyBuffsToPlayer を呼ぶ設計にする）
    }

    found->isEquipped = true;

    // 装備効果の反映は player が渡されている場合のみ行う
    if (player)
    {
        ApplyBuffsToPlayer(player);
    }
    return true;
}

bool ItemManager::UnequipItem(int itemId, PlayerData* player)
{
    if (!player) return false;
    for (auto& it : m_items) {
        if (it->id == itemId && it->isEquipped) {
            it->isEquipped = false;
            ApplyBuffsToPlayer(player);
            return true;
        }
    }
    return false;
}

const std::vector<std::unique_ptr<Item>>& ItemManager::GetAllItems() const
{
    return m_items;
}

int ItemManager::GetUsedSlots() const
{
    int sum = 0;
    for (auto& it : m_items) {
        if (it->isEquipped) sum += it->slotCost;
    }
    return sum;
}

