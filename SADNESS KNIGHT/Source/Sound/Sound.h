#pragma once

// BGM関連 ////////////////////////

// BGMタイプ
enum BGMType
{
	BGM_TITLE,
	BGM_PLAY,
	BGM_FOREST_DEEP,
	BGM_KETHER,
	BGM_CARCOMBAT,
	BGM_TIPHERETH,
	BGM_TYPE_MAX
};

// BGMロード
void LoadBGM();
// BGM再生
void PlayBGM(BGMType type);
// フェード切り替え
void FadeChangeBGM(BGMType next, int frame);
// フェード更新（毎フレーム呼び出す）
void UpdateBGMFade();
// BGM再生中か
BGMType GetCurrentBGM();
bool IsFadingBGM();
// BGM停止
void StopBGM(BGMType type);
// BGM終了
void FinBGM();
// BGM音量設定（0～10）
void SetBGMVolumeLevel(int level);
int  GetBGMVolumeLevel();

////////////////////////////////////

// SE関連 ////////////////////////

enum SEType
{
	// ==== Player系 ====
	SE_PLAYER_DASH,
	SE_PLAYER_ATTACK1,
	SE_PLAYER_ATTACK2,
	SE_PLAYER_ATTACK3,
	SE_PLAYER_RUN,
	SE_PLAYER_JUMP,
	SE_PLAYER_FALL_ATTACK,
	SE_PLAYER_HEAL,

	SE_PLAYER_DAMAGE,
	SE_PLAYER_DEAD,
	SE_ENEMY_DEAD,

	//メニューやタイトル
	SE_MENU_MOVE,     // メニューやタイトル選択移動音
	SE_MENU_DECIDE,   // タイトル決定音
	SE_MENU_OK,       //設定などのオプション時効果音
	SE_MENU_SELECT,   //設定で左右を押すことで変更した場合の効果音
	SE_MENU_CANCEL,
	SE_MENU_EQUIP,    //装備したときになるSE
	SE_MENU_REMOVE,   //装備を外した時になるSE
	SE_SPIKE_ATTACK,  //トゲに向かって近接攻撃を行った際に発生するSE
	SE_BLOCK_DAMAGE,  //破壊可能ブロックにダメージを与えた際に発生するSE
	SE_BLOCK_BREAK,   //破壊可能ブロックを攻撃によって破壊した際に発生するSE
	SE_MONEY_PICKUP,  //お金を獲得した際のSE
	SE_TYPE_MAX
};

// SEロード
void LoadSE();
// SE再生
void PlaySE(SEType type, bool loop = false);
// SE停止
void StopSE(SEType type);
// SE終了
void FinSE();
// SE音量調整
void SetSEVolumeLevel(int level);
int  GetSEVolumeLevel();