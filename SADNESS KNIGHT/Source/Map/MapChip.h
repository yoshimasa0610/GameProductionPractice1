#pragma once
#include "MapParameter.h"
#include <vector>

// =====================================
// ŠO•”•Ï”éŒ¾ivector”Åj
// =====================================
extern std::vector<std::vector<MapChipData>> g_MapChip;

// =====================================
// ŠÖ”éŒ¾
// =====================================
void SetMapSize(int xNum, int yNum);
void LoadMapChipData(const char* filePath);
void CreateMap();
void FreeMapChip();
MapChipData* GetMapChipData(int x, int y);