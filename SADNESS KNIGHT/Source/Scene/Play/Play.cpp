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
#include "../../BigBoss/Tiphereth/Tiphereth.h"
#include "../../Overlay/CheckpointMenu/CheckpointMenu.h"
#include "../../MidBoss/MidBossBase.h"
#include "../../MidBoss/StoneGolem/StoneGolem.h"
#include "../../MidBoss/Garok/Garok.h"

static ItemField g_ItemField;

PlayerData& player = GetPlayerData();
static PlayDeathState g_DeathState = PlayDeathState::None;
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

// 撃破済みボスの復活防止フラグ（リスポーンでは維持）
static bool g_Forest3MidBossDefeated = false;
static bool g_Forest7MidBossDefeated = false;
static bool g_Forest5BigBossDefeated = false;
static bool g_Forest14BigBossDefeated = false;

void SetBossDefeatSaveFlags(bool forest3MidBossDefeated, bool forest7MidBossDefeated, bool forest5BigBossDefeated)
{
	g_Forest3MidBossDefeated = forest3MidBossDefeated;
	g_Forest7MidBossDefeated = forest7MidBossDefeated;
	g_Forest5BigBossDefeated = forest5BigBossDefeated;
}

void GetBossDefeatSaveFlags(bool& forest3MidBossDefeated, bool& forest7MidBossDefeated, bool& forest5BigBossDefeated)
{
	forest3MidBossDefeated = g_Forest3MidBossDefeated;
	forest7MidBossDefeated = g_Forest7MidBossDefeated;
	forest5BigBossDefeated = g_Forest5BigBossDefeated;
}

static const float FOREST3_STONE_GOLEM_X = 1920.0f;
static const float FOREST3_STONE_GOLEM_Y = 448.0f;
static const float FOREST3_MIDBOSS_CAMERA_X = 1280.0f;
static const float FOREST3_MIDBOSS_CAMERA_Y = 448.0f;
static const float FOREST3_MIDBOSS_AREA_LEFT = 768.0f;
static const float FOREST3_MIDBOSS_AREA_RIGHT = 2816.0f;
static const float FOREST3_MIDBOSS_AREA_TOP = 0.0f;
static const float FOREST3_MIDBOSS_AREA_BOTTOM = 1080.0f;

static const float FOREST5_BOSS_X = 992.0f;
static const float FOREST5_BOSS_Y = 352.0f;
static const float FOREST5_BOSS_AREA_LEFT = 0.0f;
static const float FOREST5_BOSS_AREA_RIGHT = 1920.0f;
static const float FOREST5_BOSS_AREA_TOP = 0.0f;
static const float FOREST5_BOSS_AREA_BOTTOM = 1080.0f;

static const float FOREST7_GAROK_X = 1536.0f;
static const float FOREST7_GAROK_Y = 608.0f;
static const float FOREST7_MIDBOSS_CAMERA_X = 1200.0f;
static const float FOREST7_MIDBOSS_CAMERA_Y = 400.0f;
static const float FOREST7_MIDBOSS_AREA_LEFT = 768.0f;
static const float FOREST7_MIDBOSS_AREA_RIGHT = 3072.0f;
static const float FOREST7_MIDBOSS_AREA_TOP = 0.0f;
static const float FOREST7_MIDBOSS_AREA_BOTTOM = 1080.0f;
static const float FOREST7_DEBUG_PLAYER_X = 1312.0f;
static const float FOREST7_DEBUG_PLAYER_Y = 608.0f;
static const float FOREST14_BOSS_X = 1472.0f;
static const float FOREST14_BOSS_Y = 736.0f;

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
	g_Forest3MidBossDefeated = false;
	g_Forest7MidBossDefeated = false;
	g_Forest5BigBossDefeated = false;
	g_Forest14BigBossDefeated = false;
	SetCameraFixed(false);
}

void LoadPlayScene()
{
	LoadPlayer();
	LoadUIImage();

	SetBossDefeatSaveFlags(
		g_SaveData.forest3MidBossDefeated,
		g_SaveData.forest7MidBossDefeated,
		g_SaveData.forest5BigBossDefeated
	);
	player.hasDoubleJump = g_SaveData.hasDoubleJump;
	player.hasDiveAttack = g_SaveData.hasDiveAttack;

	// SaveData からスタート地点を設定
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

	// セーブ復帰後にバフ再計算
	g_ItemManager.ApplyBuffsToPlayer(&player);

	// =========================
//  デバッグ：Equipアイテムを追加
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

	// Playerの死亡検知
	if (g_DeathState == PlayDeathState::None && !IsPlayerAlive())
	{
		StartFadeOutEx(FadeType::Death);
		g_DeathState = PlayDeathState::WaitFadeOut;
	}

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
		if (stageName && strcmp(stageName, "forest_5") == 0 && !g_Forest5BigBossDefeated)
		{
			SpawnKether(FOREST5_BOSS_X, FOREST5_BOSS_Y);
			g_EnemySpawned = true;
		}
		else if (stageName && strcmp(stageName, "forest_14") == 0 && !g_Forest14BigBossDefeated)
		{
			SpawnTiphereth(FOREST14_BOSS_X, FOREST14_BOSS_Y);
			g_EnemySpawned = true;
		}
	}

	if (!g_MidBossSpawned && IsPlayerAlive() && IsPlayerGrounded())
	{
		const char* stageName = GetCurrentStageName();
		if (stageName && strcmp(stageName, "forest_3") == 0 && !g_Forest3MidBossDefeated)
		{
			SpawnStoneGolem(FOREST3_STONE_GOLEM_X, FOREST3_STONE_GOLEM_Y);
			g_MidBossSpawned = true;
		}
		else if (stageName && strcmp(stageName, "forest_7") == 0 && !g_Forest7MidBossDefeated)
		{
			SpawnGarok(FOREST7_GAROK_X, FOREST7_GAROK_Y);
			g_MidBossSpawned = true;
		}
	}

	const char* currentStage = GetCurrentStageName();
	if (currentStage != nullptr)
	{
		if (strcmp(currentStage, "forest_3") == 0 && g_MidBossSpawned && !IsMidBossAlive())
		{
			g_Forest3MidBossDefeated = true;
		}
		else if (strcmp(currentStage, "forest_7") == 0 && g_MidBossSpawned && !IsMidBossAlive())
		{
			g_Forest7MidBossDefeated = true;
		}
		else if (strcmp(currentStage, "forest_5") == 0 && g_EnemySpawned && !IsBigBossAlive())
		{
			g_Forest5BigBossDefeated = true;
		}
		else if (strcmp(currentStage, "forest_14") == 0 && g_EnemySpawned && !IsBigBossAlive())
		{
			g_Forest14BigBossDefeated = true;
		}
	}

	const bool isForest3 = (currentStage != nullptr && strcmp(currentStage, "forest_3") == 0);
    const bool isForest7 = (currentStage != nullptr && strcmp(currentStage, "forest_7") == 0);
    if (isForest3 || isForest7)
    {
        const float areaLeft = isForest3 ? FOREST3_MIDBOSS_AREA_LEFT : FOREST7_MIDBOSS_AREA_LEFT;
        const float areaRight = isForest3 ? FOREST3_MIDBOSS_AREA_RIGHT : FOREST7_MIDBOSS_AREA_RIGHT;
        const float areaTop = isForest3 ? FOREST3_MIDBOSS_AREA_TOP : FOREST7_MIDBOSS_AREA_TOP;
        const float areaBottom = isForest3 ? FOREST3_MIDBOSS_AREA_BOTTOM : FOREST7_MIDBOSS_AREA_BOTTOM;
        const float cameraX = isForest3 ? FOREST3_MIDBOSS_CAMERA_X : FOREST7_MIDBOSS_CAMERA_X;
        const float cameraY = isForest3 ? FOREST3_MIDBOSS_CAMERA_Y : FOREST7_MIDBOSS_CAMERA_Y;

        const float playerCenterX = player.posX + player.width * 0.5f;
        const float playerCenterY = player.posY + player.height * 0.5f;
        const bool inMidBossArea =
            (playerCenterX >= areaLeft && playerCenterX <= areaRight &&
             playerCenterY >= areaTop && playerCenterY <= areaBottom);

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
            SetCameraFixed(true, cameraX, cameraY);
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
        if (!(isForest3 || isForest7) || !g_MidBossCameraLocked)
        {
            SetCameraFixed(false);
        }
        if (!g_MidBossStageLockActive)
        {
            UnlockStageTransition(); // 念のため
        }
    }

	UpdateCamera();

	switch (g_DeathState)
	{
	case PlayDeathState::WaitFadeOut:
		if (IsFadeOutFinished())
		{
			g_DeathState = PlayDeathState::Respawning;
		}
		break;

	case PlayDeathState::Respawning:
		RespawnFromCheckpoint();

		StartFadeInEx(FadeType::Death);
		g_DeathState = PlayDeathState::FadeIn;
		break;

	case PlayDeathState::FadeIn:
		if (IsFadeFinished())
		{
			g_DeathState = PlayDeathState::None;
		}
		break;
	}

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
	// 現在メモリ上の進行状態を維持したままチェックポイントへ復帰
	const SaveData& data = g_SaveData;

	// ボス関連のロック状態をリセット
	g_BossCameraLocked = false;
	g_MidBossCameraLocked = false;
	g_MidBossStageLockActive = false;
	g_BigBossStageLockActive = false;
	g_IsBossBGMPlaying = false;
	SetCameraFixed(false);
	UnlockStageTransition();

	// 敵・ボスはクリアしてステージを再構築（撃破済みフラグは維持）
	ClearEnemies();
	ClearMidBosses();
	ClearBigBosses();
	g_EnemySpawned = false;
	g_MidBossSpawned = false;

	InitStage();
	LoadStage(
		data.stageName,
		(float)data.checkpointX,
		(float)data.checkpointY
	);
	InitCheckpoint(data.stageName);
	g_ItemField.LoadForStage(data.stageName);

	// パッシブや育成状態を保持したまま死亡状態のみ解除
	ResetPlayerDeath();
	player.posX = (float)data.checkpointX;
	player.posY = (float)data.checkpointY;
	player.velocityX = 0.0f;
	player.velocityY = 0.0f;
	player.isGrounded = false;
	player.jumpCount = 0;

	// 所持バフの再適用
	g_ItemManager.ApplyBuffsToPlayer(&player);

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