#pragma once

// BGM関連 ////////////////////////

// BGMタイプ
enum BGMType
{
	BGM_TITLE,
	BGM_PLAY,
	BGM_TYPE_MAX
};

// BGMロード
void LoadBGM();
// BGM再生
void PlayBGM(BGMType type);
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
	SE_PLAYRE_ATTACK,
	SE_PLAYRE_WEAK_SHOT, //遠距離攻撃（弱）のSE
	SE_PLAYRE_SHOT, //遠距離攻撃（強）のSE
	SE_PLAYRE_SHOT_HIT, //その遠距離攻撃が当たった際のSE（今は共通）
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
void PlaySE(SEType type);
// SE終了
void FinSE();
// SE音量調整
void SetSEVolumeLevel(int level);
int  GetSEVolumeLevel();