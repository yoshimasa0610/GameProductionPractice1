#include "Player.h"
#include "../Input/Input.h"
#include "../Animation/Animation.h"
#include "../Skill/SkillManager.h"
#include "DxLib.h"
#include <string.h>
#include <cmath>
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

    AnimationData idleAnim;
    AnimationData walkAnim;
    AnimationData runStartAnim;
    AnimationData runStopAnim;
    AnimationData jumpAnim;
    AnimationData fallAnim;
    AnimationData landAnim;

    SkillManager skillManager;

    int g_playerColliderId = -1;

    enum class RunAnimState
    {
        None,
        Start,
        Run,
        Stop
    };

    RunAnimState runAnimState = RunAnimState::None;
    bool prevFacingRight = true;
    float prevAbsVelX = 0.0f;

    int currentMoveDir = 0;

    bool prevIsGrounded = false;
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
    bool FileExists(const char* path);
    void DrawAnimationAligned(const AnimationData& anim, int baseX, int baseY, bool flip); // 追加
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

    runAnimState = RunAnimState::None;
    prevFacingRight = playerData.isFacingRight;
    prevIsGrounded = playerData.isGrounded; // 追加
    
    // ===== 基礎ステータス =====
    playerData.baseMaxHp = 150;
    playerData.baseMaxSlot = 5;       // 仮に5スロット
    playerData.basehealPower = 90;

    // ===== 現在値（初期は基礎値と同じ） =====
    playerData.maxHP = playerData.baseMaxHp;
    playerData.currentHP = playerData.maxHP;
    playerData.maxSlot = playerData.baseMaxSlot;

    // ===== 装備補正 =====
    playerData.healPowerBonus = 0;
    playerData.damageTakenRate = 0.0f;
    playerData.skillCountRate = 0.0f;
    playerData.skillCooldownRate = 0.0f;
    playerData.healCountBonus = 0;

    // ひつように応じてろりろり
    playerData.attackPower = 100;
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
    slash.duration = 30;

    skillManager.AddSkill(slash);
    skillManager.EquipSkill(0, 0, 1);
}

// プレイヤーのリソース読み込み
void LoadPlayer()
{
    bool idleLoaded = LoadAnimationFromSheet(idleAnim, "Data/Player/Idle.png", 10, 46, 55, 8, AnimationMode::Loop);
    (void)idleLoaded;

    bool runLoaded = LoadAnimationAuto(walkAnim, "Data/Player/Run.png", 65, 48, 6, AnimationMode::Loop);
    (void)runLoaded;

    if (FileExists("Data/Player/Run_Start.png"))
    {
        bool runStartLoaded = LoadAnimationAuto(runStartAnim, "Data/Player/Run_Start.png", 59, 53, 6, AnimationMode::Once);
        (void)runStartLoaded;
    }
    else
    {
        InitAnimation(runStartAnim);
    }

    if (FileExists("Data/Player/Run_Stop.png"))
    {
        // 252x212 / 4x4 = 16コマ中、15コマだけ使用
        bool runStopLoaded = LoadAnimationFromSheetRange(runStopAnim, "Data/Player/Run_Stop.png",
            16, 0, 15, 63, 53, 6, AnimationMode::Once);
        (void)runStopLoaded;
    }
    else
    {
        InitAnimation(runStopAnim);
    }

    // Jump.png（256x480 / 4x6 = 24フレーム）
    if (FileExists("Data/Player/Jump.png"))
    {
        bool jumpLoaded = LoadAnimationFromSheetRange(jumpAnim, "Data/Player/Jump.png",
            24, 0, 13, 64, 80, 6, AnimationMode::Once);
        bool fallLoaded = LoadAnimationFromSheetRange(fallAnim, "Data/Player/Jump.png",
            24, 13, 6, 64, 80, 6, AnimationMode::Loop);
        bool landLoaded = LoadAnimationFromSheetRange(landAnim, "Data/Player/Jump.png",
            24, 19, 5, 64, 80, 6, AnimationMode::Once);
        (void)jumpLoaded;
        (void)fallLoaded;
        (void)landLoaded;
    }
    else
    {
        InitAnimation(jumpAnim);
        InitAnimation(fallAnim);
        InitAnimation(landAnim);
    }
}

// プレイヤーの更新
void UpdatePlayer()
{
    // プレイヤーが生きている場合のみ更新
    if (playerData.currentHP <= 0)
    {
        return;
    }
    // 前フレームの位置を保存（衝突処理などで必要）
    playerData.prevPosX = playerData.posX;
    playerData.prevPosY = playerData.posY;

    ProcessInput();
    UpdatePhysics();

    skillManager.Update(&playerData);

    UpdateState();
    UpdatePlayerAnimation();
}

// プレイヤーの描画
void DrawPlayer()
{
    int baseX = static_cast<int>(playerData.posX);
    int baseY = static_cast<int>(playerData.posY);
    bool flip = playerData.isFacingRight;

    bool hasAnimation = (idleAnim.frames != nullptr && idleAnim.frameCount > 0);

    if (hasAnimation)
    {
        if (playerData.isGrounded && runAnimState == RunAnimState::Stop && runStopAnim.frames != nullptr && !IsAnimationFinished(runStopAnim))
        {
            if (GetCurrentAnimationFrame(runStopAnim) != -1)
            {
                DrawAnimationAligned(runStopAnim, baseX, baseY, flip);
                return;
            }
        }

        switch (playerData.state)
        {
        case PlayerState::Idle:
            DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        case PlayerState::Walk:
            if (runAnimState == RunAnimState::Start && runStartAnim.frames != nullptr)
            {
                DrawAnimationAligned(runStartAnim, baseX, baseY, flip);
            }
            else
            {
                DrawAnimationAligned(walkAnim, baseX, baseY, flip);
            }
            break;
        case PlayerState::Jump:
            if (jumpAnim.frames != nullptr)
                DrawAnimationAligned(jumpAnim, baseX, baseY, flip);
            else
                DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        case PlayerState::Fall:
            if (fallAnim.frames != nullptr)
                DrawAnimationAligned(fallAnim, baseX, baseY, flip);
            else
                DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        case PlayerState::UsingSkill:
            DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        case PlayerState::Land:
            if (landAnim.frames != nullptr)
                DrawAnimationAligned(landAnim, baseX, baseY, flip);
            else
                DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        }
    }
    else
    {
        // 既存の簡易描画はそのまま
    }

    // 既存のデバッグ表示はそのまま
}

// プレイヤーのリソース解放
void UnloadPlayer()
{
    UnloadAnimation(idleAnim);
    UnloadAnimation(walkAnim);
    UnloadAnimation(runStartAnim);
    UnloadAnimation(runStopAnim);
    UnloadAnimation(jumpAnim);
    UnloadAnimation(fallAnim);
    UnloadAnimation(landAnim); // 追加

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
    // 装備によるダメージ減少の補正を加えました
    int finalDamage = (int)floor(damage * (1.0f + playerData.damageTakenRate));
    if (finalDamage < 1) finalDamage = 1;

    playerData.currentHP -= finalDamage;
    if (playerData.currentHP < 0) playerData.currentHP = 0;
    printfDx("Player damaged! HP: %d / %d\n", playerData.currentHP, playerData.maxHP);
}

// HPを回復する
void HealPlayerHP(int healAmount)
{
    int finalHeal = healAmount;

    // 回復力補正を適用 
    finalHeal += playerData.healPowerBonus;

    playerData.currentHP += finalHeal;

    if (playerData.currentHP > playerData.maxHP)
        playerData.currentHP = playerData.maxHP;

    printfDx("Player healed! +%d HP: %d / %d\n",
        finalHeal,
        playerData.currentHP,
        playerData.maxHP);
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
        currentMoveDir = 0;

        if (IsInputKey(KEY_LEFT))  currentMoveDir -= 1;
        if (IsInputKey(KEY_RIGHT)) currentMoveDir += 1;

        float horizontal = (float)currentMoveDir;

        playerData.velocityX = horizontal * MOVE_SPEED;

        if (horizontal > 0.0f) playerData.isFacingRight = true;
        else if (horizontal < 0.0f) playerData.isFacingRight = false;

        if (IsTriggerKey(KEY_JUMP))
        {
            if (IsInputKey(KEY_DOWN))
            {
                playerData.dropThrough = true;
                playerData.dropTimer = 15;
            }
            else
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

        // すり抜けタイマー更新
        if (playerData.dropTimer > 0)
        {
            playerData.dropTimer--;
        }
        else
		{// タイマーが切れたらすり抜け状態を解除
            playerData.dropThrough = false;
        }
    }

    // 状態更新
    void UpdateState()
    {
        if (playerData.state == PlayerState::UsingSkill)
        {
            return;
        }

        PlayerState newState;

        if (!playerData.isGrounded)
        {
            newState = (playerData.velocityY < 0.0f) ? PlayerState::Jump : PlayerState::Fall;
        }
        else
        {
            // 着地中でも入力があるなら Walk を優先
            bool hasMoveInput = std::fabs(playerData.velocityX) > 0.01f;

            if (!prevIsGrounded && playerData.state == PlayerState::Fall)
            {
                newState = PlayerState::Land;
            }
            else if (playerData.state == PlayerState::Land && !IsAnimationFinished(landAnim) && !hasMoveInput)
            {
                newState = PlayerState::Land;
            }
            else
            {
                newState = hasMoveInput ? PlayerState::Walk : PlayerState::Idle;
            }
        }

        if (newState != playerData.state)
        {
            playerData.state = newState;
            runAnimState = RunAnimState::None;

            if (playerData.state == PlayerState::Jump)
            {
                ResetAnimation(jumpAnim);
            }
            else if (playerData.state == PlayerState::Fall)
            {
                ResetAnimation(fallAnim);
            }
            else if (playerData.state == PlayerState::Land)
            {
                ResetAnimation(landAnim);
            }
        }

        prevIsGrounded = playerData.isGrounded;
    }

    // アニメーション更新
    void UpdatePlayerAnimation()
    {
        float absVelX = std::fabs(playerData.velocityX);
        bool wasMoving = prevAbsVelX > 0.01f;
        bool isMoving = absVelX > 0.01f;

        if (playerData.state == PlayerState::Walk && (currentMoveDir != 0))
        {
            // 走り始め（停止→移動）"
            if (!wasMoving && runStartAnim.frames != nullptr)
            {
                runAnimState = RunAnimState::Start;
                ResetAnimation(runStartAnim);
            }

            if (runAnimState == RunAnimState::Start && runStartAnim.frames != nullptr)
            {
                UpdateAnimation(runStartAnim);
                if (IsAnimationFinished(runStartAnim))
                {
                    runAnimState = RunAnimState::Run;
                    ResetAnimation(walkAnim);
                }
            }
            else
            {
                runAnimState = RunAnimState::Run;

                int speed = (int)(10.0f - (absVelX / MOVE_SPEED) * 8.0f);
                if (speed < 2) speed = 2;
                if (speed > 10) speed = 10;
                SetAnimationSpeed(walkAnim, speed);

                UpdateAnimation(walkAnim);
            }
        }
        else
        {
            // 停止モーション（移動→停止）
            if (playerData.isGrounded && wasMoving && runStopAnim.frames != nullptr)
            {
                runAnimState = RunAnimState::Stop;
                ResetAnimation(runStopAnim);
            }

            if (runAnimState == RunAnimState::Stop && runStopAnim.frames != nullptr)
            {
                UpdateAnimation(runStopAnim);
                if (IsAnimationFinished(runStopAnim))
                {
                    runAnimState = RunAnimState::None;

                    if (playerData.state == PlayerState::Idle)
                    {
                        ResetAnimation(idleAnim);
                    }
                    else if (playerData.state == PlayerState::Walk)
                    {
                        ResetAnimation(walkAnim);
                    }
                }
            }

            switch (playerData.state)
            {
            case PlayerState::Idle:
            case PlayerState::UsingSkill:
                UpdateAnimation(idleAnim);
                break;
            case PlayerState::Jump:
                UpdateAnimation(jumpAnim);
                break;
            case PlayerState::Fall:
                UpdateAnimation(fallAnim);
                break;
            case PlayerState::Land:
                UpdateAnimation(landAnim);
                break;
            default:
                break;
            }
        }

        prevFacingRight = playerData.isFacingRight;
        prevAbsVelX = absVelX;
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

    bool FileExists(const char* path)
    {
        int handle = FileRead_open(path);
        if (handle == 0)
        {
            return false;
        }
        FileRead_close(handle);
        return true;
    }

    void DrawAnimationAligned(const AnimationData& anim, int baseX, int baseY, bool flip)
    {
        int frameHandle = GetCurrentAnimationFrame(anim);
        if (frameHandle == -1)
        {
            return;
        }

        int w = 0;
        int h = 0;
        GetGraphSize(frameHandle, &w, &h);

        // 固定枠(PLAYER_WIDTH/HEIGHT)の中で中央寄せ + 足元揃え
        int drawX = baseX - (PLAYER_WIDTH / 2) + (PLAYER_WIDTH - w) / 2;
        int drawY = baseY - PLAYER_HEIGHT + (PLAYER_HEIGHT - h);

        if (flip)
        {
            DrawTurnGraph(drawX, drawY, frameHandle, TRUE);
        }
        else
        {
            DrawGraph(drawX, drawY, frameHandle, TRUE);
        }
    }
}