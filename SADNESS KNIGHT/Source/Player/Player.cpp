#include "Player.h"
#include "../Input/Input.h"
#include "../Skill/SkillManager.h"
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

    SkillManager skillManager;
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
    playerData.maxHP = 150;
    playerData.currentHP = 150;
    playerData.attackPower = 10;
    playerData.money = 0;
    
    // アニメーション
    playerData.currentFrame = 0;
    playerData.animationCounter = 0;

    // ===== テスト用スキル登録 =====
    SkillData slash;
    slash.id = 1;
    slash.name = "Slash";
    slash.type = SkillType::Attack;
    slash.coolTime = 120;
    slash.maxUseCount = -1;
    slash.power = 20;
    slash.duration = 30;

    skillManager.AddSkill(slash);
    skillManager.EquipSkill(0, 0, 1);
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

    skillManager.Update(&playerData);

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
    
    // 状態表示
    const char* stateStr = "UNKNOWN";
    switch (playerData.state)
    {
    case PlayerState::Idle:   stateStr = "IDLE";   break;
    case PlayerState::Walk:   stateStr = "WALK";   break;
    case PlayerState::Jump:   stateStr = "JUMP";   break;
    case PlayerState::Fall:   stateStr = "FALL";   break;
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
        if (IsSkill1Pressed())
        {
            skillManager.UseSkill(0, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsSkill2Pressed())
        {
            skillManager.UseSkill(1, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsSkill3Pressed())
        {
            skillManager.UseSkill(2, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsChangeSetPressed())
        {
            skillManager.ChangeSet();
        }
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
        if (playerData.state == PlayerState::UsingSkill)
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
}