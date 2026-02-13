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
/*
int ItemManager::GetPlayerMaxSlots(const PlayerData* player) const
{
    // player->baseSlot を参照して装備による addMaxSlot を加える
    int baseSlot = 5;
    if (player) baseSlot = player->baseMaxSlot; // 後述で PlayerData に baseMaxSlot を追加しています

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
    /*player->maxHp = player->baseMaxHp;
    player->recoverWait = player->baseRecoverWait;
    player->maxSlot = player->baseMaxSlot;
    player->healPowerBonus = 0;
    player->damageReductionPercent = 0.0f;
    player->isShieldWhileRecover = false;
    player->lifeGainOnHitBonus = 0;
    player->hasLowHpDamageReduce = false;
    player->healOnHit = false;
    player->healOnHitValue = 0;
    player->healOnHitRate = 0;
    player->moveSpeedMul = 1.0f;


    player->hasSpecialWeapon = false;
    player->atkState.type = PlayerAttackType::None;
    
    // 合算
    int addMaxHp = 0;
    int addMaxLife = 0;
    int addLifeGainOnHit = 0;
    int reduceRecoverWait = 0;
    int minRecoverOverride = 0;
    int addMaxSlot = 0;
    float damageReduce = 0.0f;
    int addInvincibleOnHit = 0;

    for (auto& it : m_items)
    {
        if (!it->isEquipped)
            continue;
        
        addMaxHp += it->buff.addMaxHp;
        addMaxLife += it->buff.addMaxLife;
        addLifeGainOnHit += it->buff.addLifeGainOnHit;
        reduceRecoverWait += it->buff.reduceRecoverWait;
        if (it->buff.minRecoverWaitOverride > 0) {
            // 最小値は小さい方（より短縮される方）を適用
            if (minRecoverOverride == 0) minRecoverOverride = it->buff.minRecoverWaitOverride;
            else minRecoverOverride = std::min(minRecoverOverride, it->buff.minRecoverWaitOverride);
        }
        addMaxSlot += it->buff.addMaxSlot;
        damageReduce += it->buff.damageReductionPercent;
        addInvincibleOnHit += it->buff.addInvincibleOnHit;
        //if (it->buff.shieldWhileRecover) ShieldWhileRecover = true;
        if (it->buff.reduceDamageWhenLowHp)
            player->hasLowHpDamageReduce = true;

        const BuffEffect& buff = it->buff;
        
    }

    // 適用（上限や下限を考慮）
    /*player->maxHp = std::max(1, player->baseMaxHp + addMaxHp);
    player->hp = std::min(player->hp, player->maxHp);
    player->lifeGainOnHitBonus = addLifeGainOnHit;
    // recoverWait を減らす（ただし minRecoverWaitOverride がある場合はそれを尊重）
    int newRecover = player->baseRecoverWait - reduceRecoverWait;
    if (minRecoverOverride > 0) newRecover = std::max(newRecover, minRecoverOverride);
    // 既存の最小値 PLAYER_RECOVER_MIN を下回らせない
    if (newRecover < PLAYER_RECOVER_MIN) newRecover = PLAYER_RECOVER_MIN;
    player->recoverWait = newRecover;
    //player->isShieldWhileRecover = shieldWhileRecover;
    player->damageReductionPercent = damageReduce; // 合算（注意: 複数装備時の処理はデザイン次第）
    player->invincibleOnHitBonus = addInvincibleOnHit;
    player->maxSlot = player->baseMaxSlot + addMaxSlot;
}

bool ItemManager::HasEquippedSpecial() const
{
    for (const auto& it : m_items)
    {
        if (it->isEquipped &&
        {
            return true;
        }
    }
    return false;
}

// 所持しているか
bool ItemManager_IsItemOwned(int itemId)
{
    const auto& items = g_ItemManager.GetAllItems();
    for (const auto& it : items)
        if (it->id == itemId && it->isOwned)
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
        it->isOwned = false;
        it->isEquipped = false;
    }
}

// 装備する（セーブデータ復元時などでPlayer不在）
void ItemManager_EquipItem(int itemId)
{
    g_ItemManager.EquipItem(itemId, nullptr);
}

// アイテム削除（所持解除）
void ItemManager_RemoveItem(int itemId)
{
    auto& items = const_cast<std::vector<std::unique_ptr<Item>>&>(g_ItemManager.GetAllItems());
    for (auto& it : items)
        if (it->id == itemId)
            it->isOwned = false;
}*/