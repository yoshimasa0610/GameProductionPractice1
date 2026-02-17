#include "Input.h"
#include "ControlConfig.h"

// 入力状態
static int g_InputState = 0;
static int g_PrevInputState = 0;
// 決定後一部部分でそのまま適用されるケースがあるため、追加
static int g_OKGuardTimer = 0;

void InitInput()
{
    g_InputState = 0;
    g_PrevInputState = 0;
    g_OKGuardTimer = 0;

    ControlConfig::InitDefault();
}

void UpdateInput()
{
    g_PrevInputState = g_InputState;
    g_InputState = 0;

    if (g_OKGuardTimer > 0)
        g_OKGuardTimer--;

    // 全 InputKey を総当たりで取得
    for (int i = 0; i < 15; i++)
    {
        InputKey key = static_cast<InputKey>(1 << i);
        if (ControlConfig::IsPressed(key))
        {
            g_InputState |= key;
        }
    }
}

bool IsInputKey(InputKey key)
{
    return (g_InputState & key) != 0;
}

bool IsTriggerKey(InputKey key)
{
    return ((g_InputState & key) != 0) &&
        ((g_PrevInputState & key) == 0);
}

bool IsInputOKGuarded()
{
    if (g_OKGuardTimer > 0)
        return false;

    if (IsTriggerKey(KEY_OK))
    {
        g_OKGuardTimer = 12;
        return true;
    }
    return false;
}