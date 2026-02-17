#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "Player/Player.h"
#include "Input/Input.h"

INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    // ウィンドウモード設定
    ChangeWindowMode(TRUE);

    // 画面サイズ設定
    SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

    // ウィンドウタイトル
    SetMainWindowText("SADNESS KNIGHT - Player Test");

    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 描画先を裏画面に設定
    SetDrawScreen(DX_SCREEN_BACK);

    // 画像ファイルの存在を確認
    printfDx("=== Checking Image Files ===\n");
    if (FileRead_open("Data/Player/Idle.png") != 0)
    {
        printfDx("Idle.png: FOUND\n");
        FileRead_close(FileRead_open("Data/Player/Idle.png"));
    }
    else
    {
        printfDx("Idle.png: NOT FOUND\n");
    }
    
    if (FileRead_open("Data/Player/Walk.png") != 0)
    {
        printfDx("Walk.png: FOUND\n");
        FileRead_close(FileRead_open("Data/Player/Walk.png"));
    }
    else
    {
        printfDx("Walk.png: NOT FOUND\n");
    }
    
    if (FileRead_open("Data/Player/Jump.png") != 0)
    {
        printfDx("Jump.png: FOUND\n");
        FileRead_close(FileRead_open("Data/Player/Jump.png"));
    }
    else
    {
        printfDx("Jump.png: NOT FOUND\n");
    }
    printfDx("============================\n");

    // 入力システム初期化
    InitInput();

    // プレイヤー初期化（画面中央、地面より少し上）
    InitPlayer(400.0f, 500.0f);
    LoadPlayer();

    // メインループ
    while (ProcessMessage() == 0)
    {
        // 画面クリア（背景を黒に）
        ClearDrawScreen();

        // 背景色確認用
        DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(20, 20, 40), TRUE);

        // 入力更新
        UpdateInput();

        // プレイヤー更新
        UpdatePlayer();

        // プレイヤー描画
        DrawPlayer();

        // 地面のライン表示
        DrawLine(0, 700, SCREEN_WIDTH, 700, GetColor(100, 255, 100), 2);

        // FPS表示
        DrawFormatString(SCREEN_WIDTH - 100, 10, GetColor(255, 255, 255), "FPS: %.1f", GetFPS());

        // 操作説明
        DrawFormatString(10, 200, GetColor(200, 200, 200), "=== Controls ===");
        DrawFormatString(10, 220, GetColor(255, 255, 255), "[A][D] Move");
        DrawFormatString(10, 240, GetColor(255, 255, 255), "[W] Jump");
        DrawFormatString(10, 260, GetColor(255, 255, 255), "[Q][E][F] Skills");
        DrawFormatString(10, 280, GetColor(255, 255, 255), "[ESC] Exit");

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
