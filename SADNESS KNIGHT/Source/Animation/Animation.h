#pragma once

// アニメーションの再生モード
enum class AnimationMode
{
    Loop,       // ループ再生
    Once,       // 一度だけ再生
    PingPong    // 往復再生
};

// アニメーションデータ構造体
struct AnimationData
{
    int* frames;                // フレーム画像配列
    int frameCount;             // フレーム数
    int currentFrame;           // 現在のフレーム
    int animationCounter;       // アニメーションカウンター
    int animationSpeed;         // アニメーション速度（数値が大きいほど遅い）
    AnimationMode mode;         // 再生モード
    bool isPlaying;             // 再生中かどうか
    bool isFinished;            // 再生が終了したか（Onceモードのみ）
    bool isPingPongReverse;     // PingPongモードで逆再生中か
};

// ===== 初期化・解放 =====

// アニメーションデータの初期化
void InitAnimation(AnimationData& anim);

// スプライトシートを読み込んでアニメーションを作成
bool LoadAnimationFromSheet(AnimationData& anim, const char* filePath, 
                           int frameCount, int frameWidth, int frameHeight,
                           int animSpeed, AnimationMode mode = AnimationMode::Loop);

// スプライトシートから範囲を指定して読み込み
bool LoadAnimationFromSheetRange(AnimationData& anim, const char* filePath,
                                int totalFrames, int startFrame, int frameCount,
                                int frameWidth, int frameHeight,
                                int animSpeed, AnimationMode mode = AnimationMode::Loop);

// スプライトシートからジャンプと落下のアニメーションを分けて読み込み
bool LoadJumpFallAnimations(AnimationData& jumpAnim, AnimationData& fallAnim,
                            const char* filePath, int totalFrames, int jumpFrameCount,
                            int frameWidth, int frameHeight);

// アニメーション画像を解放
void UnloadAnimation(AnimationData& anim);

// ===== 更新・制御 =====

// アニメーションを更新
void UpdateAnimation(AnimationData& anim);

// アニメーションを再生開始
void PlayAnimation(AnimationData& anim);

// アニメーションを停止
void StopAnimation(AnimationData& anim);

// アニメーションを最初から再生
void ResetAnimation(AnimationData& anim);

// アニメーション速度を変更
void SetAnimationSpeed(AnimationData& anim, int speed);

// 特定のフレームに設定
void SetAnimationFrame(AnimationData& anim, int frame);

// ===== 取得 =====

// 現在のフレーム画像ハンドルを取得
int GetCurrentAnimationFrame(const AnimationData& anim);

// アニメーションが終了したか
bool IsAnimationFinished(const AnimationData& anim);

// アニメーションが再生中か
bool IsAnimationPlaying(const AnimationData& anim);

// ===== 描画 =====

// アニメーションを描画（座標指定）
void DrawAnimation(const AnimationData& anim, int x, int y, bool turnFlag = false);

// アニメーションを回転描画
void DrawAnimationRotated(const AnimationData& anim, int x, int y, double scale, double angle, bool turnFlag = false);

// 追加: 画像サイズから自動で分割数を決めて読み込むヘルパー
bool LoadAnimationAuto(AnimationData& anim, const char* filePath,
                       int frameWidth, int frameHeight,
                       int animSpeed, AnimationMode mode = AnimationMode::Loop);