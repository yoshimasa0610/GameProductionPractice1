#pragma once

enum class PlayDeathState
{
    None,
    WaitFadeOut,   // フェードアウト待ち
    Respawning,   // リスポーン処理中
    FadeIn        // フェードイン中
};

// 関数のプロトタイプ宣言
void InitPlayScene();
void LoadPlayScene();
void StartPlayScene();
void StepPlayScene();
void UpdatePlayScene();
void DrawPlayScene();
void FinPlayScene();

// --- ポーズ制御用を追加 ---
void SetPaused(bool paused); // ポーズ状態を設定する
bool IsPaused();             // 現在ポーズ中かどうかを取得する