#pragma once

// 前方宣言
class SceneManager;

/// <summary>
/// タイトルシーン
/// Zキー: プレイシーンへ遷移
/// Xキー: オプションシーンへ遷移
/// </summary>
class TitleScene
{
private:
    SceneManager* sceneManager;  // シーンマネージャーへの参照

public:
    TitleScene(SceneManager* manager);
    ~TitleScene();

    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize();

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
};