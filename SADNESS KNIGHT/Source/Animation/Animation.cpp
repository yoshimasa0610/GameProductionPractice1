#include "Animation.h"
#include "DxLib.h"

// アニメーションデータの初期化
void InitAnimation(AnimationData& anim)
{
    anim.frames = nullptr;
    anim.frameCount = 0;
    anim.currentFrame = 0;
    anim.animationCounter = 0;
    anim.animationSpeed = 8;
    anim.mode = AnimationMode::Loop;
    anim.isPlaying = true;
    anim.isFinished = false;
    anim.isPingPongReverse = false;
}

// スプライトシートを読み込んでアニメーションを作成
bool LoadAnimationFromSheet(AnimationData& anim, const char* filePath, 
                           int frameCount, int frameWidth, int frameHeight,
                           int animSpeed, AnimationMode mode)
{
    // 初期化
    InitAnimation(anim);
    
    // フレーム配列を確保
    anim.frames = new int[frameCount];
    anim.frameCount = frameCount;
    anim.animationSpeed = animSpeed;
    anim.mode = mode;
    
    // 配列を-1で初期化
    for (int i = 0; i < frameCount; i++)
    {
        anim.frames[i] = -1;
    }
    
    // スプライトシートを分割読み込み
    int result = LoadDivGraph(
        filePath,
        frameCount,
        frameCount,     // 横方向の分割数
        1,              // 縦方向の分割数
        frameWidth,
        frameHeight,
        anim.frames
    );
    
    // デバッグ: 結果を表示
    printfDx("LoadDivGraph '%s': result=%d, first frame=%d\n", 
             filePath, result, anim.frames[0]);
    
    // 読み込み失敗チェック（result == -1 または最初のフレームが-1）
    if (result == -1 || anim.frames[0] == -1)
    {
        printfDx("LoadAnimationFromSheet FAILED: %s\n", filePath);
        delete[] anim.frames;
        anim.frames = nullptr;
        anim.frameCount = 0;
        return false;
    }
    
    printfDx("LoadAnimationFromSheet SUCCESS: %s (%d frames)\n", filePath, frameCount);
    return true;
}

// スプライトシートから範囲を指定して読み込み
bool LoadAnimationFromSheetRange(AnimationData& anim, const char* filePath,
                                int totalFrames, int startFrame, int frameCount,
                                int frameWidth, int frameHeight,
                                int animSpeed, AnimationMode mode)
{
    // 一時的に全フレームを読み込む
    int* allFrames = new int[totalFrames];
    
    // 配列を-1で初期化
    for (int i = 0; i < totalFrames; i++)
    {
        allFrames[i] = -1;
    }
    
    int result = LoadDivGraph(
        filePath,
        totalFrames,
        totalFrames,
        1,
        frameWidth,
        frameHeight,
        allFrames
    );
    
    if (result == -1 || allFrames[0] == -1)
    {
        delete[] allFrames;
        return false;
    }
    
    // 初期化
    InitAnimation(anim);
    anim.frameCount = frameCount;
    anim.frames = new int[frameCount];
    anim.animationSpeed = animSpeed;
    anim.mode = mode;
    
    // 指定範囲のフレームをコピー
    for (int i = 0; i < frameCount; i++)
    {
        anim.frames[i] = allFrames[startFrame + i];
    }
    
    // 一時配列を解放
    delete[] allFrames;
    
    return true;
}

// スプライトシートからジャンプと落下のアニメーションを分けて読み込み
bool LoadJumpFallAnimations(AnimationData& jumpAnim, AnimationData& fallAnim,
                            const char* filePath, int totalFrames, int jumpFrameCount,
                            int frameWidth, int frameHeight)
{
    // 全フレームを一時的に読み込む
    int* allFrames = new int[totalFrames];
    
    // 配列を-1で初期化
    for (int i = 0; i < totalFrames; i++)
    {
        allFrames[i] = -1;
    }
    
    int result = LoadDivGraph(
        filePath,
        totalFrames,
        totalFrames,
        1,
        frameWidth,
        frameHeight,
        allFrames
    );
    
    printfDx("LoadJumpFallAnimations LoadDivGraph '%s': result=%d, first=%d\n", 
             filePath, result, allFrames[0]);
    
    if (result == -1 || allFrames[0] == -1)
    {
        printfDx("LoadJumpFallAnimations FAILED\n");
        delete[] allFrames;
        return false;
    }
    
    // ジャンプアニメーションを初期化
    InitAnimation(jumpAnim);
    jumpAnim.frameCount = jumpFrameCount;
    jumpAnim.frames = new int[jumpFrameCount];
    jumpAnim.animationSpeed = 4;
    jumpAnim.mode = AnimationMode::Once;
    
    // 落下アニメーションを初期化
    int fallFrameCount = totalFrames - jumpFrameCount;
    InitAnimation(fallAnim);
    fallAnim.frameCount = fallFrameCount;
    fallAnim.frames = new int[fallFrameCount];
    fallAnim.animationSpeed = 4;
    fallAnim.mode = AnimationMode::Loop;
    
    // ジャンプフレームをコピー（前半）
    for (int i = 0; i < jumpFrameCount; i++)
    {
        jumpAnim.frames[i] = allFrames[i];
    }
    
    // 落下フレームをコピー（後半）
    for (int i = 0; i < fallFrameCount; i++)
    {
        fallAnim.frames[i] = allFrames[jumpFrameCount + i];
    }
    
    // 一時配列を解放
    delete[] allFrames;
    
    printfDx("LoadJumpFallAnimations SUCCESS: jump=%d, fall=%d frames\n", 
             jumpFrameCount, fallFrameCount);
    return true;
}

// アニメーション画像を解放
void UnloadAnimation(AnimationData& anim)
{
    if (anim.frames != nullptr)
    {
        // すべてのフレーム画像を解放
        for (int i = 0; i < anim.frameCount; i++)
        {
            if (anim.frames[i] != -1)
            {
                DeleteGraph(anim.frames[i]);
            }
        }
        
        delete[] anim.frames;
        anim.frames = nullptr;
    }
    
    InitAnimation(anim);
}

// アニメーションを更新
void UpdateAnimation(AnimationData& anim)
{
    // 再生中でない、またはフレームがない場合は何もしない
    if (!anim.isPlaying || anim.frames == nullptr || anim.frameCount <= 0)
    {
        return;
    }
    
    // アニメーションカウンターを増やす
    anim.animationCounter++;
    
    // 指定速度に達したらフレームを進める
    if (anim.animationCounter >= anim.animationSpeed)
    {
        anim.animationCounter = 0;
        
        switch (anim.mode)
        {
        case AnimationMode::Loop:
            // ループ再生
            anim.currentFrame++;
            if (anim.currentFrame >= anim.frameCount)
            {
                anim.currentFrame = 0;
            }
            break;
            
        case AnimationMode::Once:
            // 一度だけ再生
            if (!anim.isFinished)
            {
                anim.currentFrame++;
                if (anim.currentFrame >= anim.frameCount)
                {
                    anim.currentFrame = anim.frameCount - 1;
                    anim.isFinished = true;
                    anim.isPlaying = false;
                }
            }
            break;
            
        case AnimationMode::PingPong:
            // 往復再生
            if (!anim.isPingPongReverse)
            {
                // 順方向
                anim.currentFrame++;
                if (anim.currentFrame >= anim.frameCount)
                {
                    anim.currentFrame = anim.frameCount - 2;
                    anim.isPingPongReverse = true;
                }
            }
            else
            {
                // 逆方向
                anim.currentFrame--;
                if (anim.currentFrame < 0)
                {
                    anim.currentFrame = 1;
                    anim.isPingPongReverse = false;
                }
            }
            break;
        }
    }
}

// アニメーションを再生開始
void PlayAnimation(AnimationData& anim)
{
    anim.isPlaying = true;
}

// アニメーションを停止
void StopAnimation(AnimationData& anim)
{
    anim.isPlaying = false;
}

// アニメーションを最初から再生
void ResetAnimation(AnimationData& anim)
{
    anim.currentFrame = 0;
    anim.animationCounter = 0;
    anim.isFinished = false;
    anim.isPingPongReverse = false;
    anim.isPlaying = true;
}

// アニメーション速度を変更
void SetAnimationSpeed(AnimationData& anim, int speed)
{
    if (speed > 0)
    {
        anim.animationSpeed = speed;
    }
}

// 特定のフレームに設定
void SetAnimationFrame(AnimationData& anim, int frame)
{
    if (frame >= 0 && frame < anim.frameCount)
    {
        anim.currentFrame = frame;
        anim.animationCounter = 0;
    }
}

// 現在のフレーム画像ハンドルを取得
int GetCurrentAnimationFrame(const AnimationData& anim)
{
    if (anim.frames == nullptr || anim.currentFrame >= anim.frameCount)
    {
        return -1;
    }
    return anim.frames[anim.currentFrame];
}

// アニメーションが終了したか
bool IsAnimationFinished(const AnimationData& anim)
{
    return anim.isFinished;
}

// アニメーションが再生中か
bool IsAnimationPlaying(const AnimationData& anim)
{
    return anim.isPlaying;
}

// アニメーションを描画（座標指定）
void DrawAnimation(const AnimationData& anim, int x, int y, bool turnFlag)
{
    int frameHandle = GetCurrentAnimationFrame(anim);
    if (frameHandle == -1)
    {
        return;
    }
    
    if (turnFlag)
    {
        // 左右反転描画
        DrawTurnGraph(x, y, frameHandle, TRUE);
    }
    else
    {
        // 通常描画
        DrawGraph(x, y, frameHandle, TRUE);
    }
}

// アニメーションを回転描画
void DrawAnimationRotated(const AnimationData& anim, int x, int y, double scale, double angle, bool turnFlag)
{
    int frameHandle = GetCurrentAnimationFrame(anim);
    if (frameHandle == -1)
    {
        return;
    }
    
    if (turnFlag)
    {
        // 左右反転 + 回転描画
        DrawRotaGraph(x, y, scale, angle, frameHandle, TRUE, TRUE);
    }
    else
    {
        // 回転描画
        DrawRotaGraph(x, y, scale, angle, frameHandle, TRUE, FALSE);
    }
}