#include "CheckpointMenu.h"
#include "DxLib.h"
#include "../../Input/Input.h"
#include "../../Scene/Play/Play.h"
#include "../Equip/EquipMenu.h"
#include "../SkillMenu/SkillMenu.h"
#include "../OverlayMenu.h"
#include "../../Map/Checkpoint/CheckpointManager.h"

// チェックポイントメニューの実装
static bool g_IsCheckpointMenuOpen = false;
// プレイヤーデータへの参照（装備やスキルメニューで使用）
static PlayerData* g_PlayerRef = nullptr;
// メインメニューの現在選択されている項目
static CheckpointMenuItem g_CurrentItem = CheckpointMenuItem::Equip;
// サブメニュー（装備やスキル選択）に入っているかどうか
static bool g_IsInSubMenu = false;
// フォントを大きくするための変数
static int g_MenuFont = -1;
static int g_TitleFont = -1;

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

    if (g_MenuFont == -1)
    {
        g_MenuFont = CreateFontToHandle("游明朝", 28, 3);
    }

    if (g_TitleFont == -1)
    {
        g_TitleFont = CreateFontToHandle(nullptr, 24, 3);
    }

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
    switch (g_CurrentItem)
    {
    case CheckpointMenuItem::Equip:
        OpenEquipMenu(g_PlayerRef);
        g_IsInSubMenu = true;
        break;

    case CheckpointMenuItem::Skill:
        OpenSkillMenu(g_PlayerRef);
        g_IsInSubMenu = true;
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

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawBox(0, 0, sw, sh, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

static void DrawLeftMenu()
{
    int sw, sh;
    GetScreenState(&sw, &sh, nullptr);

    int startX = 40;
    int startY = 200;
    int lineH = 80;

    DrawStringToHandle(
        startX,
        100,
        "レストポイント",
        GetColor(255, 255, 255),
        g_TitleFont
    );

    for (int i = 0; i < (int)CheckpointMenuItem::Count; i++)
    {
        int y = startY + i * lineH;
        bool selected = ((int)g_CurrentItem == i) && !g_IsInSubMenu;

        if (selected)
        {
            DrawStringToHandle(
                startX + 40,
                y,
                "●",
                GetColor(255, 80, 80),
                g_MenuFont
            );
        }

        int color = selected ? GetColor(255, 255, 255) : GetColor(180, 180, 180);

        DrawStringToHandle(
            startX + 80,
            y,
            GetItemName((CheckpointMenuItem)i),
            color,
            g_MenuFont
        );
    }
}

void DrawCheckpointMenu()
{
    if (!g_IsCheckpointMenuOpen) return;

    if (!g_IsInSubMenu)
    {
        DrawBackground();
        DrawLeftMenu();
    }

    OverlayArea area = GetOverlayContentArea();

    switch (g_CurrentItem)
    {
    case CheckpointMenuItem::Equip:
        if (g_IsInSubMenu)
        {
            DrawOverlayBackground();
            DrawEquipMenuScene(area);
        }
        break;

    case CheckpointMenuItem::Skill:
        if (g_IsInSubMenu)
        {
            DrawOverlayBackground();
            DrawSkillMenuScene(area);
        }
        break;

    default:
        break;
    }
}