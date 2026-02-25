#include "SkillMenu.h"
#include "../../Skill/SkillManager.h"
#include "../../Input/Input.h"
#include "../../Player/Player.h"
#include "DxLib.h"
#include "../../Map/Checkpoint/CheckpointManager.h"

extern SkillManager g_SkillManager;

enum class SkillMenuMode
{
    SlotSelect,
    SkillSelect
};

static SkillMenuMode g_Mode = SkillMenuMode::SlotSelect;
static PlayerData* g_Player = nullptr;

static int g_SelectedSlot = 0;
static int g_SelectedSkillIndex = 0;
static int g_EditSet = 0;

void OpenSkillMenu(PlayerData* player)
{
    g_Player = player;
    g_Mode = SkillMenuMode::SlotSelect;
    g_SelectedSlot = 0;
    g_SelectedSkillIndex = 0;
    g_EditSet = g_SkillManager.GetCurrentSet();
}

static void BuildOwnedSkills(std::vector<int>& out)
{
    out.clear();
    for (auto& s : g_SkillManager.GetSkills())
        out.push_back(s->GetID());
}

void UpdateSkillMenuScene()
{
    if (!IsPlayerSitting()) return;

    if (g_Mode == SkillMenuMode::SlotSelect)
    {
        if (IsTriggerKey(KEY_LEFT)) g_SelectedSlot--;
        if (IsTriggerKey(KEY_RIGHT)) g_SelectedSlot++;
        if (IsTriggerKey(KEY_UP)) g_EditSet = 0;
        if (IsTriggerKey(KEY_DOWN)) g_EditSet = 1;

        if (g_SelectedSlot < 0) g_SelectedSlot = 0;
        if (g_SelectedSlot > 2) g_SelectedSlot = 2;

        if (IsTriggerKey(KEY_OK))
        {
            g_Mode = SkillMenuMode::SkillSelect;
            g_SelectedSkillIndex = 0;
        }

        // 解除
        if (IsTriggerKey(KEY_CANCEL))
        {
            g_SkillManager.UnequipSkill(g_EditSet, g_SelectedSlot);
        }
    }
    else
    {
        std::vector<int> list;
        BuildOwnedSkills(list);

        if (list.empty()) return;

        if (IsTriggerKey(KEY_UP)) g_SelectedSkillIndex--;
        if (IsTriggerKey(KEY_DOWN)) g_SelectedSkillIndex++;

        if (g_SelectedSkillIndex < 0) g_SelectedSkillIndex = 0;
        if (g_SelectedSkillIndex >= list.size())
            g_SelectedSkillIndex = list.size() - 1;

        if (IsTriggerKey(KEY_OK))
        {
            g_SkillManager.EquipSkill(
                g_EditSet,
                g_SelectedSlot,
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

void DrawSkillMenuScene()
{
    int w, h;
    GetScreenState(&w, &h, nullptr);

    DrawBox(0, 0, w, h, GetColor(0, 0, 0), TRUE);

    int baseX = 80;
    int baseY = 80;

    // ===== セット表示 =====
    for (int s = 0; s < 2; s++)
    {
        for (int i = 0; i < 3; i++)
        {
            int id = g_SkillManager.GetEquipSkill(s, i);

            int x = baseX + s * 220 + i * 60;
            int y = baseY;

            DrawCircle(x, y, 20, GetColor(80, 120, 200), TRUE);

            if (g_Mode == SkillMenuMode::SlotSelect
                && s == g_EditSet
                && i == g_SelectedSlot)
            {
                DrawCircle(x, y, 24, GetColor(255, 255, 0), FALSE);
            }
        }
    }

    // ===== スキル一覧 =====
    std::vector<int> list;
    BuildOwnedSkills(list);

    int lx = baseX;
    int ly = baseY + 120;

    for (int i = 0; i < list.size(); i++)
    {
        int x = lx + (i % 6) * 60;
        int y = ly + (i / 6) * 60;

        DrawBox(x, y, x + 48, y + 48, GetColor(80, 120, 200), TRUE);

        if (g_Mode == SkillMenuMode::SkillSelect
            && i == g_SelectedSkillIndex)
        {
            DrawBox(x - 2, y - 2, x + 50, y + 50, GetColor(255, 255, 0), FALSE);
        }
    }

    // ===== 右詳細 =====
    DrawBox(w - 360, 80, w - 80, 520, GetColor(60, 60, 60), TRUE);
}