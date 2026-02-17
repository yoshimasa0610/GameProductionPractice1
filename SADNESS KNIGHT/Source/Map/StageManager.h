#pragma once
#include "DxLib.h"
#include "../Player/Player.h"
#include "../Map/MapParameter.h"

// ===================================================
// ステージ全体管理：現在ステージ・遷移処理・フェード
// ===================================================

// 現在ステージのマップを取得・設計
const char* GetCurrentMapName();
void SetCurrentMap(const char* mapName);

// 現在のステージ名を取得／設定
const char* GetCurrentStageName();
void SetCurrentStage(const char* stageName);

// ステージ初期化と終了
void InitStage();
void FinStage();

// ステージ遷移
void LoadStage(const char* stageName, float playerSpawnX, float playerSpawnY);
void ReloadStage();  // 再読み込み（デバッグ用）

// ExitBlockとの接触処理（マップから呼び出される）
void HandleStageExit(const MapChipData& exitBlock);

// フェード制御（共通ユーティリティ）
void FadeOut(int frame = 30);
void FadeIn(int frame = 30);

void DrawMapOutsideMask();

//背景ロード
void LoadBackground(const char* mapFolder);

void DrawBackgroundFar();
void DrawBackgroundMid();
void DrawForeground();