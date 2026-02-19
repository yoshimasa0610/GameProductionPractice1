#include "Animation.h"
#include "DxLib.h"
#include <memory>

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
    InitAnimation(anim);

    anim.frames = new int[frameCount];
    anim.frameCount = frameCount;
    anim.animationSpeed = animSpeed;
    anim.mode = mode;

    for (int i = 0; i < frameCount; i++)
    {
        anim.frames[i] = -1;
    }

    int result = LoadDivGraph(
        filePath,
        frameCount,
        frameCount,
        1,
        frameWidth,
        frameHeight,
        anim.frames
    );

    if (result == -1 || anim.frames[0] == -1)
    {
        delete[] anim.frames;
        anim.frames = nullptr;
        anim.frameCount = 0;
        return false;
    }

    return true;
}

// スプライトシートから範囲を指定して読み込み
bool LoadAnimationFromSheetRange(AnimationData& anim, const char* filePath,
                                int totalFrames, int startFrame, int frameCount,
                                int frameWidth, int frameHeight,
                                int animSpeed, AnimationMode mode)
{
    // 画像サイズから列・行を取得
    int tempHandle = LoadGraph(filePath);
    if (tempHandle == -1)
    {
        return false;
    }

    int imgW = 0, imgH = 0;
    GetGraphSize(tempHandle, &imgW, &imgH);
    DeleteGraph(tempHandle);

    if (frameWidth <= 0 || frameHeight <= 0)
    {
        return false;
    }

    int cols = imgW / frameWidth;
    int rows = imgH / frameHeight;
    if (cols <= 0 || rows <= 0)
    {
        return false;
    }

    int computedTotal = cols * rows;
    totalFrames = computedTotal;

    if (startFrame < 0 || frameCount <= 0 || startFrame + frameCount > totalFrames)
    {
        return false;
    }

    int* allFrames = new int[totalFrames];

    for (int i = 0; i < totalFrames; i++)
    {
        allFrames[i] = -1;
    }

    int result = LoadDivGraph(
        filePath,
        totalFrames,
        cols,
        rows,
        frameWidth,
        frameHeight,
        allFrames
    );

    if (result == -1 || allFrames[0] == -1)
    {
        delete[] allFrames;
        return false;
    }

    InitAnimation(anim);
    anim.frameCount = frameCount;
    anim.frames = new int[frameCount];
    anim.animationSpeed = animSpeed;
    anim.mode = mode;

    for (int i = 0; i < frameCount; i++)
    {
        anim.frames[i] = allFrames[startFrame + i];
    }

    delete[] allFrames;

    return true;
}

// スプライトシートからジャンプと落下のアニメーションを分けて読み込み
bool LoadJumpFallAnimations(AnimationData& jumpAnim, AnimationData& fallAnim,
                            const char* filePath, int totalFrames, int jumpFrameCount,
                            int frameWidth, int frameHeight)
{
    int* allFrames = new int[totalFrames];

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

    InitAnimation(jumpAnim);
    jumpAnim.frameCount = jumpFrameCount;
    jumpAnim.frames = new int[jumpFrameCount];
    jumpAnim.animationSpeed = 4;
    jumpAnim.mode = AnimationMode::Once;

    int fallFrameCount = totalFrames - jumpFrameCount;
    InitAnimation(fallAnim);
    fallAnim.frameCount = fallFrameCount;
    fallAnim.frames = new int[fallFrameCount];
    fallAnim.animationSpeed = 4;
    fallAnim.mode = AnimationMode::Loop;

    for (int i = 0; i < jumpFrameCount; i++)
    {
        jumpAnim.frames[i] = allFrames[i];
    }

    for (int i = 0; i < fallFrameCount; i++)
    {
        fallAnim.frames[i] = allFrames[jumpFrameCount + i];
    }

    delete[] allFrames;

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

// 追加: 画像サイズを取得して自動で分割読み込みする関数
bool LoadAnimationAuto(AnimationData& anim, const char* filePath,
    int frameWidth, int frameHeight,
    int animSpeed, AnimationMode mode)
{
    int tmpHandle = LoadGraph(filePath);
    if (tmpHandle == -1)
    {
        return false;
    }

    int imgW = 0, imgH = 0;
    GetGraphSize(tmpHandle, &imgW, &imgH);
    DeleteGraph(tmpHandle);

    if (frameWidth <= 0 || frameHeight <= 0)
    {
        return false;
    }

    if (imgW % frameWidth != 0 || imgH % frameHeight != 0)
    {
        return false;
    }

    int cols = imgW / frameWidth;
    int rows = imgH / frameHeight;
    int totalFrames = cols * rows;
    if (totalFrames <= 0)
    {
        return false;
    }

    InitAnimation(anim);
    anim.frames = new int[totalFrames];
    anim.frameCount = totalFrames;
    anim.animationSpeed = animSpeed;
    anim.mode = mode;

    for (int i = 0; i < totalFrames; ++i) anim.frames[i] = -1;

    int result = LoadDivGraph(
        filePath,
        totalFrames,
        cols,
        rows,
        frameWidth,
        frameHeight,
        anim.frames
	);
}