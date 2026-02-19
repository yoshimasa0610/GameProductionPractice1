#include "ItemManager.h"
#include "Item.h"
#include "../Player/Player.h"
#include "DxLib.h"
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

//Playerが進んでいないのでコメントアウト中

int ItemManager::GetPlayerMaxSlots(const PlayerData* player) const
{
    // player->baseSlot を参照して装備による addMaxSlot を加える
    int baseSlot = 5;
    if (player) baseSlot = player->baseMaxSlot; // PlayerData に baseMaxSlot を追加しています

    int add = 0;
    for (auto& it : m_items) {
        if (it->isEquipped) add += it->buff.addMaxSlot;
    }
    return baseSlot + add;
}

void ItemManager::ApplyBuffsToPlayer(PlayerData* player)
{
    if (!player) return;

    // まずベース値に戻す（InitPlayer で base 値が設定されている前提）
    player->maxHP = player->baseMaxHp;
    player->maxSlot = player->baseMaxSlot;
    player->healPowerBonus = 0;

    // 合算
    int addMaxHp = 0;
    int addMaxSlot = 0;
    int addHealPower = 0;


    for (auto& it : m_items)
    {
        if (it->ownedCount <= 0)
            continue;

        // Passive → 所持数分適用
        if (it->type == ItemType::Passive)
        {
            addMaxHp += it->buff.addMaxHp * it->ownedCount;
            addMaxSlot += it->buff.addMaxSlot * it->ownedCount;
            addHealPower += it->buff.healPowerBonus * it->ownedCount;
        }

        // Equip → 装備中のみ
        if (it->type == ItemType::Equip && it->isEquipped)
        {
            addMaxHp += it->buff.addMaxHp;
            addMaxSlot += it->buff.addMaxSlot;
        }
    }

    // ===== HP上限増加（最大値が増えた分回復）=====
    int oldMax = player->maxHP;

    // 新しい最大HPを計算
    player->maxHP = player->baseMaxHp + addMaxHp;

    // 差分
    int diff = player->maxHP - oldMax;

    if (diff > 0)
    {
        player->currentHP += diff;

        if (player->currentHP > player->maxHP)
            player->currentHP = player->maxHP;
    }

    // ===== スロット =====
    player->maxSlot += addMaxSlot;

    // ===== 回復補正 =====
    player->healPowerBonus = addHealPower;
}


// 所持しているか
bool ItemManager_IsItemOwned(int itemId)
{
    const auto& items = g_ItemManager.GetAllItems();
    for (const auto& it : items)
        if (it->id == itemId && it->ownedCount > 0)
            return true;
    return false;
}

// 装備中のアイテム数
int ItemManager_GetEquippedItemCount()
{
    int count = 0;
    const auto& items = g_ItemManager.GetAllItems();
    for (const auto& it : items)
        if (it->isEquipped)
            count++;
    return count;
}

// 指定インデックスの装備中アイテムIDを返す
int ItemManager_GetEquippedItemID(int index)
{
    int i = 0;
    const auto& items = g_ItemManager.GetAllItems();
    for (const auto& it : items)
    {
        if (it->isEquipped)
        {
            if (i == index)
                return it->id;
            i++;
        }
    }
    return -1;
}

// アイテムを所持状態にする
void ItemManager_AddItem(int itemId)
{
    g_ItemManager.UnlockItem(itemId);
}

// 全装備解除
void ItemManager_ClearEquippedItems()
{
    auto& items = const_cast<std::vector<std::unique_ptr<Item>>&>(g_ItemManager.GetAllItems());
    for (auto& it : items)
        it->isEquipped = false;
}

// 所持・装備・状態を初期化
void ItemManager_ClearAllItems(void)
{
    auto& items =
        const_cast<std::vector<std::unique_ptr<Item>>&>(g_ItemManager.GetAllItems());

    for (auto& it : items)
    {
        it->ownedCount = 0;
        it->isEquipped = false;
    }
}

// 装備する（セーブデータ復元時などでPlayer不在）
void ItemManager_EquipItem(int itemId)
{
    g_ItemManager.EquipItem(itemId, nullptr);
}

std::vector<std::unique_ptr<Item>>& ItemManager::AccessItems()
{
    return m_items;
}


// アイテム削除（所持解除）
void ItemManager_RemoveItem(int itemId)
{
    //auto& items = const_cast<std::vector<std::unique_ptr<Item>>&>(g_ItemManager.GetAllItems());
    auto& items = g_ItemManager.AccessItems();
    for (auto& it : items)
    {
        if (it->id == itemId)
        {
            it->ownedCount = 0;
            it->isEquipped = false;
        }
    }
}

void ItemManager::LoadItemIcons()
{
    for (auto& it : m_items)
    {
        if (!it->iconSmallPath.empty())
        {
            it->iconSmallHandle = LoadGraph(it->iconSmallPath.c_str());
        }

        if (!it->iconLargePath.empty())
        {
            it->iconLargeHandle = LoadGraph(it->iconLargePath.c_str());
        }
    }
}
