#pragma once

enum class FadeState
{
    None,
    FadeOut,
    FadeIn
};

enum class FadeType
{
    Title,
    Stage,
    Death
};

void InitFade();
void StartFadeOut(int duration);
void StartFadeIn(int duration);

void UpdateFade();
void DrawFade();

bool IsFading();
bool IsFadeOutFinished();
bool IsFadeFinished();

void StartFadeOutEx(FadeType type);
void StartFadeInEx(FadeType type);