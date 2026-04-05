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
    enum MidBossProjectileKind
    {
        StoneBullet = 0,
        StoneBeam = 1
    };

    struct MidBossProjectile
    {
        bool active = false;
        int kind = 0;
        float posX = 0.0f;
        float posY = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float colliderWidth = 0.0f;
        float colliderHeight = 0.0f;
        float speed = 0.0f;
        float lifeTimer = 0.0f;
        bool homing = false;
        float homingTurnRate = 0.0f;
        bool facingRight = true;
        int colliderId = -1;
        int animCounter = 0;
    };

    struct MidBossData
    {
        bool active = false;
        MidBossType type = MidBossType::StoneGolem;
        float posX = 0.0f;
        float posY = 0.0f;
        float width = 160.0f;
        float height = 160.0f;
        bool facingRight = true;

        int maxHP = 420;
        int hp = 420;
        int attackPower = 24;
        int colliderId = -1;

        AnimationData idle;
        AnimationData attack;
        AnimationData die;

        bool isDead = false;
        int deathBlinkTimer = 0;

        bool isAggro = false;
        float detectRange = 520.0f;

        bool isPreparing = false;
        float prepareTimer = 0.0f;
        int pendingAttack = 0; // 1:shot 2:beam
        bool isAttacking = false;
        float attackTimer = 0.0f;
        float cooldownTimer = 0.0f;

        bool phase2 = false;
        int attackCounter = 0;
        bool barrageActive = false;
        float barrageTimer = 0.0f;
        float barrageShotTimer = 0.0f;

        EnemyData attackOwner{};
    };

    std::vector<MidBossData> g_midBosses;
    std::vector<MidBossProjectile> g_midProjectiles;

    int g_stoneSheetHandle = -1;
    int g_stoneBulletHandle = -1;
    int g_stoneBulletGlowHandle = -1;
    int g_stoneBeamFrames[5] = { -1, -1, -1, -1, -1 };

    const float SHOT_PREPARE = 1.5f;
    const float BEAM_PREPARE = 2.0f;
    const float SHOT_RECOVERY = 2.0f;
    const float BEAM_RECOVERY = 4.0f;
    const float BEAM_ACTIVE = 1.0f;
    const float BARRAGE_DURATION = 4.0f;
    const float BARRAGE_INTERVAL = 0.22f;
    const float BULLET_SPEED = 6.0f;
    const float STONE_BULLET_DRAW_SIZE = 96.0f;
    const float STONE_BULLET_HITBOX_SIZE = 60.0f;

    void ClearProjectile(MidBossProjectile& p)
    {
        if (p.colliderId != -1)
        {
            DestroyCollider(p.colliderId);
            p.colliderId = -1;
        }
        p.active = false;
    }

    void ClearProjectiles()
    {
        for (auto& p : g_midProjectiles)
        {
            ClearProjectile(p);
        }
        g_midProjectiles.clear();
    }

    void LoadStoneAssets()
    {
        if (g_stoneSheetHandle == -1)
        {
            g_stoneSheetHandle = LoadGraph("Data/MidBoss/StoneGolem/Character_sheet.png");
        }

        if (g_stoneBulletHandle == -1)
        {
            g_stoneBulletHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile.png");
        }

        if (g_stoneBulletGlowHandle == -1)
        {
            g_stoneBulletGlowHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile_glowing.png");
        }

        if (g_stoneBeamFrames[0] == -1)
        {
            int loaded[5] = { -1, -1, -1, -1, -1 };
            int result = LoadDivGraph(
                "Data/MidBoss/StoneGolem/Laser_sheet.png",
                5,
                1,
                5,
                300,
                300,
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

    void InitStoneAnimations(MidBossData& b)
    {
        const char* sheetPath = "Data/MidBoss/StoneGolem/Character_sheet.png";
        int test = LoadGraph(sheetPath);
        if (test != -1)
        {
            DeleteGraph(test);
        }

        LoadAnimationFromSheetRange(b.idle, sheetPath, 0, 10, 8, 100, 100, 5, AnimationMode::Loop);
        LoadAnimationFromSheetRange(b.attack, sheetPath, 0, 20, 8, 100, 100, 4, AnimationMode::Once);
        LoadAnimationFromSheetRange(b.die, sheetPath, 0, 70, 12, 100, 100, 5, AnimationMode::Once);
    }

    void SpawnStoneBullet(MidBossData& b, float startX, float startY, float targetX, float targetY, bool homing)
    {
        float dx = targetX - startX;
        float dy = targetY - startY;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
        {
            dx = b.facingRight ? 1.0f : -1.0f;
            dy = 0.0f;
            len = 1.0f;
        }

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
        p.homing = homing;
        p.homingTurnRate = homing ? 0.08f : 0.0f;
        p.facingRight = (p.velocityX >= 0.0f);
        p.colliderId = CreateCollider(
            ColliderTag::Attack,
            p.posX - p.colliderWidth * 0.5f,
            p.posY - p.colliderHeight * 0.5f,
            p.colliderWidth,
            p.colliderHeight,
            &b.attackOwner);
        g_midProjectiles.push_back(p);
    }

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

void InitMidBossSystem()
{
    g_midBosses.clear();
    ClearProjectiles();
    LoadStoneAssets();
}

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

int SpawnMidBoss(MidBossType type, float x, float y)
{
    MidBossData b{};
    b.active = true;
    b.type = type;
    b.posX = x;
    b.posY = y;

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

    const float bodyW = b.width * 0.42f;
    const float bodyH = b.height * 0.82f;
    const float left = b.posX - bodyW * 0.5f;
    const float top = (b.posY - b.height * 0.5f) - bodyH * 0.5f;
    b.colliderId = CreateCollider(ColliderTag::Enemy, left, top, bodyW, bodyH, nullptr);

    g_midBosses.push_back(b);
    return static_cast<int>(g_midBosses.size() - 1);
}

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
            b.deathBlinkTimer = 0;
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

        b.facingRight = (dx >= 0.0f);
        b.phase2 = (b.hp <= b.maxHP / 2);
        b.isAggro = (dist <= b.detectRange);

        if (b.cooldownTimer > 0.0f) b.cooldownTimer -= dt;

        if (b.barrageActive)
        {
            b.barrageTimer -= dt;
            b.barrageShotTimer -= dt;
            if (b.barrageShotTimer <= 0.0f)
            {
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

        if (!b.isPreparing && !b.isAttacking && !b.barrageActive && b.isAggro && b.cooldownTimer <= 0.0f)
        {
            const bool beam = ((b.attackCounter % 4) == 3);
            b.pendingAttack = beam ? 2 : 1;
            b.prepareTimer = beam ? BEAM_PREPARE : SHOT_PREPARE;
            b.isPreparing = true;
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
                    const bool homing = b.phase2;
                    SpawnStoneBullet(b, b.posX, b.posY - b.height * 0.62f, tx, ty, homing);
                    if (b.phase2)
                    {
                        SpawnStoneBullet(b, b.posX - b.width * 0.35f, b.posY - b.height * 0.62f, tx, ty, false);
                        SpawnStoneBullet(b, b.posX + b.width * 0.35f, b.posY - b.height * 0.62f, tx, ty, false);
                        SpawnStoneBullet(b, b.posX, b.posY - b.height * 0.95f, tx, ty, false);
                    }
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
                ResetAnimation(b.attack);
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
            UpdateAnimation(b.attack);
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
            UpdateCollider(
                pr.colliderId,
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

    for (const auto& b : g_midBosses)
    {
        if (!b.active) continue;
        if (b.isDead && IsAnimationFinished(b.die) && ((b.deathBlinkTimer / 4) % 2 == 1)) continue;

        int drawX = static_cast<int>((b.posX - camera.posX) * camera.scale);
        int drawY = static_cast<int>((b.posY - camera.posY) * camera.scale);
        int drawW = static_cast<int>(b.width * camera.scale);
        int drawH = static_cast<int>(b.height * camera.scale);

        const int left = drawX - drawW / 2;
        const int top = drawY - drawH;
        const int right = drawX + drawW / 2;
        const int bottom = drawY;

        const AnimationData* anim = &b.idle;
        if (b.isDead) anim = &b.die;
        else if (b.isPreparing || b.isAttacking) anim = &b.attack;

        const int handle = (anim->frames != nullptr) ? GetCurrentAnimationFrame(*anim) : -1;

        if (b.isPreparing || b.isAttacking)
        {
            SetDrawBright(255, 96, 96);
        }
        else
        {
            SetDrawBright(255, 255, 255);
        }

        if (handle != -1)
        {
            if (b.facingRight) DrawExtendGraph(left, top, right, bottom, handle, TRUE);
            else DrawExtendGraph(right, top, left, bottom, handle, TRUE);
        }
        else
        {
            DrawBox(left, top, right, bottom, GetColor(160, 160, 160), TRUE);
        }

        SetDrawBright(255, 255, 255);
    }

    SetDrawBright(255, 255, 255);

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
                const int srcX = 0;
                const int srcY = 100;
                const int srcW = 300;
                const int srcH = 100;
                if (p.facingRight)
                {
                    DrawRectExtendGraph(right, top, left, bottom, srcX, srcY, srcW, srcH, beamFrame, TRUE);
                }
                else
                {
                    DrawRectExtendGraph(left, top, right, bottom, srcX, srcY, srcW, srcH, beamFrame, TRUE);
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
