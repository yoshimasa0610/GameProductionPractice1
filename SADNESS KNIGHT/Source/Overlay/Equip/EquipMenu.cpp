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
    g_SelectedIndex = 0;
    g_MessageTimer = 0;
    g_Message.clear();
    g_PlayerRef = player;
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
        if (items[i]->isOwned) outIndices.push_back((int)i);
    }
}


void UpdateEquipMenuScene()
{
    if (!g_IsEquipMenuOpen)
        return;

    const auto& items = g_ItemManager.GetAllItems();

    if (items.empty())
    {
        return;
    }
    // 所持アイテムのみのインデックスを構築
    std::vector<int> ownedIndices;
    BuildOwnedIndexList(items, ownedIndices);

    if (ownedIndices.empty()) {
        g_SelectedIndex = 0;
        // 閉じる入力だけは受け付ける
        if (IsTriggerKey(KEY_CANCEL))
        {
            SetPaused(false);
            CloseEquipMenu();
        }
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

    // 閉じる（Esc / KEY_CANCEL）
    if (IsTriggerKey(KEY_CANCEL))
    {
        SetPaused(false);
        CloseEquipMenu();
        return;
    }

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

void DrawEquipMenuScene()
{
    if (!g_IsEquipMenuOpen)
        return;

    const auto& items = g_ItemManager.GetAllItems();

    for (size_t i = 0; i < items.size(); ++i)
    {
        DrawFormatString(
            10, 200 + i * 16,
            GetColor(255, 255, 0),
            "item[%d] id=%d owned=%d equipped=%d",
            (int)i,
            items[i]->id,
            items[i]->isOwned,
            items[i]->isEquipped
        );
    }
    if (items.empty()) return;

    // 所持アイテムインデックス
    std::vector<int> ownedIndices;
    BuildOwnedIndexList(items, ownedIndices);
    DrawFormatString(10, 62, GetColor(0, 200, 255), "ownedIndices=%d sel=%d sitting=%d",
        (int)ownedIndices.size(), g_SelectedIndex, (int)IsPlayerSitting());

    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    int winW = screenW - 120;
    int winH = screenH - 120;
    int winX = (screenW - winW) / 2;
    int winY = (screenH - winH) / 2;

    // ===============================
    // UI 基準 & スケール
    // ===============================
    const float uiScale = 1.5f;
    const int baseX = winX;
    const int baseY = winY;

    //フォント
    static int g_EquipFont = -1;
    if (g_EquipFont < 0)
    {
        g_EquipFont = CreateFontToHandle(
            "ＭＳ ゴシック",
            int(16 * uiScale),   // ← 好きに調整可（20～26あたり）
            3,
            DX_FONTTYPE_ANTIALIASING
        );
    }

    auto SX = [&](int x) { return baseX + int(x * uiScale); };
    auto SY = [&](int y) { return baseY + int(y * uiScale); };
    auto SS = [&](int s) { return int(s * uiScale); };

    // プレイヤーのスロット情報
    int usedSlots = g_ItemManager.GetUsedSlots();
    int maxSlots = g_ItemManager.GetPlayerMaxSlots(g_PlayerRef);

    // ===============================
    // 背景暗転
    // ===============================
    DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // メインウィンドウ
    DrawBox(winX, winY, winX + winW, winY + winH, GetColor(20, 20, 20), TRUE);
    DrawBox(winX, winY, winX + winW, winY + winH, GetColor(160, 160, 160), FALSE);

    // タイトル
    DrawString(SX(40), SY(20), "=== チャーム装備 ===", GetColor(255, 255, 255), g_EquipFont);

    // 左上 装備中アイテムとスロット表示
    DrawString(SX(40), SY(60), "装備中", GetColor(255, 255, 200));

    int eqX = SX(40);
    int eqY = SY(90);
    int equippedCount = 0;
    int iconSize = SS(48);
    int iconStep = SS(52);

    for (const auto& it : items)
    {
        if (!it->isEquipped) continue;

        int x = eqX + (equippedCount % 6) * iconStep;
        int y = eqY + (equippedCount / 6) * iconStep;

        if (it->iconSmallHandle > 0)
            DrawExtendGraph(x, y, x + iconSize, y + iconSize, it->iconSmallHandle, TRUE);
        else
            DrawBox(x, y, x + iconSize, y + iconSize, GetColor(180, 180, 180), TRUE);

        equippedCount++;
    }

    // スロットバー
    char slotBuf[64];
    int sx = SX(40);
    int sy = eqY + SS(70);
    DrawString(sx, sy - SS(20), "スロット", GetColor(180, 220, 255));
    for (int i = 0; i < maxSlots; ++i)
    {
        int cx = sx + i * SS(18);
        if (i < usedSlots)
            DrawBox(cx, sy, cx + SS(12), sy + SS(12), GetColor(255, 200, 50), TRUE);
        else
            DrawBox(cx, sy, cx + SS(12), sy + SS(12), GetColor(100, 100, 100), FALSE);
    }

    // 左下 所持アイテム一覧（画像のみ）
    DrawString(SX(40), SY(200), "所持チャーム", GetColor(255, 255, 200), g_EquipFont);
    int listX = SX(40);
    int listY = SY(230);
    int cols = 8;
    int displayed = 0;
    if (ownedIndices.empty())
    {
        DrawString(
            listX,
            listY,
            "所持しているチャームはありません",
            GetColor(180, 180, 180),
            g_EquipFont
        );
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
                DrawBox(x - SS(2), y - SS(2),
                    x + iconSize + SS(2),
                    y + iconSize + SS(2),
                    GetColor(255, 255, 0), FALSE);

            DrawBox(x, y, x + iconSize, y + iconSize,
                GetColor(180, 180, 180), TRUE);

            if (it->isEquipped)
                DrawBox(x, y, x + iconSize, y + iconSize,
                    GetColor(0, 255, 0), FALSE);
        }
    }

    // ===============================
    // 右側 詳細ウィンドウ
    // ===============================
    int dx = winX + winW - SS(380);
    int dy = SY(70);

    DrawBox(dx - SS(10), dy - SS(10), dx + SS(340), dy + SS(420), GetColor(30, 30, 30), TRUE);
    DrawBox(dx - SS(10), dy - SS(10), dx + SS(340), dy + SS(420), GetColor(120, 120, 120), FALSE);

    // 選択中アイテム参照
    if (!ownedIndices.empty())
    {
        if (g_SelectedIndex < 0) g_SelectedIndex = 0;
        if ((size_t)g_SelectedIndex >= ownedIndices.size())
            g_SelectedIndex = (int)ownedIndices.size() - 1;

        const auto& sel = items[ownedIndices[g_SelectedIndex]];
        DrawString(dx, dy, sel->name.c_str(), GetColor(255, 255, 255));

        // 大アイコン
        if (sel->iconLargeHandle > 0)
            DrawExtendGraph(
                dx + SS(20), dy + SS(30),
                dx + SS(140), dy + SS(150),
                sel->iconLargeHandle, TRUE
            );

        // 必要スロット
        char buf[64];
        sprintf_s(buf, "消費スロット: %d", sel->slotCost);
        DrawString(dx, dy + SS(170), buf, GetColor(180, 220, 255));

        // ===============================
     // 効果説明（systemDesc）
     // ===============================
        int lineY = dy + SS(200);

        DrawString(dx, lineY, "効果", GetColor(200, 220, 255));
        lineY += SS(22);

        for (const auto& line : sel->systemDesc)
        {
            DrawString(dx + SS(10), lineY, line.c_str(), GetColor(220, 220, 220));
            lineY += SS(20);
        }

        // ===============================
        // フレーバー説明（flavorDesc）
        // ===============================
        if (!sel->flavorDesc.empty())
        {
            lineY += SS(10);
            DrawString(dx, lineY, "説明", GetColor(200, 220, 255));
            lineY += SS(22);

            for (const auto& line : sel->flavorDesc)
            {
                DrawString(dx + SS(10), lineY, line.c_str(), GetColor(180, 180, 180));
                lineY += SS(20);
            }
        }
    }

    // 下部 ガイドと決定ボタン（座っている時のみ表示）
    int guideY = winY + winH - SS(60);
    if (IsPlayerSitting())
        DrawString(SX(40), guideY, "[↑↓←→]選択  [SPACE]装備/解除  [Esc]閉じる", GetColor(255, 255, 255));
    else
    {
        DrawString(SX(40), guideY, "[Esc]閉じる", GetColor(200, 200, 200));
        DrawString(SX(400), guideY, "装備変更は休憩中のみ可能です。", GetColor(200, 200, 120));
    }

    // メッセージ表示
    if (g_MessageTimer > 0)
    {
        DrawStringToHandle(
            SX(40), SY(480),
            g_Message.c_str(),
            GetColor(255, 200, 100),
            g_EquipFont
        );
    }
}

//void FinEquipMenuScene()
//{
//    // Save current state
//    ExportSaveData(&g_SaveData);
//    SaveGame(&g_SaveData, 0);
//}