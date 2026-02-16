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

/// <summary>
/// プレイヤーのデータ構造体
/// </summary>
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

/// <summary>
/// プレイヤーの初期化
/// </summary>
void InitPlayer(float startX, float startY);

/// <summary>
/// プレイヤーのリソース読み込み
/// </summary>
void LoadPlayer();

/// <summary>
/// プレイヤーの更新
/// </summary>
void UpdatePlayer();

/// <summary>
/// プレイヤーの描画
/// </summary>
void DrawPlayer();

/// <summary>
/// プレイヤーのリソース解放
/// </summary>
void UnloadPlayer();

/// <summary>
/// プレイヤーデータを取得
/// </summary>
PlayerData& GetPlayerData();

/// <summary>
/// HPにダメージを与える
/// </summary>
/// <param name="damage">ダメージ量</param>
void DamagePlayerHP(int damage);

/// <summary>
/// HPを回復する
/// </summary>
/// <param name="healAmount">回復量</param>
void HealPlayerHP(int healAmount);

/// <summary>
/// MPを消費する
/// </summary>
/// <param name="mpCost">消費MP</param>
/// <returns>MPが足りて消費できた場合true</returns>
bool ConsumePlayerMP(int mpCost);

/// <summary>
/// MPを回復する
/// </summary>
/// <param name="recoverAmount">回復量</param>
void RecoverPlayerMP(int recoverAmount);

/// <summary>
/// お金を追加
/// </summary>
/// <param name="amount">追加する金額</param>
void AddPlayerMoney(int amount);

/// <summary>
/// お金を使用
/// </summary>
/// <param name="amount">使用する金額</param>
/// <returns>お金が足りて使用できた場合true</returns>
bool SpendPlayerMoney(int amount);

/// <summary>
/// スキルを習得する
/// </summary>
/// <param name="skillNumber">スキル番号(1-3)</param>
void UnlockSkill(int skillNumber);