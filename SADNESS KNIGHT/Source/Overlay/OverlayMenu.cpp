#include "OverlayMenu.h"
#include "../Overlay/Equip/EquipMenu.h"
#include "../Input/Input.h"
#include "../Overlay/SkillMenu/SkillMenu.h"

static OverlayTab g_CurrentTab = OverlayTab::Equip;
static PlayerData* g_PlayerRef = nullptr;

bool g_IsOverlayOpen = false;

void OpenOverlayMenu(PlayerData* player)
{
    g_IsOverlayOpen = true;
    g_PlayerRef = player;
    g_CurrentTab = OverlayTab::Equip;

    // Equipタブ開始
    SetEquipMenuPlayer(player);
}

void CloseOverlayMenu()
{
    g_IsOverlayOpen = false;
}
// 次のタブに移動
void NextOverlayTab()
{
    if (g_CurrentTab == OverlayTab::Equip)
        g_CurrentTab = OverlayTab::Skill;
    else
        g_CurrentTab = OverlayTab::Equip;
}
// 前のタブに移動（今回はタブが2つしかないのでNextと同じ）
void PrevOverlayTab()
{
    NextOverlayTab();
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

void DrawOverlayMenu()
{
    if (!g_IsOverlayOpen) return;

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