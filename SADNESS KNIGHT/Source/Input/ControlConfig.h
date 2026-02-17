#pragma once
#include "DxLib.h"
#include <unordered_map>
#include <string>
#include "Input.h"

enum DeviceType
{
    DEVICE_KEYBOARD,
    DEVICE_PAD
};

struct BindKey
{
    DeviceType device;
    int keyCode;   // DxLib key or PAD_INPUT_*
};

struct ActionBind
{
    BindKey keyboard;
    BindKey pad;
};

class ControlConfig
{
public:
    // デフォルト割り当て生成
    static void InitDefault();

    // 割り当て取得 / 設定
    static BindKey GetBind(InputKey key, DeviceType device);
    static void SetBind(InputKey key, DeviceType device, const BindKey& bind);

    // 役割のあるキー同士を変える際に必要な処理
    static InputKey FindActionByBind(const BindKey& bind);
    static void Assign(InputKey target, const BindKey& bind);
    static void Swap(InputKey a, InputKey b, DeviceType device);

    // 入力判定（現在）
    static bool IsPressed(InputKey key);

    // キー名表示用
    static std::string GetKeyName(const BindKey& bind);

    // 重複チェック
    static bool IsDuplicate(InputKey target, const BindKey& bind);

    static bool IsSupportedBind(const BindKey& bind);

private:
    static std::unordered_map<InputKey, ActionBind> m_Binds;
};