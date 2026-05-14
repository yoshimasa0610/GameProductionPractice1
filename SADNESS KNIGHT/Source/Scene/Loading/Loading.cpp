#include "DxLib.h"
#include "../SceneManager.h"
#include "../Play/Play.h"
#include "../../Fade/Fade.h"
#include "../../GameSetting/GameSetting.h"

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

        //ここでPlaySceneをロード、Initしていたせいで
        //挙動がおかしかったです。詫びろ
		//↓諸悪の根源をここに置いておきます。
        //InitPlayScene();
        //LoadPlayScene();
    }

    g_Timer++;

    if (g_Timer > 60)
    {
        ChangeScene(SCENE_PLAY);
    }
}

void DrawLoadingScene()
{
    DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);

    DrawString(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2, "Now Loading...", GetColor(255, 255, 255));
}

void FinLoadingScene() {}