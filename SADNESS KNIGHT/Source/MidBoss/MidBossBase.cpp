#include "MidBossBase.h"
#include "../Animation/Animation.h"
#include "../Camera/Camera.h"
#include "../Collision/Collision.h"
#include "../Map/MapManager.h"
#include "../Player/Player.h"
#include "../Enemy/EnemyBase.h"
#include "DxLib.h"
#include <vector>
#include <cmath>

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
        AnimationData idle, attack, die;

        // 死亡状態
        bool isDead = false;              // 死亡中フラグ
        int deathBlinkTimer = 0;          // 死亡後の点滅タイマー

        // AI状態
        bool isAggro = false;             // 戦闘状態か
        float detectRange = 520.0f;       // プレイヤー検知範囲

        // 攻撃準備状態
        bool isPreparing = false;         // 攻撃予備動作中か
        float prepareTimer = 0.0f;        // 予備動作の時間
        int pendingAttack = 0;            // 1:shot 2:beam

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
    const float BARRAGE_DURATION = 4.0f;  // 連射モードの継続時間
    const float BARRAGE_INTERVAL = 0.22f; // 連射時の1発目との間隔
    const float BULLET_SPEED = 6.0f;      // 弾の移動速度

    //============================================================
    // 弾・ビームのサイズ
    //============================================================
    const float STONE_BULLET_DRAW_SIZE = 96.0f;    // 弾の描画サイズ
    const float STONE_BULLET_HITBOX_SIZE = 60.0f;  // 弾の当たり判定サイズ

    //============================================================
    // 描画オフセット（ボスとビーム）
    //============================================================
    const float STONE_GOLEM_DRAW_OFFSET_X = 0.0f;  // ボス描画のX偏移（ピクセル）
    const float STONE_GOLEM_DRAW_OFFSET_Y = 0.0f;  // ボス描画のY偏移（ピクセル）
    const float STONE_BEAM_DRAW_OFFSET_X = 57.0f;   // ビーム描画のX偏移（ピクセル）
    const float STONE_BEAM_DRAW_OFFSET_Y = 28.0f;   // ビーム描画のY偏移（ピクセル）
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
        // attack: 行20、8フレーム
        LoadAnimationFromSheetRange(b.attack, sheetPath, 0, 20, 8, 100, 100, 4, AnimationMode::Once);
        // die: 行70、12フレーム
        LoadAnimationFromSheetRange(b.die, sheetPath, 0, 70, 12, 100, 100, 5, AnimationMode::Once);
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

    // ビームを発射
    void SpawnStoneBeam(MidBossData& b, float targetX)
    {
        MidBossProjectile p{};
        p.active = true;
        p.kind = StoneBeam;
        p.width = 340.0f;
        p.height = 96.0f;
        p.colliderWidth = p.width;
        p.colliderHeight = p.height;
        p.facingRight = (targetX >= b.posX);

        // ボスの目の位置からビームを発射
        const float eyeX = b.posX + (p.facingRight ? (b.width * 0.12f) : -(b.width * 0.12f));
        const float eyeY = b.posY - b.height * 0.78f;
        p.posX = eyeX + (p.facingRight ? (p.width * 0.5f) : -(p.width * 0.5f));
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
        UnloadAnimation(b.idle);
        UnloadAnimation(b.attack);
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
        b.maxHP = 420;
        b.hp = b.maxHP;
        b.attackPower = 24;
        b.detectRange = 1400.0f;
        InitStoneAnimations(b);
    }

    b.attackOwner = {};
    b.attackOwner.attackPower = b.attackPower;

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

    // --------- ミッドボス本体の更新 ---------
    for (auto& b : g_midBosses)
    {
        if (!b.active) continue;

        // --------- 死亡判定 ---------
        if (b.hp <= 0 && !b.isDead)
        {
            b.isDead = true;
            b.isPreparing = false;
            b.isAttacking = false;
            b.barrageActive = false;
            b.deathBlinkTimer = 0;
            ResetAnimation(b.die);
            if (b.colliderId != -1)
            {
                DestroyCollider(b.colliderId);
                b.colliderId = -1;
            }
        }

        // --------- 死亡演出中 ---------
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

        // --------- AI計算：プレイヤーとの距離・方向 ---------
        const float dx = player.posX - b.posX;
        const float dy = player.posY - b.posY;
        const float dist = std::sqrt(dx * dx + dy * dy);

        b.facingRight = (dx >= 0.0f);
        b.phase2 = (b.hp <= b.maxHP / 2);      // HP半分でフェーズ2に突入
        b.isAggro = (dist <= b.detectRange);    // プレイヤーが近くにいるか

        // --------- クールダウンの更新 ---------
        if (b.cooldownTimer > 0.0f) b.cooldownTimer -= dt;

        // --------- 連射モード（フェーズ2の4秒連射） ---------
        if (b.barrageActive)
        {
            b.barrageTimer -= dt;
            b.barrageShotTimer -= dt;
            if (b.barrageShotTimer <= 0.0f)
            {
                // 左右＋上の3方向に非追従の弾を発射
                const float tx = player.posX;
                const float ty = player.posY - PLAYER_HEIGHT * 0.5f;
                SpawnStoneBullet(b, b.posX - b.width * 0.35f, b.posY - b.height * 0.62f, tx, ty, false);
                SpawnStoneBullet(b, b.posX + b.width * 0.35f, b.posY - b.height * 0.62f, tx, ty, false);
                SpawnStoneBullet(b, b.posX, b.posY - b.height * 0.95f, tx, ty, false);
                b.barrageShotTimer = BARRAGE_INTERVAL;
            }
            if (b.barrageTimer <= 0.0f)
            {
                b.barrageActive = false;
                b.cooldownTimer = SHOT_RECOVERY;
            }
        }

        // --------- 攻撃パターン選択 ---------
        if (!b.isPreparing && !b.isAttacking && !b.barrageActive && b.isAggro && b.cooldownTimer <= 0.0f)
        {
            const bool beam = ((b.attackCounter % 4) == 3);  // 4回目の攻撃でビーム
            b.pendingAttack = beam ? 2 : 1;                   // 1:弾 2:ビーム
            b.prepareTimer = beam ? BEAM_PREPARE : SHOT_PREPARE;
            b.isPreparing = true;
        }

        // --------- 予備動作中 ---------
        if (b.isPreparing)
        {
            b.prepareTimer -= dt;
            if (b.prepareTimer <= 0.0f)
            {
                b.isPreparing = false;
                b.isAttacking = true;

                if (b.pendingAttack == 1)
                {
                    // 弾を発射
                    const float tx = player.posX;
                    const float ty = player.posY - PLAYER_HEIGHT * 0.5f;
                    
                    if (b.phase2)
                    {
                        // フェーズ2：中央から追従弾を1発だけ発射
                        SpawnStoneBullet(b, b.posX, b.posY - b.height * 0.62f, tx, ty, true);
                    }
                    else
                    {
                        // フェーズ1：中央から非追従弾を1発発射
                        SpawnStoneBullet(b, b.posX, b.posY - b.height * 0.62f, tx, ty, false);
                    }
                    b.attackTimer = 0.35f;
                    b.cooldownTimer = SHOT_RECOVERY;
                }
                else
                {
                    // ビームを発射
                    SpawnStoneBeam(b, player.posX);
                    b.attackTimer = BEAM_ACTIVE;
                    b.cooldownTimer = BEAM_RECOVERY;
                }

                b.attackCounter++;
                ResetAnimation(b.attack);
            }
        }

        // --------- 攻撃中 ---------
        if (b.isAttacking)
        {
            b.attackTimer -= dt;
            if (b.attackTimer <= 0.0f)
            {
                b.isAttacking = false;
                // フェーズ2で5回目の攻撃後に連射モードへ
                if (b.phase2 && !b.barrageActive && b.attackCounter >= 5)
                {
                    b.attackCounter = 0;
                    b.barrageActive = true;
                    b.barrageTimer = BARRAGE_DURATION;
                    b.barrageShotTimer = 0.0f;
                }
            }
        }

        // --------- 当たり判定の更新 ---------
        if (b.colliderId != -1)
        {
            const float bodyW = b.width * 0.42f;
            const float bodyH = b.height * 0.82f;
            const float left = b.posX - bodyW * 0.5f;
            const float top = (b.posY - b.height * 0.5f) - bodyH * 0.5f;
            UpdateCollider(b.colliderId, left, top, bodyW, bodyH);
        }

        // --------- アニメーション更新 ---------
        if (b.isAttacking || b.isPreparing)
        {
            UpdateAnimation(b.attack);
        }
        else
        {
            UpdateAnimation(b.idle);
        }
    }

    // --------- プロジェクタイル（弾・ビーム）の更新 ---------
    const PlayerData& p = GetPlayerData();
    const float block = 32.0f;
    for (auto& pr : g_midProjectiles)
    {
        if (!pr.active) continue;

        pr.lifeTimer -= dt;
        pr.animCounter++;

        // --------- ホーミング処理（中央の弾のみ） ---------
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

        // --------- 移動 ---------
        pr.posX += pr.velocityX * GetDeathSlowMotionScale();
        pr.posY += pr.velocityY * GetDeathSlowMotionScale();
        pr.facingRight = (pr.velocityX >= 0.0f);

        // --------- プレイヤー衝突判定（追従弾のみ） ---------
        if (pr.kind == StoneBullet && pr.homing)
        {
            // プレイヤーとの距離を計算
            float playerCenterX = p.posX + PLAYER_WIDTH * 0.5f;
            float playerCenterY = p.posY + PLAYER_HEIGHT * 0.5f;
            float dx = pr.posX - playerCenterX;
            float dy = pr.posY - playerCenterY;
            float dist = std::sqrt(dx * dx + dy * dy);

            // 弾のサイズとプレイヤーのサイズでの衝突判定
            float collisionDist = (pr.colliderWidth * 0.5f) + (PLAYER_WIDTH * 0.5f);

            if (dist < collisionDist)
            {
                // プレイヤーに当たった（回避中でも消える）
                ClearProjectile(pr);
                continue;
            }
        }

        // --------- 弾のみ：ブロック衝突判定 ---------
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

        // --------- 生存時間チェック ---------
        if (pr.lifeTimer <= 0.0f)
        {
            ClearProjectile(pr);
            continue;
        }

        // --------- 当たり判定更新 ---------
        if (pr.colliderId != -1)
        {
            UpdateCollider(
                pr.colliderId,
                pr.posX - pr.colliderWidth * 0.5f,
                pr.posY - pr.colliderHeight * 0.5f,
                pr.colliderWidth,
                pr.colliderHeight);
        }
    }
}

//============================================================
// ミッドボス描画（毎フレーム呼ばれる）
//============================================================

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

        const int left = drawX - drawW / 2;
        const int top = drawY - drawH;
        const int right = drawX + drawW / 2;
        const int bottom = drawY;

        // アニメーション選択
        const AnimationData* anim = &b.idle;
        if (b.isDead) anim = &b.die;
        else if (b.isPreparing || b.isAttacking) anim = &b.attack;

        const int handle = (anim->frames != nullptr) ? GetCurrentAnimationFrame(*anim) : -1;

        // 予備動作・攻撃中は赤く発光
        if (b.isPreparing || b.isAttacking)
        {
            SetDrawBright(255, 96, 96);
        }
        else
        {
            SetDrawBright(255, 255, 255);
        }

        // 描画
        if (handle != -1)
        {
            if (b.facingRight) DrawExtendGraph(left, top, right, bottom, handle, TRUE);
            else DrawExtendGraph(right, top, left, bottom, handle, TRUE);
        }
        else
        {
            DrawBox(left, top, right, bottom, GetColor(160, 160, 160), TRUE);
        }

        // 描画色をリセット
        SetDrawBright(255, 255, 255);
    }

    // 画面全体の描画色をリセット（敵以外への影響を防ぐ）
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
            // 弾を描画
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
            // ビームを描画（5フレーム分のアニメーション）
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

//============================================================
// ミッドボスダメージ処理
//============================================================

// 当たり判定IDからミッドボスを検索してダメージを与える
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

//============================================================
// ミッドボス状態照会
//============================================================

// ミッドボスが生存しているか
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

// ミッドボスのHP取得
int GetMidBossHP()
{
    for (const auto& b : g_midBosses)
    {
        if (b.active && !b.isDead) return b.hp;
    }
    return 0;
}

// ミッドボスの最大HP取得
int GetMidBossMaxHP()
{
    for (const auto& b : g_midBosses)
    {
        if (b.active) return b.maxHP;
    }
    return 0;
}
