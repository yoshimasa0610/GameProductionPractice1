#pragma once
#include "DxLib.h"

#define ACTION_MAX 20
// 入力ボタン定義（ゲーム内で共通利用）
enum InputKey
{
    INPUT_NONE = 0,
    KEY_UP = (1 << 0),
    KEY_DOWN = (1 << 1),
    KEY_LEFT = (1 << 2),
    KEY_RIGHT = (1 << 3),
    KEY_JUMP = (1 << 4),  // ジャンプ
    KEY_SKILL1 = (1 << 5),  // スキル1
    KEY_SKILL2 = (1 << 6),  // スキル2
    KEY_SKILL3 = (1 << 7),  // スキル3
    KEY_CHANGE = (1 << 8),  // スキルセットの変更
    KEY_DODGE = (1 << 9),  // 回避
    KEY_HEAL = (1 << 10), // 回復(残機?)
    KEY_MENU = (1 << 11), // メニュー
    KEY_INVENTORY = (1 << 12), // インベントリ
    KEY_OK = ((1 << 13)),  //決定!!!!
    KEY_CANCEL = ((1 << 14)),  //メニューなどの戻る
    KEY_UI_LEFT = ((1 << 15)), //UI用のキーなぜか上記のKEY_LEFTと一緒にしたらクラッシュしたので
    KEY_UI_RIGHT = ((1 << 16)),//こういう感じにしております
    KEY_DIVE_ATTACK = ((1 << 17)),
    KEY_CLEAR = ((1 << 18)),
};

// プロトタイプ宣言
void InitInput();
void UpdateInput();

void ResetInput();

void AutoDetectInputMode();

bool IsInputKey(InputKey key);     // 入力判定
bool IsTriggerKey(InputKey key);   // トリガー判定

bool IsInputOKGuarded();

int GetInputFrame();