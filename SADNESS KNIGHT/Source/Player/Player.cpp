#include "Player.h"
#include "../Input/Input.h"
#include "../Animation/Animation.h"
#include "../Skill/SkillManager.h"
#include "DxLib.h"
#include <string.h>
#include <cmath>
#include "../Collision/Collision.h"
#include "../Map/MapManager.h"
#include "../Camera/Camera.h"
#include "../Map/Checkpoint/CheckpointManager.h"
#include "../Sound/Sound.h"

// プレイヤーのパラメータ定数
namespace
{
    const float MOVE_SPEED = 3.5f;          // 移動速度
    const float JUMP_POWER = 10.5f;         // ジャンプ力
    const float GRAVITY = 0.3f;             // 重力
    const float MAX_FALL_SPEED = 6.0f;      // 最大落下速度
    const int MAX_JUMP_COUNT = 2;           // 最大ジャンプ回数（二段ジャンプ）
    const float SPAWN_RAISE_OFFSET = 16.0f; // スポーン時に少し上げる量
    const float DODGE_DISTANCE = 50.0f;     // 回避ダッシュ距離
    const int DODGE_COOLDOWN = 30;          // 回避クールダウン（フレーム）
    const float HURT_KNOCKBACK_SPEED = 8.0f;   // 被弾ノックバック初速（約2ブロック）
    const float HURT_KNOCKBACK_DAMP = 0.88f;   // 被弾ノックバック減衰
    const float DEATH_KNOCKBACK_SPEED = 10.5f; // 死亡ノックバック初速（約3ブロック）
    const float DEATH_KNOCKBACK_DAMP = 0.90f;  // 死亡ノックバック減衰
    const float DEATH_SLOWMO_SCALE = 0.3f;     // 死亡時のスローモーション倍率

    const float DIVE_ATTACK_SPEED = 12.0f;     // 落下攻撃の速度
    const int DIVE_ATTACK_DAMAGE = 150;        // 落下攻撃のダメージ
    const float DIVE_ATTACK_WIDTH = 120.0f;    // 当たり判定の幅（横は広め）
    const float DIVE_ATTACK_UP_HEIGHT = 3.0f; // 上方向の当たり判定
    const float DIVE_ATTACK_DOWN_HEIGHT = 3.0f;// 下方向の当たり判定（薄く）
    const int DIVE_ATTACK_MIN_ACTIVE_FRAMES = 8;
    const int DIVE_ATTACK_AIR_END_FRAME = 5;       // 13〜18コマ
    const int DIVE_ATTACK_LAND_START_FRAME = 6;    // 19コマ目開始
    const int DIVE_ATTACK_RECOVERY_FRAMES = 5;     // 使用後硬直
    const int DIVE_ATTACK_DRAW_OFFSET_X = 0;
    const int DIVE_ATTACK_DRAW_OFFSET_Y = 0;
    const int DIVE_ATTACK_LANDED_EXTRA_OFFSET_Y = 70;
    const bool DEBUG_UNLOCK_DIVE_ATTACK = true;
    const int SLASH1_FORWARD_DRAW_OFFSET = 40;
    const int SLASH2_FORWARD_DRAW_OFFSET = 18;
    const int SLASH3_FORWARD_DRAW_OFFSET = 24;
    const int SLASH1_FRAME_COMPENSATE = 0;
    const int SLASH2_FRAME_COMPENSATE = 1;
    const int SLASH3_FRAME_COMPENSATE = 1;
}

// プレイヤーデータとアニメーション
namespace
{
    PlayerData playerData;

    PlayerAnimations playerAnims;

    int g_playerColliderId = -1;
    bool g_IsPlayerDead = false;
    float g_DeathSlowMoTimer = 0.0f;

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
    int g_DiveAttackLockFrames = 0;
    int g_DiveAttackRecoveryFrames = -1;
    bool g_DiveAttackLanded = false;
    bool g_DiveAttackColliderRestored = false;
    int g_DiveAttackDrawOffsetX = DIVE_ATTACK_DRAW_OFFSET_X;
    int g_DiveAttackDrawOffsetY = DIVE_ATTACK_DRAW_OFFSET_Y;
    int g_LastSlashComboIndex = -1;
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
    bool PlacePlayerAtMapCenter();
    Skill* GetActiveSlashSkill();
}

// プレイヤーの初期化
void InitPlayer(float startX, float startY)
{
    playerData = {};
    playerData.posX = startX;
    playerData.posY = startY;
    playerData.state = PlayerState::Idle;
    playerData.isFacingRight = true;
    
    playerData.hasDoubleJump = false;
    playerData.healCount = 3;
    playerData.maxHealCount = 3;
    
    playerData.baseMaxHp = 150;
    playerData.baseMaxSlot = 5;
    playerData.basehealPower = 90;
    playerData.maxHP = 150;
    playerData.currentHP = 150;
    playerData.maxSlot = 5;
    playerData.attackPower = 100;
    
    playerData.diveAttackSpeed = DIVE_ATTACK_SPEED;
    playerData.diveAttackDamage = DIVE_ATTACK_DAMAGE;

    g_PendingCenterSpawn = true;
    runAnimState = RunAnimState::None;
    prevFacingRight = true;
    prevIsGrounded = false;
    
    g_DiveAttackLockFrames = 0;
    g_DiveAttackRecoveryFrames = -1;
    g_DiveAttackLanded = false;
    g_DiveAttackColliderRestored = false;
    g_DiveAttackDrawOffsetX = DIVE_ATTACK_DRAW_OFFSET_X;
    g_DiveAttackDrawOffsetY = DIVE_ATTACK_DRAW_OFFSET_Y;
    g_IsPlayerDead = false;
    g_DeathSlowMoTimer = 0.0f;
    g_LastSlashComboIndex = -1;

    if (DEBUG_UNLOCK_DIVE_ATTACK) UnlockDiveAttack();

    g_playerColliderId = CreatePlayerCollider(startX, startY, (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT, &playerData);
}

// プレイヤーのリソース読み込み
void LoadPlayer()
{
    LoadPlayerAnimations(playerAnims);
}

// プレイヤーの更新
void UpdatePlayer()
{
    if (playerData.currentHP <= 0 && !g_IsPlayerDead)
    {
        g_IsPlayerDead = true;
        PlaySE(SE_PLAYER_DEAD);
        playerData.state = PlayerState::Death;
        playerData.isInvincible = true;
        playerData.velocityX = playerData.isFacingRight ? -DEATH_KNOCKBACK_SPEED : DEATH_KNOCKBACK_SPEED;
        playerData.velocityY = -5.0f;
        g_DeathSlowMoTimer = 2.0f;
        if (playerAnims.death.frames != nullptr)
        {
            ResetAnimation(playerAnims.death);
        }
    }

    if (playerData.state == PlayerState::Death)
    {
        if (g_DeathSlowMoTimer > 0.0f)
        {
            g_DeathSlowMoTimer -= (1.0f / 60.0f);
            if (g_DeathSlowMoTimer < 0.0f) g_DeathSlowMoTimer = 0.0f;
        }
    }

    if (playerData.invincibleTimer > 0)
    {
        playerData.invincibleTimer--;
        if (playerData.invincibleTimer <= 0 && playerData.state != PlayerState::Dodging)
        {
            playerData.isInvincible = false;
        }
    }

    if (g_PendingCenterSpawn && PlacePlayerAtMapCenter())
    {
        g_PendingCenterSpawn = false;
        playerData.state = PlayerState::Idle;
        playerData.isInvincible = false;
        runAnimState = RunAnimState::None;
    }

    playerData.prevPosX = playerData.posX;
    playerData.prevPosY = playerData.posY;

    ProcessInput();
    UpdatePhysics();
    g_SkillManager.Update(&playerData);
    UpdateState();
    UpdatePlayerAnimation();

    if (playerData.showDashEffect)
    {
        UpdateAnimation(playerAnims.dashEffect);
        if (IsAnimationFinished(playerAnims.dashEffect))
        {
            playerData.showDashEffect = false;
            ResetAnimation(playerAnims.dashEffect);
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

    bool hasAnimation = (playerAnims.idle.frames != nullptr && playerAnims.idle.frameCount > 0);
    Skill* activeSlashSkill = GetActiveSlashSkill();

    // ダッシュエフェクトの描画（プレイヤーの後ろに描画）
    if (playerData.showDashEffect && playerAnims.dashEffect.frames != nullptr)
    {
        int effectX = static_cast<int>(std::round((playerData.dashEffectX - camera.posX) * camera.scale));
        int effectY = static_cast<int>(std::round((playerData.dashEffectY - camera.posY) * camera.scale));

        int frameHandle = GetCurrentAnimationFrame(playerAnims.dashEffect);
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
        if (activeSlashSkill != nullptr)
        {
            AnimationData* slashAnim = nullptr;
            int slashCombo = 0;
            switch (activeSlashSkill->GetComboIndex())
            {
            case 0: slashAnim = &playerAnims.slash1; slashCombo = 0; break;
            case 1: slashAnim = &playerAnims.slash2; slashCombo = 1; break;
            default: slashAnim = &playerAnims.slash3; slashCombo = 2; break;
            }

            if (slashAnim != nullptr && slashAnim->frames != nullptr && slashAnim->frameCount > 0)
            {
                int animFrame = activeSlashSkill->GetFrame();
                if (animFrame >= slashAnim->frameCount) animFrame = slashAnim->frameCount - 1;
                if (animFrame < 0) animFrame = 0;
                SetAnimationFrame(*slashAnim, animFrame);

                int forwardOffset = 0;
                if (slashCombo == 0) forwardOffset = SLASH1_FORWARD_DRAW_OFFSET + (animFrame * SLASH1_FRAME_COMPENSATE);
                else if (slashCombo == 1) forwardOffset = SLASH2_FORWARD_DRAW_OFFSET + (animFrame * SLASH2_FRAME_COMPENSATE);
                else if (slashCombo == 2) forwardOffset = SLASH3_FORWARD_DRAW_OFFSET + (animFrame * SLASH3_FRAME_COMPENSATE);
                const int drawOffsetX = playerData.isFacingRight ? forwardOffset : -forwardOffset;

                DrawAnimationAlignedOffset(*slashAnim, baseX, baseY, flip, drawOffsetX, 0);
                return;
            }
        }

        if ((playerData.state == PlayerState::Idle || playerData.state == PlayerState::Walk) &&
            playerData.isGrounded && runAnimState == RunAnimState::Stop &&
            playerAnims.runStop.frames != nullptr && !IsAnimationFinished(playerAnims.runStop))
        {
            if (GetCurrentAnimationFrame(playerAnims.runStop) != -1)
            {
                DrawAnimationAligned(playerAnims.runStop, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
                return;
            }
        }

        switch (playerData.state)
        {
        case PlayerState::Idle:
            DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Walk:
            if (runAnimState == RunAnimState::Start && playerAnims.runStart.frames != nullptr)
            {
                DrawAnimationAligned(playerAnims.runStart, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            }
            else
            {
                DrawAnimationAligned(playerAnims.walk, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            }
            break;
        case PlayerState::Jump:
            if (playerAnims.jump.frames != nullptr)
                DrawAnimationAligned(playerAnims.jump, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Fall:
            if (playerAnims.fall.frames != nullptr)
                DrawAnimationAligned(playerAnims.fall, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::UsingSkill:
        {
            Skill* activeAttackSkill = nullptr;
            for (auto& s : g_SkillManager.GetSkills())
            {
                if (s->IsActive() && s->GetType() == SkillType::Attack)
                {
                    activeAttackSkill = s.get();
                    break;
                }
            }

            if (activeAttackSkill != nullptr && activeAttackSkill->GetID() == 1)
            {
                AnimationData* slashAnim = nullptr;
                int slashCombo = 0;
                switch (activeAttackSkill->GetComboIndex())
                {
                case 0: slashAnim = &playerAnims.slash1; slashCombo = 0; break;
                case 1: slashAnim = &playerAnims.slash2; slashCombo = 1; break;
                default: slashAnim = &playerAnims.slash3; slashCombo = 2; break;
                }

                if (slashAnim != nullptr && slashAnim->frames != nullptr && slashAnim->frameCount > 0)
                {
                    int animFrame = activeAttackSkill->GetFrame();
                    if (animFrame >= slashAnim->frameCount) animFrame = slashAnim->frameCount - 1;
                    if (animFrame < 0) animFrame = 0;
                    SetAnimationFrame(*slashAnim, animFrame);

                    int forwardOffset = 0;
                    if (slashCombo == 0) forwardOffset = SLASH1_FORWARD_DRAW_OFFSET + (animFrame * SLASH1_FRAME_COMPENSATE);
                    else if (slashCombo == 1) forwardOffset = SLASH2_FORWARD_DRAW_OFFSET + (animFrame * SLASH2_FRAME_COMPENSATE);
                    else if (slashCombo == 2) forwardOffset = SLASH3_FORWARD_DRAW_OFFSET + (animFrame * SLASH3_FRAME_COMPENSATE);
                    const int drawOffsetX = playerData.isFacingRight ? forwardOffset : -forwardOffset;

                    DrawAnimationAlignedOffset(*slashAnim, baseX, baseY, flip, drawOffsetX, 0);
                }
                else
                {
                    DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
                }
            }
            else
            {
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            }
            break;
        }
        case PlayerState::Land:
            if (playerAnims.land.frames != nullptr)
                DrawAnimationAligned(playerAnims.land, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Hurt:
            if (playerAnims.hurt.frames != nullptr)
                DrawAnimationAligned(playerAnims.hurt, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Death:
            if (playerAnims.death.frames != nullptr)
                DrawAnimationAligned(playerAnims.death, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Healing:
            if (playerAnims.healing.frames != nullptr)
                DrawAnimationAligned(playerAnims.healing, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::Dodging:
            if (playerAnims.dodge.frames != nullptr)
                DrawAnimationAligned(playerAnims.dodge, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        case PlayerState::DiveAttack:
            if (playerAnims.diveAttack.frames != nullptr && playerAnims.diveAttack.frameCount > 0)
            {
                int drawOffsetY = g_DiveAttackDrawOffsetY;
                if (g_DiveAttackLanded)
                {
                    drawOffsetY += DIVE_ATTACK_LANDED_EXTRA_OFFSET_Y;
                }
                DrawAnimationAlignedOffset(playerAnims.diveAttack, baseX, baseY, flip, g_DiveAttackDrawOffsetX, drawOffsetY);
            }
            else if (playerAnims.fall.frames != nullptr)
                DrawAnimationAligned(playerAnims.fall, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            else
                DrawAnimationAligned(playerAnims.idle, baseX, baseY, flip, PLAYER_WIDTH, PLAYER_HEIGHT);
            break;
        }
    }
    else
    {
        // 既存の簡易描画はそのまま
    }

    // 落下攻撃の当たり判定可視化（デバッグ用）
    if (playerData.state == PlayerState::DiveAttack)
    {
        const float attackHeight = DIVE_ATTACK_UP_HEIGHT + DIVE_ATTACK_DOWN_HEIGHT;
        int hitboxLeft = static_cast<int>((playerData.posX - DIVE_ATTACK_WIDTH / 2.0f - camera.posX) * camera.scale);
        int hitboxTop = static_cast<int>((playerData.posY - attackHeight - camera.posY) * camera.scale);
        int hitboxWidth = static_cast<int>(DIVE_ATTACK_WIDTH * camera.scale);
        int hitboxHeight = static_cast<int>(attackHeight * camera.scale);

        DrawBox(hitboxLeft, hitboxTop, hitboxLeft + hitboxWidth, hitboxTop + hitboxHeight,
                GetColor(255, 0, 0), FALSE);
    }

    // 既存のデバッグ表示はそのまま

    // スキルデバッグ表示
    // 邪魔だと思うので一時的にコメントアウトしておきます。まだ消さないでください
    /*
    {
        int x = 20;
        int y = 60;

        DrawString(x, y, "=== Skill Debug ===", GetColor(255, 255, 0));
        y += 20;

        int currentSet = g_SkillManager.GetCurrentSet();

        DrawFormatString(x, y, GetColor(255, 255, 255),
            "Current Set : %d", currentSet + 1);
        y += 20;

        for (int i = 0; i < 3; i++)
        {
            int skillID = g_SkillManager.GetEquipSkill(currentSet, i);

            if (skillID == -1)
            {
                DrawFormatString(x, y, GetColor(150, 150, 150),
                    "Slot %d : (Empty)", i + 1);
            }
            else
            {
                const SkillData& data = GetSkillData(skillID);

                DrawFormatString(x, y, GetColor(255, 255, 255),
                    "Slot %d : %s", i + 1, data.name.c_str());
            }

            y += 20;
        }
        DrawFormatString(
            x,
            y + 10,
            GetColor(255,200,200),
            "PlayerState : %d",
            (int)playerData.state);
        DrawFormatString(20, 360, GetColor(255, 255, 255),
            "LastSkill : %d",
            g_SkillManager.GetLastUsedSkillID());
    }*/
}

// プレイヤーのリソース解放
void UnloadPlayer()
{
    UnloadPlayerAnimations(playerAnims);
    
    if (g_playerColliderId != -1)
    {
        DestroyCollider(g_playerColliderId);
        g_playerColliderId = -1;
    }
}

// ===== データ取得 =====

PlayerData& GetPlayerData() { return playerData; }
float GetPlayerPosX() { return playerData.posX; }
float GetPlayerPosY() { return playerData.posY; }
void GetPlayerPos(float& outX, float& outY) { outX = playerData.posX; outY = playerData.posY; }
float GetPlayerVelocityX() { return playerData.velocityX; }
float GetPlayerVelocityY() { return playerData.velocityY; }
PlayerState GetPlayerState() { return playerData.state; }
bool IsPlayerFacingRight() { return playerData.isFacingRight; }
bool IsPlayerGrounded() { return playerData.isGrounded; }
bool IsPlayerAlive() { return playerData.currentHP > 0; }
int GetPlayerHP() { return playerData.currentHP; }
int GetPlayerMaxHP() { return playerData.maxHP; }
int GetPlayerAttack() { return playerData.attackPower; }

// ===== データ操作 =====

// HPにダメージを与える
void DamagePlayerHP(int damage)
{
    if (playerData.isInvincible)
    {
        return;
    }

    if (playerData.state == PlayerState::Healing)
    {
        playerData.state = PlayerState::Idle;
        playerData.healExecuted = false;
    }

    int finalDamage = (int)floor(damage * (1.0f + playerData.damageTakenRate));
    if (finalDamage < 1) finalDamage = 1;

    playerData.currentHP -= finalDamage;
    if (playerData.currentHP < 0) playerData.currentHP = 0;

    if (playerData.currentHP > 0)
    {
        PlaySE(SE_PLAYER_DAMAGE);
        playerData.isInvincible = true;
        playerData.invincibleTimer = 90; // 1.5秒 (60fps)

        if (playerData.state != PlayerState::Dodging && playerData.state != PlayerState::DiveAttack)
        {
            playerData.state = PlayerState::Hurt;
            playerData.velocityX = playerData.isFacingRight ? -HURT_KNOCKBACK_SPEED : HURT_KNOCKBACK_SPEED;
            if (playerData.isGrounded)
            {
                playerData.velocityY = -3.0f;
            }
             if (playerAnims.hurt.frames != nullptr)
             {
                 ResetAnimation(playerAnims.hurt);
             }
        }
    }
}

// HPを回復する
void HealPlayerHP(int healAmount)
{
    int finalHeal = healAmount + playerData.healPowerBonus;

    playerData.currentHP += finalHeal;
    if (playerData.currentHP > playerData.maxHP)
        playerData.currentHP = playerData.maxHP;
}

// ===== パッシブアビリティ =====

void UnlockDoubleJump() { playerData.hasDoubleJump = true; }
bool HasDoubleJump() { return playerData.hasDoubleJump; }
void UnlockDiveAttack() { playerData.hasDiveAttack = true; }
bool HasDiveAttack() { return playerData.hasDiveAttack; }
void OnCatCombatDefeated() { UnlockDiveAttack(); }

// ===== 回復関連 ===== GetHealCount() { return playerData.healCount; }
// 残り回復回数取得
int GetHealCount()
{
    return GetPlayerData().healCount;
}

// 最大回復回数取得
int GetMaxHealCount()
{
    return GetPlayerData().maxHealCount;
}

// ===== 回避関連 =====
void TryHeal()
{
    if (playerData.healCount > 0 && 
        playerData.isGrounded && 
        playerData.state != PlayerState::Healing &&
        playerData.state != PlayerState::UsingSkill)
    {
        playerData.velocityX = 0.0f;
        currentMoveDir = 0;
        runAnimState = RunAnimState::None;
        playerData.state = PlayerState::Healing;
        playerData.healExecuted = false;
        ResetAnimation(playerAnims.healing);
    }
}

// ===== 回避関連 =====

bool IsPlayerInvincible() { return playerData.isInvincible; }

float GetDeathSlowMotionScale()
{
    if (g_DeathSlowMoTimer > 0.0f)
    {
        return DEATH_SLOWMO_SCALE;
    }
    return 1.0f;
}

void TryDodge()
{
    if (playerData.dodgeCooldown > 0 ||
        playerData.state == PlayerState::Healing ||
        playerData.state == PlayerState::UsingSkill ||
        playerData.state == PlayerState::Dodging)
    {
        return;
    }

    playerData.state = PlayerState::Dodging;
    PlaySE(SE_PLAYER_DASH);
    ResetAnimation(playerAnims.dodge);
    playerData.isInvincible = true;
    
    float dashDirection = playerData.isFacingRight ? 1.0f : -1.0f;
    playerData.velocityX = dashDirection * (DODGE_DISTANCE / 6.0f);
    playerData.dodgeCooldown = DODGE_COOLDOWN;
    runAnimState = RunAnimState::None;
    
    playerData.showDashEffect = true;
    playerData.dashEffectX = playerData.posX;
    playerData.dashEffectY = playerData.posY;
    playerData.dashEffectFacingRight = playerData.isFacingRight;
    ResetAnimation(playerAnims.dashEffect);
}

// ===== 内部関数 =====
namespace
{
    void ProcessInput()
    {
        // 座ってる間は操作禁止
        if (IsPlayerSitting())
        {
            playerData.velocityX = 0.0f;
            return;
        }
        ProcessMovement();
        ProcessSkills();
    }

    void ProcessMovement()
    {
        if (playerData.state == PlayerState::Dodging || playerData.state == PlayerState::DiveAttack || playerData.state == PlayerState::Hurt || playerData.state == PlayerState::Death)
            return;

        if (GetActiveSlashSkill() != nullptr)
        {
            currentMoveDir = 0;
            playerData.velocityX = 0.0f;
            return;
        }

        if (playerData.state == PlayerState::Healing)
        {
            playerData.velocityX = 0.0f;
            currentMoveDir = 0;
            if (IsTriggerKey(KEY_JUMP))
            {
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
            }
            else return;
        }

        currentMoveDir = 0;
        if (IsInputKey(KEY_LEFT))  currentMoveDir -= 1;
        if (IsInputKey(KEY_RIGHT)) currentMoveDir += 1;

        playerData.velocityX = currentMoveDir * MOVE_SPEED;

        if (currentMoveDir > 0) playerData.isFacingRight = true;
        else if (currentMoveDir < 0) playerData.isFacingRight = false;

        if (IsTriggerKey(KEY_JUMP))
        {
            if (IsInputKey(KEY_DOWN))
            {
                playerData.dropThrough = true;
                playerData.dropTimer = 15;
            }
            else ExecuteJump();
        }

        if (playerData.hasDiveAttack && !playerData.isGrounded && IsInputKey(KEY_DOWN) && IsTriggerKey(KEY_DIVE_ATTACK))
        {
            playerData.state = PlayerState::DiveAttack;
            PlaySE(SE_PLAYER_FALL_ATTACK);
            playerData.isDiveAttacking = true;
            playerData.velocityX = 0.0f;
            playerData.velocityY = playerData.diveAttackSpeed;
            g_DiveAttackLockFrames = DIVE_ATTACK_MIN_ACTIVE_FRAMES;
            g_DiveAttackRecoveryFrames = -1;
            g_DiveAttackLanded = false;
            g_DiveAttackColliderRestored = false;
            runAnimState = RunAnimState::None;
            ResetAnimation(playerAnims.diveAttack);
            SetAnimationFrame(playerAnims.diveAttack, 0);
        }
    }

    // スキルの発動
    void ProcessSkills()
    {
        if (playerData.state == PlayerState::DiveAttack || playerData.state == PlayerState::Dodging || playerData.state == PlayerState::Hurt || playerData.state == PlayerState::Death)
            return;

        if (IsTriggerKey(KEY_SKILL1))
        {
            int set = g_SkillManager.GetCurrentSet();
            int id = g_SkillManager.GetEquipSkill(set, 0);

            if (id != -1)
            {
                g_SkillManager.UseSkill(0, &playerData);

                Skill* activeSlash = GetActiveSlashSkill();
                if (activeSlash != nullptr)
                {
                    playerData.state = PlayerState::UsingSkill;
                    int combo = activeSlash->GetComboIndex();
                    if (combo == 0) PlaySE(SE_PLAYER_ATTACK1);
                    else if (combo == 1) PlaySE(SE_PLAYER_ATTACK2);
                    else PlaySE(SE_PLAYER_ATTACK3);
                }
                else if (g_SkillManager.GetSkillTypeInSlot(0) == SkillType::Attack)
                {
                    playerData.state = PlayerState::UsingSkill;
                }
            }
        }
        else if (IsTriggerKey(KEY_SKILL2))
        {
            int set = g_SkillManager.GetCurrentSet();
            int id = g_SkillManager.GetEquipSkill(set, 1);

            if (id != -1)
            {
                g_SkillManager.UseSkill(1, &playerData);
                if (g_SkillManager.GetSkillTypeInSlot(1) == SkillType::Attack)
                {
                    playerData.state = PlayerState::UsingSkill;
                }
            }
        }
        else if (IsTriggerKey(KEY_SKILL3))
        {
            int set = g_SkillManager.GetCurrentSet();
            int id = g_SkillManager.GetEquipSkill(set, 2);

            if (id != -1)
            {
                g_SkillManager.UseSkill(2, &playerData);
                if (g_SkillManager.GetSkillTypeInSlot(2) == SkillType::Attack)
                {
                    playerData.state = PlayerState::UsingSkill;
                }
            }
        }

        if (IsTriggerKey(KEY_CHANGE)) g_SkillManager.ChangeSet();
        if (IsTriggerKey(KEY_HEAL)) TryHeal();
        if (IsTriggerKey(KEY_DODGE)) TryDodge();
    }

    void UpdatePhysics()
    {
        Skill* activeSlashSkill = GetActiveSlashSkill();
        if (activeSlashSkill != nullptr && !playerData.isGrounded && playerData.state != PlayerState::Death)
        {
            if (g_LastSlashComboIndex != activeSlashSkill->GetComboIndex())
            {
                g_LastSlashComboIndex = activeSlashSkill->GetComboIndex();
                if (playerData.velocityY > -1.0f)
                {
                    playerData.velocityY = -1.0f;
                }
            }
        }
        else
        {
            g_LastSlashComboIndex = -1;
        }

        if (playerData.state == PlayerState::Death)
        {
            const float slowMo = (g_DeathSlowMoTimer > 0.0f) ? DEATH_SLOWMO_SCALE : 1.0f;

            playerData.velocityX *= DEATH_KNOCKBACK_DAMP;
            if (std::fabs(playerData.velocityX) < 0.1f)
            {
                playerData.velocityX = 0.0f;
            }

            if (!playerData.isGrounded)
            {
                playerData.velocityY += GRAVITY * slowMo;
                if (playerData.velocityY > MAX_FALL_SPEED) playerData.velocityY = MAX_FALL_SPEED;
            }

            // 横ノックバックは見た目優先でスローモーションの影響を受けない
            playerData.posX += playerData.velocityX;
            playerData.posY += playerData.velocityY * slowMo;

            if (g_playerColliderId != -1)
            {
                UpdatePlayerColliderNormal(g_playerColliderId, playerData.posX, playerData.posY,
                    (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
            }

            ResolveCollisions();

            if (playerData.isGrounded && playerData.velocityY > 0.0f)
                playerData.velocityY = 0.0f;

            return;
        }

        if (playerData.state == PlayerState::Hurt)
        {
            playerData.velocityX *= HURT_KNOCKBACK_DAMP;
            if (std::fabs(playerData.velocityX) < 0.1f)
            {
                playerData.velocityX = 0.0f;
            }
        }

        if (playerData.state == PlayerState::DiveAttack)
        {
            if (g_DiveAttackLockFrames > 0)
            {
                g_DiveAttackLockFrames--;
            }

            if (!g_DiveAttackLanded)
            {
                playerData.velocityY = playerData.diveAttackSpeed;
                playerData.posX += playerData.velocityX;
                playerData.posY += playerData.velocityY;
            }
            else
            {
                playerData.velocityX = 0.0f;
                playerData.velocityY = 0.0f;
            }

            // コライダー位置更新（落下攻撃時も通常判定を使用）
            if (g_playerColliderId != -1)
            {
                UpdatePlayerColliderNormal(g_playerColliderId, playerData.posX, playerData.posY,
                                           (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
            }

            ResolveCollisions();

            // 着地で衝撃波区間へ移行（着地前には出さない）
            if (!g_DiveAttackLanded && playerData.isGrounded && g_DiveAttackLockFrames <= 0)
            {
                // 勝手に追加しときましたb
                // DiveAttack 床破壊処理
                float left = playerData.posX - DIVE_ATTACK_WIDTH * 0.5f;
                float right = playerData.posX + DIVE_ATTACK_WIDTH * 0.5f;
                float y = playerData.posY + 4;

                int mapStartX = (int)(left / MAP_CHIP_WIDTH);
                int mapEndX = (int)(right / MAP_CHIP_WIDTH);
                int mapY = (int)(y / MAP_CHIP_HEIGHT);

                for (int x = mapStartX; x <= mapEndX; x++)
                {
                    DamageMapChip(x, mapY, playerData.diveAttackDamage, true);
                }

                g_DiveAttackLanded = true;
                playerData.velocityY = 0.0f;

                if (playerAnims.diveAttack.frames != nullptr && playerAnims.diveAttack.frameCount > DIVE_ATTACK_LAND_START_FRAME)
                {
                    if (playerAnims.diveAttack.currentFrame < DIVE_ATTACK_LAND_START_FRAME)
                    {
                        SetAnimationFrame(playerAnims.diveAttack, DIVE_ATTACK_LAND_START_FRAME);
                    }
                }
            }
            return;
        }

        if (!playerData.isGrounded)
        {
            playerData.velocityY += GRAVITY;
            if (playerData.velocityY > MAX_FALL_SPEED) playerData.velocityY = MAX_FALL_SPEED;
        }

        playerData.posX += playerData.velocityX;
        playerData.posY += playerData.velocityY;

        // コライダー位置更新
        if (g_playerColliderId != -1)
        {
            UpdatePlayerColliderNormal(g_playerColliderId, playerData.posX, playerData.posY, 
                                       (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
        }

        // 衝突解決（ブロックなどと干渉していればここで押し出し等が行われる）
        ResolveCollisions();

        if (playerData.isGrounded && playerData.velocityY > 0.0f)
            playerData.velocityY = 0.0f;

        const int mapW = GetMapWidth();
        const int mapH = GetMapHeight();
        
        if (mapH > 0 && playerData.posY > mapH)
        {
            playerData.posY = (float)mapH;
            playerData.velocityY = 0.0f;
            playerData.isGrounded = true;
            playerData.jumpCount = 0;
        }

        if (playerData.posX < 0.0f) playerData.posX = 0.0f;
        if (mapW > 0 && playerData.posX > mapW) playerData.posX = (float)mapW;

        if (playerData.dropTimer > 0) playerData.dropTimer--;
        else playerData.dropThrough = false;

        if (playerData.dodgeCooldown > 0) playerData.dodgeCooldown--;
    }

    void UpdateState()
    {
        if (playerData.state == PlayerState::Death)
        {
            // 死亡アニメ終了後も Death 状態を維持（復活処理側で遷移）
            prevIsGrounded = playerData.isGrounded;
            return;
        }

        if (playerData.state == PlayerState::DiveAttack)
        {
            if (!g_DiveAttackLanded)
            {
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            const bool hasDiveAnim = (playerAnims.diveAttack.frameCount > 0 && playerAnims.diveAttack.frames != nullptr);
            const bool diveAnimEnded = hasDiveAnim ? IsAnimationFinished(playerAnims.diveAttack) : true;

            if (diveAnimEnded)
            {
                if (g_DiveAttackRecoveryFrames < 0)
                {
                    g_DiveAttackRecoveryFrames = DIVE_ATTACK_RECOVERY_FRAMES;
                }

                if (g_DiveAttackRecoveryFrames > 0)
                {
                    g_DiveAttackRecoveryFrames--;
                    prevIsGrounded = playerData.isGrounded;
                    return;
                }

                // ダイブアタック側に復帰モーション(34〜37)が含まれるため
                // Land ではなく Idle に戻す
                playerData.state = PlayerState::Idle;
                playerData.isDiveAttacking = false;
                g_DiveAttackRecoveryFrames = -1;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            prevIsGrounded = playerData.isGrounded;
            return;
        }

        if (playerData.state == PlayerState::Dodging)
        {
            if (playerAnims.dodge.frameCount == 0 || playerAnims.dodge.frames == nullptr)
            {
                playerData.state = playerData.isGrounded ? PlayerState::Idle : PlayerState::Fall;
                playerData.isInvincible = false;
                playerData.velocityX = 0.0f;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            if (IsAnimationFinished(playerAnims.dodge))
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

        if (playerData.state == PlayerState::Hurt)
        {
            if (playerAnims.hurt.frames == nullptr || playerAnims.hurt.frameCount == 0 || IsAnimationFinished(playerAnims.hurt))
            {
                playerData.state = playerData.isGrounded ? PlayerState::Idle : PlayerState::Fall;
            }
            prevIsGrounded = playerData.isGrounded;
            return;
        }

        if (playerData.state == PlayerState::Healing)
        {
            if (IsTriggerKey(KEY_LEFT) || IsTriggerKey(KEY_RIGHT))
            {
                playerData.velocityX = 0.0f;
                currentMoveDir = 0;
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            if (IsAnimationFinished(playerAnims.healing))
            {
                playerData.state = PlayerState::Idle;
                playerData.healExecuted = false;
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            if (!playerData.healExecuted && playerAnims.healing.currentFrame == 13)
            {
                if (playerData.currentHP < playerData.maxHP)
                {
                    HealPlayerHP(playerData.basehealPower);
                    playerData.healCount--;
                    PlaySE(SE_PLAYER_HEAL);
                }
                playerData.healExecuted = true;
            }

            prevIsGrounded = playerData.isGrounded;
            return;
        }
        // ここがreturnしかしていなかったのでスキルが終わらなかった。
        // 終わるように書きました。
        if (playerData.state == PlayerState::UsingSkill)
        {
            bool anySkillActive = false;

            for (auto& s : g_SkillManager.GetSkills())
            {
                if (s->IsActive())
                {
                    anySkillActive = true;
                    break;
                }
            }

            // スキル終了
            if (!anySkillActive)
            {
                playerData.state = playerData.isGrounded ? PlayerState::Idle : PlayerState::Fall;
            }

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

            // 前フレームが空中で今フレームが接地 → Land
            if (!prevIsGrounded && playerData.state == PlayerState::Fall)
            {
                newState = PlayerState::Land;
            }
            // Landアニメ再生中は完了まで維持（移動入力がなければ）
            else if (playerData.state == PlayerState::Land && !IsAnimationFinished(playerAnims.land) && !hasMoveInput)
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
            if (playerData.state == PlayerState::Dodging)
            {
                prevIsGrounded = playerData.isGrounded;
                return;
            }

            playerData.state = newState;
            runAnimState = RunAnimState::None;

            if (playerData.state == PlayerState::Jump)
            {
                ResetAnimation(playerAnims.jump);
            }
            else if (playerData.state == PlayerState::Fall)
            {
                ResetAnimation(playerAnims.fall);
            }
            else if (playerData.state == PlayerState::Land)
            {
                ResetAnimation(playerAnims.land);
            }
        }

        prevIsGrounded = playerData.isGrounded;
    }

    void UpdatePlayerAnimation()
    {
        float absVelX = std::fabs(playerData.velocityX);
        bool wasMoving = prevAbsVelX > 0.01f;

        if (playerData.state == PlayerState::DiveAttack)
        {
            if (playerAnims.diveAttack.frames != nullptr && playerAnims.diveAttack.frameCount > 0)
            {
                UpdateAnimation(playerAnims.diveAttack);

                if (!g_DiveAttackLanded && playerAnims.diveAttack.currentFrame > DIVE_ATTACK_AIR_END_FRAME)
                {
                    SetAnimationFrame(playerAnims.diveAttack, DIVE_ATTACK_AIR_END_FRAME);
                }
            }
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::Dodging)
        {
            UpdateAnimation(playerAnims.dodge);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::Hurt)
        {
            UpdateAnimation(playerAnims.hurt);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::Death)
        {
            UpdateAnimation(playerAnims.death);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::UsingSkill)
        {
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::Healing)
        {
            UpdateAnimation(playerAnims.healing);
            prevFacingRight = playerData.isFacingRight;
            prevAbsVelX = absVelX;
            return;
        }

        if (playerData.state == PlayerState::Jump)
        {
            UpdateAnimation(playerAnims.jump);
        }
        else if (playerData.state == PlayerState::Fall)
        {
            UpdateAnimation(playerAnims.fall);
        }
        else if (playerData.state == PlayerState::Land)
        {
            UpdateAnimation(playerAnims.land);
        }
        else if (playerData.state == PlayerState::Walk && (currentMoveDir != 0))
        {
            if (playerData.state == PlayerState::Walk &&
                currentMoveDir != 0 &&
                playerData.isGrounded)
            {
                if (!wasMoving)
                {
                    PlaySE(SE_PLAYER_RUN, true);
                }
            }
            if (!wasMoving && playerAnims.runStart.frames != nullptr)
            {
                runAnimState = RunAnimState::Start;
                ResetAnimation(playerAnims.runStart);
            }

            if (runAnimState == RunAnimState::Start && playerAnims.runStart.frames != nullptr)
            {
                UpdateAnimation(playerAnims.runStart);
                if (IsAnimationFinished(playerAnims.runStart))
                {
                    runAnimState = RunAnimState::Run;
                    ResetAnimation(playerAnims.walk);
                }
            }
            else
            {
                runAnimState = RunAnimState::Run;

                int speed = (int)(10.0f - (absVelX / MOVE_SPEED) * 8.0f);
                if (speed < 2) speed = 2;
                if (speed > 10) speed = 10;
                SetAnimationSpeed(playerAnims.walk, speed);

                UpdateAnimation(playerAnims.walk);
            }
        }
        else
        {
			// SEを止める
            bool shouldPlayRunSE = (playerData.isGrounded && currentMoveDir != 0);

            if (shouldPlayRunSE)
            {
                if (!wasMoving)
                {
                    PlaySE(SE_PLAYER_RUN, true);
                }
            }
            else
            {
                StopSE(SE_PLAYER_RUN);
            }

            if (playerData.isGrounded && wasMoving && playerAnims.runStop.frames != nullptr)
            {
                runAnimState = RunAnimState::Stop;
                ResetAnimation(playerAnims.runStop);
            }

            if (runAnimState == RunAnimState::Stop && playerAnims.runStop.frames != nullptr)
            {
                UpdateAnimation(playerAnims.runStop);
                if (IsAnimationFinished(playerAnims.runStop))
                {
                    runAnimState = RunAnimState::None;

                    if (playerData.state == PlayerState::Idle)
                    {
                        ResetAnimation(playerAnims.idle);
                    }
                    else if (playerData.state == PlayerState::Walk)
                    {
                        ResetAnimation(playerAnims.walk);
                    }
                }
            }
            else
            {
                if (playerData.state == PlayerState::Idle || playerData.state == PlayerState::UsingSkill)
                {
                    UpdateAnimation(playerAnims.idle);
                }
            }
        }

        prevFacingRight = playerData.isFacingRight;
        prevAbsVelX = absVelX;
    }

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

        int maxJumps = playerData.hasDoubleJump ? MAX_JUMP_COUNT : 1;

        if (playerData.jumpCount < maxJumps)
        {
            playerData.velocityY = -JUMP_POWER;
            playerData.isGrounded = false;
            playerData.jumpCount++;
            PlaySE(SE_PLAYER_JUMP);
            lastJumpInputFrame = inputFrame;
            ResetAnimation(playerAnims.jump);
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
                    UpdatePlayerColliderNormal(g_playerColliderId, playerData.posX, playerData.posY, 
                                               (float)PLAYER_WIDTH, (float)PLAYER_HEIGHT);
                }
                return true;
            }
        }

        return false;
    }

    Skill* GetActiveSlashSkill()
    {
        for (auto& s : g_SkillManager.GetSkills())
        {
            if (s->IsActive() && s->GetType() == SkillType::Attack && s->GetID() == 1)
            {
                return s.get();
            }
        }
        return nullptr;
    }
}