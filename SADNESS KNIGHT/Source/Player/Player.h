#pragma once

// プレイヤーの状態
enum class PlayerState
{
    Idle,       // 待機
    Walk,       // 歩き
    Jump,       // ジャンプ
    Fall,       // 落下
    Skill1,     // スキル1使用中
    Skill2,     // スキル2使用中
    Skill3      // スキル3使用中
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
    int maxMP;
    int currentMP;
    int attackPower;
    int defense;
    int money;
    
    // スキル
    bool hasSkill1;
    bool hasSkill2;
    bool hasSkill3;
    int skill1MP;
    int skill2MP;
    int skill3MP;
    
    // アニメーション
    int currentFrame;
    int animationCounter;
    int mpRegenCounter;
};

// ===== 初期化・更新・描画 =====

// プレイヤーの初期化
// 引数: startX - 開始X座標, startY - 開始Y座標
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
// 戻り値: プレイヤーデータへの参照
PlayerData& GetPlayerData();

// 位置情報取得
// プレイヤーのX座標を取得
// 戻り値: X座標
float GetPlayerPosX();

// プレイヤーのY座標を取得
// 戻り値: Y座標
float GetPlayerPosY();

// プレイヤーの座標を取得
// 引数: outX - X座標の格納先, outY - Y座標の格納先
void GetPlayerPos(float& outX, float& outY);

// プレイヤーの速度Xを取得
// 戻り値: X方向の速度
float GetPlayerVelocityX();

// プレイヤーの速度Yを取得
// 戻り値: Y方向の速度
float GetPlayerVelocityY();

// 状態取得
// プレイヤーの状態を取得
// 戻り値: 現在の状態
PlayerState GetPlayerState();

// プレイヤーが右を向いているか
// 戻り値: 右向きの場合true
bool IsPlayerFacingRight();

// プレイヤーが地面にいるか
// 戻り値: 地面に接地している場合true
bool IsPlayerGrounded();

// プレイヤーが生きているか
// 戻り値: HP > 0の場合true
bool IsPlayerAlive();

// ステータス取得
// 現在のHPを取得
// 戻り値: 現在のHP
int GetPlayerHP();

// 最大HPを取得
// 戻り値: 最大HP
int GetPlayerMaxHP();

// 現在のMPを取得
// 戻り値: 現在のMP
int GetPlayerMP();

// 最大MPを取得
// 戻り値: 最大MP
int GetPlayerMaxMP();

// 攻撃力を取得
// 戻り値: 攻撃力
int GetPlayerAttack();

// 防御力を取得
// 戻り値: 防御力
int GetPlayerDefense();

// 所持金を取得
// 戻り値: 所持金
int GetPlayerMoney();

// スキル情報取得
// スキルが使用可能か
// 引数: skillNumber - スキル番号(1-3)
// 戻り値: 習得済みの場合true
bool CanUseSkill(int skillNumber);

// ===== データ操作 =====

// HPにダメージを与える
// 引数: damage - ダメージ量
void DamagePlayerHP(int damage);

// HPを回復する
// 引数: healAmount - 回復量
void HealPlayerHP(int healAmount);

// MPを消費する
// 引数: mpCost - 消費MP
// 戻り値: MPが足りて消費できた場合true
bool ConsumePlayerMP(int mpCost);

// MPを回復する
// 引数: recoverAmount - 回復量
void RecoverPlayerMP(int recoverAmount);

// お金を追加
// 引数: amount - 追加する金額
void AddPlayerMoney(int amount);

// お金を使用
// 引数: amount - 使用する金額
// 戻り値: お金が足りて使用できた場合true
bool SpendPlayerMoney(int amount);

// スキルを習得する
// 引数: skillNumber - スキル番号(1-3)
void UnlockSkill(int skillNumber);