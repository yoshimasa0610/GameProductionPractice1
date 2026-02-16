#pragma once

// キーの状態
enum class KeyState
{
    None,       // 押されていない
    Pressed,    // 押された瞬間
    Held,       // 押し続けている
    Released    // 離された瞬間
};

/// <summary>
/// 入力システムの初期化
/// </summary>
void InitInput();

/// <summary>
/// 入力システムの更新（毎フレーム呼び出す）
/// </summary>
void UpdateInput();

// 移動入力取得関数
/// <summary>
/// 左移動キー(A)が押されているか
/// </summary>
bool IsMoveLeft();

/// <summary>
/// 右移動キー(D)が押されているか
/// </summary>
bool IsMoveRight();

/// <summary>
/// 上移動キー(W)が押されているか
/// </summary>
bool IsMoveUp();

/// <summary>
/// 下移動キー(S)が押されているか
/// </summary>
bool IsMoveDown();

/// <summary>
/// 水平方向の移動入力を取得 (-1.0f: 左, 0.0f: なし, 1.0f: 右)
/// </summary>
float GetMoveHorizontal();

/// <summary>
/// 垂直方向の移動入力を取得 (-1.0f: 下, 0.0f: なし, 1.0f: 上)
/// </summary>
float GetMoveVertical();

// スキル入力取得関数
/// <summary>
/// スキル1(Q)の状態を取得
/// </summary>
KeyState GetSkill1State();

/// <summary>
/// スキル2(E)の状態を取得
/// </summary>
KeyState GetSkill2State();

/// <summary>
/// スキル3(F)の状態を取得
/// </summary>
KeyState GetSkill3State();

/// <summary>
/// スキル1(Q)が押された瞬間か
/// </summary>
bool IsSkill1Pressed();

/// <summary>
/// スキル2(E)が押された瞬間か
/// </summary>
bool IsSkill2Pressed();

/// <summary>
/// スキル3(F)が押された瞬間か
/// </summary>
bool IsSkill3Pressed();

// セット切替 (Rキー)
KeyState GetChangeSetState();
bool IsChangeSetPressed();