#include "Camera.h"
#include "DxLib.h"
#include "../Player/Player.h"
#include "../GameSetting/GameSetting.h"
#include "../Map/MapManager.h"

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

    float playerCenterX = player.posX + player.width / 2;
    float playerCenterY = player.posY + player.height / 2;

    float targetX = playerCenterX - SCREEN_WIDTH / 2;
    float targetY = playerCenterY - SCREEN_HEIGHT / 2;

    camera.posX += (targetX - camera.posX) * CAMERA_SPEED;
    camera.posY += (targetY - camera.posY) * CAMERA_SPEED;

    float mapWidth = (float)GetMapWidth();
    float mapHeight = (float)GetMapHeight();

    if (mapWidth <= SCREEN_WIDTH)
    {
        camera.posX = -(SCREEN_WIDTH - mapWidth) * 0.5f;
    }
    else
    {
        float maxX = mapWidth - SCREEN_WIDTH;
        camera.posX = max(0.0f, min(camera.posX, maxX));
    }

    if (mapHeight <= SCREEN_HEIGHT)
    {
        camera.posY = -(SCREEN_HEIGHT - mapHeight) * 0.5f;
    }
    else
    {
        float maxY = mapHeight - SCREEN_HEIGHT;
        camera.posY = max(0.0f, min(camera.posY, maxY));
    }
}

void DrawCamera()
{
	//SetDrawArea((int)camera.posX,(int)camera.posY,(int)(camera.posX + SCREEN_WIDTH),(int)(camera.posY + SCREEN_HEIGHT));
}

void FinalCamera()
{

}

// カメラ位置とスケールを考慮した変換関数。
float WorldToScreenX(float worldX, const CameraData& camera)
{
	return (worldX - camera.posX) * camera.scale;
}

float WorldToScreenY(float worldY, const CameraData& camera)
{
	return (worldY - camera.posY) * camera.scale;
}

CameraData GetCamera()
{
	return camera;
}