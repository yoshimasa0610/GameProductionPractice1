#include "OverlayMenu.h"
#include "../Overlay/Equip/EquipMenu.h"
#include "../Input/Input.h"
#include "../Overlay/SkillMenu/SkillMenu.h"
#include "../Scene/Play/Play.h"

static OverlayTab g_CurrentTab = OverlayTab::Equip;
static PlayerData* g_PlayerRef = nullptr;
static bool g_IsOverlayOpen = false;

void OpenOverlayMenu(PlayerData* player)
{
    g_IsOverlayOpen = true;
    g_PlayerRef = player;
    g_CurrentTab = OverlayTab::Equip;
    SetPaused(true);
    // Equipタブ開始
    SetEquipMenuPlayer(player);
}

void CloseOverlayMenu()
{
    g_IsOverlayOpen = false;
    SetPaused(false);
}

static void OnTabChanged()
{
    switch (g_CurrentTab)
    {
    case OverlayTab::Equip:
        OpenEquipMenu(g_PlayerRef);
        break;

    case OverlayTab::Skill:
        OpenSkillMenu(g_PlayerRef);
        break;
    }
}

// 次のタブに移動
void NextOverlayTab()
{
    int index = (int)g_CurrentTab;
    index++;
    if (index >= (int)OverlayTab::Count)
        index = 0;

    g_CurrentTab = (OverlayTab)index;

    OnTabChanged();
}

void PrevOverlayTab()
{
    int index = (int)g_CurrentTab;
    index--;
    if (index < 0)
        index = (int)OverlayTab::Count - 1;

    g_CurrentTab = (OverlayTab)index;

    OnTabChanged();
}
// オーバーレイメニューの更新
void UpdateOverlayMenu()
{
    if (!g_IsOverlayOpen) return;

        if (IsTriggerKey(KEY_CANCEL))
    {
        CloseOverlayMenu();
        return;
    }

    // LR入力（仮）
    if (IsTriggerKey(KEY_LEFT))
        PrevOverlayTab();

    if (IsTriggerKey(KEY_RIGHT))
        NextOverlayTab();

    switch (g_CurrentTab)
    {
    case OverlayTab::Equip:
        UpdateEquipMenuScene();
        break;

    case OverlayTab::Skill:
        UpdateSkillMenuScene();
        break;
    }
}

static void DrawOverlayTabs()
{
    int x = 100;
    int y = 40;

    const char* names[] =
    {
        "Equip",
        "Skill"
    };

    for (int i = 0; i < (int)OverlayTab::Count; i++)
    {
        int color =
            (i == (int)g_CurrentTab)
            ? GetColor(255, 255, 0)
            : GetColor(180, 180, 180);

        DrawString(x + i * 140, y, names[i], color);
    }
}

void DrawOverlayMenu()
{
    if (!g_IsOverlayOpen) return;

    DrawOverlayTabs();

    switch (g_CurrentTab)
    {
    case OverlayTab::Equip:
        DrawEquipMenuScene();
        break;

    case OverlayTab::Skill:
        DrawSkillMenuScene();
        break;
    }
}

bool IsOverlayOpen()
{
    return g_IsOverlayOpen;
}