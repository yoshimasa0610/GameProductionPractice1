#pragma once

// プレイヤーの状態
enum class PlayerState
{
    Idle,       // 待機
    Walk,       // 歩き
    Jump,       // ジャンプ
    Fall,       // 落下
    UsingSkill,    // ← スキルは一つに統一します
};

// プレイヤーのデータ構造体
struct PlayerData
{
    // 位置情報
    float posX;
    float posY;
    float velocityX;
    float velocityY;

    // 状態
    PlayerState state;
    bool isFacingRight;
    bool isGrounded;
    int jumpCount;

    // ステータス
    int maxHP;
    int currentHP;
    int attackPower;
    int money;

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

// 所持金を取得
int GetPlayerMoney();

// スキルが使用可能か
bool CanUseSkill(int skillNumber);

// スキルの残り使用回数を取得
int GetSkillCount(int skillNumber);

// スキルの最大使用回数を取得
int GetSkillMaxCount(int skillNumber);

// ===== データ操作 =====

// HPにダメージを与える
void DamagePlayerHP(int damage);

// HPを回復する
void HealPlayerHP(int healAmount);

// お金を追加
void AddPlayerMoney(int amount);

// お金を使用
bool SpendPlayerMoney(int amount);

// スキルを習得する
void UnlockSkill(int skillNumber);

// スキルの使用回数を回復する
void RestoreSkillCount(int skillNumber, int amount);

// スキルの使用回数を最大まで回復する
void RestoreSkillCountToMax(int skillNumber);