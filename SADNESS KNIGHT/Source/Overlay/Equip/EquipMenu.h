#ifndef SCENE_EQUIP_MENU_H
#define SCENE_EQUIP_MENU_H

struct PlayerData;

extern bool g_IsEquipMenuOpen;

void SetEquipMode(bool enable);
void SetEquipMenuPlayer(PlayerData* player);

// シーンのライフサイクル
void InitEquipMenuScene();
void LoadEquipMenuScene();
void StartEquipMenuScene();
void OpenEquipMenu(PlayerData* player);
void StepEquipMenuScene();
void UpdateEquipMenuScene();
void DrawEquipMenuScene();
void FinEquipMenuScene();



#endif