#pragma once
// 椅子（チェックポイント）に座っているかどうかを管理するシンプルな API
// 他モジュールからは IsPlayerSitting() を呼び、Checkpoint が座らせるときは SetPlayerSitting(true) を呼ぶ。
enum class SitState
{
    None,      // 何もしてない
    Sitting    // 座っている
};

void SetSitState(SitState state);
SitState GetSitState();

// 便利関数
bool IsPlayerSitting();