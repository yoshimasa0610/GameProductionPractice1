#pragma once

struct CameraData
{
	float posX;
	float posY;
};

void InitCamera();
void StepCamera();
void UpdateCamera();
void DrawCamera();
void FinalCamera();

CameraData GetCamera();