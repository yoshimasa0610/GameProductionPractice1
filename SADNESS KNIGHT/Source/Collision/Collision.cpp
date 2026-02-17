#include "Collision.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include "../Player/Player.h"
#include "DxLib.h"
#include "../Map/MapParameter.h"
#include "../Map/MapChip.h"

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

    if (aRight <= bLeft) return false;
    if (aLeft >= bRight) return false;
    if (aBottom <= bTop) return false;
    if (aTop >= bBottom) return false;
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

// 内部：プレイヤーとブロックの押し出し / 着地判定（player の owner は PlayerData* を想定）
static void ResolvePlayerBlock(PlayerData* player, const Collider& block, Collider& pc)
{
    if (player == nullptr) return;

    // プレイヤー矩形（左上基準）
    float pLeft = pc.left;
    float pTop = pc.top;
    float pW = pc.width;
    float pH = pc.height;

    float bLeft = block.left;
    float bTop = block.top;
    float bW = block.width;
    float bH = block.height;

    // 中心差分で浸透量を計算
    float pCenterX = pLeft + pW * 0.5f;
    float pCenterY = pTop + pH * 0.5f;
    float bCenterX = bLeft + bW * 0.5f;
    float bCenterY = bTop + bH * 0.5f;

    float dx = pCenterX - bCenterX;
    float dy = pCenterY - bCenterY;
    float overlapX = (pW * 0.5f + bW * 0.5f) - std::fabs(dx);
    float overlapY = (pH * 0.5f + bH * 0.5f) - std::fabs(dy);

    if (overlapX > 0.0f && overlapY > 0.0f)
    {
        if (overlapX < overlapY)
        {
            // 横方向に押し出す
            if (dx > 0.0f) // プレイヤーが右側 -> 右へ押し出す
                player->posX += overlapX;
            else
                player->posX -= overlapX;
        }
        else
        {
            // 縦方向に押し出す
            if (dy > 0.0f) // プレイヤーが下側 -> 下へ押し出す
            {
                player->posY += overlapY;
            }
            else // プレイヤーが上側 -> ブロック上に着地
            {
                player->posY -= overlapY;
                player->velocityY = 0.0f;
                player->isGrounded = true;
                player->jumpCount = 0;
            }
        }
    }
}

// 衝突解決メイン（シンプルな O(n^2)）
void ResolveCollisions()
{
    if (g_colliders.empty()) return;

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
                if (player) ResolvePlayerBlock(player, b, a);
            }
            else if (b.tag == ColliderTag::Player && a.tag == ColliderTag::Block)
            {
                PlayerData* player = static_cast<PlayerData*>(b.owner);
                if (player) ResolvePlayerBlock(player, a, b);
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
            // Player <-> Enemy : 当たりを通知（簡易実装：プレイヤーへ固定ダメージ）
            else if ((a.tag == ColliderTag::Player && b.tag == ColliderTag::Enemy) ||
                     (b.tag == ColliderTag::Player && a.tag == ColliderTag::Enemy))
            {
                Collider* playerC = (a.tag == ColliderTag::Player) ? &a : &b;
                PlayerData* player = static_cast<PlayerData*>(playerC->owner);
                if (player)
                {
                    // 固定ダメージ（必要に応じて owner をキャストして敵のダメージ値を参照する実装に変更）
                    DamagePlayerHP(10);
                }
            }
            // 必要であれば他組合せの処理を追加
        }
    }
}