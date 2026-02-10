#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "FPS/FPS.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // ウィンドウモード有効化
    ChangeWindowMode(TRUE);

    // 画面解像度とカラービット数を設定
    SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

    // DxLib初期化
    if (DxLib_Init() == -1)
    {
        return -1; // 初期化失敗なら即終了
    }

    // ウィンドウサイズ設定
    SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 透過色設定
    SetTransColor(TRANS_COLOR_R, TRANS_COLOR_G, TRANS_COLOR_B);

    // 裏画面を描画先に設定
    SetDrawScreen(DX_SCREEN_BACK);

    // =====================
    // メインループ
    // =====================
    while (ProcessMessage() >= 0)
    {
        Sleep(1);            // CPU負荷軽減
        ClearDrawScreen();   // 画面クリア

        //UpdateInput();       // 入力更新
        //SceneManagerUpdate();// シーン管理更新
        UpdateFPS();         // FPS更新
        FPSWait();           // フレーム調整

        ScreenFlip();        // 表画面と裏画面を入れ替え
    }

    // =====================
    // 終了処理
    // =====================
    /*FinUIImage();
    FinUIText();
    FinBGM();
    FinSE();
    FinScene();*/
    DxLib_End();

    return 0;
}