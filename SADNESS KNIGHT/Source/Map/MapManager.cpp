#include "MapManager.h"
#include "MapParameter.h"
#include "MapChip.h"
#include "StageManager.h"
#include "Block.h"
#include "../Player/Player.h"
#include <cstring>
#include <vector>
#include <string>
#include "../GameSetting/GameSetting.h"
#include "../Sound/Sound.h"
#include "../Money/MoneyDropSystem.h"
#include "../Collision/Collision.h" 
// ===================================================
// 内部データ
// ===================================================
static std::string g_DebugLog = "";
static char g_CurrentMap[128] = "forest_1";

#define CHECK_ROUND_NUM (2)  // プレイヤー周囲のチェック範囲


void SetDebugLog(const char* msg)
{
	g_DebugLog = msg;
}

void AddDebugLog(const char* msg)
{
	g_DebugLog += "\n";
	g_DebugLog += msg;
}

void DrawDebugLog()
{
	DrawFormatString(10, 500, GetColor(255, 255, 0), g_DebugLog.c_str());
}

// 下記の二つの破壊することのできるブロックかはこの関数を経由させる
inline bool IsBreakableBlock(MapChipType type)
{
	return type == BREAKABLE_WALL || type == BREAKABLE_STATUE;
}

// ===================================================
// マップ名関連
// ===================================================
const char* GetCurrentMapName()
{
	return g_CurrentMap;
}

int GetMapWidth()
{
	return (int)(g_MapChipXNum * MAP_CHIP_WIDTH);
}

int GetMapHeight()
{
	return (int)(g_MapChipYNum * MAP_CHIP_HEIGHT);
}

// ===================================================
// 初期化・終了
// ===================================================
void InitMap()
{
	InitBlock();
}

void FinMap()
{
	FreeMapChip();
	ClearAllBlocks();
}

// ===================================================
// マップ読み込み・生成
// ===================================================
const char* GetFieldName(const char* stageName)
{
	static char field[128];
	const char* underscore = strchr(stageName, '_');
	if (underscore)
	{
		size_t len = underscore - stageName;
		strncpy_s(field, stageName, len);
		field[len] = '\0';
	}
	else
	{
		strcpy_s(field, "misc");
	}
	return field;
}

void LoadMap(const char* mapFolder)
{
	char pathInfo[256];
	char pathBin[256];
	sprintf_s(pathInfo, "Data/Map/%s/%s/MapInfo.txt", GetFieldName(mapFolder), mapFolder);
	sprintf_s(pathBin, "Data/Map/%s/%s/Map.bin", GetFieldName(mapFolder), mapFolder);

	FILE* fp = nullptr;
	if (fopen_s(&fp, pathInfo, "r") == 0 && fp)
	{
		int xNum = 0, yNum = 0;
		fscanf_s(fp, "%d %d", &xNum, &yNum);
		fclose(fp);

		if (xNum > 0 && yNum > 0)
		{
			SetMapSize(xNum, yNum);
			printf_s("Map size set to %d x %d\n", xNum, yNum);
		}
	}

	LoadMapChipData(pathBin);
	LoadBlock();
}

void StartMap()
{
	CreateMap();
}

// ===================================================
// 描画
// ===================================================
void DrawMap()
{
	DrawBlock();
}

// ===================================================
// ステージ切り替え（直接呼ぶことは推奨しない）
// ===================================================
void ChangeMap(const char* mapFolder)
{
	strcpy_s(g_CurrentMap, sizeof(g_CurrentMap), mapFolder);
	SetCurrentStage(mapFolder);
	LoadMap(mapFolder);
	StartMap();
}

void ChangeMap(const char* mapFolder, float newPlayerX, float newPlayerY)
{
	strcpy_s(g_CurrentMap, sizeof(g_CurrentMap), mapFolder);
	LoadMap(mapFolder);
	StartMap();
	GetPlayerPos(newPlayerX, newPlayerY);

}

// ===================================================
// ExitBlock → StageManager に委譲
// ===================================================
void HandleExitBlock(const MapChipData& exitBlock)
{
	HandleStageExit(exitBlock);
}