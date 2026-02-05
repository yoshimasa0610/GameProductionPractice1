#include "SceneManager.h"
#include "TitleScene.h"
// 他のメンバーが実装する際に以下を追加してください
// #include "PlayScene.h"
// #include "OptionScene.h"

SceneManager::SceneManager()
    : currentSceneType(SceneType::None)
    , nextSceneType(SceneType::None)
    , isSceneChanging(false)
    , titleScene(nullptr)
    // , playScene(nullptr)
    // , optionScene(nullptr)
{
}

SceneManager::~SceneManager()
{
    Finalize();
}

void SceneManager::Initialize(SceneType firstScene)
{
    nextSceneType = firstScene;
    ChangeScene();
}

void SceneManager::Update()
{
    // シーン遷移処理
    if (isSceneChanging)
    {
        ChangeScene();
        isSceneChanging = false;
    }

    // 現在のシーンを更新
    switch (currentSceneType)
    {
    case SceneType::Title:
        if (titleScene != nullptr)
        {
            titleScene->Update();
        }
        break;

    // 他のメンバーが実装する際に以下のコメントを外してください
    //case SceneType::Play:
    //    if (playScene != nullptr)
    //    {
    //        playScene->Update();
    //    }
    //    break;

    //case SceneType::Option:
    //    if (optionScene != nullptr)
    //    {
    //        optionScene->Update();
    //    }
    //    break;

    default:
        break;
    }
}

void SceneManager::Draw()
{
    // 現在のシーンを描画
    switch (currentSceneType)
    {
    case SceneType::Title:
        if (titleScene != nullptr)
        {
            titleScene->Draw();
        }
        break;

    // 他のメンバーが実装する際に以下のコメントを外してください
    //case SceneType::Play:
    //    if (playScene != nullptr)
    //    {
    //        playScene->Draw();
    //    }
    //    break;

    //case SceneType::Option:
    //    if (optionScene != nullptr)
    //    {
    //        optionScene->Draw();
    //    }
    //    break;

    default:
        break;
    }
}

void SceneManager::Finalize()
{
    DeleteCurrentScene();
}

void SceneManager::RequestSceneChange(SceneType nextScene)
{
    nextSceneType = nextScene;
    isSceneChanging = true;
}

void SceneManager::DeleteCurrentScene()
{
    // 現在のシーンを削除
    if (titleScene != nullptr)
    {
        titleScene->Finalize();
        delete titleScene;
        titleScene = nullptr;
    }

    // 他のメンバーが実装する際に以下のコメントを外してください
    //if (playScene != nullptr)
    //{
    //    playScene->Finalize();
    //    delete playScene;
    //    playScene = nullptr;
    //}

    //if (optionScene != nullptr)
    //{
    //    optionScene->Finalize();
    //    delete optionScene;
    //    optionScene = nullptr;
    //}
}

void SceneManager::CreateAndInitializeScene(SceneType sceneType)
{
    switch (sceneType)
    {
    case SceneType::Title:
        titleScene = new TitleScene(this);
        titleScene->Initialize();
        break;

    // 他のメンバーが実装する際に以下のコメントを外してください
    //case SceneType::Play:
    //    playScene = new PlayScene(this);
    //    playScene->Initialize();
    //    break;

    //case SceneType::Option:
    //    optionScene = new OptionScene(this);
    //    optionScene->Initialize();
    //    break;

    default:
        break;
    }
}

void SceneManager::ChangeScene()
{
    // 現在のシーンを終了・削除
    DeleteCurrentScene();

    // 新しいシーンを生成・初期化
    currentSceneType = nextSceneType;
    CreateAndInitializeScene(nextSceneType);

    nextSceneType = SceneType::None;
}