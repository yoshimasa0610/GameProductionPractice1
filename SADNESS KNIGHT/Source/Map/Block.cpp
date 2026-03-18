#include "Block.h"
#include "../Camera/Camera.h"
#include "DxLib.h"
#include <corecrt_math.h>

//============================================================
// ブロック（マップチップ）システム
//============================================================
// このファイルはマップの地形を管理します
// 通常は MapManager や StageManager から呼ばれます
//============================================================

// グローバル変数
BlockData g_Blocks[BLOCK_MAX] = { 0 };              // 全ブロックデータ
int g_BlockHandle[BLOCK_TYPE_MAX] = { 0 };         // ブロック種別ごとの画像
int g_NormalTileHandles[49];                       // 通常ブロックのタイル画像 (7x7)
// アイテムや装飾など、ブロック以外のプロック用ハンドル（必要に応じて増やす）
int g_PropHandles[32];
// オブジェクト全般のシート（必要に応じて増やす）
int objectSheet;
int g_SemiSolidHandles[4];

// visual(0～15) → タイルシート番号に変換
int g_AutoTileTable[16] =
{
	24, // 0000  単体
	17, // 0001  上
	3,  // 0010  下
	10, // 0011  上下
	23, // 0100  左
	16, // 0101  上左
	2,  // 0110  下左
	9,  // 0111  上下左
	21, // 1000  右
	14, // 1001  上右
	0,  // 1010  下右
	7,  // 1011  上下右
	22, // 1100  左右
	15, // 1101  上左右
	1,  // 1110  下左右
	8   // 1111  全方向
};

//============================================================
// 初期化：ブロックシステムの初期化
//============================================================
void InitBlock()
{
	BlockData* block = g_Blocks;
	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		block->active = false;
		block->pos = VGet(0.0f, 0.0f, 0.0f);
		block->type = MAP_CHIP_NONE;
		block->handle = 0;
	}
}

//============================================================
// リソース読み込み：ブロック画像の読み込み
//============================================================
void LoadBlock()
{
	// 通常ブロック用のタイルシート読み込み（7x7の49枚）
	LoadDivGraph(
		"Data/Map/Normal_Sheet.png",
		49,        // 総数
		7,         // 横
		7,         // 縦
		512,       // 1タイル幅
		512,       // 1タイル高さ
		g_NormalTileHandles
	);

	// シートを1枚ロード
	objectSheet = LoadGraph("Data/Map/Object.png");

	// 木箱
	g_PropHandles[0] = DerivationGraph(
		42, 20,
		45, 45,
		objectSheet
	);

	// 樽
	g_PropHandles[1] = DerivationGraph(
		195, 29,
		27, 35,
		objectSheet
	);

	// ツボ
	g_PropHandles[2] = DerivationGraph(
		195, 29,
		27, 35,
		objectSheet
	);

	// 石像
	g_PropHandles[3] = DerivationGraph(
		700, 115,
		35, 77,
		objectSheet
	);

	// 単体
	g_SemiSolidHandles[0] = DerivationGraph(
		60, 382,
		32, 32,
		objectSheet
	);

	// 左端
	g_SemiSolidHandles[1] = DerivationGraph(
		92, 382,
		32, 32,
		objectSheet
	);

	// 中央
	g_SemiSolidHandles[2] = DerivationGraph(
		124, 382,
		32, 32,
		objectSheet
	);

	// 右端
	g_SemiSolidHandles[3] = DerivationGraph(
		156, 382,
		32, 32,
		objectSheet
	);
}

//============================================================
// 開始処理（現在未使用）
//============================================================
void StartBlock()
{
}

//============================================================
// 更新処理（現在未使用）
//============================================================
void StepBlock()
{
}

//============================================================
// 描画：全ブロックの描画
//============================================================
// カメラ座標に合わせて画面内のブロックのみ描画
//============================================================
void DrawBlock()
{
	CameraData camera = GetCamera();
	float cameraScale = camera.scale;

	const float DRAW_TILE_SIZE = 32.0f;     // 描画サイズ（ワールド座標）

	BlockData* block = g_Blocks;

	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		if (!block->active) continue;

		float worldX = block->pos.x;
		float worldY = block->pos.y;

		// カメラ座標に変換
		float bx = (worldX - camera.posX) * cameraScale;
		float by = (worldY - camera.posY) * cameraScale;

		// 画面外スキップ（最適化）
		if (bx + DRAW_TILE_SIZE * cameraScale < 0) continue;
		if (by + DRAW_TILE_SIZE * cameraScale < 0) continue;
		if (bx > 1600) continue;
		if (by > 900) continue;

		int drawX = (int)roundf(bx);
		int drawY = (int)roundf(by);
		float imageScale = DRAW_TILE_SIZE / (float)block->imageSize;
		float drawScale = cameraScale * imageScale;

		// ブロック描画
		DrawRotaGraph(
			drawX + (int)(DRAW_TILE_SIZE * cameraScale * 0.5f),
			drawY + (int)(DRAW_TILE_SIZE * cameraScale * 0.5f),
			drawScale,
			0.0,
			block->handle,
			TRUE
		);
	}
}

//============================================================
// 終了処理：画像リソースの解放
//============================================================
void FinBlock()
{
	// ブロック画像の解放
	for (int i = 0; i < BLOCK_TYPE_MAX; i++)
	{
		if (g_BlockHandle[i] > 0)
		{
			DeleteGraph(g_BlockHandle[i]);
			g_BlockHandle[i] = 0;
		}
	}

	// タイル画像の解放
	for (int i = 0; i < 49; i++)
	{
		if (g_NormalTileHandles[i] > 0)
			DeleteGraph(g_NormalTileHandles[i]);
	}
}

//============================================================
// 全ブロッククリア：全ブロックを非アクティブ化
//============================================================
// ステージ切り替え時などに使用
//============================================================
void ClearAllBlocks()
{
	for (int i = 0; i < BLOCK_MAX; i++)
	{
		g_Blocks[i].active = false;
		g_Blocks[i].handle = 0;
		g_Blocks[i].type = MAP_CHIP_NONE;
		g_Blocks[i].pos = VGet(0.0f, 0.0f, 0.0f);
	}
}

//============================================================
// ブロック作成：新しいブロックを配置
//============================================================
// 引数:
//   type   - ブロックの種類（NORMAL_BLOCK, SPIKE_BLOCKなど）
//   pos    - 配置座標（ワールド座標）
//   visual - 見た目の接続情報（オートタイル用）
// 戻り値:
//   作成されたブロックのポインタ（失敗時はnullptr）
//============================================================
BlockData* CreateBlock(MapChipType type, VECTOR pos, int visual)
{
	for (int i = 0; i < BLOCK_MAX; i++)
	{
		BlockData& block = g_Blocks[i];
		if (!block.active)
		{
			block.active = true;
			block.pos = pos;
			block.type = type;
			block.visual = visual;
			block.isSolid = (type != MAP_CHIP_NONE);

			// 通常ブロックの場合、オートタイル処理
			if (type == NORMAL_BLOCK)
			{
				int mask = visual & 0xF;
				int index = g_AutoTileTable[mask];

				// 全方向つながっている場合のみ内角判定
				if (mask == (NB_UP | NB_DOWN | NB_LEFT | NB_RIGHT))
				{
					bool hasUL = (visual & NB_UL) != 0;
					bool hasUR = (visual & NB_UR) != 0;
					bool hasDL = (visual & NB_DL) != 0;
					bool hasDR = (visual & NB_DR) != 0;

					// 内角タイル番号を設定
					if (!hasUL)
						index = 12;  // 左上内角
					else if (!hasUR)
						index = 11;  // 右上内角
					else if (!hasDL)
						index = 5;   // 左下内角
					else if (!hasDR)
						index = 4;   // 右下内角
				}

				block.tileIndex = index;
				block.handle = g_NormalTileHandles[index];
				block.imageSize = 512.0f;
			}
			else if (type == BREAKABLE_OBJECT)
			{
				block.handle = g_PropHandles[visual];

				if (visual == 0)      block.imageSize = 45.0f; // 木箱
				else if (visual == 1) block.imageSize = 35.0f; // 樽
				else if (visual == 2) block.imageSize = 35.0f; // 壺
				else if (visual == 3) block.imageSize = 77.0f; // 石像

				block.collisionW = MAP_CHIP_WIDTH;
				block.collisionH = MAP_CHIP_HEIGHT;
			}
			else if (type == SEMI_SOLID_BLOCK)
			{
				int index = visual;

				// 安全対策（念のため）
				if (index < 0 || index > 3) index = 0;

				block.handle = g_SemiSolidHandles[index];

				block.imageSize = 32.0f;
				block.collisionW = MAP_CHIP_WIDTH;
				block.collisionH = 8.0f;
			}
			else
			{
				// その他のブロックタイプ
				block.handle = g_BlockHandle[type];
				block.imageSize = 512.0f;
			}

			return &block;
		}
	}
	return nullptr;
}



BlockData* GetBlocks()
{
	return g_Blocks;
}


