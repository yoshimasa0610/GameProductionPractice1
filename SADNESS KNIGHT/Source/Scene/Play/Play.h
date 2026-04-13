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

// セーブ用進行状況（ボス撃破）
void SetBossDefeatSaveFlags(bool forest3MidBossDefeated, bool forest7MidBossDefeated, bool forest5BigBossDefeated);
void GetBossDefeatSaveFlags(bool& forest3MidBossDefeated, bool& forest7MidBossDefeated, bool& forest5BigBossDefeated);

// --- ポーズ管理用に追加 ---
void SetPaused(bool paused); // ポーズ状態を設定する
bool IsPaused();             // 現在ポーズ中かどうかを取得する

#ifdef __cplusplus
extern "C" {
#endif
void LockStageTransition();
void UnlockStageTransition();
bool IsStageLocked();
#ifdef __cplusplus
}
#endif