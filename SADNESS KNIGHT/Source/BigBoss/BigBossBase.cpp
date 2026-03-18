#include "BigBossBase.h"
#include "../Camera/Camera.h"
#include "../Collision/Collision.h"
#include "../Player/Player.h"
#include "../Map/MapManager.h"
#include "../Enemy/EnemyBase.h"
#include "DxLib.h"
#include <vector>
#include <cmath>
#include <cstdio>

namespace
{
    struct FrameAnim
    {
        std::vector<int> frames;
        int frame = 0;
        int counter = 0;
        int speed = 6;
    };

    enum class KetherState
    {
        Idle1,
        ThrowBook,
        Fire1,
        Transform,
        Idle2,
        Fire2,
        Fist2,
        Died
    };

    struct BigBossData
    {
        bool active = false;
        BigBossType type = BigBossType::Kether;
        float posX = 0.0f;
        float posY = 0.0f;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float width = 140.0f;
        float height = 140.0f;
        int colliderId = -1;
        int attackColliderId = -1;
        bool isFacingRight = true;

        int maxHP = 500;
        int hp = 500;
        bool phase2 = false;
        float stateTimer = 0.0f;
        float aiTimer = 0.0f;
        KetherState state = KetherState::Idle1;

        FrameAnim idle1;
        FrameAnim throwBook;
        FrameAnim fire1;
        FrameAnim transform;
        FrameAnim idle2;
        FrameAnim fire2;
        FrameAnim fist2;
        FrameAnim died;

        FrameAnim bookFx;
        FrameAnim fireFx1;
        FrameAnim fireFx2;

        int shadowIdle = -1;
        int shadowThrowBook = -1;
        int shadowFire1 = -1;
        int shadowTransform = -1;
        int shadowIdle2 = -1;
        int shadowFire2 = -1;
        int shadowFist2 = -1;
        int shadowDied = -1;

        int bookProjectileAnim = 0;
        int fireEffectAnim = 0;
        float fireEffectX = 0.0f;
        float fireEffectY = 0.0f;
        bool showFireEffect = false;

        EnemyData attackOwner{};
        bool rewardGranted = false;

        float fistTargetX = 0.0f;
        float fistTargetY = 0.0f;
        bool fistSlamDone = false;
    };

    std::vector<BigBossData> g_bigBosses;

    const float KETHER_BODY_HALF_WIDTH_RATIO = 0.15f;
    const float KETHER_BODY_WIDTH_RATIO = 0.30f;
    const float KETHER_BODY_HEIGHT_RATIO = 0.52f;

    void ReleaseAnim(FrameAnim& a)
    {
        for (int h : a.frames)
        {
            if (h != -1) DeleteGraph(h);
        }
        a.frames.clear();
        a.frame = 0;
        a.counter = 0;
    }

    void UpdateAnim(FrameAnim& a, bool loop)
    {
        if (a.frames.empty()) return;
        a.counter++;
        if (a.counter < a.speed) return;
        a.counter = 0;

        if (loop)
        {
            a.frame = (a.frame + 1) % static_cast<int>(a.frames.size());
        }
        else if (a.frame + 1 < static_cast<int>(a.frames.size()))
        {
            a.frame++;
        }
    }

    bool IsAnimFinished(const FrameAnim& a)
    {
        if (a.frames.empty()) return true;
        return a.frame >= static_cast<int>(a.frames.size()) - 1;
    }

    int GetAnimHandle(const FrameAnim& a)
    {
        if (a.frames.empty()) return -1;
        if (a.frame < 0 || a.frame >= static_cast<int>(a.frames.size())) return -1;
        return a.frames[a.frame];
    }

    void ResetAnim(FrameAnim& a)
    {
        a.frame = 0;
        a.counter = 0;
    }

    void LoadFrames(FrameAnim& out, const char* folder, const char* prefix, int start, int count, int speed)
    {
        out.speed = speed;
        out.frame = 0;
        out.counter = 0;
        out.frames.reserve(count);

        char path[320];
        for (int i = 0; i < count; ++i)
        {
            const int index = start + i;
            sprintf_s(path, sizeof(path), "%s%s_%d.png", folder, prefix, index);
            int h = LoadGraph(path);
            if (h == -1)
            {
                sprintf_s(path, sizeof(path), "%s%s%d.png", folder, prefix, index);
                h = LoadGraph(path);
            }

            if (h != -1)
            {
                out.frames.push_back(h);
            }
        }
    }

    int LoadSingle(const char* path)
    {
        return LoadGraph(path);
    }

    float GetGroundYNearPlayerX(float playerX, float fallbackY)
    {
        const float block = 32.0f;
        const int gridX = static_cast<int>(playerX / block);
        const int startY = static_cast<int>(fallbackY / block);

        for (int y = startY; y < startY + 12; ++y)
        {
            MapChipData* mc = GetMapChipData(gridX, y);
            if (mc != nullptr && (mc->mapChip == NORMAL_BLOCK || mc->mapChip == SEMI_SOLID_BLOCK))
            {
                return y * block;
            }
        }

        const float mapBottom = static_cast<float>(GetMapHeight());
        return (mapBottom > 0.0f) ? mapBottom : fallbackY;
    }

    void LoadKetherAssets(BigBossData& b)
    {
        LoadFrames(b.idle1, "Data/BigBoss/Kether/Idle/", "WingedMozgus", 1, 6, 6);
        LoadFrames(b.throwBook, "Data/BigBoss/Kether/Throw Book/PNG/", "Mozgus", 1, 4, 5);
        LoadFrames(b.fire1, "Data/BigBoss/Kether/Breathing FIre/PNG/", "Mozgus", 1, 13, 4);
        LoadFrames(b.transform, "Data/BigBoss/Kether/Transform/PNG/", "Mozgus", 1, 13, 5);
        LoadFrames(b.idle2, "Data/BigBoss/Kether/Transform Idle/PNG/", "Mozgus", 1, 6, 6);
        LoadFrames(b.fire2, "Data/BigBoss/Kether/Transform Fire Breath/PNG/", "Mozgus", 1, 15, 4);
        LoadFrames(b.fist2, "Data/BigBoss/Kether/Transform Fist Atk/PNG/", "Mozgus", 1, 11, 4);
        LoadFrames(b.died, "Data/BigBoss/Kether/Died/PNG/", "Mozgus", 1, 9, 6);

        LoadFrames(b.bookFx, "Data/BigBoss/Kether/Throw Book/PNG/", "Book", 1, 2, 6);
        LoadFrames(b.fireFx1, "Data/BigBoss/Kether/Breathing FIre/PNG/", "Fire", 1, 8, 3);
        LoadFrames(b.fireFx2, "Data/BigBoss/Kether/Transform Fire Breath/PNG/", "Fire", 1, 10, 3);

        b.shadowIdle = LoadSingle("Data/BigBoss/Kether/Throw Book/PNG/Shadow.png");
        b.shadowThrowBook = LoadSingle("Data/BigBoss/Kether/Throw Book/PNG/Shadow.png");
        b.shadowFire1 = LoadSingle("Data/BigBoss/Kether/Breathing FIre/PNG/Shadow.png");
        b.shadowTransform = LoadSingle("Data/BigBoss/Kether/Transform/PNG/Shadow.png");
        b.shadowIdle2 = LoadSingle("Data/BigBoss/Kether/Transform Idle/PNG/Shadow.png");
        b.shadowFire2 = LoadSingle("Data/BigBoss/Kether/Transform Fire Breath/PNG/Shadow.png");
        b.shadowFist2 = LoadSingle("Data/BigBoss/Kether/Transform Fist Atk/PNG/Shadow.png");
        b.shadowDied = LoadSingle("Data/BigBoss/Kether/Died/PNG/Shadow.png");
    }

    void UnloadKetherAssets(BigBossData& b)
    {
        ReleaseAnim(b.idle1);
        ReleaseAnim(b.throwBook);
        ReleaseAnim(b.fire1);
        ReleaseAnim(b.transform);
        ReleaseAnim(b.idle2);
        ReleaseAnim(b.fire2);
        ReleaseAnim(b.fist2);
        ReleaseAnim(b.died);

        ReleaseAnim(b.bookFx);
        ReleaseAnim(b.fireFx1);
        ReleaseAnim(b.fireFx2);

        int* shadows[] = {
            &b.shadowIdle, &b.shadowThrowBook, &b.shadowFire1, &b.shadowTransform,
            &b.shadowIdle2, &b.shadowFire2, &b.shadowFist2, &b.shadowDied
        };
        for (int* s : shadows)
        {
            if (*s != -1)
            {
                DeleteGraph(*s);
                *s = -1;
            }
        }
    }

    void UpdateKether(BigBossData& b)
    {
        PlayerData& player = GetPlayerData();
        const float dt = 1.0f / 60.0f;

        b.aiTimer += dt;
        b.stateTimer += dt;

        if (b.state == KetherState::Died)
        {
            const float gravity = 0.32f;
            const float maxFall = 8.0f;
            const float deathGroundVisualOffset = 24.0f;
            const float prevY = b.posY;

            b.velocityY += gravity;
            if (b.velocityY > maxFall) b.velocityY = maxFall;
            b.posY += b.velocityY;

            // 死亡落下中も床ブロックで停止（経路チェックで貫通防止）
            const float halfW = b.width * KETHER_BODY_HALF_WIDTH_RATIO;
            const float checkLeft = b.posX - halfW;
            const float checkRight = b.posX + halfW;

            const float moveY = b.posY - prevY;
            int ySteps = static_cast<int>(std::fabs(moveY) / 4.0f) + 1;
            if (ySteps < 1) ySteps = 1;
            if (ySteps > 8) ySteps = 8;

            bool landed = false;
            for (int s = 0; s <= ySteps && !landed; ++s)
            {
                const float t = static_cast<float>(s) / static_cast<float>(ySteps);
                const float sampleBottom = prevY + moveY * t;

                for (int i = 0; i < 3; ++i)
                {
                    const float checkX = checkLeft + (checkRight - checkLeft) * (i / 2.0f);
                    const int gridX = static_cast<int>(checkX / 32.0f);
                    const int gridY = static_cast<int>((sampleBottom + 1.0f) / 32.0f);
                    MapChipData* mc = GetMapChipData(gridX, gridY);
                    if (mc != nullptr && (mc->mapChip == NORMAL_BLOCK || mc->mapChip == SEMI_SOLID_BLOCK))
                    {
                        const float blockTop = gridY * 32.0f;
                        if (sampleBottom >= blockTop)
                        {
                            b.posY = blockTop + deathGroundVisualOffset;
                            b.velocityY = 0.0f;
                            landed = true;
                            break;
                        }
                    }
                }
            }

            const float mapBottom = static_cast<float>(GetMapHeight());
            if (mapBottom > 0.0f && b.posY > mapBottom + deathGroundVisualOffset)
            {
                b.posY = mapBottom + deathGroundVisualOffset;
                b.velocityY = 0.0f;
            }

            UpdateAnim(b.died, false);
            return;
        }

        // 低空飛行
        const float targetY = player.posY - 56.0f;
        float dy = targetY - b.posY;
        if (dy > 2.0f) dy = 2.0f;
        if (dy < -2.0f) dy = -2.0f;
        b.posY += dy * 0.35f;

        float dx = player.posX - b.posX;
        b.isFacingRight = (dx >= 0.0f);
        float hover = b.phase2 ? 84.0f : 96.0f;
        float speed = b.phase2 ? 2.3f : 1.6f;
        if (std::fabs(dx) > hover)
        {
            b.posX += (dx > 0.0f ? speed : -speed);
        }

        if (!b.phase2 && b.hp <= (b.maxHP / 2))
        {
            b.phase2 = true;
            b.state = KetherState::Transform;
            b.stateTimer = 0.0f;
            ResetAnim(b.transform);
            player.velocityX = (player.posX >= b.posX) ? 9.0f : -9.0f;
            player.velocityY = -2.0f;
        }

        switch (b.state)
        {
        case KetherState::Idle1:
            UpdateAnim(b.idle1, true);
            if (b.stateTimer > 1.6f)
            {
                b.stateTimer = 0.0f;
                if ((GetRand(99)) < 45)
                {
                    b.state = KetherState::ThrowBook;
                    ResetAnim(b.throwBook);
                    ResetAnim(b.bookFx);
                    b.bookProjectileAnim = 0;
                }
                else
                {
                    b.state = KetherState::Fire1;
                    ResetAnim(b.fire1);
                    ResetAnim(b.fireFx1);
                    b.fireEffectAnim = 0;
                    b.showFireEffect = true;
                }
            }
            break;

        case KetherState::ThrowBook:
            UpdateAnim(b.throwBook, false);
            UpdateAnim(b.bookFx, true);
            if (b.stateTimer > 1.0f)
            {
                b.state = b.phase2 ? KetherState::Idle2 : KetherState::Idle1;
                b.stateTimer = 0.0f;
            }
            break;

        case KetherState::Fire1:
            UpdateAnim(b.fire1, true);
            UpdateAnim(b.fireFx1, true);
            if (b.stateTimer > 2.6f)
            {
                b.showFireEffect = false;
                b.state = b.phase2 ? KetherState::Idle2 : KetherState::Idle1;
                b.stateTimer = 0.0f;
            }
            break;

        case KetherState::Transform:
            UpdateAnim(b.transform, false);
            if (IsAnimFinished(b.transform))
            {
                b.state = KetherState::Idle2;
                b.stateTimer = 0.0f;
                ResetAnim(b.idle2);
            }
            break;

        case KetherState::Idle2:
            UpdateAnim(b.idle2, true);
            if (b.stateTimer > 1.2f)
            {
                b.stateTimer = 0.0f;
                if (GetRand(99) < 65)
                {
                    b.state = KetherState::Fire2;
                    ResetAnim(b.fire2);
                    ResetAnim(b.fireFx2);
                    b.showFireEffect = true;
                }
                else
                {
                    b.state = KetherState::Fist2;
                    ResetAnim(b.fist2);
                    b.fistTargetX = player.posX;
                    b.fistTargetY = GetGroundYNearPlayerX(player.posX, player.posY);
                    b.fistSlamDone = false;
                }
            }
            break;

        case KetherState::Fire2:
            UpdateAnim(b.fire2, true);
            UpdateAnim(b.fireFx2, true);
            if (b.stateTimer > 3.4f)
            {
                b.showFireEffect = false;
                b.state = KetherState::Idle2;
                b.stateTimer = 0.0f;
            }
            break;

        case KetherState::Fist2:
            UpdateAnim(b.fist2, false);
            if (!b.fistSlamDone)
            {
                const float toX = b.fistTargetX - b.posX;
                if (toX > 7.5f) b.posX += 7.5f;
                else if (toX < -7.5f) b.posX -= 7.5f;
                else b.posX = b.fistTargetX;

                const float targetY = b.fistTargetY + 20.0f;
                const float toY = targetY - b.posY;
                if (toY > 7.5f) b.posY += 7.5f;
                else if (toY < -7.5f) b.posY -= 7.5f;
                else b.posY = targetY;

                if ((std::fabs(b.posX - b.fistTargetX) <= 8.0f && std::fabs(b.posY - targetY) <= 10.0f && b.stateTimer >= 0.30f) || b.stateTimer >= 0.75f)
                {
                    b.fistSlamDone = true;
                }
            }
            if (b.stateTimer > 1.2f)
            {
                b.state = KetherState::Idle2;
                b.stateTimer = 0.0f;
            }
            break;

        case KetherState::Died:
            UpdateAnim(b.died, false);
            break;
        }

        if (b.colliderId != -1)
        {
            const float left = b.posX - (b.width * KETHER_BODY_HALF_WIDTH_RATIO);
            const float top = b.posY - (b.height * KETHER_BODY_HEIGHT_RATIO);
            const float w = b.width * KETHER_BODY_WIDTH_RATIO;
            const float h = b.height * KETHER_BODY_HEIGHT_RATIO;
            UpdateCollider(b.colliderId, left, top, w, h);
        }

        const bool fireState = (b.state == KetherState::Fire1 || b.state == KetherState::Fire2);
        const bool fistState = (b.state == KetherState::Fist2);
        if (fireState || fistState)
        {
            float atkW = 0.0f;
            float atkH = 0.0f;
            float atkX = b.posX;
            float atkY = b.posY;
            bool enableAttack = true;

            if (fireState)
            {
                atkW = (b.state == KetherState::Fire2) ? 92.0f : 78.0f;
                atkH = (b.state == KetherState::Fire2) ? 220.0f : 185.0f;
                atkY = b.posY + (atkH * 0.62f);
            }
            else
            {
                atkW = b.phase2 ? 128.0f : 96.0f;
                atkH = b.phase2 ? 52.0f : 64.0f;
                if (b.phase2)
                {
                    atkX = b.fistTargetX;
                    atkY = b.fistTargetY + 14.0f;
                    enableAttack = b.fistSlamDone && (b.stateTimer >= 0.30f && b.stateTimer <= 1.00f);
                }
                else
                {
                    atkX = b.posX + (b.isFacingRight ? b.width * 0.45f : -b.width * 0.45f);
                    atkY = b.posY - b.height * 0.35f;
                }
            }

            const float left = atkX - atkW * 0.5f;
            const float top = atkY - atkH * 0.5f;

            b.attackOwner.attackPower = fireState ? (b.state == KetherState::Fire2 ? 22 : 14) : 18;
            if (!enableAttack)
            {
                if (b.attackColliderId != -1)
                {
                    DestroyCollider(b.attackColliderId);
                    b.attackColliderId = -1;
                }
            }
            else if (b.attackColliderId == -1)
            {
                b.attackColliderId = CreateCollider(ColliderTag::Attack, left, top, atkW, atkH, &b.attackOwner);
            }
            else
            {
                UpdateCollider(b.attackColliderId, left, top, atkW, atkH);
            }
        }
        else if (b.attackColliderId != -1)
        {
            DestroyCollider(b.attackColliderId);
            b.attackColliderId = -1;
        }
    }

    void DrawAnimFit(const CameraData& camera, float x, float y, float w, float h, int handle)
    {
        if (handle == -1) return;
        const int cx = static_cast<int>((x - camera.posX) * camera.scale);
        const int cy = static_cast<int>((y - camera.posY) * camera.scale);
        const int halfW = static_cast<int>(w * 0.5f * camera.scale);
        const int hh = static_cast<int>(h * camera.scale);
        DrawExtendGraph(cx - halfW, cy - hh, cx + halfW, cy, handle, TRUE);
    }
}

int GetBigBossHP()
{
    for (const auto& b : g_bigBosses)
    {
        if (!b.active) continue;
        return b.hp;
    }
    return 0;
}

int GetBigBossMaxHP()
{
    for (const auto& b : g_bigBosses)
    {
        if (!b.active) continue;
        return b.maxHP;
    }
    return 1;
}

bool IsBigBossAlive()
{
    for (const auto& b : g_bigBosses)
    {
        if (b.active) return true;
    }
    return false;
}

void InitBigBossSystem()
{
    g_bigBosses.clear();
}

int SpawnBigBoss(BigBossType type, float x, float y)
{
    BigBossData b{};
    b.active = true;
    b.type = type;
    b.posX = x;
    b.posY = y;
    b.width = 340.0f;
    b.height = 340.0f;
    b.maxHP = 500;
    b.hp = 500;
    b.state = KetherState::Idle1;

    if (type == BigBossType::Kether)
    {
        LoadKetherAssets(b);
        b.attackOwner.type = EnemyType::BigQuartist;
        b.attackOwner.attackPower = 14;
    }

    const float left = b.posX - (b.width * KETHER_BODY_HALF_WIDTH_RATIO);
    const float top = b.posY - (b.height * KETHER_BODY_HEIGHT_RATIO);
    const float w = b.width * KETHER_BODY_WIDTH_RATIO;
    const float h = b.height * KETHER_BODY_HEIGHT_RATIO;
    b.colliderId = CreateCollider(ColliderTag::Enemy, left, top, w, h, nullptr);

    g_bigBosses.push_back(b);
    return static_cast<int>(g_bigBosses.size() - 1);
}

void UpdateBigBosses()
{
    for (auto& b : g_bigBosses)
    {
        if (!b.active) continue;
        if (b.type == BigBossType::Kether)
        {
            UpdateKether(b);
        }
    }
}

void DrawBigBosses()
{
    const CameraData camera = GetCamera();

    for (const auto& b : g_bigBosses)
    {
        if (!b.active) continue;

        int body = -1;
        int fx = -1;

        switch (b.state)
        {
        case KetherState::Idle1:
            body = GetAnimHandle(b.idle1);
            break;
        case KetherState::ThrowBook:
            body = GetAnimHandle(b.throwBook);
            fx = GetAnimHandle(b.bookFx);
            break;
        case KetherState::Fire1:
            body = GetAnimHandle(b.fire1);
            fx = GetAnimHandle(b.fireFx1);
            break;
        case KetherState::Transform:
            body = GetAnimHandle(b.transform);
            break;
        case KetherState::Idle2:
            body = GetAnimHandle(b.idle2);
            break;
        case KetherState::Fire2:
            body = GetAnimHandle(b.fire2);
            fx = GetAnimHandle(b.fireFx2);
            break;
        case KetherState::Fist2:
            body = GetAnimHandle(b.fist2);
            break;
        case KetherState::Died:
            body = GetAnimHandle(b.died);
            break;
        }

        DrawAnimFit(camera, b.posX, b.posY, b.width, b.height, body);

        const int cLeft = static_cast<int>(((b.posX - (b.width * KETHER_BODY_HALF_WIDTH_RATIO)) - camera.posX) * camera.scale);
        const int cTop = static_cast<int>(((b.posY - (b.height * KETHER_BODY_HEIGHT_RATIO)) - camera.posY) * camera.scale);
        const int cRight = static_cast<int>(((b.posX + (b.width * KETHER_BODY_HALF_WIDTH_RATIO)) - camera.posX) * camera.scale);
        const int cBottom = static_cast<int>(((b.posY) - camera.posY) * camera.scale);
        DrawBox(cLeft, cTop, cRight, cBottom, GetColor(255, 80, 80), FALSE);

        if (fx != -1)
        {
            const float dir = (GetPlayerData().posX >= b.posX) ? 1.0f : -1.0f;
            float fxX = b.posX;
            float fxY = b.posY - b.height * 0.70f;
            float fxW = 52.0f;
            float fxH = 36.0f;

            if (b.state == KetherState::ThrowBook)
            {
                fxW = 88.0f;
                fxH = 60.0f;
                fxX = b.posX + dir * (8.0f + b.stateTimer * 20.0f);
                fxY = b.posY - b.height * 0.68f + b.stateTimer * 220.0f;
            }
            else if (b.state == KetherState::Fire1 || b.state == KetherState::Fire2)
            {
                fxW = (b.state == KetherState::Fire2) ? 112.0f : 94.0f;
                fxH = (b.state == KetherState::Fire2) ? 225.0f : 190.0f;
                fxX = b.posX;
                fxY = b.posY + fxH * 0.60f;
                const float pulse = 1.0f + 0.08f * std::sin(b.stateTimer * 18.0f);
                fxW *= pulse;
            }

            DrawAnimFit(camera, fxX, fxY, fxW, fxH, fx);
        }
    }
}

void ClearBigBosses()
{
    for (auto& b : g_bigBosses)
    {
        if (b.colliderId != -1)
        {
            DestroyCollider(b.colliderId);
            b.colliderId = -1;
        }
        if (b.attackColliderId != -1)
        {
            DestroyCollider(b.attackColliderId);
            b.attackColliderId = -1;
        }

        if (b.type == BigBossType::Kether)
        {
            UnloadKetherAssets(b);
        }

        b.active = false;
    }
    g_bigBosses.clear();
}

bool DamageBigBossByColliderId(int colliderId, int damage)
{
    if (damage <= 0) damage = 1;

    for (auto& b : g_bigBosses)
    {
        if (!b.active) continue;
        if (b.colliderId != colliderId) continue;

        if (b.state == KetherState::Died) return true;

        b.hp -= damage;
        if (b.hp < 0) b.hp = 0;

        if (b.hp <= 0)
        {
            b.state = KetherState::Died;
            b.stateTimer = 0.0f;
            b.velocityX = 0.0f;
            b.velocityY = 0.0f;
            ResetAnim(b.died);

            if (!b.rewardGranted)
            {
                UnlockDoubleJump();
                b.rewardGranted = true;
            }

            if (b.attackColliderId != -1)
            {
                DestroyCollider(b.attackColliderId);
                b.attackColliderId = -1;
            }
            if (b.colliderId != -1)
            {
                DestroyCollider(b.colliderId);
                b.colliderId = -1;
            }
        }

        return true;
    }

    return false;
}


