#pragma once
#include "DxLib.h"

#define BLOCK_MAX 4096         //ゲーム全体で扱えるブロックの最大数 

// マップ1チップの大きさ（共通）
#define MAP_CHIP_WIDTH  (50.0f)
#define MAP_CHIP_HEIGHT (50.0f)

// =====================================
// ■ マップサイズ動的設定対応
// =====================================
// Excelから読み取るサイズに応じて設定される
extern int g_MapChipXNum;   // X方向のチップ数
extern int g_MapChipYNum;   // Y方向のチップ数

enum MapChipType
{
	MAP_CHIP_NONE = 0,
	NORMAL_BLOCK,          //壁や天井などのブロック
	EXIT_BLOCK,            //対応しているステージに移動
	BREAKABLE_WALL,
	BREAKABLE_STATUE,
	SPIKE_BLOCK,           // トゲ（ダメージ）
	BACKGROUND_BLOCK,      // 背景（装飾用、当たり判定なし）

	MOVING_BLOCK,          // 移動するブロック（後で実装）
	BLOCK_TYPE_MAX,
};

struct BlockData
{
	bool active = false;           // 有効フラグ
	int handle = 0;            // 画像ハンドル
	int tileIndex = 0;  //シート内の番号
	MapChipType type = MAP_CHIP_NONE; // ブロック種別
	VECTOR pos = VGet(0.0f, 0.0f, 0.0f); // ワールド座標
	bool isSolid = false;           // 当たり判定があるかどうか

	int visual;//見た目の変化
};

struct MapChipData
{
	int mapChip = 0;           // チップID（Excelの値）
	int xIndex = 0;         // マップ配列上の X インデックス（0..）
	int yIndex = 0;         // マップ配列上の Y インデックス（0..）
	int width = 0;             // マップの横幅（チップ数）
	int height = 0;
	bool isSolid = false;
	BlockData* data = nullptr;       // 対応するブロックデータ
	int hp = 0;
};

// =====================================
// ■ 関数プロトタイプ（後で使う）
// =====================================
void SetMapSize(int xNum, int yNum);  // 動的サイズ設定
int GetMapWidth();                    // マップの横幅(px)
int GetMapHeight();                   // マップの縦幅(px)