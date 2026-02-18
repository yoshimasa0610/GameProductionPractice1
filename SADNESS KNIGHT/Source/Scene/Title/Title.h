#pragma once

// ƒ^ƒCƒgƒ‹‰æ–Ê‚Ìó‘Ô‚â
enum class TitleState
{
	MainMenu,
	SelectSlot_New,
	SelectSlot_Continue,
	ConfirmOverwrite,
	FadingOut
};

void InitTitleScene();
void LoadTitleScene();
void StartTitleScene();
void StepTitleScene();
void UpdateTitleScene();
void DrawTitleScene();
void FinTitleScene();
