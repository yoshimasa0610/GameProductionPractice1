/// <summary>
/// タイトルシーンヘッダー
/// タイトル画面の表示と入力処理を管理
/// </summary>

#pragma once

// 前方宣言（循環参照を防ぐため）
class SceneManager;

/// <summary>
/// タイトルシーンクラス
/// タイトル画面の表示、キー入力によるシーン遷移を管理
/// Zキー: プレイシーンへ遷移
/// Xキー: オプションシーンへ遷移
/// </summary>
class TitleScene
{
private:
    SceneManager* sceneManager;  // シーンマネージャーへの参照（シーン遷移に使用）

    // 画像ハンドル
    int bgHandle;       // 背景画像ハンドル
    int titleHandle;    // タイトルロゴ画像ハンドル

public:
    /// <summary>
    /// コンストラクタ
    /// シーンマネージャーへの参照を保存し、画像ハンドルを初期化
    /// </summary>
    /// <param name="manager">シーンマネージャーへのポインタ</param>
    TitleScene(SceneManager* manager);

    /// <summary>
    /// デストラクタ
    /// 特別な処理は行わない（Finalize()でリソース解放）
    /// </summary>
    ~TitleScene();

    /// <summary>
    /// 初期化処理
    /// タイトル画面で使用する画像を読み込む
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// キー入力をチェックし、シーン遷移をリクエスト
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// 背景、タイトルロゴ、操作説明テキストを描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 終了処理
    /// 読み込んだ画像を解放し、メモリリークを防止
    /// </summary>
    void Finalize();
};