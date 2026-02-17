#pragma once

struct CameraData
{
	float posX;
	float posY;
	float scale;//ズームや画面を引き伸ばすためのもの。1.0fが等倍、2.0fなら2倍、0.5fなら半分など。
};

void InitCamera();
void StepCamera();
void UpdateCamera();
void DrawCamera();
void FinalCamera();

CameraData GetCamera();