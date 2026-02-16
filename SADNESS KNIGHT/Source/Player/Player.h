#pragma once

// プレイヤーの状態
enum class PlayerState
{
    Idle,       // 待機
    Walk,       // 歩き
    Jump,       // ジャンプ
    Fall,       // 落下
    Skill1,     // スキル1使用中
    Skill2,     // スキル2使用中
    Skill3      // スキル3使用中
};

/// <summary>
/// プレイヤーの初期化
/// </summary>
/// <param name="startX">開始X座標</param>
/// <param name="startY">開始Y座標</param>
void InitPlayer(float startX, float startY);

/// <summary>
/// プレイヤーのリソース読み込み
/// </summary>
void LoadPlayer();

/// <summary>
/// プレイヤーの更新
/// </summary>
void UpdatePlayer();

/// <summary>
/// プレイヤーの描画
/// </summary>
void DrawPlayer();

/// <summary>
/// プレイヤーのリソース解放
/// </summary>
void UnloadPlayer();

/// <summary>
/// プレイヤーのX座標を取得
/// </summary>
float GetPlayerX();

/// <summary>
/// プレイヤーのY座標を取得
/// </summary>
float GetPlayerY();

/// <summary>
/// プレイヤーの座標を取得
/// </summary>
void GetPlayerPos(float& outX, float& outY);

/// <summary>
/// プレイヤーの状態を取得
/// </summary>
PlayerState GetPlayerState();

/// <summary>
/// プレイヤーが右を向いているか
/// </summary>
bool IsPlayerFacingRight();

/// <summary>
/// プレイヤーが地面にいるか
/// </summary>
bool IsPlayerGrounded();