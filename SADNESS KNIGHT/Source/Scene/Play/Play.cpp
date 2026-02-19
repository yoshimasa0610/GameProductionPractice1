#include "DxLib.h"
#include "../../Scene/SceneManager.h"
#include "../../Map/MapManager.h"
#include "../../Player/Player.h"
#include "../../Camera/Camera.h"
#include "../../Collision/Collision.h"
#include "../../Input/Input.h"
#include "../../Overlay/Equip/EquipMenu.h"
#include "../../UIImage/UIImage.h"
#include "../../Map/StageManager.h"
#include "../../Map/Checkpoint/CheckpointManager.h"
#include "../../SaveSystem/SaveSystem.h"
#include "../../Map/Checkpoint/Checkpoint.h"
#include "Play.h"
#include "../../Overlay/Menu/Menu.h"
#include "../../Overlay/Option/Option.h"
#include "../../Money/MoneyPopup.h"
#include "../../Fade/Fade.h"
#include "../../Money/MoneyDropSystem.h"
#include "../../Sound/Sound.h"
#include "../../Item/ItemField.h"
#include "../../Money/MoneyManager.h"

static ItemField g_ItemField;
PlayerData& player = GetPlayerData();
extern SaveData  g_SaveData;

// --- CSVロード用デバッグ表示 ---
static char g_SpawnLoadDebug[256] = { 0 };

//時間
static float g_ElapsedTime = 0.0f;

void InitPlayScene()
{
	InitMoneyDrops();
	InitMoneyPopup();
}

void LoadPlayScene()
{
	LoadPlayer();

	// SaveData からステージをロード
	if (g_SaveData.stageName[0] == '\0')
	{
		strcpy_s(g_SaveData.stageName, "forest_1");
		g_SaveData.checkpointX = 100;
		g_SaveData.checkpointY = 386;
	}
	InitStage();
	LoadStage(
		g_SaveData.stageName,
		(float)g_SaveData.checkpointX,
		(float)g_SaveData.checkpointY
	);
	InitCheckpoint(g_SaveData.stageName);

	// フィールドアイテム
	g_ItemField.LoadForStage(GetCurrentStageName());

	// セーブ復元後にバフ再計算
	g_ItemManager.ApplyBuffsToPlayer(&player);
}

static BGMType GetStageBGM(const char* stageName)
{
	// デフォルト
	return BGM_PLAY;
}

void StartPlayScene()
{
	InitFade();

	BGMType bgm = GetStageBGM(GetCurrentStageName());
	PlayBGM(bgm);
}

// --- ポーズ制御フラグ ---
static bool g_IsPaused = false;

void SetPaused(bool paused)
{
	g_IsPaused = paused;
}
// めちゃくちゃざっくりいうと、ゲームの進行を一時停止するためのフラグと関数です。
bool IsPaused()
{
	return g_IsPaused;
}

void StepPlayScene()
{
	if (g_IsPaused) return; // ポーズ中は進行を止める

	float deltaTime = 1.0f / 60.0f;
	g_ElapsedTime += deltaTime;

	// お金更新（吸い寄せ & 回収）
	UpdateMoneyDrops(
		player.posX,
		player.posY
	);

	// ---- フィールドアイテムとの当たり判定 ----
	g_ItemField.Update(
		GetPlayerPosX(),
		GetPlayerPosY(),
		(float)PLAYER_WIDTH,
		(float)PLAYER_HEIGHT
	);

	// ---- このフレーム拾ったアイテム取得 ----
	auto picked = g_ItemField.FetchPickedItems();
	for (int id : picked)
	{
		// 所持にする
		ItemManager_AddItem(id);
		g_ItemManager.ApplyBuffsToPlayer(&player);
		// 即セーブ（チェックポイント制なら後回しでもOK）
		ExportSaveData(&g_SaveData);
		SaveGame(&g_SaveData, g_CurrentSaveSlot);
	}
	UpdateMoneyPopup(1.0f / 60.0f);
}

//前方宣言
static void RespawnFromCheckpoint();

void UpdatePlayScene()
{
	/*
	// ===== 死亡チェック =====
	if (g_PlayerData.lifeState == PlayerLifeState::FadingOut &&
		g_DeathState == PlayDeathState::None)
	{
		g_DeathState = PlayDeathState::WaitFadeOut;
	}

	if (g_DeathState == PlayDeathState::WaitFadeOut)
	{
		UpdateFade();

		if (IsFadeOutFinished())
		{
			RespawnFromCheckpoint();

			// フェードイン開始
			StartFadeIn(60);
			g_DeathState = PlayDeathState::FadeIn;
		}
		return;
	}

	if (g_DeathState == PlayDeathState::FadeIn)
	{
		UpdateFade();

		if (IsFadeFinished())
		{
			g_DeathState = PlayDeathState::None;
		}
		return;
	}
	*/
	if (IsOptionOpen())
	{
		UpdateOption();
		return;
	}

	if (g_IsMenuOpen)
	{
		UpdateMenu();
		return;
	}

	if (g_IsEquipMenuOpen)
	{
		// PlayScene の更新を止める
		UpdateEquipMenuScene();
		return;
	}

	if (IsTriggerKey(KEY_MENU))
	{
		OpenMenu();
		return;
	}

	if (IsTriggerKey(KEY_INVENTORY))
	{
		//シーンが変わっている間は一時中止
		SetPaused(true);
		// 装備変更自体は椅子判定でチェックするため、ここでは false にして開くだけ
		SetEquipMode(IsPlayerSitting());

		// プレイヤーデータ参照を渡す（EquipMenu が参照するため）
		SetEquipMenuPlayer(&player);

		OpenEquipMenu(&player);
		return;
	}

	// チェックポイント更新
	UpdateCheckpoint(
		&g_SaveData,
		(int)player.posX,
		(int)player.posY,
		(int)PLAYER_WIDTH,
		(int)PLAYER_HEIGHT
	);

	if (g_IsPaused) return; // ポーズ中は更新を止める

	// プレイヤー更新とか

	UpdateFade();
}

void DrawPlayScene()
{
	DrawBackgroundFar();
	DrawBackgroundMid();

	DrawMap();
	DrawCheckpoint();
	g_ItemField.Draw();
	DrawMoneyDrops();
	DrawMapOutsideMask();
	DrawForeground();

	g_MoneyManager.Draw();
	DrawMoneyPopup();

	if (IsOptionOpen())
	{
		DrawOption();
		return;
	}

	if (g_IsMenuOpen)
	{
		DrawMenu();
	}

	if (g_IsEquipMenuOpen)
	{
		DrawEquipMenuScene(); // 半透明 UI
	}
	DrawFade();
}

void FinPlayScene()
{
	// マップ終了
	FinStage();

}

static void RespawnFromCheckpoint()
{
	//プレイヤーが死亡した際、復活地点から再開するための処理
	//ResetPlayerDeath();

	// セーブデータをロードし直す
	if (DoesSaveExist(g_CurrentSaveSlot))
	{
		SaveData data;
		if (LoadGame(&data, g_CurrentSaveSlot))
		{
			ImportSaveData(&data);
			g_ItemManager.ApplyBuffsToPlayer(&player);
			// ステージ再ロード
			InitStage();
			LoadStage(
				data.stageName,
				(float)data.checkpointX,
				(float)data.checkpointY
			);
			InitCheckpoint(data.stageName);
		}
	}

	SetPaused(false);
}