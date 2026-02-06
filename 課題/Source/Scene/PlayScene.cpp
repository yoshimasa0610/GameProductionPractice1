/// <summary>
/// プレイシーン実装
/// ゲームプレイ中の処理を実装
/// </summary>

#include "PlayScene.h"
#include "../SceneManager.h"
#include <DxLib.h>

/// <summary>
/// コンストラクタ
/// シーンマネージャーへの参照を保存し、全ての変数を初期化
/// </summary>
/// <param name="manager">シーンマネージャーへのポインタ</param>
PlayScene::PlayScene(SceneManager* manager)
    : sceneManager(manager)
    , playBGHandle(-1)  // -1は未読み込み状態
{
    // 全キャラクターデータを初期化
    for (int i = 0; i < CHARACTER_TYPE_MAX; i++)
    {
        characterData[i].handle = -1;                  // 画像ハンドルを無効化
        characterData[i].pos = VGet(0.0f, 0.0f, 0.0f); // 位置を原点に設定
    }
}

/// <summary>
/// デストラクタ
/// 特別な処理は行わない（Finalize()でリソース解放）
/// </summary>
PlayScene::~PlayScene()
{
}

/// <summary>
/// 初期化処理
/// リソースの読み込みと初期設定を順次実行
/// </summary>
void PlayScene::Initialize()
{
    InitPlayScene();    // 初期化
    LoadPlayScene();    // リソース読み込み
    StartPlayScene();   // 開始処理
}

/// <summary>
/// 更新処理
/// 入力処理とゲームロジックの更新を実行
/// </summary>
void PlayScene::Update()
{
    StepPlayScene();    // 入力処理
    UpdatePlayScene();  // ゲームロジック更新
}

/// <summary>
/// 描画処理
/// 画面クリア、ゲーム描画、画面更新を実行
/// </summary>
void PlayScene::Draw()
{
    ClearDrawScreen();  // 画面クリア
    DrawPlayScene();    // ゲーム描画
    ScreenFlip();       // 裏画面を表画面に反映
}

/// <summary>
/// 終了処理
/// リソースを解放してメモリリークを防止
/// </summary>
void PlayScene::Finalize()
{
    FinPlayScene();
}

/// <summary>
/// プレイシーンの初期化処理（内部用）
/// 必要に応じて追加の初期化を実装
/// </summary>
void PlayScene::InitPlayScene()
{
    // 初期化処理（必要に応じて実装）
}

/// <summary>
/// プレイシーンのリソース読み込み
/// 背景とキャラクターの画像をファイルから読み込む
/// </summary>
void PlayScene::LoadPlayScene()
{
    // 背景画像の読み込み
    playBGHandle = LoadGraph(_T("Data/Play/BG.png"));

    // プレイヤーキャラクター画像の読み込み
    characterData[CHARACTER_TYPE_HERO].handle =
        LoadGraph(_T("Data/Player/Player.png"));

    // 敵キャラクターA画像の読み込み
    characterData[CHARACTER_TYPE_ENEMY_A].handle =
        LoadGraph(_T("Data/Enemy/DragonFly.png"));

    // 敵キャラクターB画像の読み込み
    characterData[CHARACTER_TYPE_ENEMY_B].handle =
        LoadGraph(_T("Data/Enemy/Flies.png"));
}

/// <summary>
/// プレイシーンの開始処理
/// 全キャラクターの初期位置を設定
/// </summary>
void PlayScene::StartPlayScene()
{
    // プレイヤーキャラクターの初期位置
    characterData[CHARACTER_TYPE_HERO].pos =
        VGet(100.0f, 100.0f, 0.0f);

    // 敵キャラクターAの初期位置
    characterData[CHARACTER_TYPE_ENEMY_A].pos =
        VGet(800.0f, 200.0f, 0.0f);

    // 敵キャラクターBの初期位置
    characterData[CHARACTER_TYPE_ENEMY_B].pos =
        VGet(900.0f, 600.0f, 0.0f);
}

/// <summary>
/// プレイシーンの入力処理
/// Cキーが押されたらタイトルシーンへ遷移をリクエスト
/// </summary>
void PlayScene::StepPlayScene()
{
    // Cキーでタイトルへ戻る
    if (CheckHitKey(KEY_INPUT_C))
    {
        sceneManager->RequestSceneChange(SceneType::Title);
    }
}

/// <summary>
/// プレイシーンの更新処理
/// ゲームロジックの更新を実装（現在は未実装）
/// </summary>
    void PlayScene::UpdatePlayScene()
{
    // ゲームロジックの更新処理（必要に応じて実装）
}

/// <summary>
/// プレイシーンの描画処理
/// 背景と全キャラクターを画面に描画
/// </summary>
void PlayScene::DrawPlayScene()
{
    // 背景を描画
    if (playBGHandle != -1)  // 読み込み成功時のみ描画
    {
        DrawGraph(0, 0, playBGHandle, TRUE);
    }

    // 全キャラクターを描画
    for (int i = 0; i < CHARACTER_TYPE_MAX; i++)
    {
        if (characterData[i].handle != -1)  // 読み込み成功時のみ描画
        {
            DrawGraph(
                (int)characterData[i].pos.x,    // X座標
                (int)characterData[i].pos.y,    // Y座標
                characterData[i].handle,        // 画像ハンドル
                TRUE                            // 透過処理有効
            );
        }
    }
}

/// <summary>
/// プレイシーンの終了処理（内部用）
/// 読み込んだ全ての画像を解放
/// </summary>
void PlayScene::FinPlayScene()
{
    // 背景画像を削除
    if (playBGHandle != -1)
    {
        DeleteGraph(playBGHandle);
        playBGHandle = -1;  // ハンドルを無効化
    }

    // 全キャラクター画像を削除
    for (int i = 0; i < CHARACTER_TYPE_MAX; i++)
    {
        if (characterData[i].handle != -1)
        {
            DeleteGraph(characterData[i].handle);
            characterData[i].handle = -1;  // ハンドルを無効化
        }
    }
}
