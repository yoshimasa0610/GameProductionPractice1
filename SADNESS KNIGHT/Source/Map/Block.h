#pragma once
#include "MapParameter.h"

enum NormalBlockVisual
{
    NB_UP = 1 << 0,
    NB_DOWN = 1 << 1,
    NB_LEFT = 1 << 2,
    NB_RIGHT = 1 << 3,

    // ŽÎ‚ßiŠpˆ——pj
    NB_UL = 1 << 4,
    NB_UR = 1 << 5,
    NB_DL = 1 << 6,
    NB_DR = 1 << 7,
};

void InitBlock();
void LoadBlock();
void StartBlock();
void StepBlock();
void DrawBlock();
void FinBlock();

void ClearAllBlocks();

BlockData* CreateBlock(MapChipType type, VECTOR pos, int visual);
BlockData* GetBlocks();