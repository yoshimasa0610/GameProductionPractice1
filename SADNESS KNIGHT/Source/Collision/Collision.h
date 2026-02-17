#pragma once
#include <cstdint>

// PlayerData を参照する宣言（完全定義は実装側で取り込む）
struct PlayerData;

// Map 側データ構造体（前方宣言）
struct MapChipData;

//
// Collision モジュール
// - シンプルな AABB コライダー管理（作成・更新・破棄）
// - 毎フレーム ResolveCollisions を呼ぶことでプレイヤーとブロックの押し出し・着地判定、
//   プレイヤーと敵の当たり検出を行います。
///

// コライダーのタグ（用途に応じて拡張）
enum class ColliderTag
{
    Player,
    Enemy,
	Block,// 壁や床などの通常ブロック
	SemiSolid,// 下からはすり抜けるブロック
    Other
};

// コライダーID
using ColliderId = int;

// コライダー作成（左上座標、幅・高さ、任意の owner ポインタを渡す）
// owner には PlayerData* や Enemy 構造体のポインタを入れて通知に使えます。
ColliderId CreateCollider(ColliderTag tag, float left, float top, float width, float height, void* owner = nullptr);

// コライダー情報を更新（毎フレーム位置が変わるものは呼ぶ）
void UpdateCollider(ColliderId id, float left, float top, float width, float height);

// 指定コライダーを無効化（破棄）
void DestroyCollider(ColliderId id);

// 全コライダーを消去（ステージ切替時など）
void ClearAllColliders();

// 衝突解決を行う（ゲームループ内で毎フレーム呼ぶ）
// - プレイヤー⇔ブロック の押し出し・着地判定
// - プレイヤー⇔敵 の当たり検出（現在はプレイヤーにダメージを与える実装）
void ResolveCollisions();

// AABB 判定ユーティリティ（左上基準）
bool AABBIntersect(float aLeft, float aTop, float aW, float aH,
                   float bLeft, float bTop, float bW, float bH);

// --- プレイヤー用便利判定 / 補正関数 ---
// MapManager 等の既存呼び出し形式に合わせたオーバーロードを提供します。
// - PlayerHitNormalBlockX/Y(PlayerData*, newPos) : PlayerData を明示的に渡す版
// - PlayerHitNormalBlockX/Y(newPos)             : 位置だけ渡す版（互換）
// - PlayerHitNormalBlockX/Y(const MapChipData&) : MapManager が呼ぶ版（タイル情報を渡す）
bool PlayerHitNormalBlockX(PlayerData* player, float newPosX);
bool PlayerHitNormalBlockY(PlayerData* player, float newPosY);

bool PlayerHitNormalBlockX(const MapChipData& mc);
bool PlayerHitNormalBlockY(const MapChipData& mc);

// --- マップ管理向けラッパー（MapManager から使う想定） ---
ColliderId CreateBlockCollider(float left, float top, float width, float height);
void RemoveBlockCollider(ColliderId id);