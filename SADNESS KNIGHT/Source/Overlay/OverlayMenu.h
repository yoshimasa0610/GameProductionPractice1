#pragma once

struct PlayerData;

enum class OverlayTab
{
    Equip = 0,
    Skill,
    // 将来用
    // Status,
    // Map,

    Count
};

struct OverlayArea
{
    int x;
    int y;
    int w;
    int h;
};
OverlayArea GetOverlayContentArea();

bool IsOverlayOpen();

// 開閉
void OpenOverlayMenu(PlayerData* player);
void CloseOverlayMenu();

// タブ操作
void NextOverlayTab();
void PrevOverlayTab();

void DrawOverlayBackground();

// Sceneライフサイクル
void UpdateOverlayMenu();
void DrawOverlayMenu();