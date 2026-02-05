#include "../SceneManager.h"
#include <DxLib.h>
#include "PlayerScene.h"

int g_PlayBGHandle = -1;
CharacterData g_CharacterData[CHARACTER_TYPE_MAX] = {};

void PlayerScene::InitPlayScene()
{
}

void PlayerScene::LoadPlayScene()
{
    // ”wŒi
    g_PlayBGHandle = LoadGraph(_T("Data/Play/BG.png"));

    // ƒLƒƒƒ‰‰æ‘œ
    g_CharacterData[CHARACTER_TYPE_HERO].handle =
        LoadGraph(_T("Data/Player/Player.png"));

    g_CharacterData[CHARACTER_TYPE_ENEMY_A].handle =
        LoadGraph(_T("Data/Enemy/DragonFly.png"));

    g_CharacterData[CHARACTER_TYPE_ENEMY_B].handle =
        LoadGraph(_T("Data/Enemy/Flies.png"));
}

void PlayerScene::StartPlayScene()
{
    // ‰ŠúˆÊ’uÝ’è
    g_CharacterData[CHARACTER_TYPE_HERO].pos =
        VGet(100.0f, 100.0f, 0.0f);

    g_CharacterData[CHARACTER_TYPE_ENEMY_A].pos =
        VGet(800.0f, 200.0f, 0.0f);

    g_CharacterData[CHARACTER_TYPE_ENEMY_B].pos =
        VGet(900.0f, 600.0f, 0.0f);
}

void PlayerScene::StepPlayScene()
{
    if (CheckHitKey(KEY_INPUT_C))
    {
        sceneManager->RequestSceneChange(SceneType::Title);
    }
}

void PlayerScene::UpdatePlayScene()
{
}

void PlayerScene::DrawPlayScene()
{
    // ”wŒi
    DrawGraph(0, 0, g_PlayBGHandle, TRUE);

    // ƒLƒƒƒ‰•`‰æ
    for (int i = 0; i < CHARACTER_TYPE_MAX; i++)
    {
        DrawGraph(
            (int)g_CharacterData[i].pos.x,
            (int)g_CharacterData[i].pos.y,
            g_CharacterData[i].handle,
            TRUE
        );
    }
}

void FinPlayScene()
{
    // ”wŒiíœ
    DeleteGraph(g_PlayBGHandle);
    g_PlayBGHandle = -1;

    // ƒLƒƒƒ‰íœ
    for (int i = 0; i < CHARACTER_TYPE_MAX; i++)
    {
        DeleteGraph(g_CharacterData[i].handle);
        g_CharacterData[i].handle = -1;
    }
}
