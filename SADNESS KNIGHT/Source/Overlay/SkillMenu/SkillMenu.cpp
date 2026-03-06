#include "SkillMenu.h"
#include "../../Skill/SkillManager.h"
#include "../../Input/Input.h"
#include "../../Player/Player.h"
#include "DxLib.h"
#include "../../Map/Checkpoint/CheckpointManager.h"
#include "../OverlayMenu.h"
#include <algorithm>

extern SkillManager g_SkillManager;

enum class SkillMenuMode
{
    SlotSelect,
    SkillSelect
};
bool g_IsSkillMenuOpen = false;
static SkillMenuMode g_Mode = SkillMenuMode::SlotSelect;
static PlayerData* g_Player = nullptr;

static int g_SelectedSlot = 0;
static int g_SelectedSkillIndex = 0;
static int g_EditSet = 0;
// スキルメニューを開く
void OpenSkillMenu(PlayerData* player)
{
    g_IsSkillMenuOpen = true;
    g_Player = player;
    g_Mode = SkillMenuMode::SlotSelect;
    g_SelectedSlot = 0;
    g_SelectedSkillIndex = 0;
    g_EditSet = g_SkillManager.GetCurrentSet();
}
// スキルメニュー閉じる
void CloseSkillMenu()
{
    g_IsSkillMenuOpen = false;
}

// スキルIDのリストを作る
static void BuildOwnedSkills(std::vector<int>& out)
{
    out.clear();
    for (auto& s : g_SkillManager.GetSkills())
        out.push_back(s->GetID());
    // ID順ソート
    std::sort(out.begin(), out.end());
}
// スキルメニューの更新
void UpdateSkillMenuScene()
{
    //if (!g_IsSkillMenuOpen) return;
    /*
    // 閉じる
    if (IsTriggerKey(KEY_CANCEL))
    {
        CloseSkillMenu();
        return;
    }
    */
    //if (!IsPlayerSitting()) return;

    if (g_Mode == SkillMenuMode::SlotSelect)
    {
        if (IsTriggerKey(KEY_LEFT))  g_SelectedSlot--;
        if (IsTriggerKey(KEY_RIGHT)) g_SelectedSlot++;

        if (g_SelectedSlot < 0) g_SelectedSlot = 0;
        if (g_SelectedSlot > 5) g_SelectedSlot = 5;

        // set と slot を計算
        g_EditSet = g_SelectedSlot / 3;
        int slot = g_SelectedSlot % 3;

        if (IsTriggerKey(KEY_OK))
        {
            g_Mode = SkillMenuMode::SkillSelect;
            g_SelectedSkillIndex = 0;
        }

        // 解除
        if (IsTriggerKey(KEY_CANCEL))
        {
            int slot = g_SelectedSlot % 3;

            g_SkillManager.UnequipSkill(
                g_EditSet,
                slot
            );
        }
    }
    else
    {
        std::vector<int> list;
        BuildOwnedSkills(list);

        if (list.empty())
        {
            g_SelectedSkillIndex = 0;
            //DrawString(SX(40), SY(140), "No Skills", GetColor(200, 200, 200));
        }

        if (IsTriggerKey(KEY_UP))    g_SelectedSkillIndex -= 6;
        if (IsTriggerKey(KEY_DOWN))  g_SelectedSkillIndex += 6;
        if (IsTriggerKey(KEY_LEFT))  g_SelectedSkillIndex--;
        if (IsTriggerKey(KEY_RIGHT)) g_SelectedSkillIndex++;

        if (g_SelectedSkillIndex < 0) g_SelectedSkillIndex = 0;
        if (g_SelectedSkillIndex >= list.size())
            g_SelectedSkillIndex = list.size() - 1;

        if (IsTriggerKey(KEY_OK))
        {
            int slot = g_SelectedSlot % 3;

            g_SkillManager.EquipSkill(
                g_EditSet,
                slot,
                list[g_SelectedSkillIndex]
            );
            g_Mode = SkillMenuMode::SlotSelect;
        }

        if (IsTriggerKey(KEY_CANCEL))
        {
            g_Mode = SkillMenuMode::SlotSelect;
        }
    }
}

void DrawSkillMenuScene(const OverlayArea& area)
{
    if (!g_IsSkillMenuOpen) return;

    // ===============================
    // UI基準
    // ===============================
    const float uiScale = 1.2f;

    int baseX = area.x;
    int baseY = area.y;

    auto SX = [&](int x) { return baseX + int(x * uiScale); };
    auto SY = [&](int y) { return baseY + int(y * uiScale); };
    auto SS = [&](int s) { return int(s * uiScale); };

    // ===============================
    // セット表示（上）
    // ===============================

    DrawString(
        SX(30),
        SY(0),
        "セット中のスキル",
        GetColor(255, 255, 200));

    for (int s = 0; s < 2; s++)
    {
        int setX = SX(10 + s * 220);
        int setY = SY(35);

        DrawString(
            setX,
            setY,
            "Set",
            GetColor(200, 200, 200));

        DrawFormatString(
            setX + SS(8),
            setY + SS(18),
            GetColor(255, 255, 255),
            "%d",
            s + 1);

        for (int i = 0; i < 3; i++)
        {
            int x = SX(60 + s * 220 + i * 60);
            int y = SY(50);

            int skillID = g_SkillManager.GetEquipSkill(s, i);

            if (skillID == -1)
            {
                DrawCircle(x, y, SS(20), GetColor(80, 80, 80), TRUE);
            }
            else
            {
                const SkillData& data = GetSkillData(skillID);

                DrawGraph(
                    x - SS(20),
                    y - SS(20),
                    data.iconSmallHandle,
                    TRUE);
            }
            int index = s * 3 + i;

            if (g_Mode == SkillMenuMode::SlotSelect
                && index == g_SelectedSlot)
            {
                DrawCircle(x, y, SS(24), GetColor(255, 255, 0), FALSE);
            }
        }
    }

    // ===============================
    // 区切り線
    // ===============================

    int separatorY = SY(100);

    int lineWidth = SS(200);
    int centerX = SX(200);

    DrawLine(
        centerX - lineWidth / 2,
        separatorY,
        centerX + lineWidth / 2,
        separatorY,
        GetColor(160, 160, 160)
    );

    DrawString(
        SX(40),
        SY(120),
        "取得スキル",
        GetColor(255, 255, 200));

    // ===============================
    // スキル一覧
    // ===============================
    std::vector<int> list;
    BuildOwnedSkills(list);

    int cols = 7;
    int iconSize = SS(48);
    int step = SS(56);

    int listX = SX(40);
    int listY = SY(140);

    for (int i = 0; i < (int)list.size(); i++)
    {
        int col = i % cols;
        int row = i / cols;

        int x = listX + col * step;
        int y = listY + row * step;

        // 選択枠
        if (g_Mode == SkillMenuMode::SkillSelect &&
            i == g_SelectedSkillIndex)
        {
            DrawBox(
                x - 2,
                y - 2,
                x + iconSize + 2,
                y + iconSize + 2,
                GetColor(255, 255, 0),
                FALSE);
        }

        // 背景
        DrawBox(
            x,
            y,
            x + iconSize,
            y + iconSize,
            GetColor(90, 90, 90),
            TRUE);

        const SkillData& data = GetSkillData(list[i]);

        if (data.iconSmallHandle > 0)
        {
            DrawExtendGraph(
                x,
                y,
                x + iconSize,
                y + iconSize,
                data.iconSmallHandle,
                TRUE);
        }
        
        // 装備マーク
        if (g_SkillManager.IsSkillEquipped(list[i]))
        {
            DrawString(
                x + 4,
                y + 2,
                "E",
                GetColor(0, 255, 0));
        }
        
    }

    // ===============================
    // 右詳細ウィンドウ
    // ===============================
    int dx = area.x + area.w - SS(320);
    int dy = area.y + SS(40);

    DrawBox(
        dx,
        dy,
        dx + SS(300),
        dy + SS(420),
        GetColor(60, 60, 60),
        TRUE);

    DrawBox(
        dx,
        dy,
        dx + SS(300),
        dy + SS(420),
        GetColor(120, 120, 120),
        FALSE);

    // ===============================
    // 選択スキル取得
    // ===============================

    int selectedSkillID = -1;

    if (g_Mode == SkillMenuMode::SkillSelect)
    {
        if (!list.empty())
            selectedSkillID = list[g_SelectedSkillIndex];
    }
    else
    {
        int slot = g_SelectedSlot % 3;

        selectedSkillID =
            g_SkillManager.GetEquipSkill(
                g_EditSet,
                slot);
    }

    if (selectedSkillID == -1) return;

    const SkillData& data = GetSkillData(selectedSkillID);

    // ===============================
    // スキル名
    // ===============================

    DrawString(
        dx + SS(20),
        dy + SS(20),
        data.name.c_str(),
        GetColor(255, 255, 255));

    // ===============================
    // アイコン
    // ===============================

    if (data.iconLargeHandle != -1)
    {
        DrawGraph(
            dx + SS(100),
            dy + SS(60),
            data.iconLargeHandle,
            TRUE);
    }

    // ===============================
    // システム説明
    // ===============================

    int textY = dy + SS(200);

    for (auto& line : data.systemDesc)
    {
        DrawString(
            dx + SS(20),
            textY,
            line.c_str(),
            GetColor(255, 255, 255));

        textY += SS(20);
    }

    textY += SS(10);

    // ===============================
    // フレーバー説明
    // ===============================

    for (auto& line : data.flavorDesc)
    {
        DrawString(
            dx + SS(20),
            textY,
            line.c_str(),
            GetColor(180, 180, 180));

        textY += SS(20);
    }
}