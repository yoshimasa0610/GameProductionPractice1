#pragma once

/// <summary>
/// シーンの種類
/// </summary>
enum class SceneType
{
    Title,      // タイトルシーン
    Play,       // プレイシーン (他のメンバーが実装)
    Option,     // オプションシーン (他のメンバーが実装)
    None        // シーンなし
};

// 前方宣言
class TitleScene;
class PlayScene;
class OptionScene;

/// <summary>
/// シーン管理クラス
/// シーンの生成・削除・遷移を管理する
/// </summary>
class SceneManager
{
private:
    SceneType currentSceneType;     // 現在のシーン種類
    SceneType nextSceneType;        // 次のシーン種類
    bool isSceneChanging;           // シーン遷移中フラグ

    // 各シーンのポインタ (実際に使用されているシーンのみ非NULL)
    TitleScene* titleScene;
    PlayScene* playScene;
    OptionScene* optionScene;

    /// <summary>
    /// 現在のシーンを削除する
    /// </summary>
    void DeleteCurrentScene();

    /// <summary>
    /// 指定されたシーンを生成・初期化する
    /// </summary>
    void CreateAndInitializeScene(SceneType sceneType);

    /// <summary>
    /// シーンを変更する
    /// </summary>
    void ChangeScene();

public:
    SceneManager();
    ~SceneManager();

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="firstScene">最初に起動するシーン</param>
    void Initialize(SceneType firstScene);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// </summary>
    void Draw();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// シーン遷移をリクエストする
    /// </summary>
    /// <param name="nextScene">遷移先のシーン</param>
    void RequestSceneChange(SceneType nextScene);
};