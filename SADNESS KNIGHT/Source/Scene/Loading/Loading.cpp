#include "DxLib.h"
#include "../SceneManager.h"
#include "../Play/Play.h"
#include "../../Fade/Fade.h"

static int g_Timer = 0;
static bool g_LoadStarted = false;

void InitLoadingScene()
{
    InitFade();
    StartFadeIn(30);

    g_Timer = 0;
    g_LoadStarted = false;
}

void LoadLoadingScene() {}
void StartLoadingScene() {}
void StepLoadingScene() {}

void UpdateLoadingScene()
{
    
    if (!g_LoadStarted)
    {
        g_LoadStarted = true;

        InitPlayScene();
        LoadPlayScene();
    }

    g_Timer++;

    if (g_Timer > 60)
    {
        ChangeScene(SCENE_PLAY);
    }
}

void DrawLoadingScene()
{
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);

    DrawString(540, 360, "Now Loading...", GetColor(255, 255, 255));
}

void FinLoadingScene() {}