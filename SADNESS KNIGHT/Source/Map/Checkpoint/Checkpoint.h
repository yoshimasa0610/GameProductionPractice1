#pragma once
#include "../../SaveSystem/SaveSystem.h"

// チェックポイント（椅子）構造体
struct Checkpoint
{
    int id;
    int x, y;
    int w, h;
    bool isPlayerNear = false;//Playerが椅子と接触しているか
};

// ステージ単位ライフサイクル
void InitCheckpoint(const char* stageName);
void FinCheckpoint();

// 毎フレーム
void UpdateCheckpoint(
    SaveData* save,
    int playerX, int playerY, int pw, int ph
);

void DrawCheckpoint();