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
#include "../../Overlay/CheckpointMenu/CheckpointMenu.h"
#include "../../MidBoss/MidBossBase.h"
#include "../../MidBoss/Door of truth/StoneGolem.h"

static ItemField g_ItemField;

PlayerData& player = GetPlayerData();
extern SaveData  g_SaveData;

// --- CSVロード用デバッグ表示 ---
static char g_SpawnLoadDebug[256] = { 0 };

//時間
static float g_ElapsedTime = 0.0f;
static bool g_IsPaused = false;
static bool g_EnemySpawned = false;
static bool g_MidBossSpawned = false;
static char g_LastSpawnStage[64] = "";
static bool g_BossCameraLocked = false;
static bool g_MidBossCameraLocked = false;
static bool g_MidBossStageLockActive = false;
static bool g_BigBossStageLockActive = false;

static const float FOREST3_STONE_GOLEM_X = 1920.0f;
static const float FOREST3_STONE_GOLEM_Y = 448.0f;
static const float FOREST3_MIDBOSS_CAMERA_X = 1600.0f;
static const float FOREST3_MIDBOSS_CAMERA_Y = 448.0f;
static const float FOREST3_MIDBOSS_AREA_LEFT = 768.0f;
static const float FOREST3_MIDBOSS_AREA_RIGHT = 2816.0f;
static const float FOREST3_MIDBOSS_AREA_TOP = 0.0f;
static const float FOREST3_MIDBOSS_AREA_BOTTOM = 1080.0f;
static const float FOREST5_BOSS_X = 992.0f;
static const float FOREST5_BOSS_Y = 352.0f;
// ボスエリアロック範囲をさらに広げる
static const float FOREST5_BOSS_AREA_LEFT = 0.0f;
static const float FOREST5_BOSS_AREA_RIGHT = 1920.0f;
static const float FOREST5_BOSS_AREA_TOP = 0.0f;
static const float FOREST5_BOSS_AREA_BOTTOM = 1080.0f;
static bool g_IsBossBGMPlaying = false;
// --- ボスエリア遷移ロック ---
static bool g_StageLocked = false;
void LockStageTransition() { g_StageLocked = true; }
void UnlockStageTransition() { g_StageLocked = false; }
bool IsStageLocked() { return g_StageLocked; }

void InitPlayScene()
{
	InitCamera();
	InitMoneyDrops();
	InitMoneyPopup();
	InitEnemySystem();
	InitMidBossSystem();
	InitBigBossSystem();
	InitPlayer(544.0f, 384.0f);
	g_EnemySpawned = false;
	g_MidBossSpawned = false;
	g_BossCameraLocked = false;
	g_MidBossCameraLocked = false;
	g_MidBossStageLockActive = false;
	g_BigBossStageLockActive = false;
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
	//g_SkillManager.AddSkill(GetSkillData(2), &player);
	g_SkillManager.EquipSkill(0, 0, 1);
	//g_SkillManager.EquipSkill(0, 1, 2);
	// バフ再計算
	g_ItemManager.ApplyBuffsToPlayer(&player);

#endif
}

void StartPlayScene()
{
	InitFade();

	BGMType bgm = GetStageBGM(GetCurrentStageName());
	// 現在のBGMで判定
	if (!IsFadingBGM() && GetCurrentBGM() != bgm)
	{
		PlayBGM(bgm);
	}

	
	g_EnemySpawned = false;
	g_MidBossSpawned = false;
	g_BossCameraLocked = false;
	g_MidBossCameraLocked = false;
	g_MidBossStageLockActive = false;
	g_BigBossStageLockActive = false;
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
	UpdateMidBosses();
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
	if (g_IsPaused && !IsCheckpointMenuOpen() &&
		!IsOverlayOpen() &&
		!g_IsMenuOpen &&
		!IsOptionOpen())
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

	if (IsCheckpointMenuOpen())
	{
		UpdateCheckpointMenu();
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

	if (IsTriggerKey(KEY_INVENTORY) && !IsPlayerSitting())
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

	const char* stageForSpawn = GetCurrentStageName();
	if (stageForSpawn != nullptr)
	{
		if (strcmp(g_LastSpawnStage, stageForSpawn) != 0)
		{
			strcpy_s(g_LastSpawnStage, stageForSpawn);
			g_EnemySpawned = false;
			g_MidBossSpawned = false;
		}
	}

	if (!g_EnemySpawned && IsPlayerAlive() && IsPlayerGrounded())
	{
		const char* stageName = GetCurrentStageName();
		if (stageName && strcmp(stageName, "forest_5") == 0)
		{
			SpawnKether(FOREST5_BOSS_X, FOREST5_BOSS_Y);
			g_EnemySpawned = true;
		}
	}

	if (!g_MidBossSpawned && IsPlayerAlive() && IsPlayerGrounded())
	{
		const char* stageName = GetCurrentStageName();
		if (stageName && strcmp(stageName, "forest_3") == 0)
		{
			SpawnStoneGolem(FOREST3_STONE_GOLEM_X, FOREST3_STONE_GOLEM_Y);
			g_MidBossSpawned = true;
		}
	}

	const char* currentStage = GetCurrentStageName();
	const bool isForest3 = (currentStage != nullptr && strcmp(currentStage, "forest_3") == 0);
	if (isForest3)
	{
		const float playerCenterX = player.posX + player.width * 0.5f;
		const float playerCenterY = player.posY + player.height * 0.5f;
		const bool inMidBossArea =
			(playerCenterX >= FOREST3_MIDBOSS_AREA_LEFT && playerCenterX <= FOREST3_MIDBOSS_AREA_RIGHT &&
			 playerCenterY >= FOREST3_MIDBOSS_AREA_TOP && playerCenterY <= FOREST3_MIDBOSS_AREA_BOTTOM);

		if (!g_MidBossStageLockActive && inMidBossArea && IsMidBossAlive())
		{
			g_MidBossStageLockActive = true;
			LockStageTransition();
		}
		if (g_MidBossStageLockActive && !IsMidBossAlive())
		{
			g_MidBossStageLockActive = false;
			if (!g_BigBossStageLockActive)
			{
				UnlockStageTransition();
			}
		}

		if (!g_MidBossCameraLocked && inMidBossArea && IsMidBossAlive())
		{
			g_MidBossCameraLocked = true;
		}
		if (g_MidBossCameraLocked)
		{
			SetCameraFixed(true, FOREST3_MIDBOSS_CAMERA_X, FOREST3_MIDBOSS_CAMERA_Y);
			if (!IsMidBossAlive())
			{
				g_MidBossCameraLocked = false;
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
		g_MidBossCameraLocked = false;
	}

	const bool isForest5 = (currentStage != nullptr && strcmp(currentStage, "forest_5") == 0);
	if (isForest5)
    {
        const float playerCenterX = player.posX + player.width * 0.5f;
        const float playerCenterY = player.posY + player.height * 0.5f;
        const bool inBossArea =
            (playerCenterX >= FOREST5_BOSS_AREA_LEFT && playerCenterX <= FOREST5_BOSS_AREA_RIGHT &&
             playerCenterY >= FOREST5_BOSS_AREA_TOP && playerCenterY <= FOREST5_BOSS_AREA_BOTTOM);

        if (!g_BigBossStageLockActive && inBossArea && IsBigBossAlive())
        {
            g_BigBossStageLockActive = true;
            LockStageTransition(); // ボスエリア突入でロック
        }
		// ボスエリア侵入時にBGM切り替え
		if (inBossArea && IsBigBossAlive() && !g_IsBossBGMPlaying)
		{
			if (!IsFadingBGM())
			{
				FadeChangeBGM(BGM_KETHER, 60);
				g_IsBossBGMPlaying = true;
			}
		}
        if (g_BigBossStageLockActive && !IsBigBossAlive())
        {
            g_BigBossStageLockActive = false;
            if (!g_MidBossStageLockActive)
            {
                UnlockStageTransition(); // ボス撃破でロック解除
            }
        }
		if (g_IsBossBGMPlaying && !IsBigBossAlive())
		{
			if (!IsFadingBGM())
			{
				BGMType stageBGM = GetStageBGM(GetCurrentStageName());
				FadeChangeBGM(stageBGM, 60);
				g_IsBossBGMPlaying = false;
			}
		}
        // カメラ固定ロジックはそのまま
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
        if (!isForest3 || !g_MidBossCameraLocked)
        {
            SetCameraFixed(false);
        }
        if (!g_MidBossStageLockActive)
        {
            UnlockStageTransition(); // 念のため
        }
    }

	UpdateCamera();
	UpdateFade();
	UpdateUIImage();
}

void DrawPlayScene()
{
	DrawBackgroundFar();
	DrawBackgroundMid();

	g_ItemField.Draw();
	DrawMap();
	DrawCheckpoint();
	DrawMoneyDrops();
	DrawMapOutsideMask();
	DrawForeground();

	DrawEnemies();
	DrawMidBosses();
	DrawBigBosses();
	DrawPlayer();
	DrawUIImage();
	g_MoneyManager.Draw();

	if (IsOptionOpen())
	{
		DrawOption();
		return;
	}

	if (IsCheckpointMenuOpen())
	{
		DrawCheckpointMenu();
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
	ClearMidBosses();
	ClearBigBosses();
	g_BossCameraLocked = false;
	g_MidBossCameraLocked = false;
	g_MidBossStageLockActive = false;
	g_BigBossStageLockActive = false;
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

// --- ステージ切替禁止ガード ---
#define GUARD_BOSS_STAGE_LOCK if (g_BossStageLocked) return;
// 例: ChangeScene, ChangeStage, LoadStage, FinStage などの直前で
// GUARD_BOSS_STAGE_LOCK
// --- ステージ切替系関数の冒頭に以下を追加してください ---
// void ChangeScene(...) { GUARD_BOSS_STAGE_LOCK; ... }
// void ChangeStage(...) { GUARD_BOSS_STAGE_LOCK; ... }
// void LoadStage(...) { GUARD_BOSS_STAGE_LOCK; ... }
// void FinStage(...) { GUARD_BOSS_STAGE_LOCK; ... }
// ...既存のChangeSceneやChangeStage呼び出しの直前に挿入してください...