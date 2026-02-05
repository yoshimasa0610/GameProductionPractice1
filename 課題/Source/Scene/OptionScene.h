/// <summary>
/// オプションシーンヘッダー
/// オプション設定画面の管理を行う
/// </summary>

    #pragma once

// 前方宣言
class SceneManager;

/// <summary>
/// オプションシーンクラス
/// オプション画面の表示とキー入力による遷移を管理
/// Cキー: タイトルシーンに戻る
/// </summary>
class OptionScene
{
private:
    SceneManager* sceneManager;  // シーンマネージャーへの参照

    // 画像ハンドル
    int bgHandle;       // 背景画像ハンドル
    int optionHandle;   // オプション文字画像ハンドル
    int bgmHandle;      // BGM文字画像ハンドル
    int seHandle;       // SE文字画像ハンドル
    int gaugeHandle;    // ゲージ画像ハンドル

public:
    /// <summary>
    /// コンストラクタ
    /// シーンマネージャーへの参照を保存し、画像ハンドルを初期化
    /// </summary>
    /// <param name="manager">シーンマネージャーへのポインタ</param>
    OptionScene(SceneManager* manager);

    /// <summary>
    /// デストラクタ
    /// 特別な処理は行わない（Finalize()でリソース解放）
    /// </summary>
    ~OptionScene();

    /// <summary>
    /// 初期化処理
    /// オプション画面で使用する画像を読み込む
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// キー入力をチェックし、シーン遷移をリクエスト
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// 背景、オプション要素、ゲージを描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 終了処理
    /// 読み込んだ画像を解放し、メモリリークを防止
    /// </summary>
    void Finalize();
};
