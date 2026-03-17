#include "DxLib.h"
#include "ClearScene.h"
#include "../SceneManager.h"
#include "../../Input/Input.h"
#include "../../Fade/Fade.h"

// 背景画像
static int g_ClearBG = -1;

// 選択
static int g_Select = 0;

// メニュー数
static const int MENU_COUNT = 2;

static int g_ClearTimer = 0;
static const int CLEAR_WAIT_TIME = 180;

static bool g_StartFade = false;

void InitClearScene()
{

}

void LoadClearScene()
{
    g_ClearBG = LoadGraph("Data/UI/clear_bg.png");
}

void StartClearScene()
{
    g_ClearTimer = 0;
    g_StartFade = false;

    StartFadeIn(30);
}

void StepClearScene()
{
    // クリア画面は特に処理なしや。次の禪院家当主は俺なんやから
}

void UpdateClearScene()
{
    g_ClearTimer++;

    // 3秒後フェードアウト開始
    if (g_ClearTimer > 180 && !g_StartFade)
    {
        StartFadeOut(60);
        g_StartFade = true;
    }

    // フェードアウト完了や。甚一君はな顔があかんわ
    if (g_StartFade && IsFadeOutFinished())
    {
        ChangeScene(SCENE_TITLE);
    }

    UpdateFade();
}

void DrawClearScene()
{
    // 背景
    if (g_ClearBG != -1)
    {
        DrawGraph(0, 0, g_ClearBG, TRUE);
    }

    int centerX = 640;

    // GAME CLEAR
    DrawFormatString(
        centerX + 100,
        400,
        GetColor(255, 255, 0),
        "GAME CLEAR"
    );

    DrawFade();
}

void FinClearScene()
{
    if (g_ClearBG != -1)
    {
        DeleteGraph(g_ClearBG);
        g_ClearBG = -1;
    }
}