#include "Camera.h"
#include "DxLib.h"
#include "../Player/Player.h"
#include "../GameSetting/GameSetting.h"

static CameraData camera;

static const float CAMERA_SPEED = 0.1f;

void InitCamera()
{
	camera.posX = 0.0f;
	camera.posY = 0.0f;
	camera.scale = 1.0f;
}

void StepCamera()
{

}

void UpdateCamera()
{
	PlayerData& player = GetPlayerData();

	float targetX = player.posX - SCREEN_WIDTH / 2;
	float targetY = player.posY - SCREEN_HEIGHT / 2;

	camera.posX += (targetX - camera.posX) * CAMERA_SPEED;
	camera.posY += (targetY - camera.posY) * CAMERA_SPEED;

}

void DrawCamera()
{
	SetDrawArea((int)camera.posX,(int)camera.posY,(int)(camera.posX + SCREEN_WIDTH),(int)(camera.posY + SCREEN_HEIGHT));
}

void FinalCamera()
{

}

CameraData GetCamera()
{
	return camera;
}