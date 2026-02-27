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
#include "../../Overlay/Option/Option.h"
#include "../../Animation/Animation.h"

#define TITLE_POS_X 280
#define TITLE_POS_Y 130

#define MENU_POS_X 700
#define MENU_POS_Y 550
#define MENU_INTERVAL 50

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SLOT_WIDTH 1050
#define SLOT_HEIGHT 180
#define SLOT_SPACING 210

// 状態管理
static TitleState g_TitleState = TitleState::MainMenu;

static int g_SelectedMenu = 0;
static int g_SelectedSlot = 0;
static int g_ConfirmIndex = 0;

static bool g_SaveSlotExists[SAVE_SLOT_MAX];

// 画像
static int g_BGHandle = -1;
static int g_TitleHandle = -1;

static int g_SelectFrameHandle = -1;

// 初期化
const char* g_MenuItems[] =
{
	"はじめから",
	"つづきから",
	"オプション",
	"ゲーム終了"
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
	g_BGHandle = LoadGraph("Data/Title/TitleBG.png");
	g_TitleHandle = LoadGraph("Data/Title/TitleText.png");

	g_SelectFrameHandle = LoadGraph("Data/UI/UI_Elements01.png");
}

void StartTitleScene()
{
	PlayBGM(BGM_TITLE);
	for (int i = 0; i < SAVE_SLOT_MAX; i++)
	{
		g_SaveSlotExists[i] = DoesSaveExist(i);
	}
}

void StepTitleScene()
{

}

static void StartNewGame(int slot)
{
	SaveData data{};

	 data.maxHP = PLAYER_INIT_HP;

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

		g_CurrentSaveSlot = g_SelectedSlot;

		if (isNewGame)
		{
			StartNewGame(g_SelectedSlot);
		}
		else
		{
			SaveData data;
			if (LoadGame(&data, g_SelectedSlot))
			{
				ImportSaveData(&data);
			}
		}

		StartFadeOut(60);
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
		UpdateFade();

		if (IsFadeOutFinished())
		{
			StopBGM(BGM_TITLE);
			PlayBGM(BGM_PLAY);
			ResetInput();
			ChangeScene(SCENE_PLAY);
		}
		return;
	}

	switch (g_TitleState)
	{
	case TitleState::MainMenu:

		if (IsTriggerKey(KEY_UP))
		{
			g_SelectedMenu = (g_SelectedMenu + g_MenuCount - 1) % g_MenuCount;
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
	// 背景
	DrawGraph(0, 0, g_BGHandle, TRUE);
	DrawGraph(TITLE_POS_X, TITLE_POS_Y, g_TitleHandle, TRUE);

	// スロット画面のときは背景を暗くするんや
	if (g_TitleState == TitleState::SelectSlot_New ||
		g_TitleState == TitleState::SelectSlot_Continue ||
		g_TitleState == TitleState::ConfirmOverwrite)
	{
		int screenW, screenH;
		GetDrawScreenSize(&screenW, &screenH);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
		DrawBox(0, 0, screenW, screenH, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// メインメニュー
	if (g_TitleState == TitleState::MainMenu)
	{
		for (int i = 0; i < g_MenuCount; i++)
		{
			int y = MENU_POS_Y + i * MENU_INTERVAL;
			int x = MENU_POS_X;

			DrawString(x, y, g_MenuItems[i], GetColor(255, 255, 255));

			if (i == g_SelectedMenu)
			{
				int textWidth = GetDrawStringWidth(
					g_MenuItems[i],
					strlen(g_MenuItems[i])
				);

				int paddingX = 40;
				int paddingY = 20;

				int textLeft = x - textWidth / 8;

				int frameW = textWidth + paddingX * 2;

				int frameX = textLeft - paddingX;

				int frameY = y - paddingY;

				int drawH = 60;

				DrawRectExtendGraph(
					frameX,
					frameY,
					frameX + 90,
					frameY + drawH,
					0, 95,
					90, 85,
					g_SelectFrameHandle,
					TRUE
				);

				DrawRectExtendGraph(
					frameX + frameW - 45,
					frameY,
					frameX + frameW,
					frameY + drawH,
					280, 95,
					45, 85,
					g_SelectFrameHandle,
					TRUE
				);
			}
		}
	}

	// スロット選択
	else if (g_TitleState == TitleState::SelectSlot_New ||g_TitleState == TitleState::SelectSlot_Continue)
	{
		int offsetX = 150;

		int titleW = GetDrawStringWidth("データ選択", strlen("データ選択"));
		DrawString((SCREEN_WIDTH - titleW) / 2 + offsetX,120,"データ選択",GetColor(255, 255, 255));

		for (int i = 0; i < SAVE_SLOT_MAX; i++)
		{
			bool selected = (i == g_SelectedSlot);

			int width = SLOT_WIDTH;
			int height = SLOT_HEIGHT;

			int x = (SCREEN_WIDTH - width) / 2 + offsetX;

			int y = 200 + i * SLOT_SPACING;

			int frameColor = selected? GetColor(255, 255, 255): GetColor(80, 80, 80);

			DrawBox(x, y, x + width, y + height, frameColor, FALSE);

			int textColor = selected? GetColor(255, 255, 255): GetColor(150, 150, 150);

			DrawFormatString(x + 30, y + 20, textColor,"SLOT %d", i + 1);

			SaveData summary;
			if (LoadSaveSummary(i, &summary))
			{
				DrawFormatString(x + 200, y + 25, textColor,"ステージ : %s", summary.stageName);

				DrawFormatString(x + 200, y + 60, textColor,"HP : %d / %d",summary.currentHP,summary.maxHP);
			}
			else
			{
				DrawString(x + 200, y + 45,"新規作成", textColor);
			}
		}
	}

	// 上書き確認
	else if (g_TitleState == TitleState::ConfirmOverwrite)
	{
		int offsetX = 150;
		int offsetY = 80;

		int boxW = 600;
		int boxH = 200;

		int x = (SCREEN_WIDTH - boxW) / 2 + offsetX;
		int y = (SCREEN_HEIGHT - boxH) / 2 + offsetY;

		DrawBox(x, y, x + boxW, y + boxH,GetColor(255, 255, 255), FALSE);

		const char* msg = "セーブデータを上書きしますか？";int msgW = GetDrawStringWidth(msg, strlen(msg));

		DrawString(x + (boxW - msgW) / 2,y + 40,msg,GetColor(255, 255, 255));

		int yesColor = (g_ConfirmIndex == 0)? GetColor(255, 255, 0): GetColor(255, 255, 255);

		int noColor = (g_ConfirmIndex == 1)? GetColor(255, 255, 0): GetColor(255, 255, 255);

		const char* yes = "はい";
		const char* no = "いいえ";

		int yesW = GetDrawStringWidth(yes, strlen(yes));
		int noW = GetDrawStringWidth(no, strlen(no));

		int buttonSpacing = 100;

		int centerX = x + boxW / 2;

		DrawString(centerX - buttonSpacing - yesW / 2,y + 120,yes,yesColor);

		DrawString(centerX + buttonSpacing - noW / 2,y + 120,no,noColor);
	}

	if (g_TitleState == TitleState::SelectSlot_New ||g_TitleState == TitleState::SelectSlot_Continue)
	{
		const char* guide1 = "SPACE : 決定";
		const char* guide2 = "A   : 戻る";

		int colorKey = GetColor(255, 255, 255);
		int colorText = GetColor(150, 150, 150);

		// 右寄せ用に文字幅取得
		int w1 = GetDrawStringWidth(guide1, strlen(guide1));
		int w2 = GetDrawStringWidth(guide2, strlen(guide2));

		int x1 = SCREEN_WIDTH - w1 + 300;
		int x2 = SCREEN_WIDTH - w2 + 300;

		int y2 = SCREEN_HEIGHT + 150;
		int y1 = y2 - 25;

		// 少し透明にするんやな。どうすんの？真希ちゃん
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);

		DrawString(x1, y1, guide1, colorText);
		DrawString(x2, y2, guide2, colorText);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// オプション描画
	if (IsOptionOpen())
	{
		DrawOption();
	}
}

void FinTitleScene()
{
	DeleteGraph(g_BGHandle);
	DeleteGraph(g_TitleHandle);
	StopBGM(BGM_TITLE);
}