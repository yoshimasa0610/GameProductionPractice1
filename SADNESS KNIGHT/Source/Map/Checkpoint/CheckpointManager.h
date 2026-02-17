#pragma once

// 椅子（チェックポイント）に座っているかどうかを管理するシンプルな API
// 他モジュールからは IsPlayerSitting() を呼び、Checkpoint が座らせるときは SetPlayerSitting(true) を呼ぶ。

void SetPlayerSitting(bool sitting);
bool IsPlayerSitting();