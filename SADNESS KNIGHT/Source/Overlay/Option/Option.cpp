/*
#include "DxLib.h"
#include "Option.h"
#include "../../Scene/SceneManager.h"
#include "../../Input/Input.h"
#include "../../Scene/Play/Play.h"
#include "../../GameSetting/GameSetting.h"
#include "../../Sound/Sound.h"

//==================================================
// 内部状態
//==================================================

static bool g_IsOptionOpen = false;

static OptionState    g_OptionState = OPTION_STATE_CATEGORY_SELECT;
static OptionCategory g_CategoryIndex = OPTION_CATEGORY_SOUND;

// 右側用インデックス
static int g_ItemIndex = 0;

// 音量（0～10）
static int g_BGMVolumeLevel = 5;
static int g_SEVolumeLevel = 5;

// 操作説明ウィンドウ表示中か
static bool g_IsControlHelpOpen = false;
// 操作説明用インデックス
static int g_HelpItemIndex = 0;

// 現在選択中のアクション
static int g_ConfigActionIndex = 0;
static int g_KeyWaitStartFrame = 0;
static int  g_WaitStartTime = 0;

// キーコンフィグ Overlay 表示中か
static bool g_IsControlConfigOpen = false;
// キーを変更する際にフラグを変える
static bool g_IsKeyConfigChanged = false;
static KeyConfigState g_KeyConfigState = KEYCFG_NORMAL;
// 確認用（キー配置を変える際に相手が役割持ちの場合に使用）
static InputKey g_PendingTarget = INPUT_NONE;
static InputKey g_PendingOther = INPUT_NONE;
static BindKey  g_PendingBind;
// キーを入れ替えるかどうかの専用確認Index
static int g_ConfirmIndex = 0;
// 現在編集中のデバイス
static DeviceType g_KeyConfigDevice = DEVICE_KEYBOARD;

// こちらもキーの配置を変える際に相手が役割持ちかどうかを確認する
static int GetActionIndexFromInputKey(InputKey key)
{
    for (int i = 0; i < ACTION_MAX; i++)
    {
        if (key == (InputKey)(1 << i))
        {
            return i;
        }
    }
    return -1;
}

//==================================================
// 表示用文字列
//==================================================

static const char* g_CategoryNames[] =
{
    "音量",
    "操作設定",
    "操作説明",
    "戻る"
};

//==================================================
// 操作説明データ
//==================================================

struct ControlHelpData
{
    const char* name;
    const char* description;
};
static const ControlHelpData g_ControlHelpList[] =
{
    {
        "移動",
        "・左右キー、またはLスティックでプレイヤーを移動します。\n"
        "\n"
        "・また、空中でも操作可能です。"
    },
    {
        "ジャンプ",
        "・ジャンプキーで跳躍します。\n"
        "\n"
        "・2段ジャンプが可能です。"
    },
     {
        "ダッシュ",
        "・素早く前方へ移動します。\n"
        "\n"
        "・ダッシュ中は一時的に無敵です。\n"
        "\n"
        "・ダッシュを使用すると短時間の間使用できません。\n"
        "\n"
        "・空中では一回しか発動できません。\n"
    },
    {
        "近接攻撃",
        "・近接攻撃キーで前方を攻撃します。\n"
        "\n"
    },
    {

    },
    {

    },
    {
        "回復",
        "・回復キーを長押しすると、回復を行う体制に移行します。\n"
        "・その状態で一定の時間経過すると、Lifeを消費してHPが回復します。\n"

    },
};

static const int g_ControlHelpCount =
sizeof(g_ControlHelpList) / sizeof(g_ControlHelpList[0]);

// キーコンフィグの変更時に必要な処理
static const char* g_ActionNames[ACTION_MAX] =
{
    "上移動",
    "下移動",
    "左移動",
    "右移動",
    "ジャンプ",
    "近接攻撃",
    "遠距離攻撃（強）",
    "遠距離攻撃（弱）",
    "ダッシュ",
    "回復",
    "マップ",
    "メニュー",
    "インベントリ",
    "決定",
    "キャンセル"
};


//==================================================
// 基本制御
//==================================================

void OpenOption()
{
    g_IsOptionOpen = true;
    g_OptionState = OPTION_STATE_CATEGORY_SELECT;
    g_CategoryIndex = OPTION_CATEGORY_SOUND;
    g_ItemIndex = 0;
    g_HelpItemIndex = 0;
    g_IsControlHelpOpen = false;
    g_ConfigActionIndex = 0;
}

void CloseOption()
{
    g_IsOptionOpen = false;
    g_IsControlHelpOpen = false;
    g_IsControlConfigOpen = false;

    GameSetting::Save();
}

bool IsOptionOpen()
{
    return g_IsOptionOpen;
}

//==================================================
// Update
//==================================================

static void UpdateCategorySelect()
{
    if (IsTriggerKey(KEY_UP))
    {
        g_CategoryIndex =
            (OptionCategory)((g_CategoryIndex - 1 + OPTION_CATEGORY_MAX) % OPTION_CATEGORY_MAX);
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_DOWN))
    {
        g_CategoryIndex =
            (OptionCategory)((g_CategoryIndex + 1) % OPTION_CATEGORY_MAX);
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_OK))
    {
        if (g_CategoryIndex == OPTION_CATEGORY_BACK)
        {
            PlaySE(SE_MENU_OK);
            CloseOption();
            return;
        }

        if (g_CategoryIndex == OPTION_CATEGORY_CONTROL_HELP)
        {
            // 操作説明は OK 押下で開く
            g_IsControlHelpOpen = true;
            g_HelpItemIndex = 0;
            g_OptionState = OPTION_STATE_ITEM_ADJUST;
            PlaySE(SE_MENU_OK);
            return;
        }

        if (g_CategoryIndex == OPTION_CATEGORY_CONTROL_CONFIG)
        {
            g_IsControlConfigOpen = true;
            g_ConfigActionIndex = 0;
            g_OptionState = OPTION_STATE_ITEM_ADJUST;
            PlaySE(SE_MENU_OK);
            return;
        }

        g_ItemIndex = 0;
        g_OptionState = OPTION_STATE_ITEM_ADJUST;
        PlaySE(SE_MENU_OK);
    }

    if (IsTriggerKey(KEY_CANCEL))
    {
        CloseOption();
    }
}

static void UpdateSoundOption()
{
    const int itemCount = 2; // BGM / SE

    if (IsTriggerKey(KEY_UP))
    {
        g_ItemIndex = (g_ItemIndex - 1 + itemCount) % itemCount;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_DOWN))
    {
        g_ItemIndex = (g_ItemIndex + 1) % itemCount;
        PlaySE(SE_MENU_MOVE);
    }

    if (g_ItemIndex == 0)
    {
        if (IsTriggerKey(KEY_LEFT))
        {
            GameSetting::SetBGMVolume(GameSetting::GetBGMVolume() - 1);
            g_BGMVolumeLevel = GameSetting::GetBGMVolume();
            PlaySE(SE_MENU_SELECT);
        }

        if (IsTriggerKey(KEY_RIGHT))
        {
            GameSetting::SetBGMVolume(GameSetting::GetBGMVolume() + 1);
            g_BGMVolumeLevel = GameSetting::GetBGMVolume();
            PlaySE(SE_MENU_SELECT);
        }
    }
    else if (g_ItemIndex == 1)
    {
        if (IsTriggerKey(KEY_LEFT))
        {
            GameSetting::SetSEVolume(GameSetting::GetSEVolume() - 1);
            g_SEVolumeLevel = GameSetting::GetSEVolume();
            PlaySE(SE_MENU_SELECT);
        }

        if (IsTriggerKey(KEY_RIGHT))
        {
            GameSetting::SetSEVolume(GameSetting::GetSEVolume() + 1);
            g_SEVolumeLevel = GameSetting::GetSEVolume();
            PlaySE(SE_MENU_SELECT);
        }
    }
}

static void UpdateControlHelp()
{
    if (!g_IsControlHelpOpen) return;

    if (IsTriggerKey(KEY_UP))
    {
        g_HelpItemIndex =
            (g_HelpItemIndex - 1 + g_ControlHelpCount) % g_ControlHelpCount;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_DOWN))
    {
        g_HelpItemIndex =
            (g_HelpItemIndex + 1) % g_ControlHelpCount;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_CANCEL))
    {
        g_IsControlHelpOpen = false;
        g_OptionState = OPTION_STATE_CATEGORY_SELECT;
        PlaySE(SE_MENU_OK);
    }
}

static void UpdateControlConfig()
{
    InputKey target = (InputKey)(1 << g_ConfigActionIndex);

    if (g_KeyConfigState == KEYCFG_NORMAL)
    {
        if (IsTriggerKey(KEY_LEFT) || IsTriggerKey(KEY_RIGHT))
        {
            g_KeyConfigDevice =
                (g_KeyConfigDevice == DEVICE_KEYBOARD)
                ? DEVICE_PAD
                : DEVICE_KEYBOARD;

            PlaySE(SE_MENU_SELECT);
            return;
        }
    }

    // ==============================
    // 入力待ち
    // ==============================
    if (g_KeyConfigState == KEYCFG_WAIT_KEY)
    {
        // OK誤爆防止（200ms）
        if (GetNowCount() - g_WaitStartTime < 200)
            return;

        InputKey target = (InputKey)(1 << g_ConfigActionIndex);

        // ---------- Keyboard ----------
        if (g_KeyConfigDevice == DEVICE_KEYBOARD)
        {
            for (int key = 0; key < 256; key++)
            {
                if (CheckHitKey(key))
                {
                    BindKey bind{ DEVICE_KEYBOARD, key };

                    // 対応キーか？
                    if (!ControlConfig::IsSupportedBind(bind))
                    {
                        // 非対応キー → メッセージ表示用状態へ
                        g_KeyConfigState = KEYCFG_UNSUPPORTED_KEY;
                        PlaySE(SE_MENU_CANCEL);
                        return;
                    }

                    InputKey other = ControlConfig::FindActionByBind(bind);

                    if (other == INPUT_NONE || other == target)
                    {
                        // 重複なし or 自分自身
                        ControlConfig::Assign(target, bind);
                        g_IsKeyConfigChanged = true;
                        PlaySE(SE_MENU_OK);
                        g_KeyConfigState = KEYCFG_NORMAL;
                    }
                    else
                    {
                        // 重複あり → 確認へ
                        g_PendingTarget = target;
                        g_PendingOther = other;
                        g_PendingBind = bind;
                        g_KeyConfigState = KEYCFG_CONFIRM_SWAP;
                        PlaySE(SE_MENU_SELECT);
                    }
                    return;
                }
            }
        }

        // ---------- Pad ----------
        if (g_KeyConfigDevice == DEVICE_PAD)
        {
            int pad = GetJoypadInputState(DX_INPUT_PAD1);
            if (pad != 0)
            {
                int padKey = pad & -pad;
                BindKey bind{ DEVICE_PAD, padKey };

                if (!ControlConfig::IsSupportedBind(bind))
                {
                    g_KeyConfigState = KEYCFG_UNSUPPORTED_KEY;
                    PlaySE(SE_MENU_CANCEL);
                    return;
                }

                InputKey other = ControlConfig::FindActionByBind(bind);

                if (other == INPUT_NONE || other == target)
                {
                    ControlConfig::Assign(target, bind);
                    g_IsKeyConfigChanged = true;
                    PlaySE(SE_MENU_OK);
                    g_KeyConfigState = KEYCFG_NORMAL;
                }
                else
                {
                    g_PendingTarget = target;
                    g_PendingOther = other;
                    g_PendingBind = bind;
                    g_KeyConfigState = KEYCFG_CONFIRM_SWAP;
                    PlaySE(SE_MENU_SELECT);
                }
                return;
            }
        }

        return;
    }

    // ==============================
    // 入れ替え確認中
    // ==============================
    if (g_KeyConfigState == KEYCFG_CONFIRM_SWAP)
    {
        // 選択切り替え
        if (IsTriggerKey(KEY_LEFT) || IsTriggerKey(KEY_UP))
        {
            g_ConfirmIndex = 0;
            PlaySE(SE_MENU_MOVE);
        }
        if (IsTriggerKey(KEY_RIGHT) || IsTriggerKey(KEY_DOWN))
        {
            g_ConfirmIndex = 1;
            PlaySE(SE_MENU_MOVE);
        }

        // 決定
        if (IsTriggerKey(KEY_OK))
        {
            if (g_ConfirmIndex == 0)
            {
                // YES → 入れ替え
                ControlConfig::Swap(g_PendingTarget, g_PendingOther, g_KeyConfigDevice);
                g_IsKeyConfigChanged = true;
                PlaySE(SE_MENU_OK);
            }
            else
            {
                // NO
                PlaySE(SE_MENU_CANCEL);
            }

            g_KeyConfigState = KEYCFG_NORMAL;
            return;
        }

        // CANCEL は常にキャンセル扱い
        if (IsTriggerKey(KEY_CANCEL))
        {
            PlaySE(SE_MENU_CANCEL);
            g_KeyConfigState = KEYCFG_NORMAL;
            return;
        }

        return;
    }

    // ==============================
    // 非対応キー警告中
    // ==============================
    if (g_KeyConfigState == KEYCFG_UNSUPPORTED_KEY)
    {
        // CANCEL で入力待ちに戻る
        if (IsTriggerKey(KEY_CANCEL))
        {
            g_KeyConfigState = KEYCFG_WAIT_KEY;
            g_WaitStartTime = GetNowCount(); // 誤爆防止を再適用
            PlaySE(SE_MENU_CANCEL);
        }
        return;
    }

    // 通常操作
    if (IsTriggerKey(KEY_UP))
    {
        g_ConfigActionIndex =
            (g_ConfigActionIndex - 1 + ACTION_MAX) % ACTION_MAX;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsTriggerKey(KEY_DOWN))
    {
        g_ConfigActionIndex =
            (g_ConfigActionIndex + 1) % ACTION_MAX;
        PlaySE(SE_MENU_MOVE);
    }

    if (IsInputOKGuarded())
    {
        g_KeyConfigState = KEYCFG_WAIT_KEY;
        g_WaitStartTime = GetNowCount();
        PlaySE(SE_MENU_OK);
    }

    if (g_KeyConfigState == KEYCFG_WAIT_KEY)
    {
        if (IsTriggerKey(KEY_CANCEL))
        {
            g_KeyConfigState = KEYCFG_NORMAL;
            PlaySE(SE_MENU_CANCEL);
        }
        return;
    }

    if (IsTriggerKey(KEY_CANCEL))
    {
        g_OptionState = OPTION_STATE_CATEGORY_SELECT;
        PlaySE(SE_MENU_OK);
    }
}

static void UpdateControlConfigOverlay()
{
    UpdateControlConfig();

    // キーの入れ替え確認中はここで何もしない
    if (g_KeyConfigState != KEYCFG_NORMAL)
        return;

    if (IsTriggerKey(KEY_CANCEL))
    {
        if (g_IsKeyConfigChanged)
        {
            GameSetting::Save();
            g_IsKeyConfigChanged = false;
        }
        g_IsControlConfigOpen = false;
        g_OptionState = OPTION_STATE_CATEGORY_SELECT;
        PlaySE(SE_MENU_CANCEL);
    }
}

static void UpdateItemAdjust()
{
    // 操作説明 Overlay
    if (g_CategoryIndex == OPTION_CATEGORY_CONTROL_HELP &&
        g_IsControlHelpOpen)
    {
        UpdateControlHelp();
        return;
    }

    //　キーコンフィグ Overlay
    if (g_CategoryIndex == OPTION_CATEGORY_CONTROL_CONFIG &&
        g_IsControlConfigOpen)
    {
        UpdateControlConfigOverlay();
        return;
    }

    if (IsTriggerKey(KEY_CANCEL))
    {
        g_OptionState = OPTION_STATE_CATEGORY_SELECT;
        PlaySE(SE_MENU_CANCEL);
        return;
    }

    switch (g_CategoryIndex)
    {
    case OPTION_CATEGORY_SOUND:
        UpdateSoundOption();
        break;

    case OPTION_CATEGORY_CONTROL_CONFIG:
        UpdateControlConfig();
        break;

    case OPTION_CATEGORY_CONTROL_HELP:
        //上記のもので問題なかったため、コメントアウト中
        //UpdateControlHelp();
        break;
    default:
        if (IsTriggerKey(KEY_CANCEL))
        {
            g_OptionState = OPTION_STATE_CATEGORY_SELECT;
            PlaySE(SE_MENU_OK);
        }
        break;
    }
}

void UpdateOption()
{
    if (!g_IsOptionOpen) return;

    switch (g_OptionState)
    {
    case OPTION_STATE_CATEGORY_SELECT:
        UpdateCategorySelect();
        break;

    case OPTION_STATE_ITEM_ADJUST:
        UpdateItemAdjust();
        break;
    }
}

//==================================================
// Draw
//==================================================

static void DrawCategory(int x, int y)
{
    for (int i = 0; i < OPTION_CATEGORY_MAX; i++)
    {
        int color =
            (g_OptionState == OPTION_STATE_CATEGORY_SELECT && i == g_CategoryIndex)
            ? GetColor(255, 255, 0)
            : GetColor(255, 255, 255);

        DrawString(x, y + i * 40, g_CategoryNames[i], color);
    }
}

static void DrawSoundOption(int x, int y)
{
    int colorBGM = (g_ItemIndex == 0) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
    int colorSE = (g_ItemIndex == 1) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);

    DrawFormatString(x, y, colorBGM, "BGM 音量 : %3d", g_BGMVolumeLevel);
    DrawFormatString(x, y + 40, colorSE, "SE  音量 : %3d", g_SEVolumeLevel);
}

// 操作説明の専用オーバーレイ
static void DrawControlHelpOverlay()
{
    int winW = 900;
    int winH = 500;
    int x = SCREEN_WIDTH / 2 - winW / 2;
    int y = SCREEN_HEIGHT / 2 - winH / 2;

    DrawBox(x, y, x + winW, y + winH, GetColor(20, 20, 20), TRUE);
    DrawBox(x, y, x + winW, y + winH, GetColor(200, 200, 200), FALSE);

    // 左リスト
    int listX = x + 20;
    int listY = y + 40;

    for (int i = 0; i < g_ControlHelpCount; i++)
    {
        int color = (i == g_HelpItemIndex)
            ? GetColor(255, 255, 0)
            : GetColor(255, 255, 255);

        DrawString(listX, listY + i * 30, g_ControlHelpList[i].name, color);
    }

    // 右説明
    int descX = x + 250;
    int descY = y + 40;

    DrawString(descX, descY,
        g_ControlHelpList[g_HelpItemIndex].description,
        GetColor(220, 220, 220));
}

// キーコンフィグ用のオーバーレイ描画
static void DrawControlConfigOverlay()
{
    int winW = 900;
    int winH = 520;
    int x = SCREEN_WIDTH / 2 - winW / 2;
    int y = SCREEN_HEIGHT / 2 - winH / 2;

    DrawBox(x, y, x + winW, y + winH, GetColor(20, 20, 20), TRUE);
    DrawBox(x, y, x + winW, y + winH, GetColor(200, 200, 200), FALSE);

    DrawFormatString(
        x + winW - 220,
        y + 20,
        GetColor(200, 200, 200),
        "[ %s ]  ←→ 切替",
        (g_KeyConfigDevice == DEVICE_KEYBOARD) ? "Keyboard" : "Pad"
    );

    DrawString(x + 20, y + 15, "キーコンフィグ", GetColor(255, 255, 0));

    int listX = x + 30;
    int listY = y + 60;

    for (int i = 0; i < ACTION_MAX; i++)
    {
        InputKey key = (InputKey)(1 << i);
        BindKey bind = ControlConfig::GetBind(key, g_KeyConfigDevice);

        int color = (i == g_ConfigActionIndex)
            ? GetColor(255, 255, 0)
            : GetColor(255, 255, 255);

        DrawFormatString(
            listX,
            listY + i * 28,
            color,
            "%-12s : %s",
            g_ActionNames[i],
            ControlConfig::GetKeyName(bind).c_str()
        );
    }

    if (g_KeyConfigState == KEYCFG_WAIT_KEY)
    {
        DrawString(
            listX,
            listY + ACTION_MAX * 28 + 20,
            "新しいキーを入力してください",
            GetColor(255, 120, 120)
        );
    }
    else if (g_KeyConfigState == KEYCFG_UNSUPPORTED_KEY)
    {
        DrawString(
            listX,
            listY + ACTION_MAX * 28 + 20,
            "このキーは対応していません",
            GetColor(255, 120, 120)
        );

        DrawString(
            listX + 300,
            listY + ACTION_MAX * 28 + 20,
            "CANCELで戻った後、再度別のキーを入力してください",
            GetColor(200, 200, 200)
        );
    }
    else
    {
        DrawString(
            listX,
            y + winH - 30,
            "↑↓ : 選択 / OK : 変更 / CANCEL : 戻る",
            GetColor(180, 180, 180)
        );
    }
}

// キーコンフィグですでに割り当てられているキーに対しての追加オーバーレイ
static void DrawKeyConfigConfirmOverlay()
{
    int winW = 600;
    int winH = 220;
    int x = SCREEN_WIDTH / 2 - winW / 2;
    int y = SCREEN_HEIGHT / 2 - winH / 2;

    DrawBox(x, y, x + winW, y + winH, GetColor(30, 30, 30), TRUE);
    DrawBox(x, y, x + winW, y + winH, GetColor(200, 200, 200), FALSE);

    int targetIndex = GetActionIndexFromInputKey(g_PendingTarget);
    int otherIndex = GetActionIndexFromInputKey(g_PendingOther);

    const char* targetName =
        (targetIndex >= 0) ? g_ActionNames[targetIndex] : "不明";

    const char* otherName =
        (otherIndex >= 0) ? g_ActionNames[otherIndex] : "不明";

    int yesColor = (g_ConfirmIndex == 0)
        ? GetColor(255, 255, 0)
        : GetColor(180, 180, 180);

    int noColor = (g_ConfirmIndex == 1)
        ? GetColor(255, 255, 0)
        : GetColor(180, 180, 180);

    DrawFormatString(
        x + 30,
        y + 40,
        GetColor(255, 255, 255),
        "このキーはすでに「%s」に割り当てられています。",
        otherName
    );

    DrawFormatString(
        x + 30,
        y + 80,
        GetColor(255, 255, 255),
        "「%s」と入れ替えますか？",
        targetName
    );

    DrawString(x + 120, y + 150, "はい", yesColor);
    DrawString(x + 360, y + 150, "いいえ", noColor);
}

//すべてのDrawのまとめ
void DrawOption()
{
    if (!g_IsOptionOpen) return;

    // 背景暗転
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    int windowW = 1000;
    int windowH = 600;

    int cx = SCREEN_WIDTH / 2 - windowW / 2;
    int cy = SCREEN_HEIGHT / 2 - windowH / 2;

    // 左タブ
    int tabW = 250;

    // 右内容
    int contentW = windowW - tabW - 20;

    // 外枠
    DrawBox(cx, cy, cx + windowW, cy + windowH, GetColor(30, 30, 30), TRUE);
    DrawBox(cx, cy, cx + windowW, cy + windowH, GetColor(180, 180, 180), FALSE);

    // タイトル
    DrawString(cx + windowW / 2 - 40, cy + 20, "OPTION", GetColor(255, 255, 0));

    int tabX = cx + 20;
    int tabY = cy + 70;

    DrawBox(
        tabX,
        tabY,
        tabX + tabW,
        cy + windowH - 20,
        GetColor(20, 20, 20),
        TRUE
    );    // 左：カテゴリ
    DrawCategory(tabX + 20, tabY + 20);

    int contentX = tabX + tabW + 20;
    int contentY = tabY;

    DrawBox(
        contentX,
        contentY,
        cx + windowW - 20,
        cy + windowH - 20,
        GetColor(25, 25, 25),
        TRUE
    );
    // 右：内容
    switch (g_CategoryIndex)
    {
    case OPTION_CATEGORY_SOUND:
        DrawSoundOption(contentX + 20, contentY + 20);
        break;

    case OPTION_CATEGORY_CONTROL_CONFIG:
        DrawString(contentX + 20, contentY + 20,
            "キーコンフィグ（ボタン配置）を設定できます",
            GetColor(180, 180, 180));
        if (g_IsControlConfigOpen)
        {
            DrawControlConfigOverlay();

            if (g_KeyConfigState == KEYCFG_CONFIRM_SWAP)
            {
                DrawKeyConfigConfirmOverlay();
            }
        }
        break;

    case OPTION_CATEGORY_CONTROL_HELP:
        DrawString(contentX + 20, contentY + 20,
            "各種、操作説明を表示します",
            GetColor(180, 180, 180));
        if (g_IsControlHelpOpen)
        {
            DrawControlHelpOverlay();
        }
        break;
    }

}
void FinOptionScene()
{
}
*/