/// <summary>
/// シーンマネージャー実装
/// シーンの生成・削除・遷移ロジックを実装
/// </summary>

#include "SceneManager.h"
#include "TitleScene.h"
#include "Scene/PlayScene.h"
#include "Scene/OptionScene.h"

/// <summary>
/// コンストラクタ
/// 全てのメンバ変数を初期状態に設定
/// </summary>
SceneManager::SceneManager()
    : currentSceneType(SceneType::None)
    , nextSceneType(SceneType::None)
    , isSceneChanging(false)
    , titleScene(nullptr)
    , playScene(nullptr)
    , optionScene(nullptr)
{
}

/// <summary>
/// デストラクタ
/// Finalize()を呼び出して確実にリソースを解放
/// </summary>
SceneManager::~SceneManager()
{
    Finalize();
}

/// <summary>
/// 初期化処理
/// 指定されたシーンを最初のシーンとして生成・開始する
/// </summary>
/// <param name="firstScene">最初に起動するシーンの種類</param>
void SceneManager::Initialize(SceneType firstScene)
{
    nextSceneType = firstScene;
    ChangeScene();
}

/// <summary>
/// 更新処理
/// シーン遷移フラグをチェックし、必要に応じてシーン遷移を実行
/// その後、現在のシーンの更新処理を呼び出す
/// </summary>
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

    case SceneType::Play:
        if (playScene != nullptr)
        {
            playScene->Update();
        }
        break;

    case SceneType::Option:
        if (optionScene != nullptr)
        {
            optionScene->Update();
        }
        break;

    default:
        break;
    }
}

/// <summary>
/// 描画処理
/// 現在アクティブなシーンの描画処理を呼び出す
/// </summary>
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

    case SceneType::Play:
        if (playScene != nullptr)
        {
            playScene->Draw();
        }
        break;

    case SceneType::Option:
        if (optionScene != nullptr)
        {
            optionScene->Draw();
        }
        break;

    default:
        break;
    }
}

/// <summary>
/// 終了処理
/// 全てのシーンを削除し、リソースを解放する
/// </summary>
void SceneManager::Finalize()
{
    DeleteCurrentScene();
}

/// <summary>
/// シーン遷移をリクエストする
/// 各シーンから呼び出され、次のフレームでシーン遷移が実行される
/// </summary>
/// <param name="nextScene">遷移先のシーンの種類</param>
void SceneManager::RequestSceneChange(SceneType nextScene)
{
    nextSceneType = nextScene;
    isSceneChanging = true;
}

/// <summary>
/// 現在のシーンを削除する
/// 各シーンのFinalize()を呼び出してからメモリを解放
/// </summary>
void SceneManager::DeleteCurrentScene()
{
    // 現在のシーンを削除
    if (titleScene != nullptr)
    {
        titleScene->Finalize();
        delete titleScene;
        titleScene = nullptr;
    }

    if (playScene != nullptr)
    {
        playScene->Finalize();
        delete playScene;
        playScene = nullptr;
    }

    if (optionScene != nullptr)
    {
        optionScene->Finalize();
        delete optionScene;
        optionScene = nullptr;
    }
}

/// <summary>
/// 指定されたシーンを生成し、Initialize()を呼び出す
/// </summary>
/// <param name="sceneType">生成するシーンの種類</param>
void SceneManager::CreateAndInitializeScene(SceneType sceneType)
{
    switch (sceneType)
    {
    case SceneType::Title:
        titleScene = new TitleScene(this);
        titleScene->Initialize();
        break;

    case SceneType::Play:
        playScene = new PlayScene(this);
        playScene->Initialize();
        break;

    case SceneType::Option:
        optionScene = new OptionScene(this);
        optionScene->Initialize();
        break;

    default:
        break;
    }
}

/// <summary>
/// シーンを変更する
/// 現在のシーンを終了・削除し、新しいシーンを生成・初期化する
/// </summary>
void SceneManager::ChangeScene()
{
    // 現在のシーンを終了・削除
    DeleteCurrentScene();

    // 新しいシーンを生成・初期化
    currentSceneType = nextSceneType;
    CreateAndInitializeScene(nextSceneType);

    nextSceneType = SceneType::None;
}