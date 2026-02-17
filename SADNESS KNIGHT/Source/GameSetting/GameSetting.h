#pragma once
//画面サイズは前のままにしてるけど変えてもいいよん

// 画面設定
#define SCREEN_WIDTH (1600)		// 画面の横幅
#define SCREEN_HEIGHT (900)	// 画面の高さ
#define SCREEN_COLOR_DEPTH (32)	// 画面のカラービット数

// 透過色
#define TRANS_COLOR_R (1)	// 透過色のR値
#define TRANS_COLOR_G (100)	// 透過色のG値
#define TRANS_COLOR_B (2)	// 透過色のB値

//==============================
// GameSetting
//==============================
#include "../Input/Input.h"
#include "../Input/ControlConfig.h"

namespace GameSetting
{
    // 起動時
    void Load();

    // 保存
    void Save();

    // ---- Sound ----
    void SetBGMVolume(int v);
    void SetSEVolume(int v);
    int  GetBGMVolume();
    int  GetSEVolume();

    // ---- KeyConfig ----
    void ApplyToControlConfig();
    // デフォルト復帰（未実装）
    void ResetKeyConfig();
}