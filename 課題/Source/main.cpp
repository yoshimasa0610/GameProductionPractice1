#include "DxLib.h"
#include "SceneManager.h"

// WinMain関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);

    // 画面サイズを設定
    SetGraphMode(800, 600, 32);

    // DXライブラリの初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // 描画先を裏画面に設定
    SetDrawScreen(DX_SCREEN_BACK);

    // シーンマネージャーの生成
    SceneManager* sceneManager = new SceneManager();

    // タイトルシーンから開始
    sceneManager->Initialize(SceneType::Title);

    // メインループ
    while (ProcessMessage() == 0)
    {
        // ESCキーで終了
        if (CheckHitKey(KEY_INPUT_ESCAPE))
        {
            break;
        }

        // 更新処理
        sceneManager->Update();

        // 描画処理
        sceneManager->Draw();
    }

    // シーンマネージャーの終了処理
    sceneManager->Finalize();
    delete sceneManager;

    // DXライブラリの終了処理
    DxLib_End();

    return 0;
}