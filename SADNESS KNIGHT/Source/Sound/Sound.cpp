#include "DxLib.h"
#include "Sound.h"
#include "../Scene/Title/Title.h"

int g_BGMHandle[BGM_TYPE_MAX] = { 0 };
int g_SEHandle[SE_TYPE_MAX] = { 0 };
static int g_BGMVolume = 128;
static int g_SEVolume = 128;
static int g_BGMVolumeLevel = 5; // 0～10
static int g_SEVolumeLevel = 5;
static BGMType g_CurrentBGM = BGM_TYPE_MAX; // 現在再生中
static bool g_IsFading = false;
static int g_FadeFrame = 0;
static int g_FadeTimer = 0;
static BGMType g_NextBGM = BGM_TYPE_MAX;

static int VolumeLevelToDx(int level)
{
	return level * 25;
}


// BGMロード
void LoadBGM()
{
	g_BGMHandle[BGM_TITLE] = LoadSoundMem("Data/Sound/BGM/Title.ogg");
	g_BGMHandle[BGM_PLAY] = LoadSoundMem("Data/Sound/BGM/Play.ogg");
	g_BGMHandle[BGM_KETHER] = LoadSoundMem("Data/Sound/BGM/Kether.ogg");
	g_BGMHandle[BGM_CARCOMBAT] = LoadSoundMem("Data/Sound/BGM/Catcombat.ogg");
	g_BGMHandle[BGM_TIPHERETH] = LoadSoundMem("Data/Sound/BGM/Tiphereth.ogg");
}

// BGM再生
void PlayBGM(BGMType type)
{
	// 同じBGMなら何もしない
	if (g_CurrentBGM == type)
		return;

	// 現在のBGM停止
	if (g_CurrentBGM != BGM_TYPE_MAX)
	{
		StopSoundMem(g_BGMHandle[g_CurrentBGM]);
	}

	// 新しいBGM再生
	PlaySoundMem(g_BGMHandle[type], DX_PLAYTYPE_LOOP);
	ChangeVolumeSoundMem(g_BGMVolume, g_BGMHandle[type]);

	g_CurrentBGM = type;
}

// フェード切り替え
void FadeChangeBGM(BGMType next, int frame)
{
	if (g_CurrentBGM == next)
		return;

	g_IsFading = true;
	g_FadeFrame = frame;
	g_FadeTimer = 0;
	g_NextBGM = next;
}

// フェード更新（毎フレーム呼び出す）
void UpdateBGMFade()
{
	if (!g_IsFading) return;

	g_FadeTimer++;

	float t = (float)g_FadeTimer / g_FadeFrame;

	// 前半：フェードアウト
	if (t < 0.5f)
	{
		int vol = (int)(g_BGMVolume * (1.0f - t * 2));
		if (g_CurrentBGM != BGM_TYPE_MAX)
		{
			ChangeVolumeSoundMem(vol, g_BGMHandle[g_CurrentBGM]);
		}
	}
	// 後半：フェードイン
	else
	{
		// 切り替え（1回だけ）
		if (g_CurrentBGM != g_NextBGM)
		{
			if (g_CurrentBGM != BGM_TYPE_MAX)
			{
				StopSoundMem(g_BGMHandle[g_CurrentBGM]);
			}

			PlaySoundMem(g_BGMHandle[g_NextBGM], DX_PLAYTYPE_LOOP);
			g_CurrentBGM = g_NextBGM;
		}

		float fadeInT = (t - 0.5f) * 2.0f;
		int vol = (int)(g_BGMVolume * fadeInT);

		ChangeVolumeSoundMem(vol, g_BGMHandle[g_CurrentBGM]);
	}

	// 終了
	if (g_FadeTimer >= g_FadeFrame)
	{
		ChangeVolumeSoundMem(g_BGMVolume, g_BGMHandle[g_CurrentBGM]);

		g_IsFading = false;
		g_NextBGM = BGM_TYPE_MAX;
	}
}

bool IsPlayingBGM(BGMType type)
{
	return CheckSoundMem(g_BGMHandle[type]) == 1;
}

// BGM停止
void StopBGM(BGMType type)
{
	StopSoundMem(g_BGMHandle[type]);

	if (g_CurrentBGM == type)
	{
		g_CurrentBGM = BGM_TYPE_MAX;
	}
}

// BGM終了
void FinBGM()
{
	for (int i = 0; i < BGM_TYPE_MAX; i++)
	{
		DeleteSoundMem(g_BGMHandle[i]);
	}
}

// 全BGM停止
void StopAllBGM()
{
	for (int i = 0; i < BGM_TYPE_MAX; i++)
	{
		StopSoundMem(g_BGMHandle[i]);
	}
	g_CurrentBGM = BGM_TYPE_MAX;
}

// BGM音量設定
void SetBGMVolumeLevel(int level)
{
	if (level < 0)  level = 0;
	if (level > 10) level = 10;

	g_BGMVolumeLevel = level;
	g_BGMVolume = VolumeLevelToDx(level);

	for (int i = 0; i < BGM_TYPE_MAX; i++)
	{
		if (g_BGMHandle[i] != 0)
		{
			ChangeVolumeSoundMem(g_BGMVolume, g_BGMHandle[i]);
		}
	}
}

bool IsFadingBGM()
{
	return g_IsFading;
}

// 現在のBGMタイプを取得
BGMType GetCurrentBGM()
{
	return g_CurrentBGM;
}

int GetBGMVolumeLevel()
{
	return g_BGMVolumeLevel;
}

// SEロード
void LoadSE()
{
	g_SEHandle[SE_PLAYRE_ATTACK] = LoadSoundMem("Data/Sound/SE/PlayerAttack.ogg");
	g_SEHandle[SE_PLAYRE_WEAK_SHOT] = LoadSoundMem("Data/Sound/SE/PlayerWeakShot.ogg");
	g_SEHandle[SE_PLAYRE_SHOT] = LoadSoundMem("Data/Sound/SE/PlayerShot.ogg");
	g_SEHandle[SE_PLAYRE_SHOT_HIT] = LoadSoundMem("Data/Sound/SE/PlayerShotHit.ogg");
	g_SEHandle[SE_PLAYER_DAMAGE] = LoadSoundMem("Data/Sound/SE/PlayerDamage.ogg");
	g_SEHandle[SE_PLAYER_DEAD] = LoadSoundMem("Data/Sound/SE/PlayerDead.ogg");
	g_SEHandle[SE_ENEMY_DEAD] = LoadSoundMem("Data/Sound/SE/EnemyDead.ogg");

	g_SEHandle[SE_MENU_MOVE] = LoadSoundMem("Data/Sound/SE/MenuMove.ogg");
	g_SEHandle[SE_MENU_DECIDE] = LoadSoundMem("Data/Sound/SE/MenuDecide.ogg");
	g_SEHandle[SE_MENU_OK] = LoadSoundMem("Data/Sound/SE/MenuOK.ogg");
	g_SEHandle[SE_MENU_SELECT] = LoadSoundMem("Data/Sound/SE/MenuSelect.ogg");
	g_SEHandle[SE_MENU_CANCEL] = LoadSoundMem("Data/Sound/SE/MenuCancel.ogg");
	g_SEHandle[SE_MONEY_PICKUP] = LoadSoundMem("Data/Sound/SE/MoneyPickup.ogg");
	g_SEHandle[SE_BLOCK_DAMAGE] = LoadSoundMem("Data/Sound/SE/BlockDamage.ogg");
	g_SEHandle[SE_BLOCK_BREAK] = LoadSoundMem("Data/Sound/SE/BlockBreak.ogg");
	g_SEHandle[SE_MENU_EQUIP] = LoadSoundMem("Data/Sound/SE/Equip.ogg");
	g_SEHandle[SE_MENU_REMOVE] = LoadSoundMem("Data/Sound/SE/Remove.ogg");
}

// SE再生
void PlaySE(SEType type)
{
	PlaySoundMem(g_SEHandle[type], DX_PLAYTYPE_BACK);
	ChangeVolumeSoundMem(g_SEVolume, g_SEHandle[type]); // 再生時に音量適用
}

// SE終了
void FinSE()
{
	for (int i = 0; i < SE_TYPE_MAX; i++)
	{
		DeleteSoundMem(g_SEHandle[i]);
	}
}

void SetSEVolumeLevel(int level)
{
	if (level < 0)  level = 0;
	if (level > 10) level = 10;

	g_SEVolumeLevel = level;
	g_SEVolume = VolumeLevelToDx(level);

	for (int i = 0; i < SE_TYPE_MAX; i++)
	{
		if (g_SEHandle[i] != 0)
		{
			ChangeVolumeSoundMem(g_SEVolume, g_SEHandle[i]);
		}
	}
}

int GetSEVolumeLevel()
{
	return g_SEVolumeLevel;
}