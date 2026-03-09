#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include "../Animation/Animation.h"
#include "../Map/MapManager.h"
#include "../Camera/Camera.h"
#include "DxLib.h"
#include <cmath>
#include <cstdio>




namespace
{
    std::vector<EnemyData> g_enemies;

    //============================================================
    // 敵の設定テーブル（ここを編集してバランス調整）
    //============================================================
    // 
    // 各パラメータの説明：
    // HP         : 体力（プレイヤーの攻撃力は100が基準）
    // 攻撃力     : プレイヤーへのダメージ
    // 移動速度   : 1.0が標準、2.0で2倍速
    // 幅・高さ   : 当たり判定のサイズ（ピクセル）
    // 検知範囲   : この距離内でプレイヤーを追跡開始
    // 攻撃範囲   : この距離内で攻撃
    // 攻撃時間   : 攻撃判定が有効な時間（秒）
    // クールダウン: 次の攻撃までの待ち時間（秒）
    // ジャンプ   : ジャンプできるか（true/false）
    // ジャンプ力 : ジャンプの高さ（8.0が標準）
    //
    //============================================================
    
    const EnemyConfig g_enemyConfigs[static_cast<int>(EnemyType::Count)] = {
        // { HP, 攻撃, 速度, 幅, 高さ, 検知, 攻撃範囲, 攻撃時間, CD, ジャンプ, J力, パス }
        
        // Slime: 初心者向けの弱い敵
        { 12, 4, 1.0f, 24.0f, 20.0f, 140.0f, 18.0f, 0.15f, 1.2f, false, 0.0f, "Assets/Enemies/Slime/" },
        
        // Skeleton: 標準的な敵、ジャンプで追いかけてくる
        { 60, 12, 1.5f, 30.0f, 56.0f, 280.0f, 48.0f, 0.2f, 1.0f, true, 8.0f, "Assets/Enemies/Skeleton/" },
        
        // Cultists: 汎用的な近接戦闘員
        { 40, 12, 1.2f, 30.0f, 56.0f, 260.0f, 40.0f, 0.25f, 1.0f, false, 0.0f, "Assets/Enemies/Cultists/" },
        
        // AssassinCultist: 素早く動く刺客、攻撃は弱め
        { 30, 8, 2.0f, 28.0f, 44.0f, 220.0f, 36.0f, 0.18f, 0.8f, true, 9.0f, "Assets/Enemies/AssassinCultist/" },
        
        // BigQuartist: ボス級の大型敵、遅いが強力
        { 250, 30, 0.8f, 64.0f, 96.0f, 360.0f, 100.0f, 0.3f, 1.6f, false, 0.0f, "Assets/Enemies/BigQuartist/" },
        
        // TwistedCaltis: 変則的な動きをする中ボス
        { 90, 16, 1.8f, 34.0f, 52.0f, 320.0f, 44.0f, 0.22f, 1.2f, true, 10.0f, "Assets/Enemies/TwistedCaltis/" }
    };

    const float GRAVITY = 0.3f;
    const float MAX_FALL_SPEED = 6.0f;
    const float PATROL_RANGE_BLOCKS = 5.0f;
    const float BLOCK_SIZE = 32.0f;
    
    // レイキャストによる視線判定（ブロックを透視できない）
    bool HasLineOfSight(float fromX, float fromY, float toX, float toY)
    {
        float dx = toX - fromX;
        float dy = toY - fromY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < 0.01f) return true;
        
        // レイを細かくサンプリング
        int steps = static_cast<int>(distance / 8.0f) + 1;
        float stepX = dx / steps;
        float stepY = dy / steps;
        
        for (int i = 0; i < steps; i++)
        {
            float checkX = fromX + stepX * i;
            float checkY = fromY + stepY * i;
            
            // マップチップのインデックスを計算
            int gridX = static_cast<int>(checkX / BLOCK_SIZE);
            int gridY = static_cast<int>(checkY / BLOCK_SIZE);
            
            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                return false; // ブロックに遮られている
            }
        }
        
        return true;
    }
    
    // 敵のマップ当たり判定（地面検出）
    void ResolveEnemyMapCollision(EnemyData& e)
    {
        const float halfW = e.width * 0.5f;
        const float checkBottom = e.posY;
        const float checkTop = e.posY - e.height;
        const float checkLeft = e.posX - halfW;
        const float checkRight = e.posX + halfW;
        
        e.isGrounded = false;
        
        // 下方向の当たり判定（複数点チェック）
        for (int i = 0; i < 3; i++)
        {
            float checkX = checkLeft + (checkRight - checkLeft) * (i / 2.0f);
            int gridX = static_cast<int>(checkX / BLOCK_SIZE);
            int gridY = static_cast<int>((checkBottom + 1.0f) / BLOCK_SIZE);
            
            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                float blockTop = gridY * BLOCK_SIZE;
                if (e.posY > blockTop && e.velocityY >= 0.0f)
                {
                    e.posY = blockTop;
                    e.velocityY = 0.0f;
                    e.isGrounded = true;
                }
            }
        }
        
        // 左右の壁判定
        if (e.velocityX < 0.0f) // 左移動
        {
            int gridX = static_cast<int>((checkLeft - 1.0f) / BLOCK_SIZE);
            int gridY = static_cast<int>((e.posY - e.height * 0.5f) / BLOCK_SIZE);
            
            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                float blockRight = (gridX + 1) * BLOCK_SIZE;
                if (e.posX - halfW < blockRight)
                {
                    e.posX = blockRight + halfW;
                    e.velocityX = 0.0f;
                    e.patrolDirection = 1; // 方向転換
                }
            }
        }
        else if (e.velocityX > 0.0f) // 右移動
        {
            int gridX = static_cast<int>((checkRight + 1.0f) / BLOCK_SIZE);
            int gridY = static_cast<int>((e.posY - e.height * 0.5f) / BLOCK_SIZE);
            
            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                float blockLeft = gridX * BLOCK_SIZE;
                if (e.posX + halfW > blockLeft)
                {
                    e.posX = blockLeft - halfW;
                    e.velocityX = 0.0f;
                    e.patrolDirection = -1; // 方向転換
                }
            }
        }
        
        // 崖検出（徘徊中に落ちないように）
        if (!e.isAggro && e.isGrounded && std::fabs(e.velocityX) > 0.01f)
        {
            float frontX = e.posX + (e.velocityX > 0 ? halfW + 4.0f : -halfW - 4.0f);
            int gridX = static_cast<int>(frontX / BLOCK_SIZE);
            int gridY = static_cast<int>((e.posY + 8.0f) / BLOCK_SIZE);
            
            MapChipData* mcFront = GetMapChipData(gridX, gridY);
            if (mcFront == nullptr || mcFront->mapChip == MAP_CHIP_NONE)
            {
                // 前方が崖なので方向転換
                e.patrolDirection *= -1;
                e.velocityX = 0.0f;
            }
        }
    }
}




const EnemyConfig& GetEnemyConfig(EnemyType type)
{
    return g_enemyConfigs[static_cast<int>(type)];
}


void InitEnemySystem()
{
    g_enemies.clear();
}

int SpawnEnemy(EnemyType type, float x, float y)
{
    const EnemyConfig& config = GetEnemyConfig(type);
    
    printfDx("--- SpawnEnemy ---\n");
    printfDx("Type: %d, Pos: (%.1f, %.1f)\n", (int)type, x, y);
    printfDx("Config: W=%.1f H=%.1f\n", config.width, config.height);
    
    EnemyData e{};
    e.active = true;
    e.type = type;
    e.posX = x;
    e.posY = y;
    e.velocityX = 0.0f;
    e.velocityY = 0.0f;
    e.isFacingRight = true;
    e.isAggro = false;
    e.isGrounded = false;
    
    e.patrolStartX = x;
    e.patrolRange = PATROL_RANGE_BLOCKS * BLOCK_SIZE;
    e.patrolDirection = 1;
    e.hasLineOfSight = false;
    
    e.maxHP = config.maxHP;
    e.currentHP = config.maxHP;
    e.attackPower = config.attackPower;
    e.moveSpeed = config.moveSpeed;
    e.detectRange = config.detectRange;
    e.attackRange = config.attackRange;
    e.attackDuration = config.attackDuration;
    e.attackCooldown = config.attackCooldown;
    e.attackTimer = 0.0f;
    e.cooldownTimer = 0.0f;
    e.canJump = config.canJump;
    e.jumpPower = config.jumpPower;
    e.width = config.width;
    e.height = config.height;

    float left = e.posX - (e.width * 0.5f);
    float top = e.posY - e.height;
    e.colliderId = CreateCollider(ColliderTag::Enemy, left, top, e.width, e.height, nullptr);

    printfDx("Collider: L=%.1f T=%.1f W=%.1f H=%.1f\n", left, top, e.width, e.height);

    e.animations = LoadEnemyAnimations(type);
    
    printfDx("Animations: %s\n", e.animations ? "OK" : "NULL");

    g_enemies.push_back(e);
    
    printfDx("Enemy added to vector. Total: %d\n", (int)g_enemies.size());
    
    return static_cast<int>(g_enemies.size() - 1);
}




// 敵破棄
void DespawnEnemy(int index)
{
    if (index < 0 || index >= (int)g_enemies.size()) return;
    EnemyData& e = g_enemies[index];
    if (e.colliderId != -1)
    {
        DestroyCollider(e.colliderId);
        e.colliderId = -1;
    }
    if (e.animations != nullptr)
    {
        UnloadEnemyAnimations(e.animations);
        e.animations = nullptr;
    }
    e.active = false;
}

// 全削除
void ClearEnemies()
{
    for (auto& e : g_enemies)
    {
        if (e.colliderId != -1)
        {
            DestroyCollider(e.colliderId);
            e.colliderId = -1;
        }
        if (e.animations != nullptr)
        {
            UnloadEnemyAnimations(e.animations);
            e.animations = nullptr;
        }
    }
    g_enemies.clear();
}

void UpdateEnemies()
{
    PlayerData& player = GetPlayerData();
    const float FRAME_TIME = 1.0f / 60.0f;
    const float DETECTION_RANGE = 4.0f * BLOCK_SIZE; // 4ブロック

    for (auto& e : g_enemies)
    {
        if (!e.active) continue;

        if (e.currentHP <= 0)
        {
            if (e.colliderId != -1)
            {
                DestroyCollider(e.colliderId);
                e.colliderId = -1;
            }
            if (e.animations != nullptr)
            {
                UnloadEnemyAnimations(e.animations);
                e.animations = nullptr;
            }
            e.active = false;
            continue;
        }

        // プレイヤーとの距離と視線判定
        float dx = player.posX - e.posX;
        float dy = player.posY - e.posY;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        // 視線判定（敵の目の位置からプレイヤーの中心へ）
        float eyeY = e.posY - e.height * 0.7f;
        float playerCenterY = player.posY - (PLAYER_HEIGHT * 0.5f);
        e.hasLineOfSight = HasLineOfSight(e.posX, eyeY, player.posX, playerCenterY);
        
        // プレイヤー検知（距離内 && 視線が通っている）
        if (dist <= DETECTION_RANGE && e.hasLineOfSight)
        {
            e.isAggro = true;
            e.isFacingRight = (dx >= 0.0f);
        }
        else
        {
            e.isAggro = false;
        }

        if (e.cooldownTimer > 0.0f) e.cooldownTimer -= FRAME_TIME;
        if (e.attackTimer > 0.0f) e.attackTimer -= FRAME_TIME;

        // AI行動
        if (e.isAggro)
        {
            // プレイヤー追跡
            if (std::fabs(dx) > e.attackRange * 0.5f)
            {
                e.velocityX = (dx > 0.0f) ? e.moveSpeed : -e.moveSpeed;
            }
            else
            {
                e.velocityX = 0.0f;
            }
        }
        else
        {
            // 徘徊モード
            if (e.isGrounded)
            {
                float distFromStart = e.posX - e.patrolStartX;
                
                // 徘徊範囲を超えたら方向転換
                if (distFromStart > e.patrolRange)
                {
                    e.patrolDirection = -1;
                }
                else if (distFromStart < -e.patrolRange)
                {
                    e.patrolDirection = 1;
                }
                
                e.velocityX = e.patrolDirection * (e.moveSpeed * 0.5f);
                e.isFacingRight = (e.patrolDirection > 0);
            }
        }

        // 重力
        if (!e.isGrounded)
        {
            e.velocityY += GRAVITY;
            if (e.velocityY > MAX_FALL_SPEED) e.velocityY = MAX_FALL_SPEED;
        }

        // 位置更新
        e.posX += e.velocityX;
        e.posY += e.velocityY;
        
        // マップとの当たり判定
        ResolveEnemyMapCollision(e);

        // コライダー更新
        if (e.colliderId != -1)
        {
            float left = e.posX - (e.width * 0.5f);
            float top = e.posY - e.height;
            UpdateCollider(e.colliderId, left, top, e.width, e.height);
        }

        // アニメーション更新
        if (e.animations != nullptr)
        {
            if (std::fabs(e.velocityX) > 0.01f)
                UpdateAnimation(e.animations->move);
            else
                UpdateAnimation(e.animations->idle);
        }
    }
}



void DrawEnemies()
{
    CameraData camera = GetCamera();
    
    // デバッグ: 大きな文字で表示
    DrawFormatString(400, 20, GetColor(255, 0, 255), "*** DRAW ENEMIES v2.0 ***");
    DrawFormatString(400, 50, GetColor(255, 255, 0), "Enemy Count: %d", g_enemies.size());
    DrawFormatString(400, 70, GetColor(255, 255, 0), "Camera: (%.1f, %.1f)", camera.posX, camera.posY);
    
    int debugIndex = 0;
    for (const auto& e : g_enemies)
    {
        DrawFormatString(400, 90 + debugIndex * 20, GetColor(0, 255, 255), 
                        "=== ENEMY %d ===", debugIndex);
        
        if (!e.active) 
        {
            DrawFormatString(400, 110 + debugIndex * 20, GetColor(255, 0, 0), "INACTIVE!");
            debugIndex++;
            continue;
        }

        // 敵のposYは足元。プレイヤーと同じ座標系
        int drawX = static_cast<int>((e.posX - camera.posX) * camera.scale);
        int drawY = static_cast<int>((e.posY - camera.posY) * camera.scale);

        DrawFormatString(400, 110 + debugIndex * 60, GetColor(255, 255, 255), 
                        "World: (%.0f, %.0f)", e.posX, e.posY);
        DrawFormatString(400, 130 + debugIndex * 60, GetColor(255, 255, 255), 
                        "Screen: (%d, %d)", drawX, drawY);

        // 赤丸で敵の足元を表示
        DrawCircle(drawX, drawY, 15, GetColor(255, 0, 0), TRUE);
        DrawCircle(drawX, drawY, 18, GetColor(255, 255, 0), FALSE);
        DrawCircle(drawX, drawY, 3, GetColor(0, 255, 0), TRUE);
        DrawFormatString(drawX + 20, drawY - 15, GetColor(255, 255, 0), "SLIME %d", debugIndex);



        // 画像描画位置は足元から高さ分上
        int drawTopY = drawY - static_cast<int>(e.height * camera.scale);

        if (e.animations != nullptr)
        {
            AnimationData* currentAnim = nullptr;
            
            if (std::fabs(e.velocityX) > 0.01f)
                currentAnim = &e.animations->move;
            else
                currentAnim = &e.animations->idle;
            
            if (currentAnim != nullptr && currentAnim->frames != nullptr)
            {
                DrawAnimation(*currentAnim, drawX - static_cast<int>(e.width * 0.5f * camera.scale), drawTopY, !e.isFacingRight);
            }
        }
        else
        {
            int halfW = static_cast<int>(e.width * 0.5f * camera.scale);
            int h = static_cast<int>(e.height * camera.scale);

            unsigned int color = GetColor(200, 50, 50);
            DrawBox(drawX - halfW, drawTopY, drawX + halfW, drawY, color, TRUE);

            if (e.isFacingRight)
            {
                DrawTriangle(drawX + halfW, drawTopY + h / 2, drawX + halfW - 8, drawTopY + h / 2 - 6, 
                           drawX + halfW - 8, drawTopY + h / 2 + 6, GetColor(255, 255, 0), TRUE);
            }
            else
            {
                DrawTriangle(drawX - halfW, drawTopY + h / 2, drawX - halfW + 8, drawTopY + h / 2 - 6, 
                           drawX - halfW + 8, drawTopY + h / 2 + 6, GetColor(255, 255, 0), TRUE);
            }
        }
        
        debugIndex++;
    }
}





// アクセサ
EnemyData* GetEnemy(int index)
{
    if (index < 0 || index >= (int)g_enemies.size()) return nullptr;
    return &g_enemies[index];
}

int GetEnemyCount()
{
    return static_cast<int>(g_enemies.size());
}

//============================================================
// アニメーション管理
//============================================================

EnemyAnimations* LoadEnemyAnimations(EnemyType type)
{
    EnemyAnimations* anims = new EnemyAnimations();
    const char* basePath = GetEnemyConfig(type).animationPath;
    
    switch (type)
    {
    case EnemyType::Slime:
        LoadAnimationFromFiles(anims->idle, "Data/Enemy/Slime/", "slime-idle", 4, 8, AnimationMode::Loop);
        LoadAnimationFromFiles(anims->move, "Data/Enemy/Slime/", "slime-move", 4, 6, AnimationMode::Loop);
        LoadAnimationFromFiles(anims->attack, "Data/Enemy/Slime/", "slime-attack", 5, 4, AnimationMode::Once);
        LoadAnimationFromFiles(anims->hurt, "Data/Enemy/Slime/", "slime-hurt", 4, 3, AnimationMode::Once);
        LoadAnimationFromFiles(anims->die, "Data/Enemy/Slime/", "slime-die", 4, 5, AnimationMode::Once);
        break;
        
    default:
        // 他の敵は未実装
        InitAnimation(anims->idle);
        InitAnimation(anims->move);
        InitAnimation(anims->attack);
        InitAnimation(anims->hurt);
        InitAnimation(anims->die);
        break;
    }
    
    return anims;
}

void UnloadEnemyAnimations(EnemyAnimations* anims)
{
    if (anims != nullptr)
    {
        UnloadAnimation(anims->idle);
        UnloadAnimation(anims->move);
        UnloadAnimation(anims->attack);
        UnloadAnimation(anims->hurt);
        UnloadAnimation(anims->die);
        delete anims;
    }
}
