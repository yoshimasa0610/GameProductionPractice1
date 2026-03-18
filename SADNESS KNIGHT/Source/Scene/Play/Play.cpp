#include "DxLib.h"
#include "../../Scene/SceneManager.h"
#include "../../Map/MapManager.h"
#include "../../Player/Player.h"
#include "../../Camera/Camera.h"
#include "../../Collision/Collision.h"
#include "../../Input/Input.h"
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
#include "../../Overlay/OverlayMenu.h"
#include "../../Skill/SkillData.h"
#include "../../Skill/SkillManager.h"
#include "../../Enemy/EnemyBase.h"
#include "../../BigBoss/BigBossBase.h"
#include "../../BigBoss/Kether/Kether.h"

static ItemField g_ItemField;

PlayerData& player = GetPlayerData();
extern SaveData  g_SaveData;

// --- CSVロード用デバッグ表示 ---
static char g_SpawnLoadDebug[256] = { 0 };

//時間
static float g_ElapsedTime = 0.0f;
static bool g_IsPaused = false;
static bool g_EnemySpawned = false;
static bool g_BossCameraLocked = false;

static const float FOREST5_BOSS_X = 992.0f;
static const float FOREST5_BOSS_Y = 352.0f;
static const float FOREST5_BOSS_AREA_LEFT = 760.0f;
static const float FOREST5_BOSS_AREA_RIGHT = 1240.0f;
static const float FOREST5_BOSS_AREA_TOP = 80.0f;
static const float FOREST5_BOSS_AREA_BOTTOM = 760.0f;

void InitPlayScene()
{
	InitCamera();
	InitMoneyDrops();
	InitMoneyPopup();
	InitEnemySystem();
	InitBigBossSystem();
	InitPlayer(544.0f, 384.0f);
	g_EnemySpawned = false;
	g_BossCameraLocked = false;
	SetCameraFixed(false);
}

void LoadPlayScene()
{
	LoadPlayer();
	LoadUIImage();

	// SaveData からステージをロード
	if (g_SaveData.stageName[0] == '\0')
	{
		strcpy_s(g_SaveData.stageName, "forest_1");
		g_SaveData.checkpointX = 544.0f;
		g_SaveData.checkpointY = 384.0f;
	}
	
#ifdef _DEBUG
	// デバッグ用：forest_5ステージでボスの近くにスポーン
	strcpy_s(g_SaveData.stageName, "forest_5");
	g_SaveData.checkpointX = 960.0f;
	g_SaveData.checkpointY = 352.0f;
#endif
	
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

	// =========================
//  デバッグ：Equipアイテム強制所持
// =========================
#ifdef _DEBUG

	ItemManager_AddItem(10);
	ItemManager_AddItem(11);
	ItemManager_AddItem(12);
	g_SkillManager.AddSkill(GetSkillData(1), &player);
	g_SkillManager.AddSkill(GetSkillData(2), &player);
	g_SkillManager.EquipSkill(0, 0, 1);
	g_SkillManager.EquipSkill(0, 1, 2);
	// バフ再計算
	g_ItemManager.ApplyBuffsToPlayer(&player);

#endif
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
	
	g_EnemySpawned = false;
	g_BossCameraLocked = false;
	SetCameraFixed(false);
}


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
	if (g_IsPaused) return;

	float deltaTime = 1.0f / 60.0f;
	float slowMoScale = GetDeathSlowMotionScale();
	g_ElapsedTime += deltaTime * slowMoScale;

	UpdateEnemies();
	UpdateBigBosses();

	UpdateMoneyDrops(
		player.posX,
		player.posY
	);

	g_ItemField.Update(
		GetPlayerPosX(),
		GetPlayerPosY(),
		(float)PLAYER_WIDTH,
		(float)PLAYER_HEIGHT
	);

	auto picked = g_ItemField.FetchPickedItems();
	for (int id : picked)
	{
		ItemManager_AddItem(id);
		g_ItemManager.ApplyBuffsToPlayer(&player);
		ExportSaveData(&g_SaveData);
		SaveGame(&g_SaveData, g_CurrentSaveSlot);
	}
	UpdateMoneyPopup((1.0f / 60.0f) * slowMoScale);
}

static void RespawnFromCheckpoint();

void UpdatePlayScene()
{
	if (g_IsPaused && !IsOverlayOpen() && !g_IsMenuOpen && !IsOptionOpen())
	{
		g_IsPaused = false;
	}

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

	if (IsOverlayOpen())
	{
		UpdateOverlayMenu();
		return;
	}

	if (IsTriggerKey(KEY_MENU))
	{
		OpenMenu();
		return;
	}

	if (IsTriggerKey(KEY_INVENTORY))
	{
		SetPaused(true);
		OpenOverlayMenu(&player);
		return;
	}

	if (CheckHitKey(KEY_INPUT_F))
	{
		ChangeScene(SCENE_CLEAR);
	}

	UpdateCheckpoint(
		&g_SaveData,
		(int)player.posX,
		(int)player.posY,
		(int)PLAYER_WIDTH,
		(int)PLAYER_HEIGHT
	);

	if (g_IsPaused) return;

	UpdatePlayer();

	if (!g_EnemySpawned && IsPlayerAlive() && IsPlayerGrounded())
	{
		const char* stageName = GetCurrentStageName();
		if (stageName != nullptr && strcmp(stageName, "forest_5") == 0)
		{
			SpawnKether(FOREST5_BOSS_X, FOREST5_BOSS_Y);
			g_EnemySpawned = true;
		}
	}

	const char* currentStage = GetCurrentStageName();
	const bool isForest5 = (currentStage != nullptr && strcmp(currentStage, "forest_5") == 0);
	if (isForest5)
	{
		const float playerCenterX = player.posX + player.width * 0.5f;
		const float playerCenterY = player.posY + player.height * 0.5f;
		const bool inBossArea =
			(playerCenterX >= FOREST5_BOSS_AREA_LEFT && playerCenterX <= FOREST5_BOSS_AREA_RIGHT &&
			 playerCenterY >= FOREST5_BOSS_AREA_TOP && playerCenterY <= FOREST5_BOSS_AREA_BOTTOM);

		if (!g_BossCameraLocked && inBossArea)
		{
			g_BossCameraLocked = true;
		}

		if (g_BossCameraLocked)
		{
			SetCameraFixed(true, FOREST5_BOSS_X, FOREST5_BOSS_Y);
			if (!IsBigBossAlive())
			{
				g_BossCameraLocked = false;
				SetCameraFixed(false);
			}
		}
		else
		{
			SetCameraFixed(false);
		}
	}
	else
	{
		g_BossCameraLocked = false;
		SetCameraFixed(false);
	}

	UpdateCamera();
	UpdateFade();
	UpdateUIImage();
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

	DrawEnemies();
	DrawBigBosses();
	DrawPlayer();
	DrawUIImage();
	g_MoneyManager.Draw();

	if (IsOptionOpen())

	{
		DrawOption();
		return;
	}

	if (g_IsMenuOpen)
	{
		DrawMenu();
	}

	if (IsOverlayOpen())
	{
		DrawOverlayMenu();
	}
	DrawFade();
}

void FinPlayScene()
{
	UnloadUIImage();
	// マップ終了
	FinStage();
	ClearBigBosses();
	g_BossCameraLocked = false;
	SetCameraFixed(false);

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