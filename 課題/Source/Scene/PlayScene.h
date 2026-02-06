/// <summary>
/// プレイシーンヘッダー
/// ゲームプレイ画面の管理を行う
/// </summary>

#pragma once
#include "DxLib.h"

// 前方宣言
class SceneManager;

/// <summary>
/// キャラクターの種類を表す列挙型
/// </summary>
enum CharacterType
{
    CHARACTER_TYPE_HERO = 0,    // プレイヤーキャラクター
    CHARACTER_TYPE_ENEMY_A,     // 敵キャラクターA
    CHARACTER_TYPE_ENEMY_B,     // 敵キャラクターB

    CHARACTER_TYPE_MAX          // キャラクターの総数
};

/// <summary>
/// キャラクターデータ構造体
/// 位置と画像ハンドルを管理
/// </summary>
struct CharacterData
{
    VECTOR pos;     // キャラクターの位置（X, Y, Z座標）
    int handle;     // キャラクター画像のハンドル
};

/// <summary>
/// プレイシーンクラス
/// ゲームプレイ中の更新・描画・入力処理を管理
/// Cキー: タイトルシーンへ戻る
/// </summary>
class PlayScene
{
private:
    SceneManager* sceneManager;  // シーンマネージャーへの参照

    // 画像ハンドル
    int playBGHandle;                               // プレイ画面の背景画像ハンドル
    CharacterData characterData[CHARACTER_TYPE_MAX]; // 全キャラクターのデータ配列

    /// <summary>
    /// プレイシーンの初期化処理（内部用）
    /// </summary>
    void InitPlayScene();

    /// <summary>
    /// プレイシーンのリソース読み込み
    /// 画像ファイルを読み込む
    /// </summary>
    void LoadPlayScene();

    /// <summary>
    /// プレイシーンの開始処理
    /// キャラクターの初期位置を設定
    /// </summary>
    void StartPlayScene();

    /// <summary>
    /// プレイシーンの入力処理
    /// キー入力をチェックしてシーン遷移を管理
    /// </summary>
    void StepPlayScene();

    /// <summary>
    /// プレイシーンの更新処理
    /// ゲームロジックを実行
    /// </summary>
    void UpdatePlayScene();

    /// <summary>
    /// プレイシーンの描画処理
    /// 背景とキャラクターを描画
    /// </summary>
    void DrawPlayScene();

    /// <summary>
    /// プレイシーンの終了処理（内部用）
    /// リソースを解放
    /// </summary>
    void FinPlayScene();

public:
    /// <summary>
    /// コンストラクタ
    /// シーンマネージャーへの参照を保存し、変数を初期化
    /// </summary>
    /// <param name="manager">シーンマネージャーへのポインタ</param>
    PlayScene(SceneManager* manager);

    /// <summary>
    /// デストラクタ
    /// 特別な処理は行わない（Finalize()でリソース解放）
    /// </summary>
    ~PlayScene();

    /// <summary>
    /// 初期化処理
    /// リソースの読み込みと初期設定を実行
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新処理
    /// 入力処理とゲームロジックの更新を実行
    /// </summary>
    void Update();

    /// <summary>
    /// 描画処理
    /// 画面クリア、描画、画面更新を実行
    /// </summary>
    void Draw();

    /// <summary>
    /// 終了処理
    /// 読み込んだリソースを解放
    /// </summary>
    void Finalize();
};