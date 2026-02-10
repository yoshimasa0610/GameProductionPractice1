#include "DxLib.h"
#include "FPS.h"

// 平均を計算するタイミング
#define FPS_SAMPLE_NUM (60) // 60フレームに一度平均を計算する

// ゲームのFPS
#define FPS (60)

int g_StartTime;      // 測定開始時刻
int g_Count;          // カウンタ
float g_Fps;          // 現在のFPS

void InitFPS()
{
    g_StartTime = GetNowCount();
    g_Count = 0;
    g_Fps = 0;
}

void UpdateFPS()
{
    // 1フレーム目なら時刻を記憶
    if (g_Count == 0)
    {
        g_StartTime = GetNowCount();
    }

    // サンプル数と同じ回数フレームが回ったら平均を計算する
    if (g_Count == FPS_SAMPLE_NUM) {
        int time = GetNowCount();
        // かかった時間（ﾐﾘ秒）をサンプル数で割り平均とする（その値が現在のFPS値）
        g_Fps = 1000.f / ((time - g_StartTime) / (float)FPS_SAMPLE_NUM);
        g_Count = 0;
        g_StartTime = time;
    }
    g_Count++;

}

void DrawFPS()
{
}

void FPSWait()
{
    // かかった時間
    int takeTime = GetNowCount() - g_StartTime;

    // 待機時間
    // FPSの値から1フレームにかかってほしい時間（g_Count * 1000 / FPS）
    // 実際に経過した時間（takeTime）
    // 実際の時間が早すぎた場合は、その差分を待機時間とする
    int waitTime = g_Count * 1000 / FPS - takeTime;

    // 待機
    if (waitTime > 0)
    {
        Sleep(waitTime);
    }
}
