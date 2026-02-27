#pragma once
struct PlayerData;
struct OverlayArea;
extern bool g_IsSkillMenuOpen;

void OpenSkillMenu(PlayerData* player);
void CloseSkillMenu();

void UpdateSkillMenuScene();
void DrawSkillMenuScene(const OverlayArea& area);