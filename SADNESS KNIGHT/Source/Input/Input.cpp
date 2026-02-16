#include "Input.h"
#include "DxLib.h"

// 内部で使用する変数
namespace
{
    char currentKeyState[256] = { 0 };   // 現在のフレームのキー状態
    char previousKeyState[256] = { 0 };  // 前のフレームのキー状態

    /// <summary>
    /// 指定したキーの状態を取得
    /// </summary>
    KeyState GetKeyStateInternal(int keyCode)
    {
        bool current = currentKeyState[keyCode] == 1;
        bool previous = previousKeyState[keyCode] == 1;

        if (current && !previous)
        {
            return KeyState::Pressed;   // 押された瞬間
        }
        else if (current && previous)
        {
            return KeyState::Held;      // 押し続けている
        }
        else if (!current && previous)
        {
            return KeyState::Released;  // 離された瞬間
        }

        return KeyState::None;          // 押されていない
    }
}

/// <summary>
/// 入力システムの初期化
/// </summary>
void InitInput()
{
    // キー状態を初期化
    for (int i = 0; i < 256; i++)
    {
        currentKeyState[i] = 0;
        previousKeyState[i] = 0;
    }
}

/// <summary>
/// 入力システムの更新（毎フレーム呼び出す）
/// </summary>
void UpdateInput()
{
    // 前フレームのキー状態を保存
    for (int i = 0; i < 256; i++)
    {
        previousKeyState[i] = currentKeyState[i];
    }

    // 現在のキー状態を取得
    GetHitKeyStateAll(currentKeyState);
}

/// <summary>
/// 左移動キー(A)が押されているか
/// </summary>
bool IsMoveLeft()
{
    return currentKeyState[KEY_INPUT_A] == 1;
}

/// <summary>
/// 右移動キー(D)が押されているか
/// </summary>
bool IsMoveRight()
{
    return currentKeyState[KEY_INPUT_D] == 1;
}

/// <summary>
/// 上移動キー(W)が押されているか
/// </summary>
bool IsMoveUp()
{
    return currentKeyState[KEY_INPUT_W] == 1;
}

/// <summary>
/// 下移動キー(S)が押されているか
/// </summary>
bool IsMoveDown()
{
    return currentKeyState[KEY_INPUT_S] == 1;
}

/// <summary>
/// 水平方向の移動入力を取得
/// </summary>
float GetMoveHorizontal()
{
    float horizontal = 0.0f;

    if (IsMoveLeft())
    {
        horizontal -= 1.0f;
    }
    if (IsMoveRight())
    {
        horizontal += 1.0f;
    }

    return horizontal;
}

/// <summary>
/// 垂直方向の移動入力を取得
/// </summary>
float GetMoveVertical()
{
    float vertical = 0.0f;

    if (IsMoveDown())
    {
        vertical -= 1.0f;
    }
    if (IsMoveUp())
    {
        vertical += 1.0f;
    }

    return vertical;
}

/// <summary>
/// スキル1(Q)の状態を取得
/// </summary>
KeyState GetSkill1State()
{
    return GetKeyStateInternal(KEY_INPUT_Q);
}

/// <summary>
/// スキル2(E)の状態を取得
/// </summary>
KeyState GetSkill2State()
{
    return GetKeyStateInternal(KEY_INPUT_E);
}

/// <summary>
/// スキル3(F)の状態を取得
/// </summary>
KeyState GetSkill3State()
{
    return GetKeyStateInternal(KEY_INPUT_F);
}

/// <summary>
/// スキル1(Q)が押された瞬間か
/// </summary>
bool IsSkill1Pressed()
{
    return GetSkill1State() == KeyState::Pressed;
}

/// <summary>
/// スキル2(E)が押された瞬間か
/// </summary>
bool IsSkill2Pressed()
{
    return GetSkill2State() == KeyState::Pressed;
}

/// <summary>
/// スキル3(F)が押された瞬間か
/// </summary>
bool IsSkill3Pressed()
{
    return GetSkill3State() == KeyState::Pressed;
}

KeyState GetChangeSetState()
{
    return GetKeyStateInternal(KEY_INPUT_R);
}

bool IsChangeSetPressed()
{
    return GetChangeSetState() == KeyState::Pressed;
}

