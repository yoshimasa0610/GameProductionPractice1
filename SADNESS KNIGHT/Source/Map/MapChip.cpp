#include "MapChip.h"
#include "MapManager.h"
#include "DxLib.h"
#include "../GameSetting/GameSetting.h"
#include "MapParameter.h"
#include "Block.h"
#include <vector>
#include <string>
#include "../Collision/Collision.h"
#include "../Money/MoneyDropSystem.h"

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

    long expected = (long)g_MapChipXNum * (long)g_MapChipYNum * 2;

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
            int chip = (idx < fileSize) ? temp[idx++] : 0;
            int visual = (idx < fileSize) ? temp[idx++] : 0;

            g_MapChip[y][x].mapChip = chip;
            g_MapChip[y][x].visual = visual;
            g_MapChip[y][x].xIndex = x;
            g_MapChip[y][x].yIndex = y;
            g_MapChip[y][x].isSolid = (chip != MAP_CHIP_NONE);
            g_MapChip[y][x].data = nullptr;
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

            float left = x * MAP_CHIP_WIDTH;
            float top = y * MAP_CHIP_HEIGHT;

            created++;

            // デフォルト初期化
            g_MapChip[y][x].colliderId = -1;
            g_MapChip[y][x].hp = 0;

            // -------------------------
            // Block生成
            // -------------------------
            int visual = g_MapChip[y][x].visual;

            if (type == NORMAL_BLOCK)
            {
                visual = DecideNormalBlockVisual(x, y);
            }

            g_MapChip[y][x].data = CreateBlock(type, pos, visual);

            // -------------------------
            // Collider生成
            // -------------------------
            switch (type)
            {
            case NORMAL_BLOCK:

                g_MapChip[y][x].colliderId =
                    CreateCollider(
                        ColliderTag::Block,
                        left,
                        top,
                        MAP_CHIP_WIDTH,
                        MAP_CHIP_HEIGHT,
                        nullptr
                    );

                break;

            case EXIT_BLOCK:

                g_MapChip[y][x].colliderId =
                    CreateCollider(
                        ColliderTag::Exit,
                        left,
                        top,
                        MAP_CHIP_WIDTH,
                        MAP_CHIP_HEIGHT,
                        nullptr
                    );
                DrawFormatString(20, 120, GetColor(255, 255, 0),
                    "Exit collider created %d %d\n", x, y);
                break;

            case SEMI_SOLID_BLOCK:

                g_MapChip[y][x].colliderId =
                    CreateCollider(
                        ColliderTag::SemiSolid,
                        left,
                        top,
                        MAP_CHIP_WIDTH,
                        8.0f // 上面のみ判定
                    );

                break;

            case BREAKABLE_OBJECT:

                g_MapChip[y][x].colliderId =
                    CreateCollider(
                        ColliderTag::Block,
                        left,
                        top,
                        MAP_CHIP_WIDTH,
                        MAP_CHIP_HEIGHT,
                        nullptr
                    );

                g_MapChip[y][x].hp = 1;

                break;

            default:
                break;
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

bool DamageMapChip(int x, int y, int damage, bool isDiveAttack)
{
    MapChipData* chip = GetMapChipData(x, y);
    if (!chip) return false;

    MapChipType type = (MapChipType)chip->mapChip;

    // 破壊対象か確認
    if (type != BREAKABLE_OBJECT &&
        type != BREAKABLE_DIVE_FLOOR)
    {
        return false;
    }

    if (type == BREAKABLE_DIVE_FLOOR && !isDiveAttack)
        return false;

    chip->hp -= damage;

    if (chip->hp > 0)
        return false;

    // -------------------------
    // 破壊処理
    // -------------------------

    float worldX = x * MAP_CHIP_WIDTH + MAP_CHIP_WIDTH * 0.5f;
    float worldY = y * MAP_CHIP_HEIGHT + MAP_CHIP_HEIGHT * 0.5f;
    /*
    // ドロップ処理
    if (type == BREAKABLE_STATUE)
    {
        int money = DecideStatueDropMoney();
        if (money > 0)
        {
            SpawnMoneyDrops(worldX, worldY, money);
        }
    }

    // 壁は低確率ドロップ
    if (type == BREAKABLE_WALL)
    {
        if (GetRand(3) == 0) // 25%
        {
            SpawnMoneyDrops(worldX, worldY, 1);
        }
    }
    */
    // Collider削除
    if (chip->colliderId >= 0)
    {
        DestroyCollider(chip->colliderId);
        chip->colliderId = -1;
    }

    // Block削除
    if (chip->data)
    {
        chip->data->active = false;
        chip->data = nullptr;
    }

    chip->mapChip = MAP_CHIP_NONE;
    chip->isSolid = false;
    chip->hp = 0;
    // 破壊SE
    //PlaySE(SE_BREAK_WALL);
    return true;
}

// ============================
// メモリ解放（vectorなのでほぼ不要）
// ============================
void FreeMapChip()
{
    g_MapChip.clear();
}