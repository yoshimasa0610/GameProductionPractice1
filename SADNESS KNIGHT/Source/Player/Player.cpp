#include "Player.h"
#include "../Input/Input.h"
#include "../Animation/Animation.h"
#include "../Skill/SkillManager.h"
#include "DxLib.h"
#include <string.h>
#include "../Collision/Collision.h" // 追加

// プレイヤーのパラメータ定数
namespace
{
    const float MOVE_SPEED = 5.0f;          // 移動速度
    const float JUMP_POWER = 15.0f;         // ジャンプ力
    const float GRAVITY = 0.8f;             // 重力
    const float MAX_FALL_SPEED = 20.0f;     // 最大落下速度
    const int MAX_JUMP_COUNT = 2;           // 最大ジャンプ回数（二段ジャンプ）
    const float GROUND_Y = 700.0f;          // 地面のY座標（画面下に近い位置）
}

// プレイヤーデータとアニメーション
namespace
{
    PlayerData playerData;
    
    // アニメーションデータ
    AnimationData idleAnim;
    AnimationData walkAnim;
    AnimationData jumpAnim;
    AnimationData fallAnim;

    SkillManager skillManager;

    // プレイヤー用コライダーID
    int g_playerColliderId = -1;
}

// 内部関数の宣言
namespace
{
    void ProcessInput();
    void ProcessMovement();
    void ProcessSkills();
    void UpdatePhysics();
    void UpdateState();
    void UpdatePlayerAnimation();
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

    // プレイヤーのコライダーを作成（左上座標で渡す）
    float left = playerData.posX - (PLAYER_WIDTH / 2.0f);
    float top = playerData.posY - PLAYER_HEIGHT;
    g_playerColliderId = CreateCollider(ColliderTag::Player, left, top, (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT, &playerData);

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
    printfDx("=== Loading Player Animations ===\n");
    
    // Idle: 460x55 → 10フレーム横並び、1フレームは46x55
    bool idleLoaded = LoadAnimationFromSheet(idleAnim, "Data/Player/Idle.png", 10, 46, 55, 8, AnimationMode::Loop);
    printfDx("Idle Animation: %s (frames: %d)\n", idleLoaded ? "SUCCESS" : "FAILED", idleAnim.frameCount);
    
    // Walk と Jump は後で追加
    printfDx("Walk Animation: SKIPPED\n");
    printfDx("Jump/Fall Animation: SKIPPED\n");
    
    printfDx("=================================\n");
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
    UpdatePlayerAnimation();
}

// プレイヤーの描画
void DrawPlayer()
{
    int drawX = static_cast<int>(playerData.posX);
    int drawY = static_cast<int>(playerData.posY);

    // アニメーションが読み込まれているか確認
    bool hasAnimation = (idleAnim.frames != nullptr && idleAnim.frameCount > 0);

    if (hasAnimation)
    {
        // 状態に応じたアニメーションを描画
        switch (playerData.state)
        {
        case PlayerState::Idle:
            DrawAnimation(idleAnim, drawX, drawY, !playerData.isFacingRight);
            break;
        case PlayerState::Walk:
            if (walkAnim.frames != nullptr)
                DrawAnimation(walkAnim, drawX, drawY, !playerData.isFacingRight);
            else
                DrawAnimation(idleAnim, drawX, drawY, !playerData.isFacingRight);
            break;
        case PlayerState::Jump:
            if (jumpAnim.frames != nullptr)
                DrawAnimation(jumpAnim, drawX, drawY, !playerData.isFacingRight);
            else
                DrawAnimation(idleAnim, drawX, drawY, !playerData.isFacingRight);
            break;
        case PlayerState::Fall:
            if (fallAnim.frames != nullptr)
                DrawAnimation(fallAnim, drawX, drawY, !playerData.isFacingRight);
            else
                DrawAnimation(idleAnim, drawX, drawY, !playerData.isFacingRight);
            break;
        case PlayerState::UsingSkill:
            DrawAnimation(idleAnim, drawX, drawY, !playerData.isFacingRight);
            break;
        }
    }
    else
    {
        // デバッグ用の簡易描画（画像が読み込まれていない場合）
        unsigned int color = GetColor(255, 255, 255);
        int halfW = PLAYER_WIDTH / 2;
        int h = PLAYER_HEIGHT;
        DrawBox(drawX - halfW, drawY - h, drawX + halfW, drawY, color, TRUE);

        if (playerData.isFacingRight)
        {
            DrawTriangle(drawX + halfW, drawY - h/2, drawX + halfW - (halfW/2), drawY - h/2 - 8, drawX + halfW - (halfW/2), drawY - h/2 + 8, GetColor(255, 0, 0), TRUE);
        }
        else
        {
            DrawTriangle(drawX - halfW, drawY - h/2, drawX - halfW + (halfW/2), drawY - h/2 - 8, drawX - halfW + (halfW/2), drawY - h/2 + 8, GetColor(255, 0, 0), TRUE);
        }
    }

    // デバッグ表示（サイズ表示を追加)
    DrawFormatString(10, 130, GetColor(255, 255, 255), "Size: %d x %d", PLAYER_WIDTH, PLAYER_HEIGHT);

    // デバッグ情報表示
    DrawFormatString(10, 10, GetColor(255, 255, 255), "Position: (%.1f, %.1f)", playerData.posX, playerData.posY);
    DrawFormatString(10, 30, GetColor(255, 100, 100), "HP: %d / %d", playerData.currentHP, playerData.maxHP);
    DrawFormatString(10, 50, GetColor(200, 255, 200), "Money: %d", playerData.money);
    DrawFormatString(10, 70, GetColor(255, 255, 255), "ATK: %d", playerData.attackPower);
    DrawFormatString(10, 90, GetColor(255, 255, 255), "Grounded: %s", playerData.isGrounded ? "YES" : "NO");
    DrawFormatString(10, 110, GetColor(255, 200, 0), "Has Anim: %s", hasAnimation ? "YES" : "NO");
    
    // 状態表示
    const char* stateStr = "UNKNOWN";
    switch (playerData.state)
    {
    case PlayerState::Idle:       stateStr = "IDLE";        break;
    case PlayerState::Walk:       stateStr = "WALK";        break;
    case PlayerState::Jump:       stateStr = "JUMP";        break;
    case PlayerState::Fall:       stateStr = "FALL";        break;
    case PlayerState::UsingSkill: stateStr = "USING SKILL"; break;
    }
    DrawFormatString(10, 130, GetColor(255, 255, 0), "State: %s", stateStr);

    // プレイヤーが死んでいる場合
    if (playerData.currentHP <= 0)
    {
        DrawFormatString(700, 400, GetColor(255, 0, 0), "GAME OVER");
    }
}

// プレイヤーのリソース解放
void UnloadPlayer()
{
    // アニメーションを解放
    UnloadAnimation(idleAnim);
    UnloadAnimation(walkAnim);
    UnloadAnimation(jumpAnim);
    UnloadAnimation(fallAnim);

    // コライダー破棄
    if (g_playerColliderId != -1)
    {
        DestroyCollider(g_playerColliderId);
        g_playerColliderId = -1;
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
        float horizontal = 0.0f;

        if (IsInputKey(KEY_LEFT))  horizontal -= 1.0f;
        if (IsInputKey(KEY_RIGHT)) horizontal += 1.0f;

        playerData.velocityX = horizontal * MOVE_SPEED;

        if (horizontal > 0.0f) playerData.isFacingRight = true;
        else if (horizontal < 0.0f) playerData.isFacingRight = false;

        // ジャンプはトリガー判定
        if (IsTriggerKey(KEY_JUMP))
		{// 下キーを押しながらジャンプした場合はすり抜け処理を優先
            if (IsInputKey(KEY_DOWN))
            {
                playerData.dropThrough = true;
                playerData.dropTimer = 15;
            }
			else// 通常のジャンプ処理
            {
                ExecuteJump();
            }
        }
    }

    // スキル処理
    void ProcessSkills()
    {
        if (IsTriggerKey(KEY_SKILL1))
        {
            skillManager.UseSkill(0, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsTriggerKey(KEY_SKILL2))
        {
            skillManager.UseSkill(1, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsTriggerKey(KEY_SKILL3))
        {
            skillManager.UseSkill(2, &playerData);
            playerData.state = PlayerState::UsingSkill;
        }

        if (IsTriggerKey(KEY_CHANGE))
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

        // コライダー位置更新（左上で渡す）
        if (g_playerColliderId != -1)
        {
            float left = playerData.posX - (PLAYER_WIDTH / 2.0f);
            float top = playerData.posY - PLAYER_HEIGHT;
            UpdateCollider(g_playerColliderId, left, top, (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
        }

        // 衝突解決（ブロックなどと干渉していればここで押し出し等が行われる）
        ResolveCollisions();

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
    void UpdatePlayerAnimation()
    {
        // 状態に応じてアニメーションを更新
        switch (playerData.state)
        {
        case PlayerState::Idle:
            UpdateAnimation(idleAnim);
            break;
        case PlayerState::Walk:
            UpdateAnimation(walkAnim);
            break;
        case PlayerState::Jump:
            UpdateAnimation(jumpAnim);
            break;
        case PlayerState::Fall:
            UpdateAnimation(fallAnim);
            break;
        case PlayerState::UsingSkill:
            UpdateAnimation(idleAnim);
            break;
        default:
            break;
        }
    }

    // ジャンプに関する処理
    void ExecuteJump()
    {
        if (playerData.jumpCount < MAX_JUMP_COUNT)
        {
            playerData.velocityY = -JUMP_POWER;
            playerData.isGrounded = false;
            playerData.jumpCount++;
            
            // ジャンプアニメーションをリセット
            ResetAnimation(jumpAnim);
        }
    }
}