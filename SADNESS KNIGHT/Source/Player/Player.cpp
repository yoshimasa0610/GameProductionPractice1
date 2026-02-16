#include "Player.h"
#include "../Input/Input.h"
#include "DxLib.h"

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
    const int MP_REGEN_RATE = 60;           // MP回復速度（60フレームに1回復）
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
    void UpdateMPRegen();
    void ExecuteJump();
    void UseSkill1();
    void UseSkill2();
    void UseSkill3();
}

/// <summary>
/// プレイヤーの初期化
/// </summary>
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
    playerData.maxMP = 50;
    playerData.currentMP = 50;
    playerData.attackPower = 10;
    playerData.defense = 5;
    playerData.money = 0;
    
    // スキル
    playerData.hasSkill1 = true;
    playerData.hasSkill2 = false;
    playerData.hasSkill3 = false;
    playerData.skill1MP = 10;
    playerData.skill2MP = 20;
    playerData.skill3MP = 30;
    
    // アニメーション
    playerData.currentFrame = 0;
    playerData.animationCounter = 0;
    playerData.mpRegenCounter = 0;
}

/// <summary>
/// プレイヤーのリソース読み込み
/// </summary>
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

/// <summary>
/// プレイヤーの更新
/// </summary>
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
    UpdateMPRegen();
}

/// <summary>
/// プレイヤーの描画
/// </summary>
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
    DrawFormatString(10, 50, GetColor(100, 100, 255), "MP: %d / %d", playerData.currentMP, playerData.maxMP);
    DrawFormatString(10, 70, GetColor(200, 255, 200), "Money: %d", playerData.money);
    DrawFormatString(10, 90, GetColor(255, 255, 255), "ATK: %d  DEF: %d", playerData.attackPower, playerData.defense);
    DrawFormatString(10, 110, GetColor(255, 255, 255), "Grounded: %s", playerData.isGrounded ? "YES" : "NO");
    
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
    DrawFormatString(10, 130, GetColor(255, 255, 0), "State: %s", stateStr);

    // プレイヤーが死んでいる場合
    if (playerData.currentHP <= 0)
    {
        DrawFormatString(700, 400, GetColor(255, 0, 0), "GAME OVER");
    }
}

/// <summary>
/// プレイヤーのリソース解放
/// </summary>
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

/// <summary>
/// プレイヤーデータを取得
/// </summary>
PlayerData& GetPlayerData()
{
    return playerData;
}

/// <summary>
/// HPにダメージを与える
/// </summary>
void DamagePlayerHP(int damage)
{
    playerData.currentHP -= damage;
    if (playerData.currentHP < 0) playerData.currentHP = 0;
    printfDx("Player damaged! HP: %d / %d\n", playerData.currentHP, playerData.maxHP);
}

/// <summary>
/// HPを回復する
/// </summary>
void HealPlayerHP(int healAmount)
{
    playerData.currentHP += healAmount;
    if (playerData.currentHP > playerData.maxHP) playerData.currentHP = playerData.maxHP;
    printfDx("Player healed! HP: %d / %d\n", playerData.currentHP, playerData.maxHP);
}

/// <summary>
/// MPを消費する
/// </summary>
bool ConsumePlayerMP(int mpCost)
{
    if (playerData.currentMP < mpCost)
    {
        printfDx("Not enough MP! Current: %d, Required: %d\n", playerData.currentMP, mpCost);
        return false;
    }
    playerData.currentMP -= mpCost;
    printfDx("MP consumed! MP: %d / %d\n", playerData.currentMP, playerData.maxMP);
    return true;
}

/// <summary>
/// MPを回復する
/// </summary>
void RecoverPlayerMP(int recoverAmount)
{
    playerData.currentMP += recoverAmount;
    if (playerData.currentMP > playerData.maxMP) playerData.currentMP = playerData.maxMP;
}

/// <summary>
/// お金を追加
/// </summary>
void AddPlayerMoney(int amount)
{
    playerData.money += amount;
    printfDx("Money gained: +%d (Total: %d)\n", amount, playerData.money);
}

/// <summary>
/// お金を使用
/// </summary>
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

/// <summary>
/// スキルを習得する
/// </summary>
void UnlockSkill(int skillNumber)
{
    switch (skillNumber)
    {
    case 1: playerData.hasSkill1 = true; printfDx("Skill 1 Unlocked!\n"); break;
    case 2: playerData.hasSkill2 = true; printfDx("Skill 2 Unlocked!\n"); break;
    case 3: playerData.hasSkill3 = true; printfDx("Skill 3 Unlocked!\n"); break;
    }
}

// 内部関数の実装
namespace
{
    void ProcessInput()
    {
        ProcessMovement();
        ProcessSkills();
    }

    void ProcessMovement()
    {
        float horizontal = GetMoveHorizontal();
        playerData.velocityX = horizontal * MOVE_SPEED;

        if (horizontal > 0.0f) playerData.isFacingRight = true;
        else if (horizontal < 0.0f) playerData.isFacingRight = false;

        if (IsMoveUp()) ExecuteJump();
    }

    void ProcessSkills()
    {
        if (IsSkill1Pressed() && playerData.hasSkill1) UseSkill1();
        if (IsSkill2Pressed() && playerData.hasSkill2) UseSkill2();
        if (IsSkill3Pressed() && playerData.hasSkill3) UseSkill3();
    }

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

    void UpdateMPRegen()
    {
        playerData.mpRegenCounter++;
        if (playerData.mpRegenCounter >= MP_REGEN_RATE)
        {
            playerData.mpRegenCounter = 0;
            RecoverPlayerMP(1);
        }
    }

    void ExecuteJump()
    {
        if (playerData.jumpCount < MAX_JUMP_COUNT)
        {
            playerData.velocityY = -JUMP_POWER;
            playerData.isGrounded = false;
            playerData.jumpCount++;
        }
    }

    void UseSkill1()
    {
        if (!ConsumePlayerMP(playerData.skill1MP)) return;
        playerData.state = PlayerState::Skill1;
        printfDx("Skill 1 Used!\n");
    }

    void UseSkill2()
    {
        if (!ConsumePlayerMP(playerData.skill2MP)) return;
        playerData.state = PlayerState::Skill2;
        printfDx("Skill 2 Used!\n");
    }

    void UseSkill3()
    {
        if (!ConsumePlayerMP(playerData.skill3MP)) return;
        playerData.state = PlayerState::Skill3;
        printfDx("Skill 3 Used!\n");
    }
}