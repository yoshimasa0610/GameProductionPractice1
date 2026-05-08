#include "MidBossBase.h"
#include "../Animation/Animation.h"
#include "../Camera/Camera.h"
#include "../Collision/Collision.h"
#include "../Map/MapManager.h"
#include "../Player/Player.h"
#include "../Enemy/EnemyBase.h"
#include "StoneGolem/StoneGolem.h"
#include "Garok/Garok.h"
#include "DxLib.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace
{
    //============================================================
    // ミッドボスプロジェクタイル（弾・ビーム）の種類定義
    //============================================================
    enum MidBossProjectileKind
    {
        StoneBullet = 0,  // 弾
        StoneBeam = 1     // ビーム
    };

    //============================================================
    // ミッドボスプロジェクタイル（弾・ビーム）のデータ構造
    //============================================================
    struct MidBossProjectile
    {
        bool active = false;              // アクティブフラグ
        int kind = 0;                     // StoneBullet or StoneBeam
        float posX = 0.0f, posY = 0.0f;   // 位置
        float velocityX = 0.0f, velocityY = 0.0f;  // 速度
        float width = 0.0f, height = 0.0f;         // 描画サイズ
        float colliderWidth = 0.0f, colliderHeight = 0.0f;  // 当たり判定サイズ
        float speed = 0.0f;               // 速度（ノルム）
        float lifeTimer = 0.0f;           // 生存時間カウント
        bool homing = false;              // ホーミング可否（弾のみ）
        float homingTurnRate = 0.0f;      // ホーミング角速度
        bool facingRight = true;          // 右向きか
        int colliderId = -1;              // 当たり判定ID
        int animCounter = 0;              // アニメーション進行度（ビームのフレーム選択用）
    };

    //============================================================
    // ミッドボス（敵ボス）のデータ構造
    //============================================================
    struct MidBossData
    {
        bool active = false;              // アクティブフラグ
        MidBossType type = MidBossType::StoneGolem;  // ボスの種類
        float posX = 0.0f, posY = 0.0f;   // 位置
        float width = 160.0f, height = 160.0f;      // サイズ
        bool facingRight = true;          // 右向きか
        bool garokMoving = false;
        float garokRunAnimHold = 0.0f;
        const AnimationData* garokCurrentAnim = nullptr;

        // ステータス
        int maxHP = 420, hp = 420;        // HP
        int attackPower = 24;             // 攻撃力
        int colliderId = -1;              // 当たり判定ID

        // アニメーション
        AnimationData idle, attack, beamAttack, die;

        // Garok用アニメーション
        AnimationData garokRun, garokAttack2, garokAttack3, garokDashAttack, garokJump, garokHurt;

        // 死亡状態
        bool isDead = false;              // 死亡中フラグ
        int deathBlinkTimer = 0;          // 死亡後の点滅タイマー

        // AI状態
        bool isAggro = false;             // 戦闘状態か
        float detectRange = 520.0f;       // プレイヤー検知範囲

        // 攻撃準備状態
        bool isPreparing = false;         // 攻撃予備動作中か
        float prepareTimer = 0.0f;        // 予備動作の時間
        int pendingAttack = 0;            // 1:shot 2:beam / Garok: 1:combo 2:dash 3:barrage

        // 攻撃状態
        bool isAttacking = false;         // 攻撃モーション中か
        float attackTimer = 0.0f;         // 攻撃タイマー
        float cooldownTimer = 0.0f;       // 攻撃クールダウン

        // フェーズ2（HP半分以降）の特殊行動
        bool phase2 = false;              // フェーズ2突入フラグ
        int attackCounter = 0;            // 攻撃回数カウンタ
        bool barrageActive = false;       // 連射モード中か
        float barrageTimer = 0.0f;        // 連射の継続時間
        float barrageShotTimer = 0.0f;    // 連射の1発目までの間隔

        // Garok用の戦闘制御
        float targetLockedX = 0.0f;
        float targetLockedY = 0.0f;
        int garokComboMax = 1;
        int garokAnimStage = 1;
        int garokBarrageStep = -1;
        float garokAttackElapsed = 0.0f;
        float garokAttackHitLife = 0.0f;
        bool garokHitDone1 = false;
        bool garokHitDone2 = false;
        bool garokHitDone3 = false;
        int garokAttackColliderId = -1;
        bool garokAttackColliderFixed = false;
        float garokAttackColliderCenterX = 0.0f;
        float garokAttackColliderCenterY = 0.0f;
        float garokDashTargetX = 0.0f;
        bool garokJumping = false;
        float garokJumpTimer = 0.0f;
        float garokJumpDuration = 1.2f;
        float garokJumpStartX = 0.0f;
        float garokJumpStartY = 0.0f;
        float garokJumpTargetX = 0.0f;
        float garokJumpTargetY = 0.0f;
        float garokJumpCooldownTimer = 6.0f;
        float garokVelocityY = 0.0f;
        bool garokGrounded = false;

        EnemyData attackOwner{};          // 攻撃判定の所有者（ダメージ計算用）
    };

    // グローバル変数：ミッドボス管理
    std::vector<MidBossData> g_midBosses;        // 全ミッドボスリスト
    std::vector<MidBossProjectile> g_midProjectiles;  // 全プロジェクタイル

    //============================================================
    // グラフィックハンドル（画像読み込み）
    //============================================================
    int g_stoneSheetHandle = -1;          // キャラクタースプライトシート
    int g_stoneBulletHandle = -1;         // 弾画像
    int g_stoneBulletGlowHandle = -1;     // 発光版弾画像
    int g_stoneBeamFrames[5] = { -1, -1, -1, -1, -1 };  // ビーム5フレーム

    //============================================================
    // 攻撃・描画の個別調整値（各ボス個別CPPから参照）
    //============================================================
    const StoneGolemTuning& STONE_TUNING = GetStoneGolemTuning();
    const GarokTuning& GAROK_TUNING = GetGarokTuning();

    // 弾・ビーム発射位置をボス描画オフセットに合わせる補正
    const float STONE_PROJECTILE_OFFSET_X = STONE_TUNING.drawOffsetX;
    const float STONE_PROJECTILE_OFFSET_Y = STONE_TUNING.drawOffsetY;

    //============================================================
    // ヘルパー関数：プロジェクタイル（弾・ビーム）の管理
    //============================================================

    // プロジェクタイル1つをクリア
    void ClearProjectile(MidBossProjectile& p)
    {
        if (p.colliderId != -1)
        {
            DestroyCollider(p.colliderId);
            p.colliderId = -1;
        }
        p.active = false;
    }

    // 乱射開始時に残っている追従弾を消す
    void ClearHomingStoneBullets()
    {
        for (auto& p : g_midProjectiles)
        {
            if (!p.active) continue;
            if (p.kind == StoneBullet && p.homing)
            {
                ClearProjectile(p);
            }
        }
    }

    // 全プロジェクタイルをクリア
    void ClearProjectiles()
    {
        for (auto& p : g_midProjectiles)
        {
            ClearProjectile(p);
        }
        g_midProjectiles.clear();
    }

    //============================================================
    // グラフィックロード関数
    //============================================================

    // ストーンゴーレムのグラフィックリソースを読み込み
    void LoadStoneAssets()
    {
        // キャラクタースプライトシート（1000x1000）
        if (g_stoneSheetHandle == -1)
        {
            g_stoneSheetHandle = LoadGraph("Data/MidBoss/StoneGolem/Character_sheet.png");
        }

        // 弾画像（100x100）
        if (g_stoneBulletHandle == -1)
        {
            g_stoneBulletHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile.png");
        }

        // 発光版弾画像（300x300）
        if (g_stoneBulletGlowHandle == -1)
        {
            g_stoneBulletGlowHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile_glowing.png");
        }

        // ビームシート（300x1500）を5フレームに分割
        if (g_stoneBeamFrames[0] == -1)
        {
            int loaded[5] = { -1, -1, -1, -1, -1 };
            int result = LoadDivGraph(
                "Data/MidBoss/StoneGolem/Laser_sheet.png",
                5,        // フレーム数
                1, 5,     // 列数、行数
                300, 300, // フレーム幅、フレーム高さ
                loaded);
            if (result != -1)
            {
                for (int i = 0; i < 5; ++i)
                {
                    g_stoneBeamFrames[i] = loaded[i];
                }
            }
        }
    }

    // ストーンゴーレムのグラフィックリソースを解放
    void ReleaseStoneAssets()
    {
        if (g_stoneSheetHandle != -1)
        {
            DeleteGraph(g_stoneSheetHandle);
            g_stoneSheetHandle = -1;
        }
        if (g_stoneBulletHandle != -1)
        {
            DeleteGraph(g_stoneBulletHandle);
            g_stoneBulletHandle = -1;
        }
        if (g_stoneBulletGlowHandle != -1)
        {
            DeleteGraph(g_stoneBulletGlowHandle);
            g_stoneBulletGlowHandle = -1;
        }
        for (int i = 0; i < 5; ++i)
        {
            if (g_stoneBeamFrames[i] != -1)
            {
                DeleteGraph(g_stoneBeamFrames[i]);
                g_stoneBeamFrames[i] = -1;
            }
        }
    }

    //============================================================
    // アニメーション関数
    //============================================================

    // ストーンゴーレムのアニメーションを初期化
    // sheetは1000x1000、100x100のセルが10x10で格子状に配置
    void InitStoneAnimations(MidBossData& b)
    {
        const char* sheetPath = "Data/MidBoss/StoneGolem/Character_sheet.png";
        int test = LoadGraph(sheetPath);
        if (test != -1)
        {
            DeleteGraph(test);
        }

        // idle: 行10、8フレーム
        LoadAnimationFromSheetRange(b.idle, sheetPath, 0, 10, 8, 100, 100, 5, AnimationMode::Loop);
        // shot attack
        LoadAnimationFromSheetRange(b.attack, sheetPath, 0, 20, 8, 100, 100, 4, AnimationMode::Once);
        // beam attack: 4段目（要求モーション）
        LoadAnimationFromSheetRange(b.beamAttack, sheetPath, 0, 30, 8, 100, 100, 4, AnimationMode::Once);
        // die: 行70、12フレーム
        LoadAnimationFromSheetRange(b.die, sheetPath, 0, 70, 12, 100, 100, 5, AnimationMode::Once);
    }

    // Garokのアニメーション初期化
    void InitGarokAnimations(MidBossData& b)
    {
        LoadAnimationFromSheet(b.idle, "Data/MidBoss/Garok/IDLE.png", 6, 192, 64, 5, AnimationMode::Loop);
        LoadAnimationFromSheet(b.garokRun, "Data/MidBoss/Garok/RUN.png", 6, 192, 64, 4, AnimationMode::Loop);
        LoadAnimationFromSheet(b.attack, "Data/MidBoss/Garok/ATTACK 1.png", 6, 192, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokAttack2, "Data/MidBoss/Garok/ATTACK 2.png", 4, 192, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokAttack3, "Data/MidBoss/Garok/ATTACK 3.png", 5, 192, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokDashAttack, "Data/MidBoss/Garok/DASH ATTACK.png", 12, 192, 64, 3, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokJump, "Data/MidBoss/Garok/JUMP.png", 3, 192, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokHurt, "Data/MidBoss/Garok/HURT.png", 6, 192, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.die, "Data/MidBoss/Garok/DEATH.png", 8, 192, 64, 4, AnimationMode::Once);
    }

    void ClearGarokAttackCollider(MidBossData& b)
    {
        if (b.garokAttackColliderId != -1)
        {
            DestroyCollider(b.garokAttackColliderId);
            b.garokAttackColliderId = -1;
        }
        b.garokAttackColliderFixed = false;
    }

    void GetGarokAttackColliderCenter(const MidBossData& b, float& centerX, float& centerY)
    {
        if (b.garokAttackColliderFixed)
        {
            centerX = b.garokAttackColliderCenterX;
            centerY = b.garokAttackColliderCenterY;
            return;
        }

        const float front = b.width * GAROK_TUNING.hitboxFrontRatio;
        centerX = b.posX + (b.facingRight ? front : -front);
        centerY = b.posY - b.height * GAROK_TUNING.contactYRatio;
    }

    void GetGarokLockedTargetAttackCenter(const MidBossData& b, float& centerX, float& centerY)
    {
        const float attackW = b.width * GAROK_TUNING.hitboxWidthRatio;
        const float dir = b.facingRight ? 1.0f : -1.0f;
        const float minOffset = attackW * 0.25f;
        const float maxOffset = b.width * GAROK_TUNING.hitboxFrontRatio + attackW * 0.35f;
        float targetOffset = (b.targetLockedX - b.posX) * dir;
        targetOffset = (std::max)(minOffset, (std::min)(maxOffset, targetOffset));

        centerX = b.posX + dir * targetOffset;

        const float targetCenterY = b.targetLockedY - PLAYER_HEIGHT * 0.5f;
        const float minY = b.posY - b.height * 0.82f;
        const float maxY = b.posY - b.height * 0.18f;
        centerY = (std::max)(minY, (std::min)(maxY, targetCenterY));
    }

    void UpdateGarokAttackCollider(MidBossData& b)
    {
        if (b.garokAttackColliderId == -1) return;

        const float attackW = b.width * GAROK_TUNING.hitboxWidthRatio;
        const float attackH = b.height * GAROK_TUNING.hitboxHeightRatio;
        float centerX = 0.0f;
        float centerY = 0.0f;
        GetGarokAttackColliderCenter(b, centerX, centerY);
        UpdateCollider(
            b.garokAttackColliderId,
            centerX - attackW * 0.5f,
            centerY - attackH * 0.5f,
            attackW,
            attackH);
    }

    void ActivateGarokAttackCollider(MidBossData& b)
    {
        b.garokAttackColliderFixed = false;
        const float attackW = b.width * GAROK_TUNING.hitboxWidthRatio;
        const float attackH = b.height * GAROK_TUNING.hitboxHeightRatio;
        float centerX = 0.0f;
        float centerY = 0.0f;
        GetGarokAttackColliderCenter(b, centerX, centerY);

        if (b.garokAttackColliderId == -1)
        {
            b.garokAttackColliderId = CreateCollider(
                ColliderTag::Attack,
                centerX - attackW * 0.5f,
                centerY - attackH * 0.5f,
                attackW,
                attackH,
                &b.attackOwner);
        }
        else
        {
            UpdateCollider(
                b.garokAttackColliderId,
                centerX - attackW * 0.5f,
                centerY - attackH * 0.5f,
                attackW,
                attackH);
        }
    }

    void ActivateGarokAttackColliderAt(MidBossData& b, float centerX, float centerY)
    {
        const float attackW = b.width * GAROK_TUNING.hitboxWidthRatio;
        const float attackH = b.height * GAROK_TUNING.hitboxHeightRatio;
        b.garokAttackColliderFixed = true;
        b.garokAttackColliderCenterX = centerX;
        b.garokAttackColliderCenterY = centerY;

        if (b.garokAttackColliderId == -1)
        {
            b.garokAttackColliderId = CreateCollider(
                ColliderTag::Attack,
                centerX - attackW * 0.5f,
                centerY - attackH * 0.5f,
                attackW,
                attackH,
                &b.attackOwner);
        }
        else
        {
            UpdateCollider(
                b.garokAttackColliderId,
                centerX - attackW * 0.5f,
                centerY - attackH * 0.5f,
                attackW,
                attackH);
        }
    }

    void GetGarokArenaBounds(float& left, float& right)
    {
        left = GAROK_TUNING.arenaLeft + 32.0f;
        right = GAROK_TUNING.arenaRight - 32.0f;

        const int mapWidth = GetMapWidth();
        if (mapWidth > 64)
        {
            left = 32.0f;
            right = static_cast<float>(mapWidth) - 32.0f;
        }

        if (right < left)
        {
            right = left;
        }
    }

    float RandomGarokLandingX()
    {
        float arenaLeft = 0.0f;
        float arenaRight = 0.0f;
        GetGarokArenaBounds(arenaLeft, arenaRight);

        const int minX = static_cast<int>(arenaLeft + 96.0f);
        const int maxX = static_cast<int>(arenaRight - 96.0f);
        const int range = maxX - minX;
        if (range <= 0) return static_cast<float>(minX);

        return static_cast<float>(minX + GetRand(range));
    }

    bool IsGarokSolidAt(float worldX, float worldY)
    {
        const int gx = static_cast<int>(std::floor(worldX / MAP_CHIP_WIDTH));
        const int gy = static_cast<int>(std::floor(worldY / MAP_CHIP_HEIGHT));
        MapChipData* mc = GetMapChipData(gx, gy);
        if (mc == nullptr) return false;
        return (mc->mapChip == NORMAL_BLOCK || mc->mapChip == SEMI_SOLID_BLOCK);
    }

    void MoveGarokXWithCollision(MidBossData& b, float moveX)
    {
        if (std::fabs(moveX) < 0.001f) return;
        b.posX += moveX;
    }

    void ApplyGarokGravityAndMapCollision(MidBossData& b)
    {
        if (b.garokJumping) return;

        b.garokGrounded = false;
        b.garokVelocityY += GAROK_TUNING.gravity * GetDeathSlowMotionScale();
        if (b.garokVelocityY > GAROK_TUNING.maxFallSpeed) b.garokVelocityY = GAROK_TUNING.maxFallSpeed;

        float nextY = b.posY + b.garokVelocityY * GetDeathSlowMotionScale();
        const float sampleLeft = b.posX - b.width * 0.18f;
        const float sampleCenter = b.posX;
        const float sampleRight = b.posX + b.width * 0.18f;

        if (b.garokVelocityY >= 0.0f)
        {
            const float feetY = nextY + 1.0f;
            if (IsGarokSolidAt(sampleLeft, feetY) || IsGarokSolidAt(sampleCenter, feetY) || IsGarokSolidAt(sampleRight, feetY))
            {
                const int gy = static_cast<int>(std::floor(feetY / MAP_CHIP_HEIGHT));
                nextY = gy * MAP_CHIP_HEIGHT;
                b.garokVelocityY = 0.0f;
                b.garokGrounded = true;
            }
        }
        else
        {
            const float headY = nextY - b.height;
            if (IsGarokSolidAt(sampleLeft, headY) || IsGarokSolidAt(sampleCenter, headY) || IsGarokSolidAt(sampleRight, headY))
            {
                const int gy = static_cast<int>(std::floor(headY / MAP_CHIP_HEIGHT));
                nextY = (gy + 1) * MAP_CHIP_HEIGHT + b.height;
                b.garokVelocityY = 0.0f;
            }
        }

        b.posY = nextY;
    }

    float FindGroundYBelow(float worldX, float fromY, float fallbackY)
    {
        const float block = 32.0f;
        const int gx = static_cast<int>(worldX / block);
        int startY = static_cast<int>(fromY / block);
        if (startY < 0) startY = 0;

        for (int y = startY; y < startY + 48; ++y)
        {
            MapChipData* mc = GetMapChipData(gx, y);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                return y * block;
            }
        }
        const int mapHeight = GetMapHeight();
        return (mapHeight > 0) ? static_cast<float>(mapHeight) : fallbackY;
    }

    //============================================================
    // プロジェクタイル生成関数
    //============================================================

    // 弾を発射
    void SpawnStoneBullet(MidBossData& b, float startX, float startY, float targetX, float targetY, bool homing)
    {
        // 方向を計算
        float dx = targetX - startX;
        float dy = targetY - startY;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
        {
            dx = b.facingRight ? 1.0f : -1.0f;
            dy = 0.0f;
            len = 1.0f;
        }

        // 弾データを作成
        MidBossProjectile p{};
        p.active = true;
        p.kind = StoneBullet;
        p.posX = startX;
        p.posY = startY;
        p.width = STONE_TUNING.bulletDrawSize;
        p.height = STONE_TUNING.bulletDrawSize;
        p.colliderWidth = STONE_TUNING.bulletHitboxSize;
        p.colliderHeight = STONE_TUNING.bulletHitboxSize;
        p.speed = STONE_TUNING.bulletSpeed;
        p.velocityX = (dx / len) * p.speed;
        p.velocityY = (dy / len) * p.speed;
        p.lifeTimer = 4.0f;
        p.homing = homing;                          // ホーミング有無は引数で指定
        p.homingTurnRate = homing ? STONE_TUNING.homingTurnRate : 0.0f;   // ホーミング時のみ回転速度を設定
        p.facingRight = (p.velocityX >= 0.0f);

        // 当たり判定を作成
        p.colliderId = CreateCollider(
            ColliderTag::Attack,
            p.posX - p.colliderWidth * 0.5f,
            p.posY - p.colliderHeight * 0.5f,
            p.colliderWidth,
            p.colliderHeight,
            &b.attackOwner);

        g_midProjectiles.push_back(p);
    }

    // ビームを発射方向の壁端まで伸ばしたときの長さを計算
    float ComputeStoneBeamLength(float originX, float originY, bool facingRight, float beamHeight)
    {
        const float minBeamLength = 340.0f;
        const int mapWidth = GetMapWidth();
        if (mapWidth <= 0 || g_MapChipXNum <= 0 || g_MapChipYNum <= 0)
        {
            return minBeamLength;
        }

        const int dir = facingRight ? 1 : -1;
        const int startXIndex = static_cast<int>(std::floor(originX / MAP_CHIP_WIDTH));

        int minYIndex = static_cast<int>(std::floor((originY - beamHeight * 0.5f) / MAP_CHIP_HEIGHT));
        int maxYIndex = static_cast<int>(std::floor((originY + beamHeight * 0.5f - 1.0f) / MAP_CHIP_HEIGHT));
        minYIndex = (std::max)(0, minYIndex);
        maxYIndex = (std::min)(g_MapChipYNum - 1, maxYIndex);

        float wallX = facingRight ? static_cast<float>(mapWidth) : 0.0f;

        for (int x = startXIndex + dir; x >= 0 && x < g_MapChipXNum; x += dir)
        {
            bool hitWall = false;
            for (int y = minYIndex; y <= maxYIndex; ++y)
            {
                MapChipData* mc = GetMapChipData(x, y);
                if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
                {
                    hitWall = true;
                    break;
                }
            }

            if (hitWall)
            {
                wallX = facingRight ? (x * MAP_CHIP_WIDTH) : ((x + 1) * MAP_CHIP_WIDTH);
                break;
            }
        }

        float beamLength = std::fabs(wallX - originX);
        if (beamLength < minBeamLength)
        {
            beamLength = minBeamLength;
        }
        return beamLength;
    }

    // ビームを発射
    void SpawnStoneBeam(MidBossData& b, float targetX)
    {
        MidBossProjectile p{};
        p.active = true;
        p.kind = StoneBeam;
        p.height = 96.0f;

        const float baseX = b.posX + STONE_PROJECTILE_OFFSET_X;
        const float baseY = b.posY + STONE_PROJECTILE_OFFSET_Y;
        p.facingRight = (targetX >= baseX);

        // ボスの目の位置からビームを発射
        const float dir = p.facingRight ? 1.0f : -1.0f;
        const float eyeX = baseX + dir * (b.width * 0.12f + STONE_TUNING.beamMuzzleFineTuneX);
        const float eyeY = baseY - b.height * 0.78f;

        const float beamLength = ComputeStoneBeamLength(eyeX, eyeY, p.facingRight, p.height);
        p.width = beamLength;
        p.colliderWidth = p.width;
        p.colliderHeight = p.height;
        p.posX = eyeX + (p.facingRight ? (beamLength * 0.5f) : -(beamLength * 0.5f));
        p.posY = eyeY;

        p.lifeTimer = STONE_TUNING.beamActive;
        p.colliderId = CreateCollider(
            ColliderTag::Attack,
            p.posX - p.colliderWidth * 0.5f,
            p.posY - p.colliderHeight * 0.5f,
            p.colliderWidth,
            p.colliderHeight,
            &b.attackOwner);
        g_midProjectiles.push_back(p);
    }
}

//============================================================
// 外部API：ミッドボスシステムの初期化・更新・描画・クリア
//============================================================

// システムの初期化
void InitMidBossSystem()
{
    g_midBosses.clear();
    ClearProjectiles();
    LoadStoneAssets();
}

// ステージ遷移時などのクリア処理
void ClearMidBosses()
{
    for (auto& b : g_midBosses)
    {
        if (b.colliderId != -1)
        {
            DestroyCollider(b.colliderId);
            b.colliderId = -1;
        }
        ClearGarokAttackCollider(b);
        UnloadAnimation(b.idle);
        UnloadAnimation(b.attack);
        UnloadAnimation(b.beamAttack);
        UnloadAnimation(b.garokRun);
        UnloadAnimation(b.garokAttack2);
        UnloadAnimation(b.garokAttack3);
        UnloadAnimation(b.garokDashAttack);
        UnloadAnimation(b.garokJump);
        UnloadAnimation(b.garokHurt);
        UnloadAnimation(b.die);
    }
    g_midBosses.clear();
    ClearProjectiles();
    ReleaseStoneAssets();
}

//============================================================
// ミッドボス生成
//============================================================

// ミッドボスをスポーン（Play.cppから呼ばれる）
int SpawnMidBoss(MidBossType type, float x, float y)
{
    MidBossData b{};
    b.active = true;
    b.type = type;
    b.posX = x;
    b.posY = y;

    // ボスタイプに応じた初期化
    if (type == MidBossType::StoneGolem)
    {
        LoadStoneAssets();
        b.width = 160.0f;
        b.height = 160.0f;
        b.maxHP = GetStoneGolemMaxHP();
        b.hp = b.maxHP;
        b.attackPower = GetStoneGolemAttackPower();
        b.detectRange = 1400.0f;
        InitStoneAnimations(b);
    }
    else if (type == MidBossType::Garok)
    {
        b.width = 170.0f;
        b.height = 170.0f;
        b.maxHP = GetGarokMaxHP();
        b.hp = b.maxHP;
        b.attackPower = GetGarokAttackPower();
        b.detectRange = 1600.0f;
        b.garokJumpCooldownTimer = GAROK_TUNING.jumpInterval;
        InitGarokAnimations(b);
    }

    b.attackOwner = {};
    b.attackOwner.attackPower = b.attackPower;
    b.attackOwner.type = EnemyType::BigQuartist;

    // 当たり判定を作成（本体）
    const float bodyW = b.width * 0.42f;
    const float bodyH = b.height * 0.82f;
    const float left = b.posX - bodyW * 0.5f;
    const float top = (b.posY - b.height * 0.5f) - bodyH * 0.5f;
    b.colliderId = CreateCollider(ColliderTag::Enemy, left, top, bodyW, bodyH, nullptr);

    g_midBosses.push_back(b);
    return static_cast<int>(g_midBosses.size() - 1);
}

//============================================================
// ミッドボス更新（毎フレーム呼ばれる）
//============================================================

void UpdateMidBosses()
{
    PlayerData& player = GetPlayerData();
    const float dt = (1.0f / 60.0f) * GetDeathSlowMotionScale();

    for (auto& b : g_midBosses)
    {
        if (!b.active) continue;

        if (b.hp <= 0 && !b.isDead)
        {
            b.isDead = true;
            b.isPreparing = false;
            b.isAttacking = false;
            b.barrageActive = false;
            b.garokJumping = false;
            b.deathBlinkTimer = 0;
            ClearGarokAttackCollider(b);
            ResetAnimation(b.die);
            if (b.colliderId != -1)
            {
                DestroyCollider(b.colliderId);
                b.colliderId = -1;
            }
        }

        if (b.isDead)
        {
            UpdateAnimation(b.die);
            if (IsAnimationFinished(b.die))
            {
                b.deathBlinkTimer++;
                if (b.deathBlinkTimer >= 90)
                {
                    b.active = false;
                }
            }
            continue;
        }

        const float dx = player.posX - b.posX;
        const float dy = player.posY - b.posY;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (b.type != MidBossType::Garok || std::fabs(dx) > GAROK_TUNING.facingDeadzone)
        {
            b.facingRight = (dx >= 0.0f);
        }
        b.phase2 = (b.hp <= b.maxHP / 2);
        b.isAggro = (dist <= b.detectRange);

        if (b.type == MidBossType::Garok)
        {
            b.garokMoving = false;
            if (b.garokRunAnimHold > 0.0f) b.garokRunAnimHold -= dt;
            {
                float arenaLeft = 0.0f;
                float arenaRight = 0.0f;
                GetGarokArenaBounds(arenaLeft, arenaRight);
                b.posX = (std::max)(arenaLeft, (std::min)(arenaRight, b.posX));
            }
            ApplyGarokGravityAndMapCollision(b);

            if (b.cooldownTimer > 0.0f) b.cooldownTimer -= dt;
            if (b.garokJumpCooldownTimer > 0.0f) b.garokJumpCooldownTimer -= dt;

            if (b.garokAttackHitLife > 0.0f)
            {
                b.garokAttackHitLife -= dt;
                UpdateGarokAttackCollider(b);
                if (b.garokAttackHitLife <= 0.0f) ClearGarokAttackCollider(b);
            }
            else
            {
                ClearGarokAttackCollider(b);
            }

            if (b.garokJumping)
            {
                b.garokJumpTimer += dt;
                float t = b.garokJumpTimer / b.garokJumpDuration;
                if (t > 1.0f) t = 1.0f;
                const float arc = std::sin(t * DX_PI_F) * GAROK_TUNING.jumpHeight;
                b.posX = b.garokJumpStartX + (b.garokJumpTargetX - b.garokJumpStartX) * t;
                b.posY = b.garokJumpStartY + (b.garokJumpTargetY - b.garokJumpStartY) * t - arc;

                if (t >= 1.0f)
                {
                    b.garokJumping = false;
                    b.garokJumpCooldownTimer = GAROK_TUNING.jumpInterval;
                    b.cooldownTimer = (std::max)(b.cooldownTimer, 1.0f);
                    b.posY = b.garokJumpTargetY;
                    b.garokVelocityY = 0.0f;
                    b.garokGrounded = true;
                }
            }
            else if (!GAROK_TUNING.enableJump)
            {
                b.garokJumping = false;
            }
            else if (b.garokJumpCooldownTimer <= 0.0f && !b.isPreparing && !b.isAttacking)
            {
                b.garokJumping = true;
                b.garokJumpTimer = 0.0f;
                b.garokJumpDuration = 1.15f;
                b.garokJumpStartX = b.posX;
                b.garokJumpStartY = b.posY;
                b.garokJumpTargetX = RandomGarokLandingX();
                b.garokJumpTargetY = FindGroundYBelow(b.garokJumpTargetX, b.posY - b.height, b.posY);
                if (std::fabs(b.garokJumpTargetX - b.posX) > GAROK_TUNING.facingDeadzone)
                {
                    b.facingRight = (b.garokJumpTargetX >= b.posX);
                }
                ResetAnimation(b.garokJump);
            }

            if (!b.garokJumping)
            {
                const bool inAttackCheckRange = (std::fabs(dx) <= GAROK_TUNING.attackCheckRange) && (std::fabs(dy) <= b.height * 1.2f);

                if (!b.isPreparing && !b.isAttacking && b.isAggro && inAttackCheckRange && b.cooldownTimer <= 0.0f)
                {
                    b.targetLockedX = player.posX;
                    b.targetLockedY = player.posY;

                    if (b.phase2)
                    {
                        const int comboRoll = GetRand(99);
                        if (comboRoll < 34) b.garokComboMax = 1;
                        else if (comboRoll < 67) b.garokComboMax = 2;
                        else b.garokComboMax = 3;
                    }
                    else
                    {
                        b.garokComboMax = (GetRand(1) == 0) ? 1 : 2;
                    }

                    const int actionRoll = GetRand(99);
                    if (b.phase2)
                    {
                        if (actionRoll < 25) b.pendingAttack = 2;
                        else if (actionRoll < 50) b.pendingAttack = 3;
                        else b.pendingAttack = 1;
                    }
                    else
                    {
                        if (actionRoll < 30) b.pendingAttack = 2;
                        else b.pendingAttack = 1;
                    }

                    b.prepareTimer = GAROK_TUNING.prepareTime;
                    b.isPreparing = true;
                    b.garokAnimStage = 1;
                    b.garokBarrageStep = -1;
                    b.garokAttackElapsed = 0.0f;
                    b.garokHitDone1 = false;
                    b.garokHitDone2 = false;
                    b.garokHitDone3 = false;
                    b.garokDashTargetX = b.posX;
                    ResetAnimation(b.attack);
                    ResetAnimation(b.garokAttack2);
                    ResetAnimation(b.garokAttack3);
                    ResetAnimation(b.garokDashAttack);
                }

                if (b.isPreparing)
                {
                    b.prepareTimer -= dt;
                    if (std::fabs(b.targetLockedX - b.posX) > GAROK_TUNING.facingDeadzone)
                    {
                        b.facingRight = (b.targetLockedX >= b.posX);
                    }
                    if (b.prepareTimer <= 0.0f)
                    {
                        b.isPreparing = false;
                        b.isAttacking = true;
                        b.garokAttackElapsed = 0.0f;
                        b.garokAttackHitLife = 0.0f;
                        b.garokBarrageStep = -1;

                        if (b.pendingAttack == 1)
                        {
                            b.attackTimer = (b.garokComboMax <= 1) ? GAROK_TUNING.attackDuration1 : (b.garokComboMax == 2 ? GAROK_TUNING.attackDuration2 : GAROK_TUNING.attackDuration3);
                            ResetAnimation(b.attack);
                        }
                        else if (b.pendingAttack == 2)
                        {
                            const float dashDir = (b.targetLockedX >= b.posX) ? 1.0f : -1.0f;
                            const float throughDistance = b.width * 1.35f;
                            const float minDashDistance = b.width * 1.10f;
                            float dashTargetX = b.targetLockedX + dashDir * throughDistance;
                            if (std::fabs(dashTargetX - b.posX) < minDashDistance)
                            {
                                dashTargetX = b.posX + dashDir * minDashDistance;
                            }
                            b.garokDashTargetX = (std::max)(GAROK_TUNING.arenaLeft,
                                (std::min)(GAROK_TUNING.arenaRight, dashTargetX));
                            b.attackTimer = GAROK_TUNING.dashDuration;
                            ResetAnimation(b.garokDashAttack);
                        }
                        else
                        {
                            b.attackTimer = GAROK_TUNING.barrageDuration;
                            b.barrageShotTimer = 0.0f;
                            b.garokAnimStage = 1;
                            ResetAnimation(b.attack);
                            ResetAnimation(b.garokAttack2);
                            ResetAnimation(b.garokAttack3);
                        }
                    }
                }

                if (b.isAttacking)
                {
                    b.garokAttackElapsed += dt;

                    if (b.pendingAttack == 1)
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_TUNING.facingDeadzone)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }

                        float lockedAttackCenterX = 0.0f;
                        float lockedAttackCenterY = 0.0f;
                        GetGarokLockedTargetAttackCenter(b, lockedAttackCenterX, lockedAttackCenterY);

                        if (!b.garokHitDone1 && b.garokAttackElapsed >= 0.18f)
                        {
                            ActivateGarokAttackColliderAt(b, lockedAttackCenterX, lockedAttackCenterY);
                            b.garokAttackHitLife = 0.12f;
                            b.garokHitDone1 = true;
                        }
                        if (b.garokComboMax >= 2 && !b.garokHitDone2 && b.garokAttackElapsed >= 0.52f)
                        {
                            ActivateGarokAttackColliderAt(b, lockedAttackCenterX, lockedAttackCenterY);
                            b.garokAttackHitLife = 0.12f;
                            b.garokHitDone2 = true;
                        }
                        if (b.garokComboMax >= 3 && !b.garokHitDone3 && b.garokAttackElapsed >= 0.86f)
                        {
                            ActivateGarokAttackColliderAt(b, lockedAttackCenterX, lockedAttackCenterY);
                            b.garokAttackHitLife = 0.14f;
                            b.garokHitDone3 = true;
                        }

                        if (b.garokComboMax == 2 && b.garokAttackElapsed >= 0.34f) b.garokAnimStage = 2;
                        else if (b.garokComboMax >= 3)
                        {
                            if (b.garokAttackElapsed >= 0.68f) b.garokAnimStage = 3;
                            else if (b.garokAttackElapsed >= 0.34f) b.garokAnimStage = 2;
                            else b.garokAnimStage = 1;
                        }

                        b.attackTimer -= dt;
                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = (b.garokComboMax <= 1) ? GAROK_TUNING.cooldown1 : (b.garokComboMax == 2 ? GAROK_TUNING.cooldown2 : GAROK_TUNING.cooldown3);
                        }
                    }
                    else if (b.pendingAttack == 2)
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_TUNING.facingDeadzone)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }
                        const float dir = b.facingRight ? 1.0f : -1.0f;
                        const float prevX = b.posX;
                        MoveGarokXWithCollision(b, dir * GAROK_TUNING.dashSpeed * GetDeathSlowMotionScale());
                        b.garokMoving = true;
                        b.garokRunAnimHold = 0.12f;

                        if (b.garokAttackElapsed >= 0.18f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.10f;
                        }

                        const bool reachedDashTarget = b.facingRight ? (b.posX >= b.garokDashTargetX) : (b.posX <= b.garokDashTargetX);
                        const bool blocked = (std::fabs(b.posX - prevX) < 0.001f);

                        b.attackTimer -= dt;
                        if (reachedDashTarget || blocked)
                        {
                            b.attackTimer = 0.0f;
                        }

                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = GAROK_TUNING.cooldown2;
                        }
                    }
                    else
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_TUNING.facingDeadzone)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }
                        b.attackTimer -= dt;

                        const int totalBarrageSteps = 9;
                        const float stageStep = GAROK_TUNING.barrageDuration / static_cast<float>(totalBarrageSteps);
                        int barrageStep = static_cast<int>(b.garokAttackElapsed / stageStep);
                        if (barrageStep >= totalBarrageSteps) barrageStep = totalBarrageSteps - 1;

                        if (barrageStep != b.garokBarrageStep)
                        {
                            b.garokBarrageStep = barrageStep;
                            b.garokAnimStage = (barrageStep % 3) + 1;
                            ActivateGarokAttackColliderAt(b, b.targetLockedX, b.targetLockedY - PLAYER_HEIGHT * 0.5f);
                            b.garokAttackHitLife = 0.10f;
                        }

                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = GAROK_TUNING.barrageCooldown;
                        }
                    }
                }
            }

            if (!b.isPreparing && !b.isAttacking && !b.garokJumping && b.isAggro && std::fabs(dx) > GAROK_TUNING.attackRange)
            {
                const float moveSpeed = 2.6f;
                MoveGarokXWithCollision(b, (dx > 0.0f ? moveSpeed : -moveSpeed) * GetDeathSlowMotionScale());
                b.garokMoving = true;
                b.garokRunAnimHold = 0.12f;
                if (std::fabs(dx) > GAROK_TUNING.facingDeadzone)
                {
                    b.facingRight = (dx >= 0.0f);
                }
            }

            b.posX = (std::max)(GAROK_TUNING.arenaLeft + 32.0f, (std::min)(GAROK_TUNING.arenaRight - 32.0f, b.posX));
            ApplyGarokGravityAndMapCollision(b);

            if (b.colliderId != -1)
            {
                const float bodyW = b.width * 0.42f;
                const float bodyH = b.height * 0.82f;
                const float left = b.posX - bodyW * 0.5f;
                const float top = (b.posY - b.height * 0.5f) - bodyH * 0.5f;
                UpdateCollider(b.colliderId, left, top, bodyW, bodyH);
            }

            // Garokのアニメ選択（安定化版）
            const AnimationData* targetAnim = &b.idle;
            if (b.garokJumping) targetAnim = &b.garokJump;
            else if (b.isAttacking)
            {
                if (b.pendingAttack == 2) targetAnim = &b.garokDashAttack;
                else if (b.pendingAttack == 1 || b.pendingAttack == 3)
                {
                    if (b.garokAnimStage <= 1) targetAnim = &b.attack;
                    else if (b.garokAnimStage == 2) targetAnim = &b.garokAttack2;
                    else targetAnim = &b.garokAttack3;
                }
                else targetAnim = &b.attack;
            }
            else if (b.garokMoving || b.garokRunAnimHold > 0.0f) targetAnim = &b.garokRun;
            else targetAnim = &b.idle;

            // アニメが切り替わった時のみリセット
            if (b.garokCurrentAnim != targetAnim)
            {
                b.garokCurrentAnim = targetAnim;
                ResetAnimation(*const_cast<AnimationData*>(targetAnim));
            }
            UpdateAnimation(*const_cast<AnimationData*>(b.garokCurrentAnim));

            continue;
        }

        // StoneGolem
        if (b.cooldownTimer > 0.0f) b.cooldownTimer -= dt;

        if (b.barrageActive)
        {
            b.barrageTimer -= dt;
            b.barrageShotTimer -= dt;
            if (b.barrageShotTimer <= 0.0f)
            {
                const float tx = player.posX;
                const float ty = player.posY - PLAYER_HEIGHT * 0.5f;
                const float baseX = b.posX + STONE_PROJECTILE_OFFSET_X;
                const float baseY = b.posY + STONE_PROJECTILE_OFFSET_Y;
                SpawnStoneBullet(b, baseX - b.width * 0.35f, baseY - b.height * 0.62f, tx, ty, false);
                SpawnStoneBullet(b, baseX + b.width * 0.35f, baseY - b.height * 0.62f, tx, ty, false);
                SpawnStoneBullet(b, baseX, baseY - b.height * 0.95f, tx, ty, false);
                b.barrageShotTimer = STONE_TUNING.barrageInterval;
            }
            if (b.barrageTimer <= 0.0f)
            {
                b.barrageActive = false;
                b.cooldownTimer = STONE_TUNING.shotRecovery;
            }
        }

        if (!b.isPreparing && !b.isAttacking && !b.barrageActive && b.isAggro && b.cooldownTimer <= 0.0f)
        {
            const bool beam = ((b.attackCounter % 4) == 3);
            b.pendingAttack = beam ? 2 : 1;
            b.prepareTimer = beam ? STONE_TUNING.beamPrepare : STONE_TUNING.shotPrepare;
            b.isPreparing = true;
            if (beam) ResetAnimation(b.beamAttack);
            else ResetAnimation(b.attack);
        }

        if (b.isPreparing)
        {
            b.prepareTimer -= dt;
            if (b.prepareTimer <= 0.0f)
            {
                b.isPreparing = false;
                b.isAttacking = true;

                if (b.pendingAttack == 1)
                {
                    const float tx = player.posX;
                    const float ty = player.posY - PLAYER_HEIGHT * 0.5f;
                    const float baseX = b.posX + STONE_PROJECTILE_OFFSET_X;
                    const float baseY = b.posY + STONE_PROJECTILE_OFFSET_Y;
                    SpawnStoneBullet(b, baseX, baseY - b.height * 0.62f, tx, ty, b.phase2);
                    b.attackTimer = 0.35f;
                    b.cooldownTimer = STONE_TUNING.shotRecovery;
                }
                else
                {
                    SpawnStoneBeam(b, player.posX);
                    b.attackTimer = STONE_TUNING.beamActive;
                    b.cooldownTimer = STONE_TUNING.beamRecovery;
                }

                b.attackCounter++;
                if (b.pendingAttack == 2) ResetAnimation(b.beamAttack);
                else ResetAnimation(b.attack);
            }
        }

        if (b.isAttacking)
        {
            b.attackTimer -= dt;
            if (b.attackTimer <= 0.0f)
            {
                b.isAttacking = false;
                if (b.phase2 && !b.barrageActive && b.attackCounter >= 5)
                {
                    b.attackCounter = 0;
                    b.barrageActive = true;
                    ClearHomingStoneBullets();
                    b.barrageTimer = STONE_TUNING.barrageDuration;
                    b.barrageShotTimer = 0.0f;
                }
            }
        }

        if (b.colliderId != -1)
        {
            const float bodyW = b.width * 0.42f;
            const float bodyH = b.height * 0.82f;
            const float left = b.posX - bodyW * 0.5f;
            const float top = (b.posY - b.height * 0.5f) - bodyH * 0.5f;
            UpdateCollider(b.colliderId, left, top, bodyW, bodyH);
        }

        if (b.isAttacking || b.isPreparing)
        {
            if (b.pendingAttack == 2) UpdateAnimation(b.beamAttack);
            else UpdateAnimation(b.attack);
        }
        else
        {
            UpdateAnimation(b.idle);
        }
    }

    const PlayerData& p = GetPlayerData();
    const float block = 32.0f;
    for (auto& pr : g_midProjectiles)
    {
        if (!pr.active) continue;

        pr.lifeTimer -= dt;
        pr.animCounter++;

        if (pr.kind == StoneBullet && pr.homing)
        {
            float tx = p.posX - pr.posX;
            float ty = (p.posY - PLAYER_HEIGHT * 0.5f) - pr.posY;
            float tlen = std::sqrt(tx * tx + ty * ty);
            if (tlen > 0.001f)
            {
                tx /= tlen;
                ty /= tlen;

                float clen = std::sqrt(pr.velocityX * pr.velocityX + pr.velocityY * pr.velocityY);
                if (clen < 0.001f) clen = 1.0f;
                float cx = pr.velocityX / clen;
                float cy = pr.velocityY / clen;

                const float t = pr.homingTurnRate * GetDeathSlowMotionScale();
                float nx = cx * (1.0f - t) + tx * t;
                float ny = cy * (1.0f - t) + ty * t;
                float nlen = std::sqrt(nx * nx + ny * ny);
                if (nlen > 0.001f)
                {
                    nx /= nlen;
                    ny /= nlen;
                    pr.velocityX = nx * pr.speed;
                    pr.velocityY = ny * pr.speed;
                }
            }
        }

        pr.posX += pr.velocityX * GetDeathSlowMotionScale();
        pr.posY += pr.velocityY * GetDeathSlowMotionScale();
        pr.facingRight = (pr.velocityX >= 0.0f);

        if (pr.kind == StoneBullet)
        {
            const int gx = static_cast<int>(pr.posX / block);
            const int gy = static_cast<int>(pr.posY / block);
            MapChipData* mc = GetMapChipData(gx, gy);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                ClearProjectile(pr);
                continue;
            }
        }

        if (pr.lifeTimer <= 0.0f)
        {
            ClearProjectile(pr);
            continue;
        }

        if (pr.colliderId != -1)
        {
            UpdateCollider(pr.colliderId,
                pr.posX - pr.colliderWidth * 0.5f,
                pr.posY - pr.colliderHeight * 0.5f,
                pr.colliderWidth,
                pr.colliderHeight);
        }
    }
}

void DrawMidBosses()
{
    LoadStoneAssets();
    CameraData camera = GetCamera();

    // --------- ミッドボス本体の描画 ---------
    for (const auto& b : g_midBosses)
    {
        if (!b.active) continue;
        if (b.isDead && IsAnimationFinished(b.die) && ((b.deathBlinkTimer / 4) % 2 == 1)) continue;

        int drawX = static_cast<int>((b.posX - camera.posX) * camera.scale) + static_cast<int>(STONE_TUNING.drawOffsetX);
        int drawY = static_cast<int>((b.posY - camera.posY) * camera.scale) + static_cast<int>(STONE_TUNING.drawOffsetY);
        int drawW = static_cast<int>(b.width * camera.scale);
        int drawH = static_cast<int>(b.height * camera.scale);

        if (b.type == MidBossType::Garok)
        {
            drawX = static_cast<int>((b.posX - camera.posX) * camera.scale) + static_cast<int>(GAROK_TUNING.drawOffsetX);
            drawY = static_cast<int>((b.posY - camera.posY) * camera.scale) + static_cast<int>(GAROK_TUNING.drawOffsetY);
            drawW = static_cast<int>(drawW * GAROK_TUNING.drawWidthScale);
        }

        // アニメーション選択
        const AnimationData* anim = &b.idle;
        if (b.isDead) anim = &b.die;
        else if (b.type == MidBossType::Garok)
        {
            // Garokは更新ループで確定したものを使う
            anim = (b.garokCurrentAnim != nullptr) ? b.garokCurrentAnim : &b.idle;
        }
        else if (b.isPreparing || b.isAttacking)
        {
            anim = (b.pendingAttack == 2) ? &b.beamAttack : &b.attack;
        }

        const int left = drawX - drawW / 2;
        const int top = drawY - drawH;
        const int right = drawX + drawW / 2;
        const int bottom = drawY;

        const int handle = (anim->frames != nullptr) ? GetCurrentAnimationFrame(*anim) : -1;

        // 予備動作・攻撃中は赤く発光
        const bool shouldGlowRed = (b.type == MidBossType::Garok) ? b.isPreparing : (b.isPreparing || b.isAttacking);
        if (shouldGlowRed)
        {
            SetDrawBright(255, 96, 96);
        }
        else
        {
            SetDrawBright(255, 255, 255);
        }

        if (handle != -1)
        {
            if (b.type == MidBossType::Garok)
            {
                if (b.facingRight) DrawExtendGraph(right, top, left, bottom, handle, TRUE);
                else DrawExtendGraph(left, top, right, bottom, handle, TRUE);
            }
            else
            {
                if (b.facingRight) DrawExtendGraph(left, top, right, bottom, handle, TRUE);
                else DrawExtendGraph(right, top, left, bottom, handle, TRUE);
            }
        }
        else
        {
            DrawBox(left, top, right, bottom, GetColor(160, 160, 160), TRUE);
        }

        if (b.type == MidBossType::Garok && GAROK_TUNING.debugDraw)
        {
            int frameW = 0;
            int frameH = 0;
            if (handle != -1)
            {
                GetGraphSize(handle, &frameW, &frameH);
            }

            DrawBox(left, top, right, bottom, GetColor(0, 255, 0), FALSE);
            DrawLine(drawX - 8, drawY, drawX + 8, drawY, GetColor(255, 255, 0));
            DrawLine(drawX, drawY - 8, drawX, drawY + 8, GetColor(255, 255, 0));

            if (b.garokAttackColliderId != -1)
            {
                float colliderLeft = 0.0f;
                float colliderTop = 0.0f;
                float colliderWidth = 0.0f;
                float colliderHeight = 0.0f;
                if (GetColliderRect(b.garokAttackColliderId, colliderLeft, colliderTop, colliderWidth, colliderHeight))
                {
                    const int hitLeft = static_cast<int>((colliderLeft - camera.posX) * camera.scale);
                    const int hitTop = static_cast<int>((colliderTop - camera.posY) * camera.scale);
                    const int hitRight = static_cast<int>((colliderLeft + colliderWidth - camera.posX) * camera.scale);
                    const int hitBottom = static_cast<int>((colliderTop + colliderHeight - camera.posY) * camera.scale);
                    DrawBox(hitLeft, hitTop, hitRight, hitBottom, GetColor(255, 64, 64), FALSE);
                    DrawBox(hitLeft + 1, hitTop + 1, hitRight - 1, hitBottom - 1, GetColor(255, 160, 160), FALSE);
                }
            }

            DrawFormatString(
                left,
                top - 64,
                GetColor(255, 255, 0),
                "Garok pos(%.2f,%.2f) draw(%d,%d) vY=%.3f grd=%d",
                b.posX,
                b.posY,
                drawX,
                drawY,
                b.garokVelocityY,
                b.garokGrounded ? 1 : 0);

            DrawFormatString(
                left,
                top - 48,
                GetColor(255, 255, 0),
                "state agg=%d prep=%d atk=%d move=%d jump=%d face=%d pAtk=%d stage=%d",
                b.isAggro ? 1 : 0,
                b.isPreparing ? 1 : 0,
                b.isAttacking ? 1 : 0,
                b.garokMoving ? 1 : 0,
                b.garokJumping ? 1 : 0,
                b.facingRight ? 1 : 0,
                b.pendingAttack,
                b.garokAnimStage);

            DrawFormatString(
                left,
                top - 32,
                GetColor(255, 255, 0),
                "anim h=%d frame=%d size=%dx%d combo=%d atkT=%.2f prepT=%.2f",
                handle,
                (anim != nullptr) ? anim->currentFrame : -1,
                frameW,
                frameH,
                b.garokComboMax,
                b.attackTimer,
                b.prepareTimer);
        }

        SetDrawBright(255, 255, 255);
    }

    SetDrawBright(255, 255, 255);

    // --------- プロジェクタイル（弾・ビーム）の描画 ---------
    for (const auto& p : g_midProjectiles)
    {
        if (!p.active) continue;

        int drawX = static_cast<int>((p.posX - camera.posX) * camera.scale);
        int drawY = static_cast<int>((p.posY - camera.posY) * camera.scale);
        int halfW = static_cast<int>(p.width * 0.5f * camera.scale);
        int halfH = static_cast<int>(p.height * 0.5f * camera.scale);
        int left = drawX - halfW;
        int top = drawY - halfH;
        int right = drawX + halfW;
        int bottom = drawY + halfH;

        if (p.kind == StoneBullet)
        {
            const int bulletHandle = (g_stoneBulletHandle != -1) ? g_stoneBulletHandle : g_stoneBulletGlowHandle;
            if (bulletHandle != -1)
            {
                if (p.velocityX >= 0.0f)
                {
                    DrawExtendGraph(left, top, right, bottom, bulletHandle, TRUE);
                }
                else
                {
                    DrawExtendGraph(right, top, left, bottom, bulletHandle, TRUE);
                }
            }
            else
            {
                DrawBox(left, top, right, bottom, GetColor(180, 220, 255), TRUE);
            }
        }
        else
        {
            int beamFrame = -1;
            const int beamIndex = (p.animCounter / 3) % 5;
            if (beamIndex >= 0 && beamIndex < 5)
            {
                beamFrame = g_stoneBeamFrames[beamIndex];
            }

            if (beamFrame != -1)
            {
                int beamDrawX = drawX + static_cast<int>(STONE_TUNING.beamDrawOffsetX);
                int beamDrawY = drawY + static_cast<int>(STONE_TUNING.beamDrawOffsetY);
                int beamLeft = beamDrawX - halfW;
                int beamTop = beamDrawY - halfH;
                int beamRight = beamDrawX + halfW;
                int beamBottom = beamDrawY + halfH;

                const int srcX = 0;
                const int srcY = 100;
                const int srcW = 300;
                const int srcH = 100;
                if (p.facingRight)
                {
                    DrawRectExtendGraph(beamRight, beamTop, beamLeft, beamBottom, srcX, srcY, srcW, srcH, beamFrame, TRUE);
                }
                else
                {
                    DrawRectExtendGraph(beamLeft, beamTop, beamRight, beamBottom, srcX, srcY, srcW, srcH, beamFrame, TRUE);
                }
            }
            else
            {
                DrawBox(left, top, right, bottom, GetColor(80, 220, 255), TRUE);
            }
        }
    }
}

bool DamageMidBossByColliderId(int colliderId, int damage)
{
    for (auto& b : g_midBosses)
    {
        if (!b.active || b.isDead) continue;
        if (b.colliderId == colliderId)
        {
            b.hp -= damage;
            if (b.hp < 0) b.hp = 0;
            return true;
        }
    }
    return false;
}

bool IsMidBossAlive()
{
    for (const auto& b : g_midBosses)
    {
        if (b.active && !b.isDead)
        {
            return true;
        }
    }
    return false;
}

int GetMidBossHP()
{
    for (const auto& b : g_midBosses)
    {
        if (b.active && !b.isDead) return b.hp;
    }
    return 0;
}

int GetMidBossMaxHP()
{
    for (const auto& b : g_midBosses)
    {
        if (b.active) return b.maxHP;
    }
    return 0;
}
