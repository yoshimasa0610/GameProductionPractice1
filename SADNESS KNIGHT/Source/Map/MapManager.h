#pragma once
#include "DxLib.h"
#include "MapChip.h"
#include "MapParameter.h"

// ===================================================
// MapManager : マップチップ管理・描画・当たり判定
// ===================================================

extern int g_MapChipXNum;
extern int g_MapChipYNum;
const char* GetFieldName(const char* stageName);

// 初期化／終了
void InitMap();
void FinMap();

// マップ読み込み／生成
void LoadMap(const char* mapFolder);
void StartMap();
void CreateMap();
void FreeMapChip();

// 描画
void DrawMap();

// マップサイズ関連
void SetMapSize(int xNum, int yNum);
int GetMapWidth();
int GetMapHeight();

// 現在のマップ名
const char* GetCurrentMapName();
void ChangeMap(const char* mapFolder);
void ChangeMap(const char* mapFolder, float newPlayerX, float newPlayerY);

// デバッグログ
void DrawDebugLog();
void AddDebugLog(const char* msg);
void SetDebugLog(const char* msg);

// ExitBlockを検出したときに呼び出す
void HandleExitBlock(const MapChipData& exitBlock);