#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include "../Animation/Animation.h"
#include "../Map/MapManager.h"
#include "../Camera/Camera.h"
#include "DxLib.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "../Map/MapParameter.h"

EnemyType StringToEnemyType(const std::string& str)
{
    if (str == "Slime") return EnemyType::Slime;
    if (str == "Cultists") return EnemyType::Cultists;
    if (str == "AssassinCultist") return EnemyType::AssassinCultist;
    if (str == "BigQuartist") return EnemyType::BigQuartist;
    if (str == "TwistedCaltis") return EnemyType::TwistedCaltis;
    printf("Unknown enemy type: %s\n", str.c_str());
    return EnemyType::Slime; // fallback
}
namespace
{
    int g_loadedEnemyCount = 0;
    bool g_enemyCSVLoaded = false;
    std::string g_enemyCSVPath = "";
    bool g_enemyCSVFailed = false;

    std::vector<EnemyData> g_enemies;
    static int g_EnemyDebugFrame = 0;

    float GetEnemyBodyColliderScale(EnemyType type)
    {
        (void)type;
        return 0.82f;
    }

    float GetEnemyBodyColliderWidthAdjust(EnemyType type)
    {
        (void)type;
        return 0.50f;
    }

    float GetEnemyBodyColliderFacingOffsetRatio(EnemyType type)
    {
        (void)type;
        return 0.0f;
    }

    bool TryGetAnimationFrameSize(const AnimationData& anim, float& outWidth, float& outHeight)
    {
        if (anim.frames == nullptr || anim.frameCount <= 0 || anim.frames[0] == -1)
        {
            return false;
        }

        int w = 0;
        int h = 0;
        GetGraphSize(anim.frames[0], &w, &h);
        if (w <= 0 || h <= 0)
        {
            return false;
        }

        outWidth = static_cast<float>(w);
        outHeight = static_cast<float>(h);
        return true;
    }

    bool TryGetEnemyVisualSize(EnemyAnimations* anims, float& outWidth, float& outHeight)
    {
        if (anims == nullptr)
        {
            return false;
        }

        if (TryGetAnimationFrameSize(anims->idle, outWidth, outHeight)) return true;
        if (TryGetAnimationFrameSize(anims->move, outWidth, outHeight)) return true;
        if (TryGetAnimationFrameSize(anims->attack, outWidth, outHeight)) return true;
        if (TryGetAnimationFrameSize(anims->die, outWidth, outHeight)) return true;

        return false;
    }

    enum EnemyProjectileKind
    {
        ProjectileCultistFireball = 0,
        ProjectileBigBook = 1,
        ProjectileBigFlame = 2,
        ProjectileBigShockwave = 3
    };

    struct EnemyProjectile
    {
        bool active;
        EnemyData* owner;
        float posX;
        float posY;
        float velocityX;
        float velocityY;
        float width;
        float height;
        float lifeTimer;
        int colliderId;
        int animFrame;
        int animCounter;
        int kind;
        float traveledDistance;
        float maxTravelDistance;
    };

    std::vector<EnemyProjectile> g_enemyProjectiles;
    int g_cultistFireballFrames[4] = { -1, -1, -1, -1 };

    const float CULTIST_FIREBALL_SPEED = 4.8f;
    const float CULTIST_FIREBALL_LIFE = 2.0f;
    const float CULTIST_FIREBALL_SIZE = 34.0f;

    void DestroyProjectile(EnemyProjectile& p)
    {
        if (p.colliderId != -1)
        {
            DestroyCollider(p.colliderId);
            p.colliderId = -1;
        }
        p.active = false;
    }

    void ClearEnemyProjectiles()
    {
        for (auto& p : g_enemyProjectiles)
        {
            DestroyProjectile(p);
        }
        g_enemyProjectiles.clear();
    }

    void InitCultistFireballAssets()
    {
        const char* paths[4] = {
            "Data/Enemy/cultists/Cultist-Attack_FireBall_1.png",
            "Data/Enemy/cultists/Cultist-Attack_FireBall_2.png",
            "Data/Enemy/cultists/Cultist-Attack_FireBall_3.png",
            "Data/Enemy/cultists/Cultist-Attack_FireBall_4.png"
        };

        for (int i = 0; i < 4; i++)
        {
            if (g_cultistFireballFrames[i] == -1)
            {
                g_cultistFireballFrames[i] = LoadGraph(paths[i]);
            }
        }
    }

    void ReleaseCultistFireballAssets()
    {
        for (int i = 0; i < 4; i++)
        {
            if (g_cultistFireballFrames[i] != -1)
            {
                DeleteGraph(g_cultistFireballFrames[i]);
                g_cultistFireballFrames[i] = -1;
            }
        }
    }

    void SpawnCultistFireball(EnemyData& e, const PlayerData& player)
    {
        float dx = player.posX - e.posX;
        float dy = (player.posY - PLAYER_HEIGHT * 0.5f) - (e.posY - e.height * 0.65f);
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
        {
            dx = e.isFacingRight ? 1.0f : -1.0f;
            dy = 0.0f;
            len = 1.0f;
        }

        EnemyProjectile p{};
        p.active = true;
        p.owner = &e;
        p.width = CULTIST_FIREBALL_SIZE;
        p.height = CULTIST_FIREBALL_SIZE;
        p.posX = e.posX + (e.isFacingRight ? e.width * 0.4f : -e.width * 0.4f);
        p.posY = e.posY - e.height * 0.6f;
        p.velocityX = (dx / len) * CULTIST_FIREBALL_SPEED;
        p.velocityY = (dy / len) * CULTIST_FIREBALL_SPEED;
        p.lifeTimer = CULTIST_FIREBALL_LIFE;
        p.colliderId = CreateCollider(
            ColliderTag::Attack,
            p.posX - p.width * 0.5f,
            p.posY - p.height * 0.5f,
            p.width,
            p.height,
            p.owner);
        p.animFrame = 0;
        p.animCounter = 0;
        p.kind = ProjectileCultistFireball;
        p.traveledDistance = 0.0f;
        p.maxTravelDistance = 0.0f;
        g_enemyProjectiles.push_back(p);
    }

    void UpdateEnemyProjectiles(float slowMoScale)
    {
        const float frameTime = (1.0f / 60.0f) * slowMoScale;
        const float mapBlockSize = 32.0f;

        for (auto& p : g_enemyProjectiles)
        {
            if (!p.active) continue;

            p.lifeTimer -= frameTime;
            const float prevX = p.posX;
            const float prevY = p.posY;
            p.posX += p.velocityX * slowMoScale;
            p.posY += p.velocityY * slowMoScale;

            p.animCounter++;
            if (p.animCounter >= 4)
            {
                p.animCounter = 0;
                p.animFrame = (p.animFrame + 1) % 4;
            }

            if (p.lifeTimer <= 0.0f)
            {
                DestroyProjectile(p);
                continue;
            }

            const float moveDx = p.posX - prevX;
            const float moveDy = p.posY - prevY;
            const float moveLen = std::sqrt(moveDx * moveDx + moveDy * moveDy);
            int raySteps = static_cast<int>(moveLen / (mapBlockSize * 0.25f)) + 1;
            if (raySteps < 1) raySteps = 1;
            if (raySteps > 16) raySteps = 16;

            bool hitBlock = false;
            for (int i = 0; i <= raySteps; i++)
            {
                const float t = static_cast<float>(i) / static_cast<float>(raySteps);
                const float checkX = prevX + moveDx * t;
                const float checkY = prevY + moveDy * t;
                const int gridX = static_cast<int>(checkX / mapBlockSize);
                const int gridY = static_cast<int>(checkY / mapBlockSize);
                MapChipData* mc = GetMapChipData(gridX, gridY);
                if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
                {
                    hitBlock = true;
                    break;
                }
            }

            if (hitBlock)
            {
                DestroyProjectile(p);
                continue;
            }

            if (p.colliderId != -1)
            {
                UpdateCollider(
                    p.colliderId,
                    p.posX - p.width * 0.5f,
                    p.posY - p.height * 0.5f,
                    p.width,
                    p.height);
            }
        }
    }

    void DrawEnemyProjectiles(const CameraData& camera)
    {
        for (const auto& p : g_enemyProjectiles)
        {
            if (!p.active) continue;

            int drawX = static_cast<int>((p.posX - camera.posX) * camera.scale);
            int drawY = static_cast<int>((p.posY - camera.posY) * camera.scale);
            int halfW = static_cast<int>(p.width * 0.5f * camera.scale);
            int halfH = static_cast<int>(p.height * 0.5f * camera.scale);
            int left = drawX - halfW;
            int top = drawY - halfH;
            int right = drawX + halfW;
            int bottom = drawY + halfH;

            const int frameHandle = g_cultistFireballFrames[p.animFrame % 4];
            if (frameHandle != -1)
            {
                if (p.velocityX > 0.0f)
                {
                    DrawExtendGraph(right, top, left, bottom, frameHandle, TRUE);
                }
                else
                {
                    DrawExtendGraph(left, top, right, bottom, frameHandle, TRUE);
                }
            }
            else
            {
                DrawBox(left, top, right, bottom, GetColor(255, 120, 30), TRUE);
            }
        }
    }

    bool LoadAnimationFromFiles_Indexed(AnimationData& anim, const char* basePath, const char* prefix, int startIndex, int frameCount, int animSpeed, AnimationMode mode)
    {
        InitAnimation(anim);
        if (frameCount <= 0) return false;

        anim.frames = new int[frameCount];
        anim.frameCount = frameCount;
        anim.animationSpeed = animSpeed;
        anim.mode = mode;

        char filePath[256];
        for (int i = 0; i < frameCount; i++)
        {
            const int fileIndex = startIndex + i;

            sprintf_s(filePath, sizeof(filePath), "%s%s_%d.png", basePath, prefix, fileIndex);
            anim.frames[i] = LoadGraph(filePath);

            if (anim.frames[i] == -1)
            {
                sprintf_s(filePath, sizeof(filePath), "%s%s%d.png", basePath, prefix, fileIndex);
                anim.frames[i] = LoadGraph(filePath);
            }

            if (anim.frames[i] == -1)
            {
                for (int j = 0; j < i; j++)
                {
                    if (anim.frames[j] != -1) DeleteGraph(anim.frames[j]);
                }
                delete[] anim.frames;
                anim.frames = nullptr;
                anim.frameCount = 0;
                return false;
            }
        }
        return true;
    }

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
        { 12, 4, 1.0f, 52.0f, 44.0f, 140.0f, 48.0f, 0.45f, 1.5f, false, 0.0f, "Assets/Enemies/Slime/" },
        
        // Cultists: 近接＋火球の戦闘員
        { 40, 12, 1.2f, 50.0f, 50.0f, 320.0f, 96.0f, 0.35f, 1.0f, false, 0.0f, "Assets/Enemies/Cultists/" },
        
        // AssassinCultist: 素早く動く刺客、攻撃は弱め
        { 30, 8, 2.0f, 50.0f, 50.0f, 176.0f, 44.0f, 0.22f, 1.5f, true, 9.0f, "Assets/Enemies/AssassinCultist/" },
        
        // BigQuartist: ボス級の大型敵、低速高耐久
        { 250, 30, 0.8f, 150.0f, 150.0f, 360.0f, 88.0f, 0.75f, 1.8f, false, 0.0f, "Assets/Enemies/BigQuartist/" },
        
        // TwistedCaltis: 変則的な動きをする中ボス
        { 90, 16, 1.8f, 50.0f, 50.0f, 320.0f, 128.0f, 0.50f, 1.2f, true, 10.0f, "Assets/Enemies/TwistedCaltis/" }
    };

    const float GRAVITY = 0.3f;
    const float MAX_FALL_SPEED = 6.0f;
    const float PATROL_RANGE_BLOCKS = 5.0f;
    const float BLOCK_SIZE = 32.0f;
    const float FACE_TURN_DEADZONE = 12.0f;
    const int BIG_QUARTIST_ATTACK_HIT_START_FRAME = 5;
    const float ENEMY_ATTACK_WIDTH_SCALE = 0.75f;
    const float ENEMY_ATTACK_HEIGHT_SCALE = 0.70f;
    const float ENEMY_ATTACK_FRONT_OFFSET_RATIO = 0.34f;
    const float BIG_ATTACK_WIDTH_SCALE = 0.75f;
    const float BIG_ATTACK_HEIGHT_SCALE = 0.72f;
    const float BIG_ATTACK_BACK_OFFSET_RATIO = 0.12f;
    const float BIG_ATTACK_FRONT_OFFSET_RATIO = 0.55f;
    const float TWISTED_ATTACK_WIDTH_SCALE = 1.65f;
    const float TWISTED_ATTACK_HEIGHT_SCALE = 0.45f;
    const float TWISTED_ATTACK_FRONT_OFFSET_RATIO = 0.65f;
    const float TWISTED_ATTACK_CENTER_Y_RATIO = 0.62f;

    // レイキャストによる視線判定（ブロックを透視できない）
    bool HasLineOfSight(float fromX, float fromY, float toX, float toY)
    {
        if (!std::isfinite(fromX) || !std::isfinite(fromY) || !std::isfinite(toX) || !std::isfinite(toY))
        {
            return false;
        }

        float dx = toX - fromX;
        float dy = toY - fromY;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (!std::isfinite(distance) || distance < 0.01f) return true;

        int steps = static_cast<int>(distance / 8.0f) + 1;
        if (steps > 256) steps = 256;
        if (steps < 1) steps = 1;

        float stepX = dx / steps;
        float stepY = dy / steps;

        for (int i = 0; i < steps; i++)
        {
            float checkX = fromX + stepX * i;
            float checkY = fromY + stepY * i;

            if (!std::isfinite(checkX) || !std::isfinite(checkY))
            {
                return false;
            }

            int gridX = static_cast<int>(checkX / BLOCK_SIZE);
            int gridY = static_cast<int>(checkY / BLOCK_SIZE);

            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                return false;
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
        
        // 上方向の当たり判定（複数点チェック）
        for (int i = 0; i < 3; i++)
        {
            float checkX = checkLeft + (checkRight - checkLeft) * (i / 2.0f);
            int gridX = static_cast<int>(checkX / BLOCK_SIZE);
            int gridY = static_cast<int>((checkTop - 1.0f) / BLOCK_SIZE);
            
            MapChipData* mc = GetMapChipData(gridX, gridY);
            if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
            {
                float blockBottom = gridY * BLOCK_SIZE;
                if (e.posY < blockBottom && e.velocityY <= 0.0f)
                {
                    e.posY = blockBottom;
                    e.velocityY = 0.0f;
                    e.isGrounded = true;
                }
            }
        }
        
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
            bool hitWall = false;
            float nearestBlockRight = -1.0f;

            for (int i = 0; i < 3; i++)
            {
                const float sampleY = (e.posY - e.height) + (e.height * (i / 2.0f));
                int gridY = static_cast<int>(sampleY / BLOCK_SIZE);
                MapChipData* mc = GetMapChipData(gridX, gridY);
                if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
                {
                    hitWall = true;
                    nearestBlockRight = (gridX + 1) * BLOCK_SIZE;
                    break;
                }
            }

            if (hitWall && e.posX - halfW < nearestBlockRight)
            {
                e.posX = nearestBlockRight + halfW;
                e.velocityX = 0.0f;
                e.patrolDirection = 1; // 方向転換
            }
        }
        else if (e.velocityX > 0.0f) // 右移動
        {
            int gridX = static_cast<int>((checkRight + 1.0f) / BLOCK_SIZE);
            bool hitWall = false;
            float nearestBlockLeft = -1.0f;

            for (int i = 0; i < 3; i++)
            {
                const float sampleY = (e.posY - e.height) + (e.height * (i / 2.0f));
                int gridY = static_cast<int>(sampleY / BLOCK_SIZE);
                MapChipData* mc = GetMapChipData(gridX, gridY);
                if (mc != nullptr && mc->mapChip == NORMAL_BLOCK)
                {
                    hitWall = true;
                    nearestBlockLeft = gridX * BLOCK_SIZE;
                    break;
                }
            }

            if (hitWall && e.posX + halfW > nearestBlockLeft)
            {
                e.posX = nearestBlockLeft - halfW;
                e.velocityX = 0.0f;
                e.patrolDirection = -1; // 方向転換
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
    ClearEnemyProjectiles();
    InitCultistFireballAssets();
}

int SpawnEnemy(EnemyType type, float x, float y)
{
    const EnemyConfig& config = GetEnemyConfig(type);

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
    e.isAttacking = false;
    e.attackColliderId = -1;
    e.behaviorPattern = -1;
    e.isInvisible = false;
    e.specialTimer = 0.0f;
    e.isDying = false;
    e.deathAnimFinished = false;
    e.deathBlinkTimer = 0;
    e.width = config.width;
    e.height = config.height;

    e.animations = LoadEnemyAnimations(type);

    float visualWidth = 0.0f;
    float visualHeight = 0.0f;
    if (TryGetEnemyVisualSize(e.animations, visualWidth, visualHeight))
    {
        e.width = (visualWidth > config.width) ? visualWidth : config.width;
        e.height = (visualHeight > config.height) ? visualHeight : config.height;
    }

    if (e.type == EnemyType::Cultists)
    {
        e.width = 50.0f;
        e.height = 50.0f;
    }
    else if (e.type == EnemyType::TwistedCaltis)
    {
        e.width = 50.0f;
        e.height = 50.0f;
    }

    if (e.type != EnemyType::BigQuartist)
    {
        e.width = 75.0f;
        e.height = 75.0f;
    }

    const float bodyScale = GetEnemyBodyColliderScale(e.type);
    const float bodyWidth = e.width * bodyScale * GetEnemyBodyColliderWidthAdjust(e.type);
    const float bodyHeight = e.height * bodyScale;
    const float bodyCenterY = e.posY - (e.height * 0.5f);
    const float bodyLeft = e.posX - (bodyWidth * 0.5f);
    const float bodyTop = bodyCenterY - (bodyHeight * 0.5f);
    e.colliderId = CreateCollider(ColliderTag::Enemy, bodyLeft, bodyTop, bodyWidth, bodyHeight, nullptr);

    g_enemies.push_back(e);
    return static_cast<int>(g_enemies.size() - 1);
}

void LoadEnemiesFromCSV(const char* stageName)
{
    std::string path = "Data/Enemy/EnemySpawn/";
    path += stageName;
    path += "_Spawn.csv";
    std::ifstream file(path);
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    std::getline(file, line); // ヘッダ

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string typeStr, xStr, yStr;

        std::getline(ss, typeStr, ',');
        std::getline(ss, xStr, ',');
        std::getline(ss, yStr, ',');

        if (typeStr.empty() || xStr.empty() || yStr.empty()) continue;

        try
        {
            EnemyType type = StringToEnemyType(typeStr);

            float gridX = std::stof(xStr);
            float gridY = std::stof(yStr);

            gridX -= 1;
            gridY -= 1;

            float x = gridX * MAP_CHIP_WIDTH;
            float y = gridY * MAP_CHIP_HEIGHT;

            SpawnEnemy(type, x, y);
        }
        catch (...)
        {
            printf("Invalid CSV line skipped: %s\n", line.c_str());
        }
    }

    file.close();
}



// 敵破棄
void DespawnEnemy(int index)
{
    if (index < 0 || index >= (int)g_enemies.size()) return;
    EnemyData& e = g_enemies[index];

    for (auto& p : g_enemyProjectiles)
    {
        if (p.active && p.owner == &e)
        {
            DestroyProjectile(p);
        }
    }

    if (e.colliderId != -1)
    {
        DestroyCollider(e.colliderId);
        e.colliderId = -1;
    }
    if (e.attackColliderId != -1)
    {
        DestroyCollider(e.attackColliderId);
        e.attackColliderId = -1;
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
        if (e.attackColliderId != -1)
        {
            DestroyCollider(e.attackColliderId);
            e.attackColliderId = -1;
        }
        if (e.animations != nullptr)
        {
            UnloadEnemyAnimations(e.animations);
            e.animations = nullptr;
        }
    }
    g_enemies.clear();
    ClearEnemyProjectiles();
    ReleaseCultistFireballAssets();
}


void UpdateEnemies()
{
    PlayerData& player = GetPlayerData();
    const float FRAME_TIME = 1.0f / 60.0f;
    const float slowMoScale = GetDeathSlowMotionScale();
    const float effectiveFrameTime = FRAME_TIME * slowMoScale;

    for (auto& e : g_enemies)
    {
        if (!e.active) continue;

        if (!std::isfinite(e.posX) || !std::isfinite(e.posY) || !std::isfinite(e.velocityX) || !std::isfinite(e.velocityY))
        {
            e.active = false;
            continue;
        }

        // 死亡開始
        if (e.currentHP <= 0 && !e.isDying)
        {
            e.isDying = true;
            e.isAttacking = false;
            e.isInvisible = false;
            e.velocityX = 0.0f;
            e.velocityY = 0.0f;
            e.deathAnimFinished = false;
            e.deathBlinkTimer = 0;

            if (e.attackColliderId != -1)
            {
                DestroyCollider(e.attackColliderId);
                e.attackColliderId = -1;
            }
            if (e.colliderId != -1)
            {
                DestroyCollider(e.colliderId);
                e.colliderId = -1;
            }
            if (e.animations != nullptr)
            {
                ResetAnimation(e.animations->die);
            }
        }

        // 死亡演出中
        if (e.isDying)
        {
            if (e.animations != nullptr)
            {
                if (!e.deathAnimFinished)
                {
                    UpdateAnimation(e.animations->die);
                    if (e.animations->die.frames == nullptr || IsAnimationFinished(e.animations->die))
                    {
                        e.deathAnimFinished = true;
                        e.deathBlinkTimer = 0;
                    }
                }
                else
                {
                    e.deathBlinkTimer++;
                    if (e.deathBlinkTimer >= 90)
                    {
                        UnloadEnemyAnimations(e.animations);
                        e.animations = nullptr;
                        e.active = false;
                    }
                }
            }
            else
            {
                e.active = false;
            }
            continue;
        }

        float dx = player.posX - e.posX;
        float dy = player.posY - e.posY;
        float dist = std::sqrt(dx * dx + dy * dy);

        e.hasLineOfSight = false;
        if (std::isfinite(dist) && dist <= e.detectRange)
        {
            float eyeY = e.posY - e.height * 0.7f;
            float playerCenterY = player.posY - (PLAYER_HEIGHT * 0.5f);
            e.hasLineOfSight = HasLineOfSight(e.posX, eyeY, player.posX, playerCenterY);
        }

        if (dist <= e.detectRange && e.hasLineOfSight)
        {
            e.isAggro = true;
            if (!e.isAttacking && std::fabs(dx) > FACE_TURN_DEADZONE)
            {
                e.isFacingRight = (dx >= 0.0f);
            }
        }
        else
        {
            e.isAggro = false;
        }

        if (e.cooldownTimer > 0.0f) e.cooldownTimer -= effectiveFrameTime;

        if (e.isAttacking)
        {
            e.attackTimer -= effectiveFrameTime;
            e.velocityX *= ((e.type == EnemyType::AssassinCultist || e.type == EnemyType::BigQuartist) ? 0.95f : 0.93f);

            if (e.attackTimer <= 0.0f)
            {
                e.isAttacking = false;
                e.velocityX = 0.0f;

                if (e.attackColliderId != -1)
                {
                    DestroyCollider(e.attackColliderId);
                    e.attackColliderId = -1;
                }

                if (e.type == EnemyType::AssassinCultist && !e.isInvisible)
                {
                    e.behaviorPattern = -1;
                }
                else if (e.type == EnemyType::BigQuartist)
                {
                    e.behaviorPattern = (e.behaviorPattern >= 10) ? 11 : 1;
                }
            }
        }

        const bool specialType = (e.type == EnemyType::AssassinCultist || e.type == EnemyType::BigQuartist);
        if (specialType)
        {
            const bool isBig = (e.type == EnemyType::BigQuartist);
            const float warpWait = 0.25f;
            const float warpOffsetDist = 36.0f;
            const float warpLungeMul = 6.0f;
            const float approachMul = isBig ? 1.05f : 1.8f;
            const float attackWScale = isBig ? BIG_ATTACK_WIDTH_SCALE : ENEMY_ATTACK_WIDTH_SCALE;
            const float attackFrontOffset = isBig ? (e.width * 0.2f) : (e.width * ENEMY_ATTACK_FRONT_OFFSET_RATIO);

            if (!isBig && e.isInvisible)
            {
                e.specialTimer -= effectiveFrameTime;
                e.velocityX = 0.0f;

                if (e.specialTimer <= 0.0f)
                {
                    const float warpOffset = (dx >= 0.0f) ? -warpOffsetDist : warpOffsetDist;
                    e.posX = player.posX + warpOffset;
                    e.posY = player.posY;
                    e.isFacingRight = (player.posX >= e.posX);
                    e.isInvisible = false;

                    if (e.cooldownTimer <= 0.0f)
                    {
                        e.isAttacking = true;
                        e.attackTimer = e.attackDuration;
                        e.cooldownTimer = e.attackCooldown;
                        e.behaviorPattern = 0;

                        const float lungeSpeed = e.moveSpeed * warpLungeMul;
                        e.velocityX = e.isFacingRight ? lungeSpeed : -lungeSpeed;

                        if (e.attackColliderId != -1)
                        {
                            DestroyCollider(e.attackColliderId);
                            e.attackColliderId = -1;
                        }

                        const float attackWidth = e.width * attackWScale;
                        const float attackHeight = e.height * ENEMY_ATTACK_HEIGHT_SCALE;
                        const float attackOffsetX = e.isFacingRight ? attackFrontOffset : -attackFrontOffset - attackWidth;
                        const float attackLeft = e.posX + attackOffsetX;
                        const float attackTop = e.posY - attackHeight;
                        e.attackColliderId = CreateCollider(ColliderTag::Attack, attackLeft, attackTop, attackWidth, attackHeight, &e);

                        if (e.animations != nullptr)
                        {
                            ResetAnimation(e.animations->attack);
                        }
                    }
                }
            }
            else
            {
                if (e.behaviorPattern == 1 && !e.isAttacking)
                {
                    e.isFacingRight = (dx >= 0.0f);
                    const bool closeEnough = (std::fabs(dx) <= e.attackRange) && (std::fabs(dy) <= e.height * 1.2f);
                    if (!closeEnough)
                    {
                        const float approachSpeed = e.moveSpeed * approachMul;
                        e.velocityX = (dx >= 0.0f) ? approachSpeed : -approachSpeed;
                    }
                    else if (e.cooldownTimer <= 0.0f)
                    {
                        e.isAttacking = true;
                        e.attackTimer = e.attackDuration;
                        e.cooldownTimer = e.attackCooldown;
                        e.velocityX = 0.0f;

                        if (e.attackColliderId != -1)
                        {
                            DestroyCollider(e.attackColliderId);
                            e.attackColliderId = -1;
                        }

                        const float attackWidth = e.width * attackWScale;
                        const float attackHeight = e.height * ENEMY_ATTACK_HEIGHT_SCALE;
                        const float attackOffsetX = e.isFacingRight ? attackFrontOffset : -attackFrontOffset - attackWidth;
                        const float attackLeft = e.posX + attackOffsetX;
                        const float attackTop = e.posY - attackHeight;
                        if (!isBig)
                        {
                            e.attackColliderId = CreateCollider(ColliderTag::Attack, attackLeft, attackTop, attackWidth, attackHeight, &e);
                        }

                        if (e.animations != nullptr)
                        {
                            ResetAnimation(e.animations->attack);
                        }
                    }
                }
                else
                {
                    const bool canStart = (!e.isAttacking && e.cooldownTimer <= 0.0f && e.isAggro);
                    const bool inWarpTrigger = (!isBig) && (std::fabs(dx) <= e.detectRange * 0.9f) && (std::fabs(dy) <= e.height * 1.5f);
                    const bool inApproachTrigger = (std::fabs(dx) <= e.detectRange) && (std::fabs(dy) <= e.height * 1.5f);

                    if (canStart && (inWarpTrigger || inApproachTrigger))
                    {
                        const int action = isBig ? 1 : GetRand(1);
                        e.behaviorPattern = action;

                        if (action == 0)
                        {
                            e.isInvisible = true;
                            e.specialTimer = warpWait;
                            e.velocityX = 0.0f;
                        }
                        else
                        {
                            e.velocityX = 0.0f;
                        }
                    }
                }
            }
        }
        else
        {
            const bool isCultist = (e.type == EnemyType::Cultists);
            const bool inAttackRange = (std::fabs(dx) <= e.attackRange) && (std::fabs(dy) <= e.height);
            const bool canShootFireball = isCultist && e.isAggro &&
                (std::fabs(dx) > e.attackRange * 1.2f) &&
                (std::fabs(dx) <= e.detectRange) &&
                (std::fabs(dy) <= e.height * 1.3f);

            if (!e.isAttacking && e.cooldownTimer <= 0.0f && inAttackRange)
            {
                e.isAttacking = true;
                e.attackTimer = e.attackDuration;
                e.cooldownTimer = e.attackCooldown;

                const float lungeSpeed = e.moveSpeed * 5.0f;
                e.velocityX = (dx >= 0.0f) ? lungeSpeed : -lungeSpeed;
                e.isFacingRight = (dx >= 0.0f);

                if (e.attackColliderId != -1)
                {
                    DestroyCollider(e.attackColliderId);
                    e.attackColliderId = -1;
                }

                const bool isTwisted = (e.type == EnemyType::TwistedCaltis);
                const float attackWidth = isTwisted ? (e.width * TWISTED_ATTACK_WIDTH_SCALE) : (e.width * 1.3f);
                const float attackHeight = isTwisted ? (e.height * TWISTED_ATTACK_HEIGHT_SCALE) : e.height;
                const float attackFrontOffset = isTwisted ? (e.width * TWISTED_ATTACK_FRONT_OFFSET_RATIO) : (e.width * 0.5f);
                const float attackOffsetX = e.isFacingRight ? attackFrontOffset : -attackFrontOffset - attackWidth;
                const float attackLeft = e.posX + attackOffsetX;
                const float attackCenterY = isTwisted ? (e.posY - e.height * TWISTED_ATTACK_CENTER_Y_RATIO) : (e.posY - attackHeight * 0.5f);
                const float attackTop = attackCenterY - attackHeight * 0.5f;
                e.attackColliderId = CreateCollider(ColliderTag::Attack, attackLeft, attackTop, attackWidth, attackHeight, &e);

                if (e.animations != nullptr)
                {
                    ResetAnimation(e.animations->attack);
                }
            }
            else if (!e.isAttacking && e.cooldownTimer <= 0.0f && canShootFireball)
            {
                e.isAttacking = true;
                e.attackTimer = 0.32f;
                e.cooldownTimer = e.attackCooldown;
                e.velocityX = 0.0f;
                e.isFacingRight = (dx >= 0.0f);

                if (e.attackColliderId != -1)
                {
                    DestroyCollider(e.attackColliderId);
                    e.attackColliderId = -1;
                }

                SpawnCultistFireball(e, player);

                if (e.animations != nullptr)
                {
                    ResetAnimation(e.animations->attack);
                }
            }
        }

        const bool approachMode = (e.type == EnemyType::AssassinCultist && e.behaviorPattern == 1 && !e.isInvisible) || (e.type == EnemyType::BigQuartist);
        if (!e.isAttacking && !e.isInvisible && !approachMode && e.cooldownTimer <= 0.0f)
        {
            if (e.isAggro)
            {
                const float chaseSpeed = e.moveSpeed;
                if (std::fabs(dx) > e.attackRange * 0.5f)
                {
                    e.velocityX = (dx > 0.0f) ? chaseSpeed : -chaseSpeed;
                }
                else
                {
                    e.velocityX = 0.0f;
                }
            }
            else if (e.isGrounded)
            {
                float distFromStart = e.posX - e.patrolStartX;
                if (distFromStart > e.patrolRange) e.patrolDirection = -1;
                else if (distFromStart < -e.patrolRange) e.patrolDirection = 1;

                e.velocityX = e.patrolDirection * (e.moveSpeed * 0.5f);
                e.isFacingRight = (e.patrolDirection > 0);
            }
        }
        else if (!e.isAttacking && !e.isInvisible && !approachMode)
        {
            e.velocityX = 0.0f;
        }

        if (!e.isGrounded && e.type != EnemyType::BigQuartist)
        {
            e.velocityY += GRAVITY * slowMoScale;
            if (e.velocityY > MAX_FALL_SPEED) e.velocityY = MAX_FALL_SPEED;
        }

        e.posX += e.velocityX * slowMoScale;
        e.posY += e.velocityY * slowMoScale;

        const float mapBottom = static_cast<float>(GetMapHeight());
        if (mapBottom > 0.0f && e.posY > mapBottom)
        {
            e.posY = mapBottom;
            e.velocityY = 0.0f;
            e.isGrounded = true;
        }

        ResolveEnemyMapCollision(e);

        const float moveX = e.velocityX * slowMoScale;
        const float moveY = e.velocityY * slowMoScale;
        const float maxMoveAbs = (std::fabs(moveX) > std::fabs(moveY)) ? std::fabs(moveX) : std::fabs(moveY);
        int moveSteps = static_cast<int>(maxMoveAbs / 8.0f) + 1;
        if (moveSteps < 1) moveSteps = 1;
        if (moveSteps > 16) moveSteps = 16;

        const float stepX = moveX / static_cast<float>(moveSteps);
        const float stepY = moveY / static_cast<float>(moveSteps);

        for (int step = 0; step < moveSteps; ++step)
        {
            e.posX += stepX;
            e.posY += stepY;

            const float mapBottom = static_cast<float>(GetMapHeight());
            if (mapBottom > 0.0f && e.posY > mapBottom)
            {
                e.posY = mapBottom;
                e.velocityY = 0.0f;
                e.isGrounded = true;
            }

            ResolveEnemyMapCollision(e);
        }

        if (e.colliderId != -1)
        {
            const float bodyScale = GetEnemyBodyColliderScale(e.type);
            const float bodyWidth = e.width * bodyScale * GetEnemyBodyColliderWidthAdjust(e.type);
            const float bodyHeight = e.height * bodyScale;
            const float bodyCenterY = e.posY - (e.height * 0.5f);
            float left = e.posX - (bodyWidth * 0.5f);
            float top = bodyCenterY - (bodyHeight * 0.5f);
            UpdateCollider(e.colliderId, left, top, bodyWidth, bodyHeight);
        }

        if (e.isAttacking && e.type == EnemyType::BigQuartist && e.attackColliderId == -1)
        {
            bool canActivateAttackHit = true;
            if (e.animations != nullptr)
            {
                canActivateAttackHit = (e.animations->attack.currentFrame >= BIG_QUARTIST_ATTACK_HIT_START_FRAME);
            }

            if (canActivateAttackHit)
            {
                const float attackWidth = e.width * BIG_ATTACK_WIDTH_SCALE;
                const float attackHeight = e.height * BIG_ATTACK_HEIGHT_SCALE;
                const float backOffset = e.width * BIG_ATTACK_BACK_OFFSET_RATIO;
                const float frontOffset = e.width * BIG_ATTACK_FRONT_OFFSET_RATIO;
                const float attackLeft = e.isFacingRight ? (e.posX - backOffset) : (e.posX - frontOffset);
                const float attackTop = e.posY - attackHeight;
                e.attackColliderId = CreateCollider(ColliderTag::Attack, attackLeft, attackTop, attackWidth, attackHeight, &e);
            }
        }

        if (e.isAttacking && e.attackColliderId != -1)
        {
            const bool isBig = (e.type == EnemyType::BigQuartist);
            const bool isTwisted = (e.type == EnemyType::TwistedCaltis);

            float attackWidth = isBig ? (e.width * BIG_ATTACK_WIDTH_SCALE) : (isTwisted ? (e.width * TWISTED_ATTACK_WIDTH_SCALE) : (e.width * ENEMY_ATTACK_WIDTH_SCALE));
            float attackHeight = isBig ? (e.height * BIG_ATTACK_HEIGHT_SCALE) : (isTwisted ? (e.height * TWISTED_ATTACK_HEIGHT_SCALE) : (e.height * ENEMY_ATTACK_HEIGHT_SCALE));

            float attackLeft = 0.0f;
            if (isBig)
            {
                const float backOffset = e.width * BIG_ATTACK_BACK_OFFSET_RATIO;
                const float frontOffset = e.width * BIG_ATTACK_FRONT_OFFSET_RATIO;
                attackLeft = e.isFacingRight ? (e.posX - backOffset) : (e.posX - frontOffset);
            }
            else
            {
                const float attackFrontOffset = e.width * (isTwisted ? TWISTED_ATTACK_FRONT_OFFSET_RATIO : ENEMY_ATTACK_FRONT_OFFSET_RATIO);
                const float attackOffsetX = e.isFacingRight ? attackFrontOffset : -attackFrontOffset - attackWidth;
                attackLeft = e.posX + attackOffsetX;
            }

            const float attackCenterY = isTwisted ? (e.posY - e.height * TWISTED_ATTACK_CENTER_Y_RATIO) : (e.posY - attackHeight * 0.5f);
            const float attackTop = attackCenterY - attackHeight * 0.5f;
            UpdateCollider(e.attackColliderId, attackLeft, attackTop, attackWidth, attackHeight);
        }

        if (e.animations != nullptr)
        {
            if (e.isAttacking)
            {
                UpdateAnimation(e.animations->attack);
            }
            else if (std::fabs(e.velocityX) > 0.01f)
            {
                UpdateAnimation(e.animations->move);
            }
            else
            {
                UpdateAnimation(e.animations->idle);
            }
        }
    }

    UpdateEnemyProjectiles(slowMoScale);
}

void DrawEnemies()
{
    CameraData camera = GetCamera();

    for (const auto& e : g_enemies)
    {
        if (!e.active) continue;

        if (e.isDying && e.deathAnimFinished && ((e.deathBlinkTimer / 4) % 2 == 1))
        {
            continue;
        }

        int drawX = static_cast<int>((e.posX - camera.posX) * camera.scale);
        int drawY = static_cast<int>((e.posY - camera.posY) * camera.scale);
        int drawTopY = drawY - static_cast<int>(e.height * camera.scale);

        if (e.animations != nullptr)
        {
            AnimationData* currentAnim = nullptr;
            
            if (e.isDying)
            {
                currentAnim = &e.animations->die;
            }
            else if (e.isAttacking)
            {
                currentAnim = &e.animations->attack;
            }
            else if (std::fabs(e.velocityX) > 0.01f)
            {
                currentAnim = &e.animations->move;
            }
            else
            {
                currentAnim = &e.animations->idle;
            }
            
            if (currentAnim != nullptr && currentAnim->frames != nullptr)
            {
                const int frameHandle = GetCurrentAnimationFrame(*currentAnim);
                if (frameHandle != -1)
                {
                    int drawW = static_cast<int>(e.width * camera.scale);
                    int drawH = static_cast<int>(e.height * camera.scale);

                    if (e.type == EnemyType::TwistedCaltis && e.isAttacking)
                    {
                        int srcW = 0;
                        int srcH = 0;
                        GetGraphSize(frameHandle, &srcW, &srcH);
                        if (srcW > 0 && srcH > 0)
                        {
                            const float aspectW = e.height * (static_cast<float>(srcW) / static_cast<float>(srcH));
                            if (aspectW > e.width)
                            {
                                drawW = static_cast<int>(aspectW * camera.scale);
                            }
                        }
                        drawW = static_cast<int>(drawW * 1.10f);
                    }

                    const int baseHalfW = static_cast<int>(e.width * 0.5f * camera.scale);
                    const int baseLeft = drawX - baseHalfW;
                    const int baseRight = drawX + baseHalfW;
                    const int h = drawH;
                    const int top = drawY - h;
                    const int bottom = drawY;

                    int left = drawX - (drawW / 2);
                    int right = drawX + (drawW / 2);

                    if (e.type == EnemyType::TwistedCaltis && e.isAttacking)
                    {
                        if (e.isFacingRight)
                        {
                            left = baseLeft;
                            right = left + drawW;
                        }
                        else
                        {
                            right = baseRight;
                            left = right - drawW;
                        }
                    }

                    if ((e.type == EnemyType::AssassinCultist || e.type == EnemyType::BigQuartist) && e.isInvisible)
                    {
                        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 96);
                    }

                    if (e.isFacingRight)
                    {
                        DrawExtendGraph(right, top, left, bottom, frameHandle, TRUE);
                    }
                    else
                    {
                        DrawExtendGraph(left, top, right, bottom, frameHandle, TRUE);
                    }

                    if ((e.type == EnemyType::AssassinCultist || e.type == EnemyType::BigQuartist) && e.isInvisible)
                    {
                        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                    }
                }
            }
        }
    }

    DrawEnemyProjectiles(camera);
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

EnemyData* FindEnemyByColliderId(int colliderId)
{
    for (auto& e : g_enemies)
    {
        if (e.active && e.colliderId == colliderId)
        {
            return &e;
        }
    }
    return nullptr;
}

//============================================================
// アニメーション管理
//============================================================

EnemyAnimations* LoadEnemyAnimations(EnemyType type)
{
    EnemyAnimations* anims = new EnemyAnimations();

    switch (type)
    {
    case EnemyType::Slime:
        LoadAnimationFromFiles(anims->idle, "Data/Enemy/Slime/", "slime-idle", 4, 8, AnimationMode::Loop);
        LoadAnimationFromFiles(anims->move, "Data/Enemy/Slime/", "slime-move", 4, 6, AnimationMode::Loop);
        LoadAnimationFromFiles(anims->attack, "Data/Enemy/Slime/", "slime-attack", 5, 4, AnimationMode::Once);
        LoadAnimationFromFiles(anims->hurt, "Data/Enemy/Slime/", "slime-hurt", 4, 3, AnimationMode::Once);
        LoadAnimationFromFiles(anims->die, "Data/Enemy/Slime/", "slime-die", 4, 5, AnimationMode::Once);
        break;

    case EnemyType::Cultists:
    {
        const bool okIdle = LoadAnimationFromFiles_Indexed(anims->idle, "Data/Enemy/cultists/", "Cultist-Attack_idle", 1, 6, 6, AnimationMode::Loop);
        const bool okMove = LoadAnimationFromFiles_Indexed(anims->move, "Data/Enemy/cultists/", "Cultist-Attack_idle", 1, 6, 6, AnimationMode::Loop);
        const bool okAttack = LoadAnimationFromFiles_Indexed(anims->attack, "Data/Enemy/cultists/", "Cultist-Attack", 1, 10, 4, AnimationMode::Once);
        const bool okHurt = LoadAnimationFromFiles_Indexed(anims->hurt, "Data/Enemy/cultists/", "Cultist-Attack", 1, 10, 5, AnimationMode::Once);
        const bool okDie = LoadAnimationFromFiles_Indexed(anims->die, "Data/Enemy/cultists/", "Cultist-Attack_Death", 1, 12, 5, AnimationMode::Once);

        if (!okIdle) InitAnimation(anims->idle);
        if (!okMove) InitAnimation(anims->move);
        if (!okAttack) InitAnimation(anims->attack);
        if (!okHurt) InitAnimation(anims->hurt);
        if (!okDie) InitAnimation(anims->die);
        break;
    }

    case EnemyType::AssassinCultist:
    {
        const bool okIdle = LoadAnimationFromFiles_Indexed(anims->idle, "Data/Enemy/Assassin Cultist/", "Cultist-Assassin_ilde", 1, 8, 6, AnimationMode::Loop);
        const bool okMove = LoadAnimationFromFiles_Indexed(anims->move, "Data/Enemy/Assassin Cultist/", "Cultist-Assassin_Run", 1, 8, 5, AnimationMode::Loop);
        const bool okAttack = LoadAnimationFromFiles_Indexed(anims->attack, "Data/Enemy/Assassin Cultist/", "Cultist-Assassin_Jump", 1, 3, 4, AnimationMode::Once);
        const bool okHurt = LoadAnimationFromFiles_Indexed(anims->hurt, "Data/Enemy/Assassin Cultist/", "Cultist-Assassin_Jump", 1, 3, 4, AnimationMode::Once);
        const bool okDie = LoadAnimationFromFiles_Indexed(anims->die, "Data/Enemy/Assassin Cultist/", "Cultist-Assassin_Death", 1, 9, 5, AnimationMode::Once);

        if (!okIdle) InitAnimation(anims->idle);
        if (!okMove) InitAnimation(anims->move);
        if (!okAttack) InitAnimation(anims->attack);
        if (!okHurt) InitAnimation(anims->hurt);
        if (!okDie) InitAnimation(anims->die);
        break;
    }

    case EnemyType::BigQuartist:
    {
        const bool okIdle = LoadAnimationFromFiles_Indexed(anims->idle, "Data/Enemy/Big Quartist/", "Big-Cultist_Idle", 1, 8, 8, AnimationMode::Loop);
        const bool okMove = LoadAnimationFromFiles_Indexed(anims->move, "Data/Enemy/Big Quartist/", "Big-Cultist_run", 1, 8, 7, AnimationMode::Loop);
        const bool okAttack = LoadAnimationFromFiles_Indexed(anims->attack, "Data/Enemy/Big Quartist/", "Big-Cultist_Attack", 1, 20, 5, AnimationMode::Once);
        const bool okHurt = LoadAnimationFromFiles_Indexed(anims->hurt, "Data/Enemy/Big Quartist/", "Big-Cultist_Attack", 1, 20, 5, AnimationMode::Once);
        const bool okDie = LoadAnimationFromFiles_Indexed(anims->die, "Data/Enemy/Big Quartist/", "Big-Cultist_Death", 1, 12, 6, AnimationMode::Once);

        if (!okIdle) InitAnimation(anims->idle);
        if (!okMove) InitAnimation(anims->move);
        if (!okAttack) InitAnimation(anims->attack);
        if (!okHurt) InitAnimation(anims->hurt);
        if (!okDie) InitAnimation(anims->die);
        break;
    }

    case EnemyType::TwistedCaltis:
    {
        const bool okIdle = LoadAnimationFromFiles_Indexed(anims->idle, "Data/Enemy/Twisted Caltis/", "Twisted_Cultist_Idle", 1, 6, 6, AnimationMode::Loop);
        const bool okMove = LoadAnimationFromFiles_Indexed(anims->move, "Data/Enemy/Twisted Caltis/", "Twisted_Cultist_Walk", 1, 8, 5, AnimationMode::Loop);
        const bool okAttack = LoadAnimationFromFiles_Indexed(anims->attack, "Data/Enemy/Twisted Caltis/", "Twisted_Cultist_Attack", 1, 7, 3, AnimationMode::Once);
        const bool okHurt = LoadAnimationFromFiles_Indexed(anims->hurt, "Data/Enemy/Twisted Caltis/", "Twisted_Cultist_Attack", 1, 7, 5, AnimationMode::Once);
        const bool okDie = LoadAnimationFromFiles_Indexed(anims->die, "Data/Enemy/Twisted Caltis/", "Twisted_Cultist_Death", 1, 12, 5, AnimationMode::Once);

        if (!okIdle) InitAnimation(anims->idle);
        if (!okMove) InitAnimation(anims->move);
        if (!okAttack) InitAnimation(anims->attack);
        if (!okHurt) InitAnimation(anims->hurt);
        if (!okDie) InitAnimation(anims->die);
        break;
    }

    default:
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
