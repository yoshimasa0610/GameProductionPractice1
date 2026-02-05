/// <summary>
/// タイトルシーン実装
/// タイトル画面の表示、入力処理、画像管理を実装
/// </summary>

#include "TitleScene.h"
#include "SceneManager.h"
#include "DxLib.h"

/// <summary>
/// コンストラクタ
/// シーンマネージャーへの参照を保存し、画像ハンドルを初期化
/// </summary>
/// <param name="manager">シーンマネージャーへのポインタ</param>
TitleScene::TitleScene(SceneManager* manager)
    : sceneManager(manager)
    , bgHandle(-1)        // -1は未読み込み状態
    , titleHandle(-1)     // -1は未読み込み状態
{
}

/// <summary>
/// デストラクタ
/// 特別な処理は行わない（Finalize()でリソース解放）
/// </summary>
TitleScene::~TitleScene()
{
}

/// <summary>
/// 初期化処理
/// タイトル画面で使用する画像をファイルから読み込む
/// </summary>
void TitleScene::Initialize()
{
    // タイトルシーンの初期化処理
    // 画像の読み込み
    bgHandle = LoadGraph(_T("Data/Title/BG.png"));              // 背景画像
    titleHandle = LoadGraph(_T("Data/Title/TitleText.png"));    // タイトルロゴ画像
}

/// <summary>
/// 更新処理
/// キー入力をチェックし、対応するシーンへの遷移をリクエスト
/// </summary>
void TitleScene::Update()
{
    // Zキーでプレイシーンへ遷移
    if (CheckHitKey(KEY_INPUT_Z))
    {
        sceneManager->RequestSceneChange(SceneType::Play);
    }

    // Xキーでオプションシーンへ遷移
    if (CheckHitKey(KEY_INPUT_X))
    {
        sceneManager->RequestSceneChange(SceneType::Option);
    }

    // タイトルシーンの更新処理（必要に応じて追加）
}

/// <summary>
/// 描画処理
/// 背景、タイトルロゴ、操作説明テキストを画面に描画
/// </summary>
void TitleScene::Draw()
{
    // 背景をクリア
    ClearDrawScreen();

    // 背景画像を描画
    if (bgHandle != -1)  // 読み込み成功時のみ描画
    {
        DrawGraph(0, 0, bgHandle, TRUE);
    }

    // タイトルロゴを描画（画面中央より少し上に配置）
    if (titleHandle != -1)  // 読み込み成功時のみ描画
    {
        // タイトル画像のサイズを取得
        int width, height;
        GetGraphSize(titleHandle, &width, &height);
        
        // 画面中央（800, 450）を基準に、少し上にオフセット
        int x = 800 - (width / 2);           // 横方向: 中央揃え (1600 / 2 = 800)
        int y = 450 - (height / 2) - 100;    // 縦方向: 中央より100ピクセル上 (900 / 2 = 450)
        
        DrawGraph(x, y, titleHandle, TRUE);
    }

    // 操作説明テキスト（_T マクロで文字列をTCHAR型に変換）
    DrawString(600, 750, _T("Press Z Key : Start Game"), GetColor(255, 255, 0));
    DrawString(600, 800, _T("Press X Key : Option"), GetColor(255, 255, 0));

    // 画面の更新（裏画面を表画面に反映）
    ScreenFlip();
}

/// <summary>
/// 終了処理
/// 読み込んだ画像を解放し、メモリリークを防止
/// </summary>
void TitleScene::Finalize()
{
    // タイトルシーンの終了処理
    // 画像の解放
    if (bgHandle != -1)
    {
        DeleteGraph(bgHandle);
        bgHandle = -1;  // ハンドルを無効化
    }

    if (titleHandle != -1)
    {
        DeleteGraph(titleHandle);
        titleHandle = -1;  // ハンドルを無効化
    }
}