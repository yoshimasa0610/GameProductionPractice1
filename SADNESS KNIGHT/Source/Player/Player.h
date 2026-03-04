#pragma once

// プレイヤー描画/当たり判定の既定サイズ（ピクセル）
constexpr int PLAYER_WIDTH = 46;
constexpr int PLAYER_HEIGHT = 55;

// プレイヤーの状態
enum class PlayerState
{
    Idle,       // 待機
    Walk,       // 移動
    Jump,       // ジャンプ
    Fall,       // 落下
    Land,       // 着地
    UsingSkill, // スキル
    Healing,    // 回復中
    Dodging,    // 回避中
    DiveAttack, // 落下攻撃
};

// プレイヤーのデータ構造体
struct PlayerData
{
    // 位置情報
    float posX;
    float posY;
    float velocityX;
    float velocityY;
    float prevPosX;
    float prevPosY;

    // 状態
    PlayerState state;
    bool isFacingRight;
    bool isGrounded;
    int jumpCount;
    bool dropThrough = false;
    int dropTimer = 0;

    // ステータス
    int maxHP;
    int currentHP;
    int attackPower;
    int money;

    // 基礎ステータス
    int baseMaxHp;
    int baseMaxSlot;
    int basehealPower;

    // 現在ステータス
    int maxSlot;
    int usedSlot = 0;

    // 装備補正値
    int healPowerBonus;
    float damageTakenRate = 0.0f;
    float skillCountRate = 0.0f;
    float skillCooldownRate = 0.0f;
    int healCountBonus = 0;

    // パッシブアビリティ
    bool hasDoubleJump = false;
    bool hasDiveAttack = false;

    // 回復関連
    int healCount = 3;
    int maxHealCount = 3;
    bool healExecuted = false;

    // 回避関連
    bool isInvincible = false;
    int dodgeCooldown = 0;

    // ダッシュエフェクト関連
    bool showDashEffect = false;
    float dashEffectX = 0.0f;
    float dashEffectY = 0.0f;
    bool dashEffectFacingRight = true;

    // アニメーション
    int currentFrame;
    int animationCounter;

    // 落下攻撃関連
    bool isDiveAttacking = false;
    float diveAttackSpeed = 0.0f;
    int diveAttackDamage = 0;
};

// ===== 初期化・更新・描画 =====
// プレイヤーの初期化
void InitPlayer(float startX, float startY);

// プレイヤーのリソース読み込み
void LoadPlayer();

// プレイヤーの更新
void UpdatePlayer();

// プレイヤーの描画
void DrawPlayer();

// プレイヤーのリソース解放
void UnloadPlayer();

// ===== データ取得 =====
// プレイヤーデータ全体を取得
PlayerData& GetPlayerData();

// プレイヤーのX座標を取得
float GetPlayerPosX();

// プレイヤーのY座標を取得
float GetPlayerPosY();

// プレイヤーの座標を取得
void GetPlayerPos(float& outX, float& outY);

//
// プレイヤーの速度Xを取得
float GetPlayerVelocityX();

// プレイヤーの速度Yを取得
float GetPlayerVelocityY();

// プレイヤーの状態を取得
PlayerState GetPlayerState();

// プレイヤーが右を向いているか
bool IsPlayerFacingRight();

// プレイヤーが地面にいるか
bool IsPlayerGrounded();

// プレイヤーが生きているか
bool IsPlayerAlive();

// 現在のHPを取得
int GetPlayerHP();

// 最大HPを取得
int GetPlayerMaxHP();

// 攻撃力を取得
int GetPlayerAttack();

// HPにダメージを与える
void DamagePlayerHP(int damage);

// HPを回復する
void HealPlayerHP(int healAmount);

// ===== パッシブアビリティ =====
// 二段ジャンプを解放
void UnlockDoubleJump();

// 二段ジャンプが解放されているか
bool HasDoubleJump();

// ダイビングアタックを解放
void UnlockDiveAttack();

// ダイビングアタックが解放されているか
bool HasDiveAttack();

// キャットコンバット撃破時の解放処理
void OnCatCombatDefeated();

// ===== 回復関連 =====
// 回復を試みる（キー入力時）
void TryHeal();

// 残り回復回数を取得
int GetHealCount();

// 最大回復回数を取得
int GetMaxHealCount();

// ===== 回避関連 =====
// 回避を試みる（キー入力時）
void TryDodge();

// 無敵状態かどうかを取得
bool IsPlayerInvincible();