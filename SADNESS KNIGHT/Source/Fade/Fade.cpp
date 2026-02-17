#include "DxLib.h"
#include "Fade.h"

//
//‰¼’u‚«‚âB•Ï‚¦‚½‚«‚á•Ï‚¦‚ÈBâW‰@‰Æ“–Žå‚ÍƒƒC‚â
//
static FadeState g_FadeState = FadeState::None;
static int g_FadeTimer = 0;
static int g_FadeDuration = 0;
static int g_FadeAlpha = 0;

void InitFade()
{
    g_FadeState = FadeState::None;
    g_FadeTimer = 0;
    g_FadeAlpha = 0;
}

void StartFadeOut(int duration)
{
    g_FadeState = FadeState::FadeOut;
    g_FadeDuration = duration;
    g_FadeTimer = 0;
}

void StartFadeIn(int duration)
{
    g_FadeState = FadeState::FadeIn;
    g_FadeDuration = duration;
    g_FadeTimer = duration;
}

void UpdateFade()
{
    if (g_FadeState == FadeState::None)
        return;

    if (g_FadeState == FadeState::FadeOut)
    {
        g_FadeTimer++;
        if (g_FadeTimer >= g_FadeDuration)
        {
            g_FadeTimer = g_FadeDuration;
            g_FadeState = FadeState::None;
        }
    }
    else if (g_FadeState == FadeState::FadeIn)
    {
        g_FadeTimer--;
        if (g_FadeTimer <= 0)
        {
            g_FadeTimer = 0;
            g_FadeState = FadeState::None;
        }
    }

    g_FadeAlpha = 255 * g_FadeTimer / g_FadeDuration;
}

void DrawFade()
{
    if (g_FadeAlpha <= 0) return;

    int w, h;
    GetDrawScreenSize(&w, &h);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, g_FadeAlpha);
    DrawBox(0, 0, w, h, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

bool IsFading()
{
    return g_FadeState != FadeState::None;
}

bool IsFadeOutFinished()
{
    return g_FadeState == FadeState::None && g_FadeTimer >= g_FadeDuration;
}

bool IsFadeFinished()
{
    return g_FadeState == FadeState::None && g_FadeTimer <= 0;
}

static int GetFadeDuration(FadeType type)
{
    switch (type)
    {
    case FadeType::Title:
        return 10;   // ˆêu
    case FadeType::Stage:
        return 50;   // ’Êí‘JˆÚ
    case FadeType::Death:
        return 120;  // d‚¢‰‰o
    }
    return 30;
}

void StartFadeOutEx(FadeType type)
{
    StartFadeOut(GetFadeDuration(type));
}

void StartFadeInEx(FadeType type)
{
    StartFadeIn(GetFadeDuration(type));
}