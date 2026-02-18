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

// カメラ位置とスケールを考慮した変換関数
float WorldToScreenX(float worldX, const CameraData& camera);
float WorldToScreenY(float worldY, const CameraData& camera);

CameraData GetCamera();