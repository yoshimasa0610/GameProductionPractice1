#include <stdio.h>
#include <string.h>
#include "SaveSystem.h"
#include "../Player/Player.h"
#include "../Item/ItemManager.h"
#include "../Item/ItemField.h"
#include "../Money/MoneyManager.h"

#define PLAYER_INIT_HP         (5)
SaveData g_SaveData;

int g_CurrentSaveSlot = 0;

// ==============================
// 内部関数：セーブファイルパス
// ==============================
static void GetSaveFilePath(char* path, int slot)
{
    sprintf_s(path, 64, "Data/save%d.bin", slot);
}

// ==============================
// チェックサム計算関数（簡易）
// ==============================
static unsigned int CalcChecksum(const SaveData* data)
{
    const unsigned char* bytes = (const unsigned char*)data;
    unsigned int sum = 0;
    for (size_t i = 0; i < sizeof(SaveData); i++)
        sum += bytes[i];
    return sum;
}

// ========================
// ゲームデータ保存
// ========================
void SaveGame(const SaveData* data, int slot)
{
    char path[64];
    GetSaveFilePath(path, slot);

    FILE* fp;
    fopen_s(&fp, path, "wb");
    if (!fp)
    {
        printf("セーブ失敗: スロット%dを開けません。\n", slot);
        return;
    }

    SaveHeader header;
    memcpy(header.signature, "SAVE", 4);
    header.version = SAVE_FILE_VERSION;
    header.checksum = CalcChecksum(data);

    if (fwrite(&header, sizeof(header), 1, fp) != 1 ||
        fwrite(data, sizeof(SaveData), 1, fp) != 1)
    {
        printf("スロット%dへの書き込みに失敗しました。\n", slot);
    }
    else
    {
        printf("スロット%dにセーブしました。\n", slot);
    }

    fclose(fp);

}

int LoadGame(SaveData* data, int slot)
{
    char path[64];
    GetSaveFilePath(path, slot);

    FILE* fp;
    fopen_s(&fp, path, "rb");
    if (!fp)
    {
        printf("スロット%dのセーブデータが見つかりません。\n", slot);
        return 0;
    }

    SaveHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1)
    {
        printf("スロット%dのヘッダー読み込みに失敗しました。\n", slot);
        fclose(fp);
        return 0;
    }

    // ファイル識別子確認
    if (strncmp(header.signature, "SAVE", 4) != 0)
    {
        printf("スロット%d: 無効なセーブデータです。\n", slot);
        fclose(fp);
        return 0;
    }

    // バージョン確認
    if (header.version != SAVE_FILE_VERSION)
    {
        printf("スロット%d: セーブデータのバージョンが異なります。\n", slot);
        fclose(fp);
        return 0;
    }

    // データ本体読み込み
    if (fread(data, sizeof(SaveData), 1, fp) != 1)
    {
        printf("スロット%d: データ読み込みに失敗しました。\n", slot);
        fclose(fp);
        return 0;
    }

    // チェックサム検証
    unsigned int actualChecksum = CalcChecksum(data);
    if (actualChecksum != header.checksum)
    {
        printf("スロット%d: セーブデータが破損しています。\n", slot);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    printf("スロット%dのセーブデータを正常に読み込みました。\n", slot);
    return 1;
}

bool DoesSaveExist(int slot)
{
    char path[64];
    GetSaveFilePath(path, slot);

    FILE* fp;
    fopen_s(&fp, path, "rb");
    if (!fp)
        return false;

    fclose(fp);
    return true;
}

bool LoadSaveSummary(int slot, SaveData* outData)
{
    char path[64];
    sprintf_s(path, 64, "Data/save%d.bin", slot);

    FILE* fp;
    fopen_s(&fp, path, "rb");
    if (!fp) return false;

    SaveHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return false;
    }

    if (strncmp(header.signature, "SAVE", 4) != 0)
    {
        fclose(fp);
        return false;
    }

    // SaveDataだけ読む（Importしない）
    if (fread(outData, sizeof(SaveData), 1, fp) != 1)
    {
        fclose(fp);
        return false;
    }

    fclose(fp);
    return true;
}
/*
void ReachCheckpoint(int cpX, int cpY, const char* stageName)
{
    ExportSaveData(&g_SaveData);

    g_SaveData.checkpointX = cpX;
    g_SaveData.checkpointY = cpY;
    strcpy_s(g_SaveData.stageName, stageName);
    g_SaveData.hp = g_SaveData.maxHp;

    SaveGame(&g_SaveData, g_CurrentSaveSlot);
}

// ==============================
// ExportSaveData
// ゲームの状態を SaveData 構造体に反映
// ==============================
void ExportSaveData(SaveData* data)
{
    // --- プレイヤー情報をコピー ---
    data->hp = g_PlayerData.hp;
    data->maxHp = g_PlayerData.maxHp;
    data->healGauge = g_PlayerData.healGauge;
    data->maxHealGauge = g_PlayerData.maxHealGauge;
    data->rangedGauge = g_PlayerData.rangedGauge;
    data->maxRangedGauge = g_PlayerData.maxRangedGauge;

    data->money = g_MoneyManager.GetMoney();

    // --- 所持アイテム情報 ---
     // 安全のため一旦全部 false にクリア
    for (int i = 0; i < SAVE_MAX_ITEM; ++i) data->ownedItems[i] = false;

    // アイテムマネージャーのアイテムリストを走査して、id が範囲内のものだけ保存する
    const auto& items = g_ItemManager.GetAllItems();
    for (const auto& it : items)
    {
        int id = it->id;
        if (id >= 0 && id < SAVE_MAX_ITEM)
        {
            data->ownedItems[id] = it->isOwned;
        }
        else
        {
            // id が大きい／負の場合はログに出す（開発時に役立つ）
            printf("ExportSaveData: item id %d is out of save range\n", id);
        }
    }

    // --- 装備情報 ---
    data->equippedItemCount = ItemManager_GetEquippedItemCount();
    for (int i = 0; i < SAVE_MAX_EQUIP; i++)
    {
        data->equippedItemIDs[i] = ItemManager_GetEquippedItemID(i);
    }
    ItemField::ExportToSave(*data);
}

// ==============================
// ImportSaveData
// SaveDataの内容をゲームに反映
// ==============================
void ImportSaveData(const SaveData* data)
{
    g_SaveData = *data;

    ItemManager_ClearAllItems();
    ItemField::ImportFromSave(*data);


    // --- プレイヤー情報を復元 ---
    g_PlayerData.hp = data->hp;
    g_PlayerData.maxHp = data->maxHp;
    g_PlayerData.healGauge = data->healGauge;
    g_PlayerData.maxHealGauge = data->maxHealGauge;
    g_PlayerData.rangedGauge = data->rangedGauge;
    g_PlayerData.maxRangedGauge = data->maxRangedGauge;

    g_MoneyManager.SetMoney(data->money);

    // --- アイテム所持情報を反映 ---
    //ItemManager_ClearAllItems();
    for (int i = 0; i < SAVE_MAX_ITEM; i++)
    {
        if (data->ownedItems[i])
            ItemManager_AddItem(i);     // 所持しているなら登録
    }

    // --- 装備情報を反映 ---
    ItemManager_ClearEquippedItems();
    for (int i = 0; i < data->equippedItemCount; i++)
    {
        int id = data->equippedItemIDs[i];
        if (id >= 0)
            ItemManager_EquipItem(id);
    }
    g_ItemManager.ApplyBuffsToPlayer(&g_PlayerData);
}


// セーブスロットが複数あるときこんな感じでリセットを
// 行わないと別のスロットでボスが沸かないことある
// まだボス作ってないと思うのでコメントアウト中
// 中ボスにも同様のものが必要かと思います。
/*
void ResetWorldProgress()
{
    for (int i = 0; i < BOSS_TYPE_MAX; ++i)
        g_SaveData.bossDefeated[i] = false;
}
*/