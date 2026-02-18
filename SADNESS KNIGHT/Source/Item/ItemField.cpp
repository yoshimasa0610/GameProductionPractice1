#include "DxLib.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Map/MapManager.h"
#include "../SaveSystem/SaveSystem.h"
#include "ItemField.h"
#include "../Camera/Camera.h"

std::unordered_set<int> ItemField::obtainedFieldItemIds;
int ItemField::fieldItemGraph = -1;
// ===============================
// SaveData → ItemField
// ===============================
void ItemField::ImportFromSave(const SaveData& save)
{
    obtainedFieldItemIds.clear();
    for (int i = 0; i < save.obtainedFieldItemCount; ++i)
    {
        obtainedFieldItemIds.insert(save.obtainedFieldItemIds[i]);
    }
}

// ===============================
// ItemField → SaveData
// ===============================
void ItemField::ExportToSave(SaveData& save)
{
    save.obtainedFieldItemCount = 0;

    for (int id : obtainedFieldItemIds)
    {
        if (save.obtainedFieldItemCount >= SAVE_MAX_FIELD_ITEMS)
            break;

        save.obtainedFieldItemIds[save.obtainedFieldItemCount++] = id;
    }
}

// ===============================
// はじめから用
// ===============================
void ItemField::ClearAll()
{
    obtainedFieldItemIds.clear();
}

void ItemField::LoadForStage(const char* stageName)
{
    items.clear();
    pickedBuffer.clear();

    if (fieldItemGraph < 0)
    {
        fieldItemGraph = LoadGraph("Data/Item/ItemField/ItemField.png");
    }

    const char* fieldName = GetFieldName(stageName);
    // ステージ名から CSV ファイルのパスを生成
    // 例： Data/Map/forest/forest_1/items.csv
    std::string path = "";
    path += "Data/Map/";
    path += fieldName;
    path += "/";
    path += stageName;
    path += "/ItemField.csv";

    //printfDx("[ItemField] CSV Path: %s\n", path.c_str());

    LoadCSV(path);
}

void ItemField::LoadCSV(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        // 読み込み失敗 → 何も置かない
        printfDx("ItemField CSV 読み込み失敗: %s\n", path.c_str());
        return;
    }

    std::string line;
    // === ヘッダー行をスキップ ===
    if (!std::getline(file, line))
    {
        // 空ファイル
        return;
    }
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;

        FieldItem item{};

        // fieldItemId
        if (!std::getline(ss, token, ',')) continue;
        item.fieldItemId = std::stoi(token);

        // x
        if (!std::getline(ss, token, ',')) continue;
        item.x = std::stof(token);

        // y
        if (!std::getline(ss, token, ',')) continue;
        item.y = std::stof(token);

        // w
        if (!std::getline(ss, token, ',')) continue;
        item.w = std::stof(token);

        // h
        if (!std::getline(ss, token, ',')) continue;
        item.h = std::stof(token);

        // itemId
        if (!std::getline(ss, token, ',')) continue;
        item.itemId = std::stoi(token);

        // 既に取得済みなら最初から消す
        if (obtainedFieldItemIds.count(item.fieldItemId) > 0)
        {
            item.picked = true;
        }

        items.push_back(item);
    }
}

void ItemField::Update(float playerX, float playerY, float playerW, float playerH)
{
    for (auto& it : items)
    {
        if (it.picked) continue;

        // アニメーション更新
        it.animTime += 0.05f;

        if (AABBIntersect(
            playerX, playerY, playerW, playerH,
            it.x, it.y, it.w, it.h))
        {
            it.picked = true;
            pickedBuffer.push_back(it.itemId);

            // 配置IDを記録
            obtainedFieldItemIds.insert(it.fieldItemId);
        }
    }
}

void ItemField::Draw() const
{
    CameraData cam = GetCamera();

    for (const auto& it : items)
    {
        if (it.picked) continue;

        // 上下ふわふわ（ワールド空間）
        float offsetY = sinf(it.animTime) * 6.0f;

        // α点滅
        int alpha = (int)(128 + sinf(it.animTime * 2.0f) * 127);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // ワールド → スクリーン変換
        float worldX = it.x + it.w * 0.5f - 16.0f;
        float worldY = it.y + it.h * 0.5f - 16.0f + offsetY;

        int screenX = (int)WorldToScreenX(worldX, cam);
        int screenY = (int)WorldToScreenY(worldY, cam);

        DrawGraph(
            screenX,
            screenY,
            fieldItemGraph,
            TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

std::vector<int> ItemField::FetchPickedItems()
{
    std::vector<int> out = pickedBuffer;
    pickedBuffer.clear();
    return out;
}
