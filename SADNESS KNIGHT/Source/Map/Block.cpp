#include "Block.h"
#include "../Camera/Camera.h"
#include "DxLib.h"
#include <corecrt_math.h>

BlockData g_Blocks[BLOCK_MAX] = { 0 };
int g_BlockHandle[BLOCK_TYPE_MAX] = { 0 };
// NORMAL_BLOCK 用の見た目別ハンドル
int g_NormalTileHandles[49];   // 7x7 = 49

// visual(0～15) → タイルシート番号変換
int g_AutoTileTable[16] =
{
	24, // 0000  単体
	17, // 0001  上
	3, // 0010  下
	10, // 0011  上下
	23, // 0100  左
	16, // 0101  上左
	2, // 0110  下左
	9, // 0111  上下左
	21, // 1000  右
	14, // 1001  上右
	0, // 1010  下右
	7, // 1011  上下右
	22, // 1100  左右
	15, // 1101  上左右
	1, // 1110  下左右
	8  // 1111  全方向
};

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

void LoadBlock()
{
	LoadDivGraph(
		"Data/Map/Normal_Sheet.png",
		49,        // 総数
		7,         // 横
		7,         // 縦
		512,       // 1タイル幅
		512,       // 1タイル高さ
		g_NormalTileHandles
	);

	g_BlockHandle[SPIKE_BLOCK] = LoadGraph("Data/Map/SpikeBlock.png");
}

void StartBlock()
{
}

void StepBlock()
{
}

void DrawBlock()
{
	CameraData camera = GetCamera();

	float cameraScale = camera.scale;

	const float DRAW_TILE_SIZE = 52.0f;     // ← 描画だけ52px
	float imageScale = DRAW_TILE_SIZE / 512.0f;

	BlockData* block = g_Blocks;

	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		if (!block->active) continue;

		float worldX = block->pos.x;
		float worldY = block->pos.y;

		float bx = (worldX - camera.posX) * cameraScale;
		float by = (worldY - camera.posY) * cameraScale;

		// 画面外スキップ
		if (bx + DRAW_TILE_SIZE * cameraScale < 0) continue;
		if (by + DRAW_TILE_SIZE * cameraScale < 0) continue;
		if (bx > 1600) continue;
		if (by > 900) continue;

		int drawX = (int)roundf(bx);
		int drawY = (int)roundf(by);

		float drawScale = cameraScale * imageScale;

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

void FinBlock()
{
	for (int i = 0; i < BLOCK_TYPE_MAX; i++)
	{
		if (g_BlockHandle[i] > 0)
		{
			DeleteGraph(g_BlockHandle[i]);
			g_BlockHandle[i] = 0;
		}
	}
	for (int i = 0; i < 49; i++)
	{
		if (g_NormalTileHandles[i] > 0)
			DeleteGraph(g_NormalTileHandles[i]);
	}
}

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

					// ここに内角タイル番号を書く
					if (!hasUL)
						index = 12;  // 左上内角
					else if (!hasUR)
						index = 11;  // 右上内角
					else if (!hasDL)
						index = 5;  // 左下内角
					else if (!hasDR)
						index = 4;  // 右下内角
				}

				block.tileIndex = index;
				block.handle = g_NormalTileHandles[index];
			}
			else
				block.handle = g_BlockHandle[type];

			return &block;
		}
	}
	return nullptr;
}

BlockData* GetBlocks()
{
	return g_Blocks;
}
