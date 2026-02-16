#include "MoneyDropSystem.h"
#include "DxLib.h"
#include "../Money/MoneyManager.h"
#include <corecrt_math.h>
#include "MoneyPopup.h"
#include "../Camera/Camera.h"
#include "../Sound/Sound.h"

static MoneyDrop g_drops[MAX_MONEY_DROPS];
static int g_MoneyImgSmall = -1;
static int g_MoneyImgMid = -1;
static int g_MoneyImgLarge = -1;
//==================================================
// 初期化
//==================================================
void InitMoneyDrops()
{
    for (int i = 0; i < MAX_MONEY_DROPS; ++i)
    {
        g_drops[i].active = false;
    }
    if (g_MoneyImgSmall < 0)
    {
        g_MoneyImgSmall = LoadGraph("Data/Item/Coin/coin.png");//coin_small.png
        g_MoneyImgMid = LoadGraph("Data/Item/Coin/coin_mid.png");
        g_MoneyImgLarge = LoadGraph("Data/Item/Coin/coin_large.png");
    }
}

//==================================================
// スポーン（合計金額 → コインに分割して散らばる）
//==================================================
void SpawnMoneyDrops(float x, float y, int totalMoney)
{
    while (totalMoney > 0)
    {
        int coin = 1 + (GetRand(2)); // 1～3G のコインに分割
        if (coin > totalMoney) coin = totalMoney;

        // 空きスロットを探す
        for (int i = 0; i < MAX_MONEY_DROPS; ++i)
        {
            if (!g_drops[i].active)
            {
                MoneyDrop& d = g_drops[i];
                d.active = true;

                d.x = x;
                d.y = y - 32;    // 敵の中心より少し上から出す

                // ランダムな初速
                d.vx = ((GetRand(100) / 100.0f) - 0.5f) * 4.0f; // -2.0～2.0
                d.vy = -(GetRand(200) / 100.0f + 3.0f);         // -3.0～ -5.0

                d.value = coin;
                d.radius = 16;

                if (coin <= 2)
                {
                    d.visual = MONEY_VISUAL_SMALL;
                }
                else if (coin <= 5)
                {
                    d.visual = MONEY_VISUAL_MIDDLE;
                }
                else
                {
                    d.visual = MONEY_VISUAL_LARGE;
                }

                break;
            }
        }

        totalMoney -= coin;
    }
}

//==================================================
// 更新（物理 & 回収）
//==================================================
void UpdateMoneyDrops(float playerX, float playerY)
{
    for (int i = 0; i < MAX_MONEY_DROPS; ++i)
    {
        MoneyDrop& d = g_drops[i];
        if (!d.active) continue;

        // 重力
        d.vy += 0.2f;

        // 移動
        d.x += d.vx;
        d.y += d.vy;

        // 地面で停止（仮：y > 600 を地面として扱う）
        if (d.y > 600)
        {
            d.y = 600;
            d.vx *= 0.8f;
            d.vy = 0;
        }

        float dx = playerX - d.x;
        float dy = playerY - d.y;
        float distSq = dx * dx + dy * dy;

        // 吸引開始距離
        const float attractDist = 120.0f;

        if (distSq < attractDist * attractDist)
        {
            float dist = sqrtf(distSq);
            if (dist > 0.1f)
            {
                float ax = dx / dist;
                float ay = dy / dist;

                d.vx += ax * 0.5f;
                d.vy += ay * 0.5f;
            }
        }


        if (distSq < (d.radius * d.radius))
        {
            // お金を増やす
            g_MoneyManager.AddMoney(d.value);
            // お金を獲得したことを知らせる
            AddMoneyPopup(d.value);
            //お金獲得時のSE
            PlaySE(SE_MONEY_PICKUP);

            // 非アクティブに
            d.active = false;
        }
    }
}
/*
//==================================================
// 描画
//==================================================
void DrawMoneyDrops()
{
    auto cam = GetCamera();
    float scale = cam.scale;
    float camX = GetCameraX();
    float camY = GetCameraY();

    for (int i = 0; i < MAX_MONEY_DROPS; ++i)
    {
        MoneyDrop& d = g_drops[i];
        if (!d.active) continue;

        // ワールド → スクリーン変換
        float sx = (d.x - camX) * scale;
        float sy = (d.y - camY) * scale;
        int   r = (int)(d.radius * scale);

        // コイン画像（中心合わせ）
        int halfSize = (int)(16 * scale); // 元画像が 32x32 前提

        int img = g_MoneyImgSmall;
        switch (d.visual)
        {
        case MONEY_VISUAL_MIDDLE:
            img = g_MoneyImgMid;
            break;
        case MONEY_VISUAL_LARGE:
            img = g_MoneyImgLarge;
            break;
        default:
            img = g_MoneyImgSmall;
            break;
        }

        // コイン画像描画（中心合わせ）
        DrawExtendGraph(
            (int)(sx - halfSize),
            (int)(sy - halfSize),
            (int)(sx + halfSize),
            (int)(sy + halfSize),
            img,
            TRUE
        );

        // 高額のみ数値表示
        if (d.visual == MONEY_VISUAL_LARGE)
        {
            DrawFormatStringF(
                sx - 6 * scale,
                sy - 32 * scale,
                GetColor(255, 255, 0),
                "%d",
                d.value
            );
        }
    }
}
*/

// たるとか
int DecideStatueDropMoney()
{
    // 50%で何も落とさない
    if (GetRand(1) == 0)
    {
        return 0;
    }

    // 3～10G
    return 3 + GetRand(7);
}