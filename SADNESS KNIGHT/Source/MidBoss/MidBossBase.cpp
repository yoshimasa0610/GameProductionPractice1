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
        float garokAttackElapsed = 0.0f;
        float garokAttackHitLife = 0.0f;
        bool garokHitDone1 = false;
        bool garokHitDone2 = false;
        bool garokHitDone3 = false;
        int garokAttackColliderId = -1;
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
    // 攻撃パラメータ（タイミング・スピード）
    //============================================================
    const float SHOT_PREPARE = 1.5f;      // 弾発射までの予備動作時間
    const float BEAM_PREPARE = 2.0f;      // ビーム発射までの予備動作時間
    const float SHOT_RECOVERY = 2.0f;     // 弾発射後のクールダウン
    const float BEAM_RECOVERY = 4.0f;     // ビーム発射後のクールダウン
    const float BEAM_ACTIVE = 1.0f;       // ビーム持続時間
    const float BARRAGE_DURATION = 3.0f;  // 連射モードの継続時間
    const float BARRAGE_INTERVAL = 0.22f; // 連射時の1発目との間隔
    const float BULLET_SPEED = 6.0f;      // 弾の移動速度

    // Garok用パラメータ
    const float GAROK_PREPARE_TIME = 1.0f;
    const float GAROK_ATTACK_DURATION_1 = 2.0f;
    const float GAROK_ATTACK_DURATION_2 = 2.7f;
    const float GAROK_ATTACK_DURATION_3 = 3.7f;
    const float GAROK_COOLDOWN_1 = 2.0f;
    const float GAROK_COOLDOWN_2 = 3.0f;
    const float GAROK_COOLDOWN_3 = 4.0f;
    const float GAROK_BARRAGE_DURATION = 3.0f;
    const float GAROK_BARRAGE_COOLDOWN = 5.0f;
    const float GAROK_DASH_DURATION = 2.0f;
    const float GAROK_DASH_SPEED = 11.0f;
    const float GAROK_JUMP_INTERVAL = 8.0f;
    const float GAROK_JUMP_HEIGHT = 210.0f;
    const float GAROK_HITBOX_WIDTH_RATIO = 0.95f;
    const float GAROK_HITBOX_HEIGHT_RATIO = 0.62f;
    const float GAROK_HITBOX_FRONT_RATIO = 0.56f;
    const float GAROK_CONTACT_Y_RATIO = 0.45f;
    const float GAROK_FACING_DEADZONE = 24.0f;
    const float GAROK_ARENA_LEFT = 768.0f;
    const float GAROK_ARENA_RIGHT = 3072.0f;
    const float GAROK_ATTACK_TRIGGER_RANGE = 32.0f * 5.0f;
    const float GAROK_GRAVITY = 0.35f;
    const float GAROK_MAX_FALL_SPEED = 9.0f;

    //============================================================
    // 弾・ビームのサイズ
    //============================================================
    const float STONE_BULLET_DRAW_SIZE = 96.0f;    // 弾の描画サイズ
    const float STONE_BULLET_HITBOX_SIZE = 60.0f;  // 弾の当たり判定サイズ

    //============================================================
    // 描画オフセット（ボスとビーム）
    //============================================================
    const float STONE_GOLEM_DRAW_OFFSET_X = 0.0f;  // ボス描画のX偏移（ピクセル）
    const float STONE_GOLEM_DRAW_OFFSET_Y = 80.0f;  // ボス描画のY偏移（ピクセル）
    const float STONE_BEAM_DRAW_OFFSET_X = 100.0f;   // ビーム描画のX偏移（ピクセル）
    const float STONE_BEAM_DRAW_OFFSET_Y = 28.0f;   // ビーム描画のY偏移（ピクセル）
    const float STONE_BEAM_MUZZLE_FINE_TUNE_X = -70.0f; // 発射起点の微調整（+で外側、-で内側）

    // Garok描画ズレ補正
    const float GAROK_DRAW_OFFSET_X = -12.0f;
    const float GAROK_DRAW_OFFSET_Y = 64.0f;
    const float GAROK_DRAW_ATTACK2_OFFSET_X = -8.0f;
    const float GAROK_DRAW_ATTACK3_OFFSET_X = -6.0f;
    const float GAROK_DRAW_DASH_OFFSET_X = -18.0f;

    // 弾・ビーム発射位置をボス描画オフセットに合わせる補正
    const float STONE_PROJECTILE_OFFSET_X = STONE_GOLEM_DRAW_OFFSET_X;
    const float STONE_PROJECTILE_OFFSET_Y = STONE_GOLEM_DRAW_OFFSET_Y;

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
        LoadAnimationFromSheet(b.idle, "Data/MidBoss/Garok/IDLE.png", 18, 64, 64, 5, AnimationMode::Loop);
        LoadAnimationFromSheet(b.garokRun, "Data/MidBoss/Garok/RUN.png", 18, 64, 64, 4, AnimationMode::Loop);
        LoadAnimationFromSheet(b.attack, "Data/MidBoss/Garok/ATTACK 1.png", 18, 64, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokAttack2, "Data/MidBoss/Garok/ATTACK 2.png", 12, 64, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokAttack3, "Data/MidBoss/Garok/ATTACK 3.png", 15, 64, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokDashAttack, "Data/MidBoss/Garok/DASH ATTACK.png", 36, 64, 64, 3, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokJump, "Data/MidBoss/Garok/JUMP.png", 9, 64, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.garokHurt, "Data/MidBoss/Garok/HURT.png", 18, 64, 64, 4, AnimationMode::Once);
        LoadAnimationFromSheet(b.die, "Data/MidBoss/Garok/DEATH.png", 24, 64, 64, 4, AnimationMode::Once);
    }

    void ClearGarokAttackCollider(MidBossData& b)
    {
        if (b.garokAttackColliderId != -1)
        {
            DestroyCollider(b.garokAttackColliderId);
            b.garokAttackColliderId = -1;
        }
    }

    void UpdateGarokAttackCollider(MidBossData& b)
    {
        if (b.garokAttackColliderId == -1) return;

        const float attackW = b.width * GAROK_HITBOX_WIDTH_RATIO;
        const float attackH = b.height * GAROK_HITBOX_HEIGHT_RATIO;
        const float front = b.width * GAROK_HITBOX_FRONT_RATIO;
        const float centerX = b.posX + (b.facingRight ? front : -front);
        const float centerY = b.posY - b.height * GAROK_CONTACT_Y_RATIO;
        UpdateCollider(
            b.garokAttackColliderId,
            centerX - attackW * 0.5f,
            centerY - attackH * 0.5f,
            attackW,
            attackH);
    }

    void ActivateGarokAttackCollider(MidBossData& b)
    {
        const float attackW = b.width * GAROK_HITBOX_WIDTH_RATIO;
        const float attackH = b.height * GAROK_HITBOX_HEIGHT_RATIO;
        const float front = b.width * GAROK_HITBOX_FRONT_RATIO;
        const float centerX = b.posX + (b.facingRight ? front : -front);
        const float centerY = b.posY - b.height * GAROK_CONTACT_Y_RATIO;

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

    float RandomGarokLandingX()
    {
        const int minX = static_cast<int>(GAROK_ARENA_LEFT + 128.0f);
        const int maxX = static_cast<int>(GAROK_ARENA_RIGHT - 128.0f);
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

        const float halfW = b.width * 0.20f;
        const float topY = b.posY - b.height * 0.92f;
        const float midY = b.posY - b.height * 0.55f;
        const float lowY = b.posY - b.height * 0.20f;

        const float sideX = b.posX + moveX + (moveX > 0.0f ? halfW : -halfW);
        if (IsGarokSolidAt(sideX, topY) || IsGarokSolidAt(sideX, midY) || IsGarokSolidAt(sideX, lowY))
        {
            return;
        }

        b.posX += moveX;
    }

    void ApplyGarokGravityAndMapCollision(MidBossData& b)
    {
        if (b.garokJumping) return;

        b.garokGrounded = false;
        b.garokVelocityY += GAROK_GRAVITY * GetDeathSlowMotionScale();
        if (b.garokVelocityY > GAROK_MAX_FALL_SPEED) b.garokVelocityY = GAROK_MAX_FALL_SPEED;

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
        p.width = STONE_BULLET_DRAW_SIZE;
        p.height = STONE_BULLET_DRAW_SIZE;
        p.colliderWidth = STONE_BULLET_HITBOX_SIZE;
        p.colliderHeight = STONE_BULLET_HITBOX_SIZE;
        p.speed = BULLET_SPEED;
        p.velocityX = (dx / len) * p.speed;
        p.velocityY = (dy / len) * p.speed;
        p.lifeTimer = 4.0f;
        p.homing = homing;                          // ホーミング有無は引数で指定
        p.homingTurnRate = homing ? 0.08f : 0.0f;   // ホーミング時のみ回転速度を設定
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
        const float eyeX = baseX + dir * (b.width * 0.12f + STONE_BEAM_MUZZLE_FINE_TUNE_X);
        const float eyeY = baseY - b.height * 0.78f;

        const float beamLength = ComputeStoneBeamLength(eyeX, eyeY, p.facingRight, p.height);
        p.width = beamLength;
        p.colliderWidth = p.width;
        p.colliderHeight = p.height;
        p.posX = eyeX + (p.facingRight ? (beamLength * 0.5f) : -(beamLength * 0.5f));
        p.posY = eyeY;

        p.lifeTimer = BEAM_ACTIVE;
        p.colliderId = CreateCollider(
            ColliderTag::Attack,
            p.posX - p.colliderWidth * 0.5f,
            p.posY - p.colliderHeight * 0.5f,
            p.colliderWidth,
            p.colliderHeight,
            &b.attackOwner);
        g_midProjectiles.push_back(p);
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
        b.detectRange = 1200.0f;
        b.garokJumpCooldownTimer = GAROK_JUMP_INTERVAL;
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
        if (b.type != MidBossType::Garok || std::fabs(dx) > GAROK_FACING_DEADZONE)
        {
            b.facingRight = (dx >= 0.0f);
        }
        b.phase2 = (b.hp <= b.maxHP / 2);
        b.isAggro = (dist <= b.detectRange);

        if (b.type == MidBossType::Garok)
        {
            b.posX = (std::max)(GAROK_ARENA_LEFT + 32.0f, (std::min)(GAROK_ARENA_RIGHT - 32.0f, b.posX));

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
                const float arc = std::sin(t * DX_PI_F) * GAROK_JUMP_HEIGHT;
                b.posX = b.garokJumpStartX + (b.garokJumpTargetX - b.garokJumpStartX) * t;
                b.posY = b.garokJumpStartY + (b.garokJumpTargetY - b.garokJumpStartY) * t - arc;

                if (t >= 1.0f)
                {
                    b.garokJumping = false;
                    b.garokJumpCooldownTimer = GAROK_JUMP_INTERVAL;
                    b.cooldownTimer = (std::max)(b.cooldownTimer, 1.0f);
                    b.posY = b.garokJumpTargetY;
                    b.garokVelocityY = 0.0f;
                    b.garokGrounded = true;
                }
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
                if (std::fabs(b.garokJumpTargetX - b.posX) > GAROK_FACING_DEADZONE)
                {
                    b.facingRight = (b.garokJumpTargetX >= b.posX);
                }
                ResetAnimation(b.garokJump);
            }

            if (!b.garokJumping)
            {
                const bool inAttackRange = (std::fabs(dx) <= GAROK_ATTACK_TRIGGER_RANGE) && (std::fabs(dy) <= b.height * 1.2f);

                if (!b.isPreparing && !b.isAttacking && b.isAggro && inAttackRange && b.cooldownTimer <= 0.0f)
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
                    if (b.phase2 && actionRoll < 20) b.pendingAttack = 3;
                    else if (b.phase2 && actionRoll < 45) b.pendingAttack = 2;
                    else b.pendingAttack = 1;

                    b.prepareTimer = GAROK_PREPARE_TIME;
                    b.isPreparing = true;
                    b.garokAnimStage = 1;
                    b.garokAttackElapsed = 0.0f;
                    b.garokHitDone1 = false;
                    b.garokHitDone2 = false;
                    b.garokHitDone3 = false;
                    ResetAnimation(b.attack);
                    ResetAnimation(b.garokAttack2);
                    ResetAnimation(b.garokAttack3);
                    ResetAnimation(b.garokDashAttack);
                }

                if (b.isPreparing)
                {
                    b.prepareTimer -= dt;
                    if (std::fabs(b.targetLockedX - b.posX) > GAROK_FACING_DEADZONE)
                    {
                        b.facingRight = (b.targetLockedX >= b.posX);
                    }
                    if (b.prepareTimer <= 0.0f)
                    {
                        b.isPreparing = false;
                        b.isAttacking = true;
                        b.garokAttackElapsed = 0.0f;
                        b.garokAttackHitLife = 0.0f;

                        if (b.pendingAttack == 1)
                        {
                            b.attackTimer = (b.garokComboMax <= 1) ? GAROK_ATTACK_DURATION_1 : (b.garokComboMax == 2 ? GAROK_ATTACK_DURATION_2 : GAROK_ATTACK_DURATION_3);
                            ResetAnimation(b.attack);
                        }
                        else if (b.pendingAttack == 2)
                        {
                            b.attackTimer = GAROK_DASH_DURATION;
                            ResetAnimation(b.garokDashAttack);
                        }
                        else
                        {
                            b.attackTimer = GAROK_BARRAGE_DURATION;
                            b.barrageShotTimer = 0.0f;
                            ResetAnimation(b.attack);
                        }
                    }
                }

                if (b.isAttacking)
                {
                    b.garokAttackElapsed += dt;

                    if (b.pendingAttack == 1)
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_FACING_DEADZONE)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }

                        if (!b.garokHitDone1 && b.garokAttackElapsed >= 1.0f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.20f;
                            b.garokHitDone1 = true;
                        }
                        if (b.garokComboMax >= 2 && !b.garokHitDone2 && b.garokAttackElapsed >= 1.75f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.18f;
                            b.garokHitDone2 = true;
                        }
                        if (b.garokComboMax >= 3 && !b.garokHitDone3 && b.garokAttackElapsed >= 2.75f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.18f;
                            b.garokHitDone3 = true;
                        }

                        if (b.garokComboMax == 2 && b.garokAttackElapsed >= 1.35f) b.garokAnimStage = 2;
                        else if (b.garokComboMax >= 3)
                        {
                            if (b.garokAttackElapsed >= 2.40f) b.garokAnimStage = 3;
                            else if (b.garokAttackElapsed >= 1.20f) b.garokAnimStage = 2;
                            else b.garokAnimStage = 1;
                        }

                        b.attackTimer -= dt;
                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = (b.garokComboMax <= 1) ? GAROK_COOLDOWN_1 : (b.garokComboMax == 2 ? GAROK_COOLDOWN_2 : GAROK_COOLDOWN_3);
                        }
                    }
                    else if (b.pendingAttack == 2)
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_FACING_DEADZONE)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }
                        const float dir = b.facingRight ? 1.0f : -1.0f;
                        MoveGarokXWithCollision(b, dir * GAROK_DASH_SPEED * GetDeathSlowMotionScale());

                        if (b.garokAttackElapsed >= 0.8f && b.garokAttackElapsed <= 1.6f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.08f;
                        }

                        b.attackTimer -= dt;
                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = GAROK_COOLDOWN_2;
                        }
                    }
                    else
                    {
                        if (std::fabs(b.targetLockedX - b.posX) > GAROK_FACING_DEADZONE)
                        {
                            b.facingRight = (b.targetLockedX >= b.posX);
                        }
                        b.attackTimer -= dt;
                        b.barrageShotTimer -= dt;
                        if (b.barrageShotTimer <= 0.0f)
                        {
                            ActivateGarokAttackCollider(b);
                            b.garokAttackHitLife = 0.10f;
                            b.barrageShotTimer = 0.22f;
                        }
                        if (b.attackTimer <= 0.0f)
                        {
                            b.isAttacking = false;
                            ClearGarokAttackCollider(b);
                            b.cooldownTimer = GAROK_BARRAGE_COOLDOWN;
                        }
                    }
                }
            }

            if (!b.isPreparing && !b.isAttacking && !b.garokJumping && b.isAggro && std::fabs(dx) > GAROK_ATTACK_TRIGGER_RANGE)
            {
                const float moveSpeed = 2.6f;
                MoveGarokXWithCollision(b, (dx > 0.0f ? moveSpeed : -moveSpeed) * GetDeathSlowMotionScale());
                if (std::fabs(dx) > GAROK_FACING_DEADZONE)
                {
                    b.facingRight = (dx >= 0.0f);
                }
            }

            b.posX = (std::max)(GAROK_ARENA_LEFT + 32.0f, (std::min)(GAROK_ARENA_RIGHT - 32.0f, b.posX));
            ApplyGarokGravityAndMapCollision(b);
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
                b.barrageShotTimer = BARRAGE_INTERVAL;
            }
            if (b.barrageTimer <= 0.0f)
            {
                b.barrageActive = false;
                b.cooldownTimer = SHOT_RECOVERY;
            }
        }

        if (!b.isPreparing && !b.isAttacking && !b.barrageActive && b.isAggro && b.cooldownTimer <= 0.0f)
        {
            const bool beam = ((b.attackCounter % 4) == 3);
            b.pendingAttack = beam ? 2 : 1;
            b.prepareTimer = beam ? BEAM_PREPARE : SHOT_PREPARE;
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
                    b.cooldownTimer = SHOT_RECOVERY;
                }
                else
                {
                    SpawnStoneBeam(b, player.posX);
                    b.attackTimer = BEAM_ACTIVE;
                    b.cooldownTimer = BEAM_RECOVERY;
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
                    b.barrageTimer = BARRAGE_DURATION;
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

        int drawX = static_cast<int>((b.posX - camera.posX) * camera.scale) + static_cast<int>(STONE_GOLEM_DRAW_OFFSET_X);
        int drawY = static_cast<int>((b.posY - camera.posY) * camera.scale) + static_cast<int>(STONE_GOLEM_DRAW_OFFSET_Y);
        int drawW = static_cast<int>(b.width * camera.scale);
        int drawH = static_cast<int>(b.height * camera.scale);

        // アニメーション選択
        const AnimationData* anim = &b.idle;
        if (b.isDead) anim = &b.die;
        else if (b.type == MidBossType::Garok)
        {
            if (b.garokJumping) anim = &b.garokJump;
            else if (b.isAttacking)
            {
                if (b.pendingAttack == 2) anim = &b.garokDashAttack;
                else if (b.pendingAttack == 1)
                {
                    if (b.garokAnimStage <= 1) anim = &b.attack;
                    else if (b.garokAnimStage == 2) anim = &b.garokAttack2;
                    else anim = &b.garokAttack3;
                }
                else anim = &b.attack;
            }
            else if (b.isAggro && std::fabs(GetPlayerData().posX - b.posX) > 120.0f) anim = &b.garokRun;
            else anim = &b.idle;
        }
        else if (b.isPreparing || b.isAttacking)
        {
            anim = (b.pendingAttack == 2) ? &b.beamAttack : &b.attack;
        }

        if (b.type == MidBossType::Garok)
        {
            float garokOffsetX = GAROK_DRAW_OFFSET_X;
            if (anim == &b.garokAttack2) garokOffsetX += GAROK_DRAW_ATTACK2_OFFSET_X;
            else if (anim == &b.garokAttack3) garokOffsetX += GAROK_DRAW_ATTACK3_OFFSET_X;
            else if (anim == &b.garokDashAttack) garokOffsetX += GAROK_DRAW_DASH_OFFSET_X;

            drawX += static_cast<int>(garokOffsetX);
            drawY += static_cast<int>(GAROK_DRAW_OFFSET_Y);
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
                // Garok素材は初期向きが逆なので反転ロジックを反対にする
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
                int beamDrawX = drawX + static_cast<int>(STONE_BEAM_DRAW_OFFSET_X);
                int beamDrawY = drawY + static_cast<int>(STONE_BEAM_DRAW_OFFSET_Y);
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
