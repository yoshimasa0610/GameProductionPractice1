#include "Collision.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include "../Player/Player.h"
#include "../Enemy/EnemyBase.h"
#include "../Skill/Skill.h"
#include "DxLib.h"
#include "../Map/MapParameter.h"
#include "../Map/MapChip.h"
#include "../Map/StageManager.h"
#include "../Map/MapManager.h"
#include "../BigBoss/BigBossBase.h"

//
// シンプルなコライダー実装
//

// 内部コライダー表現（左上座標ベース）
struct Collider
{
    ColliderId id;
    ColliderTag tag;
    float left;
    float top;
    float width;
    float height;
    void* owner;
    bool active;
};

namespace
{
    std::vector<Collider> g_colliders;
    ColliderId g_nextId = 1;
    int g_playerEnemyHitCooldown = 0;
}

// MapManager 互換：シンプルな呼び出し版（GetPlayerData を使って内部実装に委譲）
bool PlayerHitNormalBlockX(float newPosX)
{
    return PlayerHitNormalBlockX(&GetPlayerData(), newPosX);
}

bool PlayerHitNormalBlockY(float newPosY)
{
    return PlayerHitNormalBlockY(&GetPlayerData(), newPosY);
}

// MapChipData 版：MapManager がタイル情報を渡す場合のラッパー
bool PlayerHitNormalBlockX(const MapChipData& mc)
{
    PlayerData& player = GetPlayerData();
    float attemptedX = player.posX + player.velocityX;
    // ブロック矩形（ワールド左上）
    float bx = mc.xIndex * MAP_CHIP_WIDTH;
    float by = mc.yIndex * MAP_CHIP_HEIGHT;
    float bw = MAP_CHIP_WIDTH;
    float bh = MAP_CHIP_HEIGHT;

    const float w = static_cast<float>(PLAYER_WIDTH);
    const float h = static_cast<float>(PLAYER_HEIGHT);

    float pLeft = attemptedX - (w * 0.5f);
    float pTop = player.posY - h;

    if (AABBIntersect(pLeft, pTop, w, h, bx, by, bw, bh))
    {
        float dx = attemptedX - player.posX;
        if (dx > 0.0f)
        {
            player.posX = bx - (w * 0.5f);
        }
        else if (dx < 0.0f)
        {
            player.posX = (bx + bw) + (w * 0.5f);
        }
        return true;
    }
    return false;
}

bool PlayerHitNormalBlockY(const MapChipData& mc)
{
    PlayerData& player = GetPlayerData();
    float attemptedY = player.posY + player.velocityY;
    // ブロック矩形（ワールド左上）
    float bx = mc.xIndex * MAP_CHIP_WIDTH;
    float by = mc.yIndex * MAP_CHIP_HEIGHT;
    float bw = MAP_CHIP_WIDTH;
    float bh = MAP_CHIP_HEIGHT;

    const float w = static_cast<float>(PLAYER_WIDTH);
    const float h = static_cast<float>(PLAYER_HEIGHT);

    float pLeft = player.posX - (w * 0.5f);
    float pTop = attemptedY - h;

    if (AABBIntersect(pLeft, pTop, w, h, bx, by, bw, bh))
    {
        float dy = attemptedY - player.posY;
        if (dy > 0.0f)
        {
            // 着地
            player.posY = by;
            player.velocityY = 0.0f;
            player.isGrounded = true;
            player.jumpCount = 0;
        }
        else if (dy < 0.0f)
        {
            // 天井ヒット
            player.posY = (by + bh) + h;
            player.velocityY = 0.0f;
        }
        return true;
    }
    return false;
}

// 作成
ColliderId CreateCollider(ColliderTag tag, float left, float top, float width, float height, void* owner)
{
    Collider c;
    c.id = g_nextId++;
    c.tag = tag;
    c.left = left;
    c.top = top;
    c.width = width;
    c.height = height;
    c.owner = owner;
    c.active = true;
    g_colliders.push_back(c);
    return c.id;
}

// マップ管理用ラッパー：ブロックを作成（左上座標）
ColliderId CreateBlockCollider(float left, float top, float width, float height)
{
    return CreateCollider(ColliderTag::Block, left, top, width, height, nullptr);
}

// マップ管理用ラッパー：ブロック削除
void RemoveBlockCollider(ColliderId id)
{
    DestroyCollider(id);
}

// 更新
void UpdateCollider(ColliderId id, float left, float top, float width, float height)
{
    for (auto& c : g_colliders)
    {
        if (c.id == id && c.active)
        {
            c.left = left;
            c.top = top;
            c.width = width;
            c.height = height;
            return;
        }
    }
}

// 破棄（無効化）
void DestroyCollider(ColliderId id)
{
    auto it = std::find_if(g_colliders.begin(), g_colliders.end(), [id](const Collider& c) { return c.id == id; });
    if (it != g_colliders.end())
    {
        it->active = false;
    }
}

// 全削除
void ClearAllColliders()
{
    g_colliders.clear();
    g_nextId = 1;
}

// AABB 判定（左上基準）
bool AABBIntersect(float aLeft, float aTop, float aW, float aH,
                   float bLeft, float bTop, float bW, float bH)
{
    float aRight = aLeft + aW;
    float aBottom = aTop + aH;
    float bRight = bLeft + bW;
    float bBottom = bTop + bH;

    // 辺がちょうど接している場合は「接触あり」として扱う
    // （接地の1フレーム抜けによる Jump/Fall のちらつきを防ぐ）
    if (aRight < bLeft) return false;
    if (aLeft > bRight) return false;
    if (aBottom < bTop) return false;
    if (aTop > bBottom) return false;
    return true;
}

// ---------------------------------------
// プレイヤー向け直接判定 / 補正関数の実装
// 既存コードの PLAYER_WIDTH / PLAYER_HEIGHT 形式に合わせて動作します。
// ---------------------------------------

// X 方向の当たり判定と補正
bool PlayerHitNormalBlockX(PlayerData* player, float newPosX)
{
    if (player == nullptr) return false;

    const float w = static_cast<float>(PLAYER_WIDTH);
    const float h = static_cast<float>(PLAYER_HEIGHT);

    // 試行位置の矩形（左上基準）
    float pLeft = newPosX - (w * 0.5f);
    float pTop = player->posY - h;

    for (const auto& c : g_colliders)
    {
        if (!c.active) continue;
        if (c.tag != ColliderTag::Block) continue;

        if (AABBIntersect(pLeft, pTop, w, h, c.left, c.top, c.width, c.height))
        {
            // 衝突した -> X 方向のみで補正する
            float dx = newPosX - player->posX;
            if (dx > 0.0f)
            {
                // 右移動時：プレイヤーの右端をブロックの左端に合わせる
                player->posX = c.left - (w * 0.5f);
            }
            else if (dx < 0.0f)
            {
                // 左移動時：プレイヤーの左端をブロックの右端に合わせる
                player->posX = (c.left + c.width) + (w * 0.5f);
            }
            return true;
        }
    }

    return false;
}

// Y 方向の当たり判定と補正
bool PlayerHitNormalBlockY(PlayerData* player, float newPosY)
{
    if (player == nullptr) return false;

    const float w = static_cast<float>(PLAYER_WIDTH);
    const float h = static_cast<float>(PLAYER_HEIGHT);

    // 試行位置の矩形（左上基準）
    float pLeft = player->posX - (w * 0.5f);
    float pTop = newPosY - h;

    for (const auto& c : g_colliders)
    {
        if (!c.active) continue;
        if (c.tag != ColliderTag::Block) continue;

        if (AABBIntersect(pLeft, pTop, w, h, c.left, c.top, c.width, c.height))
        {
            // 衝突した -> Y 方向のみで補正する
            float dy = newPosY - player->posY;
            if (dy > 0.0f)
            {
                // 下方向に移動 -> 着地
                // プレイヤーの底面 (posY) をブロックの top に合わせる
                player->posY = c.top;
                player->velocityY = 0.0f;
                player->isGrounded = true;
                player->jumpCount = 0;
            }
            else if (dy < 0.0f)
            {
                // 上方向に移動 -> 天井に当たる
                // プレイヤーの上面 (posY - h) をブロックの bottom に合わせる
                player->posY = (c.top + c.height) + h;
                player->velocityY = 0.0f;
            }
            return true;
        }
    }

    return false;
}

bool SnapPlayerToGround(PlayerData* player, float maxDistance)
{
    if (player == nullptr) return false;

    const float w = static_cast<float>(PLAYER_WIDTH);
    float pLeft = player->posX - (w * 0.5f);
    float pRight = pLeft + w;

    bool found = false;
    float bestTop = 0.0f;

    for (const auto& c : g_colliders)
    {
        if (!c.active) continue;
        if (c.tag != ColliderTag::Block && c.tag != ColliderTag::SemiSolid) continue;

        float bLeft = c.left;
        float bRight = c.left + c.width;

        if (pRight <= bLeft || pLeft >= bRight) continue;

        // プレイヤーより下にある床だけ対象
        if (c.top < player->posY) continue;

        float dist = c.top - player->posY;
        if (dist > maxDistance) continue;

        // もっとも下側（Yが大きい）床を優先
        if (!found || c.top > bestTop)
        {
            found = true;
            bestTop = c.top;
        }
    }

    if (!found) return false;

    player->posY = bestTop;
    player->velocityY = 0.0f;
    player->isGrounded = true;
    player->jumpCount = 0;
    return true;
}

// 内部：プレイヤーとブロックの押し出し / 着地判定（player の owner は PlayerData* を想定）
static void ResolvePlayerBlock(PlayerData* player, const Collider& block, Collider& pc)
{
    if (player == nullptr) return;

    const float pW = pc.width;
    const float pH = pc.height;

    const float bLeft = block.left;
    const float bTop = block.top;
    const float bRight = block.left + block.width;
    const float bBottom = block.top + block.height;

    const float curLeft = pc.left;
    const float curTop = pc.top;
    const float curRight = curLeft + pW;
    const float curBottom = curTop + pH;

    const float prevLeft = player->prevPosX - pW * 0.5f;
    const float prevTop = player->prevPosY - pH;
    const float prevRight = prevLeft + pW;
    const float prevBottom = prevTop + pH;

    // そもそも重なっていなければ何もしない
    if (!AABBIntersect(curLeft, curTop, pW, pH, block.left, block.top, block.width, block.height))
    {
        return;
    }

    const bool overlapX = (curRight > bLeft && curLeft < bRight);
    const bool overlapY = (curBottom > bTop && curTop < bBottom);

    const float overlapWidth = std::fmax(0.0f, std::fmin(curRight, bRight) - std::fmax(curLeft, bLeft));
    const bool cornerLikeCeilingContact = (overlapWidth <= pW * 0.40f);
    const bool pushingRightWallNow = (player->velocityX > 0.0f && curRight > bLeft && prevRight <= bLeft + 2.0f);
    const bool pushingLeftWallNow = (player->velocityX < 0.0f && curLeft < bRight && prevLeft >= bRight - 2.0f);
    const bool movingIntoWallNow = (pushingRightWallNow || pushingLeftWallNow);

    // 1) 着地判定（上から）
    if (player->velocityY >= 0.0f && overlapX && prevBottom <= bTop + 1.0f && curBottom >= bTop)
    {
        player->posY = bTop;
        player->velocityY = 0.0f;
        player->isGrounded = true;
        player->jumpCount = 0;

        pc.left = player->posX - pW * 0.5f;
        pc.top = player->posY - pH;
        return;
    }

    // 2) 天井ヒット（下から）
    if (player->velocityY < 0.0f && overlapX && prevTop >= bBottom - 1.0f && curTop <= bBottom
        && !(cornerLikeCeilingContact && movingIntoWallNow))
    {
        player->posY = bBottom + pH;
        player->velocityY = 0.0f;

        pc.left = player->posX - pW * 0.5f;
        pc.top = player->posY - pH;
        return;
    }

    // 3) 壁ヒット（左右）
    if (overlapY)
    {
        const float wallSnapEps = 1.25f;
        const bool pushingRightWall = (player->velocityX > 0.0f && curRight > bLeft && curLeft < bLeft);
        const bool pushingLeftWall = (player->velocityX < 0.0f && curLeft < bRight && curRight > bRight);

        // 空中で移動入力により壁へ押し込んだ時は、最優先で横補正して引っかかりを防ぐ
        if (!player->isGrounded && (pushingRightWall || pushingLeftWall))
        {
            if (pushingRightWall)
            {
                player->posX = (bLeft - wallSnapEps) - pW * 0.5f;
            }
            else
            {
                player->posX = (bRight + wallSnapEps) + pW * 0.5f;
            }

            pc.left = player->posX - pW * 0.5f;
            pc.top = player->posY - pH;
            return;
        }

        if (player->velocityX > 0.0f && prevRight <= bLeft + 1.0f && curRight >= bLeft)
        {
            player->posX = bLeft - pW * 0.5f;
            pc.left = player->posX - pW * 0.5f;
            pc.top = player->posY - pH;
            return;
        }
        if (player->velocityX < 0.0f && prevLeft >= bRight - 1.0f && curLeft <= bRight)
        {
            player->posX = bRight + pW * 0.5f;
            pc.left = player->posX - pW * 0.5f;
            pc.top = player->posY - pH;
            return;
        }
    }

    // 4) 最後の保険: 重なりが小さい軸へ押し出し
    const float pCenterX = curLeft + pW * 0.5f;
    const float pCenterY = curTop + pH * 0.5f;
    const float bCenterX = (bLeft + bRight) * 0.5f;
    const float bCenterY = (bTop + bBottom) * 0.5f;
    const float dx = pCenterX - bCenterX;
    const float dy = pCenterY - bCenterY;
    const float penX = (pW * 0.5f + block.width * 0.5f) - std::fabs(dx);
    const float penY = (pH * 0.5f + block.height * 0.5f) - std::fabs(dy);

    const bool airborneWallClimbRisk = (!player->isGrounded && player->velocityY < 0.0f);
    if (airborneWallClimbRisk)
    {
        // 上昇中はジャンプ継続を優先し、常に横方向へ逃がす
        if (dx >= 0.0f) player->posX += penX;
        else player->posX -= penX;
    }
    else if (penX < penY)
    {
        player->posX += (dx >= 0.0f) ? penX : -penX;
    }
    else
    {
        player->posY += (dy >= 0.0f) ? penY : -penY;
    }

    pc.left = player->posX - pW * 0.5f;
    pc.top = player->posY - pH;
}

// 衝突解決メイン（シンプルな O(n^2)）
void ResolveCollisions()
{
    if (g_colliders.empty()) return;

    if (g_playerEnemyHitCooldown > 0)
    {
        --g_playerEnemyHitCooldown;
    }

    for (auto& c : g_colliders)
    {
        if (!c.active) continue;

        if (c.tag == ColliderTag::Player)
        {
            PlayerData* player = static_cast<PlayerData*>(c.owner);
            if (player)
            {
                player->isGrounded = false;
            }
        }
    }

    for (size_t i = 0; i < g_colliders.size(); ++i)
    {
        auto& a = g_colliders[i];
        if (!a.active) continue;

        for (size_t j = i + 1; j < g_colliders.size(); ++j)
        {
            auto& b = g_colliders[j];
            if (!b.active) continue;

            if (!AABBIntersect(a.left, a.top, a.width, a.height, b.left, b.top, b.width, b.height))
                continue;

            // Player <-> Block
            if (a.tag == ColliderTag::Player && b.tag == ColliderTag::Block)
            {
                PlayerData* player = static_cast<PlayerData*>(a.owner);
                if (!player) continue;
                ResolvePlayerBlock(player, b, a);
            }
            else if (a.tag == ColliderTag::Attack && b.tag == ColliderTag::Block)
            {
                PlayerData* player = static_cast<PlayerData*>(a.owner);
                if (player != &GetPlayerData())
                {
                    continue;
                }

                bool isDive = (player->state == PlayerState::DiveAttack);

                int mapX = (int)((b.left + b.width * 0.5f) / MAP_CHIP_WIDTH);
                int mapY = (int)((b.top + b.height * 0.5f) / MAP_CHIP_HEIGHT);

                DamageMapChip(mapX, mapY, 1, isDive);
            }
            else if (b.tag == ColliderTag::Player && a.tag == ColliderTag::Block)
            {
                PlayerData* player = static_cast<PlayerData*>(b.owner);
                if (player) ResolvePlayerBlock(player, a, b);
            }
            else if (b.tag == ColliderTag::Attack && a.tag == ColliderTag::Block)
            {
                PlayerData* player = static_cast<PlayerData*>(b.owner);
                if (player != &GetPlayerData())
                {
                    continue;
                }

                bool isDive = (player->state == PlayerState::DiveAttack);

                int mapX = (int)((b.left + b.width * 0.5f) / MAP_CHIP_WIDTH);
                int mapY = (int)((b.top + b.height * 0.5f) / MAP_CHIP_HEIGHT);

                DamageMapChip(mapX, mapY, 1, isDive);
            }
			// Player <-> SemiSolid : 下からの当たりを判定
            //（簡易実装：プレイヤーが下から接触している場合のみブロックとして扱う）
            if (a.tag == ColliderTag::Player && b.tag == ColliderTag::SemiSolid)
            {
                PlayerData* player = static_cast<PlayerData*>(a.owner);
                if (!player) continue;

                if (player->dropThrough)
                    continue;

                // 上昇中は無視
                if (player->velocityY < 0.0f)
                    continue;
				// プレイヤー矩形（左上基準）
                const float w = static_cast<float>(PLAYER_WIDTH);
                const float h = static_cast<float>(PLAYER_HEIGHT);
				// プレイヤーの底面と足場の上面を比較
                float playerBottom = player->posY;
                float prevPlayerBottom = player->prevPosY;
                float platformTop = b.top;

                // 前フレームで足場より上にいた
                bool wasAbove = prevPlayerBottom <= platformTop;

                // 今フレームで足場を跨いだ
                bool crossed = playerBottom >= platformTop;

                if (wasAbove && crossed)
                {
                    // 横方向が重なっているか確認（安全化）
                    float pLeft = player->posX - (w * 0.5f);
                    float pRight = pLeft + w;
					// 足場の左右
                    float bLeft = b.left;
                    float bRight = b.left + b.width;
					// 横方向が重なっていれば着地させる
                    if (pRight > bLeft && pLeft < bRight)
					{// 着地
                        player->posY = platformTop;
                        player->velocityY = 0.0f;
                        player->isGrounded = true;
                        player->jumpCount = 0;
                    }
                }
            }
            if (a.tag == ColliderTag::Player && b.tag == ColliderTag::Exit)
            {
                int mapX = (int)((b.left + b.width * 0.5f) / MAP_CHIP_WIDTH);
                int mapY = (int)((b.top + b.height * 0.5f) / MAP_CHIP_HEIGHT);

                MapChipData* chip = GetMapChipData(mapX, mapY);

                if (chip)
                {
                    DrawFormatString(20, 120, GetColor(255, 255, 0),
                        "Exit touched %d %d", mapX, mapY);
                    HandleExitBlock(*chip);
                }
            }
            // Player <-> Enemy : 当たりを通知（簡易実装：プレイヤーへ固定ダメージ）
            else if ((a.tag == ColliderTag::Player && b.tag == ColliderTag::Enemy) ||
                     (b.tag == ColliderTag::Player && a.tag == ColliderTag::Enemy))
            {
                Collider* playerC = (a.tag == ColliderTag::Player) ? &a : &b;
                Collider* enemyC = (a.tag == ColliderTag::Enemy) ? &a : &b;
                PlayerData* player = static_cast<PlayerData*>(playerC->owner);
                EnemyData* enemy = FindEnemyByColliderId(enemyC->id);

                if (player && player->currentHP > 0)
                {
                    if (g_playerEnemyHitCooldown <= 0)
                    {
                        DamagePlayerHP(10);
                        g_playerEnemyHitCooldown = 30; // 0.5秒(60fps想定)
                    }
                }
            }
            // Player <-> Attack : 敵の攻撃判定
            else if ((a.tag == ColliderTag::Player && b.tag == ColliderTag::Attack) ||
                     (b.tag == ColliderTag::Player && a.tag == ColliderTag::Attack))
            {
                Collider* playerC = (a.tag == ColliderTag::Player) ? &a : &b;
                Collider* attackC = (a.tag == ColliderTag::Attack) ? &a : &b;
                PlayerData* player = static_cast<PlayerData*>(playerC->owner);
                
                if (player && player->currentHP > 0 && attackC->owner != nullptr)
                {
                    if (g_playerEnemyHitCooldown <= 0)
                    {
                        EnemyData* enemy = static_cast<EnemyData*>(attackC->owner);
                        DamagePlayerHP(enemy->attackPower);
                        g_playerEnemyHitCooldown = 30;
                    }
                }
            }
            // Enemy <-> Other : プレイヤー攻撃スキル判定
            else if ((a.tag == ColliderTag::Enemy && b.tag == ColliderTag::Other) ||
                     (b.tag == ColliderTag::Enemy && a.tag == ColliderTag::Other))
            {
                Collider* enemyC = (a.tag == ColliderTag::Enemy) ? &a : &b;
                Collider* attackC = (a.tag == ColliderTag::Other) ? &a : &b;

                EnemyData* enemy = FindEnemyByColliderId(enemyC->id);
                Skill* skill = static_cast<Skill*>(attackC->owner);
                if (skill != nullptr)
                {
                    // どの弾か特定
                    auto* bullet = skill->FindBulletByCollider(attackC->id);

                    bool canHit = false;

                    if (bullet)
                    {
                        // ===== Follow弾の処理 =====
                        if (bullet->hitTargets.find(enemyC) == bullet->hitTargets.end())
                        {
                            bullet->hitTargets.insert(enemyC);
                            canHit = true;
                        }
                    }
                    else
                    {
                        // ===== 通常スキルの処理 =====
                        if (skill->RegisterHit(enemyC))
                        {
                            canHit = true;
                        }
                    }

                    if (canHit)
                    {
                        int damage = static_cast<int>(GetPlayerAttack() * skill->GetCurrentAttackRate());
                        if (damage < 1) damage = 1;

                        if (enemy != nullptr)
                        {
                            enemy->currentHP -= damage;
                            if (enemy->currentHP < 0) enemy->currentHP = 0;
                        }
                        else
                        {
                            DamageBigBossByColliderId(enemyC->id, damage);
                        }
                    }
                }
            }
            // Block <-> Other : プレイヤー攻撃スキルで破壊可能オブジェクトを破壊
            else if ((a.tag == ColliderTag::Block && b.tag == ColliderTag::Other) ||
                     (b.tag == ColliderTag::Block && a.tag == ColliderTag::Other))
            {
                Collider* blockC = (a.tag == ColliderTag::Block) ? &a : &b;
                Collider* attackC = (a.tag == ColliderTag::Other) ? &a : &b;
                Skill* skill = static_cast<Skill*>(attackC->owner);
                if (skill != nullptr && skill->GetType() == SkillType::Attack && skill->IsHitActive())
                {
                    const int mapX = static_cast<int>((blockC->left + blockC->width * 0.5f) / MAP_CHIP_WIDTH);
                    const int mapY = static_cast<int>((blockC->top + blockC->height * 0.5f) / MAP_CHIP_HEIGHT);
                    const int damage = static_cast<int>(GetPlayerAttack() * skill->GetCurrentAttackRate());
                    DamageMapChip(mapX, mapY, damage > 0 ? damage : 1, false);
                }
            }
            // 必要であれば他組合せの処理を追加

        }
    }
}

void DestroyCollidersByTag(ColliderTag tag)
{
    for (auto& c : g_colliders)
    {
        if (c.tag == tag)
        {
            c.active = false;
        }
    }
}

// ===== プレイヤーコライダー管理 =====

ColliderId CreatePlayerCollider(float centerX, float centerY, float width, float height, void* owner)
{
    float left = centerX - (width / 2.0f);
    float top = centerY - height;
    return CreateCollider(ColliderTag::Player, left, top, width, height, owner);
}

void UpdatePlayerColliderNormal(ColliderId id, float centerX, float centerY, float width, float height)
{
    float left = centerX - (width / 2.0f);
    float top = centerY - height;
    UpdateCollider(id, left, top, width, height);
}

void UpdatePlayerColliderDiveAttack(ColliderId id, float centerX, float centerY, float width, float upHeight, float downHeight)
{
    float totalHeight = upHeight + downHeight;
    float left = centerX - (width / 2.0f);
    float top = centerY - totalHeight;
    UpdateCollider(id, left, top, width, totalHeight);
}
