#include "Checkpoint.h"
#include "DxLib.h"
#include "../../SaveSystem/SaveSystem.h"
#include "../../Input/Input.h"
#include "../../Collision/Collision.h"
#include "../../Player/Player.h"
#include "../../Overlay/Equip/EquipMenu.h"
#include "../../Scene/Play/Play.h"
#include "../Checkpoint/CheckpointManager.h"
#include "../../Map/StageManager.h"
#include "../MapManager.h"
#include <vector>
#include <cstdio>
#include "../../Camera/Camera.h"
#include "../../Skill/SkillManager.h"
#include "../../Overlay/CheckpointMenu/CheckpointMenu.h"

static const int INTERACTION_KEY = KEY_UP;
static std::vector<Checkpoint> g_Checkpoints;
static int g_CheckpointImg = -1;

void InitCheckpoint(const char* stageName)
{
    g_Checkpoints.clear();

    if (g_CheckpointImg < 0)
    {
        g_CheckpointImg = LoadGraph("Data/Map/chair.png");
    }

    // forest_1 → forest
    const char* field = GetFieldName(stageName);

    char path[256];
    sprintf_s(path,
        "Data/Map/%s/%s/Checkpoint.csv",
        field, stageName);

    FILE* fp = nullptr;
    if (fopen_s(&fp, path, "r") != 0 || !fp)
    {
        printf_s("[Checkpoint] CSV not found: %s\n", path);
        return;
    }

    // ヘッダ読み飛ばし
    char line[256];
    fgets(line, sizeof(line), fp);

    Checkpoint cp;
    int gridX, gridY;

    while (fscanf_s(fp, "%d,%d,%d,%d,%d",
        &cp.id,
        &gridX,
        &gridY,
        &cp.w,
        &cp.h) == 5)
    {
        cp.x = gridX * MAP_CHIP_WIDTH;
        cp.y = gridY * MAP_CHIP_HEIGHT;
        cp.w *= MAP_CHIP_WIDTH;
        cp.h *= MAP_CHIP_HEIGHT;
        g_Checkpoints.push_back(cp);
    }

    fclose(fp);

    printf_s("[Checkpoint] loaded %d checkpoints\n",
        (int)g_Checkpoints.size());
}

void DrawCheckpoint()
{
    CameraData camera = GetCamera();
    for (const auto& cp : g_Checkpoints)

    {
        int drawX = (int)WorldToScreenX((float)cp.x, camera);
        int drawY = (int)WorldToScreenY((float)cp.y, camera);

        // 今回は椅子画像を等倍表示する前提
        DrawGraph(drawX, drawY, g_CheckpointImg, TRUE);

        // プレイヤーが近くにいるときだけ表示
        if (cp.isPlayerNear && !IsPlayerSitting())
        {
            int textX = drawX + cp.w / 2 - 24;
            int textY = drawY - 20;

            DrawString(
                textX,
                textY,
                "↑休憩",
                GetColor(255, 255, 255)
            );
        }
    }
}

void FinCheckpoint()
{
    g_Checkpoints.clear();
}


// ===============================
// チェックポイント座った時の処理
// ===============================
static void ActivateCheckpoint(SaveData* save, const Checkpoint& cp)
{
    // Player を参照して回復／セーブをする
    // Player モジュール内のグローバル g_PlayerData を直接使う
    PlayerData& player = GetPlayerData(); // Player.cpp で定義されているグローバル
    //g_PlayerData.hp = g_PlayerData.maxHp;

    // =========================
    // 回復処理
    // =========================
    // HP全回復
    player.currentHP = player.maxHP;
    // 回復回数リセット
    player.healCount = player.maxHealCount;
    // スキル回数リセット
    g_SkillManager.RecalculateUses(&player);


        // 座る
    SetSitState(SitState::Sitting);

    // セーブ通知
    ReachCheckpoint(
        cp.x,
        cp.y,
        GetCurrentStageName()
    );
    /*// 位置・ステージ更新
    save->checkpointX = chairX;
    save->checkpointY = chairY;
    save->stageNumber = stageNumber;

    // セーブデータへエクスポート
    ExportSaveData(save);
    SaveGame(save, 0); // 仮にスロット0に固定
    */
    OpenCheckpointMenu(&player);
}

// ===============================
// 椅子とのインタラクション検出（毎フレーム呼ぶ）
// player の矩形、椅子の矩形を渡す
// ===============================
static void CheckpointInteraction(
    SaveData* save,
    int playerX, int playerY, int pw, int ph,
    Checkpoint& cp)
{
    // プレイヤーと椅子の当たり判定
    if (AABBIntersect(
        (float)playerX, (float)playerY, (float)pw, (float)ph,
        (float)cp.x, (float)cp.y, (float)cp.w, (float)cp.h))
    {
        cp.isPlayerNear = true;

        // 座る（セーブと回復）  トリガー判定を使う
        if (cp.isPlayerNear && IsTriggerKey(KEY_UP))
        {
            if (!IsPlayerSitting())
            {
                ActivateCheckpoint(save, cp);
            }
            else
            {
                // 立つ
                SetSitState(SitState::None);
            }
        }
    }
    else
    {
        cp.isPlayerNear = false;
        // 椅子から離れたら座りフラグを解除
    }

    // 座っているときのみ装備着脱が許可されるが、メニュー自体はどこでも開ける仕様。
    // ただし Checkpoint 側でもインベントリから直接メニューを開けるようにしておく（どこでも開けてもよい）。
    if (IsTriggerKey(KEY_INVENTORY))
    {
        // メニューを開くときには装備変更許可フラグ（SetEquipMode）をセット
        // 今回はメニューはどこでも開けるが、装備変更は椅子でのみ許可する仕様なので SetEquipMode は座り状態にする
        SetEquipMode(IsPlayerSitting()); // EquipMenu 側でも二重チェックするが念の為渡す
        // EquipMenu がプレイヤーデータ参照を必要とする場合は渡す（省略可能）
        PlayerData& player = GetPlayerData();
        SetEquipMenuPlayer(&player);
        SetPaused(true);
    }
}

void UpdateCheckpoint(
    SaveData* save,
    int playerX, int playerY, int pw, int ph
)
{
    if (IsCheckpointMenuOpen())
    {
        UpdateCheckpointMenu();
        return;
    }

    for (auto& cp : g_Checkpoints)
    {
        CheckpointInteraction(
            save,
            playerX, playerY, pw, ph,
            cp
        );
    }
}