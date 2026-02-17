#include "GameSetting.h"
#include <fstream>
#include <sstream>
#include "../Sound/Sound.h"

//==============================
// 内部データ
//==============================

static int g_BGMVolume = 5;
static int g_SEVolume = 5;

static const char* FILE_NAME = "GameSetting.txt";

//==============================
// Load / Save
//==============================

void GameSetting::Load()
{
    g_BGMVolume = 5;
    g_SEVolume = 5;

    ControlConfig::InitDefault();

    std::ifstream ifs(FILE_NAME);
    if (!ifs.is_open())
    {
        // デフォルトでも必ず反映
        SetBGMVolumeLevel(g_BGMVolume);
        SetSEVolumeLevel(g_SEVolume);
        return;
    }

    std::string tag;
    while (ifs >> tag)
    {
        if (tag == "BGM")
        {
            ifs >> g_BGMVolume;
        }
        else if (tag == "SE")
        {
            ifs >> g_SEVolume;
        }
        else if (tag == "KEY")
        {
            int key;
            int device;
            int keyCode;

            ifs >> key >> device >> keyCode;

            BindKey bind;
            bind.device = (DeviceType)device;
            bind.keyCode = keyCode;

            ControlConfig::SetBind((InputKey)key, bind.device, bind);
        }
    }
    SetBGMVolumeLevel(g_BGMVolume);
    SetSEVolumeLevel(g_SEVolume);
}

void GameSetting::Save()
{
    std::ofstream ofs(FILE_NAME, std::ios::trunc);
    if (!ofs.is_open())
        return;

    ofs << "BGM " << g_BGMVolume << "\n";
    ofs << "SE " << g_SEVolume << "\n";

    for (int i = 0; i < ACTION_MAX; i++)
    {
        InputKey key = (InputKey)(1 << i);

        // Keyboard
        BindKey kb = ControlConfig::GetBind(key, DEVICE_KEYBOARD);
        ofs << "KEY " << key << " " << kb.device << " " << kb.keyCode << "\n";

        // Pad
        BindKey pad = ControlConfig::GetBind(key, DEVICE_PAD);
        ofs << "KEY " << key << " " << pad.device << " " << pad.keyCode << "\n";
    }
}

//==============================
// Sound
//==============================

void GameSetting::SetBGMVolume(int v)
{
    g_BGMVolume = (v < 0) ? 0 : (v > 10 ? 10 : v);
    SetBGMVolumeLevel(g_BGMVolume);
}
void GameSetting::SetSEVolume(int v)
{
    g_SEVolume = (v < 0) ? 0 : (v > 10 ? 10 : v);
    SetSEVolumeLevel(g_SEVolume);
}
int GameSetting::GetBGMVolume()
{
    return g_BGMVolume;
}

int GameSetting::GetSEVolume()
{
    return g_SEVolume;
}

void GameSetting::ApplyToControlConfig()
{
    // 現状は Load() で直接反映済み
}