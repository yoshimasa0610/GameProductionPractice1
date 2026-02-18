#pragma once
#include <vector>
#include <string>
#include "../Collision/Collision.h"
#include <unordered_set>

struct SaveData;

// フィールドに落ちているアイテムひとつ
struct FieldItem
{
    // fieldItemId は itemId と同一の値を使用する
    int fieldItemId;
    float x, y;             // 位置
    float w, h;             // サイズ（白い四角）
    int itemId;             // アイテム種類ID（必要なら後で使う）
    bool picked = false;    // 拾われたら true

    // ===== 演出用 =====
    float animTime = 0.0f;   // アニメーション用時間
};

// マップに応じたフィールドアイテム管理
class ItemField
{
public:
    // 現在ステージ名からアイテム配置を読み込む
    void LoadForStage(const char* stageName);

    // 更新（プレイヤーとの接触判定）
    void Update(float playerX, float playerY, float playerW, float playerH);

    // 描画
    void Draw() const;

    // 拾ったアイテムを取り出す（PlayScene で処理する用）
    std::vector<int> FetchPickedItems();

    // SaveSystem 連携
    static void ImportFromSave(const SaveData& save);
    static void ExportToSave(SaveData& save);
    static void ClearAll();   // 「はじめから」用

private:
    std::vector<FieldItem> items;
    std::vector<int> pickedBuffer;    // 今フレーム拾われた itemId
    // 取得済み配置ID
    static std::unordered_set<int> obtainedFieldItemIds;
    // 内部用：CSV 読み込み
    void LoadCSV(const std::string& path);
    static int fieldItemGraph;   // 共通のキラキラ画像
};