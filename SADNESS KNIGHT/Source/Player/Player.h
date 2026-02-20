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
};

// プレイヤーのデータ構造体
struct PlayerData
{
    // 位置情報
    float posX;
    float posY;
    float velocityX;
    float velocityY;
	float prevPosX;// 前フレームの位置（衝突処理などで必要）
	float prevPosY;// 前フレームの位置（衝突処理などで必要）

    // 状態
    PlayerState state;
    bool isFacingRight;
    bool isGrounded;
    int jumpCount;
	bool dropThrough = false;// すり抜け中かどうか(特定のブロックをすり抜けるのに必要)
	int dropTimer = 0;// すり抜けタイマー（フレーム数）

    // ステータス
    int maxHP;
    int currentHP;
    int attackPower;
    int money;

    // ===== 追加（基礎値 + 拡張値管理用） =====
    // 基礎ステータス（装備補正なし）
    int baseMaxHp; // なんの補正もないときのHP仕様書で言うなら150。maxHPを装備補正込みにします
    int baseMaxSlot; // 装備を行うために必要なスロットのベース。
    int basehealPower; // 基礎回復値。healPowerBonusがあるなら追加で加算していく感じ

    // 現在ステータス（装備補正込み）
    int maxSlot;

    // 装備補正値
    int healPowerBonus; //回復量の増加

    // ===== バフ倍率（装備・パッシブ結果）=====
    float damageTakenRate = 0.0f;     // 被ダメ倍率（-0.2 = 20%軽減）
    float skillCountRate = 0.0f;      // スキル回数倍率
    float skillCooldownRate = 0.0f;   // リキャ短縮
    int healCountBonus = 0;           // 回復回数+

    // アニメーション
    int currentFrame;
    int animationCounter;
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