#include "ControlConfig.h"

std::unordered_map<InputKey, ActionBind> ControlConfig::m_Binds;

void ControlConfig::InitDefault()
{
    m_Binds.clear();

    for (int i = 0; i < ACTION_MAX; i++)
    {
        InputKey key = (InputKey)(1 << i);
        m_Binds[key] = ActionBind{};
    }

    // =====================
    // Keyboard defaults
    // =====================
    m_Binds[KEY_UP].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_UP };
    m_Binds[KEY_DOWN].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_DOWN };
    m_Binds[KEY_LEFT].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_LEFT };
    m_Binds[KEY_RIGHT].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_RIGHT };

    m_Binds[KEY_JUMP].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_SPACE };
    m_Binds[KEY_OK].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_SPACE };
    m_Binds[KEY_SKILL1].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_X };
    m_Binds[KEY_SKILL2].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_Z };
    m_Binds[KEY_SKILL3].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_D };
    m_Binds[KEY_CHANGE].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_C };
    m_Binds[KEY_DODGE].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_A };
    m_Binds[KEY_HEAL].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_TAB };
    m_Binds[KEY_MENU].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_ESCAPE };
    m_Binds[KEY_CANCEL].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_ESCAPE };
    m_Binds[KEY_INVENTORY].keyboard = { DEVICE_KEYBOARD, KEY_INPUT_S };

    // =====================
    // Pad defaults（仮でOK）
    // =====================
    m_Binds[KEY_UP].pad = { DEVICE_PAD, PAD_INPUT_UP };
    m_Binds[KEY_DOWN].pad = { DEVICE_PAD, PAD_INPUT_DOWN };
    m_Binds[KEY_LEFT].pad = { DEVICE_PAD, PAD_INPUT_LEFT };
    m_Binds[KEY_RIGHT].pad = { DEVICE_PAD, PAD_INPUT_RIGHT };

    m_Binds[KEY_JUMP].pad = { DEVICE_PAD, PAD_INPUT_A };
    m_Binds[KEY_SKILL1].pad = { DEVICE_PAD, PAD_INPUT_X };
    m_Binds[KEY_SKILL2].pad = { DEVICE_PAD, PAD_INPUT_Y };
    m_Binds[KEY_SKILL3].pad = { DEVICE_PAD, PAD_INPUT_B };
    m_Binds[KEY_CHANGE].pad = { DEVICE_PAD, PAD_INPUT_B };
    m_Binds[KEY_DODGE].pad = { DEVICE_PAD, PAD_INPUT_R };
    m_Binds[KEY_OK].pad = { DEVICE_PAD, PAD_INPUT_A };
    m_Binds[KEY_CANCEL].pad = { DEVICE_PAD, PAD_INPUT_B };
    m_Binds[KEY_MENU].pad = { DEVICE_PAD, PAD_INPUT_START };
}

BindKey ControlConfig::GetBind(InputKey key, DeviceType device)
{
    if (device == DEVICE_KEYBOARD)
        return m_Binds[key].keyboard;
    else
        return m_Binds[key].pad;
}

void ControlConfig::SetBind(InputKey key, DeviceType device, const BindKey& bind)
{
    if (device == DEVICE_KEYBOARD)
        m_Binds[key].keyboard = bind;
    else
        m_Binds[key].pad = bind;
}

InputKey ControlConfig::FindActionByBind(const BindKey& bind)
{
    for (auto& pair : m_Binds)
    {
        const ActionBind& ab = pair.second;

        const BindKey& b =
            (bind.device == DEVICE_KEYBOARD)
            ? ab.keyboard
            : ab.pad;

        if (b.device == bind.device &&
            b.keyCode == bind.keyCode)
        {
            return pair.first;
        }
    }
    return INPUT_NONE;
}

void ControlConfig::Swap(InputKey a, InputKey b, DeviceType device)
{
    if (device == DEVICE_KEYBOARD)
        std::swap(m_Binds[a].keyboard, m_Binds[b].keyboard);
    else
        std::swap(m_Binds[a].pad, m_Binds[b].pad);
}

void ControlConfig::Assign(InputKey target, const BindKey& bind)
{
    if (bind.device == DEVICE_KEYBOARD)
    {
        m_Binds[target].keyboard = bind;
    }
    else
    {
        m_Binds[target].pad = bind;
    }
}

bool ControlConfig::IsPressed(InputKey key)
{
    const ActionBind& ab = m_Binds[key];

    // Keyboard
    if (ab.keyboard.device == DEVICE_KEYBOARD &&
        CheckHitKey(ab.keyboard.keyCode))
        return true;

    // Pad
    if (ab.pad.device == DEVICE_PAD)
    {
        int pad = GetJoypadInputState(DX_INPUT_PAD1);
        if (pad & ab.pad.keyCode)
            return true;
    }

    return false;
}

bool ControlConfig::IsDuplicate(InputKey target, const BindKey& bind)
{
    for (auto& pair : m_Binds)
    {
        if (pair.first == target)
            continue;

        const ActionBind& ab = pair.second;
        const BindKey& other =
            (bind.device == DEVICE_KEYBOARD)
            ? ab.keyboard
            : ab.pad;

        if (other.device == bind.device &&
            other.keyCode == bind.keyCode)
        {
            return true;
        }
    }
    return false;
}

std::string ControlConfig::GetKeyName(const BindKey& bind)
{
    if (bind.device == DEVICE_KEYBOARD)
    {
        switch (bind.keyCode)
        {
        case KEY_INPUT_UP: return "Up";
        case KEY_INPUT_DOWN: return "Down";
        case KEY_INPUT_LEFT: return "Left";
        case KEY_INPUT_RIGHT: return "Right";
        case KEY_INPUT_SPACE: return "Space";
        case KEY_INPUT_X: return "X";
        case KEY_INPUT_Z: return "Z";
        case KEY_INPUT_C: return "C";
        case KEY_INPUT_A: return "A";
        case KEY_INPUT_D: return "D";
        case KEY_INPUT_S: return "S";
        case KEY_INPUT_TAB: return "Tab";
        case KEY_INPUT_ESCAPE: return "Esc";
            //以下キーコンフィグを変えたい用
        case KEY_INPUT_Q: return "Q";
        case KEY_INPUT_W: return "W";
        case KEY_INPUT_E: return "E";
        case KEY_INPUT_R: return "R";
        case KEY_INPUT_F: return "F";
        case KEY_INPUT_V: return "V";
        case KEY_INPUT_LSHIFT: return "SHIFT";
        case KEY_INPUT_LCONTROL: return "Ctrl";
        default: return "Key";
        }
    }

    if (bind.device == DEVICE_PAD)
    {
        switch (bind.keyCode)
        {
        case PAD_INPUT_UP: return "Up";
        case PAD_INPUT_DOWN: return "Down";
        case PAD_INPUT_LEFT: return "Left";
        case PAD_INPUT_RIGHT: return "Right";
        case PAD_INPUT_A: return "A";
        case PAD_INPUT_B: return "B";
        case PAD_INPUT_X: return "X";
        case PAD_INPUT_Y: return "Y";
        case PAD_INPUT_L: return "L1";
        case PAD_INPUT_R: return "R1";
        case PAD_INPUT_W: return "R2";
        case PAD_INPUT_Z: return "L2";
        case PAD_INPUT_START: return "Start";
        default: return "Pad";
        }
    }
}

bool ControlConfig::IsSupportedBind(const BindKey& bind)
{
    if (bind.device == DEVICE_KEYBOARD)
    {
        switch (bind.keyCode)
        {
        case KEY_INPUT_UP:
        case KEY_INPUT_DOWN:
        case KEY_INPUT_LEFT:
        case KEY_INPUT_RIGHT:
        case KEY_INPUT_SPACE:
        case KEY_INPUT_X:
        case KEY_INPUT_Z:
        case KEY_INPUT_C:
        case KEY_INPUT_A:
        case KEY_INPUT_D:
        case KEY_INPUT_S:
        case KEY_INPUT_TAB:
        case KEY_INPUT_ESCAPE:
        case KEY_INPUT_Q:
        case KEY_INPUT_W:
        case KEY_INPUT_E:
        case KEY_INPUT_R:
        case KEY_INPUT_F:
        case KEY_INPUT_V:
        case KEY_INPUT_LSHIFT:
        case KEY_INPUT_LCONTROL:
            return true;
        default:
            return false;
        }
    }

    if (bind.device == DEVICE_PAD)
    {
        switch (bind.keyCode)
        {
        case PAD_INPUT_A:
        case PAD_INPUT_B:
        case PAD_INPUT_X:
        case PAD_INPUT_Y:
        case PAD_INPUT_L:
        case PAD_INPUT_R:
        case PAD_INPUT_W:
        case PAD_INPUT_Z:
        case PAD_INPUT_START:
            return true;
        default:
            return false;
        }
    }

    return false;
}