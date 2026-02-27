#ifndef SCENE_EQUIP_MENU_H
#define SCENE_EQUIP_MENU_H

struct PlayerData;
struct OverlayArea;
extern bool g_IsEquipMenuOpen;

void CloseEquipMenu();
void SetEquipMode(bool enable);
void SetEquipMenuPlayer(PlayerData* player);

// シーンのライフサイクル
void InitEquipMenuScene();
void LoadEquipMenuScene();
void StartEquipMenuScene();
void OpenEquipMenu(PlayerData* player);
void StepEquipMenuScene();
void UpdateEquipMenuScene();
void DrawEquipMenuScene(const OverlayArea& area);
void FinEquipMenuScene();
#endif