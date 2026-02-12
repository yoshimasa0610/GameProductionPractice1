#pragma once
#include <vector>
#include <string>

enum class ItemType
{
    Passive,    // 所持だけで効果
    Equip,      // 装備して効果
};

//enum class PlayerAttackType;
struct BuffEffect
{
   //ここにバフの内容を記載（後からわかるようコメントでも書いて）
};

class Item
{
public:
    int id;                    // 内部ID
    std::string name;          // 表示名
    ItemType type;
    int slotCost;              // 装備に必要なスロット数
    int ownedCount = 0;
    bool isEquipped = false;   // 装備中フラグ
    BuffEffect buff;           // 効果

    std::string iconSmallPath;  // 小アイコン画像パス (例: "Data/Item/Charm/Icon_Small_01.png")
    std::string iconLargePath;  // 大アイコン画像パス (例: "Data/Item/Charm/Icon_Large_01.png")

    int iconSmallHandle = -1;   // DxLibハンドル（読み込み後に格納）
    int iconLargeHandle = -1;

    // 説明用
    std::vector<std::string> systemDesc;   // システム説明（補助）
    std::vector<std::string> flavorDesc;   // フレーバー説明

    Item(int id,
        const std::string& name,
        ItemType type,
        int slotCost,
        const BuffEffect& buff,
        const std::string& iconSmallPath = "",
        const std::string& iconLargePath = "");
};
