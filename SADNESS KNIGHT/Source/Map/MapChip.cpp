#include "MapChip.h"
#include "MapManager.h"
#include "DxLib.h"
#include "../GameSetting/GameSetting.h"
#include "MapParameter.h"
#include "Block.h"
#include <vector>
#include <string>

// ============================
// 可変サイズ対応
// ============================
int g_MapChipXNum = 64;
int g_MapChipYNum = 18;

// vectorベースに変更
std::vector<std::vector<MapChipData>> g_MapChip;

// ============================
// マップサイズ設定
// ============================
void SetMapSize(int xNum, int yNum)
{
    g_MapChipXNum = xNum;
    g_MapChipYNum = yNum;

    g_MapChip.clear();
    g_MapChip.resize(g_MapChipYNum);
    for (int i = 0; i < g_MapChipYNum; i++)
    {
        g_MapChip[i].resize(g_MapChipXNum);
    }
}

// ============================
// マップデータ読み込み（.bin対応）
// ============================
void LoadMapChipData(const char* filePath)
{
    FILE* fp = nullptr;
    if (fopen_s(&fp, filePath, "rb") != 0 || !fp)
    {
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long expected = (long)g_MapChipXNum * (long)g_MapChipYNum;

    if (fileSize != expected)
    {
    }

    std::vector<unsigned char> temp(fileSize);
    fread(temp.data(), 1, fileSize, fp);
    fclose(fp);

    long idx = 0;
    for (int y = 0; y < g_MapChipYNum; y++)
    {
        for (int x = 0; x < g_MapChipXNum; x++)
        {
            int map = (idx < fileSize) ? temp[idx] : 0;
            g_MapChip[y][x].mapChip = map;
            g_MapChip[y][x].xIndex = x;
            g_MapChip[y][x].yIndex = y;
            g_MapChip[y][x].isSolid = (map != MAP_CHIP_NONE);
            g_MapChip[y][x].data = nullptr;
            idx++;
        }
    }
}

int DecideNormalBlockVisual(int x, int y)
{
    auto isSame = [&](int tx, int ty)
    {
        if (tx < 0 || tx >= g_MapChipXNum ||
            ty < 0 || ty >= g_MapChipYNum)
        {
            return true;
        }
        MapChipData* mc = GetMapChipData(tx, ty);
        return mc && mc->mapChip == NORMAL_BLOCK;
    };

    int v = 0;

    if (isSame(x, y - 1)) v |= NB_UP;
    if (isSame(x, y + 1)) v |= NB_DOWN;
    if (isSame(x - 1, y)) v |= NB_LEFT;
    if (isSame(x + 1, y)) v |= NB_RIGHT;

    if (isSame(x, y - 1) &&
        isSame(x - 1, y) &&
        isSame(x - 1, y - 1))
        v |= NB_UL;

    if (isSame(x, y - 1) &&
        isSame(x + 1, y) &&
        isSame(x + 1, y - 1))
        v |= NB_UR;

    if (isSame(x, y + 1) &&
        isSame(x - 1, y) &&
        isSame(x - 1, y + 1))
        v |= NB_DL;

    if (isSame(x, y + 1) &&
        isSame(x + 1, y) &&
        isSame(x + 1, y + 1))
        v |= NB_DR;

    return v;
}

// ============================
// マップ作成（BlockData生成）
// ============================
void CreateMap()
{
    ClearAllBlocks();

    int created = 0;
    for (int y = 0; y < g_MapChipYNum; y++)
    {
        for (int x = 0; x < g_MapChipXNum; x++)
        {
            MapChipType type = (MapChipType)g_MapChip[y][x].mapChip;
            if (type == MAP_CHIP_NONE) continue;

            VECTOR pos = VGet(x * MAP_CHIP_WIDTH, y * MAP_CHIP_HEIGHT, 0.0f);
            created++;

            if (type == NORMAL_BLOCK)
            {
                int visual = DecideNormalBlockVisual(x, y);
                g_MapChip[y][x].data = CreateBlock(type, pos, visual);
            }
            else
            {
                g_MapChip[y][x].data = CreateBlock(type, pos, 0);
            }

            if (type == EXIT_BLOCK)
            {
            }

            if (type == BREAKABLE_WALL)
            {
                g_MapChip[y][x].hp = 1; // 壁
            }
            else if (type == BREAKABLE_STATUE)
            {
                g_MapChip[y][x].hp = 1; // 石像（後で増やしてもOK）
            }
            else
            {
                g_MapChip[y][x].hp = 0;
            }
        }
    }
}

// ============================
// 参照取得（安全版）
// ============================
MapChipData* GetMapChipData(int x, int y)
{
    if (y < 0 || y >= g_MapChipYNum || x < 0 || x >= g_MapChipXNum)
        return nullptr;
    return &g_MapChip[y][x];
}

// ============================
// メモリ解放（vectorなのでほぼ不要）
// ============================
void FreeMapChip()
{
    g_MapChip.clear();
}