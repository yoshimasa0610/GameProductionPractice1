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
       

        // テスト用: Uキーで二段ジャンプを解放
        if (CheckHitKey(KEY_INPUT_U) == 1)
        {
            static bool unlocked = false;
            if (!unlocked)
            {
                UnlockDoubleJump();
                unlocked = true;
            }
        }

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
        DrawFormatString(10, 280, GetColor(255, 255, 255), "[TAB] Heal");
        DrawFormatString(10, 300, GetColor(255, 255, 255), "[SHIFT] Dodge");
        DrawFormatString(10, 320, GetColor(255, 255, 255), "[U] Unlock Double Jump (Test)");
        DrawFormatString(10, 340, GetColor(255, 255, 255), "[ESC] Exit");
        
        // 二段ジャンプの状態表示
        if (HasDoubleJump())
        {
            DrawFormatString(10, 370, GetColor(100, 255, 100), "Double Jump: UNLOCKED");
        }
        else
        {
            DrawFormatString(10, 370, GetColor(255, 100, 100), "Double Jump: LOCKED");
        }
        
        // 回復回数の表示
        DrawFormatString(10, 390, GetColor(100, 255, 255), "Heal Count: %d / %d", GetHealCount(), GetMaxHealCount());
        
        // 無敵状態の表示
        if (IsPlayerInvincible())
        {
            DrawFormatString(10, 410, GetColor(255, 255, 0), "INVINCIBLE!");
        }

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
