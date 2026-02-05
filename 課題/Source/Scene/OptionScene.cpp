/// <summary>
/// オプションシーン実装
/// オプション設定画面の表示と入力処理を実装
/// </summary>

#include "OptionScene.h"
#include "DxLib.h"
#include "../SceneManager.h"

/// <summary>
/// コンストラクタ
/// シーンマネージャーへの参照を保存し、全ての画像ハンドルを初期化
/// </summary>
/// <param name="manager">シーンマネージャーへのポインタ</param>
OptionScene::OptionScene(SceneManager* manager)
    : sceneManager(manager)
    , bgHandle(-1)       // -1は未読み込み状態
    , optionHandle(-1)   // -1は未読み込み状態
    , bgmHandle(-1)      // -1は未読み込み状態
    , seHandle(-1)       // -1は未読み込み状態
    , gaugeHandle(-1)    // -1は未読み込み状態
{
}

/// <summary>
/// デストラクタ
/// 特別な処理は行わない（Finalize()でリソース解放）
/// </summary>
OptionScene::~OptionScene()
{
}

/// <summary>
/// 初期化処理
/// オプション画面で使用する全ての画像をファイルから読み込む
/// </summary>
void OptionScene::Initialize()
{
    // 画像読み込み
    bgHandle = LoadGraph(_T("Data/Option/BG.png"));           // 背景画像
    optionHandle = LoadGraph(_T("Data/Option/Option.png"));   // オプション文字画像
    bgmHandle = LoadGraph(_T("Data/Option/BGM.png"));         // BGM文字画像
    seHandle = LoadGraph(_T("Data/Option/SE.png"));           // SE文字画像
    gaugeHandle = LoadGraph(_T("Data/Option/Gauge.png"));     // ゲージ画像
}

/// <summary>
/// 更新処理
/// Cキーが押されたらタイトルシーンへの遷移をリクエスト
/// </summary>
void OptionScene::Update()
{
    // Cキーでタイトルへ戻る
    if (CheckHitKey(KEY_INPUT_C))
    {
        sceneManager->RequestSceneChange(SceneType::Title);
    }
}

/// <summary>
/// 描画処理
/// 背景、オプション要素、ゲージを画面に描画
/// </summary>
void OptionScene::Draw()
{
    // 画面クリア
    ClearDrawScreen();

    // 画像描画（座標は調整可能）
    DrawGraph(0, 0, bgHandle, TRUE);          // 背景
    DrawGraph(600, 50, optionHandle, TRUE);   // オプション文字
    DrawGraph(200, 280, bgmHandle, TRUE);     // BGM文字
    DrawGraph(200, 480, seHandle, TRUE);      // SE文字
    DrawGraph(600, 250, gaugeHandle, TRUE);   // BGM用ゲージ
    DrawGraph(600, 450, gaugeHandle, TRUE);   // SE用ゲージ

    // 画面更新（裏画面を表画面に反映）
    ScreenFlip();
}

/// <summary>
/// 終了処理
/// 読み込んだ全ての画像を解放し、メモリリークを防止
/// </summary>
void OptionScene::Finalize()
{
    // 画像解放
    DeleteGraph(bgHandle);
    DeleteGraph(optionHandle);
    DeleteGraph(bgmHandle);
    DeleteGraph(seHandle);
    DeleteGraph(gaugeHandle);
}
