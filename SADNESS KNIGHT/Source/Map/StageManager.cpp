#include "StageManager.h"
#include "../Map/MapManager.h"
#include "../Player/Player.h"
#include <string>
#include <vector>
#include "../Camera/Camera.h"
#include "../Scene/Play/Play.h"
#include "../Fade/Fade.h"
#include "../Map/Checkpoint/Checkpoint.h"
//ステージの背景
int g_BackgroundFar = -1; // 遠景
int g_BackgroundMid = -1; // 中景
int g_Foreground = -1;    // 前景

// -------------------------------------
// 内部構造
// -------------------------------------
struct ExitPoint
{
	int x, y;
	char target[64];
	float spawnX, spawnY;
};

static std::string g_CurrentStage = "forest_1";
static std::vector<ExitPoint> g_ExitPoints;

// -------------------------------------
// ステージ基本情報
// -------------------------------------
const char* GetCurrentStageName()
{
	return g_CurrentStage.c_str();
}

void SetCurrentStage(const char* stageName)
{
	g_CurrentStage = stageName;
}

// -------------------------------------
// ステージ読み込み
// -------------------------------------
void InitStage()
{
	InitMap();
}

void FinStage()
{
	FinMap();
	g_ExitPoints.clear();
	FinCheckpoint();
}

// -------------------------------------
// Exit情報のロード
// -------------------------------------
static void LoadExitInfo(const char* stageName)
{
	g_ExitPoints.clear();

	char path[256];
	sprintf_s(path, "Data/Map/%s/%s/ExitInfo.txt", GetFieldName(stageName), stageName);

	FILE* fp = nullptr;
	if (fopen_s(&fp, path, "r") != 0 || !fp)
	{
		printf_s("ExitInfo not found for %s\n", stageName);
		return;
	}

	ExitPoint ep;
	while (fscanf_s(fp, "%d %d %s %f %f",
		&ep.x, &ep.y,
		ep.target, (unsigned)_countof(ep.target),
		&ep.spawnX, &ep.spawnY) == 5)
	{
		// Excel → ゲーム座標へ変換
		ep.x -= 1;
		ep.y -= 1;

		g_ExitPoints.push_back(ep);
		printf_s("[Exit] (%d, %d) -> %s (%.1f, %.1f)\n",
			ep.x, ep.y, ep.target, ep.spawnX, ep.spawnY);
	}
	fclose(fp);
}

// -------------------------------------
// ステージ切り替え
// -------------------------------------
void LoadStage(const char* stageName, float playerSpawnX, float playerSpawnY)
{
	// 既にロード中なら無視（再入防止）
	static bool s_loading = false;
	if (s_loading) return;
	s_loading = true;

	StartFadeOutEx(FadeType::Stage);
	SetCurrentStage(stageName);
	//ClearAllEnemies();
	FinMap();
	DestroyCollidersByTag(ColliderTag::Block);
	DestroyCollidersByTag(ColliderTag::Exit);
	DestroyCollidersByTag(ColliderTag::SemiSolid);
	LoadMap(stageName);
	FinCheckpoint();
	InitCheckpoint(stageName);
	LoadBackground(stageName);
	StartMap();
	LoadExitInfo(stageName);
	//OnStageLoaded();
	
		// 敵スポーンCSVを読み込む
	char csvPath[256];
	sprintf_s(csvPath, "Data/EnemySpawn/%s_Spawn.csv", GetCurrentStageName());
	// ログ：実際に開くパスを出す（画面上）
	{
		char dbg[256];
		sprintf_s(dbg, "[SpawnCSV] Attempting load: %s", csvPath);
		AddDebugLog(dbg);
	}
	/*
	if (!g_EnemySpawnSystem.LoadSpawnCSV(csvPath))
	{
		char buf[256];
		sprintf_s(buf, "[SpawnCSV] LOAD FAILED : %s", csvPath);
		AddDebugLog(buf);
	}
	else
	{
		char buf[256];
		sprintf_s(buf, "[SpawnCSV] LOAD SUCCESS : %s", csvPath);
		AddDebugLog(buf);
	}
	*/

	// プレイヤーのスポーン位置を反映
	PlayerData& player = GetPlayerData();
	player.posX = playerSpawnX;
	player.posY = playerSpawnY;
	player.velocityX = 0.0f;
	player.velocityY = 0.0f;
	player.isGrounded = false;
	player.jumpCount = 0;

	StartFadeInEx(FadeType::Stage);

	s_loading = false; // ロード終了（ガード解除）
}

void ReloadStage()
{
	PlayerData player = GetPlayerData();
	LoadStage(g_CurrentStage.c_str(), player.posX, player.posY);
}

// -------------------------------------
// ExitBlockヒット時の処理
// -------------------------------------
void HandleStageExit(const MapChipData& exitBlock)
{
	DrawFormatString(20, 200, GetColor(255, 255, 0),
		"Touched Exit Block %d %d",
		exitBlock.xIndex,
		exitBlock.yIndex);

	for (auto& e : g_ExitPoints)
	{
		DrawFormatString(20, 220, GetColor(0, 255, 255),
			"ExitInfo %d %d -> %s",
			e.x, e.y, e.target);

		if (exitBlock.xIndex == e.x && exitBlock.yIndex == e.y)
		{
			DrawFormatString(20, 240, GetColor(255, 0, 0),
				"EXIT MATCH FOUND");

			LoadStage(e.target, e.spawnX, e.spawnY);
			return;
		}
	}

	DrawFormatString(20, 260, GetColor(255, 0, 0),
		"NO EXIT MATCH");
}

// -------------------------------------
// フェード処理
// -------------------------------------
void FadeOut(int frame)
{
	StartFadeOut(frame);
	/*const int step = 8;
	for (int b = 255; b >= 0; b -= step)
	{
		SetDrawBright(b, b, b);
		ClearDrawScreen();
		DrawBackgroundFar();
		DrawBackgroundMid();
		DrawMap();
		DrawPlayer();
		DrawForeground();
		ScreenFlip();
		Sleep(ms / (255 / step + 1));
	}*/
}

void FadeIn(int frame)
{
	StartFadeIn(frame);
	/*const int step = 8;
	for (int b = 0; b <= 255; b += step)
	{
		SetDrawBright(b, b, b);
		ClearDrawScreen();
		DrawBackgroundFar();
		DrawBackgroundMid();
		DrawMap();
		DrawPlayer();
		DrawForeground();
		ScreenFlip();
		Sleep(ms / (255 / step + 1));
	}
	SetDrawBright(255, 255, 255);*/
}

void LoadBackground(const char* stageName)
{
	const char* field = GetFieldName(stageName);

	char pathFar[256];
	char pathMid[256];
	char pathFore[256];

	sprintf_s(pathFar, "Data/Map/%s/%s/background_far.png", field, stageName);
	sprintf_s(pathMid, "Data/Map/%s/%s/background_mid.png", field, stageName);
	sprintf_s(pathFore, "Data/Map/%s/%s/foreground.png", field, stageName);

	if (g_BackgroundFar > 0) DeleteGraph(g_BackgroundFar);
	if (g_BackgroundMid > 0) DeleteGraph(g_BackgroundMid);
	if (g_Foreground > 0) DeleteGraph(g_Foreground);

	g_BackgroundFar = LoadGraph(pathFar);
	g_BackgroundMid = LoadGraph(pathMid);
	g_Foreground = LoadGraph(pathFore);
}

void DrawBackgroundLayer(int handle, const CameraData& camera, float scale, float parallax)
{
	int bgW, bgH;
	GetGraphSize(handle, &bgW, &bgH);

	float bx = -(camera.posX * parallax) * scale;
	float by = -(camera.posY * parallax) * scale;

	float drawW = bgW * scale;
	float drawH = bgH * scale;

	DrawExtendGraph(
		(int)bx, (int)by,
		(int)(bx + drawW),
		(int)(by + drawH),
		handle,
		TRUE
	);
}

void DrawMapOutsideMask()
{
	CameraData camera = GetCamera();
	float scale = camera.scale;

	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	// マップの表示範囲（スクリーン座標）
	float mapLeft = -camera.posX * scale;
	float mapTop = -camera.posY * scale;
	float mapRight = mapLeft + GetMapWidth() * scale;
	float mapBottom = mapTop + GetMapHeight() * scale;

	int black = GetColor(0, 0, 0);

	// 上
	DrawBox(0, 0, screenW, (int)mapTop, black, TRUE);
	// 下
	DrawBox(0, (int)mapBottom, screenW, screenH, black, TRUE);
	// 左
	DrawBox(0, (int)mapTop, (int)mapLeft, (int)mapBottom, black, TRUE);
	// 右
	DrawBox((int)mapRight, (int)mapTop, screenW, (int)mapBottom, black, TRUE);
}

// 遠景レイヤー
void DrawBackgroundFar()
{
	if (g_BackgroundFar <= 0) return;

	CameraData camera = GetCamera();
	float scale = camera.scale;

	DrawBackgroundLayer(g_BackgroundFar, camera, scale, 0.15f);
}

// 中景
void DrawBackgroundMid()
{
	if (g_BackgroundMid <= 0) return;

	CameraData camera = GetCamera();
	float scale = camera.scale;

	DrawBackgroundLayer(g_BackgroundMid, camera, scale, 0.35f);
}

// 前景
void DrawForeground()
{
	if (g_Foreground <= 0) return;

	CameraData camera = GetCamera();
	float scale = camera.scale;

	DrawBackgroundLayer(g_Foreground, camera, scale, 1.1f);
}