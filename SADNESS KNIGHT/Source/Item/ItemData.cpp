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

    // ここからは装備することで効果を発動できる装備アイテムの内容です

    // =========================
    // 10. ダメージ軽減
    // =========================
    {
        auto item = std::make_unique<Item>();

        item->id = 10;
        item->name = "守りのチャーム";
        item->type = ItemType::Equip;
        item->slotCost = 2;

        item->iconSmallPath = "Data/Item/Icon/charm_guard_small.png";
        item->iconLargePath = "Data/Item/Icon/charm_guard_large.png";

        item->buff.damageTakenRate = -0.10f; // 10%軽減

        item->systemDesc = {
            "受けるダメージが15%減少する",
            "他の軽減効果と重複する"
        };

        item->flavorDesc = {
            "古い守護の刻印。",
            "装着者を静かに守り続ける。"
        };

        g_ItemManager.AddItem(std::move(item));
    }


    {
        auto item = std::make_unique<Item>();

        item->id = 11;
        item->name = "魔力のチャーム";
        item->type = ItemType::Equip;
        item->slotCost = 2;

        item->buff.skillCountRate = 0.15f;

        item->systemDesc = {
            "スキル使用回数が15%増加する"
        };

        g_ItemManager.AddItem(std::move(item));
    }
}
