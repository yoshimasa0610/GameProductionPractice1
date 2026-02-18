#include "DxLib.h"
#include "../SceneManager.h"
#include "Title.h"
#include "../Play/Play.h"
#include "../../Input/Input.h"
#include "../../SaveSystem/SaveSystem.h"
#include "../../Overlay/Option/Option.h"
#include "../../Map/StageManager.h"
#include "../../Money/MoneyManager.h"
#include "../../Fade/Fade.h"
#include "../../Player/Player.h"
#include "../../Sound/Sound.h"
#include "../../Item/Item.h"

#define TITLE_POS_X 280
#define TITLE_POS_Y 130

#define MENU_POS_X 700
#define MENU_POS_Y 450
#define MENU_INTERVAL 50

// 状態管理
static TitleState g_TitleState = TitleState::MainMenu;

static int g_SelectedMenu = 0;
static int g_SelectedSlot = 0;
static int g_ConfirmIndex = 0;

static bool g_SaveSlotExists[SAVE_SLOT_MAX];

// 画像
static int g_BGHandle = -1;
static int g_TitleHandle = -1;

// 初期化
const char* g_MenuItems[] =
{
	"はじめから",
	"つづきから",
	"オプション",
	"ゲームを終了する"
};

static const int g_MenuCount =
sizeof(g_MenuItems) / sizeof(g_MenuItems[0]);

void InitTitleScene()
{
	g_TitleState = TitleState::MainMenu;

	g_SelectedMenu = 0;
	g_SelectedSlot = 0;
	g_ConfirmIndex = 0;

	InitFade();
	StartFadeInEx(FadeType::Title);
}

void LoadTitleScene()
{

}

void StartTitleScene()
{
	
}

void StepTitleScene()
{

}

static void StartNewGame(int slot)
{
	SaveData data{};
	PlayerData data{};

	// data.maxHP = PLAYER_INIT_HP;

	strcpy_s(data.stageName, "forest_1");

	SaveGame(&data, slot);

	g_CurrentSaveSlot = slot;
	ImportSaveData(&data);
}

static void UpdateSlotSelect(bool isNewGame)
{
	if (IsTriggerKey(KEY_UP))
	{
		g_SelectedSlot = (g_SelectedSlot + SAVE_SLOT_MAX - 1) % SAVE_SLOT_MAX;
	}

	if (IsTriggerKey(KEY_DOWN))
	{
		g_SelectedSlot = (g_SelectedSlot + 1) % SAVE_SLOT_MAX;
	}

	if (IsTriggerKey(KEY_CANCEL))
	{
		g_TitleState = TitleState::MainMenu;
	}

	if (IsTriggerKey(KEY_OK))
	{
		// 続きからでセーブなし
		if (!isNewGame && !g_SaveSlotExists[g_SelectedSlot])
			return;

		// 上書き確認
		if (isNewGame && g_SaveSlotExists[g_SelectedSlot])
		{
			g_ConfirmIndex = 0;
			g_TitleState = TitleState::ConfirmOverwrite;
			return;
		}

		StartNewGame(g_SelectedSlot);

		StartFadeOutEx(FadeType::Title);
		g_TitleState = TitleState::FadingOut;
	}
}

void UpdateTitleScene()
{
	UpdateFade();

	if (IsOptionOpen())
	{
		UpdateOption();
		return;
	}

	if (g_TitleState == TitleState::FadingOut)
	{
		if (IsFadeFinished())
		{
			StopBGM(BGM_TITLE);
			PlayBGM(BGM_PLAY);
			ChangeScene(SCENE_PLAY);
		}
		return;
	}

	switch (g_TitleState)
	{
	case TitleState::MainMenu:

		if (IsTriggerKey(KEY_UP))
		{
			g_SelectedMenu = (g_SelectedMenu + 1) % g_MenuCount;
			PlaySE(SE_MENU_MOVE);
		}

		if (IsTriggerKey(KEY_DOWN))
		{
			g_SelectedMenu = (g_SelectedMenu + 1) % g_MenuCount;
			PlaySE(SE_MENU_MOVE);
		}

		if (IsTriggerKey(KEY_OK))
		{
			PlaySE(SE_MENU_DECIDE);

			switch (g_SelectedMenu)
			{
			case 0:
				g_TitleState = TitleState::SelectSlot_New;
				break;
			case 1:
				g_TitleState = TitleState::SelectSlot_Continue;
				break;
			case 2:
				OpenOption();
				break;
			case 3:
				DxLib_End();
				break;
			}
		}
		break;

	case TitleState::SelectSlot_New:
		UpdateSlotSelect(true);
		break;

	case TitleState::SelectSlot_Continue:
		UpdateSlotSelect(false);
		break;

	case TitleState::ConfirmOverwrite:
		if (IsTriggerKey(KEY_LEFT) || IsTriggerKey(KEY_RIGHT))
		{
			g_ConfirmIndex = 1 - g_ConfirmIndex;
			PlaySE(SE_MENU_MOVE);
		}

		if (IsTriggerKey(KEY_OK))
		{
			PlaySE(SE_MENU_DECIDE);

			if (g_ConfirmIndex == 0)
			{
				// YES → 上書きするんや
				g_CurrentSaveSlot = g_SelectedSlot;
				StartNewGame(g_SelectedSlot);
				StartFadeOut(60);
				g_TitleState = TitleState::FadingOut;
			}
			else
			{
				// NO → スロット選択に戻るんや
				g_TitleState = TitleState::SelectSlot_New;
			}
		}

		if (IsTriggerKey(KEY_CANCEL))
		{
			g_TitleState = TitleState::SelectSlot_New;
		}
		break;
	}
}

void DrawTitleScene()
{
	DrawGraph(0, 0, g_BGHandle, TRUE);
	DrawGraph(TITLE_POS_X, TITLE_POS_Y, g_TitleHandle, TRUE);

	// メインメニュー
	if (g_TitleState == TitleState::MainMenu)
	{
		for (int i = 0; i < g_MenuCount; i++)
		{
			int y = MENU_POS_Y + 1 * MENU_INTERVAL;

			int color = (i == g_SelectedMenu)
				? GetColor(255, 255, 0)
				: GetColor(255, 255, 255);

			DrawString(MENU_POS_X, y, g_MenuItems[i], color);
		}
	}
	else if (g_TitleState == TitleState::SelectSlot_New ||g_TitleState == TitleState::SelectSlot_Continue)
	{
		DrawString(MENU_POS_X - 30, MENU_POS_Y - 40,
			"セーブスロット選択", GetColor(255, 255, 255));

		for (int i = 0; i < SAVE_SLOT_MAX; i++)
		{
			int x = MENU_POS_X - 40;
			int y = MENU_POS_Y + i * 110;

			bool selected = (i == g_SelectedSlot);

			DrawBox(x, y, x + 360, y + 90, selected ? GetColor(255, 255, 0) : GetColor(200, 200, 200), FALSE);

			DrawFormatString(x + 10, y + 10,GetColor(255, 255, 255),"SLOT %d", i + 1);

			SaveData summary;
			if (LoadSaveSummary(i, &summary))
			{
				DrawFormatString(x + 10, y + 35,GetColor(200, 200, 255),"ステージ : %s", summary.stageName);

				DrawFormatString(x + 10, y + 55,GetColor(200, 200, 255),"HP : %d / %d", summary.currentHP, summary.maxHP);
			}
			else
			{
				DrawString(x + 10, y + 45,"セーブデータがありません",GetColor(150, 150, 150));
			}
		}
	}
}

void FinTitleScene()
{
	DeleteGraph(g_BGHandle);
	DeleteGraph(g_TitleHandle);
	StopBGM(BGM_TITLE);
}