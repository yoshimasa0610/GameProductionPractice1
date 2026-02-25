#include "DxLib.h"
#include "FPS.h"

// 平均を計算するタイミング
#define FPS_SAMPLE_NUM (60) // 60フレームに一度平均を計算する

// ゲームのFPS（固定値）
#define TARGET_FPS (60)
#define TARGET_FRAME_TIME (1000.0 / TARGET_FPS) // 1フレームにかかるべき時間（ミリ秒）

int g_StartTime;      // 測定開始時刻
int g_FrameStartTime; // フレーム開始時刻
int g_Count;          // カウンタ
float g_Fps;          // 現在のFPS
double g_DeltaTime;   // デルタタイム（秒）
double g_FrameTimeSumMs;    // フレーム時間合計（ミリ秒）
double g_FrameTimeMinMs;    // フレーム時間最小（ミリ秒）
double g_FrameTimeMaxMs;    // フレーム時間最大（ミリ秒）
double g_FrameTimeAvgMs;    // フレーム時間平均（ミリ秒）
double g_FrameTimeMinMsCurrent; // 現在のサンプルの最小（ミリ秒）
double g_FrameTimeMaxMsCurrent; // 現在のサンプルの最大（ミリ秒）

void InitFPS()
{
    g_StartTime = GetNowCount();
    g_FrameStartTime = g_StartTime;
    g_Count = 0;
    g_Fps = TARGET_FPS;
    g_DeltaTime = 1.0 / TARGET_FPS;
    g_FrameTimeSumMs = 0.0;
    g_FrameTimeMinMs = TARGET_FRAME_TIME;
    g_FrameTimeMaxMs = TARGET_FRAME_TIME;
    g_FrameTimeAvgMs = TARGET_FRAME_TIME;
    g_FrameTimeMinMsCurrent = TARGET_FRAME_TIME;
    g_FrameTimeMaxMsCurrent = TARGET_FRAME_TIME;
    
    // DxLibの描画を垂直同期に合わせる（ティアリング防止）
    SetWaitVSyncFlag(TRUE);
}

void UpdateFPS()
{
    // 前フレームからの経過時間を計算（デルタタイム）
    int currentTime = GetNowCount();
    int elapsedTime = currentTime - g_FrameStartTime;
    
    // デルタタイムを秒単位で保存（ただし、異常値は補正）
    double frameTimeMs = TARGET_FRAME_TIME;
    if (elapsedTime > 0 && elapsedTime < 500) // 0.5秒以上かかった場合は異常
    {
        g_DeltaTime = elapsedTime / 1000.0;
        frameTimeMs = static_cast<double>(elapsedTime);
    }
    else
    {
        g_DeltaTime = 1.0 / TARGET_FPS; // 異常値の場合はターゲットFPSのデルタタイムを使用
    }
    
    g_FrameStartTime = currentTime;

    // サンプル数と同じ回数フレームが回ったら平均を計算する
    if (g_Count == FPS_SAMPLE_NUM) {
        g_Fps = 1000.f / ((currentTime - g_StartTime) / (float)FPS_SAMPLE_NUM);
        g_FrameTimeAvgMs = g_FrameTimeSumMs / FPS_SAMPLE_NUM;
        g_FrameTimeMinMs = g_FrameTimeMinMsCurrent;
        g_FrameTimeMaxMs = g_FrameTimeMaxMsCurrent;
        g_Count = 0;
        g_StartTime = currentTime;
        g_FrameTimeSumMs = 0.0;
    }

    // 1フレーム目なら時刻を記憶
    if (g_Count == 0)
    {
        g_StartTime = currentTime;
        g_FrameTimeMinMsCurrent = frameTimeMs;
        g_FrameTimeMaxMsCurrent = frameTimeMs;
    }
    else
    {
        if (frameTimeMs < g_FrameTimeMinMsCurrent)
        {
            g_FrameTimeMinMsCurrent = frameTimeMs;
        }
        if (frameTimeMs > g_FrameTimeMaxMsCurrent)
        {
            g_FrameTimeMaxMsCurrent = frameTimeMs;
        }
    }

    g_FrameTimeSumMs += frameTimeMs;
    g_Count++;
}

void DrawFPS()
{
    // FPS表示（デバッグ用）
    DrawFormatString(10, 10, GetColor(255, 255, 255), "FPS: %.1f", g_Fps);
    DrawFormatString(10, 30, GetColor(255, 255, 255), "DeltaTime: %.4f", g_DeltaTime);
    DrawFormatString(10, 50, GetColor(255, 255, 255), "Frame(ms) avg: %.2f", g_FrameTimeAvgMs);
    DrawFormatString(10, 70, GetColor(255, 255, 255), "Frame(ms) min: %.2f max: %.2f", g_FrameTimeMinMs, g_FrameTimeMaxMs);
}

void FPSWait()
{
    // 目標フレーム時間との差分を計算
    int currentTime = GetNowCount();
    double targetTime = g_Count * TARGET_FRAME_TIME;
    double actualTime = currentTime - g_StartTime;
    
    // 待機時間を計算
    int waitTime = (int)(targetTime - actualTime);

    // 待機時間が正の値なら待機
    if (waitTime > 0)
    {
        Sleep(waitTime);
    }
}

// デルタタイムを取得（秒単位）
double GetDeltaTime()
{
    return g_DeltaTime;
}

// 現在のFPSを取得
float GetCurrentFPS()
{
    return g_Fps;
}

double GetAverageFrameTimeMs()
{
    return g_FrameTimeAvgMs;
}

double GetMinFrameTimeMs()
{
    return g_FrameTimeMinMs;
}

double GetMaxFrameTimeMs()
{
    return g_FrameTimeMaxMs;
}
