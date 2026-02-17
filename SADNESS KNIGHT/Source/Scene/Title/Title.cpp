#include "DxLib.h"
#include "../SceneManager.h"
#include "Title.h"
#include "../Play/Play.h"
#include "../../Input/Input.h"
#include "../../SaveSystem/SaveSystem.h"

static TitleData title;

const char* g_MenuItems[] =
{
	"はじめから",
	"つづきから",
	"オプション",
	"ゲームを終了する"
};

void InitTitleScene()
{

}

void LoadTitleScene()
{

}

void StartTitleScene()
{

}

void StepTitleScene()
{
	// 入力待ちや
	if (title.m_InputWait > 0)
	{
		title.m_InputWait--;
		return;
	}
}

void UpdateTitleScene()
{

}

void DrawTitleScene()
{

}

void FinTitleScene()
{

}