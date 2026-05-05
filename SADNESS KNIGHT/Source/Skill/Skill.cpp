#include "Skill.h"
#include "SkillManager.h"
#include "../Enemy/EnemyBase.h"
#include <cmath>

// スキルの攻撃の判定
static void CreateAttackCollider(
    ColliderId& collider,
    const ComboStep& step,
    PlayerData* player,
    Skill* owner)
{
    float left =
        player->posX
        + (player->isFacingRight ? step.offsetX : -step.offsetX - step.hitWidth);

    float top =
        player->posY - PLAYER_HEIGHT + step.offsetY;

    collider = CreateCollider(
        ColliderTag::Other,
        left,
        top,
        step.hitWidth,
        step.hitHeight,
        owner);
}
// 追従型の攻撃判定
Skill::Skill(const SkillData& data)
    : m_data(data),
    m_currentCoolTime(0),
    m_isActive(false),
    m_activeTimer(0)
{
    if (m_data.type == SkillType::Follow)
    {
        m_followOffsetX = 46.0f;
        m_followAttackInterval = 90;
        m_followAttackCharge = 10;
        m_followAttackWidth = 20.0f;
        m_followAttackHeight = 20.0f;
        m_followLerpSpeed = 0.18f;
        m_followDetectRange = 560.0f;
        m_followSpriteHandle = LoadGraph("Data/MidBoss/StoneGolem/Character_sheet.png");
        m_followBulletSpriteHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile.png");
        if (m_followBulletSpriteHandle == -1)
        {
            m_followBulletSpriteHandle = LoadGraph("Data/MidBoss/StoneGolem/arm_projectile_glowing.png");
        }
    }
}
// スキルが使用可能か
bool Skill::CanUse() const
{
    if (m_currentCoolTime > 0) return false;
    return true;
}
// クールタイム開始
void Skill::StartCoolTime()
{
    m_currentCoolTime = m_data.coolTime;
}
// スキルの強制終了（CTは消費する）
void Skill::Activate(PlayerData* player)
{
    if (m_data.type == SkillType::Attack)
    {
        if (!m_data.comboSteps.empty())
        {
            if (m_isActive)
            {
                if (!m_comboQueued && m_comboIndex + 1 < (int)m_data.comboSteps.size())
                {
                    m_comboQueued = true;
                }
                return;
            }

            if (m_currentCoolTime > 0)
                return;

            if (m_comboTimer > 0 && m_comboIndex + 1 < (int)m_data.comboSteps.size())
            {
                m_comboIndex++;
            }
            else
            {
                m_comboIndex = 0;
            }

            const ComboStep& step = m_data.comboSteps[m_comboIndex];
            m_activeTimer = step.duration;
            m_frame = 0;
            m_hitActive = false;
            m_comboQueued = false;
            ClearHitTargets();

            if (m_attackCollider != -1)
            {
                DestroyCollider(m_attackCollider);
                m_attackCollider = -1;
            }
        }
        else
        {
            if (!CanUse()) return;
            m_activeTimer = m_data.duration;
            m_frame = 0;
            ClearHitTargets();
            StartCoolTime();
        }

        m_comboTimer = 90;
        m_isActive = true;
        return;
    }

    if (m_data.type == SkillType::Follow)
    {
        // もう一度押したら解除（手動トグル）
        if (m_isActive)
        {
            ForceEnd();
            return;
        }

        if (!CanUse()) return;

        m_isActive = true;
        m_activeTimer = 0; // 手動解除まで維持

        StartCoolTime();

        m_followAttackTimer = 0;
        m_followAttackDelay = 0;
        m_followIsShooting = false;
        m_followAnimFrame = 10;
        m_followAnimCounter = 0;
        m_followFacingRight = player->isFacingRight;

        // プレイヤー後ろに追従
        float offset = player->isFacingRight ? -m_followOffsetX : m_followOffsetX;

        m_followPosX = player->posX + offset;
        m_followPosY = player->posY - PLAYER_HEIGHT * 0.55f;

        if (m_followCollider == -1)
        {
            m_followCollider = CreateCollider(
                ColliderTag::Other,
                m_followPosX - 30.0f,
                m_followPosY - 30.0f,
                60,
                60,
                this
            );
        }

        return;
    }


    if (!CanUse()) return;
    // ...existing code...
    StartCoolTime();
}

void Skill::Update(PlayerData* player)
{
    if (m_currentCoolTime > 0)
        m_currentCoolTime--;

    if (m_isActive && m_data.type == SkillType::Attack)
    {
        if (m_activeTimer > 0)
        {
            m_activeTimer--;
            m_frame++;
        }

        if (m_activeTimer <= 0)
        {
            if (m_attackCollider != -1)
            {
                DestroyCollider(m_attackCollider);
                m_attackCollider = -1;
            }

            if (m_comboQueued && m_comboIndex + 1 < (int)m_data.comboSteps.size())
            {
                m_comboIndex++;
                const ComboStep& nextStep = m_data.comboSteps[m_comboIndex];
                m_activeTimer = nextStep.duration;
                m_frame = 0;
                m_hitActive = false;
                m_comboQueued = false;
                ClearHitTargets();
                m_comboTimer = 90;
            }
            else
            {
                m_isActive = false;
                m_comboQueued = false;

                if (m_comboIndex + 1 >= (int)m_data.comboSteps.size())
                {
                    StartCoolTime();
                    m_comboIndex = 0;
                    m_comboTimer = 0;
                }
                else
                {
                    m_comboTimer = 90;
                }
            }
        }
    }

    // 非アクティブ時だけコンボ受付時間を減らす
    if (!m_isActive && m_comboTimer > 0)
        m_comboTimer--;

    // 猶予切れでコンボ終了
    if (!m_isActive && m_data.type == SkillType::Attack && m_comboTimer <= 0)
    {
        if (m_comboIndex != 0)
        {
            StartCoolTime();
        }
        m_comboIndex = 0;
        m_comboQueued = false;
    }

    // 攻?コライダー更新
    if (!m_data.comboSteps.empty() && m_isActive)
    {
        const ComboStep& step = m_data.comboSteps[m_comboIndex];

        if (!m_hitActive && m_frame >= step.hitStartFrame && m_frame <= step.hitEndFrame)
        {
            m_hitActive = true;

            if (m_attackCollider == -1)
                CreateAttackCollider(m_attackCollider, step, player, this);
        }

        if (m_hitActive && m_frame > step.hitEndFrame)
        {
            m_hitActive = false;

            if (m_attackCollider != -1)
            {
                DestroyCollider(m_attackCollider);
                m_attackCollider = -1;
            }
        }

        if (m_attackCollider != -1)
        {
            float left =
                player->posX
                + (player->isFacingRight ? step.offsetX : -step.offsetX - step.hitWidth);

            float top = player->posY - PLAYER_HEIGHT + step.offsetY;

            UpdateCollider(
                m_attackCollider,
                left,
                top,
                step.hitWidth,
                step.hitHeight);
        }
    }

    // 追従型の処理
    if (m_data.type == SkillType::Follow && m_isActive)
    {
        m_followFacingRight = player->isFacingRight;

        // プレイヤーに追従（後ろ固定）
        float offset = player->isFacingRight ? -m_followOffsetX : m_followOffsetX;
        float targetX = player->posX + offset;
        float targetY = player->posY - PLAYER_HEIGHT * 0.55f;

        m_followPosX += (targetX - m_followPosX) * m_followLerpSpeed;
        m_followPosY += (targetY - m_followPosY) * m_followLerpSpeed;

        if (m_followCollider != -1)
        {
            UpdateCollider(m_followCollider, m_followPosX - 30.0f, m_followPosY - 30.0f, 60, 60);
        }

        // 射程内の最も近い敵を探す（通常敵/ミッドボス/ビッグボス共通）
        auto findNearestTargetCollider = [&](float fromX, float fromY, float maxRange) -> int
        {
            return FindNearestEnemyCollider(fromX, fromY, maxRange);
        };

        if (m_followAttackTimer > 0)
        {
            m_followAttackTimer--;
        }
        else
        {
            if (m_followAttackDelay <= 0)
            {
                m_followAttackDelay = m_followAttackCharge;
            }

            m_followAttackDelay--;

            if (m_followAttackDelay <= 0)
            {
                const int targetColliderId = findNearestTargetCollider(m_followPosX, m_followPosY, m_followDetectRange);

                if (targetColliderId != -1)
                {
                    m_followIsShooting = true;
                    m_followAnimFrame = 20; // StoneGolem shot row start
                    m_followAnimCounter = 0;

                    // 3発同時発射（必中ホーミング）
                    for (int i = 0; i < 3; ++i)
                    {
                        if (g_SkillManager.GetRemainingUses(m_data.id) == 0)
                        {
                            ForceEnd();
                            break;
                        }

                        FollowBullet b{};

                        // 3wayで見えるように、発射位置を少しずらす
                        const float side = m_followFacingRight ? 1.0f : -1.0f;
                        const float spawnOffsetX[3] = { -10.0f * side, 0.0f, 10.0f * side };
                        const float spawnOffsetY[3] = { -12.0f, -8.0f, -4.0f };

                        b.x = m_followPosX + spawnOffsetX[i];
                        b.y = m_followPosY + spawnOffsetY[i];

                        const float baseSpeed = 8.5f;
                        const float spreadY[3] = { -2.2f, 0.0f, 2.2f };
                        b.vx = m_followFacingRight ? baseSpeed : -baseSpeed;
                        b.vy = spreadY[i];
                        b.life = 90;
                        b.targetColliderId = targetColliderId;

                        b.collider = CreateCollider(
                            ColliderTag::Other,
                            b.x - 10.0f,
                            b.y - 10.0f,
                            m_followAttackWidth,
                            m_followAttackHeight,
                            this);

                        m_followBullets.push_back(b);

                        if (m_onConsumeUse)
                        {
                            m_onConsumeUse(m_data.id); // 1発ごとに1消費
                        }
                    }

                    m_followAttackTimer = m_followAttackInterval;
                }
            }
        }

        // シュートモーション更新（StoneGolem shot 20?27）
        if (m_followIsShooting)
        {
            m_followAnimCounter++;
            if (m_followAnimCounter >= 4)
            {
                m_followAnimCounter = 0;
                m_followAnimFrame++;
                if (m_followAnimFrame > 27)
                {
                    m_followIsShooting = false;
                    m_followAnimFrame = 10; // idle row start
                }
            }
        }
        else
        {
            m_followAnimCounter++;
            if (m_followAnimCounter >= 8)
            {
                m_followAnimCounter = 0;
                m_followAnimFrame++;
                if (m_followAnimFrame < 10 || m_followAnimFrame > 17)
                {
                    m_followAnimFrame = 10;
                }
            }
        }

        // 弾更新（ホーミング）
        for (int i = (int)m_followBullets.size() - 1; i >= 0; --i)
        {
            auto& b = m_followBullets[i];

            float targetLeft = 0, targetTop = 0, targetW = 0, targetH = 0;
            const bool hasTarget = (b.targetColliderId != -1) && GetColliderRect(b.targetColliderId, targetLeft, targetTop, targetW, targetH);
            if (hasTarget)
            {
                const float targetX = targetLeft + targetW * 0.5f;
                const float targetY = targetTop + targetH * 0.5f;
                float dx = targetX - b.x;
                float dy = targetY - b.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f)
                {
                    dx /= len;
                    dy /= len;

                    const float speed = 9.0f;
                    float curLen = std::sqrt(b.vx * b.vx + b.vy * b.vy);
                    if (curLen < 0.001f) curLen = 1.0f;
                    float curX = b.vx / curLen;
                    float curY = b.vy / curLen;

                    // 急に同じ軌道へ重ならないよう、滑らかに追従
                    const float turnRate = 0.18f;
                    float mixX = curX * (1.0f - turnRate) + dx * turnRate;
                    float mixY = curY * (1.0f - turnRate) + dy * turnRate;
                    float mixLen = std::sqrt(mixX * mixX + mixY * mixY);
                    if (mixLen > 0.001f)
                    {
                        mixX /= mixLen;
                        mixY /= mixLen;
                        b.vx = mixX * speed;
                        b.vy = mixY * speed;
                    }
                }
            }

            b.x += b.vx;
            b.y += b.vy;
            b.life--;

            UpdateCollider(b.collider, b.x - 10.0f, b.y - 10.0f, m_followAttackWidth, m_followAttackHeight);

            if (b.life <= 0)
            {
                DestroyCollider(b.collider);
                m_followBullets.erase(m_followBullets.begin() + i);
            }
        }
    }
	// 召喚型の処理
    if (m_data.type == SkillType::Summon)
    {
        for (int i = (int)m_summons.size() - 1; i >= 0; --i)
        {
            auto& s = m_summons[i];

            // ===== 寿命 =====
            s.timer--;

            if (s.timer <= 0)
            {
                if (s.collider != -1)
                    DestroyCollider(s.collider);

                if (s.attackCollider != -1)
                    DestroyCollider(s.attackCollider);

                m_summons.erase(m_summons.begin() + i);
                continue;
            }

            // ===== 本体位置更新 =====
            UpdateCollider(
                s.collider,
                s.x - 40,
                s.y - 80,
                80,
                80);

            // ===== 攻撃タイマー =====
            if (s.attackTimer > 0)
                s.attackTimer--;

            // ===== 攻撃開始 =====
            if (s.attackTimer <= 0)
            {
                // ヒット履歴リセット
                ClearHitTargets();

                if (s.attackCollider != -1)
                    DestroyCollider(s.attackCollider);

                s.attackCollider = CreateCollider(
                    ColliderTag::Other,
                    s.x - m_summonAttackWidth * 0.5f,
                    s.y - m_summonAttackHeight,
                    m_summonAttackWidth,
                    m_summonAttackHeight,
                    this);

                s.attackTimer = s.attackInterval;
                // 個体の攻撃寿命
                s.attackLife = m_summonAttackDuration;

                // 弾薬消費（Summonは攻撃した時）
                if (m_onConsumeUse)
                    m_onConsumeUse(m_data.id);
            }

            // ===== 攻撃コライダー寿命 =====
            if (s.attackCollider != -1)
            {
                s.attackLife--;

                if (s.attackLife <= 0)
                {
                    DestroyCollider(s.attackCollider);
                    s.attackCollider = -1;
                }
            }
        }
		// 召喚ユニットがいなくなったらスキル非アクティブ
        if (m_summons.empty())
            m_isActive = false;
    }
}

// スキルの攻撃倍率の取得
float Skill::GetCurrentAttackRate() const
{
    if (m_data.type == SkillType::Attack)
    {
        if (!m_data.comboSteps.empty())
            return m_data.comboSteps[m_comboIndex].attackRate;

        return m_data.attackRate;
    }

    // これ追加
    return m_data.attackRate;
}

// 敵に攻撃がヒットしたことを記録（多段ヒット対策）
bool Skill::RegisterHit(void* target)
{
    if (m_hitTargets.find(target) != m_hitTargets.end())
        return false;

    m_hitTargets.insert(target);
    return true;
}

// ヒット履歴クリア
void Skill::ClearHitTargets()
{
    m_hitTargets.clear();
}

void Skill::ForceEnd()
{
    m_isActive = false;

    if (m_followCollider != -1)
    {
        DestroyCollider(m_followCollider);
        m_followCollider = -1;
    }

    if (m_followAttackCollider != -1)
    {
        DestroyCollider(m_followAttackCollider);
        m_followAttackCollider = -1;
    }

    for (auto& b : m_followBullets)
    {
        if (b.collider != -1)
            DestroyCollider(b.collider);
    }
    m_followBullets.clear();

    if (m_summonCollider != -1)
    {
        DestroyCollider(m_summonCollider);
        m_summonCollider = -1;
    }

    for (auto& s : m_summons)
    {
        if (s.collider != -1)
            DestroyCollider(s.collider);
        if (s.attackCollider != -1)
            DestroyCollider(s.attackCollider);
    }
    m_summons.clear();
}

void Skill::Draw(const CameraData& camera) const
{
    if (!m_isActive || m_data.type != SkillType::Follow)
        return;

    const int fx = static_cast<int>((m_followPosX - camera.posX) * camera.scale);
    const int fy = static_cast<int>((m_followPosY - camera.posY) * camera.scale);

    if (m_followSpriteHandle != -1)
    {
        // StoneGolem Character_sheet 100x100
        int frame = m_followAnimFrame;
        if (frame < 10 || frame > 27) frame = 10;
        const int srcX = (frame % 10) * 100;
        const int srcY = (frame / 10) * 100;
        const int srcW = 100;
        const int srcH = 100;

        // ▼本体（追従ゴーレム）サイズ調整
        // 値を大きくすると本体が大きく表示されます
        const int drawW = static_cast<int>(112.0f * camera.scale);
        const int drawH = static_cast<int>(112.0f * camera.scale);

        if (m_followFacingRight)
        {
            DrawRectExtendGraph(
                fx - drawW / 2,
                fy - drawH,
                fx + drawW / 2,
                fy,
                srcX,
                srcY,
                srcW,
                srcH,
                m_followSpriteHandle,
                TRUE);
        }
        else
        {
            DrawRectExtendGraph(
                fx + drawW / 2,
                fy - drawH,
                fx - drawW / 2,
                fy,
                srcX,
                srcY,
                srcW,
                srcH,
                m_followSpriteHandle,
                TRUE);
        }
    }
    else
    {
        DrawBox(fx - 32, fy - 64, fx + 32, fy, GetColor(120, 120, 120), TRUE);
    }

    for (const auto& b : m_followBullets)
    {
        const int bx = static_cast<int>((b.x - camera.posX) * camera.scale);
        const int by = static_cast<int>((b.y - camera.posY) * camera.scale);

        if (m_followBulletSpriteHandle != -1)
        {
            // ▼ボス弾と同サイズ（StoneGolem: 96x96）
            const int half = static_cast<int>(48.0f * camera.scale);
            const int left = bx - half;
            const int top = by - half;
            const int right = bx + half;
            const int bottom = by + half;

            if (b.vx >= 0.0f)
            {
                DrawExtendGraph(left, top, right, bottom, m_followBulletSpriteHandle, TRUE);
            }
            else
            {
                DrawExtendGraph(right, top, left, bottom, m_followBulletSpriteHandle, TRUE);
            }
        }
        else
        {
            const int fallbackBulletRadius = static_cast<int>(48.0f * camera.scale);
            DrawCircle(bx, by, fallbackBulletRadius, GetColor(120, 220, 255), TRUE);
        }
    }
}

Skill::FollowBullet* Skill::FindBulletByCollider(ColliderId id)
{
    for (auto& b : m_followBullets)
    {
        if (b.collider == id)
            return &b;
    }
    return nullptr;
}

void Skill::ConsumeFollowBullet(ColliderId id)
{
    for (int i = static_cast<int>(m_followBullets.size()) - 1; i >= 0; --i)
    {
        if (m_followBullets[i].collider == id)
        {
            if (m_followBullets[i].collider != -1)
            {
                DestroyCollider(m_followBullets[i].collider);
            }
            m_followBullets.erase(m_followBullets.begin() + i);
            return;
        }
    }
}