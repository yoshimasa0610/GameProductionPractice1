#include "BigBossBase.h"
#include "Kether/Kether.h"
#include "Tiphereth/Tiphereth.h"
#include "../Camera/Camera.h"
#include "../Collision/Collision.h"
#include "../Player/Player.h"
#include "../Map/MapManager.h"
#include "../Map/StageManager.h"
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
        TipherethIdle,
        TipherethPrepare,
        TipherethTentacle,
        TipherethBite,
        TipherethStomp,
        TipherethCharge,
        TipherethDied,
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

        FrameAnim tipherethIdle;
        FrameAnim tipherethPrepare;
        FrameAnim tipherethTentacle;
        FrameAnim tipherethBite;
        FrameAnim tipherethStomp;
        FrameAnim tipherethCharge;
        FrameAnim tipherethDied;

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

        float bookTargetX = 0.0f;
        float bookTargetY = 0.0f;
        float bookStartX = 0.0f;
        float bookStartY = 0.0f;

        bool phaseShockwaveActive = false;
        float phaseShockwaveTimer = 0.0f;
        bool transformBlastDone = false;
        float transformPostHoldTimer = 0.0f;

        float tipherethGroundY = 0.0f;
        float tipherethTargetX = 0.0f;
        float tipherethTargetY = 0.0f;
        float tipherethChargeTargetX = 0.0f;
        KetherState tipherethQueuedState = KetherState::TipherethIdle;
        float tipherethPrepareDuration = 0.0f;
        bool tipherethHitDone1 = false;
        bool tipherethHitDone2 = false;
        bool tipherethStompResolved = false;
        bool tipherethStompShakeDone = false;
    };

    std::vector<BigBossData> g_bigBosses;
    const KetherTuning& KETHER_TUNING = GetKetherTuning();
    const TipherethTuning& TIPHERETH_TUNING = GetTipherethTuning();

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

    void SetAnimProgress(FrameAnim& a, float progress)
    {
        if (a.frames.empty()) return;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        const int lastFrame = static_cast<int>(a.frames.size()) - 1;
        a.frame = static_cast<int>(progress * static_cast<float>(lastFrame));
        if (a.frame < 0) a.frame = 0;
        if (a.frame > lastFrame) a.frame = lastFrame;
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

    float GetGroundYBelowX(float worldX, float startY, float fallbackY)
    {
        const float block = 32.0f;
        const int gridX = static_cast<int>(worldX / block);

        int startGridY = static_cast<int>(startY / block);
        if (startGridY < 0) startGridY = 0;

        int maxGridY = static_cast<int>(GetMapHeight() / block) + 2;
        if (maxGridY < startGridY + 1) maxGridY = startGridY + 1;

        for (int y = startGridY; y <= maxGridY; ++y)
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
        LoadFrames(b.transform, "Data/BigBoss/Kether/Transform/PNG/", "Mozgus", 1, 13, 12);
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

    void LoadTipherethAssets(BigBossData& b)
    {
        LoadFrames(b.tipherethIdle, "Data/BigBoss/Tiphereth/Monster Count Idle/PNG/", "Monster_Count_Idle", 1, 6, 6);
        LoadFrames(b.tipherethPrepare, "Data/BigBoss/Tiphereth/Monster Count Prepare/PNG/", "Monster_Count_Prepare", 1, 9, 6);
        LoadFrames(b.tipherethTentacle, "Data/BigBoss/Tiphereth/Monster Count Attack Tentacles/PNG/", "Monster_Count_Attack_Tentacles", 1, 12, 4);
        LoadFrames(b.tipherethBite, "Data/BigBoss/Tiphereth/Monster Count Bite Attack/PNG/", "Monster_Count_Attack_Bite", 1, 6, 4);
        LoadFrames(b.tipherethStomp, "Data/BigBoss/Tiphereth/Monster Count Stomping Attack/PNG/", "Monster_Count_Attack_Stomping", 1, 8, 4);
        LoadFrames(b.tipherethCharge, "Data/BigBoss/Tiphereth/Monster Count Charge Attack/PNG/", "Monster_Count_Attack_Charge", 1, 6, 4);
        LoadFrames(b.tipherethDied, "Data/BigBoss/Tiphereth/Monster Count Death/PNG/", "Monster_Count_Death", 1, 13, 5);
    }

    void UnloadTipherethAssets(BigBossData& b)
    {
        ReleaseAnim(b.tipherethIdle);
        ReleaseAnim(b.tipherethPrepare);
        ReleaseAnim(b.tipherethTentacle);
        ReleaseAnim(b.tipherethBite);
        ReleaseAnim(b.tipherethStomp);
        ReleaseAnim(b.tipherethCharge);
        ReleaseAnim(b.tipherethDied);
    }

    void ClearBigBossAttackCollider(BigBossData& b)
    {
        if (b.attackColliderId != -1)
        {
            DestroyCollider(b.attackColliderId);
            b.attackColliderId = -1;
        }
    }

    void SetBigBossAttackCollider(BigBossData& b, float centerX, float centerY, float width, float height, int attackPower)
    {
        b.attackOwner.attackPower = attackPower;
        const float left = centerX - width * 0.5f;
        const float top = centerY - height * 0.5f;
        if (b.attackColliderId == -1)
        {
            b.attackColliderId = CreateCollider(ColliderTag::Attack, left, top, width, height, &b.attackOwner);
        }
        else
        {
            UpdateCollider(b.attackColliderId, left, top, width, height);
        }
    }

    void UpdateTiphereth(BigBossData& b)
    {
        PlayerData& player = GetPlayerData();
        const float dt = 1.0f / 60.0f;
        const float dx = player.posX - b.posX;
        const float absDx = std::fabs(dx);

        b.stateTimer += dt;
        b.phase2 = (b.hp <= (b.maxHP / 2));
        b.posY = b.tipherethGroundY;

        if (std::fabs(dx) > 8.0f)
        {
            b.isFacingRight = (dx >= 0.0f);
        }

        if (b.state == KetherState::TipherethDied)
        {
            SetAnimProgress(b.tipherethDied, b.stateTimer / 1.6f);
            ClearBigBossAttackCollider(b);
            if (b.stateTimer >= 1.6f)
            {
                b.active = false;
            }
            return;
        }

        auto EnterTipherethPrepare = [&](KetherState queuedState, float duration)
        {
            b.tipherethQueuedState = queuedState;
            b.tipherethPrepareDuration = duration;
            b.tipherethHitDone1 = false;
            b.tipherethHitDone2 = false;
            b.tipherethStompResolved = false;
            b.tipherethStompShakeDone = false;
            b.state = KetherState::TipherethPrepare;
            b.stateTimer = 0.0f;
            ResetAnim(b.tipherethIdle);
            ClearBigBossAttackCollider(b);
        };

        switch (b.state)
        {
        case KetherState::TipherethIdle:
        {
            UpdateAnim(b.tipherethIdle, true);
            ClearBigBossAttackCollider(b);

            const float idleWait = b.phase2 ? 0.45f : 0.70f;
            if (b.stateTimer >= idleWait)
            {
                b.tipherethTargetX = player.posX;
                b.tipherethTargetY = player.posY;

                if (absDx >= TIPHERETH_TUNING.chargePriorityRange)
                {
                    const float chargeDir = (b.tipherethTargetX >= b.posX) ? 1.0f : -1.0f;
                    float dashTargetX = b.tipherethTargetX + chargeDir * TIPHERETH_TUNING.chargeThroughDistance;
                    const float mapWidth = static_cast<float>(GetMapWidth());
                    if (mapWidth > 0.0f)
                    {
                        if (dashTargetX < 96.0f) dashTargetX = 96.0f;
                        if (dashTargetX > mapWidth - 96.0f) dashTargetX = mapWidth - 96.0f;
                    }
                    b.tipherethChargeTargetX = dashTargetX;
                    EnterTipherethPrepare(KetherState::TipherethCharge, TIPHERETH_TUNING.prepareDuration);
                }
                else
                {
                    const int roll = GetRand(99);
                    if (roll < 40)
                    {
                        EnterTipherethPrepare(KetherState::TipherethTentacle, TIPHERETH_TUNING.prepareDuration);
                    }
                    else if (roll < 68)
                    {
                        EnterTipherethPrepare(KetherState::TipherethBite, TIPHERETH_TUNING.prepareDuration);
                    }
                    else if (roll < 85)
                    {
                        EnterTipherethPrepare(KetherState::TipherethStomp, TIPHERETH_TUNING.stompPrepareDuration);
                    }
                    else
                    {
                        const float chargeDir = (b.tipherethTargetX >= b.posX) ? 1.0f : -1.0f;
                        float dashTargetX = b.tipherethTargetX + chargeDir * TIPHERETH_TUNING.chargeThroughDistance;
                        const float mapWidth = static_cast<float>(GetMapWidth());
                        if (mapWidth > 0.0f)
                        {
                            if (dashTargetX < 96.0f) dashTargetX = 96.0f;
                            if (dashTargetX > mapWidth - 96.0f) dashTargetX = mapWidth - 96.0f;
                        }
                        b.tipherethChargeTargetX = dashTargetX;
                        EnterTipherethPrepare(KetherState::TipherethCharge, TIPHERETH_TUNING.prepareDuration);
                    }
                }
            }
            break;
        }
        case KetherState::TipherethPrepare:
        {
            UpdateAnim(b.tipherethIdle, true);
            ClearBigBossAttackCollider(b);
            if (b.stateTimer >= b.tipherethPrepareDuration)
            {
                b.state = b.tipherethQueuedState;
                b.stateTimer = 0.0f;
                if (b.state == KetherState::TipherethTentacle) ResetAnim(b.tipherethTentacle);
                else if (b.state == KetherState::TipherethBite) ResetAnim(b.tipherethBite);
                else if (b.state == KetherState::TipherethStomp) ResetAnim(b.tipherethStomp);
                else if (b.state == KetherState::TipherethCharge) ResetAnim(b.tipherethCharge);
            }
            break;
        }
        case KetherState::TipherethTentacle:
        {
            const float totalDuration = 1.5f;
            const float progress = b.stateTimer / totalDuration;
            SetAnimProgress(b.tipherethTentacle, progress);

            const float centerX = b.posX + (b.isFacingRight ? TIPHERETH_TUNING.tentacleFrontOffset : -TIPHERETH_TUNING.tentacleFrontOffset);
            const float centerY = b.posY + TIPHERETH_TUNING.tentacleCenterYOffset;
            const bool firstHit = (!b.tipherethHitDone1 && b.stateTimer >= 0.45f && b.stateTimer <= 0.72f);
            const bool secondHit = (!b.tipherethHitDone2 && b.stateTimer >= 0.95f && b.stateTimer <= 1.22f);
            const bool activeHit = firstHit || secondHit;

            if (activeHit)
            {
                SetBigBossAttackCollider(b, centerX, centerY, TIPHERETH_TUNING.tentacleWidth, TIPHERETH_TUNING.tentacleHeight, GetTipherethAttackPower());
                if (firstHit) b.tipherethHitDone1 = true;
                if (secondHit) b.tipherethHitDone2 = true;
            }
            else
            {
                ClearBigBossAttackCollider(b);
            }

            if (b.stateTimer >= totalDuration)
            {
                b.state = KetherState::TipherethIdle;
                b.stateTimer = 0.0f;
                ResetAnim(b.tipherethIdle);
            }
            break;
        }
        case KetherState::TipherethBite:
        {
            const float totalDuration = 1.0f;
            const float progress = b.stateTimer / totalDuration;
            SetAnimProgress(b.tipherethBite, progress);

            const bool activeHit = (b.stateTimer >= 0.48f && b.stateTimer <= 0.78f);
            const float centerX = b.posX + (b.isFacingRight ? TIPHERETH_TUNING.biteFrontOffset : -TIPHERETH_TUNING.biteFrontOffset);
            const float centerY = b.posY + TIPHERETH_TUNING.biteCenterYOffset;
            if (activeHit)
            {
                SetBigBossAttackCollider(b, centerX, centerY, TIPHERETH_TUNING.biteWidth, TIPHERETH_TUNING.biteHeight, GetTipherethAttackPower() + 4);
            }
            else
            {
                ClearBigBossAttackCollider(b);
            }

            if (b.stateTimer >= totalDuration)
            {
                b.state = KetherState::TipherethIdle;
                b.stateTimer = 0.0f;
                ResetAnim(b.tipherethIdle);
            }
            break;
        }
        case KetherState::TipherethStomp:
        {
            const float totalDuration = 2.0f;
            const float progress = b.stateTimer / totalDuration;
            SetAnimProgress(b.tipherethStomp, progress);

            const float hitStart = 1.70f;
            const bool stompActive = (b.stateTimer >= hitStart && b.stateTimer <= totalDuration);
            if (stompActive)
            {
                const float mapWidth = static_cast<float>(GetMapWidth());
                const float attackWidth = (mapWidth > 0.0f) ? mapWidth : 4096.0f;
                const float attackHeight = 160.0f;
                const float attackCenterX = attackWidth * 0.5f;
                const float attackCenterY = b.tipherethGroundY - attackHeight * 0.5f + TIPHERETH_TUNING.stompHitboxYOffset;
                SetBigBossAttackCollider(b, attackCenterX, attackCenterY, attackWidth, attackHeight, GetTipherethAttackPower());
            }
            else
            {
                ClearBigBossAttackCollider(b);
            }

            if (!b.tipherethStompResolved && b.stateTimer >= hitStart)
            {
                if (IsPlayerGrounded())
                {
                    DamagePlayerHP(GetTipherethAttackPower());
                }
                b.tipherethStompResolved = true;
            }
            if (!b.tipherethStompShakeDone && b.stateTimer >= hitStart)
            {
                StartCameraShake(0.45f, 18.0f);
                b.tipherethStompShakeDone = true;
            }

            if (b.stateTimer >= totalDuration)
            {
                ClearBigBossAttackCollider(b);
                b.state = KetherState::TipherethIdle;
                b.stateTimer = 0.0f;
                ResetAnim(b.tipherethIdle);
            }
            break;
        }
        case KetherState::TipherethCharge:
        {
            UpdateAnim(b.tipherethCharge, true);
            const float activeStart = 0.18f;
            if (b.stateTimer >= activeStart)
            {
                const float dir = b.isFacingRight ? 1.0f : -1.0f;
                b.posX += dir * (b.phase2 ? (TIPHERETH_TUNING.chargeSpeed + 2.0f) : TIPHERETH_TUNING.chargeSpeed);
                const float centerX = b.posX + dir * TIPHERETH_TUNING.chargeFrontOffset;
                const float centerY = b.posY + TIPHERETH_TUNING.chargeCenterYOffset;
                SetBigBossAttackCollider(b, centerX, centerY, TIPHERETH_TUNING.chargeWidth, TIPHERETH_TUNING.chargeHeight, GetTipherethAttackPower() + 2);
            }
            else
            {
                ClearBigBossAttackCollider(b);
            }

            const bool reachedTarget = b.isFacingRight ? (b.posX >= b.tipherethChargeTargetX) : (b.posX <= b.tipherethChargeTargetX);
            if ((b.stateTimer >= activeStart && reachedTarget) || b.stateTimer >= 1.0f)
            {
                ClearBigBossAttackCollider(b);
                b.state = KetherState::TipherethIdle;
                b.stateTimer = 0.0f;
                ResetAnim(b.tipherethIdle);
            }
            break;
        }
        default:
            break;
        }

        const float bodyW = b.width * TIPHERETH_TUNING.bodyWidthRatio;
        const float bodyH = b.height * TIPHERETH_TUNING.bodyHeightRatio;
        const float bodyCenterX = b.posX + (b.isFacingRight ? 1.0f : -1.0f) * b.width * TIPHERETH_TUNING.bodyFacingOffsetRatio;
        const float bodyLeft = bodyCenterX - bodyW * 0.5f;
        const float bodyTop = b.posY - (b.height * TIPHERETH_TUNING.bodyTopOffsetRatio);
        if (b.colliderId != -1)
        {
            UpdateCollider(b.colliderId, bodyLeft, bodyTop, bodyW, bodyH);
        }
    }

    void UpdateKether(BigBossData& b)
    {
        PlayerData& player = GetPlayerData();
        const float dt = 1.0f / 60.0f;

        b.aiTimer += dt;
        b.stateTimer += dt;

        if (b.phaseShockwaveActive)
        {
            b.phaseShockwaveTimer += dt;
            if (b.phaseShockwaveTimer >= KETHER_TUNING.phaseShockwaveDuration)
            {
                b.phaseShockwaveActive = false;
                b.phaseShockwaveTimer = 0.0f;
            }
        }

        if (b.state == KetherState::Died)
        {
            const float gravity = 0.32f;
            const float maxFall = 8.0f;
            const float deathGroundVisualOffset = 26.0f;
            const float prevY = b.posY;

            b.velocityY += gravity;
            if (b.velocityY > maxFall) b.velocityY = maxFall;
            b.posY += b.velocityY;

            // 死亡落下中も床ブロックで停止（経路チェックで貫通防止）
            const float halfW = b.width * KETHER_TUNING.bodyHalfWidthRatio;
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

        if (b.state != KetherState::Transform)
        {
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
        }
        if (!b.phase2 && b.hp <= (b.maxHP / 2))
        {
            b.phase2 = true;
            b.state = KetherState::Transform;
            b.stateTimer = 0.0f;
            ResetAnim(b.transform);
            b.transformBlastDone = false;
            b.transformPostHoldTimer = 0.0f;
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

                    b.bookTargetX = player.posX;
                    b.bookTargetY = player.posY - PLAYER_HEIGHT * 0.35f;
                    b.bookStartX = b.posX + b.width * KETHER_TUNING.bookOriginXRatio;
                    b.bookStartY = b.posY - b.height * KETHER_TUNING.bookOriginYRatio;          // 胴体中央より少し下
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
            // 投擲までの遅延を短縮
            if (b.stateTimer >= 0.80f)
            {
                UpdateAnim(b.bookFx, true);
            }
            if (b.stateTimer > 1.70f)
            {
                b.state = b.phase2 ? KetherState::Idle2 : KetherState::Idle1;
                b.stateTimer = 0.0f;
            }
            break;
        case KetherState::Fire1:
            // 大きく吸い込んでから1回長く吐く
            if (b.stateTimer < 0.90f)
            {
                UpdateAnim(b.fire1, false);
            }
            else if (!b.fire1.frames.empty())
            {
                int holdFrame = 8;
                const int maxIdx = static_cast<int>(b.fire1.frames.size()) - 1;
                if (holdFrame > maxIdx) holdFrame = maxIdx;
                if (holdFrame < 0) holdFrame = 0;
                b.fire1.frame = holdFrame;
            }
            if (b.stateTimer > 3.4f)
            {
                b.showFireEffect = false;
                b.state = b.phase2 ? KetherState::Idle2 : KetherState::Idle1;
                b.stateTimer = 0.0f;
            }
            break;
        case KetherState::Transform:
            UpdateAnim(b.transform, false);
            {
                const int lastFrame = static_cast<int>(b.transform.frames.size()) - 1;
                const bool nearEnd = (lastFrame >= 0 && b.transform.frame >= (lastFrame - 1));

                if (!b.transformBlastDone && nearEnd)
                {
                    b.phaseShockwaveActive = true;
                    b.phaseShockwaveTimer = 0.0f;

                    const float pushDir = (player.posX >= b.posX) ? 1.0f : -1.0f;
                    player.state = PlayerState::Hurt;
                    player.velocityX = pushDir * 38.0f;
                    player.velocityY = -10.0f;
                    b.transformBlastDone = true;
                }
                if (IsAnimFinished(b.transform))
                {
                    if (!b.transformBlastDone)
                    {
                        b.phaseShockwaveActive = true;
                        b.phaseShockwaveTimer = 0.0f;

                        const float pushDir = (player.posX >= b.posX) ? 1.0f : -1.0f;
                        player.state = PlayerState::Hurt;
                        player.velocityX = pushDir * 38.0f;
                        player.velocityY = -10.0f;
                        b.transformBlastDone = true;
                    }
                    b.transformPostHoldTimer += dt;
                    if (b.transformPostHoldTimer >= KETHER_TUNING.transformEndHold)
                    {
                        b.state = KetherState::Idle2;
                        b.stateTimer = 0.0f;
                        ResetAnim(b.idle2);
                    }
                }
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
            if (b.stateTimer < 0.80f)
            {
                UpdateAnim(b.fire2, false);
            }
            else if (!b.fire2.frames.empty())
            {
                int holdFrame = 9;
                const int maxIdx = static_cast<int>(b.fire2.frames.size()) - 1;
                if (holdFrame > maxIdx) holdFrame = maxIdx;
                if (holdFrame < 0) holdFrame = 0;
                b.fire2.frame = holdFrame;
            }
            if (b.stateTimer > 4.0f)
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
            const float left = b.posX - (b.width * KETHER_TUNING.bodyHalfWidthRatio);
            const float top = b.posY - (b.height * KETHER_TUNING.bodyTopOffsetRatio);
            const float w = b.width * KETHER_TUNING.bodyWidthRatio;
            const float h = b.height * KETHER_TUNING.bodyHeightRatio;
            UpdateCollider(b.colliderId, left, top, w, h);
        }
        const bool bookState = (b.state == KetherState::ThrowBook);
        const bool fireState = (b.state == KetherState::Fire1 || b.state == KetherState::Fire2);
        const bool fistState = (b.state == KetherState::Fist2);
        if (fireState || fistState || b.state == KetherState::ThrowBook)
        {
            float atkW = 0.0f;
            float atkH = 0.0f;
            float atkX = b.posX;
            float atkY = b.posY;
            bool enableAttack = true;
            if (fireState)
            {
                atkW = (b.state == KetherState::Fire2) ? 180.0f : 150.0f;
                atkX = b.posX + b.width * KETHER_TUNING.flameOriginXRatio;

                const float breathStart = (b.state == KetherState::Fire2) ? 0.80f : 0.90f;
                const float breathEnd = (b.state == KetherState::Fire2) ? 3.95f : 3.35f;
                enableAttack = (b.stateTimer >= breathStart && b.stateTimer <= breathEnd);

                const float flameTop = b.posY - b.height * KETHER_TUNING.flameOriginYRatio;
                const float groundY = GetGroundYBelowX(atkX, flameTop, player.posY);

                float fullH = (groundY > flameTop)
                    ? ((groundY - flameTop) + 24.0f)
                    : ((b.state == KetherState::Fire2) ? 260.0f : 220.0f);

                atkH = fullH;
                atkY = flameTop + atkH * 0.5f;
            }
            else
            {
                if (b.state == KetherState::ThrowBook)
                {
                    const float throwDelay = 0.80f;
                    const float flightDuration = 0.65f;
                    const float t = (b.stateTimer - throwDelay) / flightDuration;
                    const float clampedT = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);

                    atkW = 140.0f;
                    atkH = 120.0f;
                    atkX = b.bookStartX + (b.bookTargetX - b.bookStartX) * clampedT;
                    atkY = b.bookStartY + (b.bookTargetY - b.bookStartY) * clampedT;
                    enableAttack = (b.stateTimer >= throwDelay && b.stateTimer <= (throwDelay + flightDuration));
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
            }

            const float left = atkX - atkW * 0.5f;
            const float top = atkY - atkH * 0.5f;

            b.attackOwner.attackPower = (b.state == KetherState::ThrowBook) ? 20 : (fireState ? (b.state == KetherState::Fire2 ? 22 : 14) : 18);
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

    void DrawAnimFitFacing(const CameraData& camera, float x, float y, float w, float h, int handle, bool facingRight)
    {
        if (handle == -1) return;
        const int cx = static_cast<int>((x - camera.posX) * camera.scale);
        const int cy = static_cast<int>((y - camera.posY) * camera.scale);
        const int halfW = static_cast<int>(w * 0.5f * camera.scale);
        const int hh = static_cast<int>(h * camera.scale);
        if (facingRight)
        {
            DrawExtendGraph(cx - halfW, cy - hh, cx + halfW, cy, handle, TRUE);
        }
        else
        {
            DrawExtendGraph(cx + halfW, cy - hh, cx - halfW, cy, handle, TRUE);
        }
    }

    void DrawColliderRectDebug(const CameraData& camera, int colliderId, int color)
    {
        if (colliderId == -1) return;

        float left = 0.0f;
        float top = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        if (!GetColliderRect(colliderId, left, top, width, height)) return;

        const int drawLeft = static_cast<int>((left - camera.posX) * camera.scale);
        const int drawTop = static_cast<int>((top - camera.posY) * camera.scale);
        const int drawRight = static_cast<int>((left + width - camera.posX) * camera.scale);
        const int drawBottom = static_cast<int>((top + height - camera.posY) * camera.scale);
        DrawBox(drawLeft, drawTop, drawRight, drawBottom, color, FALSE);
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
        if (!b.active) continue;

        if (b.state == KetherState::Died || b.state == KetherState::TipherethDied) continue;

        return true;
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
        b.maxHP = GetKetherMaxHP();
        b.hp = b.maxHP;
        b.attackOwner.type = EnemyType::BigQuartist;
        b.attackOwner.attackPower = GetKetherAttackPower();
        const float left = b.posX - (b.width * KETHER_TUNING.bodyHalfWidthRatio);
        const float top = b.posY - (b.height * KETHER_TUNING.bodyTopOffsetRatio);
        const float w = b.width * KETHER_TUNING.bodyWidthRatio;
        const float h = b.height * KETHER_TUNING.bodyHeightRatio;
        b.colliderId = CreateCollider(ColliderTag::Enemy, left, top, w, h, nullptr);
    }
    else if (type == BigBossType::Tiphereth)
    {
        LoadTipherethAssets(b);
        b.width = 595.0f;
        b.height = 403.0f;
        b.maxHP = GetTipherethMaxHP();
        b.hp = b.maxHP;
        b.state = KetherState::TipherethIdle;
        b.attackOwner.type = EnemyType::BigQuartist;
        b.attackOwner.attackPower = GetTipherethAttackPower();
        b.tipherethGroundY = y;

        const float bodyW = b.width * TIPHERETH_TUNING.bodyWidthRatio;
        const float bodyH = b.height * TIPHERETH_TUNING.bodyHeightRatio;
        const float bodyCenterX = b.posX + (b.isFacingRight ? 1.0f : -1.0f) * b.width * TIPHERETH_TUNING.bodyFacingOffsetRatio;
        const float left = bodyCenterX - bodyW * 0.5f;
        const float top = b.posY - (b.height * TIPHERETH_TUNING.bodyTopOffsetRatio);
        b.colliderId = CreateCollider(ColliderTag::Enemy, left, top, bodyW, bodyH, nullptr);
    }

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
        else if (b.type == BigBossType::Tiphereth)
        {
            UpdateTiphereth(b);
        }
    }
}
void DrawBigBosses()
{
    const CameraData camera = GetCamera();

    const char* stageName = GetCurrentStageName();
    bool isForest5 = (stageName && strcmp(stageName, "forest_5") == 0);
    bool isForest14 = (stageName && strcmp(stageName, "forest_14") == 0);

    for (const auto& b : g_bigBosses)
    {
        if (!b.active) continue;
        if (!isForest5 && !isForest14 && (b.state == KetherState::Died || b.state == KetherState::TipherethDied)) continue;

        if (b.type == BigBossType::Tiphereth)
        {
            int body = -1;
            switch (b.state)
            {
            case KetherState::TipherethPrepare:
            case KetherState::TipherethIdle:
                body = GetAnimHandle(b.tipherethIdle);
                break;
            case KetherState::TipherethTentacle:
                body = GetAnimHandle(b.tipherethTentacle);
                break;
            case KetherState::TipherethBite:
                body = GetAnimHandle(b.tipherethBite);
                break;
            case KetherState::TipherethStomp:
                body = GetAnimHandle(b.tipherethStomp);
                break;
            case KetherState::TipherethCharge:
                body = GetAnimHandle(b.tipherethCharge);
                break;
            case KetherState::TipherethDied:
                body = GetAnimHandle(b.tipherethDied);
                break;
            default:
                body = GetAnimHandle(b.tipherethIdle);
                break;
            }

            int srcW = static_cast<int>(b.width);
            int srcH = static_cast<int>(b.height);
            if (body != -1)
            {
                GetGraphSize(body, &srcW, &srcH);
            }

            const float drawX = b.posX + TIPHERETH_TUNING.drawOffsetX;
            const float drawY = b.posY + TIPHERETH_TUNING.drawOffsetY;
            const bool facingRight = TIPHERETH_TUNING.reverseDrawFacing ? !b.isFacingRight : b.isFacingRight;
            if (b.state == KetherState::TipherethPrepare)
            {
                SetDrawBright(255, 96, 96);
            }
            DrawAnimFitFacing(camera, drawX, drawY, static_cast<float>(srcW), static_cast<float>(srcH), body, facingRight);
            SetDrawBright(255, 255, 255);

            if (TIPHERETH_TUNING.debugDraw)
            {
                DrawColliderRectDebug(camera, b.colliderId, GetColor(0, 255, 0));
                DrawColliderRectDebug(camera, b.attackColliderId, GetColor(255, 64, 64));
            }
            continue;
        }

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
            fx = !b.fireFx1.frames.empty() ? b.fireFx1.frames[0] : -1;
            break;
        case KetherState::Transform:
            body = GetAnimHandle(b.transform);
            break;
        case KetherState::Idle2:
            body = GetAnimHandle(b.idle2);
            break;
        case KetherState::Fire2:
            body = GetAnimHandle(b.fire2);
            fx = !b.fireFx2.frames.empty() ? b.fireFx2.frames[0] : -1;
            break;
        case KetherState::Fist2:
            body = GetAnimHandle(b.fist2);
            break;
        case KetherState::Died:
            body = GetAnimHandle(b.died);
            break;
        }
        float bodyDrawY = b.posY;
        if (b.state == KetherState::Died)
        {
            bodyDrawY += KETHER_TUNING.diedDrawOffsetY;
        }
        DrawAnimFit(camera, b.posX, bodyDrawY, b.width, b.height, body);
        if (b.phaseShockwaveActive)
        {
            float t = b.phaseShockwaveTimer / KETHER_TUNING.phaseShockwaveDuration;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            const int cx = static_cast<int>((b.posX - camera.posX) * camera.scale);
            const int cy = static_cast<int>((b.posY - camera.posY) * camera.scale);
            const int outer = static_cast<int>((28.0f + 180.0f * t) * camera.scale);
            const int alpha = static_cast<int>(220.0f * (1.0f - t));

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawCircle(cx, cy, outer, GetColor(255, 210, 120), FALSE);
            DrawCircle(cx, cy, outer - 2, GetColor(255, 180, 90), FALSE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
        }
        if (KETHER_TUNING.debugDraw)
        {
            // デバッグ用の円や文字列も非表示に
            // const float flameOX = b.posX + b.width * KETHER_FLAME_ORIGIN_X_RATIO;
            // const float flameOY = b.posY - b.height * KETHER_FLAME_ORIGIN_Y_RATIO;
            // const float bookOX = b.posX + b.width * KETHER_BOOK_ORIGIN_X_RATIO;
            // const float bookOY = b.posY - b.height * KETHER_BOOK_ORIGIN_Y_RATIO;
            // const int flameX = static_cast<int>((flameOX - camera.posX) * camera.scale);
            // const int flameY = static_cast<int>((flameOY - camera.posY) * camera.scale);
            // const int bookX = static_cast<int>((bookOX - camera.posX) * camera.scale);
            // const int bookY = static_cast<int>((bookOY - camera.posY) * camera.scale);
            // DrawCircle(flameX, flameY, 8, GetColor(255, 120, 0), FALSE);
            // DrawCircle(bookX, bookY, 8, GetColor(80, 220, 255), FALSE);
            // DrawFormatString(flameX + 10, flameY - 10, GetColor(255, 120, 0), "FLAME_ORIGIN");
            // DrawFormatString(bookX + 10, bookY - 10, GetColor(80, 220, 255), "BOOK_ORIGIN");
            // DrawFormatString(cLeft, cTop - 20, GetColor(255, 255, 0), "state=%d t=%.2f", static_cast<int>(b.state), b.stateTimer);
        }
        if (fx != -1)
        {
            const float dir = (GetPlayerData().posX >= b.posX) ? 1.0f : -1.0f;
            float fxX = b.posX;
            float fxY = b.posY - b.height * 0.70f;
            float fxW = 52.0f;
            float fxH = 36.0f;

            if (b.state == KetherState::ThrowBook)
            {
                fxW = 130.0f;
                fxH = 100.0f;

                const float throwDelay = 0.80f;
                const float flightDuration = 0.65f;
                const float t = (b.stateTimer - throwDelay) / flightDuration;
                const float clampedT = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t);

                fxX = b.bookStartX + (b.bookTargetX - b.bookStartX) * clampedT;
                fxY = b.bookStartY + (b.bookTargetY - b.bookStartY) * clampedT;
            }
            else if (b.state == KetherState::Fire1 || b.state == KetherState::Fire2)
            {
                fxW = (b.state == KetherState::Fire2) ? 112.0f : 94.0f;
                const float breathStart = (b.state == KetherState::Fire2) ? 0.80f : 0.90f;
                const float breathEnd = (b.state == KetherState::Fire2) ? 3.95f : 3.35f;
                const float breathProgress = (b.stateTimer - breathStart) / (breathEnd - breathStart);
                const bool isEnding = (breathProgress >= 0.88f);

                const float flameTop = (b.posY - b.height * KETHER_TUNING.flameOriginYRatio) + KETHER_TUNING.flameDrawOffsetY;
                fxX = b.posX + b.width * KETHER_TUNING.flameOriginXRatio;
                const float groundY = GetGroundYBelowX(fxX, flameTop, GetPlayerData().posY);

                const float fullH = (groundY > flameTop)
                    ? ((groundY - flameTop) + 24.0f)
                    : ((b.state == KetherState::Fire2) ? 260.0f : 220.0f);

                fxH = fullH;
                fxY = flameTop + fxH;

                const std::vector<int>& flameFrames = (b.state == KetherState::Fire2) ? b.fireFx2.frames : b.fireFx1.frames;
                if (!flameFrames.empty())
                {
                    const int frameCount = static_cast<int>(flameFrames.size());
                    const int loopCount = (frameCount >= 5) ? 5 : frameCount;
                    int frameIdx = 0;
                    if (!isEnding)
                    {
                        frameIdx = static_cast<int>((b.stateTimer - breathStart) * 20.0f) % loopCount;
                        if (frameIdx < 0) frameIdx = 0;
                    }
                    else
                    {
                        const int endStartIdx = (frameCount >= 6) ? 5 : (frameCount - 1);
                        const int endFrameCount = frameCount - endStartIdx;
                        if (endFrameCount > 1)
                        {
                            float endT = (breathProgress - 0.88f) / 0.12f;
                            if (endT < 0.0f) endT = 0.0f;
                            if (endT > 1.0f) endT = 1.0f;
                            frameIdx = endStartIdx + static_cast<int>(endT * static_cast<float>(endFrameCount - 1));
                        }
                        else
                        {
                            frameIdx = endStartIdx;
                        }
                    }
                    fx = flameFrames[frameIdx];
                }
            }
            if (b.state == KetherState::ThrowBook && b.stateTimer < 0.80f)
            {
                fx = -1;
            }
            if ((b.state == KetherState::Fire1 && b.stateTimer < 0.90f) || (b.state == KetherState::Fire2 && b.stateTimer < 0.80f))
            {
                fx = -1;
            }

            DrawAnimFit(camera, fxX, fxY, fxW, fxH, fx);
        }

        if (KETHER_TUNING.debugDraw)
        {
            DrawColliderRectDebug(camera, b.colliderId, GetColor(0, 255, 0));
            DrawColliderRectDebug(camera, b.attackColliderId, GetColor(255, 64, 64));
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
        else if (b.type == BigBossType::Tiphereth)
        {
            UnloadTipherethAssets(b);
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

        if (b.state == KetherState::Died || b.state == KetherState::TipherethDied) return true;
        if (b.type == BigBossType::Kether && b.state == KetherState::Transform) return true;

        b.hp -= damage;
        if (b.hp < 0) b.hp = 0;

        if (b.hp <= 0)
        {
            b.stateTimer = 0.0f;
            b.velocityX = 0.0f;
            b.velocityY = 0.0f;
            ClearBigBossAttackCollider(b);
            if (b.colliderId != -1)
            {
                DestroyCollider(b.colliderId);
                b.colliderId = -1;
            }

            if (b.type == BigBossType::Kether)
            {
                b.state = KetherState::Died;
                ResetAnim(b.died);
                if (!b.rewardGranted)
                {
                    UnlockDoubleJump();
                    b.rewardGranted = true;
                }
            }
            else if (b.type == BigBossType::Tiphereth)
            {
                b.state = KetherState::TipherethDied;
                ResetAnim(b.tipherethDied);
            }
        }
        return true;
    }

    return false;
}
