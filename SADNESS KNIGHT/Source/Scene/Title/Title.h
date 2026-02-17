#pragma once

struct TitleData
{
	int bgHandle;
	int logoHandle;
	int m_InputWait;
};

void InitTitleScene();
void LoadTitleScene();
void StartTitleScene();
void StepTitleScene();
void UpdateTitleScene();
void DrawTitleScene();
void FinTitleScene();
