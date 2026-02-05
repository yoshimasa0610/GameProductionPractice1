#pragma once

// 前方宣言
class SceneManager;

/// <summary>
/// オプションシーン
/// Cキーでタイトルシーンに戻る
/// </summary>
class OptionScene
{
private:
    SceneManager* sceneManager;

    // 画像ハンドル
    int bgHandle;
    int optionHandle;
    int bgmHandle;
    int seHandle;
    int gaugeHandle;

public:
    OptionScene(SceneManager* manager);
    ~OptionScene();

    void Initialize();
    void Update();
    void Draw();
    void Finalize();
};
