#pragma once
#pragma once
#include <vector>
#include "../Animation/Animation.h"

struct MapChipData;

//============================================================
// 敵システム - チーム開発用
//============================================================

// 
// 【使い方】
// 1. SpawnEnemy(EnemyType::Slime, x, y) で敵を配置
// 2. UpdateEnemies() で自動更新（メインループで呼ばれる）
// 3. DrawEnemies() で自動描画（メインループで呼ばれる）
//
// 【新しい敵の追加方法】
// 1. EnemyType に追加
// 2. EnemyBase.cpp の g_enemyConfigs に設定追加
// 3. 必要ならラッパー関数作成（Slime.cpp 参照）
//
// 詳細は Source/Enemy/README.md を参照
//============================================================

// 敵のタイプ（新しい敵はここに追加）
enum class EnemyType
{
    Slime,              // スライム：低HP、遅い
    Cultists,           // カルティスト：汎用近接
    AssassinCultist,    // アサシン：素早い
    BigQuartist,        // ビッグクォーティスト：ボス級
    TwistedCaltis,      // ツイステッドカルティス：変則型
    Count               // 敵の総数（追加不要）
};

// 敵の設定データ（EnemyBase.cpp で設定）
struct EnemyConfig
{
    int maxHP;              // 最大HP
    int attackPower;        // 攻撃力
    float moveSpeed;        // 移動速度
    float width;            // 幅（ピクセル）
    float height;           // 高さ（ピクセル）
    float detectRange;      // プレイヤー検知範囲
    float attackRange;      // 攻撃範囲
    float attackDuration;   // 攻撃持続時間（秒）
    float attackCooldown;   // 攻撃クールダウン（秒）
    bool canJump;           // ジャンプ可能か
    float jumpPower;        // ジャンプ力
    const char* animationPath; // アニメーションパス（将来用）
};

// 敵のアニメーションセット
struct EnemyAnimations
{
    AnimationData idle;
    AnimationData move;
    AnimationData attack;
    AnimationData hurt;
    AnimationData die;
};

// 敵の実行時データ（システムが自動管理）
struct EnemyData

{
    bool active;            // アクティブフラグ
    EnemyType type;         // 敵タイプ

    // 位置・速度（システムが自動更新）
    float posX;
    float posY;
    float velocityX;
    float velocityY;

    // 状態（システムが自動管理）
    bool isFacingRight;     // 右向きか
    bool isAggro;           // 戦闘状態か
    bool isGrounded;        // 接地しているか
    
    // 徘徊AI用
    float patrolStartX;     // 徘徊開始位置X
    float patrolRange;      // 徘徊範囲（左右のブロック数）
    int patrolDirection;    // 徘徊方向（1=右、-1=左）
    bool hasLineOfSight;    // プレイヤーが視界内か

    // ステータス（ダメージ処理で変更可能）
    int maxHP;
    int currentHP;
    int attackPower;

    // AI パラメータ（通常は変更不要）
    float detectRange;
    float attackRange;
    float attackDuration;
    float attackCooldown;
    float attackTimer;
    float cooldownTimer;
    float moveSpeed;
    bool canJump;
    float jumpPower;
    
    // 攻撃状態
    bool isAttacking;       // 攻撃モーション再生中か
    int attackColliderId;   // 攻撃判定コライダー

    // 特殊AI状態（主にアサシン用）
    int behaviorPattern;    // 0:透明ワープ型, 1:突進型
    bool isInvisible;       // 透明化中か
    float specialTimer;     // 特殊行動タイマー

    // 死亡演出状態（主にアサシン用）
    bool isDying;           // 死亡演出中か
    bool deathAnimFinished; // 死亡アニメ終了済みか
    int deathBlinkTimer;    // 点滅タイマー

    // 描画・当たり判定（システムが自動管理）


    float width;
    float height;
    int colliderId;
    
    // アニメーション（システムが管理）
    EnemyAnimations* animations;
};


//============================================================
// 敵システム API（チームメンバー向け）
//============================================================

// 【初期化】ゲーム開始時に自動で呼ばれる
void InitEnemySystem();

// 【更新】毎フレーム自動で呼ばれる（触らない）
void UpdateEnemies();

// 【描画】毎フレーム自動で呼ばれる（触らない）
void DrawEnemies();

// 【クリア】ステージ切り替え時に自動で呼ばれる
void ClearEnemies();

//============================================================
// 敵の生成・削除（これを使う！）
//============================================================

// 【敵を配置】
// 使用例: int idx = SpawnEnemy(EnemyType::Slime, 100.0f, 200.0f);
// 戻り値: 敵のインデックス（削除時に使用）
int SpawnEnemy(EnemyType type, float x, float y);

// 【敵を削除】
// 使用例: DespawnEnemy(enemyIndex);
void DespawnEnemy(int index);

//============================================================
// データアクセス（上級者向け）
//============================================================

// 敵データを取得（HP変更などに使用）
// 使用例: 
//   EnemyData* enemy = GetEnemy(index);
//   if (enemy) enemy->currentHP -= 10;
EnemyData* GetEnemy(int index);
int GetEnemyCount();
EnemyData* FindEnemyByColliderId(int colliderId);

// 敵の設定を取得（通常は使用しない）
const EnemyConfig& GetEnemyConfig(EnemyType type);

//============================================================
// アニメーション管理（システム内部用）
//============================================================

// 敵のアニメーションを読み込み
EnemyAnimations* LoadEnemyAnimations(EnemyType type);

// 敵のアニメーションを解放
void UnloadEnemyAnimations(EnemyAnimations* anims);

// CSVから敵を読み込む
void LoadEnemiesFromCSV(const char* stageName);