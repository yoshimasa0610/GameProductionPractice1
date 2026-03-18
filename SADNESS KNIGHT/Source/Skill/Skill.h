#pragma once
#include "SkillData.h"
#include "../Player/Player.h"
#include "../Collision/Collision.h"
#include <unordered_set>
#include <functional>
#include <vector>

class Skill
{
private:
    SkillData m_data;

    int m_currentCoolTime;
    bool m_isActive;
    int m_activeTimer;
    int m_comboIndex = 0;
    int m_comboTimer = 0;
    int m_frame = 0;
    bool m_hitActive = false;
    bool m_comboQueued = false;
    // Follow用
	float m_followOffsetX = 60.0f; // プレイヤーからの横距離
	int m_followAttackInterval = 60; // 攻撃間隔（フレーム）
	int m_followAttackTimer = 0; // 攻撃タイマー
    ColliderId m_followAttackCollider = -1;
	int m_followAttackLife = 0; // 攻撃の寿命（フレーム）
	int m_followAttackDuration = 10; // 攻撃の持続時間（フレーム）
	float m_followAttackWidth = 80; // 攻撃判定の幅
	float m_followAttackHeight = 80; // 攻撃判定の高さ
    ColliderId m_followCollider = -1;
    float m_followPosX = 0;
    float m_followPosY = 0;
    float m_followLerpSpeed = 0.15f; // 追従速度
    int m_followAttackDelay = 0;
    int m_followAttackCharge = 15; // 溜め時間
    struct FollowBullet
    {
        float x, y;
        float vx, vy;
        int life;
        ColliderId collider;
        std::unordered_set<void*> hitTargets;
    };

    std::vector<FollowBullet> m_followBullets;

    //　Summon型の処理
    float m_summonX = 0;
    float m_summonY = 0;
    ColliderId m_summonCollider = -1;
    struct SummonUnit
    {
        float x;
        float y;
        int timer;
        int attackTimer = 0;
        int attackInterval = 90;   // 攻撃間隔
        int attackLife = 0;
        ColliderId collider = -1;  // 本体
        ColliderId attackCollider = -1; // 攻撃判定
    };

    std::vector<SummonUnit> m_summons;
    int m_maxSummons = 2; // 同時数（後でdata化可能）
    float m_summonAttackWidth = 80;
    float m_summonAttackHeight = 80;
    int m_summonAttackDuration = 15;

    ColliderId m_attackCollider = -1;

    // 敵ごとのヒット履歴
    std::unordered_set<void*> m_hitTargets;
    std::function<void(int)> m_onConsumeUse;
public:
    Skill(const SkillData& data);

    void Update(PlayerData* player);
    void Activate(PlayerData* player);

    bool CanUse() const;
    bool IsActive() const { return m_isActive; }

    int GetID() const { return m_data.id; }
    SkillType GetType() const { return m_data.type; }
    int GetBaseUseCount() const { return m_data.maxUseCount; }
    float GetCurrentAttackRate() const;

    // ヒット管理
    bool RegisterHit(void* target);
    void ClearHitTargets();
    void StartCoolTime();
    // 強制終了（上限の使用回数まで消費したから）
    void ForceEnd();

    void SetConsumeCallback(std::function<void(int)> cb)
    {
        m_onConsumeUse = cb;
    }
    int GetCoolTime() const { return m_currentCoolTime; }
    int GetComboIndex() const { return m_comboIndex; }
    int GetFrame() const { return m_frame; }
    bool IsHitActive() const { return m_hitActive; }
    FollowBullet* FindBulletByCollider(ColliderId id);
};
