#include "TitleScene.h"
#include "SceneManager.h"
#include "DxLib.h"

TitleScene::TitleScene(SceneManager* manager)
    : sceneManager(manager)
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Initialize()
{
    // タイトルシーンの初期化処理
    // 必要に応じて画像の読み込みなどを行う
}

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

    // タイトルシーンの更新処理
}

void TitleScene::Draw()
{
    // 背景をクリア
    ClearDrawScreen();

    // タイトル表示（_T マクロで文字列をTCHAR型に変換）
    DrawString(320, 200, _T("===== GAME TITLE ====="), GetColor(255, 255, 255));
    DrawString(280, 280, _T("Press Z Key : Start Game"), GetColor(255, 255, 0));
    DrawString(280, 320, _T("Press X Key : Option"), GetColor(255, 255, 0));

    // 画面の更新
    ScreenFlip();
}

void TitleScene::Finalize()
{
    // タイトルシーンの終了処理
    // 必要に応じて画像の解放などを行う
}