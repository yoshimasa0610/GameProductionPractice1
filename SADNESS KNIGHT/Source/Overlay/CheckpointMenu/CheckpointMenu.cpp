#include "CheckpointMenu.h"
#include "DxLib.h"
#include "../../Input/Input.h"
#include "../../Scene/Play/Play.h"
#include "../Equip/EquipMenu.h"
#include "../SkillMenu/SkillMenu.h"
#include "../OverlayMenu.h"
#include "../../Map/Checkpoint/CheckpointManager.h"

static bool g_IsCheckpointMenuOpen = false;
static PlayerData* g_PlayerRef = nullptr;
static CheckpointMenuItem g_CurrentItem = CheckpointMenuItem::Equip;
static bool g_IsInSubMenu = false;

static const char* GetItemName(CheckpointMenuItem item)
{
    switch (item)
    {
    case CheckpointMenuItem::Equip: return "レリック装備";
    case CheckpointMenuItem::Skill: return "スキルセット";
    case CheckpointMenuItem::Leave: return "出発する";
    default: return "";
    }
}

bool IsCheckpointMenuOpen()
{
    return g_IsCheckpointMenuOpen;
}

void OpenCheckpointMenu(PlayerData* player)
{
    g_IsCheckpointMenuOpen = true;
    g_PlayerRef = player;
    g_CurrentItem = CheckpointMenuItem::Equip;
    g_IsInSubMenu = false;
    SetPaused(true);
}

void CloseCheckpointMenu()
{
    if (g_IsInSubMenu)
    {
        switch (g_CurrentItem)
        {
        case CheckpointMenuItem::Equip:
            CloseEquipMenu();
            break;

        case CheckpointMenuItem::Skill:
            CloseSkillMenu();
            break;

        default:
            break;
        }
    }

    g_IsCheckpointMenuOpen = false;
    g_PlayerRef = nullptr;
    g_IsInSubMenu = false;
    SetPaused(false);
}

static void EnterCurrentMenu()
{
    g_IsInSubMenu = true;

    switch (g_CurrentItem)
    {
    case CheckpointMenuItem::Equip:
        OpenEquipMenu(g_PlayerRef);
        break;

    case CheckpointMenuItem::Skill:
        OpenSkillMenu(g_PlayerRef);
        break;

    case CheckpointMenuItem::Leave:
        CloseCheckpointMenu();
        SetSitState(SitState::None);
        break;

    default:
        break;
    }
}

void UpdateCheckpointMenu()
{
    if (!g_IsCheckpointMenuOpen) return;

    if (!g_IsInSubMenu)
    {
        if (IsTriggerKey(KEY_UI_UP))
        {
            int index = (int)g_CurrentItem - 1;
            if (index < 0) index = (int)CheckpointMenuItem::Count - 1;
            g_CurrentItem = (CheckpointMenuItem)index;
        }

        if (IsTriggerKey(KEY_UI_DOWN))
        {
            int index = (int)g_CurrentItem + 1;
            if (index >= (int)CheckpointMenuItem::Count) index = 0;
            g_CurrentItem = (CheckpointMenuItem)index;
        }

        if (IsTriggerKey(KEY_OK))
        {
            EnterCurrentMenu();
            return;
        }

        if (IsTriggerKey(KEY_CANCEL))
        {
            CloseCheckpointMenu();
            SetSitState(SitState::None);
            return;
        }
    }
    else
    {
        if (IsTriggerKey(KEY_CANCEL))
        {
            switch (g_CurrentItem)
            {
            case CheckpointMenuItem::Equip:
                CloseEquipMenu();
                break;

            case CheckpointMenuItem::Skill:
                CloseSkillMenu();
                break;
            default:
                break;
            }

            g_IsInSubMenu = false;
            return;
        }

        switch (g_CurrentItem)
        {
        case CheckpointMenuItem::Equip:
            UpdateEquipMenuScene();
            break;

        case CheckpointMenuItem::Skill:
            UpdateSkillMenuScene();
            break;

        default:
            break;
        }
    }
}

static void DrawBackground()
{
    int sw, sh;
    GetScreenState(&sw, &sh, nullptr);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(0, 0, sw, sh, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

static void DrawLeftMenu()
{
    int sw, sh;
    GetScreenState(&sw, &sh, nullptr);

    int startX = 80;
    int startY = 140;
    int lineH = 60;

    DrawString(startX, 60, "レストポイント", GetColor(255, 255, 255));

    for (int i = 0; i < (int)CheckpointMenuItem::Count; i++)
    {
        int y = startY + i * lineH;
        bool selected = ((int)g_CurrentItem == i) && !g_IsInSubMenu;

        if (selected)
        {
            DrawString(startX - 24, y, "●", GetColor(255, 80, 80));
        }

        int color = selected ? GetColor(255, 255, 255) : GetColor(180, 180, 180);
        DrawString(startX, y, GetItemName((CheckpointMenuItem)i), color);
    }
}

void DrawCheckpointMenu()
{
    if (!g_IsCheckpointMenuOpen) return;

    DrawBackground();
    DrawLeftMenu();

    OverlayArea area = GetOverlayContentArea();

    switch (g_CurrentItem)
    {
    case CheckpointMenuItem::Equip:
        if (g_IsInSubMenu)
            DrawEquipMenuScene(area);
        break;

    case CheckpointMenuItem::Skill:
        if (g_IsInSubMenu)
            DrawSkillMenuScene(area);
        break;

    default:
        break;
    }
}