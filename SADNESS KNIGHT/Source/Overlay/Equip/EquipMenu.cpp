#include "EquipMenu.h"
#include "../../SaveSystem/SaveSystem.h"
#include "../../Input/Input.h"
#include "../../Item/ItemManager.h"
#include "../../Player/Player.h"
#include "../../Map/Checkpoint/CheckpointManager.h"
#include "DxLib.h"
#include <string>
#include <vector>
#include <algorithm>
#include "../../Sound/Sound.h"
#include "../../Scene/Play/Play.h"
#include "../OverlayMenu.h"

// 内部変数
extern  SaveData g_SaveData;
static int g_SelectedIndex = 0; // 所持アイテム（表示リスト）でのインデックス
static bool g_IsEquipMode = false; // 装備変更が許可されているか（SetEquipMode で外部から設定）
static PlayerData* g_PlayerRef = nullptr; // プレイヤーデータ参照（SetEquipMenuPlayer でセット可能）
static int g_MessageTimer = 0;
static std::string g_Message;
bool g_IsEquipMenuOpen = false;
static EquipUIMode g_UIMode = EquipUIMode::ItemSelect;

// 装備説明などの説明文の折り返し用ヘルパー関数
static int DrawWrappedString(
    int x,
    int y,
    int maxWidth,
    const std::string& text,
    unsigned int color,
    int lineHeight,
    int fontHandle = -1
)
{
    std::string line;
    int drawY = y;

    for (size_t i = 0; i < text.size(); ++i)
    {
        line.push_back(text[i]);

        int width = GetDrawStringWidth(
            line.c_str(),
            (int)line.size(),
            fontHandle
        );

        if (width > maxWidth)
        {
            line.pop_back();
            DrawString(x, drawY, line.c_str(), color);

            drawY += lineHeight;
            line.clear();

            line.push_back(text[i]);
        }
    }

    // 残りを描画
    if (!line.empty())
    {
        DrawString(x, drawY, line.c_str(), color);
        drawY += lineHeight;
    }

    return drawY; // 次に描画すべきY座標を返す
}

// 外部API
void SetEquipMode(bool enable)
{
    g_IsEquipMode = enable;
}
void SetEquipMenuPlayer(PlayerData* player)
{
    g_PlayerRef = player;
}

void OpenEquipMenu(PlayerData* player)
{
    g_IsEquipMenuOpen = true;
    g_PlayerRef = player;
    g_SelectedIndex = 0;
    g_MessageTimer = 0;
    g_Message.clear();

    SetEquipMode(true);
    g_UIMode = EquipUIMode::ItemSelect;
}

void CloseEquipMenu()
{
    g_IsEquipMenuOpen = false;
}

void LoadEquipMenuScene()
{
    // リソース読み込み等があればここに
}

void StartEquipMenuScene()
{
    // シーン開始時に何かすべきことがあれば
}

void StepEquipMenuScene()
{
    // SceneManager の Step 呼び出し構成があるならここに毎フレームの処理。今回は UpdateとDraw が分かれているので空にする。
}

// helper 所持アイテムのインデックスリストを取得
static void BuildOwnedIndexList(const std::vector<std::unique_ptr<Item>>& items, std::vector<int>& outIndices)
{
    outIndices.clear();
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i]->ownedCount > 0 && items[i]->type == ItemType::Equip)
            outIndices.push_back((int)i);
    }
}


void UpdateEquipMenuScene()
{
    const auto& items = g_ItemManager.GetAllItems();
    /*
    if (items.empty())
    {
        return;
    }*/
    // 所持アイテムのみのインデックスを構築
    std::vector<int> ownedIndices;
    BuildOwnedIndexList(items, ownedIndices);

    if (ownedIndices.empty()) {
        g_SelectedIndex = 0;
        return;
        // 閉じるか表示だけにするかは実装次第。ここでは何もしない。
    }

    // 入力処理（表示リスト上で移動）
    if (g_UIMode == EquipUIMode::ItemSelect)
    {
        if (IsTriggerKey(KEY_UP))    g_SelectedIndex -= 7;
        if (IsTriggerKey(KEY_DOWN))  g_SelectedIndex += 7;
        if (IsTriggerKey(KEY_LEFT))  g_SelectedIndex--;
        if (IsTriggerKey(KEY_RIGHT)) g_SelectedIndex++;
    }
    else if (g_UIMode == EquipUIMode::SlotView)
    {
        // 将来：スロット選択用カーソル処理
    }

    // 範囲クランプ
    if (g_SelectedIndex < 0) g_SelectedIndex = 0;
    if ((size_t)g_SelectedIndex >= ownedIndices.size()) g_SelectedIndex = (int)ownedIndices.size() - 1;
    /*
    // 閉じる（Esc / KEY_CANCEL）
    if (IsTriggerKey(KEY_CANCEL))
    {
        CloseOverlayMenu();
        return;
    }*/

    // メッセージタイマー
    if (g_MessageTimer > 0) g_MessageTimer--;

    // 決定（装備 と 解除）  ただし椅子に座っている場合のみ有効
    if (IsTriggerKey(KEY_OK))
    {
        // 椅子に座っているかどうかをチェック
        bool sitting = IsPlayerSitting();

        const auto& selectedItem = items[ownedIndices[g_SelectedIndex]];

        if (!sitting)
        {
            // 装備変更不可時 メッセージを表示（ボタンは非表示にしているが念のため）
            g_Message = "装備変更は休憩中のみ可能です。";
            g_MessageTimer = 90;
            // ここで SE を鳴らしたい場合は PlaySoundMem 等を呼ぶ
            // PlaySoundMem(s_se_invalid, DX_PLAYTYPE_BACK);
            return;
        }

        // sitting == true の場合のみ装備もしくは解除を試みる
        if (!selectedItem->isEquipped)
        {
            // --- 通常の装備処理 ---
            bool ok = g_ItemManager.EquipItem(selectedItem->id, g_PlayerRef);
            if (ok)
            {
                g_Message = "装備しました。";
                g_MessageTimer = 60;
                PlaySE(SE_MENU_EQUIP);
                ExportSaveData(&g_SaveData);
                SaveGame(&g_SaveData, 0);
            }
            else
            {
                g_Message = "スロットが足りません。";
                g_MessageTimer = 90;
            }
        }
        else
        {
            g_ItemManager.UnequipItem(selectedItem->id, g_PlayerRef);
            g_Message = "外しました。";
            g_MessageTimer = 60;
            PlaySE(SE_MENU_REMOVE);
            ExportSaveData(&g_SaveData);
            SaveGame(&g_SaveData, 0);
        }
    }
    DrawFormatString(10, 80, GetColor(255, 0, 0),
        "sel=%d ownedSize=%d",
        g_SelectedIndex,
        (int)ownedIndices.size());

    for (int i = 0; i < (int)ownedIndices.size(); ++i)
    {
        DrawFormatString(10, 100 + i * 16, GetColor(255, 255, 255),
            "ownedIndices[%d]=%d itemID=%d",
            i,
            ownedIndices[i],
            items[ownedIndices[i]]->id);
    }
}

void DrawEquipMenuScene(const OverlayArea& area)
{
    const auto& items = g_ItemManager.GetAllItems();

    // =========================
    // レイアウト分割
    // =========================

    int leftW = int(area.w * 0.65f);
    int rightW = area.w - leftW;

    int leftX = area.x;
    int rightX = area.x + leftW;

    int headerH = 60;
    int equippedH = 140;

    // =========================
    // ヘッダー
    // =========================

    int maxSlot = g_PlayerRef ? g_ItemManager.GetPlayerMaxSlots(g_PlayerRef) : 0;
    int usedSlot = g_ItemManager.GetUsedSlots();

    DrawString(leftX + 20, area.y + 20,
        "装備中のレリック",
        GetColor(255, 255, 255));

    DrawFormatString(leftX + 260, area.y + 20,
        GetColor(255, 255, 0),
        "%d / %d",
        usedSlot,
        maxSlot);

    // =========================
    // 装備中グリッド（縦2 横7）
    // =========================

    int gridX = leftX + 20;
    int gridY = area.y + headerH;

    int cols = 7;
    int rows = 2;
    int iconSize = 48;
    int step = 56;

    int equippedIndex = 0;

    for (const auto& it : items)
    {
        if (!it->isEquipped) continue;

        int col = equippedIndex % cols;
        int row = equippedIndex / cols;

        int x = gridX + col * step;
        int y = gridY + row * step;

        if (it->iconSmallHandle > 0)
            DrawExtendGraph(x, y, x + iconSize, y + iconSize, it->iconSmallHandle, TRUE);
        else
            DrawBox(x, y, x + iconSize, y + iconSize, GetColor(120, 120, 120), TRUE);

        equippedIndex++;
    }

    // =========================
    // 所持リスト
    // =========================

    std::vector<int> ownedIndices;
    BuildOwnedIndexList(items, ownedIndices);

    // ===== 区切り線 =====
    int separatorY = gridY + equippedH - 50;

    DrawLine(
        leftX + 20,
        separatorY,
        leftX + leftW - 300,//横に長く線を引きたいなら300の値を減らしてください
        separatorY,
        GetColor(160, 160, 160)
    );

    int listY = gridY + equippedH;

    DrawString(leftX + 20, listY - 30,
        "所持しているレリック",
        GetColor(255, 255, 200));

    for (int i = 0; i < (int)ownedIndices.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;

        int x = gridX + col * step;
        int y = listY + row * step;

        if (i == g_SelectedIndex)
            DrawBox(x - 2, y - 2, x + iconSize + 2, y + iconSize + 2,
                GetColor(255, 255, 0), FALSE);

        DrawBox(x, y, x + iconSize, y + iconSize,
            GetColor(90, 90, 90), TRUE);

        const auto& item = items[ownedIndices[i]];

        if (item->iconSmallHandle > 0)
        {
            DrawExtendGraph(
                x,
                y,
                x + iconSize,
                y + iconSize,
                item->iconSmallHandle,
                TRUE
            );
        }

        if (item->isEquipped)
        {
            DrawString(
                x + 4,
                y + 2,
                "E",
                GetColor(0, 255, 0)
            );
        }
        //or
        /*
        if (item->isEquipped)
        {
            DrawBox(x + 28, y, x + 48, y + 18, GetColor(0, 150, 0), TRUE);
            DrawString(x + 32, y + 2, "E", GetColor(255, 255, 255));
        }
        */
    }

    // =========================
    // 右：詳細パネル
    // =========================

    DrawBox(rightX + 20, area.y + 20,
        rightX + rightW - 20,
        area.y + area.h - 20,
        GetColor(50, 60, 80), TRUE);

    DrawBox(rightX + 20, area.y + 20,
        rightX + rightW - 20,
        area.y + area.h - 20,
        GetColor(150, 150, 180), FALSE);

    if (!ownedIndices.empty())
    {
        const auto& sel = items[ownedIndices[g_SelectedIndex]];

        DrawString(rightX + 40, area.y + 40,
            sel->name.c_str(),
            GetColor(255, 255, 255));

        DrawFormatString(
            rightX + 40,
            area.y + 90,
            GetColor(255, 200, 0),
            "使用スロット\n     %d",
            sel->slotCost
        );

        // ===== 大型アイコン表示 =====
        int iconSize = 96;
        int iconX = rightX + 165;
        int iconY = area.y + 70;

        if (sel->iconLargeHandle > 0)
        {
            DrawExtendGraph(
                iconX,
                iconY,
                iconX + iconSize,
                iconY + iconSize,
                sel->iconLargeHandle,
                TRUE
            );
        }
        else
        {
            DrawBox(
                iconX,
                iconY,
                iconX + iconSize,
                iconY + iconSize,
                GetColor(120, 120, 120),
                TRUE
            );
        }

        // テキスト開始位置を下にずらす
        int textY = iconY + iconSize + 20;

        // --- システム説明 ---
        for (const auto& line : sel->systemDesc)
        {
            textY = DrawWrappedString(
                rightX + 40,
                textY,
                rightW - 80,
                line,
                GetColor(220, 220, 220),
                22
            );
            textY += 4;
        }

        textY += 10;

        // --- フレーバー ---
        for (const auto& line : sel->flavorDesc)
        {
            textY = DrawWrappedString(
                rightX + 40,
                textY,
                rightW - 80,
                line,
                GetColor(180, 180, 180),
                22
            );
            textY += 4;
        }
    }
}
//void FinEquipMenuScene()
//{
//    // Save current state
//    ExportSaveData(&g_SaveData);
//    SaveGame(&g_SaveData, 0);
//}