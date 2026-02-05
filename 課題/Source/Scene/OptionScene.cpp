#include "OptionScene.h"
#include "DxLib.h"
#include "../SceneManager.h"

OptionScene::OptionScene(SceneManager* manager)
    : sceneManager(manager)
    , bgHandle(-1)
    , optionHandle(-1)
    , bgmHandle(-1)
    , seHandle(-1)
    , gaugeHandle(-1)
{
}

OptionScene::~OptionScene()
{
}

void OptionScene::Initialize()
{
    // 画像読み込み
    bgHandle = LoadGraph(_T("Data/Option/BG.png"));
    optionHandle = LoadGraph(_T("Data/Option/Option.png"));
    bgmHandle = LoadGraph(_T("Data/Option/BGM.png"));
    seHandle = LoadGraph(_T("Data/Option/SE.png"));
    gaugeHandle = LoadGraph(_T("Data/Option/Gauge.png"));
}

void OptionScene::Update()
{
    // Cキーでタイトルへ戻る
    if (CheckHitKey(KEY_INPUT_C))
    {
        sceneManager->RequestSceneChange(SceneType::Title);
    }
}

void OptionScene::Draw()
{
    // 画面クリア
    ClearDrawScreen();

    // 画像描画（座標は仮。必要なら調整）
    DrawGraph(0, 0, bgHandle, TRUE);
    DrawGraph(0, 0, optionHandle, TRUE);
    DrawGraph(0, 0, bgmHandle, TRUE);
    DrawGraph(0, 0, seHandle, TRUE);
    DrawGraph(0, 0, gaugeHandle, TRUE);

    // 画面更新
    ScreenFlip();
}

void OptionScene::Finalize()
{
    // 画像解放
    DeleteGraph(bgHandle);
    DeleteGraph(optionHandle);
    DeleteGraph(bgmHandle);
    DeleteGraph(seHandle);
    DeleteGraph(gaugeHandle);
}
