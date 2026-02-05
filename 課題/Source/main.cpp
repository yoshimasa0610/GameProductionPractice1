/// <summary>
/// メインファイル
/// ゲームの起動、初期化、メインループ、終了処理を管理する
/// </summary>

#include "DxLib.h"
#include "SceneManager.h"

/// <summary>
/// Windowsアプリケーションのエントリポイント
/// DXライブラリの初期化、シーンマネージャーの生成、メインループの実行を行う
/// </summary>
/// <param name="hInstance">現在のインスタンスハンドル</param>
/// <param name="hPrevInstance">以前のインスタンスハンドル（Win32では未使用）</param>
/// <param name="lpCmdLine">コマンドライン引数</param>
/// <param name="nCmdShow">ウィンドウの表示状態</param>
/// <returns>プログラムの終了コード（0: 正常終了）</returns>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);

    // 画面サイズを設定（1600x900ピクセル、32ビットカラー）
    SetGraphMode(1600, 900, 32);

    // DXライブラリの初期化
    if (DxLib_Init() == -1)
    {
        return -1;  // 初期化失敗
    }

    // 描画先を裏画面に設定（ダブルバッファリング有効化）
    SetDrawScreen(DX_SCREEN_BACK);

    // シーンマネージャーの生成
    SceneManager* sceneManager = new SceneManager();

    // タイトルシーンから開始
    sceneManager->Initialize(SceneType::Title);

    // メインループ（ウィンドウが閉じられるまで繰り返す）
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