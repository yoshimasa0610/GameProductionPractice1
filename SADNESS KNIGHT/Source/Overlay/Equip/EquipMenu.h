#ifndef SCENE_EQUIP_MENU_H
#define SCENE_EQUIP_MENU_H

struct PlayerData;

extern bool g_IsEquipMenuOpen;

void OpenEquipMenu(PlayerData* player);
void CloseEquipMenu();

void SetEquipMode(bool enable);
void SetEquipMenuPlayer(PlayerData* player);

// シーンのライフサイクル
void InitEquipMenuScene();
void LoadEquipMenuScene();
void StartEquipMenuScene();
void OpenEqipMenu(PlayerData* player);
void StepEquipMenuScene();
void UpdateEquipMenuScene();
void DrawEquipMenuScene();
void FinEquipMenuScene();



#endif