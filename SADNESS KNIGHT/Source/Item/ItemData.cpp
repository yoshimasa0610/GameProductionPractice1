#include "ItemManager.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "Item.h"

// テキストなんかは適当だから変えていいよ

void InitAllItems()
{
    // =========================
    // 1. HPUP
    // =========================
    {
        auto item = std::make_unique<Item>();
        item->id = 1;
        item->name = "HPUP";
        item->type = ItemType::Passive;
        item->slotCost = 0;
        item->iconSmallPath = "Data/Item/Icon/hp_up_small.png";
        item->iconLargePath = "Data/Item/Icon/hp_up_large.png";
        item->buff.addMaxHp = 10;
        // 説明
        item->systemDesc = {
            "最大HPが10増加する",
            "複数所持で効果が重複する"
        };

        item->flavorDesc = {
            "生命力を底上げする基礎強化モジュール。",
            "探索者の生存率を大きく引き上げる。"
        };
        g_ItemManager.AddItem(std::move(item));
    }

    // =========================
    // 2. スロットUP
    // =========================
    {
        auto item = std::make_unique<Item>();

        item->id = 2;
        item->name = "スロットUP";

        item->type = ItemType::Passive;
        item->slotCost = 0;

        item->iconSmallPath = "Data/Item/Icon/slot_up_small.png";
        item->iconLargePath = "Data/Item/Icon/slot_up_large.png";

        item->buff.addMaxSlot = 1;

        item->systemDesc = {
            "装備スロット上限が1増加する",
            "複数所持で効果が重複する"
        };

        item->flavorDesc = {
            "拡張ポートを解放する制御キー。",
            "さらなる力を扱うための余白を生む。"
        };

        g_ItemManager.AddItem(std::move(item));
    }

    // =========================
    // 3. 体力回復UP
    // =========================
    {
        auto item = std::make_unique<Item>();

        item->id = 3;
        item->name = "体力回復UP";
        item->type = ItemType::Passive;
        item->slotCost = 0;
        item->iconSmallPath = "Data/Item/Icon/heal_up_small.png";
        item->iconLargePath = "Data/Item/Icon/heal_up_large.png";
        item->buff.healPowerBonus = 10;
        item->systemDesc = {
            "回復時の回復量が10増加する",
            "複数所持で効果が重複する"
        };

        item->flavorDesc = {
            "再生プロトコルを強化する補助モジュール。",
            "肉体の修復効率を高める。"
        };
        g_ItemManager.AddItem(std::move(item));
    }


}
