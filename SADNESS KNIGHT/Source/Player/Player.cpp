#include "Player.h"
#include "../Input/Input.h"
#include "DxLib.h"
#include <string.h>

// プレイヤーのパラメータ定数
namespace
{
    const float MOVE_SPEED = 5.0f;          // 移動速度
    const float JUMP_POWER = 15.0f;         // ジャンプ力
    const float GRAVITY = 0.8f;             // 重力
    const float MAX_FALL_SPEED = 20.0f;     // 最大落下速度
    const int MAX_JUMP_COUNT = 2;           // 最大ジャンプ回数（二段ジャンプ）
    const float GROUND_Y = 600.0f;          // 地面のY座標

    // アニメーション設定
    const int IDLE_FRAME_COUNT = 10;        // アイドルアニメーションのフレーム数
    const int IDLE_ANIM_SPEED = 8;          // アニメーション速度（数値が大きいほど遅い）
}

// プレイヤーデータとアニメーション画像
namespace
{
    PlayerData playerData;
    int idleImages[IDLE_FRAME_COUNT];   // アイドルアニメーション画像
}

// 内部関数の宣言
namespace
{
    void ProcessInput();
    void ProcessMovement();
    void ProcessSkills();
    void UpdatePhysics();
    void UpdateState();
    void UpdateAnimation();
    void ExecuteJump();
    void UseSkill1();
    void UseSkill2();
    void UseSkill3();
}

// プレイヤーの初期化
void InitPlayer(float startX, float startY)
{
    // 位置情報
    playerData.posX = startX;
    playerData.posY = startY;
    playerData.velocityX = 0.0f;
    playerData.velocityY = 0.0f;
    
    // 状態
    playerData.state = PlayerState::Idle;
    playerData.isFacingRight = true;
    playerData.isGrounded = false;
    playerData.jumpCount = 0;
    
    // ステータス
    playerData.maxHP = 100;
    playerData.currentHP = 100;
    playerData.attackPower = 10;
    playerData.money = 0;
    
    // スキル
    playerData.hasSkill1 = true;
    playerData.hasSkill2 = false;
    playerData.hasSkill3 = false;
    
    // スキル使用回数の設定
    playerData.skill1Count = -1;        // スキル1は無限使用可能
    playerData.skill2Count = 5;         // スキル2は5回まで
    playerData.skill3Count = 3;         // スキル3は3回まで
    playerData.skill1MaxCount = -1;     // 無限
    playerData.skill2MaxCount = 5;      // 最大5回
    playerData.skill3MaxCount = 3;      // 最大3回
    
    // アニメーション
    playerData.currentFrame = 0;
    playerData.animationCounter = 0;
}

// プレイヤーのリソース読み込み
void LoadPlayer()
{
    LoadDivGraph(
        "Data/Player/Idle.png",
        IDLE_FRAME_COUNT,
        IDLE_FRAME_COUNT,
        1,
        64,
        64,
        idleImages
    );
}

// プレイヤーの更新
void UpdatePlayer()
{
    // プレイヤーが生きている場合のみ更新
    if (playerData.currentHP <= 0)
    {
        return;
    }

    ProcessInput();
    UpdatePhysics();
    UpdateState();
    UpdateAnimation();
}

// プレイヤーの描画
void DrawPlayer()
{
    int drawX = static_cast<int>(playerData.posX);
    int drawY = static_cast<int>(playerData.posY);

    // アイドルアニメーションの画像が読み込まれている場合
    if (playerData.state == PlayerState::Idle && idleImages[0] != -1)
    {
        if (playerData.isFacingRight)
        {
            DrawRotaGraph(drawX, drawY, 1.0, 0.0, idleImages[playerData.currentFrame], TRUE);
        }
        else
        {
            DrawTurnGraph(drawX, drawY, idleImages[playerData.currentFrame], TRUE);
        }
    }
    else
    {
        // デバッグ用の簡易描画
        unsigned int color = GetColor(255, 255, 255);
        DrawBox(drawX - 16, drawY - 32, drawX + 16, drawY + 32, color, TRUE);

        if (playerData.isFacingRight)
        {
            DrawTriangle(drawX + 16, drawY, drawX + 8, drawY - 8, drawX + 8, drawY + 8, GetColor(255, 0, 0), TRUE);
        }
        else
        {
            DrawTriangle(drawX - 16, drawY, drawX - 8, drawY - 8, drawX - 8, drawY + 8, GetColor(255, 0, 0), TRUE);
        }
    }

    // デバッグ情報表示
    DrawFormatString(10, 10, GetColor(255, 255, 255), "Position: (%.1f, %.1f)", playerData.posX, playerData.posY);
    DrawFormatString(10, 30, GetColor(255, 100, 100), "HP: %d / %d", playerData.currentHP, playerData.maxHP);
    DrawFormatString(10, 50, GetColor(200, 255, 200), "Money: %d", playerData.money);
    DrawFormatString(10, 70, GetColor(255, 255, 255), "ATK: %d", playerData.attackPower);
    DrawFormatString(10, 90, GetColor(255, 255, 255), "Grounded: %s", playerData.isGrounded ? "YES" : "NO");
    
    // スキル使用回数表示
    if (playerData.skill1Count == -1)
    {
        DrawFormatString(10, 110, GetColor(150, 255, 255), "Skill1: Infinite");
    }
    else
    {
        DrawFormatString(10, 110, GetColor(150, 255, 255), "Skill1: %d", playerData.skill1Count);
    }
    DrawFormatString(10, 130, GetColor(150, 255, 255), "Skill2: %d / %d", playerData.skill2Count, playerData.skill2MaxCount);
    DrawFormatString(10, 150, GetColor(150, 255, 255), "Skill3: %d / %d", playerData.skill3Count, playerData.skill3MaxCount);
    
    // 状態表示
    const char* stateStr = "UNKNOWN";
    switch (playerData.state)
    {
    case PlayerState::Idle:   stateStr = "IDLE";   break;
    case PlayerState::Walk:   stateStr = "WALK";   break;
    case PlayerState::Jump:   stateStr = "JUMP";   break;
    case PlayerState::Fall:   stateStr = "FALL";   break;
    case PlayerState::Skill1: stateStr = "SKILL1"; break;
    case PlayerState::Skill2: stateStr = "SKILL2"; break;
    case PlayerState::Skill3: stateStr = "SKILL3"; break;
    }
    DrawFormatString(10, 170, GetColor(255, 255, 0), "State: %s", stateStr);

    // プレイヤーが死んでいる場合
    if (playerData.currentHP <= 0)
    {
        DrawFormatString(700, 400, GetColor(255, 0, 0), "GAME OVER");
    }
}

// プレイヤーのリソース解放
void UnloadPlayer()
{
    for (int i = 0; i < IDLE_FRAME_COUNT; i++)
    {
        if (idleImages[i] != -1)
        {
            DeleteGraph(idleImages[i]);
            idleImages[i] = -1;
        }
    }
}

// ===== データ取得関数の実装 =====

// プレイヤーデータ全体を取得
PlayerData& GetPlayerData()
{
    return playerData;
}

// 位置情報取得
// プレイヤーのX座標を取得
float GetPlayerPosX()
{
    return playerData.posX;
}

// プレイヤーのY座標を取得
float GetPlayerPosY()
{
    return playerData.posY;
}

// プレイヤーの座標を取得
void GetPlayerPos(float& outX, float& outY)
{
    outX = playerData.posX;
    outY = playerData.posY;
}

// プレイヤーの速度Xを取得
float GetPlayerVelocityX()
{
    return playerData.velocityX;
}

// プレイヤーの速度Yを取得
float GetPlayerVelocityY()
{
    return playerData.velocityY;
}

// 状態取得
// プレイヤーの状態を取得
PlayerState GetPlayerState()
{
    return playerData.state;
}

// プレイヤーが右を向いているか
bool IsPlayerFacingRight()
{
    return playerData.isFacingRight;
}

// プレイヤーが地面にいるか
bool IsPlayerGrounded()
{
    return playerData.isGrounded;
}

// プレイヤーが生きているか
bool IsPlayerAlive()
{
    return playerData.currentHP > 0;
}

// ステータス取得
// 現在のHPを取得
int GetPlayerHP()
{
    return playerData.currentHP;
}

// 最大HPを取得
int GetPlayerMaxHP()
{
    return playerData.maxHP;
}

// 攻撃力を取得
int GetPlayerAttack()
{
    return playerData.attackPower;
}

// 所持金を取得
int GetPlayerMoney()
{
    return playerData.money;
}

// スキル情報取得
// スキルが使用可能か
bool CanUseSkill(int skillNumber)
{
    switch (skillNumber)
    {
    case 1: 
        return playerData.hasSkill1 && (playerData.skill1Count == -1 || playerData.skill1Count > 0);
    case 2: 
        return playerData.hasSkill2 && playerData.skill2Count > 0;
    case 3: 
        return playerData.hasSkill3 && playerData.skill3Count > 0;
    default: 
        return false;
    }
}

// スキルの残り使用回数を取得
int GetSkillCount(int skillNumber)
{
    switch (skillNumber)
    {
    case 1: return playerData.skill1Count;
    case 2: return playerData.skill2Count;
    case 3: return playerData.skill3Count;
    default: return 0;
    }
}

// スキルの最大使用回数を取得
int GetSkillMaxCount(int skillNumber)
{
    switch (skillNumber)
    {
    case 1: return playerData.skill1MaxCount;
    case 2: return playerData.skill2MaxCount;
    case 3: return playerData.skill3MaxCount;
    default: return 0;
    }
}

// ===== データ操作関数の実装 =====

// HPにダメージを与える
void DamagePlayerHP(int damage)
{
    playerData.currentHP -= damage;
    if (playerData.currentHP < 0) playerData.currentHP = 0;
    printfDx("Player damaged! HP: %d / %d\n", playerData.currentHP, playerData.maxHP);
}

// HPを回復する
void HealPlayerHP(int healAmount)
{
    playerData.currentHP += healAmount;
    if (playerData.currentHP > playerData.maxHP) playerData.currentHP = playerData.maxHP;
    printfDx("Player healed! HP: %d / %d\n", playerData.currentHP, playerData.maxHP);
}

// お金を追加
void AddPlayerMoney(int amount)
{
    playerData.money += amount;
    printfDx("Money gained: +%d (Total: %d)\n", amount, playerData.money);
}

// お金を使用
bool SpendPlayerMoney(int amount)
{
    if (playerData.money < amount)
    {
        printfDx("Not enough money! Current: %d, Required: %d\n", playerData.money, amount);
        return false;
    }
    playerData.money -= amount;
    printfDx("Money spent: -%d (Remaining: %d)\n", amount, playerData.money);
    return true;
}

// スキルを習得する
void UnlockSkill(int skillNumber)
{
    switch (skillNumber)
    {
    case 1: playerData.hasSkill1 = true; printfDx("Skill 1 Unlocked!\n"); break;
    case 2: playerData.hasSkill2 = true; printfDx("Skill 2 Unlocked!\n"); break;
    case 3: playerData.hasSkill3 = true; printfDx("Skill 3 Unlocked!\n"); break;
    }
}

// スキルの使用回数を回復する
void RestoreSkillCount(int skillNumber, int amount)
{
    switch (skillNumber)
    {
    case 1:
        // スキル1は無限なので何もしない
        break;
    case 2:
        playerData.skill2Count += amount;
        if (playerData.skill2Count > playerData.skill2MaxCount)
            playerData.skill2Count = playerData.skill2MaxCount;
        printfDx("Skill 2 count restored: %d / %d\n", playerData.skill2Count, playerData.skill2MaxCount);
        break;
    case 3:
        playerData.skill3Count += amount;
        if (playerData.skill3Count > playerData.skill3MaxCount)
            playerData.skill3Count = playerData.skill3MaxCount;
        printfDx("Skill 3 count restored: %d / %d\n", playerData.skill3Count, playerData.skill3MaxCount);
        break;
    }
}

// スキルの使用回数を最大まで回復する
void RestoreSkillCountToMax(int skillNumber)
{
    switch (skillNumber)
    {
    case 1:
        // スキル1は無限なので何もしない
        break;
    case 2:
        playerData.skill2Count = playerData.skill2MaxCount;
        printfDx("Skill 2 fully restored: %d / %d\n", playerData.skill2Count, playerData.skill2MaxCount);
        break;
    case 3:
        playerData.skill3Count = playerData.skill3MaxCount;
        printfDx("Skill 3 fully restored: %d / %d\n", playerData.skill3Count, playerData.skill3MaxCount);
        break;
    }
}

// 内部関数の実装
namespace
{
    // 入力処理
    void ProcessInput()
    {
        ProcessMovement();
        ProcessSkills();
    }

    // 移動処理
    void ProcessMovement()
    {
        float horizontal = GetMoveHorizontal();
        playerData.velocityX = horizontal * MOVE_SPEED;

        if (horizontal > 0.0f) playerData.isFacingRight = true;
        else if (horizontal < 0.0f) playerData.isFacingRight = false;

        if (IsMoveUp()) ExecuteJump();
    }

    // スキル処理
    void ProcessSkills()
    {
        if (IsSkill1Pressed() && CanUseSkill(1)) UseSkill1();
        if (IsSkill2Pressed() && CanUseSkill(2)) UseSkill2();
        if (IsSkill3Pressed() && CanUseSkill(3)) UseSkill3();
    }

    // 物理演算更新
    void UpdatePhysics()
    {
        if (!playerData.isGrounded)
        {
            playerData.velocityY += GRAVITY;
            if (playerData.velocityY > MAX_FALL_SPEED) playerData.velocityY = MAX_FALL_SPEED;
        }

        playerData.posX += playerData.velocityX;
        playerData.posY += playerData.velocityY;

        if (playerData.posY >= GROUND_Y)
        {
            playerData.posY = GROUND_Y;
            playerData.velocityY = 0.0f;
            playerData.isGrounded = true;
            playerData.jumpCount = 0;
        }
        else
        {
            playerData.isGrounded = false;
        }

        if (playerData.posX < 0.0f) playerData.posX = 0.0f;
        if (playerData.posX > 1600.0f) playerData.posX = 1600.0f;
    }

    // 状態更新
    void UpdateState()
    {
        if (playerData.state == PlayerState::Skill1 || 
            playerData.state == PlayerState::Skill2 || 
            playerData.state == PlayerState::Skill3)
        {
            return;
        }

        if (!playerData.isGrounded)
        {
            playerData.state = (playerData.velocityY < 0.0f) ? PlayerState::Jump : PlayerState::Fall;
        }
        else
        {
            playerData.state = (playerData.velocityX != 0.0f) ? PlayerState::Walk : PlayerState::Idle;
        }
    }

    // アニメーション更新
    void UpdateAnimation()
    {
        switch (playerData.state)
        {
        case PlayerState::Idle:
            playerData.animationCounter++;
            if (playerData.animationCounter >= IDLE_ANIM_SPEED)
            {
                playerData.animationCounter = 0;
                playerData.currentFrame++;
                if (playerData.currentFrame >= IDLE_FRAME_COUNT) playerData.currentFrame = 0;
            }
            break;
        default:
            playerData.currentFrame = 0;
            break;
        }
    }

    // ジャンプ実行
    void ExecuteJump()
    {
        if (playerData.jumpCount < MAX_JUMP_COUNT)
        {
            playerData.velocityY = -JUMP_POWER;
            playerData.isGrounded = false;
            playerData.jumpCount++;
        }
    }

    // スキル1を使用
    void UseSkill1()
    {
        playerData.state = PlayerState::Skill1;
        // スキル1は無限使用可能なので回数を減らさない
        printfDx("Skill 1 Used! (Infinite)\n");
    }

    // スキル2を使用
    void UseSkill2()
    {
        playerData.state = PlayerState::Skill2;
        playerData.skill2Count--;  // 使用回数を減らす
        printfDx("Skill 2 Used! Remaining: %d / %d\n", playerData.skill2Count, playerData.skill2MaxCount);
    }

    // スキル3を使用
    void UseSkill3()
    {
        playerData.state = PlayerState::Skill3;
        playerData.skill3Count--;  // 使用回数を減らす
        printfDx("Skill 3 Used! Remaining: %d / %d\n", playerData.skill3Count, playerData.skill3MaxCount);
    }
}