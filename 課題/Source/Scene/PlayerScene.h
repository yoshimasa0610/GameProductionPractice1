#pragma once
#include "DxLib.h"

enum CharacterType
{
    CHARACTER_TYPE_HERO = 0,
    CHARACTER_TYPE_ENEMY_A,
    CHARACTER_TYPE_ENEMY_B,

    CHARACTER_TYPE_MAX
};

struct CharacterData
{
    VECTOR pos;     // ˆÊ’u
    int handle;     // ‰æ‘œƒnƒ“ƒhƒ‹
};

class PlayerScene
{
private:
    SceneManager* sceneManager;

    void InitPlayScene();
    void LoadPlayScene();
    void StartPlayScene();
    void StepPlayScene();
    void UpdatePlayScene();
    void DrawPlayScene();
    void FinPlayScene();
};