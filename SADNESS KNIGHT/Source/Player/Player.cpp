#include "Player.h"
#include "../Input/Input.h"
#include "DxLib.h"

// プレイヤーのパラメータ定数
namespace
{
    const float MOVE_SPEED = 5.0f;          // 移動速度
    const float JUMP_POWER = 15.0f;         // ジャンプ力
    const float GRAVITY = 0.8f;             // 重力
    const float MAX_FALL_SPEED = 20.0f;     // 最大落下速度
    const int MAX_JUMP_COUNT = 2;           // 最大ジャンプ回数（二段ジャンプ）
    const float GROUND_Y = 600.0f;          // 地面のY座標

    // アニメーション設定
    const int IDLE_FRAME_COUNT = 10;        // アイドルアニメーションのフレーム数
    const int IDLE_ANIM_SPEED = 8;          // アニメーション速度（数値が大きいほど遅い）
}

// プレイヤーの状態を管理する変数
namespace
{
    // 位置
    float posX = 0.0f;
    float posY = 0.0f;

    // 速度
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    // 向き（true: 右, false: 左）
    bool isFacingRight = true;

    // ジャンプ関連
    bool isGrounded = false;        // 地面に接地しているか
    int jumpCount = 0;              // ジャンプ回数

    // 状態
    PlayerState currentState = PlayerState::Idle;

    // アニメーション関連
    int idleImages[IDLE_FRAME_COUNT];   // アイドルアニメーション画像
    int currentFrame = 0;               // 現在のフレーム
    int animationCounter = 0;           // アニメーションカウンター
}

// 内部関数の宣言
namespace
{
    /// <summary>
    /// 入力処理
    /// </summary>
    void ProcessInput();

    /// <summary>
    /// 移動処理
    /// </summary>
    void ProcessMovement();

    /// <summary>
    /// スキル処理
    /// </summary>
    void ProcessSkills();

    /// <summary>
    /// 物理演算更新
    /// </summary>
    void UpdatePhysics();

    /// <summary>
    /// 状態更新
    /// </summary>
    void UpdateState();

    /// <summary>
    /// アニメーション更新
    /// </summary>
    void UpdateAnimation();

    /// <summary>
    /// ジャンプ実行
    /// </summary>
    void ExecuteJump();

    /// <summary>
    /// スキル1を使用
    /// </summary>
    void UseSkill1();

    /// <summary>
    /// スキル2を使用
    /// </summary>
    void UseSkill2();

    /// <summary>
    /// スキル3を使用
    /// </summary>
    void UseSkill3();
}

/// <summary>
/// プレイヤーの初期化
/// </summary>
void InitPlayer(float startX, float startY)
{
    posX = startX;
    posY = startY;
    velocityX = 0.0f;
    velocityY = 0.0f;
    isFacingRight = true;
    isGrounded = false;
    jumpCount = 0;
    currentState = PlayerState::Idle;
    currentFrame = 0;
    animationCounter = 0;
}

/// <summary>
/// プレイヤーのリソース読み込み
/// </summary>
void LoadPlayer()
{
    // アイドルアニメーション画像を読み込む
    // スプライトシートを分割して読み込み
    LoadDivGraph(
        "Data/Player/Idle.png",    // 画像ファイルパス
        IDLE_FRAME_COUNT,               // 分割総数
        IDLE_FRAME_COUNT,               // 横方向の分割数
        1,                              // 縦方向の分割数
        64,                             // 1つの画像の幅（ピクセル）
        64,                             // 1つの画像の高さ（ピクセル）
        idleImages                      // 格納先の配列
    );

    // ※画像が読み込めない場合はデバッグ描画を使用
    // 他のアニメーション（Walk, Jump等）も同様に読み込む予定
}

/// <summary>
/// プレイヤーの更新
/// </summary>
void UpdatePlayer()
{
    // 入力処理
    ProcessInput();

    // 物理演算更新
    UpdatePhysics();

    // 状態更新
    UpdateState();

    // アニメーション更新
    UpdateAnimation();
}

/// <summary>
/// プレイヤーの描画
/// </summary>
void DrawPlayer()
{
    int drawX = static_cast<int>(posX);
    int drawY = static_cast<int>(posY);

    // アイドルアニメーションの画像が読み込まれている場合
    if (currentState == PlayerState::Idle && idleImages[0] != -1)
    {
        // 画像を反転するかどうか
        if (isFacingRight)
        {
            // 右向き（通常描画）
            DrawRotaGraph(drawX, drawY, 1.0, 0.0, idleImages[currentFrame], TRUE);
        }
        else
        {
            // 左向き（左右反転）
            DrawTurnGraph(drawX, drawY, idleImages[currentFrame], TRUE);
        }
    }
    else
    {
        // デバッグ用の簡易描画（画像が読み込まれていない場合）
        unsigned int color = GetColor(255, 255, 255);
        DrawBox(drawX - 16, drawY - 32, drawX + 16, drawY + 32, color, TRUE);

        // 向きを示す矢印
        if (isFacingRight)
        {
            DrawTriangle(
                drawX + 16, drawY,
                drawX + 8, drawY - 8,
                drawX + 8, drawY + 8,
                GetColor(255, 0, 0), TRUE
            );
        }
        else
        {
            DrawTriangle(
                drawX - 16, drawY,
                drawX - 8, drawY - 8,
                drawX - 8, drawY + 8,
                GetColor(255, 0, 0), TRUE
            );
        }
    }

    // デバッグ情報表示
    DrawFormatString(10, 10, GetColor(255, 255, 255), "Position: (%.1f, %.1f)", posX, posY);
    DrawFormatString(10, 30, GetColor(255, 255, 255), "Velocity: (%.1f, %.1f)", velocityX, velocityY);
    DrawFormatString(10, 50, GetColor(255, 255, 255), "Grounded: %s", isGrounded ? "YES" : "NO");
    DrawFormatString(10, 90, GetColor(255, 255, 255), "Animation Frame: %d / %d", currentFrame + 1, IDLE_FRAME_COUNT);
    
    // 状態表示
    const char* stateStr = "UNKNOWN";
    switch (currentState)
    {
    case PlayerState::Idle:   stateStr = "IDLE";   break;
    case PlayerState::Walk:   stateStr = "WALK";   break;
    case PlayerState::Jump:   stateStr = "JUMP";   break;
    case PlayerState::Fall:   stateStr = "FALL";   break;
    case PlayerState::Skill1: stateStr = "SKILL1"; break;
    case PlayerState::Skill2: stateStr = "SKILL2"; break;
    case PlayerState::Skill3: stateStr = "SKILL3"; break;
    }
    DrawFormatString(10, 70, GetColor(255, 255, 0), "State: %s", stateStr);
}

/// <summary>
/// プレイヤーのリソース解放
/// </summary>
void UnloadPlayer()
{
    // アイドルアニメーション画像を解放
    for (int i = 0; i < IDLE_FRAME_COUNT; i++)
    {
        if (idleImages[i] != -1)
        {
            DeleteGraph(idleImages[i]);
            idleImages[i] = -1;
        }
    }
}

/// <summary>
/// プレイヤーのX座標を取得
/// </summary>
float GetPlayerX()
{
    return posX;
}

/// <summary>
/// プレイヤーのY座標を取得
/// </summary>
float GetPlayerY()
{
    return posY;
}

/// <summary>
/// プレイヤーの座標を取得
/// </summary>
void GetPlayerPos(float& outX, float& outY)
{
    outX = posX;
    outY = posY;
}

/// <summary>
/// プレイヤーの状態を取得
/// </summary>
PlayerState GetPlayerState()
{
    return currentState;
}

/// <summary>
/// プレイヤーが右を向いているか
/// </summary>
bool IsPlayerFacingRight()
{
    return isFacingRight;
}

/// <summary>
/// プレイヤーが地面にいるか
/// </summary>
bool IsPlayerGrounded()
{
    return isGrounded;
}

// 内部関数の実装
namespace
{
    /// <summary>
    /// 入力処理
    /// </summary>
    void ProcessInput()
    {
        // 移動処理
        ProcessMovement();

        // スキル処理
        ProcessSkills();
    }

    /// <summary>
    /// 移動処理
    /// </summary>
    void ProcessMovement()
    {
        // 水平移動
        float horizontal = GetMoveHorizontal();
        velocityX = horizontal * MOVE_SPEED;

        // 向きの更新
        if (horizontal > 0.0f)
        {
            isFacingRight = true;
        }
        else if (horizontal < 0.0f)
        {
            isFacingRight = false;
        }

        // ジャンプ（Wキー）
        if (IsMoveUp())
        {
            ExecuteJump();
        }
    }

    /// <summary>
    /// スキル処理
    /// </summary>
    void ProcessSkills()
    {
        // スキル1（Qキー）
        if (IsSkill1Pressed())
        {
            UseSkill1();
        }

        // スキル2（Eキー）
        if (IsSkill2Pressed())
        {
            UseSkill2();
        }

        // スキル3（Fキー）
        if (IsSkill3Pressed())
        {
            UseSkill3();
        }
    }

    /// <summary>
    /// 物理演算更新
    /// </summary>
    void UpdatePhysics()
    {
        // 重力を適用
        if (!isGrounded)
        {
            velocityY += GRAVITY;

            // 最大落下速度を制限
            if (velocityY > MAX_FALL_SPEED)
            {
                velocityY = MAX_FALL_SPEED;
            }
        }

        // 位置を更新
        posX += velocityX;
        posY += velocityY;

        // 簡易的な地面判定
        // ※後で実際のマップとの当たり判定に変更
        if (posY >= GROUND_Y)
        {
            posY = GROUND_Y;
            velocityY = 0.0f;
            isGrounded = true;
            jumpCount = 0;  // 地面に着地したらジャンプ回数をリセット
        }
        else
        {
            isGrounded = false;
        }

        // 画面外に出ないようにする
        if (posX < 0.0f) posX = 0.0f;
        if (posX > 1600.0f) posX = 1600.0f;
    }

    /// <summary>
    /// 状態更新
    /// </summary>
    void UpdateState()
    {
        // スキル使用中は状態変更しない
        if (currentState == PlayerState::Skill1 || 
            currentState == PlayerState::Skill2 || 
            currentState == PlayerState::Skill3)
        {
            // スキルアニメーション終了後に状態をリセット
            // ※ここは後でアニメーション管理と連携
            return;
        }

        // 空中にいる場合
        if (!isGrounded)
        {
            if (velocityY < 0.0f)
            {
                currentState = PlayerState::Jump;  // 上昇中
            }
            else
            {
                currentState = PlayerState::Fall;  // 落下中
            }
        }
        // 地面にいる場合
        else
        {
            if (velocityX != 0.0f)
            {
                currentState = PlayerState::Walk;  // 歩行中
            }
            else
            {
                currentState = PlayerState::Idle;  // 待機中
            }
        }
    }

    /// <summary>
    /// アニメーション更新
    /// </summary>
    void UpdateAnimation()
    {
        // 状態に応じてアニメーションを更新
        switch (currentState)
        {
        case PlayerState::Idle:
            // アニメーションカウンターを増やす
            animationCounter++;

            // 一定カウント毎にフレームを進める
            if (animationCounter >= IDLE_ANIM_SPEED)
            {
                animationCounter = 0;
                currentFrame++;

                // 最後のフレームまで行ったら最初に戻る
                if (currentFrame >= IDLE_FRAME_COUNT)
                {
                    currentFrame = 0;
                }
            }
            break;

        case PlayerState::Walk:
            // ※歩行アニメーション（後で実装）
            currentFrame = 0;
            break;

        case PlayerState::Jump:
        case PlayerState::Fall:
            // ※ジャンプ/落下アニメーション（後で実装）
            currentFrame = 0;
            break;

        default:
            currentFrame = 0;
            break;
        }
    }

    /// <summary>
    /// ジャンプ実行
    /// </summary>
    void ExecuteJump()
    {
        // 地上または二段ジャンプ可能な場合のみジャンプ
        if (jumpCount < MAX_JUMP_COUNT)
        {
            velocityY = -JUMP_POWER;
            isGrounded = false;
            jumpCount++;
        }
    }

    /// <summary>
    /// スキル1を使用
    /// </summary>
    void UseSkill1()
    {
        // スキル1の処理（例：攻撃）
        currentState = PlayerState::Skill1;
        
        // デバッグ出力
        printfDx("Skill 1 Used!\n");
        
        // ※ここにスキル1の具体的な処理を追加
    }

    /// <summary>
    /// スキル2を使用
    /// </summary>
    void UseSkill2()
    {
        // スキル2の処理（例：ダッシュ）
        currentState = PlayerState::Skill2;
        
        // デバッグ出力
        printfDx("Skill 2 Used!\n");
        
        // ※ここにスキル2の具体的な処理を追加
    }

    /// <summary>
    /// スキル3を使用
    /// </summary>
    void UseSkill3()
    {
        // スキル3の処理（例：特殊能力）
        currentState = PlayerState::Skill3;
        
        // デバッグ出力
        printfDx("Skill 3 Used!\n");
        
        // ※ここにスキル3の具体的な処理を追加
    }
}   