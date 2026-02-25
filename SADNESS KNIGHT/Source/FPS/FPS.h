#pragma once

// 関数のプロトタイプ宣言
void InitFPS();
void UpdateFPS();
void DrawFPS();

// 1フレームが速すぎたときの待機関数
void FPSWait();

// デルタタイムを取得（秒単位）
double GetDeltaTime();

// 現在のFPSを取得
float GetCurrentFPS();

// フレーム時間（ミリ秒）の平均を取得
double GetAverageFrameTimeMs();

// フレーム時間（ミリ秒）の最小値を取得
double GetMinFrameTimeMs();

// フレーム時間（ミリ秒）の最大値を取得
double GetMaxFrameTimeMs();
