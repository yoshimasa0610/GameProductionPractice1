#include "DxLib.h"
#include "ClearScene.h"
#include "../SceneManager.h"
#include "../../Input/Input.h"
#include "../../Fade/Fade.h"
#include "../../GameSetting/GameSetting.h"

static int g_ClearBG = -1;
static int g_ClearText = -1;

static int g_ClearTimer = 0;
static bool g_StartFade = false;

void InitClearScene()
{
}

void LoadClearScene()
{
    g_ClearBG = LoadGraph("Data/UI/clear_bg.png");
    if (g_ClearBG == -1) g_ClearBG = LoadGraph("Data/Title/TitleBG.png");

    g_ClearText = LoadGraph("Data/Title/ClearUI.png");
    if (g_ClearText == -1) g_ClearText = LoadGraph("Data/Clear/ClearText.png");
    if (g_ClearText == -1) g_ClearText = LoadGraph("Data/Clear/ClearText01.png");
    if (g_ClearText == -1) g_ClearText = LoadGraph("Data/UI/ClearText.png");
    if (g_ClearText == -1) g_ClearText = LoadGraph("Data/UI/clear_text.png");
}

void StartClearScene()
{
    g_ClearTimer = 0;
    g_StartFade = false;
    StartFadeIn(30);
}

void StepClearScene()
{
}

void UpdateClearScene()
{
    g_ClearTimer++;

    if (g_ClearTimer > 180 && !g_StartFade)
    {
        StartFadeOut(60);
        g_StartFade = true;
    }

    if (g_StartFade && IsFadeOutFinished())
    {
        ChangeScene(SCENE_TITLE);
    }

    UpdateFade();
}

void DrawClearScene()
{
    if (g_ClearBG != -1)
    {
        DrawGraph(0, 0, g_ClearBG, TRUE);
    }

    if (g_ClearText != -1)
    {
        int screenW = 0;
        int screenH = 0;
        GetDrawScreenSize(&screenW, &screenH);

        int textW = 0;
        int textH = 0;
        GetGraphSize(g_ClearText, &textW, &textH);

        int drawX = (screenW - textW) / 2;
        int drawY = (screenH - textH) / 2;
        DrawGraph(drawX, drawY, g_ClearText, TRUE);
    }
    else
    {
        const char* clearText = "GAME CLEAR";
        int textW = GetDrawStringWidth(clearText, sizeof("GAME CLEAR") - 1);
        int drawX = (SCREEN_WIDTH - textW) / 2;
        int drawY = SCREEN_HEIGHT / 2;
        DrawFormatString(drawX, drawY, GetColor(255, 255, 0), "%s", clearText);
    }

    DrawFade();
}

void FinClearScene()
{
    if (g_ClearBG != -1)
    {
        DeleteGraph(g_ClearBG);
        g_ClearBG = -1;
    }

    if (g_ClearText != -1)
    {
        DeleteGraph(g_ClearText);
        g_ClearText = -1;
    }
}