#pragma once

// 関数のプロトタイプ宣言
void InitFPS();
void UpdateFPS();
void DrawFPS();

// 1フレームが速すぎたときの待機関数
void FPSWait();
