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
bool IsOverlayOpen();

// 開閉
void OpenOverlayMenu(PlayerData* player);
void CloseOverlayMenu();

// タブ操作
void NextOverlayTab();
void PrevOverlayTab();

// Sceneライフサイクル
void UpdateOverlayMenu();
void DrawOverlayMenu();