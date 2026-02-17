#pragma once

enum OptionState
{
    OPTION_STATE_CATEGORY_SELECT, // 左側を選んでいる
    OPTION_STATE_ITEM_ADJUST       // 右側を操作している
};

enum OptionCategory
{
    OPTION_CATEGORY_SOUND,
    OPTION_CATEGORY_CONTROL_CONFIG,
    OPTION_CATEGORY_CONTROL_HELP,   // 操作説明
    OPTION_CATEGORY_BACK,
    OPTION_CATEGORY_MAX
};

enum KeyConfigState
{
    KEYCFG_NORMAL,        // 通常選択中
    KEYCFG_WAIT_KEY,      // 新しいキー入力待ち
    KEYCFG_CONFIRM_SWAP,   // 入れ替え確認中
    KEYCFG_UNSUPPORTED_KEY   // 対応していないときのキーをい入力した際に起こすもの
};

enum ConfirmOverlayState
{
    CONFIRM_NONE,
    CONFIRM_WAIT,   // YES / NO 選択中
};

void OpenOption();
void CloseOption();
void UpdateOption();
void DrawOption();
bool IsOptionOpen();
