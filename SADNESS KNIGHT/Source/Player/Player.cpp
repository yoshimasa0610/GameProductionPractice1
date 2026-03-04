#include "Player.h"
#include "../Input/Input.h"
#include "../Animation/Animation.h"
#include "../Skill/SkillManager.h"
#include "DxLib.h"
#include <string.h>
#include <cmath>
#include "../Collision/Collision.h" // 追加
#include "../Map/MapManager.h"
#include "../Camera/Camera.h"

// プレイヤーのパラメータ定数
namespace
{
    const float MOVE_SPEED = 3.5f;          // 移動速度
    const float JUMP_POWER = 10.5f;         // ジャンプ力
    const float GRAVITY = 0.3f;             // 重力
    const float MAX_FALL_SPEED = 6.0f;      // 最大落下速度
    const int MAX_JUMP_COUNT = 2;           // 最大ジャンプ回数（二段ジャンプ）
    const float SPAWN_RAISE_OFFSET = 16.0f; // スポーン時に少し上げる量
    const float DODGE_DISTANCE = 80.0f;     // 回避ダッシュ距離
    const int DODGE_COOLDOWN = 30;          // 回避クールダウン（フレーム）
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
    AnimationData healingAnim;  // 回復アニメーション
    AnimationData dodgeAnim;     // 回避アニメーション
    AnimationData dashEffectAnim;// ダッシュエフェクトアニメーション

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
    int lastJumpInputFrame = -1;
    bool g_PendingCenterSpawn = false;
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
    bool PlacePlayerAtMapCenter();
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

    g_PendingCenterSpawn = true;

    runAnimState = RunAnimState::None;
    prevFacingRight = playerData.isFacingRight;
    prevIsGrounded = playerData.isGrounded; // 追加

    // ===== パッシブアビリティ =====
    playerData.hasDoubleJump = false;  // 初期状態では二段ジャンプは未解放

    // ===== 回復関連 =====
    playerData.healCount = 3;
    playerData.maxHealCount = 3;
    playerData.healExecuted = false;

    // ===== 回避関連 =====
    playerData.isInvincible = false;
    playerData.dodgeCooldown = 0;

    // ===== ダッシュエフェクト関連 =====
    playerData.showDashEffect = false;
    playerData.dashEffectX = 0.0f;
    playerData.dashEffectY = 0.0f;
    playerData.dashEffectFacingRight = true;
    
    // ===== 基礎ステータス =====
    playerData.baseMaxHp = 150;
    playerData.baseMaxSlot = 5;       // 仮に5スロット
    playerData.basehealPower = 90;

    // ===== 現在値（初期は基礎値と同じ） =====
    playerData.maxHP = playerData.baseMaxHp;
    playerData.currentHP = playerData.maxHP;
    playerData.maxSlot = playerData.baseMaxSlot;
    playerData.usedSlot = 0;
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
}

// プレイヤーのリソース読み込み
void LoadPlayer()
{
    bool idleLoaded = LoadAnimationFromSheet(idleAnim, "Data/Player/Idle.png", 10, 46, 55, 8, AnimationMode::Loop);
    (void)idleLoaded;

    bool runLoaded = LoadAnimationAuto(walkAnim, "Data/Player/Run.png", 65, 48, 8, AnimationMode::Loop);
    (void)runLoaded;

    if (FileExists("Data/Player/Run_Start.png"))
    {
        bool runStartLoaded = LoadAnimationAuto(runStartAnim, "Data/Player/Run_Start.png", 59, 53, 8, AnimationMode::Once);
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
            16, 0, 15, 63, 53, 8, AnimationMode::Once);
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
            24, 0, 13, 64, 80, 4, AnimationMode::Once);
        bool fallLoaded = LoadAnimationFromSheetRange(fallAnim, "Data/Player/Jump.png",
            24, 13, 7, 64, 80, 5, AnimationMode::Loop);
        bool landLoaded = LoadAnimationFromSheetRange(landAnim, "Data/Player/Jump.png",
            24, 20, 4, 64, 80, 4, AnimationMode::Once);
        
        // デバッグ: ロード結果を表示
        printfDx("Jump anim loaded: %d, frames=%d\n", jumpLoaded, jumpAnim.frameCount);
        printfDx("Fall anim loaded: %d, frames=%d\n", fallLoaded, fallAnim.frameCount);
        printfDx("Land anim loaded: %d, frames=%d\n", landLoaded, landAnim.frameCount);
        
        (void)jumpLoaded;
        (void)fallLoaded;
        (void)landLoaded;
    }
    else
    {
        InitAnimation(jumpAnim);
        InitAnimation(fallAnim);
        InitAnimation(landAnim);
        printfDx("Jump.png not found!\n");
    }

    // healing_merged.png（291x486 / 3x6 = 18フレーム）
    if (FileExists("Data/Player/healing_merged.png"))
    {
        bool healLoaded = LoadAnimationAuto(healingAnim, "Data/Player/healing_merged.png",
            97, 81, 8, AnimationMode::Once);
        printfDx("Healing anim loaded: %d, frames=%d\n", healLoaded, healingAnim.frameCount);
        (void)healLoaded;
    }
    else
    {
        InitAnimation(healingAnim);
        printfDx("healing_merged.png not found!\n");
    }

    // aerial dash.png（624x100 / 6x2 = 12フレーム、上の6フレームのみ使用）
    if (FileExists("Data/Player/aerial dash.png"))
    {
        // 624 / 6 = 104px per frame, 100 / 2 = 50px per row
        // 上の6フレーム（インデックス0-5）のみ使用
        bool dodgeLoaded = LoadAnimationFromSheetRange(dodgeAnim, "Data/Player/aerial dash.png",
            12, 0, 6, 104, 50, 4, AnimationMode::Once);
        printfDx("Dodge anim loaded: %d, frames=%d\n", dodgeLoaded, dodgeAnim.frameCount);
        (void)dodgeLoaded;
    }
    else
    {
        InitAnimation(dodgeAnim);
        printfDx("aerial dash.png not found!\n");
    }

    // aerial dash_smoke.png（240x314 / 4x2 = 8フレーム、上の4フレーム+下の2フレーム = 6フレーム使用）
    if (FileExists("Data/Player/aerial dash_smoke.png"))
    {
        // 240 / 4 = 60px per frame, 314 / 2 = 157px per row
        // 6フレーム使用（0-3の4フレーム + 4-5の2フレーム）
        bool dashEffectLoaded = LoadAnimationFromSheetRange(dashEffectAnim, "Data/Player/aerial dash_smoke.png",
            8, 0, 6, 60, 157, 2, AnimationMode::Once);
        printfDx("Dash effect anim loaded: %d, frames=%d\n", dashEffectLoaded, dashEffectAnim.frameCount);
        (void)dashEffectLoaded;
    }
    else
    {
        InitAnimation(dashEffectAnim);
        printfDx("aerial dash_smoke.png not found!\n");
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

    if (g_PendingCenterSpawn)
    {
        if (PlacePlayerAtMapCenter())
        {
            g_PendingCenterSpawn = false;
            playerData.state = PlayerState::Idle;
            playerData.isInvincible = false;
            runAnimState = RunAnimState::None;
        }
    }

    // 前フレームの位置を保存（衝突処理などで必要）
    playerData.prevPosX = playerData.posX;
    playerData.prevPosY = playerData.posY;

    ProcessInput();

    UpdatePhysics();

    skillManager.Update(&playerData);

    UpdateState();
    UpdatePlayerAnimation();

    // ダッシュエフェクトの更新
    if (playerData.showDashEffect)
    {
        UpdateAnimation(dashEffectAnim);

        // エフェクトアニメーションが終了したら非表示にする
        if (IsAnimationFinished(dashEffectAnim))
        {
            playerData.showDashEffect = false;
        }
    }
}

// プレイヤーの描画
void DrawPlayer()
{
    CameraData camera = GetCamera();

    int baseX = static_cast<int>(std::round((playerData.posX - camera.posX) * camera.scale));
    int baseY = static_cast<int>(std::round((playerData.posY - camera.posY) * camera.scale));
    bool flip = playerData.isFacingRight;

    bool hasAnimation = (idleAnim.frames != nullptr && idleAnim.frameCount > 0);

    // ダッシュエフェクトの描画（プレイヤーの後ろに描画）
    if (playerData.showDashEffect && dashEffectAnim.frames != nullptr)
    {
        int effectX = static_cast<int>(std::round((playerData.dashEffectX - camera.posX) * camera.scale));
        int effectY = static_cast<int>(std::round((playerData.dashEffectY - camera.posY) * camera.scale));

        int frameHandle = GetCurrentAnimationFrame(dashEffectAnim);
        if (frameHandle != -1)
        {
            int w = 0;
            int h = 0;
            GetGraphSize(frameHandle, &w, &h);

            // エフェクトをプレイヤーサイズの1.3倍に縮小
            float scale = (PLAYER_HEIGHT * 1.3f) / h;
            int scaledW = static_cast<int>(w * scale);
            int scaledH = static_cast<int>(h * scale);

            int drawX = effectX - (scaledW / 2);
            int drawY = effectY - scaledH;

            if (playerData.dashEffectFacingRight)
            {
                DrawExtendGraph(drawX, drawY, drawX + scaledW, drawY + scaledH, frameHandle, TRUE);
            }
            else
            {
                DrawExtendGraph(drawX + scaledW, drawY, drawX, drawY + scaledH, frameHandle, TRUE);
            }
        }
    }

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
        case PlayerState::Healing:
            if (healingAnim.frames != nullptr)
                DrawAnimationAligned(healingAnim, baseX, baseY, flip);
            else
                DrawAnimationAligned(idleAnim, baseX, baseY, flip);
            break;
        case PlayerState::Dodging:
            if (dodgeAnim.frames != nullptr)
                DrawAnimationAligned(dodgeAnim, baseX, baseY, flip);
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
    UnloadAnimation(landAnim);
    UnloadAnimation(healingAnim);
    UnloadAnimation(dodgeAnim);
    UnloadAnimation(dashEffectAnim);  // 追加

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
    // 無敵中はダメージを受けない
    if (playerData.isInvincible)
    {
        printfDx("無敵中！ダメージ無効\n");
        return;
    }

    // 回復中の場合はキャンセル
    if (playerData.state == PlayerState::Healing)
    {
        printfDx("回復キャンセル: ダメージを受けた\n");
        playerData.state = PlayerState::Idle;
        playerData.healExecuted = false;
    }

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

// ===== パッシブアビリティ関数の実装 =====

// 二段ジャンプを解放する
void UnlockDoubleJump()
{
    playerData.hasDoubleJump = true;
    printfDx("パッシブアビリティ解放: 二段ジャンプ\n");
}

// 二段ジャンプが解放されているか
bool HasDoubleJump()
{
    return playerData.hasDoubleJump;
}

// ===== 回復関連関数の実装 =====

// 回復を試みる（キー入力時）
void TryHeal()
{
    // デバッグ: 条件をチェック
    printfDx("TryHeal called! healCount=%d, isGrounded=%d, state=%d\n", 
             playerData.healCount, playerData.isGrounded, (int)playerData.state);

    // 回復回数が残っており、地上にいて、回復中でない場合のみ実行
    if (playerData.healCount > 0 && 
        playerData.isGrounded && 
        playerData.state != PlayerState::Healing &&
        playerData.state != PlayerState::UsingSkill)
    {
        // 回復開始時に移動を強制停止
        playerData.velocityX = 0.0f;
        currentMoveDir = 0;  // 移動入力をクリア
        
        // 走行アニメーション状態をリセット
        runAnimState = RunAnimState::None;
        
        playerData.state = PlayerState::Healing;
        playerData.healExecuted = false;  // 回復未実行状態にリセット
        ResetAnimation(healingAnim);
        printfDx("回復モーション開始！残り回復回数: %d\n", playerData.healCount);
    }
    else
    {
        printfDx("回復条件を満たしていません\n");
    }
}

// 残り回復回数を取得
int GetHealCount()
{
    return playerData.healCount;
}

// 最大回復回数を取得
int GetMaxHealCount()
{
    return playerData.maxHealCount;
}

// ===== 回避関連関数の実装 =====

// 回避を試みる（キー入力時）
void TryDodge()
{
    // クールダウン中、回復中、スキル使用中は回避できない
    if (playerData.dodgeCooldown > 0 ||
        playerData.state == PlayerState::Healing ||
        playerData.state == PlayerState::UsingSkill ||
        playerData.state == PlayerState::Dodging)  // 回避中は再発動できない
    {
        return;
    }

    // 回避状態に移行
    playerData.state = PlayerState::Dodging;
    
    // アニメーションをリセット
    ResetAnimation(dodgeAnim);
    
    // 1フレーム目から無敵付与
    playerData.isInvincible = true;
    
    // 前方にダッシュ（向いている方向に移動）
    float dashDirection = playerData.isFacingRight ? 1.0f : -1.0f;
    playerData.velocityX = dashDirection * (DODGE_DISTANCE / 6.0f);  // 6フレームで移動
    
    // クールダウン設定
    playerData.dodgeCooldown = DODGE_COOLDOWN;
    
    // 走行アニメーション状態をリセット
    runAnimState = RunAnimState::None;
    
    // ダッシュエフェクトを開始
    playerData.showDashEffect = true;
    playerData.dashEffectX = playerData.posX;
    playerData.dashEffectY = playerData.posY;
    playerData.dashEffectFacingRight = playerData.isFacingRight;
    ResetAnimation(dashEffectAnim);
    
    printfDx("回避開始！velocity=%f, frameCount=%d\n", playerData.velocityX, dodgeAnim.frameCount);
}

// 無敵状態かどうかを取得
bool IsPlayerInvincible()
{
    return playerData.isInvincible;
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
        // 回避中は移動入力を完全に無効化
        if (playerData.state == PlayerState::Dodging)
        {
            // 回避中の速度は維持（ダッシュ移動）
            return;
        }

        // 回復中の処理
        bool wasHealing = false;
        if (playerData.state == PlayerState::Healing)
        {
            playerData.velocityX = 0.0f;
            currentMoveDir = 0;  // 入力状態もクリア

            // 回復中にジャンプキーが押されたらキャンセル
            if (IsTriggerKey(KEY_JUMP))
            {
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
                wasHealing = true;
                // このまま処理を続行してジャンプを実行
            }
            else
            {
                return;  // ジャンプキー以外の入力は無視
            }
        }

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
        // 回避中でも回避キーは処理する（クールダウンでガード済み）         // ただし他のスキルは無効化
        if (playerData.state == PlayerState::Dodging)
        {
            // 回避キー入力だけは処理
            if (IsTriggerKey(KEY_DODGE))
            {
                printfDx("KEY_DODGE pressed!\n");
                TryDodge();
            }
            return;
        }

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

        // 回復キー入力処理
        if (IsTriggerKey(KEY_HEAL))
        {
            printfDx("KEY_HEAL pressed!\n");
            TryHeal();
        }

        // 回避キー入力処理
        if (IsTriggerKey(KEY_DODGE))
        {
            TryDodge();
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

        // 接地中は縦速度を確実に止める
        if (playerData.isGrounded && playerData.velocityY > 0.0f)
        {
            playerData.velocityY = 0.0f;
        }

        // マップ下に落ちすぎた場合だけ保護（床判定は衝突側に任せる）
        const int mapW = GetMapWidth();
        const int mapH = GetMapHeight();
        if (mapH > 0)
        {
            const float bottomLimit = static_cast<float>(mapH);
            if (playerData.posY > bottomLimit)
            {
                playerData.posY = bottomLimit;
                playerData.velocityY = 0.0f;
                playerData.isGrounded = true;
                playerData.jumpCount = 0;
            }
        }

        if (playerData.posX < 0.0f) playerData.posX = 0.0f;
        if (mapW > 0)
        {
            const float xMax = static_cast<float>(mapW);
            if (playerData.posX > xMax) playerData.posX = xMax;
        }

        // すり抜けタイマー更新
        if (playerData.dropTimer > 0)
        {
            playerData.dropTimer--;
        }
        else
		{// タイマーが切れたらすり抜け状態を解除
            playerData.dropThrough = false;
        }

        // 回避クールダウン更新
        if (playerData.dodgeCooldown > 0)
        {
            playerData.dodgeCooldown--;
        }
    }

    // 状態更新
    void UpdateState()
    {
        // 回避中の処理
        if (playerData.state == PlayerState::Dodging)
        {
            // アニメーションがロードされていない場合は即座に終了
            if (dodgeAnim.frameCount == 0 || dodgeAnim.frames == nullptr)
            {
                printfDx("Dodge animation not loaded! Ending dodge.\n");
                playerData.state = playerData.isGrounded ? PlayerState::Idle : PlayerState::Fall;
                playerData.isInvincible = false;
                playerData.velocityX = 0.0f;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            // アニメーションが終了したらIdleに戻る
            if (IsAnimationFinished(dodgeAnim))
            {
                playerData.state = playerData.isGrounded ? PlayerState::Idle : PlayerState::Fall;
                playerData.isInvincible = false;
                playerData.velocityX = 0.0f;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            prevIsGrounded = playerData.isGrounded;
            return;
        }

        // 回復中の処理
        if (playerData.state == PlayerState::Healing)
        {
            // 移動キーのトリガー入力（新しい入力）があればキャンセル
            if (IsTriggerKey(KEY_LEFT) || IsTriggerKey(KEY_RIGHT))
            {
                printfDx("回復キャンセル: 移動入力\n");
                
                // キャンセル時は入力をクリアしてIdleに戻る
                // 次のフレームで新しい入力を待つ
                playerData.velocityX = 0.0f;
                currentMoveDir = 0;
                
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            // アニメーションが終了したら Idle に戻る
            if (IsAnimationFinished(healingAnim))
            {
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            // 14フレーム目（インデックス13）で回復実行
            if (!playerData.healExecuted && healingAnim.currentFrame == 13)
            {
                // 体力が最大値の場合は回復処理をスキップ
                if (playerData.currentHP >= playerData.maxHP)
                {
                    printfDx("体力が最大値のため回復をスキップ（回数消費なし）\n");
                    playerData.healExecuted = true;  // フラグは立てて重複実行を防ぐ
                }
                else
                {
                    // 回復処理を実行
                    HealPlayerHP(playerData.basehealPower);
                    playerData.healCount--;
                    playerData.healExecuted = true;
                    printfDx("回復実行！残り回復回数: %d\n", playerData.healCount);
                }
            }

            prevIsGrounded = playerData.isGrounded;
            return;
        }

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
            // 接地時はWalk/Idleのみ（Landへの再遷移でアニメが揺れるのを防ぐ）
            bool hasMoveInput = std::fabs(playerData.velocityX) > 0.01f;
            newState = hasMoveInput ? PlayerState::Walk : PlayerState::Idle;
        }

        if (newState != playerData.state)
        {
            if (playerData.state == PlayerState::Dodging)
            {
                prevIsGrounded = playerData.isGrounded;
                return;
            }

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
        }

        prevIsGrounded = playerData.isGrounded;
    }

    // アニメーション更新
    void UpdatePlayerAnimation()
    {
        float absVelX = std::fabs(playerData.velocityX);
        bool wasMoving = prevAbsVelX > 0.01f;
        bool isMoving = absVelX > 0.01f;

        // 回避中は回避アニメーションを優先
        if (playerData.state == PlayerState::Dodging)
        {
            UpdateAnimation(dodgeAnim);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        // 回復中は回復アニメーションを優先
        if (playerData.state == PlayerState::Healing)
        {
            UpdateAnimation(healingAnim);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        // 空中状態（Jump/Fall/Land）は優先的に処理
        if (playerData.state == PlayerState::Jump)
        {
            UpdateAnimation(jumpAnim);
        }
        else if (playerData.state == PlayerState::Fall)
        {
            UpdateAnimation(fallAnim);
        }
        else if (playerData.state == PlayerState::Land)
        {
            UpdateAnimation(landAnim);
        }
        else if (playerData.state == PlayerState::Walk && (currentMoveDir != 0))
        {
            // 走り始め（停止→移動）
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
            else
            {
                // Idle または UsingSkill 状態
                if (playerData.state == PlayerState::Idle || playerData.state == PlayerState::UsingSkill)
                {
                    UpdateAnimation(idleAnim);
                }
            }
        }

        prevFacingRight = playerData.isFacingRight;
        prevAbsVelX = absVelX;
    }

    // ジャンプに関する処理
    void ExecuteJump()
    {
        const int inputFrame = GetInputFrame();
        if (lastJumpInputFrame == inputFrame)
        {
            return;
        }

        if (playerData.isGrounded)
        {
            playerData.jumpCount = 0;
        }

        // 二段ジャンプが解放されている場合はMAX_JUMP_COUNT、そうでない場合は1
        int maxJumps = playerData.hasDoubleJump ? MAX_JUMP_COUNT : 1;

        if (playerData.jumpCount < maxJumps)
        {
            playerData.velocityY = -JUMP_POWER;
            playerData.isGrounded = false;
            playerData.jumpCount++;
            lastJumpInputFrame = inputFrame;

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

    bool PlacePlayerAtMapCenter()
    {
        const int mapW = GetMapWidth();
        const int mapH = GetMapHeight();
        if (mapW <= 0 || mapH <= 0)
        {
            return false;
        }

        const float centerX = mapW * 0.5f;
        const float searchStep = 32.0f;
        const int maxTry = (mapW / 32) + 2;

        for (int i = 0; i < maxTry; ++i)
        {
            float tryX = centerX;
            if (i > 0)
            {
                const int ring = (i + 1) / 2;
                const float offset = ring * searchStep;
                tryX += (i % 2 == 1) ? offset : -offset;
            }

            if (tryX < (PLAYER_WIDTH * 0.5f) || tryX > (mapW - PLAYER_WIDTH * 0.5f))
            {
                continue;
            }

            playerData.posX = tryX;
            playerData.posY = 0.0f;
            playerData.velocityX = 0.0f;
            playerData.velocityY = 0.0f;
            playerData.isGrounded = false;
            playerData.jumpCount = 0;

            if (SnapPlayerToGround(&playerData, static_cast<float>(mapH)))
            {
                playerData.posY -= SPAWN_RAISE_OFFSET;
                playerData.isGrounded = false;

                if (g_playerColliderId != -1)
                {
                    float left = playerData.posX - (PLAYER_WIDTH / 2.0f);
                    float top = playerData.posY - PLAYER_HEIGHT;
                    UpdateCollider(g_playerColliderId, left, top, (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
                }
                return true;
            }
        }

        return false;
    }
}