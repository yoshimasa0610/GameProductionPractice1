#pragma once
#include <vector>
#include <memory>
#include "Item.h"

// 前方宣言（Player側にある PlayerData を使う）
struct PlayerData;

class ItemManager
{
public:
    ItemManager();
    // アイテム追加（ゲーム起動時に定義するなど）
    void AddItem(std::unique_ptr<Item> item);

    // 所持にする（フィールド拾得 or 街購入時に呼ぶ）
    bool UnlockItem(int itemId);

    // 装備を試みる（スロット上限判定済み）
    bool EquipItem(int itemId, PlayerData* player);

    // 装備解除
    bool UnequipItem(int itemId, PlayerData* player);

    // 装備一覧を返す（参照用）
    const std::vector<std::unique_ptr<Item>>& GetAllItems() const;

    // 現在装備中の合計スロット
    int GetUsedSlots() const;

    // プレイヤーのステータスに装備効果を適用する（Equip/Unequip後に呼ぶ）
    void ApplyBuffsToPlayer(PlayerData* player);

    // スロット上限（ベース + 装備ボーナス）を計算して返す
    int GetPlayerMaxSlots(const PlayerData* player) const;
private:
    std::vector<std::unique_ptr<Item>> m_items;
};

extern ItemManager g_ItemManager;

// =====================================
// SaveSystem / EquipMenu 互換関数宣言
// =====================================
bool ItemManager_IsItemOwned(int itemId);
int ItemManager_GetEquippedItemCount(void);
int ItemManager_GetEquippedItemID(int index);
void ItemManager_AddItem(int itemId);
void ItemManager_ClearEquippedItems(void);
void ItemManager_ClearAllItems(void);
void ItemManager_EquipItem(int itemId);
void ItemManager_RemoveItem(int itemId);
