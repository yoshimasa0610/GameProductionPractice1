#pragma once

struct PlayerData;
struct OverlayArea;

enum class CheckpointMenuItem
{
    Equip = 0,
    Skill,
    Leave,
    Count
};

bool IsCheckpointMenuOpen();

void OpenCheckpointMenu(PlayerData* player);
void CloseCheckpointMenu();

void UpdateCheckpointMenu();
void DrawCheckpointMenu();