#include "SceneManager.h"
#include "Title/Title.h"
#include "Play/Play.h"
#include <DxLib.h>
#include "../Input/Input.h"

// 最初に開かれるシーンを初期値にする
Scene g_NowScene = SCENE_TITLE;

// 次に遷移するシーン
Scene g_NextScene = SCENE_TITLE;

//　直前のシーンの記録
Scene g_PrevScene = SCENE_TITLE;

// シーンの状態
SceneState g_SceneState = SCENE_STATE_INIT;

// ループを終了するか
bool g_IsLoopEnd = false;

void SceneManagerUpdate()
{
	// シーンの状態ごとに各シーンの処理を呼ぶ
	switch (g_SceneState)
	{
	case SCENE_STATE_INIT:	// 初期化
		InitScene();

		// ロードへ
		g_SceneState = SCENE_STATE_LOAD;
		break;

	case SCENE_STATE_LOAD:	// ロード
		LoadScene();

		// 開始へ
		g_SceneState = SCENE_STATE_START;
		break;

	case SCENE_STATE_START:	// 開始（ループ開始前に1回だけ）
		// 開始処理
		StartScene();

		// 開始へ
		g_SceneState = SCENE_STATE_LOOP;
		break;

	case SCENE_STATE_LOOP:	// ループ（ステップ→更新→描画）
		DrawFormatString(10, 500, GetColor(255, 255, 0),
			"Scene=%d State=%d IsLoopEnd=%d",
			g_NowScene, g_SceneState, g_IsLoopEnd);
		StepScene();
		UpdateScene();
		DrawScene();
		break;

	case SCENE_STATE_FIN:	// 終了
		FinScene();
		// 次のシーンに切り替える
		g_NowScene = g_NextScene;
		// シーンは初期化から
		g_SceneState = SCENE_STATE_INIT;
		break;
	}
}

void InitScene()
{
	// 開いているシーンの初期化処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		InitTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		InitPlayScene();
		break;
	}
}

void LoadScene()
{
	// 開いているシーンのロード処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		LoadTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		LoadPlayScene();
		break;
	}
}

void StartScene()
{
	// 開いているシーンのロード処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		StartTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		StartPlayScene();
		break;
	}
}

void StepScene()
{
	// 開いているシーンのステップ処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		StepTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		StepPlayScene();
		break;
	}
}

void UpdateScene()
{
	// 開いているシーンの更新処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		UpdateTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		UpdatePlayScene();
		break;
	}
}

void DrawScene()
{
	// 開いているシーンの描画処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		DrawTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		DrawPlayScene();
		break;
	}
}

void FinScene()
{
	// 開いているシーンの終了処理を呼ぶ
	switch (g_NowScene)
	{
	case SCENE_TITLE:	// タイトル
		FinTitleScene();
		break;

	case SCENE_PLAY:	// プレイ
		FinPlayScene();
		break;
	}
}

void ChangeScene(Scene scene)
{
	// 直前のシーンを記録
	g_PrevScene = g_NowScene;

	// 次に遷移するシーンを設定
	g_NextScene = scene;

	// シーンのループを終了させる
	g_SceneState = SCENE_STATE_FIN;
}

Scene GetPrevScene()
{
	return g_PrevScene;
}