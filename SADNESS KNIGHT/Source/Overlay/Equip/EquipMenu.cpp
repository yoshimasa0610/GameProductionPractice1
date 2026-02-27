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
            // 直前までを描画
            line.pop_back();
            DrawStringToHandle(x, drawY, line.c_str(), color, fontHandle);

            drawY += lineHeight;
            line.clear();

            // はみ出した文字を次行に
            line.push_back(text[i]);
        }
    }

    // 残りを描画
    if (!line.empty())
    {
        DrawStringToHandle(x, drawY, line.c_str(), color, fontHandle);
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
        if (items[i]->ownedCount > 0)
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
    if (IsTriggerKey(KEY_UP))    g_SelectedIndex--;
    if (IsTriggerKey(KEY_DOWN))  g_SelectedIndex++;
    if (IsTriggerKey(KEY_LEFT))  g_SelectedIndex -= 6; // ページ移動（行数に合わせて調整）
    if (IsTriggerKey(KEY_RIGHT)) g_SelectedIndex += 6;

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

    // ===== 基準 =====
    const float uiScale = 1.2f;

    const int baseX = area.x;
    const int baseY = area.y;

    auto SX = [&](int x) { return baseX + int(x * uiScale); };
    auto SY = [&](int y) { return baseY + int(y * uiScale); };
    auto SS = [&](int s) { return int(s * uiScale); };

    // フォント
    static int g_EquipFont = -1;
    if (g_EquipFont < 0)
    {
        g_EquipFont = CreateFontToHandle(
            "ＭＳ ゴシック",
            int(16 * uiScale),
            3,
            DX_FONTTYPE_ANTIALIASING
        );
    }

    // =========================
    // 左側：装備中
    // =========================

    DrawString(SX(20), SY(20), "チャーム装備", GetColor(255, 255, 255), g_EquipFont);

    int eqX = SX(20);
    int eqY = SY(60);
    int iconSize = SS(48);
    int iconStep = SS(52);

    int equippedCount = 0;

    for (const auto& it : items)
    {
        if (!it->isEquipped) continue;

        int x = eqX + (equippedCount % 6) * iconStep;
        int y = eqY + (equippedCount / 6) * iconStep;

        if (it->iconSmallHandle > 0)
            DrawExtendGraph(x, y, x + iconSize, y + iconSize, it->iconSmallHandle, TRUE);
        else
            DrawBox(x, y, x + iconSize, y + iconSize, GetColor(150, 150, 150), TRUE);

        equippedCount++;
    }

    // =========================
    // 所持リスト
    // =========================

    std::vector<int> ownedIndices;
    BuildOwnedIndexList(items, ownedIndices);

    int listX = SX(20);
    int listY = SY(200);

    DrawString(listX, SY(170), "所持チャーム", GetColor(255, 255, 200), g_EquipFont);

    int cols = 6;

    if (ownedIndices.empty())
    {
        DrawString(listX, listY,
            "所持しているチャームはありません",
            GetColor(180, 180, 180),
            g_EquipFont);
    }
    else
    {
        for (int i = 0; i < (int)ownedIndices.size(); ++i)
        {
            const auto& it = items[ownedIndices[i]];

            int col = i % cols;
            int row = i / cols;

            int x = listX + col * iconStep;
            int y = listY + row * iconStep;

            if (i == g_SelectedIndex)
                DrawBox(x - 2, y - 2, x + iconSize + 2, y + iconSize + 2,
                    GetColor(255, 255, 0), FALSE);

            DrawBox(x, y, x + iconSize, y + iconSize,
                GetColor(120, 120, 120), TRUE);

            if (it->isEquipped)
                DrawBox(x, y, x + iconSize, y + iconSize,
                    GetColor(0, 255, 0), FALSE);
        }
    }

    // =========================
    // 右側：詳細
    // =========================

    int detailX = area.x + area.w - SS(320);
    int detailY = area.y + SS(40);

    DrawBox(detailX - 10, detailY - 10,
        detailX + SS(300), detailY + SS(400),
        GetColor(50, 50, 50), TRUE);

    DrawBox(detailX - 10, detailY - 10,
        detailX + SS(300), detailY + SS(400),
        GetColor(120, 120, 120), FALSE);

    if (!ownedIndices.empty())
    {
        const auto& sel = items[ownedIndices[g_SelectedIndex]];

        DrawString(detailX, detailY,
            sel->name.c_str(),
            GetColor(255, 255, 255),
            g_EquipFont);
    }
}

//void FinEquipMenuScene()
//{
//    // Save current state
//    ExportSaveData(&g_SaveData);
//    SaveGame(&g_SaveData, 0);
//}