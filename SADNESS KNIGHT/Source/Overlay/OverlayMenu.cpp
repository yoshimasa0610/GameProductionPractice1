#include "OverlayMenu.h"
#include "../Overlay/Equip/EquipMenu.h"
#include "../Input/Input.h"
#include "../Overlay/SkillMenu/SkillMenu.h"
#include "../Scene/Play/Play.h"

static OverlayTab g_CurrentTab = OverlayTab::Equip;
static PlayerData* g_PlayerRef = nullptr;
static bool g_IsOverlayOpen = false;
// 前方宣言
static void OnTabChanged();
OverlayArea GetOverlayContentArea();
static void DrawOverlayBackground();

void OpenOverlayMenu(PlayerData* player)
{
    g_IsOverlayOpen = true;
    g_PlayerRef = player;
    g_CurrentTab = OverlayTab::Equip;
    SetPaused(true);
    // Equipタブ開始
    OnTabChanged();
}

static void CloseCurrentTab()
{
    switch (g_CurrentTab)
    {
    case OverlayTab::Equip:
        CloseEquipMenu();
        break;

    case OverlayTab::Skill:
        // CloseSkillMenu(); ←あとで作る
        break;
    }
}

void CloseOverlayMenu()
{
    CloseCurrentTab();
    g_IsOverlayOpen = false;
    SetPaused(false);
}

static void OnTabChanged()
{
    CloseCurrentTab();   // ★追加

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
    if (IsTriggerKey(KEY_UI_LEFT))
        PrevOverlayTab();

    if (IsTriggerKey(KEY_UI_RIGHT))
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
    OverlayArea area = GetOverlayContentArea();

    int x = area.x + area.w / 2 - 160;
    int y = area.y - 140;

    const char* names[] =
    {
        "Relic",
        "Skill"
    };

    for (int i = 0; i < (int)OverlayTab::Count; i++)
    {
        int color =
            (i == (int)g_CurrentTab)
            ? GetColor(255, 255, 0)
            : GetColor(180, 180, 180);

        DrawString(x + i * 160, y, names[i], color);
    }
}

void DrawOverlayMenu()
{
    if (!g_IsOverlayOpen) return;

    DrawOverlayBackground();   // 背景
    DrawOverlayTabs();         // 共通タブ

    OverlayArea area = GetOverlayContentArea(); // 子用領域

    switch (g_CurrentTab)
    {
    case OverlayTab::Equip:
        DrawEquipMenuScene(area);
        break;

    case OverlayTab::Skill:
        DrawSkillMenuScene(area);
        break;
    }
}

bool IsOverlayOpen()
{
    return g_IsOverlayOpen;
}

OverlayArea GetOverlayContentArea()
{
    int w, h;
    GetScreenState(&w, &h, nullptr);

    const int panelW = 1200;
    const int panelH = 700;

    const int tabHeight = 80;

    OverlayArea area;

    // 中央配置
    int panelX = (w - panelW) / 2;
    int panelY = (h - panelH) / 2;

    area.x = panelX;
    area.y = panelY + tabHeight;
    area.w = panelW;
    area.h = panelH - tabHeight;

    return area;
}

static void DrawOverlayBackground()
{
    int w, h;
    GetScreenState(&w, &h, nullptr);

    // 灰色半透明（Playが見える）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
    DrawBox(0, 0, w, h, GetColor(40, 40, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}