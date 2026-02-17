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


void CheckMapPlayerCollision()
{
	PlayerData player = GetPlayerData();

	// プレイヤーの当たり矩形を計算（Player側の関数を使えるならそれを利用）
	float bx, by, bw, bh;
	CalcBoxCollision(player, bx, by, bw, bh);

	// 少しマージンを引いておく（端境界の丸め誤差対策）
	const float EPS = 0.001f;

	// 矩形の左上／右下ピクセル座標
	float rectLeft = bx + EPS;
	float rectTop = by + EPS;
	float rectRight = bx + bw - EPS;
	float rectBottom = by + bh - EPS;

	// タイルインデックスに変換（floorで安全に）
	int leftTile = (int)floorf(rectLeft / MAP_CHIP_WIDTH);
	int rightTile = (int)floorf(rectRight / MAP_CHIP_WIDTH);
	int topTile = (int)floorf(rectTop / MAP_CHIP_HEIGHT);
	int bottomTile = (int)floorf(rectBottom / MAP_CHIP_HEIGHT);

	// clamp to map bounds
	if (leftTile < 0) leftTile = 0;
	if (rightTile >= g_MapChipXNum) rightTile = g_MapChipXNum - 1;
	if (topTile < 0) topTile = 0;
	if (bottomTile >= g_MapChipYNum) bottomTile = g_MapChipYNum - 1;

	// まず X 軸（左右衝突）判定：各タイルを調べて X 軸用の当たり処理を呼ぶ
	for (int y = topTile; y <= bottomTile; y++)
	{
		for (int x = leftTile; x <= rightTile; x++)
		{
			MapChipData* mc = GetMapChipData(x, y);
			if (!mc) continue;

			switch (mc->mapChip)
			{
			case NORMAL_BLOCK:
			case BREAKABLE_WALL:
			case BREAKABLE_STATUE:
				PlayerHitNormalBlockX(*mc);
				break;

			case SPIKE_BLOCK:
				break;

			default:
				break;
			}
		}
	}

	// 次に Y 軸（上下衝突）判定：各タイルを調べて Y 軸用の当たり処理を呼ぶ
	for (int y = topTile; y <= bottomTile; y++)
	{
		for (int x = leftTile; x <= rightTile; x++)
		{
			MapChipData* mc = GetMapChipData(x, y);
			if (!mc) continue;

			switch (mc->mapChip)
			{
			case NORMAL_BLOCK:
			case BREAKABLE_WALL:
			case BREAKABLE_STATUE:
				PlayerHitNormalBlockY(*mc);
				break;

			case SPIKE_BLOCK:
				// ダメージ予定
				break;

			case EXIT_BLOCK:
				// 明示的に矩形同士で重なりを確認する（念のため）
			{
				// タイル矩形（ワールド）
				float tx = x * MAP_CHIP_WIDTH;
				float ty = y * MAP_CHIP_HEIGHT;
				float tw = MAP_CHIP_WIDTH;
				float th = MAP_CHIP_HEIGHT;

				// 簡易矩形当たり判定
				bool overlap =
					rectRight >= tx &&
					rectLeft <= (tx + tw) &&
					rectBottom >= ty &&
					rectTop <= (ty + th);

				if (overlap)
				{
					HandleExitBlock(*mc);
				}
				// プレイヤーの足元でEXIT_BLOCKに触れた場合のみ反応
				if (player.velocityY > 0 && player.posY + PLAYER_HEIGHT >= mc->yIndex * MAP_CHIP_HEIGHT)
				{
					HandleExitBlock(*mc);
				}
			}
			break;

			default:
				break;
			}
		}
	}
}

// プレイヤーの当たり判定と同様の方法で、マップチップと弾の当たり判定を行う
// まだ、弾を実装していないので一時コメントアウト
/*
void CheckMapBulletCollision()
{
	const auto& bullets = g_BulletManager.GetBullets();

	for (const auto& b : bullets)
	{
		if (!b || !b->IsAlive())
			continue;

		// ===== 弾の当たり矩形（中心 → 左上）=====
		float bx = b->GetX() - b->GetHitW() * 0.5f;
		float by = b->GetY() - b->GetHitH() * 0.5f;
		float bw = b->GetHitW();
		float bh = b->GetHitH();

		const float EPS = 0.001f;

		float rectLeft = bx + EPS;
		float rectTop = by + EPS;
		float rectRight = bx + bw - EPS;
		float rectBottom = by + bh - EPS;

		// ===== タイル範囲算出 =====
		int leftTile = (int)floorf(rectLeft / MAP_CHIP_WIDTH);
		int rightTile = (int)floorf(rectRight / MAP_CHIP_WIDTH);
		int topTile = (int)floorf(rectTop / MAP_CHIP_HEIGHT);
		int bottomTile = (int)floorf(rectBottom / MAP_CHIP_HEIGHT);

		// clamp
		if (leftTile < 0) leftTile = 0;
		if (rightTile >= g_MapChipXNum) rightTile = g_MapChipXNum - 1;
		if (topTile < 0) topTile = 0;
		if (bottomTile >= g_MapChipYNum) bottomTile = g_MapChipYNum - 1;

		// ===== タイルチェック =====
		for (int y = topTile; y <= bottomTile; ++y)
		{
			for (int x = leftTile; x <= rightTile; ++x)
			{
				MapChipData* mc = GetMapChipData(x, y);
				if (!mc) continue;

				switch (mc->mapChip)
				{
				case NORMAL_BLOCK:
				{
					// タイル矩形
					float tx = x * MAP_CHIP_WIDTH;
					float ty = y * MAP_CHIP_HEIGHT;

					bool hit =
						rectRight >= tx &&
						rectLeft <= tx + MAP_CHIP_WIDTH &&
						rectBottom >= ty &&
						rectTop <= ty + MAP_CHIP_HEIGHT;

					if (hit)
					{
						b->OnHitBlock();
						goto NEXT_BULLET; // この弾はもう処理しない
					}
				}
				break;

				case BREAKABLE_WALL:
				case BREAKABLE_STATUE:
				{
					// タイル矩形
					float tx = x * MAP_CHIP_WIDTH;
					float ty = y * MAP_CHIP_HEIGHT;

					bool hit =
						rectRight >= tx &&
						rectLeft <= tx + MAP_CHIP_WIDTH &&
						rectBottom >= ty &&
						rectTop <= ty + MAP_CHIP_HEIGHT;

					if (hit)
					{
						b->OnHitBlock();

						mc->hp--;
						if (mc->hp <= 0)
						{
							MapChipType destroyedType = (MapChipType)mc->mapChip;

							// 描画ブロックを消す
							if (mc->data)
							{
								mc->data->active = false;
								mc->data = nullptr;
							}

							mc->mapChip = MAP_CHIP_NONE;
							mc->isSolid = false;

							// ★ 石像だけお金を落とす
							if (destroyedType == BREAKABLE_STATUE)
							{
								int money = DecideStatueDropMoney();
								if (money > 0)
								{
									SpawnMoneyDrops(
										mc->xIndex * MAP_CHIP_WIDTH + MAP_CHIP_WIDTH * 0.5f,
										mc->yIndex * MAP_CHIP_HEIGHT + MAP_CHIP_HEIGHT * 0.5f,
										money
									);
								}
							}
						}
						goto NEXT_BULLET; // この弾はもう処理しない
					}
				}
				break;

				default:
					break;
				}
			}
		}

	NEXT_BULLET:
		continue;
	}
}
*/
// ===================================================
// ExitBlock → StageManager に委譲
// ===================================================
void HandleExitBlock(const MapChipData& exitBlock)
{
	HandleStageExit(exitBlock);
}