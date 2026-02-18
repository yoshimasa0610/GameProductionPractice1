#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H
#include "../Item/ItemManager.h"

#define SAVE_MAX_EQUIP 3
#define SAVE_MAX_ITEM 50      // 所持アイテムの最大数（必要に応じて調整）
#define SAVE_SLOT_MAX 3       // セーブスロット数
#define SAVE_MAX_FIELD_ITEMS 256
#define PLAYER_INIT_HP         (5)
#define SAVE_FILE_VERSION 1

//現在使用中のスロット
extern int g_CurrentSaveSlot;

// =======================
// セーブデータヘッダー
// =======================
typedef struct
{
    char signature[4];   // ファイル識別子 "SAVE"
    int version;         // バージョン番号
    unsigned int checksum; // データ整合性チェック
} SaveHeader;

// =======================
// セーブデータ本体
// =======================
struct SaveData
{
    // Playerのセーブする内容
    int currentHP;
    int maxHP;


    //お金
    int money;

    // Checkpoint情報
    char stageName[64];
    int checkpointX;
    int checkpointY;      // チェックポイント位置

    // 所持・装備関連
    bool ownedItems[SAVE_MAX_ITEM];    // 所持中アイテムフラグ
    int equippedItemCount;             // 装備中アイテム数
    int equippedItemIDs[SAVE_MAX_EQUIP]; // 装備中のアイテムID
    int obtainedFieldItemCount;
    int obtainedFieldItemIds[SAVE_MAX_FIELD_ITEMS];
};

// ゲームセーブ
void SaveGame(const SaveData* data, int slot);

// ゲームロード
int LoadGame(SaveData* data, int slot);

// チェックポイント到達時セーブ
void ReachCheckpoint(int cpX, int cpY, const char* stageName);

// 現在のプレイヤー状態とアイテム装備状態をSaveDataへ反映
void ExportSaveData(SaveData* data);// ゲーム→SaveData

// SaveDataからゲームへ反映
void ImportSaveData(const SaveData* data);// SaveData→ゲーム

//セーブデータを読む（それを基にプレイ時間などをタイトルで表記）
bool LoadSaveSummary(int slot, SaveData* outData);
//セーブデータがあるかどうかをチェックする
bool DoesSaveExist(int slot);


void ResetWorldProgress();

#endif