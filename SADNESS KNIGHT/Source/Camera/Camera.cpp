#include "../Map/MapManager.h"
#include "../Player/Player.h"
#include "../GameSetting/GameSetting.h"
#include "Camera.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

static CameraData camera;
static bool g_IsFixed = false;
static float g_FixedCenterX = 0.0f;
static float g_FixedCenterY = 0.0f;
static float g_ShakeTimer = 0.0f;
static float g_ShakeDuration = 0.0f;
static float g_ShakeAmplitude = 0.0f;

static const float CAMERA_SPEED = 0.1f;

void InitCamera()
{
	camera.posX = 0.0f;
	camera.posY = 0.0f;
	camera.scale = 1.0f;
	g_IsFixed = false;
	g_FixedCenterX = 0.0f;
	g_FixedCenterY = 0.0f;
    g_ShakeTimer = 0.0f;
    g_ShakeDuration = 0.0f;
    g_ShakeAmplitude = 0.0f;
}

void StepCamera()
{

}

void UpdateCamera()
{
    PlayerData& player = GetPlayerData();

    float targetX = 0.0f;
    float targetY = 0.0f;

    if (g_IsFixed)
    {
        targetX = g_FixedCenterX - SCREEN_WIDTH / 2.0f;
        targetY = g_FixedCenterY - SCREEN_HEIGHT / 2.0f;
        camera.posX = targetX;
        camera.posY = targetY;
    }
    else
    {
        float playerCenterX = player.posX + player.width / 2;
        float playerCenterY = player.posY + player.height / 2;

        targetX = playerCenterX - SCREEN_WIDTH / 2;
        targetY = playerCenterY - SCREEN_HEIGHT / 2;

        camera.posX += (targetX - camera.posX) * CAMERA_SPEED;
        camera.posY += (targetY - camera.posY) * CAMERA_SPEED;
    }

    float mapWidth = (float)GetMapWidth();
    float mapHeight = (float)GetMapHeight();

    if (mapWidth <= SCREEN_WIDTH)
    {
        camera.posX = -(SCREEN_WIDTH - mapWidth) * 0.5f;
    }
    else
    {
        float maxX = mapWidth - SCREEN_WIDTH;
        camera.posX = (std::max)(0.0f, (std::min)(camera.posX, maxX));
    }

    if (mapHeight <= SCREEN_HEIGHT)
    {
        camera.posY = -(SCREEN_HEIGHT - mapHeight) * 0.5f;
    }
    else
    {
        float maxY = mapHeight - SCREEN_HEIGHT;
        camera.posY = (std::max)(0.0f, (std::min)(camera.posY, maxY));
    }

    if (g_ShakeTimer > 0.0f)
    {
        const float shakeRatio = (g_ShakeDuration > 0.0f) ? (g_ShakeTimer / g_ShakeDuration) : 0.0f;
        const float amplitude = g_ShakeAmplitude * shakeRatio;
        const float offsetX = ((static_cast<float>(std::rand() % 2001) / 1000.0f) - 1.0f) * amplitude;
        const float offsetY = ((static_cast<float>(std::rand() % 2001) / 1000.0f) - 1.0f) * amplitude * 0.45f;
        camera.posX += offsetX;
        camera.posY += offsetY;
        g_ShakeTimer -= 1.0f / 60.0f;
        if (g_ShakeTimer < 0.0f) g_ShakeTimer = 0.0f;
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

void SetCameraFixed(bool enabled, float centerX, float centerY)
{
    g_IsFixed = enabled;
    g_FixedCenterX = centerX;
    g_FixedCenterY = centerY;
}

bool IsCameraFixed()
{
    return g_IsFixed;
}

void StartCameraShake(float duration, float amplitude)
{
    if (duration <= 0.0f || amplitude <= 0.0f) return;
    if (duration > g_ShakeTimer)
    {
        g_ShakeTimer = duration;
        g_ShakeDuration = duration;
    }
    if (amplitude > g_ShakeAmplitude)
    {
        g_ShakeAmplitude = amplitude;
    }
}

CameraData GetCamera()
{
	return camera;
}