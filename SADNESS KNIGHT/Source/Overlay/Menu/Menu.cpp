#include "DxLib.h"
#include "Menu.h"
#include "../../Input/Input.h"
#include "../../Sound/Sound.h"
#include "../../GameSetting/GameSetting.h"
#include "../Option/Option.h"
#include "../../Camera/Camera.h"
#include "../../Scene/Play/Play.h"
#include "../../Scene/SceneManager.h"

bool g_IsMenuOpen = false;

static const char* g_MenuItems[] =
{
    "ゲームに戻る",
    "オプション",
    "タイトルに戻る"
};
static const int g_MenuItemCount = sizeof(g_MenuItems) / sizeof(g_MenuItems[0]);
static int g_SelectedIndex = 0;

void OpenMenu()
{
    g_IsMenuOpen = true;
    g_SelectedIndex = 0;
    SetPaused(true);
}

void CloseMenu()
{
    g_IsMenuOpen = false;
    SetPaused(false);
}

void UpdateMenu()
{
    if (!g_IsMenuOpen) return;

    if (IsTriggerKey(KEY_UP))
    {
        g_SelectedIndex = (g_SelectedIndex - 1 + g_MenuItemCount) % g_MenuItemCount;
        PlaySE(SE_MENU_MOVE);
    }
    if (IsTriggerKey(KEY_DOWN))
    {
        g_SelectedIndex = (g_SelectedIndex + 1) % g_MenuItemCount;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_OK))
    {
        PlaySE(SE_MENU_OK);

        switch (g_SelectedIndex)
        {
        case 0: // ゲームに戻る
            CloseMenu();
            break;

        case 1: // オプション
            OpenOption();
            break;

        case 2: // タイトルに戻る
            CloseMenu();
            ChangeScene(SCENE_TITLE);
            break;
        }
    }

    if (IsTriggerKey(KEY_CANCEL))
    {
        CloseMenu();
    }
}

void DrawMenu()
{
    if (!g_IsMenuOpen) return;

    int screenW = SCREEN_WIDTH;
    int screenH = SCREEN_HEIGHT;

    // --- 半透明黒背景 ---
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160); // 0～255（160くらいが見やすい）
    DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    // --- メニューウィンドウサイズ ---
    int winW = 500;
    int winH = 300;
    int winX = (screenW - winW) / 2;
    int winY = (screenH - winH) / 2;

    // --- ウィンドウ描画 ---(メニューに枠として四角い箱が欲しいならコメント外したらいいよ)
    //DrawBox(winX, winY, winX + winW, winY + winH, GetColor(30, 30, 30), TRUE);
    //DrawBox(winX, winY, winX + winW, winY + winH, GetColor(180, 180, 180), FALSE);

    // --- タイトル ---
    DrawString(winX + 140, winY + 30, "=== メニュー ===", GetColor(255, 255, 0));

    // --- メニュー項目 ---
    for (int i = 0; i < g_MenuItemCount; i++)
    {
        int color = (i == g_SelectedIndex)
            ? GetColor(255, 255, 0)
            : GetColor(255, 255, 255);

        DrawString(
            winX + 160,
            winY + 90 + i * 40,
            g_MenuItems[i],
            color
        );
    }
}