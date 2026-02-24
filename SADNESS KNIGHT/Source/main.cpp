#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "Player/Player.h"
#include "Input/Input.h"
#include "Scene/SceneManager.h"
#include "Scene/Title/Title.h"

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    // ウィンドウモード設定
    ChangeWindowMode(TRUE);

    // 画面サイズ設定
    SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 描画先を裏画面に設定
    SetDrawScreen(DX_SCREEN_BACK);


    // 入力システム初期化
    InitInput();



    // メインループ
    while (ProcessMessage() == 0)
    {
        // 画面クリア（背景を黒に）
        ClearDrawScreen();

        // 背景色確認用
        DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(20, 20, 40), TRUE);

        // 入力更新
        UpdateInput();

        SceneManagerUpdate();
       

        // 裏画面を表画面に反映
        ScreenFlip();

        // ESCキーで終了
        if (CheckHitKey(KEY_INPUT_ESCAPE))
        {
            break;
        }
    }

    // リソース解放
    UnloadPlayer();

    // DXライブラリ終了
    DxLib_End();

    return 0;
}
