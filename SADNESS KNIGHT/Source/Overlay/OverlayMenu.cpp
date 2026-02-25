#include "OverlayMenu.h"
#include "../Overlay/Equip/EquipMenu.h"
#include "../Input/Input.h"
// 将来
// #include "../SkillMenu/SkillMenu.h"

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

void NextOverlayTab()
{
    if (g_CurrentTab == OverlayTab::Equip)
        g_CurrentTab = OverlayTab::Skill;
    else
        g_CurrentTab = OverlayTab::Equip;
}

void PrevOverlayTab()
{
    NextOverlayTab();
}

void UpdateOverlayMenu()
{
    if (!g_IsOverlayOpen) return;

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
        // UpdateSkillMenuScene();
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
        // DrawSkillMenuScene();
        break;
    }
}